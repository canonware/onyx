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
typedef struct cw_nxoe_stacko_s cw_nxoe_stacko_t;

struct cw_nxoe_stacko_s
{
    /* Payload.  Must be first field. */
    cw_nxo_t nxo;

    /* Stack/spares linkage. */
    ql_elm(cw_nxoe_stacko_t) link;
};

struct cw_nxoe_stack_s
{
    cw_nxoe_t nxoe;

#ifdef CW_THREADS
    /* Access locked if locking bit set. */
    cw_mtx_t lock;
#endif

    /* Stack. */
    ql_head(cw_nxoe_stacko_t) stack;

    /* Number of stack elements. */
    cw_uint32_t count;

    /* Number of spare elements. */
    cw_uint32_t nspare;

    /* Not used, just under stack bottom. */
    cw_nxoe_stacko_t under;

    cw_nxoe_stacko_t *ref_stacko;

#ifdef CW_THREADS
    cw_nxoe_stacko_t *below;
#endif
};

void
nxo_stack_new(cw_nxo_t *a_nxo, cw_bool_t a_locking);

void
nxo_stack_copy(cw_nxo_t *a_to, cw_nxo_t *a_from);

#ifndef CW_USE_INLINES
cw_uint32_t
nxo_stack_count(cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_push(cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_bpush(cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_under_push(cw_nxo_t *a_nxo, cw_nxo_t *a_object);

cw_bool_t
nxo_stack_pop(cw_nxo_t *a_nxo);

cw_bool_t
nxo_stack_bpop(cw_nxo_t *a_nxo);

cw_bool_t
nxo_stack_npop(cw_nxo_t *a_nxo, cw_uint32_t a_count);

cw_bool_t
nxo_stack_nbpop(cw_nxo_t *a_nxo, cw_uint32_t a_count);

void
nxo_stack_remove(cw_nxo_t *a_nxo, cw_nxo_t *a_object);

cw_nxo_t *
nxo_stack_get(const cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_bget(const cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_stack_nget(const cw_nxo_t *a_nxo, cw_uint32_t a_index);

cw_nxo_t *
nxo_stack_nbget(const cw_nxo_t *a_nxo, cw_uint32_t a_index);

cw_nxo_t *
nxo_stack_down_get(const cw_nxo_t *a_nxo, cw_nxo_t *a_object);

cw_nxo_t *
nxo_stack_up_get(const cw_nxo_t *a_nxo, cw_nxo_t *a_object);

cw_bool_t
nxo_stack_exch(cw_nxo_t *a_nxo);

void
nxo_stack_rot(cw_nxo_t *a_nxo, cw_sint32_t a_amount);

cw_bool_t
nxo_stack_roll(cw_nxo_t *a_nxo, cw_uint32_t a_count, cw_sint32_t a_amount);
#endif

/* Private, but the inline functions need these prototypes. */
cw_nxoe_stacko_t *
nxoe_p_stack_push(cw_nxoe_stack_t *a_stack);

cw_nxoe_stacko_t *
nxoe_p_stack_bpush(cw_nxoe_stack_t *a_stack);

void
nxoe_p_stack_pop(cw_nxoe_stack_t *a_stack);

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

    retval = stack->count;

    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_push(cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;
    cw_nxoe_stacko_t *stacko;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif

    /* Get an unused stacko. */
    if (qr_prev(ql_first(&stack->stack), link) != &stack->under)
    {
	/* Use spare. */
	stacko = qr_prev(ql_first(&stack->stack), link);
	nxo_no_new(&stacko->nxo);
	stack->nspare--;
    }
    else
    {
	/* A spare needs to be created.  Do this in a separate function to keep
	 * this one small, since this code path is rarely executed. */
	stacko = nxoe_p_stack_push(stack);
    }

    ql_first(&stack->stack) = stacko;
    stack->count++;
    retval = &stacko->nxo;

#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif

    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_bpush(cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;
    cw_nxoe_stacko_t *stacko;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif

    /* Get an unused stacko. */
    if (qr_prev(ql_first(&stack->stack), link) != &stack->under)
    {
	/* Use spare. */
	stacko = qr_prev(ql_first(&stack->stack), link);
	qr_remove(stacko, link);
	stack->nspare--;
    }
    else
    {
	/* A spare needs to be created.  Do this in a separate function to keep
	 * this one small, since this code path is rarely executed. */
	stacko = nxoe_p_stack_bpush(stack);
    }

    nxo_no_new(&stacko->nxo);
    qr_before_insert(&stack->under, stacko, link);
    if (ql_first(&stack->stack) == &stack->under)
    {
	ql_first(&stack->stack) = stacko;
    }
    stack->count++;
    retval = &stacko->nxo;

#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif

    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_under_push(cw_nxo_t *a_nxo, cw_nxo_t *a_object)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;
    cw_nxoe_stacko_t *stacko;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif

    /* Get an unused stacko.  If there are no spares, create one first. */
    if (qr_prev(ql_first(&stack->stack), link) != &stack->under)
    {
	/* Use spare. */
	stacko = qr_prev(ql_first(&stack->stack), link);
	stack->nspare--;
    }
    else
    {
	/* A spare needs to be created.  Do this in a separate function to keep
	 * this one small, since this code path is rarely executed. */
	stacko = nxoe_p_stack_push(stack);
    }

    /* Push under. */
    if (a_object != NULL)
    {
	/* Push under a_object. */
	nxo_no_new(&stacko->nxo);
	qr_remove(stacko, link);
	qr_after_insert((cw_nxoe_stacko_t *) a_object, stacko, link);
    }
    else
    {
	/* Same as nxo_stack_push(). */
	nxo_no_new(&stacko->nxo);
	ql_first(&stack->stack) = stacko;
    }

    stack->count++;
    retval = &stacko->nxo;

#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif

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
    if (stack->count == 0)
    {
	retval = TRUE;
	goto RETURN;
    }

    if (stack->nspare < CW_LIBONYX_STACK_CACHE)
    {
#ifdef CW_DBG
	cw_nxoe_stacko_t *spare = ql_first(&stack->stack);
#endif
	ql_first(&stack->stack) = qr_next(ql_first(&stack->stack), link);
	stack->nspare++;
#ifdef CW_DBG
	memset(&spare->nxo, 0x5a, sizeof(cw_nxo_t));
#endif
    }
    else
    {
	/* A spare needs to be discarded.  Do this in a separate function to
	 * keep this one small, since this code path is rarely executed. */
	nxoe_p_stack_pop(stack);
    }

    stack->count--;

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
    cw_nxoe_stacko_t *stacko;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->count == 0)
    {
	retval = TRUE;
	goto RETURN;
    }

    stacko = qr_prev(&stack->under, link);

    if (stacko == ql_first(&stack->stack))
    {
	ql_first(&stack->stack) = &stack->under;
    }

    /* This is non-optimal for the case of popping the last object, but it
     * probably isn't worth the extra conditional logic to handle that case
     * specially. */
    qr_remove(stacko, link);
    if (stack->nspare < CW_LIBONYX_STACK_CACHE)
    {
	qr_meld(ql_first(&stack->stack), stacko, cw_nxoe_stacko_t, link);
	stack->nspare++;
#ifdef CW_DBG
	memset(&stacko->nxo, 0x5a, sizeof(cw_nxo_t));
#endif
    }
    else
    {
	nxa_free(stacko, sizeof(cw_nxoe_stacko_t));
    }

    stack->count--;

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

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_count > stack->count)
    {
	retval = TRUE;
	goto RETURN;
    }
    if (stack->nspare + a_count <= CW_LIBONYX_STACK_CACHE)
    {
	cw_nxoe_stacko_t *top;
	cw_uint32_t i;
#ifdef CW_DBG
	cw_nxoe_stacko_t *spare = ql_first(&stack->stack);
#endif

	/* Get a pointer to what will be the new stack top. */
	for (i = 0, top = ql_first(&stack->stack); i < a_count; i++)
	{
	    top = qr_next(top, link);
	}

	ql_first(&stack->stack) = top;
	stack->nspare += a_count;
#ifdef CW_DBG
	for (i = 0; i < a_count; i++)
	{
	    memset(&spare->nxo, 0x5a, sizeof(cw_nxo_t));
	    spare = qr_next(spare, link);
	}
#endif
    }
    else
    {
	/* Spares need to be discarded.  Do this in a separate function to keep
	 * this one small, since this code path is rarely executed. */
	nxoe_p_stack_npop(stack, a_count);
    }

    stack->count -= a_count;

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

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_count > stack->count)
    {
	retval = TRUE;
	goto RETURN;
    }
    /* Popping 0 objects is legal, but the algorithm below can't handle it, so
     * check for this case specially. */
    if (a_count == 0)
    {
	retval = FALSE;
	goto RETURN;
    }
    if (stack->nspare + a_count <= CW_LIBONYX_STACK_CACHE)
    {
	cw_nxoe_stacko_t *bottom;
	cw_uint32_t i;

	/* Get a pointer to what will be the topmost object removed. */
	for (i = 0, bottom = &stack->under; i < a_count; i++)
	{
	    bottom = qr_prev(bottom, link);
	}

	if (bottom != ql_first(&stack->stack))
	{
	    qr_split(&stack->under, bottom, cw_nxoe_stacko_t, link);
	    /* Reinsert the spares in the spares region. */
	    qr_meld(ql_first(&stack->stack), bottom, cw_nxoe_stacko_t, link);
	}
	else
	{
	    /* There's no need to split and meld. */
	    ql_first(&stack->stack) = &stack->under;
	}

	stack->nspare += a_count;
#ifdef CW_DBG
	for (i = 0; i < a_count; i++)
	{
	    memset(&bottom->nxo, 0x5a, sizeof(cw_nxo_t));
	    bottom = qr_next(bottom, link);
	}
#endif
    }
    else
    {
	/* Spares need to be discarded.  Do this in a separate function to keep
	 * this one small, since this code path is rarely executed. */
	nxoe_p_stack_nbpop(stack, a_count);
    }

    stack->count -= a_count;

    retval = FALSE;
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE void
nxo_stack_remove(cw_nxo_t *a_nxo, cw_nxo_t *a_object)
{
    cw_nxoe_stack_t *stack;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif

    if (ql_first(&stack->stack) == (cw_nxoe_stacko_t *) a_object)
    {
	ql_first(&stack->stack) = qr_next(ql_first(&stack->stack), link);
    }
    qr_remove((cw_nxoe_stacko_t *) a_object, link);

    /* This is non-optimal for the case of removing the top object, but it's
     * probabaly not worth the extra conditional logic to optimize that case
     * specially. */
    if (stack->nspare < CW_LIBONYX_STACK_CACHE)
    {
	qr_before_insert(ql_first(&stack->stack),
			 (cw_nxoe_stacko_t *) a_object, link);
	stack->nspare++;
#ifdef CW_DBG
	memset(a_object, 0x5a, sizeof(cw_nxo_t));
#endif
    }
    else
    {
	nxa_free(a_object, sizeof(cw_nxoe_stacko_t));
    }

    stack->count--;

#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
}

CW_INLINE cw_nxo_t *
nxo_stack_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;
    cw_nxoe_stacko_t *stacko;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->count == 0)
    {
	retval = NULL;
	goto RETURN;
    }

    stacko = ql_first(&stack->stack);

    retval = &stacko->nxo;
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
    cw_nxoe_stacko_t *stacko;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->count == 0)
    {
	retval = NULL;
	goto RETURN;
    }

    stacko = qr_prev(&stack->under, link);

    retval = &stacko->nxo;
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
    cw_nxoe_stacko_t *stacko;
    cw_uint32_t i;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_index >= stack->count)
    {
	retval = NULL;
	goto RETURN;
    }

    for (i = 0, stacko = ql_first(&stack->stack); i < a_index; i++)
    {
	stacko = qr_next(stacko, link);
    }

    retval = &stacko->nxo;
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
    cw_nxoe_stacko_t *stacko;
    cw_uint32_t i;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_index >= stack->count)
    {
	retval = NULL;
	goto RETURN;
    }

    for (i = 0, stacko = qr_prev(&stack->under, link); i < a_index; i++)
    {
	stacko = qr_prev(stacko, link);
    }

    retval = &stacko->nxo;
    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_down_get(const cw_nxo_t *a_nxo, cw_nxo_t *a_object)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;
    cw_nxoe_stacko_t *stacko;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_object != NULL)
    {
	if (stack->count <= 1)
	{
	    retval = NULL;
	    goto RETURN;
	}
	stacko = (cw_nxoe_stacko_t *) a_object;
	stacko = qr_next(stacko, link);
	if (stacko == &stack->under)
	{
	    retval = NULL;
	    goto RETURN;
	}
    }
    else
    {
	/* Same as nxo_stack_get(). */
	if (stack->count == 0)
	{
	    retval = NULL;
	    goto RETURN;
	}

	stacko = ql_first(&stack->stack);
    }

    retval = &stacko->nxo;
    cw_dassert(retval->magic == CW_NXO_MAGIC);
    RETURN:
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
    return retval;
}

CW_INLINE cw_nxo_t *
nxo_stack_up_get(const cw_nxo_t *a_nxo, cw_nxo_t *a_object)
{
    cw_nxo_t *retval;
    cw_nxoe_stack_t *stack;
    cw_nxoe_stacko_t *stacko;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_object != NULL)
    {
	if (stack->count <= 1)
	{
	    retval = NULL;
	    goto RETURN;
	}
	stacko = (cw_nxoe_stacko_t *) a_object;
	if (stacko == ql_first(&stack->stack))
	{
	    retval = NULL;
	    goto RETURN;
	}
	stacko = qr_prev(stacko, link);
    }
    else
    {
	/* Same as nxo_stack_bget(). */
	if (stack->count == 0)
	{
	    retval = NULL;
	    goto RETURN;
	}

	stacko = qr_prev(&stack->under, link);
    }

    retval = &stacko->nxo;
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
    cw_nxoe_stacko_t *top, *below;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

    /* Get a pointer to the new top of the stack.  Then continue on to find the
     * end of the roll region. */
#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (stack->count < 2)
    {
#ifdef CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	retval = TRUE;
	goto ERROR;
    }

    top = ql_first(&stack->stack);
    top = qr_next(top, link);
    below = qr_next(top, link);

    /* We now have:
     *
     * ql_first(&stack->stack) --> /----------\ \  \
     *                             |          | |  |
     *                             |          | |   \
     *                             |          | |   / 1
     *                             |          | |  |
     *                             |          | |  /
     *                             \----------/  \
     *                     top --> /----------\  / 2
     *                             |          | |
     *                             |          | |
     *                             |          | |
     *                             |          | |
     *                             |          | |
     *                             \----------/ /
     *                   below --> /----------\
     *                             |          |
     *                             |          |
     *                             |          |
     *                             |          |
     *                             |          |
     *                             \----------/
     *
     * Set stack->below so that if the GC runs during the following code, it can
     * get at the below region. */
#ifdef CW_THREADS
    stack->below = below;
    mb_write();
#endif
    qr_split(ql_first(&stack->stack), below, cw_nxoe_stacko_t, link);
    ql_first(&stack->stack) = top;
    mb_write();
    qr_meld(top, below, cw_nxoe_stacko_t, link);
#ifdef CW_THREADS
    mb_write();
    stack->below = NULL;
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
    cw_nxoe_stacko_t *top;
    cw_uint32_t i;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif

    cw_assert(stack->count > 0);

    /* Calculate the current index of the element that will end up on top of the
     * stack.  This allows us to save a pointer to it as we iterate down/up the
     * stack on the way to the new stack top.  This code also has the side
     * effect of 'mod'ing the rotate amount, so that we don't spend a bunch of
     * time rotating the stack if the user specifies a rotate amount larger than
     * the stack.  A decent program will never do this, so it's not worth
     * specifically optimizing, but it falls out of these calculations with no
     * extra work, since we already have to deal with upward versus downward
     * rotating calculations. */
    if (a_amount < 0)
    {
	/* Convert a_amount to a positive equivalent. */
	a_amount += ((a_amount - stack->count) / stack->count) * stack->count;
    }
    a_amount %= stack->count;
    a_amount += stack->count;
    a_amount %= stack->count;

    /* Do this check after the above calculations, just in case the rotate
     * amount is an even multiple of the stack size. */
    if (a_amount == 0)
    {
	/* Noop. */
	return;
    }

    /* Get a pointer to the new top of the stack. */
    /* Start from whichever end is closest to what will be the new top of
     * stack. */
    if ((cw_uint32_t) a_amount < stack->count / 2)
    {
	/* Iterate down from the top. */
	for (i = 0, top = ql_first(&stack->stack);
	     i < (cw_uint32_t) a_amount;
	     i++)
	{
	    top = qr_next(top, link);
	}
    }
    else
    {
	/* Iterate up from the bottom. */
	for (i = 1, top = qr_prev(&stack->under, link);
	     i < (cw_uint32_t) (stack->count - a_amount);
	     i++)
	{
	    top = qr_prev(top, link);
	}
    }

    /* We now have:
     *
     * ql_first(&stack->stack) --> /----------\ \  \
     *                             |          | |  |
     *                             |          | |   \
     *                             |          | |   / a_amount
     *                             |          | |  |
     *                             |          | |  /
     *                             \----------/  \
     *                     top --> /----------\  / stack->count
     *                             |          | |  \
     *                             |          | |  |
     *                             |          | |   \
     *                             |          | |   / stack->count - a_amount
     *                             |          | |  |
     *                             \----------/ /  /
     */
    qr_split(ql_first(&stack->stack), &stack->under, cw_nxoe_stacko_t, link);
    ql_first(&stack->stack) = top;
    mb_write();
    qr_meld(top, &stack->under, cw_nxoe_stacko_t, link);
#ifdef CW_THREADS
    nxoe_p_stack_unlock(stack);
#endif
}

CW_INLINE cw_bool_t
nxo_stack_roll(cw_nxo_t *a_nxo, cw_uint32_t a_count, cw_sint32_t a_amount)
{
    cw_bool_t retval;
    cw_nxoe_stack_t *stack;
    cw_nxoe_stacko_t *top, *below;
    cw_uint32_t i;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    stack = (cw_nxoe_stack_t *) a_nxo->o.nxoe;
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

    cw_assert(a_count > 0);

    /* Calculate the current index of the element that will end up on top of the
     * stack.  This allows us to save a pointer to it as we iterate down the
     * stack on the way to the bottom of the roll region.  This code also has
     * the side effect of 'mod'ing the roll amount, so that we don't spend a
     * bunch of time rolling the stack if the user specifies a roll amount
     * larger than the roll region.  A decent program will never do this, so
     * it's not worth specifically optimizing, but it falls out of these
     * calculations with no extra work, since we already have to deal with
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

    /* Get a pointer to the new top of the stack.  Then continue on to find the
     * end of the roll region. */
#ifdef CW_THREADS
    nxoe_p_stack_lock(stack);
#endif
    if (a_count > stack->count)
    {
#ifdef CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	retval = TRUE;
	goto ERROR;
    }
    for (i = 0, top = ql_first(&stack->stack); i < (cw_uint32_t) a_amount; i++)
    {
	top = qr_next(top, link);
    }
    below = top;

    for (i = 0; i < a_count - a_amount; i++)
    {
	below = qr_next(below, link);
    }

    /* We now have:
     *
     * ql_first(&stack->stack) --> /----------\ \  \
     *                             |          | |  |
     *                             |          | |   \
     *                             |          | |   / a_amount
     *                             |          | |  |
     *                             |          | |  /
     *                             \----------/  \
     *                     top --> /----------\  / a_count
     *                             |          | |
     *                             |          | |
     *                             |          | |
     *                             |          | |
     *                             |          | |
     *                             \----------/ /
     *                   below --> /----------\
     *                             |          |
     *                             |          |
     *                             |          |
     *                             |          |
     *                             |          |
     *                             \----------/
     *
     * Set stack->below so that if the GC runs during the following code, it can
     * get at the below region. */
#ifdef CW_THREADS
    stack->below = below;
    mb_write();
#endif
    qr_split(ql_first(&stack->stack), below, cw_nxoe_stacko_t, link);
    ql_first(&stack->stack) = top;
    mb_write();
    qr_meld(top, below, cw_nxoe_stacko_t, link);
#ifdef CW_THREADS
    mb_write();
    stack->below = NULL;
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

#define NXO_STACK_DOWN_GET(r_nxo, a_nxo, a_thread, a_object)		\
    do									\
    {									\
	(r_nxo) = nxo_stack_down_get((a_nxo), (a_object));		\
	if ((r_nxo) == NULL)						\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)

#define NXO_STACK_UP_GET(r_nxo, a_nxo, a_thread, a_object)		\
    do									\
    {									\
	(r_nxo) = nxo_stack_up_get((a_nxo), (a_object));		\
	if ((r_nxo) == NULL)						\
	{								\
	    nxo_thread_nerror((a_thread), NXN_stackunderflow);		\
	    return;							\
	}								\
    } while (0)
