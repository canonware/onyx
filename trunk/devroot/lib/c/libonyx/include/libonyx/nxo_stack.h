/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

/*
 * The following types and structures are private, but need to be exposed for
 * the inline functions.
 */
typedef struct cw_nxoe_stack_s cw_nxoe_stack_t;
typedef struct cw_nxoe_stacko_s cw_nxoe_stacko_t;

struct cw_nxoe_stacko_s {
	cw_nxo_t		nxo;	/* Payload.  Must be first field. */
	ql_elm(cw_nxoe_stacko_t) link;	/* Stack/spares linkage. */
};

struct cw_nxoe_stack_s {
	cw_nxoe_t		nxoe;
#ifdef _CW_THREADS
	cw_mtx_t		lock;	/* Access locked if locking bit set. */
#endif
	cw_nxa_t		*nxa;
	ql_head(cw_nxoe_stacko_t) stack; /* Stack. */
	cw_uint32_t		count;	/* Number of stack elements. */
	cw_uint32_t		nspare;	/* Number of spare elements. */
	cw_nxoe_stacko_t	under;	/* Not used, just under stack bottom. */

	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t		ref_stage;
	cw_nxoe_stacko_t	*ref_stacko;
#ifdef _CW_THREADS
	cw_nxoe_stacko_t	*noroll;
#endif
};

void	nxo_stack_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking);
void	nxo_stack_copy(cw_nxo_t *a_to, cw_nxo_t *a_from);

#ifndef _CW_USE_INLINES
cw_uint32_t nxo_stack_count(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_stack_push(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_stack_under_push(cw_nxo_t *a_nxo, cw_nxo_t *a_object);
cw_bool_t nxo_stack_pop(cw_nxo_t *a_nxo);
cw_bool_t nxo_stack_npop(cw_nxo_t *a_nxo, cw_uint32_t a_count);
cw_nxo_t *nxo_stack_get(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_stack_nget(cw_nxo_t *a_nxo, cw_uint32_t a_index);
cw_nxo_t *nxo_stack_down_get(cw_nxo_t *a_nxo, cw_nxo_t *a_object);
cw_bool_t nxo_stack_exch(cw_nxo_t *a_nxo);
cw_bool_t nxo_stack_roll(cw_nxo_t *a_nxo, cw_uint32_t a_count, cw_sint32_t
    a_amount);
#endif

/* Private, but the inline functions need these prototypes. */
cw_nxoe_stacko_t *nxoe_p_stack_push(cw_nxoe_stack_t *a_stack);
void	nxoe_p_stack_pop(cw_nxoe_stack_t *a_stack);
void	nxoe_p_stack_npop(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count);


#if (defined(_CW_USE_INLINES) || defined(_NXO_STACK_C_))
#ifdef _CW_THREADS
/* Private, but defined here for the inline functions. */
_CW_INLINE void
nxoe_p_stack_lock(cw_nxoe_stack_t *a_nxoe)
{
	if (a_nxoe->nxoe.locking)
		mtx_lock(&a_nxoe->lock);
}

/* Private, but defined here for the inline functions. */
_CW_INLINE void
nxoe_p_stack_unlock(cw_nxoe_stack_t *a_nxoe)
{
	if (a_nxoe->nxoe.locking)
		mtx_unlock(&a_nxoe->lock);
}
#endif

_CW_INLINE cw_uint32_t
nxo_stack_count(cw_nxo_t *a_nxo)
{
	cw_uint32_t		retval;
	cw_nxoe_stack_t	*stack;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

	retval = stack->count;

	return retval;
}

_CW_INLINE cw_nxo_t *
nxo_stack_push(cw_nxo_t *a_nxo)
{
	cw_nxo_t		*retval;
	cw_nxoe_stack_t		*stack;
	cw_nxoe_stacko_t	*stacko;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif

	/* Get an unused stacko. */
	if (qr_prev(ql_first(&stack->stack), link) != &stack->under) {
		/* Use spare. */
		stacko = qr_prev(ql_first(&stack->stack), link);
		stack->nspare--;
	} else {
		/*
		 * A spare needs to be created.  Do this in a separate function
		 * to keep this one small, since this code path is rarely
		 * executed.
		 */
		stacko = nxoe_p_stack_push(stack);
	}

	nxo_no_new(&stacko->nxo);
	ql_first(&stack->stack) = stacko;
	stack->count++;
	retval = &stacko->nxo;

#ifdef _CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif

	return retval;
}

_CW_INLINE cw_nxo_t *
nxo_stack_under_push(cw_nxo_t *a_nxo, cw_nxo_t *a_object)
{
	cw_nxo_t		*retval;
	cw_nxoe_stack_t		*stack;
	cw_nxoe_stacko_t	*stacko;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif

	/* Get an unused stacko.  If there are no spare, create one first. */
	if (qr_prev(ql_first(&stack->stack), link) != &stack->under) {
		/* Use spare. */
		stacko = qr_prev(ql_first(&stack->stack), link);
		stack->nspare--;
	} else {
		/*
		 * A spare needs to be created.  Do this in a separate function
		 * to keep this one small, since this code path is rarely
		 * executed.
		 */
		stacko = nxoe_p_stack_push(stack);
	}

	/* Push under. */
	if (a_object != NULL) {
		/* Push under a_object. */
		nxo_no_new(&stacko->nxo);
		qr_remove(stacko, link);
		qr_after_insert((cw_nxoe_stacko_t *)a_object, stacko, link);
	} else {
		/* Same as nxo_stack_push(). */
		nxo_no_new(&stacko->nxo);
		ql_first(&stack->stack) = stacko;
	}

	stack->count++;
	retval = &stacko->nxo;

#ifdef _CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif

	return retval;
}

_CW_INLINE cw_bool_t
nxo_stack_pop(cw_nxo_t *a_nxo)
{
	cw_bool_t		retval;
	cw_nxoe_stack_t		*stack;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif
	if (stack->count == 0) {
		retval = TRUE;
		goto RETURN;
	}

	if (stack->nspare < _CW_LIBONYX_STACK_CACHE) {
		ql_first(&stack->stack) = qr_next(ql_first(&stack->stack),
		    link);
		stack->nspare++;
	} else {
		/*
		 * A spare needs to be discarded.  Do this in a separate
		 * function to keep this one small, since this code path is
		 * rarely executed.
		 */
		nxoe_p_stack_pop(stack);
	}

	stack->count--;

	retval = FALSE;
	RETURN:
#ifdef _CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	return retval;
}

_CW_INLINE cw_bool_t
nxo_stack_npop(cw_nxo_t *a_nxo, cw_uint32_t a_count)
{
	cw_bool_t		retval;
	cw_nxoe_stack_t		*stack;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif
	if (a_count > stack->count) {
		retval = TRUE;
		goto RETURN;
	}
	/* Get a pointer to what will be the new stack top. */
	if (stack->nspare + a_count <= _CW_LIBONYX_STACK_CACHE) {
		cw_nxoe_stacko_t	*top;
		cw_uint32_t		i;

		for (i = 0, top = ql_first(&stack->stack); i < a_count; i++)
			top = qr_next(top, link);

		ql_first(&stack->stack) = top;
		stack->nspare += a_count;
	} else {
		/*
		 * Spares need to be discarded.  Do this in a separate function
		 * to keep this one small, since this code path is rarely
		 * executed.
		 */
		nxoe_p_stack_npop(stack, a_count);
	}

	stack->count -= a_count;

	retval = FALSE;
	RETURN:
#ifdef _CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	return retval;
}

_CW_INLINE cw_nxo_t *
nxo_stack_get(cw_nxo_t *a_nxo)
{
	cw_nxo_t		*retval;
	cw_nxoe_stack_t		*stack;
	cw_nxoe_stacko_t	*stacko;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif
	if (stack->count == 0) {
		retval = NULL;
		goto RETURN;
	}

	stacko = ql_first(&stack->stack);

	retval = &stacko->nxo;
	RETURN:
#ifdef _CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	return retval;
}

_CW_INLINE cw_nxo_t *
nxo_stack_nget(cw_nxo_t *a_nxo, cw_uint32_t a_index)
{
	cw_nxo_t		*retval;
	cw_nxoe_stack_t		*stack;
	cw_nxoe_stacko_t	*stacko;
	cw_uint32_t		i;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif
	if (a_index >= stack->count) {
		retval = NULL;
		goto RETURN;
	}

	for (i = 0, stacko = ql_first(&stack->stack); i < a_index; i++)
		stacko = qr_next(stacko, link);

	retval = &stacko->nxo;
	RETURN:
#ifdef _CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	return retval;
}

_CW_INLINE cw_nxo_t *
nxo_stack_down_get(cw_nxo_t *a_nxo, cw_nxo_t *a_object)
{
	cw_nxo_t		*retval;
	cw_nxoe_stack_t		*stack;
	cw_nxoe_stacko_t	*stacko;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif
	if (a_object != NULL) {
		if (stack->count <= 1) {
			retval = NULL;
			goto RETURN;
		}
		stacko = (cw_nxoe_stacko_t *)a_object;
		stacko = qr_next(stacko, link);
		if (stacko == &stack->under) {
			retval = NULL;
			goto RETURN;
		}
	} else {
		/* Same as nxo_stack_get(). */
		if (stack->count == 0) {
			retval = NULL;
			goto RETURN;
		}

		stacko = ql_first(&stack->stack);
	}

	retval = &stacko->nxo;
	RETURN:
#ifdef _CW_THREADS
	nxoe_p_stack_unlock(stack);
#endif
	return retval;
}

_CW_INLINE cw_bool_t
nxo_stack_exch(cw_nxo_t *a_nxo)
{
	cw_bool_t		retval;
	cw_nxoe_stack_t		*stack;
	cw_nxoe_stacko_t	*top, *noroll;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

	/*
	 * Get a pointer to the new top of the stack.  Then continue on to find
	 * the end of the roll region.
	 */
#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif
	if (stack->count < 2) {
#ifdef _CW_THREADS
		nxoe_p_stack_unlock(stack);
#endif
		retval = TRUE;
		goto ERROR;
	}

	top = ql_first(&stack->stack);
	top = qr_next(top, link);
	noroll = qr_next(top, link);

	/*
	 * We now have:
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
	 *                  noroll --> /----------\
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             \----------/
	 *
	 * Set stack->noroll so that if the GC runs during the following code,
	 * it can get at the noroll region.
	 */
#ifdef _CW_THREADS
	stack->noroll = noroll;
#endif
	qr_split(ql_first(&stack->stack), noroll, link);
	ql_first(&stack->stack) = top;
	qr_meld(top, noroll, link);
#ifdef _CW_THREADS
	stack->noroll = NULL;
	nxoe_p_stack_unlock(stack);
#endif

	retval = FALSE;
	ERROR:
	return retval;
}

_CW_INLINE cw_bool_t
nxo_stack_roll(cw_nxo_t *a_nxo, cw_uint32_t a_count, cw_sint32_t a_amount)
{
	cw_bool_t		retval;
	cw_nxoe_stack_t		*stack;
	cw_nxoe_stacko_t	*top, *noroll;
	cw_uint32_t		i;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_dassert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

	_cw_assert(a_count > 0);

	/*
	 * Calculate the current index of the element that will end up on top of
	 * the stack.  This allows us to save a pointer to it as we iterate down
	 * the stack on the way to the bottom of the roll region.  This code
	 * also has the side effect of 'mod'ing the roll amount, so that we
	 * don't spend a bunch of time rolling the stack if the user specifies a
	 * roll amount larger than the roll region.  A decent program will never
	 * do this, so it's not worth specifically optimizing, but it falls out
	 * of these calculations with no extra work, since we already have to
	 * deal with upward versus downward rolling calculations.
	 */
	if (a_amount < 0) {
		/* Convert a_amount to a positive equivalent. */
		a_amount += ((a_amount - a_count) / a_count) * a_count;
	}
	a_amount %= a_count;
	a_amount += a_count;
	a_amount %= a_count;

	/*
	 * Do this check after the above calculations, just in case the roll
	 * amount is an even multiple of the roll region.
	 */
	if (a_amount == 0) {
		/* Noop. */
		goto RETURN;
	}

	/*
	 * Get a pointer to the new top of the stack.  Then continue on to find
	 * the end of the roll region.
	 */
#ifdef _CW_THREADS
	nxoe_p_stack_lock(stack);
#endif
	if (a_count > stack->count) {
#ifdef _CW_THREADS
		nxoe_p_stack_unlock(stack);
#endif
		retval = TRUE;
		goto ERROR;
	}
	for (i = 0, top = ql_first(&stack->stack); i < a_amount; i++)
		top = qr_next(top, link);
	noroll = top;

	for (i = 0; i < a_count - a_amount; i++)
		noroll = qr_next(noroll, link);

	/*
	 * We now have:
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
	 *                  noroll --> /----------\
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             \----------/
	 *
	 * Set stack->noroll so that if the GC runs during the following code,
	 * it can get at the noroll region.
	 */
#ifdef _CW_THREADS
	stack->noroll = noroll;
#endif
	qr_split(ql_first(&stack->stack), noroll, link);
	ql_first(&stack->stack) = top;
	qr_meld(top, noroll, link);
#ifdef _CW_THREADS
	stack->noroll = NULL;
	nxoe_p_stack_unlock(stack);
#endif

	RETURN:
	retval = FALSE;
	ERROR:
	return retval;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXO_STACK_C_)) */

/*
 * Convenience wrapper macros for use where errors should cause an error and
 * immediate return.
 */
#define	NXO_STACK_POP(a_nxo, a_thread) do {				\
	if (nxo_stack_pop(a_nxo)) {					\
		nxo_thread_error((a_thread),				\
		    NXO_THREADE_STACKUNDERFLOW);			\
		return;							\
	}								\
} while (0)

#define	NXO_STACK_NPOP(a_nxo, a_thread, a_count) do {			\
	if (nxo_stack_npop((a_nxo), (a_count))) {			\
		nxo_thread_error((a_thread),				\
		    NXO_THREADE_STACKUNDERFLOW);			\
		return;							\
	}								\
} while (0)

#define	NXO_STACK_GET(r_nxo, a_nxo, a_thread) do {			\
	(r_nxo) = nxo_stack_get(a_nxo);					\
	if ((r_nxo) == NULL) {						\
		nxo_thread_error((a_thread),				\
		    NXO_THREADE_STACKUNDERFLOW);			\
		return;							\
	}								\
} while (0)

#define	NXO_STACK_NGET(r_nxo, a_nxo, a_thread, a_index) do {		\
	(r_nxo) = nxo_stack_nget((a_nxo), (a_index));			\
	if ((r_nxo) == NULL) {						\
		nxo_thread_error((a_thread),				\
		    NXO_THREADE_STACKUNDERFLOW);			\
		return;							\
	}								\
} while (0)

#define	NXO_STACK_DOWN_GET(r_nxo, a_nxo, a_thread, a_object) do {	\
	(r_nxo) = nxo_stack_down_get((a_nxo), (a_object));		\
	if ((r_nxo) == NULL) {						\
		nxo_thread_error((a_thread),				\
		    NXO_THREADE_STACKUNDERFLOW);			\
		return;							\
	}								\
} while (0)
