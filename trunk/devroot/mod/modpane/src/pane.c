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

#include "../include/modpane.h"

/* This is used to assure that methods are actually passed the correct class.
 * It is not reported to the GC, so can not safely be used for any other
 * purpose. */
static cw_nxo_t s_pane;

struct cw_pane
{
    /* Pane. */
    cw_pn_t pn;

    /* Protects all pn operations. */
    cw_mtx_t mtx;

    /* Reference to =pane=, prevents premature module unload. */
    cw_nxo_t handle;

    /* Auxiliary data for pane_aux_[gs]et. */
    cw_nxo_t aux;
};

static const struct cw_modpane_method modpane_pane_methods[] = {
    MODPANE_METHOD(pane, pane),
    MODPANE_METHOD(pane, size),
    MODPANE_METHOD(pane, display)
};

static cw_nxn_t
pane_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	   struct cw_pane **r_pane);
#ifdef XXX_NOT_YET
static cw_nxoe_t *
pane_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
pane_p_delete(void *a_data, cw_uint32_t a_iter);
#endif

void
modpane_pane_init(cw_nxo_t *a_thread)
{
    nxo_no_new(&s_pane);
    modpane_class_init(a_thread, "pane", modpane_pane_methods,
		       (sizeof(modpane_pane_methods)
			/ sizeof(struct cw_modpane_method)),
		       NULL, &s_pane);
}

CW_P_INLINE void
pane_p_lock(struct cw_pane *a_pane)
{
    mtx_lock(&a_pane->mtx);
}

CW_P_INLINE void
pane_p_unlock(struct cw_pane *a_pane)
{
    mtx_unlock(&a_pane->mtx);
}

static cw_nxn_t
pane_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	      struct cw_pane **r_pane)
{
    cw_nxn_t retval;
    cw_nxo_t *tstack, *data, *name, *handle;

    tstack = nxo_thread_tstack_get(a_thread);
    name = nxo_stack_push(tstack);
    handle = nxo_stack_push(tstack);

    if (nxo_type_get(a_instance) != NXOT_INSTANCE)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    if ((retval = modpane_instance_kind(a_instance, &s_pane)))
    {
	goto RETURN;
    }

    data = nxo_instance_data_get(a_instance);
    if (nxo_type_get(data) != NXOT_DICT)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    nxo_name_new(name, "pane", sizeof("pane") - 1, FALSE);
    if (nxo_dict_lookup(data, name, handle))
    {
	retval = NXN_undefined;
	goto RETURN;
    }

    if (nxo_type_get(handle) != NXOT_HANDLE)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    *r_pane = (struct cw_pane *) nxo_handle_opaque_get(handle);
    retval = NXN_ZERO;
    RETURN:
    /* Clean up. */
    nxo_stack_npop(tstack, 2);

    return retval;
}

#ifdef XXX_NOT_YET
static cw_nxoe_t *
pane_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
//    struct cw_pane *pane = (struct cw_pane *) a_data;
    static cw_uint32_t iter;

    if (a_reset)
    {
	iter = 0;
    }

    for (retval = NULL; retval == NULL; iter++)
    {
	switch (iter)
	{
	    case 0:
	    {
//		retval = nxo_nxoe_get(&pane->);
//		break;
	    }
	    default:
	    {
		retval = NULL;
		goto RETURN;
	    }
	}
    }

    RETURN:
    return retval;
}

static cw_bool_t
pane_p_delete(void *a_data, cw_uint32_t a_iter)
{
    struct cw_pane *pane = (struct cw_pane *) a_data;

    mtx_delete(&pane->mtx);
    pn_delete(&pane->pn);
    nxa_free(pane, sizeof(struct cw_pane));

    return FALSE;
}
#endif

cw_nxn_t
modpane_pane_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread)
{
    struct cw_pane *pane;

    return pane_p_get(a_instance, a_thread, &pane);
}

/* XXX #instance :pane #instance */
void
modpane_pane_pane(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

/* #pane :size #x #y */
void
modpane_pane_size(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_pane *pane;
    cw_uint32_t x, y;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = pane_p_get(nxo, a_thread, &pane)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    pane_p_lock(pane);
    pn_size(&pane->pn, &x, &y);
    pane_p_unlock(pane);

    nxo_integer_new(nxo, x);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, y);
}

/* #pane :display #display */
void
modpane_pane_display(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}
