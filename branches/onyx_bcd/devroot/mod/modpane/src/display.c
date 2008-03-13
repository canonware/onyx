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

struct cw_display
{
    /* For GC iteration. */
    cw_uint32_t iter;

    /* Display. */
    cw_ds_t ds;

    /* Reference to =display=, prevents module unloading. */
    cw_nxo_t hook;

    /* Auxiliary data for display_{set}aux. */
    cw_nxo_t aux;
};

static const struct cw_modpane_entry modpane_display_hooks[] = {
    MODPANE_ENTRY(display),
    MODPANE_ENTRY(display_aux),
    MODPANE_ENTRY(display_setaux)
};

void
modpane_display_init(cw_nxo_t *a_thread)
{
    modpane_hooks_init(a_thread, modpane_display_hooks,
		       (sizeof(modpane_display_hooks)
			/ sizeof(struct cw_modpane_entry)));
}


static cw_nxoe_t *
display_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_display *display = (struct cw_display *) a_data;

    if (a_reset)
    {
	display->iter = 0;
    }

    switch (display->iter)
    {
	case 0:
	{
	    retval = nxo_nxoe_get(&display->hook);
	    break;
	}
	default:
	{
	    retval = NULL;
	}
    }

    display->iter++;

    return retval;
}

static cw_bool_t
display_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
    struct cw_display *display = (struct cw_display *) a_data;

    ds_delete(&display->ds);
    nxa_free(nx_nxa_get(a_nx), display, sizeof(struct cw_display));

    return FALSE;
}


cw_nxn_t
modpane_display_type(cw_nxo_t *a_nxo)
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
    if ((name_len != sizeof("display") - 1)
	|| strncmp("display", name, name_len))
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

/* %term %in %out display %=display= */
void
modpane_display(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *nxo, *tnxo, *tag;
    cw_nx_t *nx;
    struct cw_display *display;
    int in, out;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    /* %out. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    out = nxo_file_fd_get(nxo);

    /* %in. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    in = nxo_file_fd_get(nxo);

    /* %term. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    tnxo = nxo_stack_push(tstack);
    nxo_string_cstring(tnxo, nxo, a_thread);

    display = (struct cw_display *) nxa_malloc(nx_nxa_get(nx),
					       sizeof(struct cw_display));

    /* Create a reference to this operator in order to prevent the module from
     * being prematurely unloaded. */
    nxo_no_new(&display->hook);
    nxo_dup(&display->hook, nxo_stack_get(estack));

    /* Initialize the ds. */
    ds_new(&display->ds, in, out, nxo_string_get(tnxo));

    nxo_stack_pop(tstack);

    /* Create a reference to the display. */
    nxo = nxo_stack_push(ostack);
    nxo_hook_new(nxo, nx, display, NULL, display_p_ref_iter, display_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(nxo);
    nxo_name_new(tag, nx, "display", sizeof("display") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);
}

void
modpane_display_aux(void *a_data, cw_nxo_t *a_thread)
{

}

void
modpane_display_setaux(void *a_data, cw_nxo_t *a_thread)
{

}
