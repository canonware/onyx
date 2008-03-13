/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/modpane.h"

struct cw_pane
{
    /* For GC iteration. */
    cw_uint32_t iter;

    /* Pane. */
    cw_pn_t pn;

    /* Reference to =pane=, prevents module unloading. */
    cw_nxo_t hook;

    /* Auxiliary data for pane_{set}aux. */
    cw_nxo_t aux;
};

static const struct cw_modpane_entry modpane_pane_hooks[] = {
    MODPANE_ENTRY(pane),
    MODPANE_ENTRY(subpane),
    MODPANE_ENTRY(pane_aux),
    MODPANE_ENTRY(pane_setaux)
};

void
modpane_pane_init(cw_nxo_t *a_thread)
{
    modpane_hooks_init(a_thread, modpane_pane_hooks,
		       (sizeof(modpane_pane_hooks)
			/ sizeof(struct cw_modpane_entry)));
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
	    retval = nxo_nxoe_get(&pane->hook);
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
pane_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
    struct cw_pane *pane = (struct cw_pane *) a_data;

    pn_delete(&pane->pn);

    return FALSE;
}

cw_nxn_t
modpane_pane_type(cw_nxo_t *a_nxo)
{
    cw_nxn_t retval;
    cw_nxo_t *tag;
    cw_uint32_t name_len;
    const cw_uint8_t *name;

    if (nxo_type_get(a_nxo) != NXOT_HOOK)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    tag = nxo_hook_tag_get(a_nxo);
    if (nxo_type_get(tag) != NXOT_NAME)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    name_len = nxo_name_len_get(tag);
    name = nxo_name_str_get(tag);
    if ((name_len != sizeof("pane") - 1) || strncmp("pane", name, name_len))
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
    cw_nx_t *nx;
    struct cw_pane *pane;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);

    pane = (struct cw_pane *) nxa_malloc(nx_nxa_get(nx),
					 sizeof(struct cw_pane));

    /* Create a reference to this operator in order to prevent the module from
     * being prematurely unloaded. */
    nxo_no_new(&pane->hook);
    nxo_dup(&pane->hook, nxo_stack_get(estack));

    /* Initialize the pn. */
    pn_new(&pane->pn, NULL, NULL); /* XXX */

    /* Create a reference to the pane. */
    nxo = nxo_stack_push(ostack);
    nxo_hook_new(nxo, nx, pane, NULL, pane_p_ref_iter, pane_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(nxo);
    nxo_name_new(tag, nx, "pane", sizeof("pane") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);
}

void
modpane_subpane(void *a_data, cw_nxo_t *a_thread)
{

}

void
modpane_pane_aux(void *a_data, cw_nxo_t *a_thread)
{
}

void
modpane_pane_setaux(void *a_data, cw_nxo_t *a_thread)
{
}
