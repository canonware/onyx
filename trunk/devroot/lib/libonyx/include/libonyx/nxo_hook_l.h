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

typedef struct cw_nxoe_hook_s cw_nxoe_hook_t;

struct cw_nxoe_hook_s
{
    cw_nxoe_t nxoe;

    cw_nxo_t tag;
    void *data;
    cw_nxo_hook_eval_t *eval_f;
    cw_nxo_hook_ref_iter_t *ref_iter_f;
    cw_nxo_hook_delete_t *delete_f;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_hook_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_hook_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_HOOK_C_))
CW_INLINE cw_bool_t
nxoe_l_hook_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    cw_nxoe_hook_t *hook;

    hook = (cw_nxoe_hook_t *) a_nxoe;

    cw_check_ptr(hook);
    cw_dassert(hook->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(hook->nxoe.type == NXOT_HOOK);

    if (hook->delete_f != NULL)
    {
	retval = hook->delete_f(hook->data, a_iter);
    }
    else
    {
	retval = FALSE;
    }

    if (retval == FALSE)
    {
	nxa_free(hook, sizeof(cw_nxoe_hook_t));
    }

    return retval;
}

CW_INLINE cw_nxoe_t *
nxoe_l_hook_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_hook_t *hook;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static cw_uint32_t ref_stage;

    hook = (cw_nxoe_hook_t *) a_nxoe;

    if (a_reset)
    {
	ref_stage = 0;
    }

    switch (ref_stage)
    {
	case 0:
	{
	    ref_stage++;
	    retval = nxo_nxoe_get(&hook->tag);
	    if (retval != NULL)
	    {
		break;
	    }
	}
	case 1:
	{
	    ref_stage++;
	    if (hook->ref_iter_f != NULL)
	    {
		retval = hook->ref_iter_f(hook->data, TRUE);
	    }
	    else
	    {
		retval = NULL;
	    }
	    break;
	}
	case 2:
	{
	    retval = hook->ref_iter_f(hook->data, FALSE);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_HOOK_C_)) */
