/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "slate.h"

struct cw_window {
	cw_uint32_t	iter;	/* For GC iteration. */
	cw_nxo_t	hook;	/* Ref to =window=, prevents mod unload. */
	cw_nxo_t	frame;	/* Parent frame object. */
	cw_nxo_t	buffer;	/* Associated buffer. */
};

static const struct cw_slate_entry slate_window_ops[] = {
	SLATE_ENTRY(window),
	SLATE_ENTRY(window_buffer),
	SLATE_ENTRY(window_setbuffer)
};

void
slate_window_init(cw_nxo_t *a_thread)
{
	slate_ops(a_thread, slate_window_ops,
	    (sizeof(slate_window_ops) / sizeof(struct cw_slate_entry)));
}

static cw_nxoe_t *
window_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
	cw_nxoe_t		*retval;
	struct cw_window	*window = (struct cw_window *)a_data;

	if (a_reset)
		window->iter = 0;

	switch (window->iter) {
	case 0:
		retval = nxo_nxoe_get(&window->hook);
		window->iter++;
		_cw_check_ptr(retval);
		break;
	case 1:
		retval = nxo_nxoe_get(&window->frame);
		_cw_check_ptr(retval);
		window->iter++;
		break;
	case 2:
		retval = nxo_nxoe_get(&window->buffer);
		window->iter++;
		break;
	default:
		retval = NULL;
	}

	return retval;
}

static cw_bool_t
window_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
	struct cw_window	*window = (struct cw_window *)a_data;

/* 	slate_slate_lock(NULL, NULL); */
/* 	slate_slate_unlock(NULL, NULL); */

	nxa_free(nx_nxa_get(a_nx), window, sizeof(struct cw_window));

	return FALSE;
}

cw_nxn_t
window_type(cw_nxo_t *a_nxo)
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
	if ((name_len != strlen("window")) || strncmp("window", name,
	    name_len)) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	retval = NXN_ZERO;
	RETURN:
	return retval;
}

/* %=frame= %=buffer= window %=window= */
void
slate_window(cw_nxo_t *a_thread)
{
	cw_nxo_t		*estack, *ostack, *tstack, *tnxo, *tag;
	cw_nxo_t		*frame, *buffer;
	cw_nx_t			*nx;
	cw_nxn_t		error;
	struct cw_window	*window;

	estack = nxo_thread_estack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);
	NXO_STACK_GET(buffer, ostack, a_thread);
	error = buffer_type(buffer);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	NXO_STACK_DOWN_GET(frame, ostack, a_thread, buffer);
	error = frame_type(frame);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	window = (struct cw_window *)nxa_malloc(nx_nxa_get(nx),
	    sizeof(struct cw_window));

	/* Create a reference to the window. */
	tnxo = nxo_stack_push(tstack);
	nxo_hook_new(tnxo, nx, window, NULL, window_p_ref_iter,
	    window_p_delete);

	/* Set the hook tag. */
	tag = nxo_hook_tag_get(tnxo);
	nxo_name_new(tag, nx, "window", sizeof("window") - 1, FALSE);
	nxo_attr_set(tag, NXOA_EXECUTABLE);

	/*
	 * Create a reference to this operator in order to prevent the module
	 * from being prematurely unloaded.
	 */
	nxo_no_new(&window->hook);
	nxo_dup(&window->hook, nxo_stack_get(estack));

	/* Create references to the arguments. */
	nxo_no_new(&window->frame);
	nxo_dup(&window->frame, frame);

	nxo_no_new(&window->buffer);
	nxo_dup(&window->buffer, buffer);

	/* Clean up the stacks. */
	nxo_dup(frame, tnxo);
	nxo_stack_pop(ostack);
	nxo_stack_pop(tstack);
}

/* %=window= window_buffer %=buffer= */
void
slate_window_buffer(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *tstack, *nxo, *tnxo;
	cw_nxn_t		error;
	struct cw_window	*window;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = window_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	window = (struct cw_window *)nxo_hook_data_get(nxo);

	tnxo = nxo_stack_push(tstack);
	nxo_dup(tnxo, nxo);
	nxo_dup(nxo, &window->buffer);
	nxo_stack_pop(tstack);
}

/* %=window %=buffer window_setbuffer - */
void
slate_window_setbuffer(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *tstack, *nxo, *buffer;
	cw_nxn_t		error;
	struct cw_window	*window;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(buffer, ostack, a_thread);
	error = buffer_type(buffer);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, buffer);
	error = window_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	window = (struct cw_window *)nxo_hook_data_get(nxo);

	nxo_dup(&window->buffer, buffer);
	nxo_stack_npop(ostack, 2);
}
