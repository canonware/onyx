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

/* This is used to assure that methods are actually passed the correct class.
 * It is not reported to the GC, so can not safely be used for any other
 * purpose. */
static cw_nxo_t s_window;

struct cw_window
{
    /* Coordinates of top left corner, relative to the containing parent's top
     * left corner. */
    cw_uint32_t xstart, ystart;

    /* Size of window. */
    cw_uint32_t xsize, ysize;

    /* Containing frame. */
    cw_nxo_t frame;

    /* Parent window object, or a null object if the top level window within the
     * containing frame. */
    cw_nxo_t parent;

    /* Child windows, if a container window, null objects otherwise. */
    cw_nxo_t left_child;
    cw_nxo_t right_child;

    /* Buffer-related state, if not a container window, null objects otherwise.
     */

    /* Associated buffer. */
    cw_nxo_t buffer;

    /* Extent that includes the entire buffer contents that are before the
     * beginning of the visible region. */
    cw_nxo_t pre_extent;

    /* Extent that includes the visible range. */
    cw_nxo_t vis_extent;

    /* Mark. */
    cw_nxo_t mark_marker;

    /* Point. */
    cw_nxo_t point_marker;
};

static const struct cw_modslate_method modslate_window_methods[] = {
    MODSLATE_METHOD(window, window)//,
//    MODSLATE_METHOD(window, container_p),
//    MODSLATE_METHOD(window, minibuffer_p),
//    MODSLATE_METHOD(window, hsplit),
//    MODSLATE_METHOD(window, vsplit),
//    MODSLATE_METHOD(window, size_get),
//    MODSLATE_METHOD(window, size_set)
};

static cw_nxn_t
window_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	     struct cw_window **r_window);
static cw_nxoe_t *
window_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
window_p_delete(void *a_data, cw_uint32_t a_iter);

void
modslate_window_init(cw_nxo_t *a_thread)
{
    nxo_no_new(&s_window);
    modslate_class_init(a_thread, "window", modslate_window_methods,
			(sizeof(modslate_window_methods)
			 / sizeof(struct cw_modslate_method)),
			NULL, &s_window);
}

static cw_nxn_t
window_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	     struct cw_window **r_window)
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

    if ((retval = modslate_instance_kind(a_instance, &s_window)))
    {
	goto RETURN;
    }

    data = nxo_instance_data_get(a_instance);
    if (nxo_type_get(data) != NXOT_DICT)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    nxo_name_new(name, "window", sizeof("window") - 1, FALSE);
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

    *r_window = (struct cw_window *) nxo_handle_opaque_get(handle);
    retval = NXN_ZERO;
    RETURN:
    /* Clean up. */
    nxo_stack_npop(tstack, 2);

    return retval;
}

static cw_nxoe_t *
window_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_window *window = (struct cw_window *) a_data;
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
		retval = nxo_nxoe_get(&window->parent);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&window->left_child);
		break;
	    }
	    case 2:
	    {
		retval = nxo_nxoe_get(&window->right_child);
		break;
	    }
	    case 3:
	    {
		retval = nxo_nxoe_get(&window->buffer);
		break;
	    }
	    case 4:
	    {
		retval = nxo_nxoe_get(&window->pre_extent);
		break;
	    }
	    case 5:
	    {
		retval = nxo_nxoe_get(&window->vis_extent);
		break;
	    }
	    case 6:
	    {
		retval = nxo_nxoe_get(&window->mark_marker);
		break;
	    }
	    case 7:
	    {
		retval = nxo_nxoe_get(&window->point_marker);
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
window_p_delete(void *a_data, cw_uint32_t a_iter)
{
    struct cw_window	*window = (struct cw_window *)a_data;

    nxa_free(window, sizeof(struct cw_window));

    return FALSE;
}

cw_nxn_t
modslate_window_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread)
{
    struct cw_window *window;

    return window_p_get(a_instance, a_thread, &window);
}

/* #frame #instance :window #instance */
/* #frame #parent #instance :window #instance */
void
modslate_window_window(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *frame, *parent, *instance;
    cw_nxo_t *tag, *name, *handle;
    cw_nxn_t error;
    struct cw_window *window;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(parent, ostack, a_thread);
    if ((error = modslate_window_p(parent, a_thread)))
    {
	frame = parent;
	parent = NULL;
    }
    else
    {
	NXO_STACK_DOWN_GET(frame, ostack, a_thread, parent);
    }
    if ((error = modslate_frame_p(frame, a_thread)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    window = (struct cw_window *) nxa_malloc(sizeof(struct cw_window));

    /* Initialize position and size. */
    window->xstart = 0;
    window->ystart = 0;
    window->xsize = 0;
    window->ysize = 0;

    /* Create a reference to the containing frame. */
    nxo_no_new(&window->frame);
    nxo_dup(&window->frame, frame);

    /* Create a reference to the parent. */
    if (parent != NULL)
    {
	nxo_no_new(&window->parent);
	nxo_dup(&window->parent, parent);
    }
    else
    {
	nxo_null_new(&window->parent);
    }

    /* Initialize children. */
    nxo_null_new(&window->left_child);
    nxo_null_new(&window->right_child);

    /* Initialize buffer-related fields. */
    nxo_null_new(&window->buffer);
    nxo_null_new(&window->pre_extent);
    nxo_null_new(&window->vis_extent);
    nxo_null_new(&window->mark_marker);
    nxo_null_new(&window->point_marker);

    /* Create a reference to the window, now that the internals are
     * initialized. */
    name = nxo_stack_push(tstack);
    handle = nxo_stack_push(tstack);

    nxo_name_new(name, "window", sizeof("window") - 1, FALSE);
    nxo_handle_new(handle, window, NULL, window_p_ref_iter, window_p_delete);

    /* Set the handle tag. */
    tag = nxo_handle_tag_get(handle);
    nxo_dup(tag, name);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Insert into the data dict. */
    nxo_dict_def(nxo_instance_data_get(instance), name, handle);

    /* Clean up. */
    nxo_stack_npop(tstack, 2);
    nxo_stack_remove(ostack, frame);
    if (parent != NULL)
    {
	nxo_stack_remove(ostack, parent);
    }
}
