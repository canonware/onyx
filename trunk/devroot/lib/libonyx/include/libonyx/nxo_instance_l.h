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

typedef struct cw_nxoe_instance_s cw_nxoe_instance_t;

struct cw_nxoe_instance_s
{
    cw_nxoe_t nxoe;

    /* Class that this is an instance of, or null. */
    cw_nxo_t isa;

    /* Instance data, or null. */
    cw_nxo_t data;

    void *opaque;
    cw_nxo_instance_ref_iter_t *ref_iter_f;
    cw_nxo_instance_delete_t *delete_f;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_instance_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_instance_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_INSTANCE_C_))
CW_INLINE cw_bool_t
nxoe_l_instance_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    cw_nxoe_instance_t *instance;

    instance = (cw_nxoe_instance_t *) a_nxoe;

    cw_check_ptr(instance);
    cw_dassert(instance->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(instance->nxoe.type == NXOT_INSTANCE);

    if (instance->delete_f != NULL)
    {
	retval = instance->delete_f(instance->opaque, a_iter);
    }
    else
    {
	retval = FALSE;
    }

    if (retval == FALSE)
    {
	nxa_free(instance, sizeof(cw_nxoe_instance_t));
    }

    return retval;
}

CW_INLINE cw_nxoe_t *
nxoe_l_instance_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_instance_t *instance;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static cw_uint32_t ref_stage;

    instance = (cw_nxoe_instance_t *) a_nxoe;

    if (a_reset)
    {
	ref_stage = 0;
    }

    switch (ref_stage)
    {
	case 0:
	{
	    ref_stage++;
	    retval = nxo_nxoe_get(&instance->isa);
	    if (retval != NULL)
	    {
		break;
	    }
	    break;
	}
	case 1:
	{
	    ref_stage++;
	    retval = nxo_nxoe_get(&instance->data);
	    if (retval != NULL)
	    {
		break;
	    }
	}
	case 2:
	{
	    ref_stage++;
	    if (instance->ref_iter_f != NULL)
	    {
		retval = instance->ref_iter_f(instance->opaque, TRUE);
	    }
	    else
	    {
		retval = NULL;
	    }
	    break;
	}
	case 3:
	{
	    retval = instance->ref_iter_f(instance->opaque, FALSE);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_INSTANCE_C_)) */
