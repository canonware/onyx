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

typedef struct cw_nxoe_class_s cw_nxoe_class_t;

struct cw_nxoe_class_s
{
    cw_nxoe_t nxoe;

    /* Superclass, or null. */
    cw_nxo_t super;

    /* Dictionary of methods, or null. */
    cw_nxo_t methods;

    /* Dictionary of class data, or null. */
    cw_nxo_t data;

    void *opaque;
    cw_nxo_class_ref_iter_t *ref_iter_f;
    cw_nxo_class_delete_t *delete_f;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_class_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_class_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_CLASS_C_))
CW_INLINE cw_bool_t
nxoe_l_class_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    cw_nxoe_class_t *class_;

    class_ = (cw_nxoe_class_t *) a_nxoe;

    cw_check_ptr(class_);
    cw_dassert(class_->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(class_->nxoe.type == NXOT_CLASS);

    if (class_->delete_f != NULL)
    {
	retval = class_->delete_f(class_->opaque, a_iter);
    }
    else
    {
	retval = FALSE;
    }

    if (retval == FALSE)
    {
	nxa_free(class_, sizeof(cw_nxoe_class_t));
    }

    return retval;
}

CW_INLINE cw_nxoe_t *
nxoe_l_class_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_class_t *class_;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static cw_uint32_t ref_stage;

    class_ = (cw_nxoe_class_t *) a_nxoe;

    if (a_reset)
    {
	ref_stage = 0;
    }

    switch (ref_stage)
    {
	case 0:
	{
	    ref_stage++;
	    retval = nxo_nxoe_get(&class_->super);
	    if (retval != NULL)
	    {
		break;
	    }
	}
	case 1:
	{
	    ref_stage++;
	    retval = nxo_nxoe_get(&class_->methods);
	    if (retval != NULL)
	    {
		break;
	    }
	}
	case 2:
	{
	    ref_stage++;
	    retval = nxo_nxoe_get(&class_->data);
	    if (retval != NULL)
	    {
		break;
	    }
	}
	case 3:
	{
	    ref_stage++;
	    if (class_->ref_iter_f != NULL)
	    {
		retval = class_->ref_iter_f(class_->opaque, TRUE);
	    }
	    else
	    {
		retval = NULL;
	    }
	    break;
	}
	case 4:
	{
	    retval = class_->ref_iter_f(class_->opaque, FALSE);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_CLASS_C_)) */
