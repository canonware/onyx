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
bool
nxoe_l_stack_delete(cw_nxoe_t *a_nxoe, uint32_t a_iter);

cw_nxoe_t *
nxoe_l_stack_ref_iter(cw_nxoe_t *a_nxoe, bool a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_STACK_C_))
CW_INLINE bool
nxoe_l_stack_delete(cw_nxoe_t *a_nxoe, uint32_t a_iter)
{
    cw_nxoe_stack_t *stack;
    uint32_t i, count;

    stack = (cw_nxoe_stack_t *) a_nxoe;

    cw_check_ptr(stack);
    cw_dassert(stack->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(stack->nxoe.type == NXOT_STACK);

    /* Deallocate spares. */
    for (i = 0; i < stack->nspare; i++)
    {
	nxa_free(stack->spare[i], sizeof(cw_nxo_t));
    }

    /* Deallocate nxo's. */
    for (i = 0, count = stack->aend - stack->abeg; i < count; i++)
    {
	nxa_free(stack->a[stack->abase + stack->abeg + i], sizeof(cw_nxo_t));
    }

#ifdef CW_THREADS
    if (stack->nxoe.locking)
    {
	mtx_delete(&stack->lock);
    }
#endif

    /* Deallocate internal array. */
    nxa_free(stack->a, stack->ahlen * 2 * sizeof(cw_nxo_t *));

    /* Deallocate the container structure. */
    nxa_free(stack, sizeof(cw_nxoe_stack_t));

    return false;
}

CW_INLINE cw_nxoe_t *
nxoe_l_stack_ref_iter(cw_nxoe_t *a_nxoe, bool a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_stack_t *stack;
    uint32_t count;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static uint32_t iter;

    stack = (cw_nxoe_stack_t *) a_nxoe;

    if (a_reset)
    {
	iter = 0;
    }

    retval = NULL;
#ifdef CW_THREADS
    switch (stack->rstate)
    {
	case RSTATE_NONE:
	{
#endif
	    for (count = stack->aend - stack->abeg;
		 retval == NULL && iter < count;
		 iter++)
	    {
		retval = nxo_nxoe_get(stack->a[stack->abase + stack->abeg
					       + iter]);
	    }
#ifdef CW_THREADS
	    break;
	}
	case RSTATE_RMASK:
	{
	    for (count = stack->aend - stack->abeg;
		 retval == NULL && iter < count;
		 iter++)
	    {
		if (stack->abeg + iter >= stack->rbeg
		    && stack->abeg + iter < stack->rend)
		{
		    /* Masked. */
		    retval = nxo_nxoe_get(stack->r[stack->rbase + stack->rbeg
						   + iter]);
		}
		else
		{
		    retval = nxo_nxoe_get(stack->a[stack->abase + stack->abeg
						   + iter]);
		}
	    }
	    break;
	}
	case RSTATE_RONLY:
	{
	    for (count = stack->rend - stack->rbeg;
		 retval == NULL && iter < count;
		 iter++)
	    {
		retval = nxo_nxoe_get(stack->r[stack->rbase + stack->rbeg
					       + iter]);
	    }
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
#endif

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_STACK_C_)) */
