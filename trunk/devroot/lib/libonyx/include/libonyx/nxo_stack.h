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
     * protected against GC. */
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

cw_uint32_t
nxoe_p_stack_count_locking(cw_nxoe_stack_t *a_stack);

cw_nxo_t *
nxoe_p_stack_push_hard(cw_nxoe_stack_t *a_stack);

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_push_locking(cw_nxoe_stack_t *a_stack);
#endif

cw_nxo_t *
nxoe_p_stack_bpush_hard(cw_nxoe_stack_t *a_stack);

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_bpush_locking(cw_nxoe_stack_t *a_stack);
#endif

#ifdef CW_THREADS
cw_bool_t
nxoe_p_stack_pop_locking(cw_nxoe_stack_t *a_stack);
#endif

#ifdef CW_THREADS
cw_bool_t
nxoe_p_stack_bpop_locking(cw_nxoe_stack_t *a_stack);
#endif

void
nxoe_p_stack_npop_hard(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count);

#ifdef CW_THREADS
cw_bool_t
nxoe_p_stack_npop_locking(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count);
#endif

void
nxoe_p_stack_nbpop_hard(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count);

#ifdef CW_THREADS
cw_bool_t
nxoe_p_stack_nbpop_locking(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count);
#endif

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_get_locking(cw_nxoe_stack_t *a_stack);
#endif

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_bget_locking(cw_nxoe_stack_t *a_stack);
#endif

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_nget_locking(cw_nxoe_stack_t *a_stack, cw_uint32_t a_index);
#endif

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_nbget_locking(cw_nxoe_stack_t *a_stack, cw_uint32_t a_index);
#endif

#ifdef CW_THREADS
cw_bool_t
nxoe_p_stack_exch_locking(cw_nxoe_stack_t *a_stack);
#endif

#ifdef CW_THREADS
void
nxoe_p_stack_rot_locking(cw_nxoe_stack_t *a_stack, cw_sint32_t a_amount);
#endif

#ifdef CW_THREADS
cw_bool_t
nxoe_p_stack_roll_locking(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count,
			  cw_sint32_t a_amount);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_STACK_C_))
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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_count_locking(stack);
    }
    else
#endif
    {
	retval = stack->aend - stack->abeg;
    }

    return retval;
}

CW_INLINE cw_nxo_t *
nxoe_p_stack_push(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    /* Allocate new object. */
    if (a_stack->abeg > 0 && a_stack->nspare > 0)
    {
	a_stack->nspare--;
	retval = a_stack->spare[a_stack->nspare];
    }
    else
    {
	/* There isn't an empty slot above the top of the stack and/or there are
	 * no spares. */
	retval = nxoe_p_stack_push_hard(a_stack);
    }

    /* Insert new object. */
    nxo_no_new(retval);
    a_stack->a[a_stack->abase + a_stack->abeg - 1] = retval;
    mb_write();
    a_stack->abeg--;

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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_push_locking(stack);
    }
    else
#endif
    {
	retval = nxoe_p_stack_push(stack);
    }

    mb_write();
    return retval;
}

CW_INLINE cw_nxo_t *
nxoe_p_stack_bpush(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    /* Allocate new object. */
    if (a_stack->aend + 1 < a_stack->ahlen && a_stack->nspare > 0)
    {
	a_stack->nspare--;
	retval = a_stack->spare[a_stack->nspare];
    }
    else
    {
	/* There isn't an empty slot below the bottom of the stack and/or
	 * there are no spares.  Deal with this in a separate function to
	 * keep this one small, since this code path is rarely executed. */
	retval = nxoe_p_stack_bpush_hard(a_stack);
    }

    /* Insert new object. */
    nxo_no_new(retval);
    a_stack->a[a_stack->abase + a_stack->aend] = retval;
    mb_write();
    a_stack->aend++;

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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_bpush_locking(stack);
    }
    else
#endif
    {
	retval = nxoe_p_stack_bpush(stack);
    }

    mb_write();
    return retval;
}

CW_INLINE cw_bool_t
nxoe_p_stack_pop(cw_nxoe_stack_t *a_stack)
{
    cw_bool_t retval;

    if (a_stack->aend == a_stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    a_stack->abeg++;
    mb_write();
    if (a_stack->nspare < CW_LIBONYX_STACK_CACHE)
    {
	a_stack->spare[a_stack->nspare] = a_stack->a[a_stack->abase
						     + a_stack->abeg - 1];
#ifdef CW_DBG
	memset(a_stack->spare[a_stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	a_stack->nspare++;
    }
    else
    {
	nxa_free(a_stack->a[a_stack->abase + a_stack->abeg - 1],
		 sizeof(cw_nxo_t));
    }

    /* Shrink the array, if necessary. */
    if (a_stack->aend - a_stack->abeg < (a_stack->ahlen >> 3)
	&& a_stack->ahlen > a_stack->ahmin)
    {
	nxoe_p_stack_shrink(a_stack);
    }

    retval = FALSE;
    RETURN:
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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_pop_locking(stack);
    }
    else
#endif
    {
	retval = nxoe_p_stack_pop(stack);
    }

    return retval;
}

CW_INLINE cw_bool_t
nxoe_p_stack_bpop(cw_nxoe_stack_t *a_stack)
{
    cw_bool_t retval;

    if (a_stack->aend == a_stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    a_stack->aend--;
    mb_write();
    if (a_stack->nspare < CW_LIBONYX_STACK_CACHE)
    {
	a_stack->spare[a_stack->nspare] = a_stack->a[a_stack->abase
						     + a_stack->aend];
#ifdef CW_DBG
	memset(a_stack->spare[a_stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	a_stack->nspare++;
    }
    else
    {
	nxa_free(a_stack->a[a_stack->abase + a_stack->aend], sizeof(cw_nxo_t));
    }

    /* Shrink the array, if necessary. */
    if (a_stack->aend - a_stack->abeg < (a_stack->ahlen >> 3)
	&& a_stack->ahlen > a_stack->ahmin)
    {
	nxoe_p_stack_shrink(a_stack);
    }

    retval = FALSE;
    RETURN:
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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_bpop_locking(stack);
    }
    else
#endif
    {
	retval = nxoe_p_stack_bpop(stack);
    }

    return retval;
}

CW_INLINE cw_bool_t
nxoe_p_stack_npop(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count)
{
    cw_bool_t retval;
    cw_uint32_t i;

    if (a_count > a_stack->aend - a_stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    a_stack->abeg += a_count;
    mb_write();
    if (a_stack->nspare + a_count <= CW_LIBONYX_STACK_CACHE)
    {
	for (i = 0; i < a_count; i++)
	{
	    a_stack->spare[a_stack->nspare]
		= a_stack->a[a_stack->abase + a_stack->abeg - a_count + i];
#ifdef CW_DBG
	    memset(a_stack->spare[a_stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	    a_stack->nspare++;
	}
    }
    else
    {
	/* Spares need to be discarded. */
	nxoe_p_stack_npop_hard(a_stack, a_count);
    }

    /* Shrink the array, if necessary. */
    if (a_stack->aend - a_stack->abeg < (a_stack->ahlen >> 3)
	&& a_stack->ahlen > a_stack->ahmin)
    {
	nxoe_p_stack_shrink(a_stack);
    }

    retval = FALSE;
    RETURN:
    return retval;
}

CW_INLINE cw_bool_t
nxo_stack_npop(cw_nxo_t *a_nxo, cw_uint32_t a_count)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_npop_locking(stack, a_count);
    }
    else
#endif
    {
	retval = nxoe_p_stack_npop(stack, a_count);
    }

    return retval;
}

CW_INLINE cw_bool_t
nxoe_p_stack_nbpop(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count)
{
    cw_bool_t retval;
    cw_uint32_t i;

    if (a_count > a_stack->aend - a_stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    a_stack->aend -= a_count;
    mb_write();
    if (a_stack->nspare + a_count <= CW_LIBONYX_STACK_CACHE)
    {
	for (i = 0; i < a_count; i++)
	{
	    a_stack->spare[a_stack->nspare]
		= a_stack->a[a_stack->abase + a_stack->aend + i];
#ifdef CW_DBG
	    memset(a_stack->spare[a_stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	    a_stack->nspare++;
	}
    }
    else
    {
	/* Spares need to be discarded.  Do this in a separate function to
	 * keep this one small, since this code path is rarely executed. */
	nxoe_p_stack_nbpop_hard(a_stack, a_count);
    }

    /* Shrink the array, if necessary. */
    if (a_stack->aend - a_stack->abeg < (a_stack->ahlen >> 3)
	&& a_stack->ahlen > a_stack->ahmin)
    {
	nxoe_p_stack_shrink(a_stack);
    }

    retval = FALSE;
    RETURN:
    return retval;
}

CW_INLINE cw_bool_t
nxo_stack_nbpop(cw_nxo_t *a_nxo, cw_uint32_t a_count)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_nbpop_locking(stack, a_count);
    }
    else
#endif
    {
	retval = nxoe_p_stack_nbpop(stack, a_count);
    }

    return retval;
}

CW_INLINE cw_nxo_t *
nxoe_p_stack_get(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    if (a_stack->aend == a_stack->abeg)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = a_stack->a[a_stack->abase + a_stack->abeg];

    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_get_locking(stack);
    }
    else
#endif
    {
	retval = nxoe_p_stack_get(stack);
    }

    return retval;
}

CW_INLINE cw_nxo_t *
nxoe_p_stack_bget(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    if (a_stack->aend == a_stack->abeg)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = a_stack->a[a_stack->abase + a_stack->aend - 1];

    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_bget_locking(stack);
    }
    else
#endif
    {
	retval = nxoe_p_stack_bget(stack);
    }

    return retval;
}

CW_INLINE cw_nxo_t *
nxoe_p_stack_nget(cw_nxoe_stack_t *a_stack, cw_uint32_t a_index)
{
    cw_nxo_t *retval;

    if (a_index >= a_stack->aend - a_stack->abeg)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = a_stack->a[a_stack->abase + a_stack->abeg + a_index];

    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_nget_locking(stack, a_index);
    }
    else
#endif
    {
	retval = nxoe_p_stack_nget(stack, a_index);
    }

    return retval;
}

CW_INLINE cw_nxo_t *
nxoe_p_stack_nbget(cw_nxoe_stack_t *a_stack, cw_uint32_t a_index)
{
    cw_nxo_t *retval;

    if (a_index >= a_stack->aend - a_stack->abeg)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = a_stack->a[a_stack->abase + a_stack->aend - 1 - a_index];

    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_nbget_locking(stack, a_index);
    }
    else
#endif
    {
	retval = nxoe_p_stack_nbget(stack, a_index);
    }

    return retval;
}

CW_INLINE cw_bool_t
nxoe_p_stack_exch(cw_nxoe_stack_t *a_stack)
{
    cw_bool_t retval;

    if (a_stack->aend - a_stack->abeg < 2)
    {
	retval = TRUE;
	goto ERROR;
    }

    /* Protect the region that is being modified.  Do the exchange operation at
     * the same time, in order to be consistent with nxo_stack_roll(). */
    a_stack->rbeg = a_stack->abeg;
    a_stack->rend = a_stack->abeg + 2;
    a_stack->r[a_stack->rbase + a_stack->rbeg]
	= a_stack->a[a_stack->abase + a_stack->abeg + 1];
    a_stack->r[a_stack->rbase + a_stack->rbeg + 1]
	= a_stack->a[a_stack->abase + a_stack->abeg];
#ifdef CW_THREADS
    mb_write();
    a_stack->rstate = RSTATE_RMASK;
    mb_write();
#endif

    /* Exchange. */
    memcpy(&a_stack->a[a_stack->abase + a_stack->abeg],
	   &a_stack->r[a_stack->rbase + a_stack->rbeg],
	   2 * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
    /* Unprotect. */
    mb_write();
    a_stack->rstate = RSTATE_NONE;
#endif

    retval = FALSE;
    ERROR:
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
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_exch_locking(stack);
    }
    else
#endif
    {
	retval = nxoe_p_stack_exch(stack);
    }

    return retval;
}

CW_INLINE void
nxoe_p_stack_rot(cw_nxoe_stack_t *a_stack, cw_sint32_t a_amount)
{
    cw_uint32_t trbase, count;

    cw_assert(a_stack->aend > a_stack->abeg);

    /* Calculate the current index of the element that will end up on top of the
     * stack.  This allows us to do the rotation with a minimum number of
     * memcpy()s.  This code also has the side effect of 'mod'ing the rotate
     * amount, so that we don't spend a bunch of time rotating the stack if the
     * user specifies a rotate amount larger than the stack.  A decent program
     * will never do this, so it's not worth specifically optimizing, but it
     * falls out of these calculations with no extra work, since we already have
     * to deal with upward versus downward rotating calculations. */
    count = a_stack->aend - a_stack->abeg;
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
    if (count - a_amount <= a_stack->abeg)
    {
	/* Copy the end of the array to the beginning. */

	/* Protect the region that is being moved. */
	trbase = a_stack->rbase;
	a_stack->rbase = a_stack->abase;
	a_stack->rbeg = a_stack->abeg + a_amount;
	a_stack->rend = a_stack->aend;
#ifdef CW_THREADS
	mb_write();
	a_stack->rstate = RSTATE_RMASK;
	mb_write();
#endif

	/* Rotate. */
	memcpy(&a_stack->a[a_stack->abase + a_stack->abeg - (count - a_amount)],
	       &a_stack->a[a_stack->abase + a_stack->abeg + a_amount],
	       (count - a_amount) * sizeof(cw_nxo_t *));
	a_stack->abeg -= (count - a_amount);
	a_stack->aend -= (count - a_amount);

	/* Unprotect. */
#ifdef CW_THREADS
	mb_write();
	a_stack->rstate = RSTATE_NONE;
	mb_write();
#endif
	a_stack->rbase = trbase;
    }
    else if (a_amount <= a_stack->ahlen - a_stack->aend)
    {
	/* Copy the beginning of the array to the end. */

	/* Protect the region that is being moved. */
	trbase = a_stack->rbase;
	a_stack->rbase = a_stack->abase;
	a_stack->rbeg = a_stack->abeg;
	a_stack->rend = a_stack->abeg + a_amount;
#ifdef CW_THREADS
	mb_write();
	a_stack->rstate = RSTATE_RMASK;
	mb_write();
#endif

	/* Rotate. */
	memcpy(&a_stack->a[a_stack->abase + a_stack->aend],
	       &a_stack->a[a_stack->abase + a_stack->abeg],
	       a_amount * sizeof(cw_nxo_t *));
	a_stack->aend += a_amount;
	a_stack->abeg += a_amount;

	/* Unprotect. */
#ifdef CW_THREADS
	mb_write();
	a_stack->rstate = RSTATE_NONE;
	mb_write();
#endif
	a_stack->rbase = trbase;
    }
    else
    {
	/* Copy the entire array, swapping abase and rbase. */

	/* Protect the region that is being moved. */
	trbase = a_stack->rbase;
	a_stack->rbase = a_stack->abase;
	a_stack->rbeg = a_stack->abeg;
	a_stack->rend = a_stack->aend;
#ifdef CW_THREADS
	mb_write();
	a_stack->rstate = RSTATE_RONLY;
	mb_write();
#endif

	/* Rotate and center. */
	a_stack->abase = trbase;
	a_stack->abeg = (a_stack->ahlen - (a_stack->rend - a_stack->rbeg)) / 2;
	a_stack->aend = a_stack->abeg + count;
	memcpy(&a_stack->a[a_stack->abase + a_stack->abeg],
	       &a_stack->r[a_stack->rbase + a_stack->rbeg + a_amount],
	       (count - a_amount) * sizeof(cw_nxo_t *));
	memcpy(&a_stack->a[a_stack->abase + a_stack->abeg + (count - a_amount)],
	       &a_stack->r[a_stack->rbase + a_stack->rbeg],
	       a_amount * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
	/* Unprotect. */
	mb_write();
	a_stack->rstate = RSTATE_NONE;
#endif
    }
}

CW_INLINE void
nxo_stack_rot(cw_nxo_t *a_nxo, cw_sint32_t a_amount)
{
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    if (stack->nxoe.locking)
    {
	nxoe_p_stack_rot_locking(stack, a_amount);
    }
    else
#endif
    {
	nxoe_p_stack_rot(stack, a_amount);
    }
}

CW_INLINE cw_bool_t
nxoe_p_stack_roll(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count,
		  cw_sint32_t a_amount)
{
    cw_bool_t retval;

    if (a_count > a_stack->aend - a_stack->abeg)
    {
	retval = TRUE;
	goto RETURN;
    }

    /* Protect the region that is being modified.  Do the roll operation at the
     * same time, in order to save one memcpy() call. */
    a_stack->rbeg = a_stack->abeg;
    a_stack->rend = a_stack->abeg + a_count;
    memcpy(&a_stack->r[a_stack->rbase + a_stack->rbeg],
	   &a_stack->a[a_stack->abase + a_stack->abeg + a_amount],
	   (a_count - a_amount) * sizeof(cw_nxo_t *));
    memcpy(&a_stack->r[a_stack->rbase + a_stack->rbeg + (a_count - a_amount)],
	   &a_stack->a[a_stack->abase + a_stack->abeg],
	   a_amount * sizeof(cw_nxo_t *));
#ifdef CW_THREADS
    mb_write();
    a_stack->rstate = RSTATE_RMASK;
    mb_write();
#endif

    /* Roll. */
    memcpy(&a_stack->a[a_stack->abase + a_stack->abeg],
	   &a_stack->r[a_stack->rbase + a_stack->rbeg],
	   a_count * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
    /* Unprotect. */
    mb_write();
    a_stack->rstate = RSTATE_NONE;
#endif

    retval = FALSE;
    RETURN:
    return retval;
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
	retval = FALSE;
	goto RETURN;
    }

#ifdef CW_THREADS
    if (stack->nxoe.locking)
    {
	retval = nxoe_p_stack_roll_locking(stack, a_count, a_amount);
    }
    else
#endif
    {
	retval = nxoe_p_stack_roll(stack, a_count, a_amount);
    }

    RETURN:
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
