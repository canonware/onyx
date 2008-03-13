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

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_stack_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_stack_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_STACK_C_))
CW_INLINE cw_bool_t
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

CW_INLINE cw_nxoe_t *
nxoe_l_stack_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_stack_t *stack;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so as long as two
     * interpreters aren't collecting simultaneously, using a static variable
     * works fine. */
    static cw_uint32_t ref_stage;

    stack = (cw_nxoe_stack_t *) a_nxoe;

    if (a_reset)
    {
#ifdef CW_THREADS
	if (stack->below != NULL)
	{
	    /* We're in the middle of a roll operation, so need to handle the
	     * below region specially.  It's entirely possible that we'll end
	     * up reporting some/all stack elements twice, but that doesn't
	     * cause a correctness problem, whereas not reporting them at all
	     * does. */
	    ref_stage = 0;
	}
	else
#endif
	{
	    ref_stage = 2;
	}
    }

    retval = NULL;
    switch (ref_stage)
    {
#ifdef CW_THREADS
	case 0:
	{
	    /* Set up for stage 1. */
	    stack->ref_stacko = stack->below;
	    ref_stage++;
	    /* Fall through. */
	}
	case 1:
	{
	    /* below region stack iteration. */
	    for (; retval == NULL && stack->ref_stacko != &stack->under;
		 stack->ref_stacko = qr_next(stack->ref_stacko, link))
	    {
		retval = nxo_nxoe_get(&stack->ref_stacko->nxo);
	    }

	    if (retval != NULL)
	    {
		break;
	    }
	    ref_stage++;
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

	    ref_stage++;
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
	    ref_stage++;
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
	    ref_stage++;
	    /* Fall through. */
	}
	default:
	{
	    retval = NULL;
	}
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_STACK_C_)) */
