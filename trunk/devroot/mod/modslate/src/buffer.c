/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * This file contains the implementations of all the glue operators for =buffer=
 * and =marker=.  In some ways it would be nice to instead implement =buf= as a
 * more abstract type, then create an Onyx dict that implements the full range
 * of operations neccessary on a text editor buffer (such as file
 * reading/writing), and this would allow a big portion of the buffer code to be
 * written in Onyx.  However, this would create one very significant problem: it
 * wouldn't be possible to retrieve a reference to the buffer dict, given a
 * =marker=.  Thus, =buffer= exists, and there are operators to provide access
 * to all necessary =buffer= operations.
 *
 * A note about GC reference iteration is in order.  =marker=s report references
 * to their associated =buffer=s, but =buffer=s do not report references to
 * =marker=s.  This makes sense, but has implications in buf_delete() and
 * bufm_delete(), since during a GC sweep, associated buf's and bufm's may be
 * deleted, in no particular order.  Therefore, buf_delete() and bufm_delete()
 * take care to allow destruction in any order.  This mechanism relies on the
 * fact that the GC sweep is single-threaded.
 *
 ******************************************************************************/

#include "../include/modslate.h"

struct cw_buffer {
	cw_nxo_t	hook;	/* Ref to =buffer=, prevents mod unload. */
	cw_buf_t	buf;
};

/*
 * This structure is necessary since =marker=s need to report a reference to
 * their associated =buffer=s.
 */
struct cw_marker {
	cw_uint32_t	iter;	/* For reference iteration. */
	cw_nxo_t	hook;	/* Ref to =marker=, prevents mod unload. */
	cw_nxo_t	buffer_nxo;
	cw_bufm_t	bufm;
};

static const struct cw_slate_entry slate_buffer_ops[] = {
	ENTRY(buffer),
	ENTRY(buffer_length),
	ENTRY(buffer_lines),
	ENTRY(buffer_undoable),
	ENTRY(buffer_redoable),
	ENTRY(buffer_undo),
	ENTRY(buffer_redo),
	ENTRY(buffer_history_active),
	ENTRY(buffer_history_setactive),
	ENTRY(buffer_history_startgroup),
	ENTRY(buffer_history_endgroup),
	ENTRY(buffer_history_flush),
	ENTRY(marker),
	ENTRY(marker_copy),
	ENTRY(marker_buffer),
	ENTRY(marker_line),
	ENTRY(marker_seekline),
	ENTRY(marker_position),
	ENTRY(marker_seek),
	ENTRY(marker_before_get),
	ENTRY(marker_after_get),
	ENTRY(marker_before_set),
	ENTRY(marker_after_set),
	ENTRY(marker_before_insert),
	ENTRY(marker_after_insert),
	ENTRY(marker_range_get),
	ENTRY(marker_range_cut)
};

static void buffer_p_eval(void *a_data, cw_nxo_t *a_thread);
static cw_nxoe_t *buffer_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t buffer_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t
    a_iter);
static cw_nxn_t buffer_p_type(cw_nxo_t *a_nxo);

static cw_nxoe_t *marker_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t marker_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t
    a_iter);
static cw_nxn_t marker_p_type(cw_nxo_t *a_nxo);
static cw_bufw_t marker_p_whence(cw_nxo_t *a_whence);

void
slate_buffer_init(cw_nxo_t *a_thread)
{
	slate_hooks_init(a_thread, slate_buffer_ops, (sizeof(slate_buffer_ops) /
	    sizeof(struct cw_slate_entry)));
}

static void
buffer_p_eval(void *a_data, cw_nxo_t *a_thread)
{
	/*
	 * Since there is risk of deadlock if we recursively invoke the Onyx
	 * interpreter with the buffer lock held, instead malloc a copy of the
	 * buffer and feed it to the interpreter.  Catch all exceptions to
	 * deallocate the copy in case of an error.
	 */
	_cw_error("XXX Not implemented");
}

static cw_nxoe_t *
buffer_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
	cw_nxoe_t		*retval;
	struct cw_buffer	*buffer = (struct cw_buffer *)a_data;

	if (a_reset)
		retval = nxo_nxoe_get(&buffer->hook);
	else
		retval = NULL;

	return retval;
}

static cw_bool_t
buffer_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
	struct cw_buffer	*buffer = (struct cw_buffer *)a_data;

	buf_delete(&buffer->buf);
	nxa_free(nx_nxa_get(a_nx), buffer, sizeof(struct cw_buffer));

	return FALSE;
}

/*
 * Verify that a_nxo is a =buffer=.
 */
static cw_nxn_t
buffer_p_type(cw_nxo_t *a_nxo)
{
	cw_nxn_t		retval;
	cw_nxo_t		*tag;
	cw_uint32_t		name_len;
	const cw_uint8_t	*name_buffer;

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
	name_buffer = nxo_name_str_get(tag);
	if ((name_len != strlen("buffer")) || strncmp("buffer", name_buffer,
	    name_len)) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	retval = NXN_ZERO;
	RETURN:
	return retval;
}

void
slate_buffer(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*estack, *ostack, *nxo, *tag;
	cw_nx_t			*nx;
	struct cw_buffer	*buffer;

	estack = nxo_thread_estack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);

	buffer = (struct cw_buffer *)nxa_malloc(nx_nxa_get(nx),
	    sizeof(struct cw_buffer));

	/*
	 * Create a reference to this operator in order to prevent the module
	 * from being prematurely unloaded.
	 */
	nxo_no_new(&buffer->hook);
	nxo_dup(&buffer->hook, nxo_stack_get(estack));

	/* Initialize the buf. */
	buf_new(&buffer->buf, (cw_opaque_alloc_t *)nxa_malloc_e,
	    (cw_opaque_realloc_t *)nxa_realloc_e, (cw_opaque_dealloc_t
	    *)nxa_free_e, (void *)nx_nxa_get(nx));

	/* Create a reference to the buffer. */
	nxo = nxo_stack_push(ostack);
	nxo_hook_new(nxo, nx, buffer, buffer_p_eval, buffer_p_ref_iter,
	    buffer_p_delete);

	/* Set the hook tag. */
	tag = nxo_hook_tag_get(nxo);
	nxo_name_new(tag, nx, "buffer", sizeof("buffer") - 1, FALSE);
	nxo_attr_set(tag, NXOA_EXECUTABLE);
}

void
slate_buffer_length(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	nxo_integer_new(nxo, buf_len(&buffer->buf));
	buf_unlock(&buffer->buf);
}

void
slate_buffer_lines(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	nxo_integer_new(nxo, buf_nlines(&buffer->buf));
	buf_unlock(&buffer->buf);
}

void
slate_buffer_undoable(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;
	cw_bool_t		undoable;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	undoable = buf_undoable(&buffer->buf);
	buf_unlock(&buffer->buf);

	nxo_boolean_new(nxo, undoable);
}

void
slate_buffer_redoable(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;
	cw_bool_t		redoable;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	redoable = buf_redoable(&buffer->buf);
	buf_unlock(&buffer->buf);

	nxo_boolean_new(nxo, redoable);
}

/* XXX Bad API. */
void
slate_buffer_undo(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	buf_undo(buf, &marker->bufm, 1);
	buf_unlock(buf);
}

/* XXX Bad API. */
void
slate_buffer_redo(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	buf_redo(buf, &marker->bufm, 1);
	buf_unlock(buf);
}

void
slate_buffer_history_active(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;
	cw_bool_t		history_active;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	history_active = buf_hist_active_get(&buffer->buf);
	buf_unlock(&buffer->buf);

	nxo_boolean_new(nxo, history_active);
}

void
slate_buffer_history_setactive(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;
	cw_bool_t		active;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_BOOLEAN) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}
	active = nxo_boolean_get(nxo);

	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	buf_hist_active_set(&buffer->buf, active);
	buf_unlock(&buffer->buf);

	nxo_stack_npop(ostack, 2);
}

void
slate_buffer_history_startgroup(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;
	struct cw_marker	*marker;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		/* No marker specified. */
		marker = NULL;
	} else {
		marker = (struct cw_marker *)nxo_hook_data_get(nxo);
		
		NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	}

	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	if (marker != NULL)
		buf_hist_group_beg(&buffer->buf, &marker->bufm);
	else
		buf_hist_group_beg(&buffer->buf, NULL);
	buf_unlock(&buffer->buf);
}

void
slate_buffer_history_endgroup(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	buf_hist_group_end(&buffer->buf);
	buf_unlock(&buffer->buf);
}

void
slate_buffer_history_flush(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_buffer	*buffer;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	buf_lock(&buffer->buf);
	_cw_error("XXX Not implemented");
	buf_unlock(&buffer->buf);
}

static cw_nxoe_t *
marker_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
	cw_nxoe_t		*retval;
	struct cw_marker	*marker = (struct cw_marker *)a_data;

	if (a_reset)
		marker->iter = 0;

	switch (marker->iter) {
	case 0:
		marker->iter++;
		retval = nxo_nxoe_get(&marker->hook);
		if (retval != NULL)
			break;
		/* Fall through. */
	case 1:
		marker->iter++;
		retval = nxo_nxoe_get(&marker->buffer_nxo);
		break;
	default:
		retval = NULL;
	}
		
	return retval;
}

static cw_bool_t
marker_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
	struct cw_marker	*marker= (struct cw_marker *)a_data;

	bufm_delete(&marker->bufm);
	nxa_free(nx_nxa_get(a_nx), marker, sizeof(struct cw_marker));

	return FALSE;
}

/*
 * Verify that a_nxo is a =marker=.
 */
static cw_nxn_t
marker_p_type(cw_nxo_t *a_nxo)
{
	cw_nxn_t		retval;
	cw_nxo_t		*tag;
	cw_uint32_t		name_len;
	const cw_uint8_t	*name_marker;

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
	name_marker = nxo_name_str_get(tag);
	if ((name_len != strlen("marker")) || strncmp("marker", name_marker,
	    name_len)) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	retval = NXN_ZERO;
	RETURN:
	return retval;
}

static cw_bufw_t
marker_p_whence(cw_nxo_t *a_whence)
{
	cw_bufw_t		retval;
	cw_uint32_t		len;
	const cw_uint8_t	*str;

	/*
	 * a_whence is a name.  Determine whether it is /SEEK_SET, /SEEK_CUR, or
	 * /SEEK_END.
	 */
	len = nxo_name_len_get(a_whence);
	/* All the valid names are the same length, so a single length works. */
	if (len != strlen("SEEK_SET")) {
		retval = BUFW_NONE;
		goto RETURN;
	}

	str = nxo_name_str_get(a_whence);
	if (strncmp(str, "SEEK_CUR", len) == 0)
		retval = BUFW_REL;
	else if (strncmp(str, "SEEK_SET", len) == 0)
		retval = BUFW_BEG;
	else if (strncmp(str, "SEEK_END", len) == 0)
		retval = BUFW_END;
	else
		retval = BUFW_NONE;

	RETURN:
	return retval;
}

void
slate_marker(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*estack, *ostack, *tstack, *nxo, *tnxo, *tag;
	cw_nx_t			*nx;
	cw_nxn_t		error;
	struct cw_marker	*marker;
	struct cw_buffer	*buffer;

	estack = nxo_thread_estack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = buffer_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	buffer = (struct cw_buffer *)nxo_hook_data_get(nxo);

	marker = (struct cw_marker *)nxa_malloc(nx_nxa_get(nx),
	    sizeof(struct cw_marker));
	
	/*
	 * Create a reference to this operator in order to prevent the module
	 * from being prematurely unloaded.
	 */
	nxo_no_new(&marker->hook);
	nxo_dup(&marker->hook, nxo_stack_get(estack));

	nxo_no_new(&marker->buffer_nxo);
	nxo_dup(&marker->buffer_nxo, nxo);
	buf_lock(&buffer->buf);
	bufm_new(&marker->bufm, &buffer->buf, NULL);
	buf_unlock(&buffer->buf);

	/*
	 * Create a reference to the marker; keep a reference to the buf on
	 * tstack to avoid a GC race.
	 */
	tnxo = nxo_stack_push(tstack);
	nxo_dup(tnxo, nxo);
	nxo_hook_new(nxo, nx, marker, NULL, marker_p_ref_iter,
	    marker_p_delete);
	nxo_stack_pop(tstack);

	/* Set the hook tag. */
	tag = nxo_hook_tag_get(nxo);
	nxo_name_new(tag, nx, "marker", sizeof("marker") - 1, FALSE);
	nxo_attr_set(tag, NXOA_EXECUTABLE);
}

/* %=marker= marker_copy %=marker= */
void
slate_marker_copy(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*estack, *ostack, *tstack, *nxo, *tnxo, *tag;
	cw_nx_t			*nx;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker, *marker_copy;

	estack = nxo_thread_estack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	marker_copy = (struct cw_marker *)nxa_malloc(nx_nxa_get(nx),
	    sizeof(struct cw_marker));

	/*
	 * Create a reference to this operator in order to prevent the module
	 * from being prematurely unloaded.
	 */
	nxo_no_new(&marker_copy->hook);
	nxo_dup(&marker_copy->hook, nxo_stack_get(estack));

	nxo_no_new(&marker_copy->buffer_nxo);
	nxo_dup(&marker_copy->buffer_nxo, &marker->buffer_nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	bufm_new(&marker_copy->bufm, buf, NULL);
	bufm_dup(&marker_copy->bufm, &marker->bufm);
	buf_unlock(buf);

	/*
	 * Create a reference to the new marker; keep a reference to the
	 * original marker on tstack to avoid a GC race.
	 */
	tnxo = nxo_stack_push(tstack);
	nxo_dup(tnxo, nxo);
	nxo_hook_new(nxo, nx, marker_copy, NULL, marker_p_ref_iter,
	    marker_p_delete);
	nxo_stack_pop(tstack);

	/* Set the hook tag. */
	tag = nxo_hook_tag_get(nxo);
	nxo_name_new(tag, nx, "marker", sizeof("marker") - 1, FALSE);
	nxo_attr_set(tag, NXOA_EXECUTABLE);
}

/* %=marker= marker_buffer %=buffer= */
void
slate_marker_buffer(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_marker	*marker;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	nxo_dup(nxo, &marker->buffer_nxo);
}

void
slate_marker_line(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	nxo_integer_new(nxo, bufm_line(&marker->bufm));
	buf_unlock(buf);
}

/* %marker %offset %whence? marker_seekline %pos */
void
slate_marker_seekline(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_uint32_t		npop;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;
	cw_sint64_t		offset, pos;
	cw_bufw_t		whence;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo, ostack, a_thread);
	/* The whence argument is optional. */
	switch (nxo_type_get(nxo)) {
	case NXOT_NAME:
		whence = marker_p_whence(nxo);
		NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
		npop = 2;
		break;
	case NXOT_INTEGER:
		whence = BUFW_REL;
		npop = 1;
		break;
	default:
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}

	/* offset. */
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}
	offset = nxo_integer_get(nxo);

	/* marker. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker = (struct cw_marker *)nxo_hook_data_get(nxo);
	if (whence == BUFW_NONE) {
		nxo_thread_nerror(a_thread, NXN_limitcheck);
		return;
	}

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	pos = bufm_line_seek(&marker->bufm, offset, whence);
	buf_unlock(buf);

	nxo_integer_new(nxo, (cw_nxoi_t)pos);

	nxo_stack_npop(ostack, npop);
}

/* %marker marker_position %pos */
void
slate_marker_position(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	nxo_integer_new(nxo, (cw_nxoi_t)bufm_pos(&marker->bufm));
	buf_unlock(buf);
}

/* %=marker= %offset %whence/%=marker=? marker_seek %pos */
void
slate_marker_seek(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_uint32_t		npop;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker, *whence_marker = NULL;
	cw_sint64_t		offset, pos;
	cw_bufw_t		whence;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo, ostack, a_thread);
	/* The whence argument is optional. */
	switch (nxo_type_get(nxo)) {
	case NXOT_HOOK:
		error = marker_p_type(nxo);
		if (error) {
			nxo_thread_nerror(a_thread, error);
			return;
		}
		whence_marker = (struct cw_marker *)nxo_hook_data_get(nxo);

		whence = BUFW_REL;
		NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
		npop = 2;
		break;
	case NXOT_NAME:
		whence = marker_p_whence(nxo);
		NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
		npop = 2;
		break;
	case NXOT_INTEGER:
		whence = BUFW_REL;
		npop = 1;
		break;
	default:
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}

	/* offset. */
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}
	offset = nxo_integer_get(nxo);

	/* marker. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker = (struct cw_marker *)nxo_hook_data_get(nxo);
	if (whence == BUFW_NONE) {
		nxo_thread_nerror(a_thread, NXN_limitcheck);
		return;
	}

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	if (whence_marker != NULL) {
		/*
		 * Move to the location of the whence marker before doing a
		 * relative seek.
		 */
		bufm_dup(&marker->bufm, &whence_marker->bufm);
	}
	pos = bufm_seek(&marker->bufm, offset, whence);
	buf_unlock(buf);

	nxo_integer_new(nxo, (cw_nxoi_t)pos);

	nxo_stack_npop(ostack, npop);
}

/* %=marker= marker_before_get %c */
void
slate_marker_before_get(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;
	cw_uint8_t		*bp;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	bp = bufm_before_get(&marker->bufm);
	if (bp == NULL) {
		buf_unlock(buf);
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	}
	nxo_integer_new(nxo, (cw_nxoi_t)*bp);
	buf_unlock(buf);
}

/* %=marker= marker_after_get %c */
void
slate_marker_after_get(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;
	cw_uint8_t		*bp;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}

	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	bp = bufm_after_get(&marker->bufm);
	if (bp == NULL) {
		buf_unlock(buf);
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	}
	nxo_integer_new(nxo, (cw_nxoi_t)*bp);
	buf_unlock(buf);
}

/* %=marker= %val marker_before_set - */
void
slate_marker_before_set(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;
	cw_uint8_t		*bp, c;

	ostack = nxo_thread_ostack_get(a_thread);

	/* val. */
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}
	c = (cw_uint8_t)nxo_integer_get(nxo);

	/* marker. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	bp = bufm_before_get(&marker->bufm);
	if (bp == NULL) {
		buf_unlock(buf);
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	}
	*bp = c;
	buf_unlock(buf);

	nxo_stack_npop(ostack, 2);
}

/* %=marker= %val marker_after_set - */
void
slate_marker_after_set(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;
	cw_uint8_t		*bp, c;

	ostack = nxo_thread_ostack_get(a_thread);

	/* val. */
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}
	c = (cw_uint8_t)nxo_integer_get(nxo);

	/* marker. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	bp = bufm_after_get(&marker->bufm);
	if (bp == NULL) {
		buf_unlock(buf);
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	}
	*bp = c;
	buf_unlock(buf);

	nxo_stack_npop(ostack, 2);
}

/* %=marker= %str marker_before_insert - */
void
slate_marker_before_insert(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;
	cw_uint8_t		*str;
	cw_uint32_t		str_len;

	ostack = nxo_thread_ostack_get(a_thread);

	/* str. */
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_STRING) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}
	str = nxo_string_get(nxo);
	str_len = nxo_string_len_get(nxo);

	/* marker. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	bufm_before_insert(&marker->bufm, str, str_len);
	buf_unlock(buf);

	nxo_stack_npop(ostack, 2);
}

/* %=marker= %str marker_after_insert - */
void
slate_marker_after_insert(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker;
	cw_uint8_t		*str;
	cw_uint32_t		str_len;

	ostack = nxo_thread_ostack_get(a_thread);

	/* str. */
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_STRING) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}
	str = nxo_string_get(nxo);
	str_len = nxo_string_len_get(nxo);

	/* marker. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker->bufm);
	buf_lock(buf);
	bufm_after_insert(&marker->bufm, str, str_len);
	buf_unlock(buf);

	nxo_stack_npop(ostack, 2);
}

/* %=marker= %=marker= marker_range_get %string */
void
slate_marker_range_get(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker_a, *marker_b;
	cw_uint8_t		*str, *ostr;
	cw_uint64_t		pos_a, pos_b, str_len;
	cw_uint32_t		elmsize;

	ostack = nxo_thread_ostack_get(a_thread);

	/* marker_b. */
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker_b = (struct cw_marker *)nxo_hook_data_get(nxo);

	/* marker_a. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker_a = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker_a->bufm);
	if (buf != bufm_buf(&marker_b->bufm)) {
		/* XXX Throw /argcheck or something similar. */
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}

	buf_lock(buf);

	/* Get a pointer to the buffer range and calculate its length. */
	str = bufm_range_get(&marker_a->bufm, &marker_b->bufm);
	pos_a = bufm_pos(&marker_a->bufm);
	pos_b = bufm_pos(&marker_b->bufm);
	str_len = (pos_a < pos_b) ? pos_b - pos_a : pos_a - pos_b;

	/*
	 * Create an Onyx string to store the result.  Since there are two
	 * markers on the stack that have references to the buffer, it is safe
	 * to trash one of them here.
	 */
	nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), str_len);

	elmsize = buf_elmsize_get(buf);
	ostr = nxo_string_get(nxo);
	if (elmsize == 1) {
		/* Use memcpy() to copy the buffer contents to the string. */
		memcpy(ostr, str, str_len);
	} else {
		cw_uint32_t	i;

		/*
		 * Iteratively copy bytes from the buffer into the string.  It
		 * is not safe to memcpy(), since elmsize is not 1.
		 */
		ostr = nxo_string_get(nxo);
		for (i = 0; i < str_len; i++)
			ostr[i] = str[i * elmsize];
	}

	buf_unlock(buf);

	nxo_stack_pop(ostack);
}

/* %=marker= %=marker= marker_range_get %string */
void
slate_marker_range_cut(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	cw_buf_t		*buf;
	struct cw_marker	*marker_a, *marker_b;
	cw_uint8_t		*str, *ostr;
	cw_uint64_t		pos_a, pos_b, str_len;
	cw_uint32_t		elmsize;

	ostack = nxo_thread_ostack_get(a_thread);

	/* marker_b. */
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker_b = (struct cw_marker *)nxo_hook_data_get(nxo);

	/* marker_a. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	error = marker_p_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	marker_a = (struct cw_marker *)nxo_hook_data_get(nxo);

	buf = bufm_buf(&marker_a->bufm);
	if (buf != bufm_buf(&marker_b->bufm)) {
		/* XXX Throw /argcheck or something similar. */
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}

	buf_lock(buf);

	/* Get a pointer to the buffer range and calculate its length. */
	str = bufm_range_get(&marker_a->bufm, &marker_b->bufm);
	pos_a = bufm_pos(&marker_a->bufm);
	pos_b = bufm_pos(&marker_b->bufm);
	str_len = (pos_a < pos_b) ? pos_b - pos_a : pos_a - pos_b;

	/*
	 * Create an Onyx string to store the result.  Since there are two
	 * markers on the stack that have references to the buffer, it is safe
	 * to trash one of them here.
	 */
	nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), str_len);

	elmsize = buf_elmsize_get(buf);
	ostr = nxo_string_get(nxo);
	if (elmsize == 1) {
		/* Use memcpy() to copy the buffer contents to the string. */
		memcpy(ostr, str, str_len);
	} else {
		cw_uint32_t	i;

		/*
		 * Iteratively copy bytes from the buffer into the string.  It
		 * is not safe to memcpy(), since elmsize is not 1.
		 */
		ostr = nxo_string_get(nxo);
		for (i = 0; i < str_len; i++)
			ostr[i] = str[i * elmsize];
	}

	/* Remove the buffer range. */
	bufm_remove(&marker_a->bufm, &marker_b->bufm);

	buf_unlock(buf);

	nxo_stack_pop(ostack);
}
