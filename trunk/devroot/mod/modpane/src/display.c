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
static cw_nxo_t s_display;

struct cw_display
{
    /* Display. */
    cw_ds_t ds;

    /* Protects all ds operations. */
    cw_mtx_t mtx;
};

static const struct cw_modpane_method modpane_display_methods[] = {
    MODPANE_METHOD(display, display),
    MODPANE_METHOD(display, size),
    MODPANE_METHOD(display, pane),
    MODPANE_METHOD(display, start),
    MODPANE_METHOD(display, stop),
    MODPANE_METHOD(display, refresh)
};

static cw_nxn_t
display_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	      struct cw_display **r_display);
static cw_nxoe_t *
display_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
display_p_delete(void *a_data, cw_uint32_t a_iter);

void
modpane_display_init(cw_nxo_t *a_thread)
{
    nxo_no_new(&s_display);
    modpane_class_init(a_thread, "display", modpane_display_methods,
		       (sizeof(modpane_display_methods)
			/ sizeof(struct cw_modpane_method)),
		       NULL, &s_display);
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

static cw_nxn_t
display_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	      struct cw_display **r_display)
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

    if ((retval = modpane_instance_kind(a_instance, &s_display)))
    {
	goto RETURN;
    }

    data = nxo_instance_data_get(a_instance);
    if (nxo_type_get(data) != NXOT_DICT)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    nxo_name_new(name, "display", sizeof("display") - 1, FALSE);
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

    *r_display = (struct cw_display *) nxo_handle_opaque_get(handle);
    retval = NXN_ZERO;
    RETURN:
    /* Clean up. */
    nxo_stack_npop(tstack, 2);

    return retval;
}

static cw_nxoe_t *
display_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
//    struct cw_display *display = (struct cw_display *) a_data;
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
//		retval = nxo_nxoe_get(&display->);
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
display_p_delete(void *a_data, cw_uint32_t a_iter)
{
    struct cw_display *display = (struct cw_display *) a_data;

    mtx_delete(&display->mtx);
    ds_delete(&display->ds);
    nxa_free(display, sizeof(struct cw_display));

    return FALSE;
}

cw_nxn_t
modpane_display_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread)
{
    struct cw_display *display;

    return display_p_get(a_instance, a_thread, &display);
}

/* #term #infile #outfile #instance :display #instance */
void
modpane_display_display(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *term, *infile, *outfile, *instance;
    cw_nxo_t *tag, *name, *handle;
    struct cw_display *display;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(instance, ostack, a_thread);
    NXO_STACK_DOWN_GET(outfile, ostack, a_thread, instance);
    NXO_STACK_DOWN_GET(infile, ostack, a_thread, outfile);
    NXO_STACK_DOWN_GET(term, ostack, a_thread, infile);
    if (nxo_type_get(instance) != NXOT_INSTANCE
	|| nxo_type_get(outfile) != NXOT_FILE
	|| nxo_type_get(infile) != NXOT_FILE
	|| nxo_type_get(term) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    display = (struct cw_display *) nxa_malloc(sizeof(struct cw_display));

    /* Initialize the ds. */
    ds_new(&display->ds, cw_g_nxaa, nxo_file_fd_get(infile),
	   nxo_file_fd_get(outfile));

    /* Initialize the protection mutex; ds's aren't thread-safe. */
    mtx_new(&display->mtx);

    /* Create a reference to the display, now that the internals are
     * initialized. */
    name = nxo_stack_push(tstack);
    handle = nxo_stack_push(tstack);

    nxo_name_new(name, "display", sizeof("display") - 1, FALSE);
    nxo_handle_new(handle, display, NULL, display_p_ref_iter, display_p_delete);

    /* Set the handle tag. */
    tag = nxo_handle_tag_get(handle);
    nxo_dup(tag, name);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Insert into the data dict. */
    nxo_dict_def(nxo_instance_data_get(instance), name, handle);

    /* Clean up. */
    nxo_stack_npop(tstack, 2);
    nxo_stack_remove(ostack, term);
    nxo_stack_remove(ostack, infile);
    nxo_stack_remove(ostack, outfile);
}

/* #display :size #x #y */
void
modpane_display_size(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;
    cw_uint32_t x, y;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = display_p_get(nxo, a_thread, &display)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    display_p_lock(display);
    ds_size(&display->ds, &x, &y);
    display_p_unlock(display);

    nxo_integer_new(nxo, x);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, y);
}

// XXX Should be a pane method?
/* #display :pane #pane */
void
modpane_display_pane(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

/* #display :start - */
void
modpane_display_start(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = display_p_get(nxo, a_thread, &display)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    display_p_lock(display);
    /* XXX Check error return. */
    ds_start(&display->ds);
    display_p_unlock(display);
}

/* #display :stop - */
void
modpane_display_stop(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = display_p_get(nxo, a_thread, &display)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    display_p_lock(display);
    /* XXX Check error return. */
    ds_stop(&display->ds);
    display_p_unlock(display);
}

/* #display :refresh - */
void
modpane_display_refresh(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}
