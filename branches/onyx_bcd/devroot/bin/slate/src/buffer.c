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
 * This file contains the implementations of all the glue hooks for =buffer=
 * and =marker=.
 *
 ******************************************************************************/

#include "../include/slate.h"

struct cw_buffer
{
    cw_buf_t buf;

    /* Protects all buf operations. */
    cw_mtx_t mtx;

    /* Auxiliary data for buffer_{set}aux. */
    cw_nxo_t aux;

    /* Sequence number. */
    cw_nxoi_t seq;
};

/* This structure is necessary since =marker=s need to report a reference to
 * their associated =buffer=s. */
struct cw_marker
{
    /* For reference iteration. */
    cw_uint32_t iter;

    cw_nxo_t buffer_nxo;
    cw_bufm_t bufm;

    /* Sequence number. */
    cw_nxoi_t seq;
};

static const struct cw_slate_entry slate_buffer_ops[] = {
    SLATE_ENTRY(buffer),
    SLATE_ENTRY(buffer_aux),
    SLATE_ENTRY(buffer_setaux),
    SLATE_ENTRY(buffer_length),
    SLATE_ENTRY(buffer_lines),
    SLATE_ENTRY(buffer_undoable),
    SLATE_ENTRY(buffer_redoable),
    SLATE_ENTRY(buffer_undo),
    SLATE_ENTRY(buffer_redo),
    SLATE_ENTRY(buffer_history_active),
    SLATE_ENTRY(buffer_history_setactive),
    SLATE_ENTRY(buffer_history_startgroup),
    SLATE_ENTRY(buffer_history_endgroup),
    SLATE_ENTRY(buffer_history_flush),
    SLATE_ENTRY(marker),
    SLATE_ENTRY(marker_copy),
    SLATE_ENTRY(marker_buffer),
    SLATE_ENTRY(marker_line),
    SLATE_ENTRY(marker_seekline),
    SLATE_ENTRY(marker_position),
    SLATE_ENTRY(marker_seek),
    SLATE_ENTRY(marker_before_get),
    SLATE_ENTRY(marker_after_get),
    SLATE_ENTRY(marker_before_set),
    SLATE_ENTRY(marker_after_set),
    SLATE_ENTRY(marker_before_insert),
    SLATE_ENTRY(marker_after_insert),
    SLATE_ENTRY(marker_range_get),
    SLATE_ENTRY(marker_range_cut)
};

static void
buffer_p_eval(void *a_data, cw_nxo_t *a_thread);

static cw_nxoe_t *
buffer_p_ref_iter(void *a_data, cw_bool_t a_reset);

static cw_bool_t
buffer_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter);

static cw_nxoe_t *
marker_p_ref_iter(void *a_data, cw_bool_t a_reset);

static cw_bool_t
marker_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter);

static cw_bufw_t
marker_p_whence(cw_nxo_t *a_whence);

void
slate_buffer_init(cw_nxo_t *a_thread)
{
    slate_ops(a_thread, slate_buffer_ops,
	      (sizeof(slate_buffer_ops) / sizeof(struct cw_slate_entry)));
}

CW_INLINE void
buffer_p_lock(struct cw_buffer *a_buffer)
{
    mtx_lock(&a_buffer->mtx);
}

CW_INLINE void
buffer_p_unlock(struct cw_buffer *a_buffer)
{
    mtx_unlock(&a_buffer->mtx);
}

static void
buffer_p_eval(void *a_data, cw_nxo_t *a_thread)
{
    /* Since there is risk of deadlock if we recursively invoke the Onyx
     * interpreter with the buffer lock held, instead malloc a copy of the
     * buffer and feed it to the interpreter.  Catch all exceptions to
     * deallocate the copy in case of an error. */
    cw_error("XXX Not implemented");
}

static cw_nxoe_t *
buffer_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_buffer *buffer = (struct cw_buffer *) a_data;

    if (a_reset)
    {
	retval = nxo_nxoe_get(&buffer->aux);
    }
    else
    {
	retval = NULL;
    }

    return retval;
}

static cw_bool_t
buffer_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    struct cw_buffer *buffer = (struct cw_buffer *) a_data;

    /* Don't delete until the second GC sweep iteration, so that associated
     * bufm's can be deleted first. */
    if (a_iter != 1)
    {
	retval = TRUE;
	goto RETURN;
    }

    mtx_delete(&buffer->mtx);
    buf_delete(&buffer->buf);
    nxa_free(nx_nxa_get(a_nx), buffer, sizeof(struct cw_buffer));

    retval = FALSE;
    RETURN:
    return retval;
}

/*
 * Verify that a_nxo is a =buffer=.
 */
cw_nxn_t
buffer_type(cw_nxo_t *a_nxo)
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
    if ((name_len != strlen("buffer")) || strncmp("buffer", name, name_len))
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
slate_buffer(cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *nxo, *tag;
    cw_nx_t *nx;
    struct cw_buffer *buffer;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);

    buffer = (struct cw_buffer *) nxa_malloc(nx_nxa_get(nx),
					     sizeof(struct cw_buffer));

    /* Initialize the buf. */
    buf_new(&buffer->buf, (cw_opaque_alloc_t *) nxa_malloc_e,
	    (cw_opaque_realloc_t *) nxa_realloc_e,
	    (cw_opaque_dealloc_t *) nxa_free_e, (void *) nx_nxa_get(nx));

    /* Initialize the protection mutex; buf's aren't thread-safe. */
    mtx_new(&buffer->mtx);

    /* Create a reference to the buffer. */
    nxo = nxo_stack_push(ostack);
    nxo_hook_new(nxo, nx, buffer, buffer_p_eval, buffer_p_ref_iter,
		 buffer_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(nxo);
    nxo_name_new(tag, nx, "buffer", sizeof("buffer") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Initialize aux. */
    nxo_null_new(&buffer->aux);

    /* Initialize the sequence number. */
    buffer->seq = 0;
}

void
slate_buffer_aux(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    /* Avoid a GC race by using tnxo to store a reachable ref to the buffer. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_dup(nxo, &buffer->aux);
    nxo_stack_pop(tstack);
}

void
slate_buffer_setaux(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *aux;
    cw_nxn_t error;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(aux, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, aux);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    nxo_dup(&buffer->aux, aux);
    nxo_stack_npop(ostack, 2);
}


void
slate_buffer_seq(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_nxoi_t seq;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    seq = buffer->seq;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, seq);
}

void
slate_buffer_length(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_nxoi_t length;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    length = buf_len(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, length);
}

void
slate_buffer_lines(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_nxoi_t lines;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    lines = buf_nlines(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, lines);
}

void
slate_buffer_undoable(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_bool_t undoable;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    undoable = buf_undoable(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, undoable);
}

void
slate_buffer_redoable(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_bool_t redoable;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    redoable = buf_redoable(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, redoable);
}

/* %=marker= %count? buffer_undo %error */
void
slate_buffer_undo(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_nxoi_t nundo;
    cw_bool_t pop, result;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) == NXOT_INTEGER)
    {
	pop = TRUE;
	nundo = nxo_integer_get(nxo);
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    }
    else
    {
	pop = FALSE;
	nundo = 1;
    }
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    result = buf_undo(&buffer->buf, &marker->bufm, nundo);
    buffer->seq++;
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, result);

    if (pop)
    {
	nxo_stack_pop(ostack);
    }
}

/* %=marker= %count? buffer_redo %error */
void
slate_buffer_redo(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_nxoi_t nredo;
    cw_bool_t pop, result;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) == NXOT_INTEGER)
    {
	pop = TRUE;
	nredo = nxo_integer_get(nxo);
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    }
    else
    {
	pop = FALSE;
	nredo = 1;
    }
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    result = buf_redo(&buffer->buf, &marker->bufm, nredo);
    buffer->seq++;
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, result);

    if (pop)
    {
	nxo_stack_pop(ostack);
    }
}

void
slate_buffer_history_active(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_bool_t history_active;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    history_active = buf_hist_active_get(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, history_active);
}

void
slate_buffer_history_setactive(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_bool_t active;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_BOOLEAN)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    active = nxo_boolean_get(nxo);

    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    buf_hist_active_set(&buffer->buf, active);
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

void
slate_buffer_history_startgroup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    struct cw_marker *marker;
    cw_uint32_t npop;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	/* No marker specified. */
	marker = NULL;
	npop = 1;
    }
    else
    {
	marker = (struct cw_marker *) nxo_hook_data_get(nxo);
	
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	npop = 2;
    }

    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    if (marker != NULL)
    {
	buf_hist_group_beg(&buffer->buf, &marker->bufm);
    }
    else
    {
	buf_hist_group_beg(&buffer->buf, NULL);
    }
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, npop);
}

void
slate_buffer_history_endgroup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    buf_hist_group_end(&buffer->buf);
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_pop(ostack);
}

void
slate_buffer_history_flush(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);

    buffer_p_lock(buffer);
    buf_hist_flush(&buffer->buf);
    buffer->seq++;
    buffer_p_unlock(buffer);
}

static cw_nxoe_t *
marker_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_marker *marker = (struct cw_marker *) a_data;

    if (a_reset)
    {
	retval = nxo_nxoe_get(&marker->buffer_nxo);
    }
    else
    {
	retval = NULL;
    }

    return retval;
}

static cw_bool_t
marker_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
    struct cw_marker *marker;
    struct cw_buffer *buffer;

    marker = (struct cw_marker *) a_data;
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    bufm_delete(&marker->bufm);
    buffer_p_unlock(buffer);
    nxa_free(nx_nxa_get(a_nx), marker, sizeof(struct cw_marker));

    return FALSE;
}

/*
 * Verify that a_nxo is a =marker=.
 */
cw_nxn_t
marker_type(cw_nxo_t *a_nxo)
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
    if ((name_len != strlen("marker")) || strncmp("marker", name, name_len))
    {
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
    cw_bufw_t retval;
    cw_uint32_t len;
    const cw_uint8_t *str;

    /* a_whence is a name.  Determine whether it is /SEEK_SET, /SEEK_CUR, or
     * /SEEK_END. */
    len = nxo_name_len_get(a_whence);
    /* All the valid names are the same length, so a single length works. */
    if (len != strlen("SEEK_SET"))
    {
	retval = BUFW_NONE;
	goto RETURN;
    }

    str = nxo_name_str_get(a_whence);
    if (strncmp(str, "SEEK_CUR", len) == 0)
    {
	retval = BUFW_REL;
    }
    else if (strncmp(str, "SEEK_SET", len) == 0)
    {
	retval = BUFW_BEG;
    }
    else if (strncmp(str, "SEEK_END", len) == 0)
    {
	retval = BUFW_END;
    }
    else
    {
	retval = BUFW_NONE;
    }

    RETURN:
    return retval;
}

void
slate_marker(cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *nxo, *tnxo, *tag;
    cw_nx_t *nx;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = buffer_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_hook_data_get(nxo);
    marker = (struct cw_marker *) nxa_malloc(nx_nxa_get(nx),
					     sizeof(struct cw_marker));
	
    nxo_no_new(&marker->buffer_nxo);
    nxo_dup(&marker->buffer_nxo, nxo);
    buffer_p_lock(buffer);
    bufm_new(&marker->bufm, &buffer->buf);
    buffer_p_unlock(buffer);

    /* Create a reference to the marker; keep a reference to the buf on tstack
     * to avoid a GC race. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_hook_new(nxo, nx, marker, NULL, marker_p_ref_iter, marker_p_delete);
    nxo_stack_pop(tstack);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(nxo);
    nxo_name_new(tag, nx, "marker", sizeof("marker") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Initialize the sequence number. */
    marker->seq = 0;
}

void
slate_marker_seq(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_nxoi_t seq;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    seq = marker->seq;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, seq);
}

/* %=marker= marker_copy %=marker= */
void
slate_marker_copy(cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *nxo, *tnxo, *tag;
    cw_nx_t *nx;
    cw_nxn_t error;
    struct cw_marker *marker, *marker_copy;
    struct cw_buffer *buffer;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    marker_copy = (struct cw_marker *) nxa_malloc(nx_nxa_get(nx),
						  sizeof(struct cw_marker));

    nxo_no_new(&marker_copy->buffer_nxo);
    nxo_dup(&marker_copy->buffer_nxo, &marker->buffer_nxo);

    buffer_p_lock(buffer);
    bufm_new(&marker_copy->bufm, &buffer->buf);
    bufm_dup(&marker_copy->bufm, &marker->bufm);
    marker_copy->seq++;
    buffer_p_unlock(buffer);

    /* Create a reference to the new marker; keep a reference to the original
     * marker on tstack to avoid a GC race. */
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
slate_marker_buffer(cw_nxo_t *a_thread)
{
    cw_nxo_t*ostack, *tstack, *nxo, *tnxo;
    cw_nxn_t error;
    struct cw_marker *marker;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);

    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_dup(nxo, &marker->buffer_nxo);
    nxo_stack_pop(tstack);
}

void
slate_marker_line(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_nxoi_t line;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    line = bufm_line(&marker->bufm);
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, line);
}

/* %marker %offset %whence? marker_seekline %pos */
void
slate_marker_seekline(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_uint32_t npop;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_sint64_t offset, pos;
    cw_bufw_t whence;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    /* The whence argument is optional. */
    switch (nxo_type_get(nxo))
    {
	case NXOT_NAME:
	{
	    whence = marker_p_whence(nxo);
	    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	    npop = 2;
	    break;
	}
	case NXOT_INTEGER:
	{
	    whence = BUFW_REL;
	    npop = 1;
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    /* offset. */
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    offset = nxo_integer_get(nxo);

    /* marker. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);
    if (whence == BUFW_NONE)
    {
	nxo_thread_nerror(a_thread, NXN_limitcheck);
	return;
    }

    buffer_p_lock(buffer);
    pos = bufm_line_seek(&marker->bufm, offset, whence);
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, (cw_nxoi_t)pos);

    nxo_stack_npop(ostack, npop);
}

/* %marker marker_position %pos */
void
slate_marker_position(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_nxoi_t position;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    position = (cw_nxoi_t)bufm_pos(&marker->bufm);
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, position);
}

/* %=marker= %offset %whence/%=marker=? marker_seek %pos */
void
slate_marker_seek(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_uint32_t npop;
    cw_nxn_t error;
    struct cw_marker *marker, *whence_marker = NULL;
    struct cw_buffer *buffer;
    cw_sint64_t offset, pos;
    cw_bufw_t whence;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    /* The whence argument is optional. */
    switch (nxo_type_get(nxo))
    {
	case NXOT_HOOK:
	{
	    error = marker_type(nxo);
	    if (error)
	    {
		nxo_thread_nerror(a_thread, error);
		return;
	    }
	    whence_marker = (struct cw_marker *) nxo_hook_data_get(nxo);

	    whence = BUFW_REL;
	    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	    npop = 2;
	    break;
	}
	case NXOT_NAME:
	{
	    whence = marker_p_whence(nxo);
	    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	    npop = 2;
	    break;
	}
	case NXOT_INTEGER:
	{
	    whence = BUFW_REL;
	    npop = 1;
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    /* offset. */
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    offset = nxo_integer_get(nxo);

    /* marker. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);
    if (whence == BUFW_NONE)
    {
	nxo_thread_nerror(a_thread, NXN_limitcheck);
	return;
    }

    buffer_p_lock(buffer);
    if (whence_marker != NULL)
    {
	/* Move to the location of the whence marker before doing a relative
	 * seek. */
	bufm_dup(&marker->bufm, &whence_marker->bufm);
    }
    pos = bufm_seek(&marker->bufm, offset, whence);
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, (cw_nxoi_t)pos);

    nxo_stack_npop(ostack, npop);
}

/* %=marker= marker_before_get %c */
void
slate_marker_before_get(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *bp, c;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    bp = bufm_before_get(&marker->bufm);
    if (bp == NULL)
    {
	buffer_p_unlock(buffer);
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    c = *bp;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, (cw_nxoi_t)c);
}

/* %=marker= marker_after_get %c */
void
slate_marker_after_get(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *bp, c;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    bp = bufm_after_get(&marker->bufm);
    if (bp == NULL)
    {
	buffer_p_unlock(buffer);
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    c = *bp;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, (cw_nxoi_t)c);
}

/* %=marker= %val marker_before_set - */
void
slate_marker_before_set(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *bp, c;

    ostack = nxo_thread_ostack_get(a_thread);

    /* val. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    c = (cw_uint8_t)nxo_integer_get(nxo);

    /* marker. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    bp = bufm_before_get(&marker->bufm);
    if (bp == NULL)
    {
	buffer_p_unlock(buffer);
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    *bp = c;
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

/* %=marker= %val marker_after_set - */
void
slate_marker_after_set(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *bp, c;

    ostack = nxo_thread_ostack_get(a_thread);

    /* val. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    c = (cw_uint8_t)nxo_integer_get(nxo);

    /* marker. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    buffer_p_lock(buffer);
    bp = bufm_after_get(&marker->bufm);
    if (bp == NULL)
    {
	buffer_p_unlock(buffer);
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    *bp = c;
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

/* %=marker= %str marker_before_insert - */
void
slate_marker_before_insert(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *str;
    cw_uint32_t str_len;
    cw_bufv_t bufv;

    ostack = nxo_thread_ostack_get(a_thread);

    /* str. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    str = nxo_string_get(nxo);
    str_len = nxo_string_len_get(nxo);

    /* marker. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    bufv.data = str;
    bufv.len = str_len;

    buffer_p_lock(buffer);
    bufm_before_insert(&marker->bufm, &bufv, 1, 1);
    buffer->seq++;
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

/* %=marker= %str marker_after_insert - */
void
slate_marker_after_insert(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *str;
    cw_uint32_t str_len;
    cw_bufv_t bufv;

    ostack = nxo_thread_ostack_get(a_thread);

    /* str. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    str = nxo_string_get(nxo);
    str_len = nxo_string_len_get(nxo);

    /* marker. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker->buffer_nxo);

    bufv.data = str;
    bufv.len = str_len;

    buffer_p_lock(buffer);
    bufm_after_insert(&marker->bufm, &bufv, 1, 1);
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

/* %=marker= %=marker= marker_range_get %string */
void
slate_marker_range_get(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker_a, *marker_b;
    struct cw_buffer *buffer;
    cw_uint64_t pos_a, pos_b, str_len;
    cw_bufv_t *bufv, sbufv;
    cw_uint32_t bufvcnt;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker_b. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker_b = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker_b->buffer_nxo);

    /* marker_a. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker_a = (struct cw_marker *) nxo_hook_data_get(nxo);

    if (buffer != (struct cw_buffer *) nxo_hook_data_get(&marker_b->buffer_nxo))
    {
	nxo_thread_serror(a_thread, "argcheck", strlen("argcheck"));
	return;
    }

    buffer_p_lock(buffer);

    /* Get a pointer to the buffer range and calculate its length. */
    bufv = bufm_range_get(&marker_a->bufm, &marker_b->bufm, &bufvcnt);
    pos_a = bufm_pos(&marker_a->bufm);
    pos_b = bufm_pos(&marker_b->bufm);
    str_len = (pos_a < pos_b) ? pos_b - pos_a : pos_a - pos_b;

    /* Create an Onyx string to store the result.  Since there are two markers
     * on the stack that have references to the buffer, it is safe to trash one
     * of them here. */
    nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
		   nxo_thread_currentlocking(a_thread), str_len);

    sbufv.data = nxo_string_get(nxo);
    sbufv.len = str_len;
    bufv_copy(&sbufv, 1, 1, bufv, bufvcnt, buf_elmsize_get(&buffer->buf), 0);

    buffer_p_unlock(buffer);

    nxo_stack_pop(ostack);
}

/* %=marker= %=marker= marker_range_get %string */
void
slate_marker_range_cut(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker_a, *marker_b;
    struct cw_buffer *buffer;
    cw_uint64_t pos_a, pos_b, str_len;
    cw_bufv_t *bufv, sbufv;
    cw_uint32_t bufvcnt;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker_b. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker_b = (struct cw_marker *) nxo_hook_data_get(nxo);
    buffer = (struct cw_buffer *) nxo_hook_data_get(&marker_b->buffer_nxo);

    /* marker_a. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    error = marker_type(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker_a = (struct cw_marker *) nxo_hook_data_get(nxo);

    if (buffer != (struct cw_buffer *) nxo_hook_data_get(&marker_b->buffer_nxo))
    {
	nxo_thread_serror(a_thread, "argcheck", strlen("argcheck"));
	return;
    }

    buffer_p_lock(buffer);

    /* Get a pointer to the buffer range and calculate its length. */
    bufv = bufm_range_get(&marker_a->bufm, &marker_b->bufm, &bufvcnt);
    pos_a = bufm_pos(&marker_a->bufm);
    pos_b = bufm_pos(&marker_b->bufm);
    str_len = (pos_a < pos_b) ? pos_b - pos_a : pos_a - pos_b;

    /* Create an Onyx string to store the result.  Since there are two markers
     * on the stack that have references to the buffer, it is safe to trash one
     * of them here. */
    nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
		   nxo_thread_currentlocking(a_thread), str_len);

    sbufv.data = nxo_string_get(nxo);
    sbufv.len = str_len;
    bufv_copy(&sbufv, 1, 1, bufv, bufvcnt, buf_elmsize_get(&buffer->buf), 0);

    /* Remove the buffer range. */
    bufm_remove(&marker_a->bufm, &marker_b->bufm);

    buffer->seq++;
    marker_a->seq++;
    marker_b->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_pop(ostack);
}