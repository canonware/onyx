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
 * of elements are cached to avoid allocation/deallocation overhead in the
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
nxo_stack_new(cw_nxo_t *a_nxo, cw_bool_t a_locking)
{
    cw_nxoe_stack_t *stack;

    stack = (cw_nxoe_stack_t *) nxa_malloc(sizeof(cw_nxoe_stack_t));

    nxoe_l_new(&stack->nxoe, NXOT_STACK, a_locking);
#ifdef CW_THREADS
    if (a_locking)
    {
	mtx_new(&stack->lock);
    }
#endif

    ql_new(&stack->stack);

    stack->count = 0;
    stack->nspare = 0;

    ql_elm_new(&stack->under, link);
    ql_head_insert(&stack->stack, &stack->under, link);

#ifdef CW_THREADS
    stack->below = NULL;
#endif

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) stack;
    nxo_p_type_set(a_nxo, NXOT_STACK);

    nxa_l_gc_register((cw_nxoe_t *) stack);
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
    retval = (cw_nxoe_stacko_t *) nxa_malloc(sizeof(cw_nxoe_stacko_t));
    qr_new(retval, link);
    nxo_no_new(&retval->nxo);
    qr_after_insert(&a_stack->under, retval, link);

    return retval;
}

/* This function handles a special case for nxo_stack_bpush(), but is done as a
 * separate function to keep nxo_stack_bpush() small. */
cw_nxoe_stacko_t *
nxoe_p_stack_bpush(cw_nxoe_stack_t *a_stack)
{
    cw_nxoe_stacko_t *retval;

    /* No spares.  Allocate and insert one. */
    retval = (cw_nxoe_stacko_t *) nxa_malloc(sizeof(cw_nxoe_stacko_t));
    qr_new(retval, link);

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
    nxa_free(stacko, sizeof(cw_nxoe_stacko_t));
}

/* This function handles a special case for nxo_stack_npop(), but is done as a
 * separate function to keep nxo_stack_npop() small. */
void
nxoe_p_stack_npop(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count)
{
    cw_uint32_t i;
    cw_nxoe_stacko_t *top, *stacko, *tstacko;
#ifdef CW_DBG
    cw_nxoe_stacko_t *spare = ql_first(&a_stack->stack);
    cw_uint32_t nspares;
#endif

    /* We need to discard some spares, so get a pointer to the beginning of the
     * region to be removed from the ring. */
    for (i = 0, stacko = ql_first(&a_stack->stack);
	 i < CW_LIBONYX_STACK_CACHE - a_stack->nspare;
	 i++)
    {
	stacko = qr_next(stacko, link);
    }
#ifdef CW_DBG
    nspares = i;
#endif
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
    qr_split(stacko, top, cw_nxoe_stacko_t, link);

    for (i = 0; i < a_stack->nspare + a_count - CW_LIBONYX_STACK_CACHE; i++)
    {
	tstacko = qr_next(stacko, link);
	qr_remove(tstacko, link);
	nxa_free(tstacko, sizeof(cw_nxoe_stacko_t));
    }

    a_stack->nspare = CW_LIBONYX_STACK_CACHE;

#ifdef CW_DBG
    for (i = 0; i < nspares; i++, spare = qr_next(spare, link))
    {
	memset(&spare->nxo, 0x5a, sizeof(cw_nxo_t));
    }
#endif
}

/* This function handles a special case for nxo_stack_nbpop(), but is done as a
 * separate function to keep nxo_stack_nbpop() small. */
void
nxoe_p_stack_nbpop(cw_nxoe_stack_t *a_stack, cw_uint32_t a_count)
{
    cw_uint32_t i;
    cw_nxoe_stacko_t *bottom, *stacko, *tstacko;
#ifdef CW_DBG
    cw_uint32_t nspares;
#endif

    /* We need to discard some spares, so get a pointer to the beginning of the
     * region to be removed from the ring. */
    for (i = 0, stacko = &a_stack->under;
	 i < CW_LIBONYX_STACK_CACHE - a_stack->nspare;
	 i++)
    {
	stacko = qr_prev(stacko, link);
    }
#ifdef CW_DBG
    nspares = i;
#endif
    for (bottom = stacko; i < a_count; i++)
    {
	bottom = qr_prev(bottom, link);
    }

    /* We now have:
     *
     * ql_first(&a_stack->stack) --> /----------\
     *                               |          |
     *                               |          |
     *                               |          |
     *                               |          |
     *                               |          |
     *                               \----------/
     *                    bottom --> /----------\ \
     *                               |          | |
     *                               |          | |
     *                               |          | |
     *                               |          | |
     *                               |          | |
     *                               \----------/  \ a_count
     *                    stacko --> /----------\  / \
     *                               |          | |  |
     *                               |          | |  |
     *                               |          | |   \ max cache - nspare
     *                               |          | |   /
     *                               |          | |  |
     *                               \----------/ /  /
     *               stack.under --> /----------\
     *                               |          |
     *                               :          :
     *                               :          :
     *
     * 1) Split bottom/under.
     * 2) Split bottom/stacko.
     * 3) Meld first/stacko.
     * 4) Deallocate the ring pointed to by bottom. */

    qr_split(bottom, &a_stack->under, cw_nxoe_stacko_t, link);
    qr_split(bottom, stacko, cw_nxoe_stacko_t, link);
    qr_meld(ql_first(&a_stack->stack), stacko, cw_nxoe_stacko_t, link);
    
    for (i = 0; i < a_stack->nspare + a_count - CW_LIBONYX_STACK_CACHE; i++)
    {
	tstacko = qr_next(bottom, link);
	qr_remove(tstacko, link);
	nxa_free(tstacko, sizeof(cw_nxoe_stacko_t));
    }

    a_stack->nspare = CW_LIBONYX_STACK_CACHE;

#ifdef CW_DBG
    for (i = 0; i < nspares; i++, stacko = qr_next(stacko, link))
    {
	memset(&stacko->nxo, 0x5a, sizeof(cw_nxo_t));
    }
#endif
}
