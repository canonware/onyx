/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Stack object space is allocated in chunks (implemented by the stackc class)
 * in order to improve locality and reduce memory fragmentation.  Freed objects
 * within a chunk are kept in the same ring as the actual stack and re-used in
 * LIFO order.  This has the potential to cause adjacent stack objects to be
 * scattered throughout the stackc's, but typical stack operations have the same
 * effect anyway, so little care is taken to keep stack object re-allocation
 * contiguous, or even local.
 *
 * By keeping the re-allocation algorithm simple, we are able to make common
 * stack operations very fast.
 *
 * Using a ring makes it relatively simple to make all the stack operations
 * GC-safe.  One disadvantage of using rings, however, is that
 * stilo_stack_roll() re-orders stack elements, and over time, the elements
 * become jumbled enough that it is possible that additional cache misses
 * result.  However, since only a relatively small number of spare elements is
 * kept, the cache effects of jumbling should be negligible under normal
 * conditions.
 *
 ******************************************************************************/

/* Compile non-inlined functions if not using inlines. */
#define	_STILO_STACK_C_

#include "../include/libstil/libstil.h"
#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_stack_l.h"

void
stilo_stack_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking)
{
	cw_stiloe_stack_t	*stack;

	stack = (cw_stiloe_stack_t *)_cw_malloc(sizeof(cw_stiloe_stack_t));

	stiloe_l_new(&stack->stiloe, STILOT_STACK, a_locking);
	if (a_locking)
		mtx_new(&stack->lock);

	stack->stil = a_stil;
	ql_new(&stack->stack);
	ql_new(&stack->chunks);

	stack->count = 0;
	stack->nspare = 0;

	ql_elm_new(&stack->under, link);
	ql_head_insert(&stack->stack, &stack->under, link);

	stack->noroll = NULL;

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)stack;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_STACK;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)stack);
}

void
stiloe_l_stack_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_stack_t	*stack;
	cw_stiloe_stackc_t	*stackc;

	stack = (cw_stiloe_stack_t *)a_stiloe;

	_cw_check_ptr(stack);
	_cw_assert(stack->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(stack->stiloe.type == STILOT_STACK);

	/*
	 * Pop objects off the stack.  Then delete all the stackc's.
	 */
	if (stack->count > 0) {
		cw_stilo_t	stilo;

		/* Fake up a stilo. */
		stilo_l_new(&stilo, STILOT_STACK);
		stilo.o.stiloe = (cw_stiloe_t *)stack;
		
		stilo_stack_npop(&stilo, stack->count);
	}

	while (ql_first(&stack->chunks) != NULL) {
		stackc = ql_first(&stack->chunks);
		ql_remove(&stack->chunks, stackc, link);
		stila_stackc_put(stil_stila_get(stack->stil), stackc);
	}

	if (stack->stiloe.locking)
		mtx_delete(&stack->lock);

	_CW_STILOE_FREE(stack);
}

cw_stiloe_t *
stiloe_l_stack_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_stack_t	*stack;

	stack = (cw_stiloe_stack_t *)a_stiloe;

	if (a_reset) {
		if (stack->noroll != NULL) {
			/*
			 * We're in the middle of a roll operation, so need to
			 * handle the noroll region specially.  It's entirely
			 * possible that we'll end up reporting some/all stack
			 * elements twice, but that doesn't cause a correctness
			 * problem, whereas not reporting them at all does.
			 */
			stack->ref_stage = 0;
		} else
			stack->ref_stage = 2;
	}

	retval = NULL;
	switch (stack->ref_stage) {
	case 0:
		/* Set up for stage 1. */
		stack->ref_stacko = stack->noroll;
		stack->ref_stage++;
		/* Fall through. */
	case 1:
		/* noroll region stack iteration. */
		for (; retval == NULL && stack->ref_stacko != &stack->under;
		    stack->ref_stacko = qr_next(stack->ref_stacko, link))
			retval = stilo_stiloe_get(&stack->ref_stacko->stilo);

		if (retval != NULL)
			break;
		stack->ref_stage++;
		/* Fall through. */
	case 2:
		/* First roll region iteration. */
		stack->ref_stacko = ql_first(&stack->stack);
		if (stack->ref_stacko != &stack->under)
			retval = stilo_stiloe_get(&stack->ref_stacko->stilo);

		stack->ref_stage++;
		if (retval != NULL)
			break;
		/* Fall through. */
	case 3:
		/* Set up for stage 4. */
		if (stack->ref_stacko != &stack->under) {
			stack->ref_stacko = qr_next(stack->ref_stacko,
			    link);
		}
		stack->ref_stage++;
		/* Fall through. */
	case 4:
		/* Main roll region iteration. */
		for (; retval == NULL && stack->ref_stacko != &stack->under
		    && stack->ref_stacko != ql_first(&stack->stack);
		    stack->ref_stacko = qr_next(stack->ref_stacko, link))
			retval = stilo_stiloe_get(&stack->ref_stacko->stilo);

		if (retval != NULL)
			break;
		stack->ref_stage++;
		/* Fall through. */
	default:
		retval = NULL;
	}

	return retval;
}

cw_stilo_threade_t
stilo_l_stack_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilo_threade_t	retval;

	if (a_depth > 0) {
		retval = stilo_file_output(a_file, "<[[");
		if (retval)
			goto RETURN;

		retval = stilo_file_output(a_file, "XXX");
		if (retval)
			goto RETURN;

		retval = stilo_file_output(a_file, "]>");
		if (retval)
			goto RETURN;
	} else {
		retval = stilo_file_output(a_file, "-stack-");
		if (retval)
			goto RETURN;
	}

	retval = STILO_THREADE_NONE;
	RETURN:
	return retval;
}

cw_uint32_t
stilo_stack_count(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_stack_t	*stack;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	stack = (cw_stiloe_stack_t *)a_stilo->o.stiloe;
	_cw_assert(stack->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(stack->stiloe.type == STILOT_STACK);

	retval = stack->count;

	return retval;
}

cw_uint32_t
stilo_stack_index_get(cw_stilo_t *a_stilo, cw_stilo_t *a_object)
{
	cw_uint32_t		i;
	cw_stiloe_stack_t	*stack;
	cw_stiloe_stacko_t	*stacko;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	stack = (cw_stiloe_stack_t *)a_stilo->o.stiloe;
	_cw_assert(stack->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(stack->stiloe.type == STILOT_STACK);

	for (i = 0, stacko = ql_first(&stack->stack); (stacko !=
	    (cw_stiloe_stacko_t *)a_object) && (i < stack->count); stacko =
	     qr_next(stacko, link), i++)
		;
	_cw_assert(i < stack->count);

	return i;
}

void
stiloe_p_stack_spares_create(cw_stiloe_stack_t *a_stack)
{
	cw_stiloe_stackc_t	*stackc;
	cw_uint32_t		i;

	/*
	 * create a new stackc, add it to the stackc ql, and add its stacko's to
	 * the stack.
	 */
	stackc = stila_stackc_get(stil_stila_get(a_stack->stil));

	ql_elm_new(stackc, link);

	stackc->nused = 0;

	qr_new(&stackc->objects[0], link);
	stackc->objects[0].stackc = stackc;
	for (i = 1; i < _LIBSTIL_STACKC_COUNT; i++) {
		qr_new(&stackc->objects[i], link);
		qr_after_insert(&stackc->objects[i - 1],
		    &stackc->objects[i], link);
		stackc->objects[i].stackc = stackc;
	}

	ql_tail_insert(&a_stack->chunks, stackc, link);
	qr_meld(ql_first(&a_stack->stack), &stackc->objects[0], link);

	a_stack->nspare += _LIBSTIL_STACKC_COUNT;
}

void
stiloe_p_stack_spares_destroy(cw_stiloe_stack_t *a_stack, cw_stiloe_stackc_t
    *a_stackc)
{
	cw_uint32_t	i;

	/*
	 * Iterate through the objects and remove them from the stack-wide
	 * object ring.
	 */
	for (i = 0; i < _LIBSTIL_STACKC_COUNT; i++)
		qr_remove(&a_stackc->objects[i], link);

	/* Remove the stackc from the stack's list of stackc's. */
	ql_remove(&a_stack->chunks, a_stackc, link);

	/* Deallocate. */
	stila_stackc_put(stil_stila_get(a_stack->stil), a_stackc);

	a_stack->nspare -= _LIBSTIL_STACKC_COUNT;
}
