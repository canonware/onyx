/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version = slate>
 *
 ******************************************************************************/

#include "../include/modslate.h"
#include "modpane.h"

/* This is used to assure that methods are actually passed the correct class.
 * It is not reported to the GC, so can not safely be used for any other
 * purpose. */
static cw_nxo_t s_frame;

struct cw_frame
{
    /* Parent display object. */
    cw_nxo_t display;

    /* curses window */
//    WINDOW *window;

    /* curses panel, associated with window. */
//    PANEL *panel;
};

static const struct cw_modslate_method modslate_frame_methods[] = {
    MODSLATE_METHOD(frame, frame),
    MODSLATE_METHOD(frame, focus)//,
//    MODSLATE_METHOD(frame, window_current),
//    MODSLATE_METHOD(frame, window_prev),
//    MODSLATE_METHOD(frame, window_next)
};

static cw_nxn_t
frame_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	    struct cw_frame **r_frame);
static cw_nxoe_t *
frame_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
frame_p_delete(void *a_data, cw_uint32_t a_iter);

void
modslate_frame_init(cw_nxo_t *a_thread)
{
    nxo_no_new(&s_frame);
    modslate_class_init(a_thread, "frame", modslate_frame_methods,
			(sizeof(modslate_frame_methods)
			 / sizeof(struct cw_modslate_method)),
			NULL, &s_frame);
}

static cw_nxn_t
frame_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	    struct cw_frame **r_frame)
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

    if ((retval = modslate_instance_kind(a_instance, &s_frame)))
    {
	goto RETURN;
    }

    data = nxo_instance_data_get(a_instance);
    if (nxo_type_get(data) != NXOT_DICT)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    nxo_name_new(name, "frame", sizeof("frame") - 1, FALSE);
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

    *r_frame = (struct cw_frame *) nxo_handle_opaque_get(handle);
    retval = NXN_ZERO;
    RETURN:
    /* Clean up. */
    nxo_stack_npop(tstack, 2);

    return retval;
}

static cw_nxoe_t *
frame_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_frame *frame = (struct cw_frame *) a_data;
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
		retval = nxo_nxoe_get(&frame->display);
		break;
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
frame_p_delete(void *a_data, cw_uint32_t a_iter)
{
	struct cw_frame	*frame = (struct cw_frame *)a_data;

//	del_panel(frame->panel);
//	delwin(frame->window);

	nxa_free(frame, sizeof(struct cw_frame));

	return FALSE;
}

cw_nxn_t
modslate_frame_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread)
{
    struct cw_frame *frame;

    return frame_p_get(a_instance, a_thread, &frame);
}

/* #display #instance :frame #instance */
void
modslate_frame_frame(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *display, *instance;
    cw_nxo_t *tag, *name, *handle;
    cw_nxn_t error;
    struct cw_frame *frame;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(instance, ostack, a_thread);
    if (nxo_type_get(instance) != NXOT_INSTANCE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    NXO_STACK_DOWN_GET(display, ostack, a_thread, instance);
    error = modpane_display_p(display, a_thread);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    frame = (struct cw_frame *) nxa_malloc(sizeof(struct cw_frame));

    /* Create a reference to the display. */
    nxo_no_new(&frame->display);
    nxo_dup(&frame->display, display);

//    frame->window = newwin(0, 0, 0, 0);

//    frame->panel = new_panel(frame->window);

    /* Create a reference to the frame, now that the internals are
     * initialized. */
    name = nxo_stack_push(tstack);
    handle = nxo_stack_push(tstack);

    nxo_name_new(name, "frame", sizeof("frame") - 1, FALSE);
    nxo_handle_new(handle, frame, NULL, frame_p_ref_iter, frame_p_delete);

    /* Set the handle tag. */
    tag = nxo_handle_tag_get(handle);
    nxo_dup(tag, name);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Insert into the data dict. */
    nxo_dict_def(nxo_instance_data_get(instance), name, handle);

    /* Clean up. */
    nxo_stack_npop(tstack, 2);
    nxo_stack_remove(ostack, display);
}

/* #frame :focus - */
void
modslate_frame_focus(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_frame *frame;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = frame_p_get(nxo, a_thread, &frame)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

//    show_panel(frame->panel);

    nxo_stack_pop(ostack);
}
