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

struct cw_window
{
    /* For GC iteration. */
    cw_uint32_t	iter;

    /* Reference to =window=, prevents module unload. */
    cw_nxo_t hook;

    /* Auxiliary data for display_aux_[gs]et. */
    cw_nxo_t aux;

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

    /*
     * Buffer-related state, if not a container window, null objects otherwise.
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

static const struct cw_modslate_entry modslate_window_hooks[] = {
    MODSLATE_ENTRY(window),
    {"window?", modslate_window_p},
//    MODSLATE_ENTRY(window_container_p),
//    MODSLATE_ENTRY(window_minibuffer_p),
    MODSLATE_ENTRY(window_aux_get),
    MODSLATE_ENTRY(window_aux_set)//,
//    MODSLATE_ENTRY(window_hsplit),
//    MODSLATE_ENTRY(window_vsplit),
//    MODSLATE_ENTRY(window_size_get),
//    MODSLATE_ENTRY(window_size_set)
};

static cw_nxoe_t *
window_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
window_p_delete(void *a_data, cw_uint32_t a_iter);

void
modslate_window_init(cw_nxo_t *a_thread)
{
    modslate_hooks_init(a_thread, modslate_window_hooks,
			(sizeof(modslate_window_hooks)
			 / sizeof(struct cw_modslate_entry)));
}

static cw_nxoe_t *
window_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_window *window = (struct cw_window *) a_data;

    if (a_reset)
    {
	window->iter = 0;
    }

    for (retval = NULL; retval == NULL; window->iter++)
    {
	switch (window->iter)
	{
	    case 0:
	    {
		retval = nxo_nxoe_get(&window->hook);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&window->aux);
		break;
	    }
	    case 2:
	    {
		retval = nxo_nxoe_get(&window->parent);
		break;
	    }
	    case 3:
	    {
		retval = nxo_nxoe_get(&window->left_child);
		break;
	    }
	    case 4:
	    {
		retval = nxo_nxoe_get(&window->right_child);
		break;
	    }
	    case 5:
	    {
		retval = nxo_nxoe_get(&window->buffer);
		break;
	    }
	    case 6:
	    {
		retval = nxo_nxoe_get(&window->pre_extent);
		break;
	    }
	    case 7:
	    {
		retval = nxo_nxoe_get(&window->vis_extent);
		break;
	    }
	    case 8:
	    {
		retval = nxo_nxoe_get(&window->mark_marker);
		break;
	    }
	    case 9:
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

/* #=frame= window #=window= */
/* #=frame= #=window= window #=window= */
void
modslate_window(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *tnxo, *tag;
    cw_nxo_t *frame, *parent;
    cw_nxn_t error;
    struct cw_window *window;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(parent, ostack, a_thread);
    if ((error = modslate_hook_type(parent, "window")) != NXN_ZERO)
    {
	frame = parent;
	parent = NULL;
    }
    else
    {
	NXO_STACK_DOWN_GET(frame, ostack, a_thread, parent);
    }
    if ((error = modslate_hook_type(frame, "frame")) != NXN_ZERO)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    if ((error = modslate_hook_type(parent, "frame")) != NXN_ZERO
	&& (error = modslate_hook_type(parent, "window")) != NXN_ZERO)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    window = (struct cw_window *)nxa_malloc(sizeof(struct cw_window));

    /* Create a reference to this operator in order to prevent the module from
     * being prematurely unloaded. */
    nxo_no_new(&window->hook);
    nxo_dup(&window->hook, nxo_stack_get(estack));

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
    tnxo = nxo_stack_push(tstack);
    nxo_hook_new(tnxo, window, NULL, window_p_ref_iter, window_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(tnxo);
    nxo_name_new(tag, "window", sizeof("window") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Clean up the stacks. */
    nxo_dup(parent, tnxo);
    nxo_stack_pop(tstack);
}

/* #object window? #boolean */
void
modslate_window_p(void *a_data, cw_nxo_t *a_thread)
{
    modslate_hook_p(a_data, a_thread, "window");
}

void
modslate_window_aux_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    cw_nxn_t error;
    struct cw_window *window;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "window");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    window = (struct cw_window *) nxo_hook_data_get(nxo);

    /* Avoid a GC race by using tnxo to store a reachable ref to the window. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_dup(nxo, &window->aux);
    nxo_stack_pop(tstack);
}

void
modslate_window_aux_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *aux;
    cw_nxn_t error;
    struct cw_window *window;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(aux, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, aux);
    error = modslate_hook_type(nxo, "window");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    window = (struct cw_window *) nxo_hook_data_get(nxo);

    nxo_dup(&window->aux, aux);
    nxo_stack_npop(ostack, 2);
}
