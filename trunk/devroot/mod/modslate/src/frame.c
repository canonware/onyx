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

struct cw_frame
{
    /* For GC iteration. */
    cw_uint32_t	iter;

    /* Reference to =frame=, prevents module unload. */
    cw_nxo_t hook;

    /* Auxiliary data for display_aux_[gs]et. */
    cw_nxo_t aux;

    /* Parent display object. */
    cw_nxo_t display;

    /* curses window */
    WINDOW *window;

    /* curses panel, associated with window. */
    PANEL *panel;
};

static const struct cw_modslate_entry modslate_frame_hooks[] = {
    MODSLATE_ENTRY(frame),
    {"frame?", modslate_frame_p},
    MODSLATE_ENTRY(frame_aux_get),
    MODSLATE_ENTRY(frame_aux_set),
    MODSLATE_ENTRY(frame_focus)
};

static cw_nxoe_t *
frame_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
frame_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter);

void
modslate_frame_init(cw_nxo_t *a_thread)
{
    modslate_hooks_init(a_thread, modslate_frame_hooks,
			(sizeof(modslate_frame_hooks)
			 / sizeof(struct cw_modslate_entry)));
}

static cw_nxoe_t *
frame_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_frame *frame = (struct cw_frame *) a_data;

    if (a_reset)
    {
	frame->iter = 0;
    }

    for (retval = NULL; retval == NULL; frame->iter++)
    {
	switch (frame->iter)
	{
	    case 0:
	    {
		retval = nxo_nxoe_get(&frame->hook);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&frame->aux);
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
frame_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
	struct cw_frame	*frame = (struct cw_frame *)a_data;

//	slate_slate_lock(NULL, NULL);
#ifdef _CW_DBG
	{
		int	error;

		error = del_panel(frame->panel);
		if (error) {
			fprintf(stderr, "%s:%d:%s(): Error in del_panel()\n",
			    __FILE__, __LINE__, __FUNCTION__);
		}
		error = delwin(frame->window);
		if (error) {
			fprintf(stderr, "%s:%d:%s(): Error in delwin()\n",
			    __FILE__, __LINE__, __FUNCTION__);
		}
	}
#else
	del_panel(frame->panel);
	delwin(frame->window);
#endif
//	slate_slate_unlock(NULL, NULL);

	nxa_free(nx_nxa_get(a_nx), frame, sizeof(struct cw_frame));

	return FALSE;
}

/* #=display= frame #=frame= */
void
modslate_frame(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *tnxo, *tag;
    cw_nxo_t *display;
    cw_nx_t *nx;
    cw_nxn_t error;
    struct cw_frame *frame;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    NXO_STACK_GET(display, ostack, a_thread);
    error = modslate_hook_type(display, "display");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    frame = (struct cw_frame *)nxa_malloc(nx_nxa_get(nx),
					  sizeof(struct cw_frame));

    /* Create a reference to the frame. */
    tnxo = nxo_stack_push(tstack);
    nxo_hook_new(tnxo, nx, frame, NULL, frame_p_ref_iter, frame_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(tnxo);
    nxo_name_new(tag, nx, "frame", sizeof("frame") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Create a reference to this operator in order to prevent the module from
     * being prematurely unloaded. */
    nxo_no_new(&frame->hook);
    nxo_dup(&frame->hook, nxo_stack_get(estack));

    /* Create a reference to the argument. */
    nxo_no_new(&frame->display);
    nxo_dup(&frame->display, display);

    frame->window = newwin(0, 0, 0, 0);
    if (frame->window == NULL)
    {
	nxo_thread_nerror(a_thread, NXN_unregistered);
	nxo_stack_pop(tstack);
	return;
    }

    frame->panel = new_panel(frame->window);
    if (frame->panel == NULL)
    {
	nxo_thread_nerror(a_thread, NXN_unregistered);
	nxo_stack_pop(tstack);
	return;
    }

    /* Clean up the stacks. */
    nxo_dup(display, tnxo);
    nxo_stack_pop(tstack);
}

/* #object frame? #boolean */
void
modslate_frame_p(void *a_data, cw_nxo_t *a_thread)
{
    modslate_hook_p(a_data, a_thread, "frame");
}

void
modslate_frame_aux_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    cw_nxn_t error;
    struct cw_frame *frame;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "frame");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    frame = (struct cw_frame *) nxo_hook_data_get(nxo);

    /* Avoid a GC race by using tnxo to store a reachable ref to the frame. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_dup(nxo, &frame->aux);
    nxo_stack_pop(tstack);
}

void
modslate_frame_aux_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *aux;
    cw_nxn_t error;
    struct cw_frame *frame;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(aux, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, aux);
    error = modslate_hook_type(nxo, "frame");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    frame = (struct cw_frame *) nxo_hook_data_get(nxo);

    nxo_dup(&frame->aux, aux);
    nxo_stack_npop(ostack, 2);
}

void
modslate_frame_focus(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_frame *frame;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "frame");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    frame = (struct cw_frame *)nxo_hook_data_get(nxo);

    show_panel(frame->panel);

    nxo_stack_pop(ostack);
}
