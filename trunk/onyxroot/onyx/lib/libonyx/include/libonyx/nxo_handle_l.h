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

typedef struct cw_nxoe_handle_s cw_nxoe_handle_t;

struct cw_nxoe_handle_s
{
    cw_nxoe_t nxoe;

    cw_nxo_t tag;
    void *opaque;
    cw_nxo_handle_eval_t *eval_f;
    cw_nxo_handle_ref_iter_t *ref_iter_f;
    cw_nxo_handle_delete_t *delete_f;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_handle_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_handle_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_HANDLE_C_))
CW_INLINE cw_bool_t
nxoe_l_handle_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    cw_nxoe_handle_t *handle;

    handle = (cw_nxoe_handle_t *) a_nxoe;

    cw_check_ptr(handle);
    cw_dassert(handle->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(handle->nxoe.type == NXOT_HANDLE);

    if (handle->delete_f != NULL)
    {
	retval = handle->delete_f(handle->opaque, a_iter);
    }
    else
    {
	retval = FALSE;
    }

    if (retval == FALSE)
    {
	nxa_free(handle, sizeof(cw_nxoe_handle_t));
    }

    return retval;
}

CW_INLINE cw_nxoe_t *
nxoe_l_handle_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_handle_t *handle;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static cw_uint32_t ref_stage;

    handle = (cw_nxoe_handle_t *) a_nxoe;

    if (a_reset)
    {
	ref_stage = 0;
    }

    switch (ref_stage)
    {
	case 0:
	{
	    ref_stage++;
	    retval = nxo_nxoe_get(&handle->tag);
	    if (retval != NULL)
	    {
		break;
	    }
	}
	case 1:
	{
	    ref_stage++;
	    if (handle->ref_iter_f != NULL)
	    {
		retval = handle->ref_iter_f(handle->opaque, TRUE);
	    }
	    else
	    {
		retval = NULL;
	    }
	    break;
	}
	case 2:
	{
	    retval = handle->ref_iter_f(handle->opaque, FALSE);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_HANDLE_C_)) */
