/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

/* The following types and structures are private, but need to be exposed for
 * the inline functions. */
typedef struct cw_nxoe_stack_s cw_nxoe_stack_t;

struct cw_nxoe_stack_s
{
    cw_nxoe_t nxoe;

#ifdef CW_THREADS
    /* Access locked if locking bit set. */
    cw_mtx_t lock;
#endif

    /* Spare nxo's.  The spares array is filled starting at 0, and nspare is the
     * current number of spares stored in the array. */
    cw_nxo_t *spare[CW_LIBONYX_STACK_CACHE];
    cw_uint32_t nspare;

    /* Half the minimum allowable object pointer array length (two times as
     * large as the a_mincount argument passed to nxo_stack_new()).  'a' is
     * logically divided in half, so the usable length is half the true
     * length. */
    cw_uint32_t ahmin;

    /* Half the number of slots in 'a'.  'a' is logically divided in half, so
     * the usable length is half the true length. */
    cw_uint32_t ahlen;

    /* Base offset of range to use for the stack; either 0 or ahlen. */
    cw_uint32_t abase;

    /* a[abase + abeg] is the top stack element. */
    cw_uint32_t abeg;

    /* a[abase + aend] is the first element in 'a' past the bottom of the
     * stack. */
    cw_uint32_t aend;

    /* Object pointer array. */
    cw_nxo_t **a;

    /* Used to temporarily store information needed for reference iteration
     * in case a stack modification operation is interrupted while partially
     * complete. */

#ifdef CW_THREADS
    /* rstate is used to atomically toggle states pertaining to how stack
     * modification operations are protected against GC. */
    enum
    {
	RSTATE_NONE,  /* No GC protection currently needed. */
	RSTATE_RMASK, /* Mask a[base,beg,end} data with r{base,beg,end}. */
	RSTATE_RONLY, /* Only r{base,beg,end} data are valid. */
    } rstate;
#endif

    /* Base offset of range to use for temporary storage; either 0 or
     * ahlen. */
    cw_uint32_t rbase;

    /* r[rbase + rbeg] is the first element in 'r' being protected against
     * GC. */
    cw_uint32_t rbeg;

    /* r[rbase + rend] is the first element in 'r' past the range begin
     * protected agains GC. */
    cw_uint32_t rend;

    /* Object pointer array, usually (but not during some very small windows)
     * the same as 'a'. */
    cw_nxo_t **r;
};

void
nxo_stack_new(cw_nxo_t *a_nxo, cw_bool_t a_locking, cw_uint32_t a_mincount);

void
nxo_stack_copy(cw_nxo_t *a_to, cw_nxo_t *a_from);

#ifndef CW_USE_INLINES
cw_uint32_t
nxo_stack_count(cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_push(cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_bpush(cw_nxo_t *a_nxo);

cw_bool_t
nxo_stack_pop(cw_nxo_t *a_nxo);

cw_bool_t
nxo_stack_bpop(cw_nxo_t *a_nxo);

cw_bool_t
nxo_stack_npop(cw_nxo_t *a_nxo, cw_uint32_t a_count);

cw_bool_t
nxo_stack_nbpop(cw_nxo_t *a_nxo, cw_uint32_t a_count);

cw_nxo_t *
nxo_stack_get(const cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_bget(const cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_nget(const cw_nxo_t *a_nxo, cw_uint32_t a_index);

cw_nxo_t *
nxo_stack_nbget(const cw_nxo_t *a_nxo, cw_uint32_t a_index);

cw_bool_t
nxo_stack_exch(cw_nxo_t *a_nxo);

void
nxo_stack_rot(cw_nxo_t *a_nxo, cw_sint32_t a_amount);

cw_bool_t
nxo_stack_roll(cw_nxo_t *a_nxo, cw_uint32_t a_count, cw_sint32_t a_amount);
#endif

/* Private, but the inline functions need these prototypes. */
void
nxoe_p_stack_shrink(cw_nxoe_stack_t *a_stack);

cw_nxo_t *
nxoe_p_stack_push(cw_nxoe_stack_t *a_stack);

cw_nxo_t *
nxoe_p_stack_bpush(cw_nxoe_stack_t *a_stack);

void
nxoe_p_stack_npop(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count);

void
nxoe_p_stack_nbpop(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count);

#if (defined(CW_USE_INLINES) || defined(CW_NXO_STACK_C_))
#ifdef CW_THREADS
/* Private, but defined here for the inline functions. */
CW_INLINE void
nxoe_p_stack_lock(cw_nxoe_stack_t *a_nxoe)
{
    if (a_nxoe->nxoe.locking)
    {
	mtx_lock(&a_nxoe->lock);
    }
}

/* Private, but defined here for the inline functions. */
CW_INLINE void
nxoe_p_stack_unlock(cw_nxoe_stack_t *a_nxoe)
{
    if (a_nxoe->nxoe.locking)
    {
	mtx_unlock(&a_nxoe->lock);
    }
}
#endif

CW_INLINE cw_uint32_t
nxo_stack_count(cw_nxo_t *a_nxo)
{
    cw_uint32_t retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    retval = stack->aend - stack->abeg;
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif

    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_push(cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif

    /* Allocate new object. */
    if (stack->abeg > 0 && stack->nspare > 0)
    {
	stack->nspare--;
	retval = stack->spare[stack->nspare];
    }
    else
    {
	/* There isn't an empty slot above the top of the stack and/or there are
	 * no spares.  Deal with this in a separate function to keep this one
	 * small, since this code path is rarely executed. */
	retval = nxoe_p_stack_push(stack);
    }

    /* Insert new object. */
    nxo_no_new(retval);
    stack->a[stack->abase + stack->abeg - 1] = retval;
    mb_write();
    stack->abeg--;

#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif

    mb_write();
    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_bpush(cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif

    /* Allocate new object. */
    if (stack->aend + 1 < stack->ahlen && stack->nspare > 0)
    {
	stack->nspare--;
	retval = stack->spare[stack->nspare];
    }
    else
    {
	/* There isn't an empty slot below the bottom of the stack and/or there
	 * are no spares.  Deal with this in a separate function to keep this
	 * one small, since this code path is rarely executed. */
	retval = nxoe_p_stack_bpush(stack);
    }

    /* Insert new object. */
    nxo_no_new(retval);
    stack->a[stack->abase + stack->aend] = retval;
    mb_write();
    stack->aend++;

#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif

    mb_write();
    return retval;
}

CW_INLINE cw_bool_t
nxo_stack_pop(cw_nxo_t *a_nxo)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->aend == stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    stack->abeg++;
    mb_write();
    if (stack->nspare < CW_LIBONYX_STACK_CACHE)
    {
	stack->spare[stack->nspare] = stack->a[stack->abase + stack->abeg - 1];
#ifdef CW_DBG
	memset(stack->spare[stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	stack->nspare++;
    }
    else
    {
	nxa_free(stack->a[stack->abase + stack->abeg - 1], sizeof(cw_nxo_t));
    }

    /* Shrink the array, if necessary. */
    if (stack->aend - stack->abeg < stack->ahlen / 8
	&& stack->ahlen > stack->ahmin)
    {
	nxoe_p_stack_shrink(stack);
    }

    retval = FALSE;
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_bool_t
nxo_stack_bpop(cw_nxo_t *a_nxo)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->aend == stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    stack->aend--;
    mb_write();
    if (stack->nspare < CW_LIBONYX_STACK_CACHE)
    {
	stack->spare[stack->nspare] = stack->a[stack->abase + stack->aend];
#ifdef CW_DBG
	memset(stack->spare[stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	stack->nspare++;
    }
    else
    {
	nxa_free(stack->a[stack->abase + stack->aend], sizeof(cw_nxo_t));
    }

    /* Shrink the array, if necessary. */
    if (stack->aend - stack->abeg < stack->ahlen / 8
	&& stack->ahlen > stack->ahmin)
    {
	nxoe_p_stack_shrink(stack);
    }

    retval = FALSE;
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_bool_t
nxo_stack_npop(cw_nxo_t *a_nxo, cw_uint32_t a_count)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;
    cw_uint32_t i;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_count > stack->aend - stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    stack->abeg += a_count;
    mb_write();
    if (stack->nspare + a_count <= CW_LIBONYX_STACK_CACHE)
    {
	for (i = 0; i < a_count; i++)
	{
	    stack->spare[stack->nspare]
		= stack->a[stack->abase + stack->abeg - a_count + i];
#ifdef CW_DBG
	    memset(stack->spare[stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	    stack->nspare++;
	}
    }
    else
    {
	/* Spares need to be discarded.  Do this in a separate function to keep
	 * this one small, since this code path is rarely executed. */
	nxoe_p_stack_npop(stack, a_count);
    }

    /* Shrink the array, if necessary. */
    if (stack->aend - stack->abeg < stack->ahlen / 8
	&& stack->ahlen > stack->ahmin)
    {
	nxoe_p_stack_shrink(stack);
    }

    retval = FALSE;
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_bool_t
nxo_stack_nbpop(cw_nxo_t *a_nxo, cw_uint32_t a_count)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;
    cw_uint32_t i;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_count > stack->aend - stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    stack->aend -= a_count;
    mb_write();
    if (stack->nspare + a_count <= CW_LIBONYX_STACK_CACHE)
    {
	for (i = 0; i < a_count; i++)
	{
	    stack->spare[stack->nspare]
		= stack->a[stack->abase + stack->aend + i];
#ifdef CW_DBG
	    memset(stack->spare[stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	    stack->nspare++;
	}
    }
    else
    {
	/* Spares need to be discarded.  Do this in a separate function to keep
	 * this one small, since this code path is rarely executed. */
	nxoe_p_stack_nbpop(stack, a_count);
    }

    /* Shrink the array, if necessary. */
    if (stack->aend - stack->abeg < stack->ahlen / 8
	&& stack->ahlen > stack->ahmin)
    {
	nxoe_p_stack_shrink(stack);
    }

    retval = FALSE;
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->aend == stack->abeg)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = stack->a[stack->abase + stack->abeg];

    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_bget(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->aend == stack->abeg)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = stack->a[stack->abase + stack->aend - 1];

    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_nget(const cw_nxo_t *a_nxo, cw_uint32_t a_index)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_index >= stack->aend - stack->abeg)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = stack->a[stack->abase + stack->abeg + a_index];

    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_nbget(const cw_nxo_t *a_nxo, cw_uint32_t a_index)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_index >= stack->aend - stack->abeg)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = stack->a[stack->abase + stack->aend - 1 - a_index];

    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_bool_t
nxo_stack_exch(cw_nxo_t *a_nxo)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->aend - stack->abeg < 2)
    {
#ifdef CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	retval = TRUE;
	goto ERROR;
    }

    /* Protect the region that is being modified.  Do the exchange operation at
     * the same time, in order to be consistent with nxo_stack_roll(). */
    stack->rbeg = stack->abeg;
    stack->rend = stack->aend;
    stack->r[stack->rbase + stack->rbeg]
	= stack->a[stack->abase + stack->abeg + 1];
    stack->r[stack->rbase + stack->rbeg + 1]
	= stack->a[stack->abase + stack->abeg];
#ifdef CW_THREADS
    mb_write();
    stack->rstate = RSTATE_RMASK;
    mb_write();
#endif

    /* Exchange. */
    memcpy(&stack->a[stack->abase + stack->abeg],
	   &stack->r[stack->rbase + stack->rbeg],
	   2 * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
    /* Unprotect. */
    mb_write();
    stack->rstate = RSTATE_NONE;

    nxoe_p_stack_unlock(stack);
#endif

    retval = FALSE;
    ERROR:
    return retval;
}

CW_INLINE void
nxo_stack_rot(cw_nxo_t *a_nxo, cw_sint32_t a_amount)
{
    cw_nxoe_stack_t *stack;
    cw_uint32_t trbase, count;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif

    cw_assert(stack->aend > stack->abeg);

    /* Calculate the current index of the element that will end up on top of the
     * stack.  This allows us to do the rotation with a minimum number of
     * memcpy()s.  This code also has the side effect of 'mod'ing the rotate
     * amount, so that we don't spend a bunch of time rotating the stack if the
     * user specifies a rotate amount larger than the stack.  A decent program
     * will never do this, so it's not worth specifically optimizing, but it
     * falls out of these calculations with no extra work, since we already have
     * to deal with upward versus downward rotating calculations. */
    count = stack->aend - stack->abeg;
    if (a_amount < 0)
    {
	/* Convert a_amount to a positive equivalent. */
	a_amount += ((a_amount - count) / count) * count;
    }
    a_amount %= count;
    a_amount += count;
    a_amount %= count;

    /* Do this check after the above calculations, just in case the rotate
     * amount is an even multiple of the stack size. */
    if (a_amount == 0)
    {
	/* Noop. */
#ifdef CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	return;
    }

    /* We now have:
     *
     *         /----------\ \  \
     *         |          | |  |
     *         |          | |   \
     *         |          | |   / a_amount
     *         |          | |  |
     *         |          | |  /
     *         \----------/  \
     * top --> /----------\  / count
     *         |          | |  \
     *         |          | |  |
     *         |          | |   \
     *         |          | |   / count - a_amount
     *         |          | |  |
     *         \----------/ /  /
     *
     * Decide whether to do the rotation in place, or copy the entire array. */
    if (count - a_amount <= stack->abeg)
    {
	/* Copy the end of the array to the beginning. */

	/* Protect the region that is being moved. */
	trbase = stack->rbase;
	stack->rbase = stack->abase;
	stack->rbeg = stack->abeg + a_amount;
	stack->rend = stack->aend;
#ifdef CW_THREADS
	mb_write();
	stack->rstate = RSTATE_RMASK;
	mb_write();
#endif

	/* Rotate. */
	memcpy(&stack->a[stack->abase + stack->abeg - (count - a_amount)],
	       &stack->a[stack->abase + stack->abeg + a_amount],
	       (count - a_amount) * sizeof(cw_nxo_t *));
	stack->abeg -= (count - a_amount);
	stack->aend -= (count - a_amount);

	/* Unprotect. */
#ifdef CW_THREADS
	mb_write();
	stack->rstate = RSTATE_NONE;
	mb_write();
#endif
	stack->rbase = trbase;
    }
    else if (a_amount <= stack->ahlen - stack->aend)
    {
	/* Copy the beginning of the array to the end. */

	/* Protect the region that is being moved. */
	trbase = stack->rbase;
	stack->rbase = stack->abase;
	stack->rbeg = stack->abeg;
	stack->rend = stack->abeg + a_amount;
#ifdef CW_THREADS
	mb_write();
	stack->rstate = RSTATE_RMASK;
	mb_write();
#endif

	/* Rotate. */
	memcpy(&stack->a[stack->abase + stack->aend],
	       &stack->a[stack->abase + stack->abeg],
	       a_amount * sizeof(cw_nxo_t *));
	stack->aend += a_amount;
	stack->abeg += a_amount;

	/* Unprotect. */
#ifdef CW_THREADS
	mb_write();
	stack->rstate = RSTATE_NONE;
	mb_write();
#endif
	stack->rbase = trbase;
    }
    else
    {
	/* Copy the entire array, swapping abase and rbase. */

	/* Protect the region that is being moved. */
	trbase = stack->rbase;
	stack->rbase = stack->abase;
	stack->rbeg = stack->abeg;
	stack->rend = stack->aend;
#ifdef CW_THREADS
	mb_write();
	stack->rstate = RSTATE_RONLY;
	mb_write();
#endif

	/* Rotate and center. */
	stack->abase = trbase;
	stack->abeg = (stack->ahlen - (stack->rend - stack->rbeg)) / 2;
	stack->aend = stack->abeg + count;
	memcpy(&stack->a[stack->abase + stack->abeg],
	       &stack->r[stack->rbase + stack->rbeg + a_amount],
	       (count - a_amount) * sizeof(cw_nxo_t *));
	memcpy(&stack->a[stack->abase + stack->abeg + (count - a_amount)],
	       &stack->r[stack->rbase + stack->rbeg],
	       a_amount * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
	/* Unprotect. */
	mb_write();
	stack->rstate = RSTATE_NONE;
#endif
    }
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
}

CW_INLINE cw_bool_t
nxo_stack_roll(cw_nxo_t *a_nxo, cw_uint32_t a_count, cw_sint32_t a_amount)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

    cw_assert(a_count > 0);

    /* Calculate the current index of the element that will end up on top of the
     * stack.  This allows us to do the roll with a minimum number of memcpy()s.
     * This code also has the side effect of 'mod'ing the roll amount, so that
     * we don't spend a bunch of time rolling the stack if the user specifies a
     * roll amount larger than the roll region.  A decent program will never do
     * this, so it's not worth specifically optimizing, but it falls out of
     * these calculations with no extra work, since we already have to deal with
     * upward versus downward rolling calculations. */
    if (a_amount < 0)
    {
	/* Convert a_amount to a positive equivalent. */
	a_amount += ((a_amount - a_count) / a_count) * a_count;
    }
    a_amount %= a_count;
    a_amount += a_count;
    a_amount %= a_count;

    /* Do this check after the above calculations, just in case the roll amount
     * is an even multiple of the roll region. */
    if (a_amount == 0)
    {
	/* Noop. */
	goto RETURN;
    }

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_count > stack->aend - stack->abeg)
    {
#ifdef CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	retval = TRUE;
	goto ERROR;
    }

    /* Protect the region that is being modified.  Do the roll operation at the
     * same time, in order to save one memcpy() call. */
    stack->rbeg = stack->abeg;
    stack->rend = stack->abeg + a_count;
    memcpy(&stack->r[stack->rbase + stack->rbeg],
	   &stack->a[stack->abase + stack->abeg + a_amount],
	   (a_count - a_amount) * sizeof(cw_nxo_t *));
    memcpy(&stack->r[stack->rbase + stack->rbeg + (a_count - a_amount)],
	   &stack->a[stack->abase + stack->abeg],
	   a_amount * sizeof(cw_nxo_t *));
#ifdef CW_THREADS
    mb_write();
    stack->rstate = RSTATE_RMASK;
    mb_write();
#endif

    /* Roll. */
    memcpy(&stack->a[stack->abase + stack->abeg],
	   &stack->r[stack->rbase + stack->rbeg],
	   a_count * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
    /* Unprotect. */
    mb_write();
    stack->rstate = RSTATE_NONE;

    nxoe_p_stack_unlock(stack);
#endif

    RETURN:
    retval = FALSE;
    ERROR:
    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_STACK_C_)) */

/* Convenience wrapper macros for use where errors should cause an error and
 * immediate return. */
#define NXO_STACK_POP(a_nxo, a_thread)					\
    do									\
    {									\
	if (nxo_stack_pop(a_nxo))					\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)

#define NXO_STACK_BPOP(a_nxo, a_thread)					\
    do									\
    {									\
	if (nxo_stack_bpop(a_nxo))					\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)

#define NXO_STACK_NPOP(a_nxo, a_thread, a_count)			\
    do									\
    {									\
	if (nxo_stack_npop((a_nxo), (a_count)))				\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)

#define NXO_STACK_NBPOP(a_nxo, a_thread, a_count)			\
    do									\
    {									\
	if (nxo_stack_nbpop((a_nxo), (a_count)))			\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)

#define NXO_STACK_GET(r_nxo, a_nxo, a_thread)				\
     do									\
     {									\
	(r_nxo) = nxo_stack_get(a_nxo);					\
	if ((r_nxo) == NULL)						\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)

#define NXO_STACK_BGET(r_nxo, a_nxo, a_thread)				\
     do									\
     {									\
	(r_nxo) = nxo_stack_bget(a_nxo);				\
	if ((r_nxo) == NULL)						\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)

#define NXO_STACK_NGET(r_nxo, a_nxo, a_thread, a_index)			\
    do									\
    {									\
	(r_nxo) = nxo_stack_nget((a_nxo), (a_index));			\
	if ((r_nxo) == NULL)						\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)

#define NXO_STACK_NBGET(r_nxo, a_nxo, a_thread, a_index)		\
    do									\
    {									\
	(r_nxo) = nxo_stack_nbget((a_nxo), (a_index));			\
	if ((r_nxo) == NULL)						\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)
