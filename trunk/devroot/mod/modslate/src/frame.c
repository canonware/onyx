/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * There are 1-n frames per display.  Each frame is the full size of the
 * display.
 *
 ******************************************************************************/

#include "../include/modslate.h"

struct cw_frame {
	cw_uint32_t	iter;	/* For GC iteration. */
	cw_nxo_t	hook;	/* Ref to =frame=, prevents mod unload. */
	cw_nxo_t	display;/* Parent display object. */
	WINDOW		*window;/* curses window */
	PANEL		*panel;	/* curses panel, associated with window. */
};

static const struct cw_slate_entry slate_frame_ops[] = {
	SLATE_ENTRY(frame),
	SLATE_ENTRY(frame_focus)
};

void
slate_frame_init(cw_nxo_t *a_thread)
{
	slate_hooks_init(a_thread, slate_frame_ops,
	    (sizeof(slate_frame_ops) / sizeof(struct cw_slate_entry)));
}

static cw_nxoe_t *
frame_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
	cw_nxoe_t	*retval;
	struct cw_frame	*frame = (struct cw_frame *)a_data;

	if (a_reset)
		frame->iter = 0;

	switch (frame->iter) {
	case 0:
		retval = nxo_nxoe_get(&frame->hook);
		frame->iter++;
		_cw_check_ptr(retval);
		break;
	case 1:
		retval = nxo_nxoe_get(&frame->display);
		frame->iter++;
		break;
	default:
		retval = NULL;
	}

	return retval;
}

static cw_bool_t
frame_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
	struct cw_frame	*frame = (struct cw_frame *)a_data;

	slate_slate_lock(NULL, NULL);
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
	slate_slate_unlock(NULL, NULL);

	nxa_free(nx_nxa_get(a_nx), frame, sizeof(struct cw_frame));

	return FALSE;
}

cw_nxn_t
frame_type(cw_nxo_t *a_nxo)
{
	cw_nxn_t		retval;
	cw_nxo_t		*tag;
	cw_uint32_t		name_len;
	const cw_uint8_t	*name;

	if (nxo_type_get(a_nxo) != NXOT_HOOK) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	tag = nxo_hook_tag_get(a_nxo);
	if (nxo_type_get(tag) != NXOT_NAME) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	name_len = nxo_name_len_get(tag);
	name = nxo_name_str_get(tag);
	if ((name_len != strlen("frame")) || strncmp("frame", name,
	    name_len)) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	retval = NXN_ZERO;
	RETURN:
	return retval;
}

/* %=display= frame %=frame= */
void
slate_frame(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t	*estack, *ostack, *tstack, *tnxo, *tag;
	cw_nxo_t	*display;
	cw_nx_t		*nx;
	cw_nxn_t	error;
	struct cw_frame	*frame;

	estack = nxo_thread_estack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);
	NXO_STACK_GET(display, ostack, a_thread);
	error = display_type(display);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	frame = (struct cw_frame *)nxa_malloc(nx_nxa_get(nx),
	    sizeof(struct cw_frame));

	/* Create a reference to the frame. */
	tnxo = nxo_stack_push(tstack);
	nxo_hook_new(tnxo, nx, frame, NULL, frame_p_ref_iter,
	    frame_p_delete);

	/* Set the hook tag. */
	tag = nxo_hook_tag_get(tnxo);
	nxo_name_new(tag, nx, "frame", sizeof("frame") - 1, FALSE);
	nxo_attr_set(tag, NXOA_EXECUTABLE);

	/*
	 * Create a reference to this operator in order to prevent the module
	 * from being prematurely unloaded.
	 */
	nxo_no_new(&frame->hook);
	nxo_dup(&frame->hook, nxo_stack_get(estack));

	/* Create a reference to the argument. */
	nxo_no_new(&frame->display);
	nxo_dup(&frame->display, display);

	frame->window = newwin(0, 0, 0, 0);
	if (frame->window == NULL) {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		nxo_stack_pop(tstack);
		return;
	}

	frame->panel = new_panel(frame->window);
	if (frame->panel == NULL) {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		nxo_stack_pop(tstack);
		return;
	}

	/* Clean up the stacks. */
	nxo_dup(display, tnxo);
	nxo_stack_pop(tstack);
}

void
slate_frame_focus(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *nxo;
	cw_nxn_t	error;
	struct cw_frame	*frame;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = frame_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	frame = (struct cw_frame *)nxo_hook_data_get(nxo);

	show_panel(frame->panel);

	nxo_stack_pop(ostack);
}
