/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Slate <Version = slate>
 *
 * This file contains the implementations of all the glue handles for =buffer=
 * and =marker=.
 *
 ******************************************************************************/

#include "../include/modslate.h"

/* These are used to assure that methods are actually passed the correct
 * classes.  They are not reported to the GC, so can not safely be used for any
 * other purpose. */
static cw_nxo_t s_buffer;
static cw_nxo_t s_marker;
static cw_nxo_t s_extent;

struct cw_buffer
{
    cw_buf_t buf;

    /* Protects all buf operations. */
    cw_mtx_t mtx;

    /* Sequence number. */
    cw_nxoi_t seq;
};

/* This structure is necessary since markers need to report a reference to their
 * associated buffers. */
struct cw_marker
{
    cw_nxo_t buffer_instance;
    cw_nxo_t buffer_handle;
    cw_mkr_t mkr;

    /* Sequence number. */
    cw_nxoi_t seq;
};

/* This structure is necessary since extents need to report a reference to their
 * associated buffers. */
struct cw_extent
{
    cw_nxo_t buffer_instance;
    cw_nxo_t buffer_handle;
    cw_ext_t ext;

    /* Sequence number. */
    cw_nxoi_t seq;
};

static const struct cw_modslate_method modslate_buffer_methods[] =
{
    MODSLATE_METHOD(buffer, buffer),
    MODSLATE_METHOD(buffer, length),
    MODSLATE_METHOD(buffer, lines),
    MODSLATE_METHOD(buffer, extents),
    MODSLATE_METHOD(buffer, undoable),
    MODSLATE_METHOD(buffer, redoable),
    MODSLATE_METHOD(buffer, undo),
    MODSLATE_METHOD(buffer, redo),
    MODSLATE_METHOD(buffer, history_active),
    MODSLATE_METHOD(buffer, history_setactive),
    MODSLATE_METHOD(buffer, history_startgroup),
    MODSLATE_METHOD(buffer, history_endgroup),
    MODSLATE_METHOD(buffer, history_flush)
#ifdef CW_BUF_DUMP
    ,
    MODSLATE_METHOD(buffer, dump)
#endif
#ifdef CW_BUF_VALIDATE
    ,
    MODSLATE_METHOD(buffer, validate)
#endif
};

static const struct cw_modslate_method modslate_marker_methods[] =
{
    MODSLATE_METHOD(marker, marker),
    MODSLATE_METHOD(marker, copy),
    MODSLATE_METHOD(marker, buffer),
    MODSLATE_METHOD(marker, line),
    MODSLATE_METHOD(marker, seekline),
    MODSLATE_METHOD(marker, position),
    MODSLATE_METHOD(marker, seek),
    MODSLATE_METHOD(marker, before_get),
    MODSLATE_METHOD(marker, after_get),
    MODSLATE_METHOD(marker, before_set),
    MODSLATE_METHOD(marker, after_set),
    MODSLATE_METHOD(marker, before_insert),
    MODSLATE_METHOD(marker, after_insert),
    MODSLATE_METHOD(marker, range_get),
    MODSLATE_METHOD(marker, range_cut)
#ifdef CW_BUF_DUMP
    ,
    MODSLATE_METHOD(marker, dump)
#endif
#ifdef CW_BUF_VALIDATE
    ,
    MODSLATE_METHOD(marker, validate)
#endif
};

static const struct cw_modslate_method modslate_extent_methods[] =
{
    MODSLATE_METHOD(extent, extent),
    MODSLATE_METHOD(extent, copy),
    MODSLATE_METHOD(extent, buffer),
    MODSLATE_METHOD(extent, beg_get),
    MODSLATE_METHOD(extent, beg_set),
    MODSLATE_METHOD(extent, end_get),
    MODSLATE_METHOD(extent, end_set),
    MODSLATE_METHOD(extent, beg_open_get),
    MODSLATE_METHOD(extent, beg_open_set),
    MODSLATE_METHOD(extent, end_open_get),
    MODSLATE_METHOD(extent, end_open_set),
    MODSLATE_METHOD(extent, before_get),
    MODSLATE_METHOD(extent, at_get),
    MODSLATE_METHOD(extent, after_get),
    MODSLATE_METHOD(extent, prev_get),
    MODSLATE_METHOD(extent, next_get),
    MODSLATE_METHOD(extent, detachable_get),
    MODSLATE_METHOD(extent, detachable_set),
    MODSLATE_METHOD(extent, detached_get),
    MODSLATE_METHOD(extent, detach)
#ifdef CW_BUF_DUMP
    ,
    MODSLATE_METHOD(extent, dump)
#endif
#ifdef CW_BUF_VALIDATE
    ,
    MODSLATE_METHOD(extent, validate)
#endif
};

static cw_nxn_t
buffer_p_handle_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
		    cw_nxo_t *r_handle);
static cw_nxn_t
buffer_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	     struct cw_buffer **r_buffer);
static cw_bool_t
buffer_p_delete(void *a_data, cw_uint32_t a_iter);

static cw_nxn_t
marker_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	     struct cw_marker **r_marker);
static cw_nxoe_t *
marker_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
marker_p_delete(void *a_data, cw_uint32_t a_iter);
static cw_bufw_t
marker_p_whence(cw_nxo_t *a_whence);

#ifdef XXX_NOT_YET
static cw_nxoe_t *
extent_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
extent_p_delete(void *a_data, cw_uint32_t a_iter);
#endif /* XXX_NOT_YET. */

void
modslate_buffer_init(cw_nxo_t *a_thread)
{
    nxo_no_new(&s_buffer);
    modslate_class_init(a_thread, "buffer", modslate_buffer_methods,
			(sizeof(modslate_buffer_methods)
			 / sizeof(struct cw_modslate_method)),
			NULL, &s_buffer);

    nxo_no_new(&s_marker);
    modslate_class_init(a_thread, "marker", modslate_marker_methods,
			(sizeof(modslate_marker_methods)
			 / sizeof(struct cw_modslate_method)),
			NULL, &s_marker);

    nxo_no_new(&s_extent);
    modslate_class_init(a_thread, "extent", modslate_extent_methods,
			(sizeof(modslate_extent_methods)
			 / sizeof(struct cw_modslate_method)),
			NULL, &s_extent);
}

CW_P_INLINE void
buffer_p_lock(struct cw_buffer *a_buffer)
{
    mtx_lock(&a_buffer->mtx);
}

CW_P_INLINE void
buffer_p_unlock(struct cw_buffer *a_buffer)
{
    mtx_unlock(&a_buffer->mtx);
}

static cw_nxn_t
buffer_p_handle_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
		    cw_nxo_t *r_handle)
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

    if ((retval = modslate_instance_kind(a_instance, &s_buffer)))
    {
	goto RETURN;
    }

    data = nxo_instance_data_get(a_instance);
    if (nxo_type_get(data) != NXOT_DICT)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    nxo_name_new(name, "buffer", sizeof("buffer") - 1, FALSE);
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

    nxo_dup(r_handle, handle);
    retval = NXN_ZERO;
    RETURN:
    /* Clean up. */
    nxo_stack_npop(tstack, 2);

    return retval;
}

static cw_nxn_t
buffer_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	     struct cw_buffer **r_buffer)
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

    if ((retval = modslate_instance_kind(a_instance, &s_buffer)))
    {
	goto RETURN;
    }

    data = nxo_instance_data_get(a_instance);
    if (nxo_type_get(data) != NXOT_DICT)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    nxo_name_new(name, "buffer", sizeof("buffer") - 1, FALSE);
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

    *r_buffer = (struct cw_buffer *) nxo_handle_opaque_get(handle);
    retval = NXN_ZERO;
    RETURN:
    /* Clean up. */
    nxo_stack_npop(tstack, 2);

    return retval;
}

static cw_bool_t
buffer_p_delete(void *a_data, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    struct cw_buffer *buffer = (struct cw_buffer *) a_data;

    /* Don't delete until the appropriate GC sweep iteration, so that associated
     * markers can be deleted first. */
    if (a_iter != MODSLATE_GC_ITER_BUFFER)
    {
	retval = TRUE;
	goto RETURN;
    }

    mtx_delete(&buffer->mtx);
    buf_delete(&buffer->buf);
    nxa_free(buffer, sizeof(struct cw_buffer));

    retval = FALSE;
    RETURN:
    return retval;
}

cw_nxn_t
modslate_buffer_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread)
{
    struct cw_buffer *buffer;

    return buffer_p_get(a_instance, a_thread, &buffer);
}

/* #bsize #instance :buffer #instance */
void
modslate_buffer_buffer(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *bsize, *instance;
    cw_nxo_t *tag, *name, *handle;
    cw_uint32_t bufp_size;
    struct cw_buffer *buffer;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(instance, ostack, a_thread);
    NXO_STACK_DOWN_GET(bsize, ostack, a_thread, instance);
    if (nxo_type_get(bsize) != NXOT_INTEGER
	|| nxo_type_get(instance) != NXOT_INSTANCE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_integer_get(bsize) < 0 || nxo_integer_get(bsize) > UINT_MAX)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    bufp_size = nxo_integer_get(bsize);

    buffer = (struct cw_buffer *) nxa_malloc(sizeof(struct cw_buffer));

    /* Initialize the buf. */
    /* XXX Use cw_g_nxaa. */
    buf_new(&buffer->buf, bufp_size, (cw_opaque_alloc_t *) nxa_malloc_e,
	    (cw_opaque_realloc_t *) nxa_realloc_e,
	    (cw_opaque_dealloc_t *) nxa_free_e, NULL);

    /* Initialize the protection mutex; buf's aren't thread-safe. */
    mtx_new(&buffer->mtx);

    /* Initialize the sequence number. */
    buffer->seq = 0;

    /* Create a reference to the buffer, now that the internals are
     * initialized. */
    name = nxo_stack_push(tstack);
    handle = nxo_stack_push(tstack);

    nxo_name_new(name, "buffer", sizeof("buffer") - 1, FALSE);
    nxo_handle_new(handle, buffer, NULL, NULL, buffer_p_delete);

    /* Set the handle tag. */
    tag = nxo_handle_tag_get(handle);
    nxo_dup(tag, name);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Insert into the data dict. */
    nxo_dict_def(nxo_instance_data_get(instance), name, handle);

    /* Clean up. */
    nxo_stack_npop(tstack, 2);
    nxo_stack_remove(ostack, bsize);
}

/* #buffer :seq #seq */
void
modslate_buffer_seq(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_nxoi_t seq;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    seq = buffer->seq;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, seq);
}

/* #buffer :length #integer */
void
modslate_buffer_length(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_nxoi_t length;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    length = buf_len(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, length);
}

/* #buffer :lines #integer */
void
modslate_buffer_lines(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_nxoi_t lines;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    lines = buf_nlines(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, lines);
}

/* #buffer :extents #XXX */
void
modslate_buffer_extents(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

/* #buffer :undoable #boolean */
void
modslate_buffer_undoable(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_bool_t undoable;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    undoable = buf_undoable(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, undoable);
}

/* #buffer :redoable #boolean */
void
modslate_buffer_redoable(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_bool_t redoable;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    redoable = buf_redoable(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, redoable);
}

/* #count #marker #buffer :undo #error */
void
modslate_buffer_undo(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer, *mbuffer;
    cw_nxoi_t nundo;
    cw_bool_t result;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    mbuffer
	= (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);
    if (mbuffer != buffer)
    {
	nxo_thread_nerror(a_thread, NXN_argcheck);
	return;
    }

    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nundo = nxo_integer_get(nxo);

    buffer_p_lock(buffer);
    result = buf_undo(&buffer->buf, &marker->mkr, nundo);
    buffer->seq++;
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, result);

    nxo_stack_pop(ostack);
}

/* #count #marker #buffer :redo #error */
void
modslate_buffer_redo(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer, *mbuffer;
    cw_nxoi_t nredo;
    cw_bool_t result;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    mbuffer
	= (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);
    if (mbuffer != buffer)
    {
	nxo_thread_nerror(a_thread, NXN_argcheck);
	return;
    }

    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nredo = nxo_integer_get(nxo);

    buffer_p_lock(buffer);
    result = buf_redo(&buffer->buf, &marker->mkr, nredo);
    buffer->seq++;
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, result);

    nxo_stack_pop(ostack);
}

/* #buffer :history_active #boolean */
void
modslate_buffer_history_active(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_bool_t history_active;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    history_active = buf_hist_active_get(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_boolean_new(nxo, history_active);
}

/* #boolean #buffer :history_setactive - */
void
modslate_buffer_history_setactive(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;
    cw_bool_t active;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_BOOLEAN)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    active = nxo_boolean_get(nxo);

    buffer_p_lock(buffer);
    buf_hist_active_set(&buffer->buf, active);
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

/* #marker #buffer :history_startgroup - */
void
modslate_buffer_history_startgroup(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer, *mbuffer;
    struct cw_marker *marker;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    mbuffer
	= (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    if (buffer != mbuffer)
    {
	nxo_thread_nerror(a_thread, NXN_argcheck);
	return;
    }
    
    buffer_p_lock(buffer);
    buf_hist_group_beg(&buffer->buf, &marker->mkr);
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

/* #buffer :history_endgroup - */
void
modslate_buffer_history_endgroup(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    buf_hist_group_end(&buffer->buf);
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_pop(ostack);
}

void
modslate_buffer_history_flush(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    buf_hist_flush(&buffer->buf);
    buffer->seq++;
    buffer_p_unlock(buffer);
}

#ifdef CW_BUF_DUMP
/* `prefix' #buffer :dump - */
void
modslate_buffer_dump(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    cw_uint8_t *prefix;
    cw_nxn_t error;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_string_cstring(tnxo, nxo, a_thread);
    prefix = nxo_string_get(tnxo);

    buffer_p_lock(buffer);
    buf_dump(&buffer->buf, prefix, NULL, NULL);
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
    nxo_stack_pop(tstack);
}
#endif

#ifdef CW_BUF_VALIDATE
/* #buffer :validate - */
void
modslate_buffer_validate(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    buffer_p_lock(buffer);
    buf_validate(&buffer->buf);
    buffer_p_unlock(buffer);

    nxo_stack_pop(ostack);
}
#endif

/* marker. */
static cw_nxn_t
marker_p_get(cw_nxo_t *a_instance, cw_nxo_t *a_thread,
	     struct cw_marker **r_marker)
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

    if ((retval = modslate_instance_kind(a_instance, &s_marker)))
    {
	goto RETURN;
    }

    data = nxo_instance_data_get(a_instance);
    if (nxo_type_get(data) != NXOT_DICT)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    nxo_name_new(name, "marker", sizeof("marker") - 1, FALSE);
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

    *r_marker = (struct cw_marker *) nxo_handle_opaque_get(handle);
    retval = NXN_ZERO;
    RETURN:
    /* Clean up. */
    nxo_stack_npop(tstack, 2);

    return retval;
}

static cw_nxoe_t *
marker_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_marker *marker = (struct cw_marker *) a_data;
    static cw_uint32_t iter;

    if (a_reset)
    {
	iter = 0;
    }

    for (retval = NULL; retval == NULL; iter++)
    {
	switch(iter)
	{
	    case 0:
	    {
		retval = nxo_nxoe_get(&marker->buffer_instance);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&marker->buffer_handle);
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
marker_p_delete(void *a_data, cw_uint32_t a_iter)
{
    struct cw_marker *marker;
    struct cw_buffer *buffer;

    marker = (struct cw_marker *) a_data;
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    buffer_p_lock(buffer);
    mkr_delete(&marker->mkr);
    buffer_p_unlock(buffer);
    nxa_free(marker, sizeof(struct cw_marker));

    return FALSE;
}

static cw_bufw_t
marker_p_whence(cw_nxo_t *a_whence)
{
    cw_bufw_t retval;
    cw_uint32_t len;
    const cw_uint8_t *str;

    /* a_whence is a name.  Determine whether it is /SEEK_BOB, /SEEK_REL, or
     * /SEEK_EOB. */
    len = nxo_name_len_get(a_whence);
    /* All the valid names are the same length, so a single length works. */
    if (len != sizeof("SEEK_BOB") - 1)
    {
	retval = BUFW_NONE;
	goto RETURN;
    }

    str = nxo_name_str_get(a_whence);
    if (strncmp(str, "SEEK_REL", len) == 0)
    {
	retval = BUFW_REL;
    }
    else if (strncmp(str, "SEEK_BOB", len) == 0)
    {
	retval = BUFW_BOB;
    }
    else if (strncmp(str, "SEEK_EOB", len) == 0)
    {
	retval = BUFW_EOB;
    }
    else
    {
	retval = BUFW_NONE;
    }

    RETURN:
    return retval;
}

cw_nxn_t
modslate_marker_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread)
{
    struct cw_marker *marker;

    return marker_p_get(a_instance, a_thread, &marker);
}

/* #buffer #instance :marker #instance */
void
modslate_marker_marker(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *nxo, *instance;
    cw_nxo_t *tag, *name, *handle;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(instance, ostack, a_thread);
    if (nxo_type_get(instance) != NXOT_INSTANCE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, instance);
    if ((error = buffer_p_get(nxo, a_thread, &buffer)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    marker = (struct cw_marker *) nxa_malloc(sizeof(struct cw_marker));

    /* Reference the associated buffer. */
    nxo_no_new(&marker->buffer_instance);
    nxo_no_new(&marker->buffer_handle);
    if ((error = buffer_p_get(nxo, a_thread, &buffer))
	|| (error = buffer_p_handle_get(nxo, a_thread, &marker->buffer_handle)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    /* Initialize the mkr. */
    buffer_p_lock(buffer);
    mkr_new(&marker->mkr, &buffer->buf);
    buffer_p_unlock(buffer);

    /* Initialize the sequence number. */
    marker->seq = 0;

    /* Create a reference to the marker, now that the internals are
     * initialized. */
    name = nxo_stack_push(tstack);
    handle = nxo_stack_push(tstack);

    nxo_name_new(name, "marker", sizeof("marker") - 1, FALSE);
    nxo_handle_new(handle, marker, NULL, marker_p_ref_iter, marker_p_delete);

    /* Set the handle tag. */
    tag = nxo_handle_tag_get(handle);
    nxo_dup(tag, name);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Insert into the data dict. */
    nxo_dict_def(nxo_instance_data_get(instance), name, handle);

    /* Clean up. */
    nxo_stack_npop(tstack, 2);
    nxo_stack_remove(ostack, nxo);
}

/* #marker :seq #seq */
void
modslate_marker_seq(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_nxoi_t seq;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    buffer_p_lock(buffer);
    seq = marker->seq;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, seq);
}

/* #marker :copy #marker */
void
modslate_marker_copy(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker, *marker_copy;
    struct cw_buffer *buffer;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    marker_copy = (struct cw_marker *) nxa_malloc(sizeof(struct cw_marker));

    /* Reference the associated buffer. */
    nxo_no_new(&marker_copy->buffer_instance);
    nxo_dup(&marker_copy->buffer_instance, &marker->buffer_instance);

    nxo_no_new(&marker_copy->buffer_handle);
    nxo_dup(&marker_copy->buffer_handle, &marker->buffer_handle);

    /* Initialize the mkr. */
    buffer_p_lock(buffer);
    mkr_new(&marker_copy->mkr, &buffer->buf);
    mkr_dup(&marker_copy->mkr, &marker->mkr);
    buffer_p_unlock(buffer);

    /* Initialize the sequence number. */
    marker_copy->seq = 0;

    /* XXX How should the instance copy be done? */
    cw_error("XXX Not implemented.");
}

/* #marker :buffer #buffer */
void
modslate_marker_buffer(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t*ostack, *tstack, *nxo, *tnxo;
    cw_nxn_t error;
    struct cw_marker *marker;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_dup(nxo, &marker->buffer_instance);
    nxo_stack_pop(tstack);
}

/* #marker :line #line */
void
modslate_marker_line(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_nxoi_t line;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    buffer_p_lock(buffer);
    line = mkr_line(&marker->mkr);
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, line);
}

/* #offset #whence? #marker :seekline #pos */
void
modslate_marker_seekline(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_uint32_t npop;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_sint64_t offset, pos;
    cw_bufw_t whence;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    /* The whence argument is optional. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    switch (nxo_type_get(nxo))
    {
	case NXOT_NAME:
	{
	    whence = marker_p_whence(nxo);
	    if (whence == BUFW_NONE)
	    {
		nxo_thread_nerror(a_thread, NXN_limitcheck);
		return;
	    }
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

    buffer_p_lock(buffer);
    pos = mkr_line_seek(&marker->mkr, offset, whence);
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, (cw_nxoi_t)pos);

    nxo_stack_npop(ostack, npop);
}

/* #marker :position #pos */
void
modslate_marker_position(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_nxoi_t position;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    buffer_p_lock(buffer);
    position = (cw_nxoi_t)mkr_pos(&marker->mkr);
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, position);
}

/* #offset #whence/#marker? #marker :seek #pos */
void
modslate_marker_seek(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_uint32_t npop;
    cw_nxn_t error;
    struct cw_marker *marker, *whence_marker = NULL;
    struct cw_buffer *buffer, *whence_buffer;
    cw_sint64_t offset, pos;
    cw_bufw_t whence;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    /* The whence argument is optional. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INSTANCE:
	{
	    if ((error = marker_p_get(nxo, a_thread, &whence_marker)))
	    {
		nxo_thread_nerror(a_thread, error);
		return;
	    }
	    whence_buffer
		= (struct cw_buffer *)
		nxo_handle_opaque_get(&whence_marker->buffer_handle);

	    if (whence_buffer != buffer)
	    {
		nxo_thread_nerror(a_thread, NXN_argcheck);
		return;
	    }

	    whence = BUFW_REL;
	    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	    npop = 2;
	    break;
	}
	case NXOT_NAME:
	{
	    whence = marker_p_whence(nxo);
	    if (whence == BUFW_NONE)
	    {
		nxo_thread_nerror(a_thread, NXN_limitcheck);
		return;
	    }
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

    buffer_p_lock(buffer);
    if (whence_marker != NULL)
    {
	/* Move to the location of the whence marker before doing a relative
	 * seek. */
	mkr_dup(&marker->mkr, &whence_marker->mkr);
    }
    pos = mkr_seek(&marker->mkr, offset, whence);
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_integer_new(nxo, (cw_nxoi_t) pos);

    nxo_stack_npop(ostack, npop);
}

/* #marker :before_get #c */
void
modslate_marker_before_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *bp, c;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    buffer_p_lock(buffer);
    bp = mkr_before_get(&marker->mkr);
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

/* #marker :after_get #c */
void
modslate_marker_after_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *bp, c;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    buffer_p_lock(buffer);
    bp = mkr_after_get(&marker->mkr);
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

/* #marker #val :before_set - */
void
modslate_marker_before_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *bp, c;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    /* val. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    c = (cw_uint8_t) nxo_integer_get(nxo);

    buffer_p_lock(buffer);
    bp = mkr_before_get(&marker->mkr);
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

/* #val #marker :after_set - */
void
modslate_marker_after_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *bp, c;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    /* val. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    c = (cw_uint8_t) nxo_integer_get(nxo);

    buffer_p_lock(buffer);
    bp = mkr_after_get(&marker->mkr);
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

/* #str #marker :before_insert - */
void
modslate_marker_before_insert(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *str;
    cw_uint32_t str_len;
    cw_bufv_t bufv;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    /* str. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    str = nxo_string_get(nxo);
    str_len = nxo_string_len_get(nxo);

    bufv.data = str;
    bufv.len = str_len;

    buffer_p_lock(buffer);
    mkr_before_insert(&marker->mkr, &bufv, 1);
    buffer->seq++;
    marker->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

/* #str #marker :after_insert - */
void
modslate_marker_after_insert(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;
    cw_uint8_t *str;
    cw_uint32_t str_len;
    cw_bufv_t bufv;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    /* str. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    str = nxo_string_get(nxo);
    str_len = nxo_string_len_get(nxo);

    bufv.data = str;
    bufv.len = str_len;

    buffer_p_lock(buffer);
    mkr_after_insert(&marker->mkr, &bufv, 1);
    buffer->seq++;
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
}

/* #marker #marker :range_get #string */
void
modslate_marker_range_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker_a, *marker_b;
    struct cw_buffer *buffer_a, *buffer_b;
    cw_uint64_t pos_a, pos_b, str_len;
    cw_bufv_t *bufv, sbufv;
    cw_uint32_t bufvcnt;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker_b. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker_b)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer_b
	= (struct cw_buffer *) nxo_handle_opaque_get(&marker_b->buffer_handle);

    /* marker_a. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if ((error = marker_p_get(nxo, a_thread, &marker_a)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer_a
	= (struct cw_buffer *) nxo_handle_opaque_get(&marker_a->buffer_handle);

    if (buffer_a != buffer_b)
    {
	nxo_thread_nerror(a_thread, NXN_argcheck);
	return;
    }

    buffer_p_lock(buffer_a);

    /* Get a pointer to the buffer range and calculate its length. */
    bufv = mkr_range_get(&marker_a->mkr, &marker_b->mkr, &bufvcnt);
    pos_a = mkr_pos(&marker_a->mkr);
    pos_b = mkr_pos(&marker_b->mkr);
    str_len = (pos_a < pos_b) ? pos_b - pos_a : pos_a - pos_b;

    /* Create an Onyx string to store the result.  Since there are two markers
     * on the stack that have references to the buffer, it is safe to trash one
     * of them here. */
    nxo_string_new(nxo, nxo_thread_currentlocking(a_thread), str_len);

    /* Only copy if bufv isn't NULL.  The string is zero length in that case. */
    if (bufv != NULL)
    {
	sbufv.data = nxo_string_get(nxo);
	sbufv.len = str_len;
	bufv_copy(&sbufv, 1, bufv, bufvcnt, 0);
    }

    buffer_p_unlock(buffer_a);

    nxo_stack_pop(ostack);
}

/* #marker #marker :range_cut #string */
void
modslate_marker_range_cut(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker_a, *marker_b;
    struct cw_buffer *buffer_a, *buffer_b;
    cw_uint64_t pos_a, pos_b, str_len;
    cw_bufv_t *bufv, sbufv;
    cw_uint32_t bufvcnt;

    ostack = nxo_thread_ostack_get(a_thread);

    /* marker_b. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker_b)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer_b
	= (struct cw_buffer *) nxo_handle_opaque_get(&marker_b->buffer_handle);

    /* marker_a. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if ((error = marker_p_get(nxo, a_thread, &marker_a)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer_a
	= (struct cw_buffer *) nxo_handle_opaque_get(&marker_a->buffer_handle);

    if (buffer_a != buffer_b)
    {
	nxo_thread_nerror(a_thread, NXN_argcheck);
	return;
    }

    buffer_p_lock(buffer_a);

    /* Get a pointer to the buffer range and calculate its length. */
    bufv = mkr_range_get(&marker_a->mkr, &marker_b->mkr, &bufvcnt);
    pos_a = mkr_pos(&marker_a->mkr);
    pos_b = mkr_pos(&marker_b->mkr);
    str_len = (pos_a < pos_b) ? pos_b - pos_a : pos_a - pos_b;

    /* Create an Onyx string to store the result.  Since there are two markers
     * on the stack that have references to the buffer, it is safe to trash one
     * of them here. */
    nxo_string_new(nxo, nxo_thread_currentlocking(a_thread), str_len);

    /* Only copy if bufv isn't NULL.  The string is zero length in that case. */
    if (bufv != NULL)
    {
	sbufv.data = nxo_string_get(nxo);
	sbufv.len = str_len;
	bufv_copy(&sbufv, 1, bufv, bufvcnt, 0);
    }

    /* Remove the buffer range. */
    mkr_remove(&marker_a->mkr, &marker_b->mkr);

    buffer_a->seq++;
    marker_a->seq++;
    marker_b->seq++;
    buffer_p_unlock(buffer_a);

    nxo_stack_pop(ostack);
}

#ifdef CW_BUF_DUMP
/* `prefix' #marker :dump - */
void
modslate_marker_dump(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    cw_uint8_t *prefix;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);
    
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_string_cstring(tnxo, nxo, a_thread);
    prefix = nxo_string_get(tnxo);

    buffer_p_lock(buffer);
    mkr_dump(&marker->mkr, prefix, NULL, NULL);
    buffer_p_unlock(buffer);

    nxo_stack_npop(ostack, 2);
    nxo_stack_pop(tstack);
}
#endif

#ifdef CW_BUF_VALIDATE
/* #marker :validate - */
void
modslate_marker_validate(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_marker *marker;
    struct cw_buffer *buffer;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if ((error = marker_p_get(nxo, a_thread, &marker)))
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&marker->buffer_handle);

    buffer_p_lock(buffer);
    mkr_validate(&marker->mkr);
    buffer_p_unlock(buffer);

    nxo_stack_pop(ostack);
}
#endif

/* extent. */
#ifdef XXX_NOT_YET
static cw_nxoe_t *
extent_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_extent *extent = (struct cw_extent *) a_data;
    static cw_uint32_t iter;

    if (a_reset)
    {
	iter = 0;
    }

    for (retval = NULL; retval == NULL; iter++)
    {
	switch(iter)
	{
	    case 0:
	    {
		retval = nxo_nxoe_get(&extent->buffer_instance);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&extent->buffer_handle);
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
extent_p_delete(void *a_data, cw_uint32_t a_iter)
{
    struct cw_extent *extent;
    struct cw_buffer *buffer;

    extent = (struct cw_extent *) a_data;
    buffer = (struct cw_buffer *) nxo_handle_opaque_get(&extent->buffer_nxo);

    buffer_p_lock(buffer);
    ext_delete(&extent->ext);
    buffer_p_unlock(buffer);
    nxa_free(extent, sizeof(struct cw_extent));

    return FALSE;
}

cw_nxn_t
modslate_extent_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread)
{
    struct cw_extent *extent;

    return extent_p_get(a_instance, a_thread, &extent);
}
#endif /* XXX_NOT_YET. */

/* #buffer #beg #end extent #extent */
void
modslate_extent_extent(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_copy(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_buffer(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_beg_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_beg_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_end_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_end_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_beg_open_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_beg_open_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_end_open_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_end_open_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_before_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_at_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_after_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_prev_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_next_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_detachable_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_detachable_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_detached_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

void
modslate_extent_detach(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}

#ifdef CW_BUF_DUMP
void
modslate_extent_dump(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}
#endif

#ifdef CW_BUF_VALIDATE
void
modslate_extent_validate(void *a_data, cw_nxo_t *a_thread)
{
    cw_error("XXX Not implemented");
}
#endif
