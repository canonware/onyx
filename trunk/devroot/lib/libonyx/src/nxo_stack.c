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
 * nxo_stack_roll() re-orders stack elements, and over time, the elements
 * become jumbled enough that it is possible that additional cache misses
 * result.  However, since only a relatively small number of spare elements is
 * kept, the cache effects of jumbling should be negligible under normal
 * conditions.
 *
 ******************************************************************************/

/* Compile non-inlined functions if not using inlines. */
#define	_NXO_STACK_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_stack_l.h"

void
nxo_stack_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking)
{
	cw_nxoe_stack_t	*stack;

	stack = (cw_nxoe_stack_t *)_cw_malloc(sizeof(cw_nxoe_stack_t));

	nxoe_l_new(&stack->nxoe, NXOT_STACK, a_locking);
	if (a_locking)
		mtx_new(&stack->lock);

	stack->nx = a_nx;
	ql_new(&stack->stack);
	ql_new(&stack->chunks);

	stack->count = 0;
	stack->nspare = 0;

	ql_elm_new(&stack->under, link);
	ql_head_insert(&stack->stack, &stack->under, link);

	stack->noroll = NULL;

	nxo_no_new(a_nxo);
	a_nxo->o.nxoe = (cw_nxoe_t *)stack;
#ifdef _LIBONYX_DBG
	a_nxo->magic = _CW_NXO_MAGIC;
#endif
	nxo_p_type_set(a_nxo, NXOT_STACK);

	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)stack);
}

void
nxoe_l_stack_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx)
{
	cw_nxoe_stack_t		*stack;
	cw_nxoe_stackc_t	*stackc;

	stack = (cw_nxoe_stack_t *)a_nxoe;

	_cw_check_ptr(stack);
	_cw_assert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

	/*
	 * Pop objects off the stack.  Then delete all the stackc's.
	 */
	if (stack->count > 0) {
		cw_nxo_t	nxo;

		/* Fake up a nxo. */
		nxo_p_new(&nxo, NXOT_STACK);
		nxo.o.nxoe = (cw_nxoe_t *)stack;
		
		nxo_stack_npop(&nxo, stack->count);
	}

	while (ql_first(&stack->chunks) != NULL) {
		stackc = ql_first(&stack->chunks);
		ql_remove(&stack->chunks, stackc, link);
		nxa_l_stackc_put(nx_nxa_get(stack->nx), stackc);
	}

	if (stack->nxoe.locking)
		mtx_delete(&stack->lock);

	_CW_NXOE_FREE(stack);
}

cw_nxoe_t *
nxoe_l_stack_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	cw_nxoe_t	*retval;
	cw_nxoe_stack_t	*stack;

	stack = (cw_nxoe_stack_t *)a_nxoe;

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
			retval = nxo_nxoe_get(&stack->ref_stacko->nxo);

		if (retval != NULL)
			break;
		stack->ref_stage++;
		/* Fall through. */
	case 2:
		/* First roll region iteration. */
		stack->ref_stacko = ql_first(&stack->stack);
		if (stack->ref_stacko != &stack->under)
			retval = nxo_nxoe_get(&stack->ref_stacko->nxo);

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
			retval = nxo_nxoe_get(&stack->ref_stacko->nxo);

		if (retval != NULL)
			break;
		stack->ref_stage++;
		/* Fall through. */
	default:
		retval = NULL;
	}

	return retval;
}

void
nxo_l_stack_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *stack, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(stack, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(stack)
	    != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nxo_thread_stdout_get(a_thread);

	if (nxo_integer_get(depth) > 0) {
		cw_uint32_t		i;
		cw_nxo_t		*nxo;

		error = nxo_file_output(stdout_nxo, "(");
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}

		for (i = nxo_stack_count(stack); i > 0; i--) {
			nxo = nxo_stack_push(ostack);
			nxo_dup(nxo, nxo_stack_nget(stack, i - 1));
			nxo = nxo_stack_push(ostack);
			nxo_integer_new(nxo, nxo_integer_get(depth) - 1);
			_cw_onyx_code(a_thread,
			    "1 index type sprintdict exch get eval");

			if (i > 1) {
				error = nxo_file_output(stdout_nxo, " ");
				if (error) {
					nxo_thread_error(a_thread, error);
					return;
				}
			}
		}

		error = nxo_file_output(stdout_nxo, ")");
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}
	} else {
		error = nxo_file_output(stdout_nxo, "-stack-");
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}
	}

	nxo_stack_npop(ostack, 2);
}

void
nxo_stack_copy(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
	cw_nxo_t	*nxo_to, *nxo_fr;
	cw_uint32_t	i, count;

	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_NXO_MAGIC);
	  
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_NXO_MAGIC);

	for (i = 0, count = nxo_stack_count(a_from), nxo_fr = NULL, nxo_to
	    = NULL; i < count; i++) {
		nxo_fr = nxo_stack_down_get(a_from, nxo_fr);
		nxo_to = nxo_stack_under_push(a_to, nxo_to);
		nxo_dup(nxo_to, nxo_fr);
	}
}

cw_uint32_t
nxo_stack_count(cw_nxo_t *a_nxo)
{
	cw_uint32_t		retval;
	cw_nxoe_stack_t	*stack;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_assert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

	retval = stack->count;

	return retval;
}

cw_uint32_t
nxo_stack_index_get(cw_nxo_t *a_nxo, cw_nxo_t *a_object)
{
	cw_uint32_t		i;
	cw_nxoe_stack_t	*stack;
	cw_nxoe_stacko_t	*stacko;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	stack = (cw_nxoe_stack_t *)a_nxo->o.nxoe;
	_cw_assert(stack->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(stack->nxoe.type == NXOT_STACK);

	for (i = 0, stacko = ql_first(&stack->stack); (stacko !=
	    (cw_nxoe_stacko_t *)a_object) && (i < stack->count); stacko =
	     qr_next(stacko, link), i++)
		;
	_cw_assert(i < stack->count);

	return i;
}

void
nxoe_p_stack_spares_create(cw_nxoe_stack_t *a_stack)
{
	cw_nxoe_stackc_t	*stackc;
	cw_uint32_t		i;

	/*
	 * create a new stackc, add it to the stackc ql, and add its stacko's to
	 * the stack.
	 */
	stackc = nxa_l_stackc_get(nx_nxa_get(a_stack->nx));

	ql_elm_new(stackc, link);

	stackc->nused = 0;

	qr_new(&stackc->objects[0], link);
	stackc->objects[0].stackc = stackc;
	for (i = 1; i < _LIBONYX_STACKC_COUNT; i++) {
		qr_new(&stackc->objects[i], link);
		qr_after_insert(&stackc->objects[i - 1], &stackc->objects[i],
		    link);
		stackc->objects[i].stackc = stackc;
	}

	ql_tail_insert(&a_stack->chunks, stackc, link);
	qr_meld(ql_first(&a_stack->stack), &stackc->objects[0], link);

	a_stack->nspare += _LIBONYX_STACKC_COUNT;
}

void
nxoe_p_stack_spares_destroy(cw_nxoe_stack_t *a_stack, cw_nxoe_stackc_t
    *a_stackc)
{
	cw_uint32_t	i;

	/*
	 * Iterate through the objects and remove them from the stack-wide
	 * object ring.
	 */
	for (i = 0; i < _LIBONYX_STACKC_COUNT; i++)
		qr_remove(&a_stackc->objects[i], link);

	/* Remove the stackc from the stack's list of stackc's. */
	ql_remove(&a_stack->chunks, a_stackc, link);

	/* Deallocate. */
	nxa_l_stackc_put(nx_nxa_get(a_stack->nx), a_stackc);

	a_stack->nspare -= _LIBONYX_STACKC_COUNT;
}
