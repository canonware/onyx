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
 * Stack object space is allocated on a per-element basis, and a certain number
 * of elements are cached to avoid allocation deallocation overhead in the
 * common case.  Doing chunked allocation of stack elements would be slightly
 * more memory efficient (and probably more cache friendly) in the common case,
 * but would require extra code complexity in the critical paths of pushing and
 * popping.
 *
 * By keeping the re-allocation algorithm simple, we are able to make common
 * stack operations very fast.
 *
 * Using a ring makes it relatively efficient (if not simple for the reference
 * iterator) to make all the stack operations GC-safe.
 *
 ******************************************************************************/

/* Compile non-inlined functions if not using inlines. */
#define CW_NXO_STACK_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_stack_l.h"

void
nxo_stack_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking)
{
    cw_nxoe_stack_t *stack;

    stack = (cw_nxoe_stack_t *) nxa_malloc(nx_nxa_get(a_nx),
					   sizeof(cw_nxoe_stack_t));

    nxoe_l_new(&stack->nxoe, NXOT_STACK, a_locking);
#ifdef CW_THREADS
    if (a_locking)
    {
	mtx_new(&stack->lock);
    }
#endif

    stack->nxa = nx_nxa_get(a_nx);
    ql_new(&stack->stack);

    stack->count = 0;
    stack->nspare = 0;

    ql_elm_new(&stack->under, link);
    ql_head_insert(&stack->stack, &stack->under, link);

#ifdef CW_THREADS
    stack->noroll = NULL;
#endif

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) stack;
    nxo_p_type_set(a_nxo, NXOT_STACK);

    nxa_l_gc_register(stack->nxa, (cw_nxoe_t *) stack);
}

cw_bool_t
nxoe_l_stack_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_stack_t *stack;
    cw_nxoe_stacko_t *stacko, *tstacko;
    cw_uint32_t i;

    stack = (cw_nxoe_stack_t *) a_nxoe;

    cw_check_ptr(stack);
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

    /* Deallocate stacko's. */
    stacko = qr_next(&stack->under, link);
    qr_remove(&stack->under, link);
    for (i = 0; i < stack->count + stack->nspare; i++)
    {
	tstacko = qr_next(stacko, link);
	qr_remove(tstacko, link);
	nxa_free(stack->nxa, tstacko, sizeof(cw_nxoe_stacko_t));
    }

#ifdef CW_THREADS
    if (stack->nxoe.locking)
    {
	mtx_delete(&stack->lock);
    }
#endif

    nxa_free(a_nxa, stack, sizeof(cw_nxoe_stack_t));

    return FALSE;
}

cw_nxoe_t *
nxoe_l_stack_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_stack_t *stack;

    stack = (cw_nxoe_stack_t *) a_nxoe;

    if (a_reset)
    {
#ifdef CW_THREADS
	if (stack->noroll != NULL)
	{
	    /* We're in the middle of a roll operation, so need to handle the
	     * noroll region specially.  It's entirely possible that we'll end
	     * up reporting some/all stack elements twice, but that doesn't
	     * cause a correctness problem, whereas not reporting them at all
	     * does. */
	    stack->ref_stage = 0;
	}
	else
#endif
	{
	    stack->ref_stage = 2;
	}
    }

    retval = NULL;
    switch (stack->ref_stage)
    {
#ifdef CW_THREADS
	case 0:
	{
	    /* Set up for stage 1. */
	    stack->ref_stacko = stack->noroll;
	    stack->ref_stage++;
	    /* Fall through. */
	}
	case 1:
	{
	    /* noroll region stack iteration. */
	    for (; retval == NULL && stack->ref_stacko != &stack->under;
		 stack->ref_stacko = qr_next(stack->ref_stacko, link))
	    {
		retval = nxo_nxoe_get(&stack->ref_stacko->nxo);
	    }

	    if (retval != NULL)
	    {
		break;
	    }
	    stack->ref_stage++;
	    /* Fall through. */
	}
#endif
	case 2:
	{   
	    /* First roll region iteration. */
	    stack->ref_stacko = ql_first(&stack->stack);
	    if (stack->ref_stacko != &stack->under)
	    {
		retval = nxo_nxoe_get(&stack->ref_stacko->nxo);
	    }

	    stack->ref_stage++;
	    if (retval != NULL)
	    {
		break;
	    }
	    /* Fall through. */
	}
	case 3:
	{
	    /* Set up for stage 4. */
	    if (stack->ref_stacko != &stack->under)
	    {
		stack->ref_stacko = qr_next(stack->ref_stacko, link);
	    }
	    stack->ref_stage++;
	    /* Fall through. */
	}
	case 4:
	{
	    /* Main roll region iteration. */
	    for (; retval == NULL && stack->ref_stacko != &stack->under
		     && stack->ref_stacko != ql_first(&stack->stack);
		 stack->ref_stacko = qr_next(stack->ref_stacko, link))
	    {
		retval = nxo_nxoe_get(&stack->ref_stacko->nxo);
	    }

	    if (retval != NULL)
	    {
		break;
	    }
	    stack->ref_stage++;
	    /* Fall through. */
	}
	default:
	{
	    retval = NULL;
	}
    }

    return retval;
}

void
nxo_stack_copy(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
    cw_nxo_t *nxo_to, *nxo_fr;
    cw_uint32_t i, count;

    cw_check_ptr(a_to);
    cw_dassert(a_to->magic == CW_NXO_MAGIC);
	  
    cw_check_ptr(a_from);
    cw_dassert(a_from->magic == CW_NXO_MAGIC);

    for (i = 0, count = nxo_stack_count(a_from), nxo_fr = NULL, nxo_to = NULL;
	 i < count;
	 i++)
    {
	nxo_fr = nxo_stack_down_get(a_from, nxo_fr);
	nxo_to = nxo_stack_under_push(a_to, nxo_to);
	nxo_dup(nxo_to, nxo_fr);
    }
}

/* This function handles a special case for nxo_stack_push(), but is done as a
 * separate function to keep nxo_stack_push() small. */
cw_nxoe_stacko_t *
nxoe_p_stack_push(cw_nxoe_stack_t *a_stack)
{
    cw_nxoe_stacko_t *retval;

    /* No spares.  Allocate and insert one. */
    retval = (cw_nxoe_stacko_t *) nxa_malloc(a_stack->nxa,
					     sizeof(cw_nxoe_stacko_t));
    qr_new(retval, link);
    qr_after_insert(&a_stack->under, retval, link);

    return retval;
}

/* This function handles a special case for nxo_stack_pop(), but is done as a
 * separate function to keep nxo_stack_pop() small. */
void
nxoe_p_stack_pop(cw_nxoe_stack_t *a_stack)
{
    cw_nxoe_stacko_t *stacko;

    cw_assert(a_stack->nspare == CW_LIBONYX_STACK_CACHE);

    /* Throw the popped element away. */
    stacko = ql_first(&a_stack->stack);
    ql_first(&a_stack->stack) = qr_next(ql_first(&a_stack->stack), link);
    qr_remove(stacko, link);
    nxa_free(a_stack->nxa, stacko, sizeof(cw_nxoe_stacko_t));
}

/* This function handles a special case for nxo_stack_npop(), but is done as a
 * separate function to keep nxo_stack_npop() small. */
void
nxoe_p_stack_npop(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count)
{
    cw_uint32_t i;
    cw_nxoe_stacko_t *top, *stacko, *tstacko;

    /* We need to discard some spares, so get a pointer to the beginning of the
     * region to be removed from the ring. */
    for (i = 0, stacko = ql_first(&a_stack->stack);
	 i < CW_LIBONYX_STACK_CACHE - a_stack->nspare;
	 i++)
    {
	stacko = qr_next(stacko, link);
    }
    for (top = stacko; i < a_count; i++)
    {
	top = qr_next(top, link);
    }

    /* We now have:
     *
     * ql_first(&a_stack->stack) --> /----------\ \
     *                               |          | |
     *                               |          | |
     *                               |          | |
     *                               |          | |
     *                               |          | |
     *                               \----------/  \ a_count
     *                    stacko --> /----------\  / \
     *                               |          | |  |
     *                               |          | |   \ nspare
     *                               |          | |   / + a_count
     *                               |          | |  |  - max cache
     *                               |          | |  /
     *                               \----------/ /
     *                       top --> /----------\
     *                               |          |
     *                               |          |
     *                               |          |
     *                               |          |
     *                               |          |
     *                               \----------/
     *
     * Remove the region from stacko (inclusive) down to top
     * (exclusive), then deallocate those stacko's. */
    ql_first(&a_stack->stack) = top;
    qr_split(stacko, top, link);

    for (i = 0; i < a_stack->nspare + a_count - CW_LIBONYX_STACK_CACHE; i++)
    {
	tstacko = qr_next(stacko, link);
	qr_remove(tstacko, link);
	nxa_free(a_stack->nxa, tstacko, sizeof(cw_nxoe_stacko_t));
    }

    a_stack->nspare = CW_LIBONYX_STACK_CACHE;
}
