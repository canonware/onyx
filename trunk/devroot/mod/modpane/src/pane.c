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

struct cw_pane
{
    /* For GC iteration. */
    cw_uint32_t iter;

    /* Pane. */
    cw_pn_t pn;

    /* Protects all pn operations. */
    cw_mtx_t mtx;

    /* Reference to =pane=, prevents premature module unload. */
    cw_nxo_t handle;

    /* Auxiliary data for pane_aux_[gs]et. */
    cw_nxo_t aux;
};

static const struct cw_modpane_entry modpane_pane_handles[] = {
    /* pane. */
    MODPANE_ENTRY(pane),
    {"pane?", modpane_pane_p},
    MODPANE_ENTRY(pane_aux_get),
    MODPANE_ENTRY(pane_aux_set),
    MODPANE_ENTRY(pane_size),
    MODPANE_ENTRY(pane_display)
};

static cw_nxoe_t *
pane_p_ref_iter(void *a_data, cw_bool_t a_reset);

static cw_bool_t
pane_p_delete(void *a_data, cw_uint32_t a_iter);

void
modpane_pane_init(cw_nxo_t *a_thread)
{
    modpane_handles_init(a_thread, modpane_pane_handles,
			 (sizeof(modpane_pane_handles)
			  / sizeof(struct cw_modpane_entry)));
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

static cw_nxoe_t *
pane_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_pane *pane = (struct cw_pane *) a_data;

    if (a_reset)
    {
	pane->iter = 0;
    }

    switch (pane->iter)
    {
	case 0:
	{
	    retval = nxo_nxoe_get(&pane->handle);
	    cw_check_ptr(retval);
	    break;
	}
	case 1:
	{
	    retval = nxo_nxoe_get(&pane->aux);
	    break;
	}
	default:
	{
	    retval = NULL;
	}
    }
    pane->iter++;

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

/*
 * Verify that a_nxo is a =pane=.
 */
cw_nxn_t
pane_type(cw_nxo_t *a_nxo)
{
    cw_nxn_t retval;
    cw_nxo_t *tag;
    cw_uint32_t name_len;
    const cw_uint8_t *name;

    if (nxo_type_get(a_nxo) != NXOT_HANDLE)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    tag = nxo_handle_tag_get(a_nxo);
    if (nxo_type_get(tag) != NXOT_NAME)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    name_len = nxo_name_len_get(tag);
    name = nxo_name_str_get(tag);
    if ((name_len != strlen("pane")) || strncmp("pane", name, name_len))
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
modpane_pane(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *nxo, *tag;
    struct cw_pane *pane;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);

    pane = (struct cw_pane *) nxa_malloc(sizeof(struct cw_pane));

    /* Create a reference to this handle in order to prevent the module from
     * being prematurely unloaded. */
    nxo_no_new(&pane->handle);
    nxo_dup(&pane->handle, nxo_stack_get(estack));

    /* Initialize the ds. */
//    pn_new(&pane->pn, XXX);

    /* Initialize the protection mutex; ds's aren't thread-safe. */
    mtx_new(&pane->mtx);

    /* Create a reference to the pane. */
    nxo = nxo_stack_push(ostack);
    nxo_handle_new(nxo, pane, NULL, pane_p_ref_iter, pane_p_delete);

    /* Set the handle tag. */
    tag = nxo_handle_tag_get(nxo);
    nxo_name_new(tag, "pane", sizeof("pane") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Initialize aux. */
    nxo_null_new(&pane->aux);
}

/* %object pane? %boolean */
void
modpane_pane_p(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = pane_type(nxo);

    nxo_boolean_new(nxo, error ? FALSE : TRUE);
}

void
modpane_pane_aux_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    cw_nxn_t error;
    struct cw_pane *pane;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = pane_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    pane = (struct cw_pane *) nxo_handle_data_get(nxo);

    /* Avoid a GC race by using tnxo to store a reachable ref to the pane. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_dup(nxo, &pane->aux);
    nxo_stack_pop(tstack);
}

void
modpane_pane_aux_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *aux;
    cw_nxn_t error;
    struct cw_pane *pane;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(aux, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, aux);
    error = pane_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    pane = (struct cw_pane *) nxo_handle_data_get(nxo);

    nxo_dup(&pane->aux, aux);
    nxo_stack_npop(ostack, 2);
}

void
modpane_pane_size(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_pane *pane;
    cw_uint32_t x, y;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = pane_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    pane = (struct cw_pane *) nxo_handle_data_get(nxo);

    pane_p_lock(pane);
    pn_size(&pane->pn, &x, &y);
    pane_p_unlock(pane);

    nxo_integer_new(nxo, x);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, y);
}

void
modpane_pane_display(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}
