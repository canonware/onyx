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

struct cw_display
{
    /* For GC iteration. */
    cw_uint32_t iter;

    /* Reference to =display=, prevents premature module unload. */
    cw_nxo_t hook;

    /* Auxiliary data for display_aux_[gs]et. */
    cw_nxo_t aux;

    /* Display. */
    cw_ds_t ds;

    /* Protects all ds operations. */
    cw_mtx_t mtx;
};

static const struct cw_modpane_entry modpane_display_hooks[] = {
    /* display. */
    MODPANE_ENTRY(display),
    {"display?", modpane_display_p},
    MODPANE_ENTRY(display_aux_get),
    MODPANE_ENTRY(display_aux_set),
    MODPANE_ENTRY(display_size),
    MODPANE_ENTRY(display_pane),
    MODPANE_ENTRY(display_start),
    MODPANE_ENTRY(display_stop),
    MODPANE_ENTRY(display_refresh)
};

static cw_nxoe_t *
display_p_ref_iter(void *a_data, cw_bool_t a_reset);

static cw_bool_t
display_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter);

void
modpane_display_init(cw_nxo_t *a_thread)
{
    modpane_hooks_init(a_thread, modpane_display_hooks,
		       (sizeof(modpane_display_hooks)
			/ sizeof(struct cw_modpane_entry)));
}

CW_P_INLINE void
display_p_lock(struct cw_display *a_display)
{
    mtx_lock(&a_display->mtx);
}

CW_P_INLINE void
display_p_unlock(struct cw_display *a_display)
{
    mtx_unlock(&a_display->mtx);
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
	    cw_check_ptr(retval);
	    break;
	}
	case 1:
	{
	    retval = nxo_nxoe_get(&display->aux);
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

    mtx_delete(&display->mtx);
    ds_delete(&display->ds);
    nxa_free(display, sizeof(struct cw_display));

    return FALSE;
}

/*
 * Verify that a_nxo is a =display=.
 */
cw_nxn_t
display_type(cw_nxo_t *a_nxo)
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
    if ((name_len != strlen("display")) || strncmp("display", name, name_len))
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
modpane_display(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *nxo, *tag;
    cw_nxo_t *term, *infile, *outfile;
    cw_nx_t *nx;
    struct cw_display *display;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);

    NXO_STACK_GET(outfile, ostack, a_thread);
    NXO_STACK_DOWN_GET(infile, ostack, a_thread, outfile);
    NXO_STACK_DOWN_GET(term, ostack, a_thread, infile);
    if (nxo_type_get(outfile) != NXOT_FILE
	|| nxo_type_get(infile) != NXOT_FILE
	|| nxo_type_get(term) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    display = (struct cw_display *) nxa_malloc(sizeof(struct cw_display));

    /* Create a reference to this hook in order to prevent the module from being
     * prematurely unloaded. */
    nxo_no_new(&display->hook);
    nxo_dup(&display->hook, nxo_stack_get(estack));

    /* Initialize the ds. */
    /* XXX Use cw_g_nxaa. */
    ds_new(&display->ds, (cw_opaque_alloc_t *) nxa_malloc_e,
	   (cw_opaque_realloc_t *) nxa_realloc_e,
	   (cw_opaque_dealloc_t *) nxa_free_e, (void *) cw_g_nxa,
	   nxo_file_fd_get(infile), nxo_file_fd_get(outfile));

    /* Initialize the protection mutex; ds's aren't thread-safe. */
    mtx_new(&display->mtx);

    /* Create a reference to the display. */
    nxo = nxo_stack_under_push(ostack, term);
    nxo_hook_new(nxo, nx, display, NULL, display_p_ref_iter, display_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(nxo);
    nxo_name_new(tag, nx, "display", sizeof("display") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Initialize aux. */
    nxo_null_new(&display->aux);

    /* Clean up ostack. */
    nxo_stack_npop(ostack, 3);
}

/* %object display? %boolean */
void
modpane_display_p(void *a_data, cw_nxo_t *a_thread)
{
    modpane_hook_p(a_data, a_thread, "display");
}

void
modpane_display_aux_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = display_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *) nxo_hook_data_get(nxo);

    /* Avoid a GC race by using tnxo to store a reachable ref to the display. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_dup(nxo, &display->aux);
    nxo_stack_pop(tstack);
}

void
modpane_display_aux_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *aux;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(aux, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, aux);
    error = display_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *) nxo_hook_data_get(nxo);

    nxo_dup(&display->aux, aux);
    nxo_stack_npop(ostack, 2);
}

void
modpane_display_size(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;
    cw_uint32_t x, y;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = display_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *) nxo_hook_data_get(nxo);

    display_p_lock(display);
    ds_size(&display->ds, &x, &y);
    display_p_unlock(display);

    nxo_integer_new(nxo, x);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, y);
}

void
modpane_display_pane(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modpane_display_start(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = display_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *) nxo_hook_data_get(nxo);

    display_p_lock(display);
    /* XXX Check error return. */
    ds_start(&display->ds);
    display_p_unlock(display);
}

void
modpane_display_stop(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = display_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *) nxo_hook_data_get(nxo);

    display_p_lock(display);
    /* XXX Check error return. */
    ds_stop(&display->ds);
    display_p_unlock(display);
}

void
modpane_display_refresh(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}
