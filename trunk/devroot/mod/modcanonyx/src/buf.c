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

#include "modcanonyx.h"

/* buf. */
void
buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc, cw_opaque_dealloc_t
    *a_dealloc, void *a_arg)
{
}

void
buf_subbuf(cw_buf_t *a_buf, cw_bufe_t *a_bufe)
{
}

void
buf_delete(cw_buf_t *a_buf)
{
}

cw_buf_t *
buf_buf(cw_buf_t *a_buf)
{
	return NULL; /* XXX */
}

cw_uint64_t
buf_count(cw_buf_t *a_buf)
{
	return 0; /* XXX */
}

cw_bool_t
buf_undo_active_get(cw_buf_t *a_buf)
{
	return TRUE; /* XXX */
}

void
buf_undo_active_set(cw_buf_t *a_buf, cw_bool_t a_active)
{
}

cw_bool_t buf_undo(cw_buf_t *a_buf)
{
	return TRUE; /* XXX */
}

cw_bool_t
buf_redo(cw_buf_t *a_buf)
{
	return TRUE; /* XXX */
}

void
buf_history_boundary(cw_buf_t *a_buf)
{
}

cw_bool_t
buf_group_undo(cw_buf_t *a_buf)
{
	return TRUE; /* XXX */
}

cw_bool_t
buf_group_redo(cw_buf_t *a_buf)
{
	return TRUE; /* XXX */
}

void
buf_history_flush(cw_buf_t *a_buf)
{
}

cw_bufe_t *
buf_bufe_next(cw_buf_t *a_buf, cw_bufe_t *a_bufe)
{
	return NULL; /* XXX */
}

cw_bufe_t *
buf_bufe_prev(cw_buf_t *a_buf, cw_bufe_t *a_bufe)
{
	return NULL; /* XXX */
}

/* bufm. */
void
bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf, cw_bufq_t *a_bufq)
{
}

void
bufm_dup(cw_bufm_t *a_bufm, const cw_bufm_t *a_orig, cw_bufq_t *a_bufq)
{
}

void
bufm_delete(cw_bufm_t *a_bufm)
{
}

cw_buf_t *
bufm_buf(cw_bufm_t *a_bufm)
{
	return NULL; /* XXX */
}

cw_uint64_t
bufm_line(const cw_bufm_t *a_bufm)
{
	return 0; /* XXX */
}

cw_uint64_t
bufm_rel_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount)
{
	return 0; /* XXX */
}

cw_uint64_t
bufm_abs_seek(cw_bufm_t *a_bufm, cw_uint64_t a_amount)
{
	return 0; /* XXX */
}

cw_uint64_t
bufm_pos(cw_bufm_t *a_bufm)
{
	return 0; /* XXX */
}

cw_bufc_t
bufm_bufc_get(cw_bufm_t *a_bufm)
{
	return 0; /* XXX */
}

void
bufm_bufc_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
}

void
bufm_bufc_insert(cw_bufm_t *a_bufm, const cw_bufc_t *a_data, cw_uint64_t
    *a_count)
{
}

cw_bufe_t *
bufm_bufe_down(cw_bufm_t *a_bufm, cw_bufe_t *a_bufe)
{
	return NULL; /* XXX */
}

cw_bufe_t *
bufm_bufe_up(cw_bufm_t *a_bufm, cw_bufe_t *a_bufe)
{
	return NULL; /* XXX */
}

/* bufe. */
void
bufe_new(cw_bufe_t *a_bufe, cw_bufm_t *a_beg, cw_bufm_t *a_end, cw_bool_t
    a_beg_open, cw_bool_t a_end_open, cw_bool_t a_detachable, cw_bufq_t *a_bufq)
{
}

void
bufe_dup(cw_bufe_t *a_bufe, cw_bufe_t *a_orig, cw_bufq_t *a_bufq)
{
}

void
bufe_delete(cw_bufe_t *a_bufe)
{
}

cw_buf_t *
bufe_buf(cw_bufm_t *a_bufm)
{
	return NULL; /* XXX */
}

const cw_bufm_t *
bufe_beg(cw_bufe_t *a_bufe)
{
	return NULL; /* XXX */
}

const cw_bufm_t *
bufe_end(cw_bufe_t *a_bufe)
{
	return NULL; /* XXX */
}

cw_bufe_t *
bufe_bufe_next(cw_bufe_t *a_bufe, cw_bufe_t *a_curr)
{
	return NULL; /* XXX */
}

cw_bufe_t *
bufe_bufe_prev(cw_bufe_t *a_bufe, cw_bufe_t *a_curr)
{
	return NULL; /* XXX */
}

void
bufe_cut(cw_bufe_t *a_bufe)
{
}

cw_uint32_t
bufe_foreground_get(cw_bufe_t *a_bufe)
{
	return 0; /* XXX */
}

void
bufe_foreground_set(cw_bufe_t *a_bufe, cw_uint32_t a_foreground)
{
}

cw_uint32_t
bufe_background_get(cw_bufe_t *a_bufe)
{
	return 0; /* XXX */
}

void
bufe_background_set(cw_bufe_t *a_bufe, cw_uint32_t a_background)
{
}

cw_bool_t
bufe_bold_get(cw_bufe_t *a_bufe)
{
	return TRUE; /* XXX */
}

void
bufe_bold_set(cw_bufe_t *a_bufe, cw_bool_t a_bold)
{
}

cw_bool_t
bufe_italic_get(cw_bufe_t *a_bufe)
{
	return TRUE; /* XXX */
}

void
bufe_italic_set(cw_bufe_t *a_bufe, cw_bool_t a_italic)
{
}

cw_bool_t
bufe_underline_get(cw_bufe_t *a_bufe)
{
	return TRUE; /* XXX */
}

void
bufe_underline_set(cw_bufe_t *a_bufe, cw_bool_t a_underline)
{
}

/* bufq. */
void
bufq_new(cw_bufq_t *a_bufq)
{
}

void
bufq_delete(cw_bufq_t *a_bufq)
{
}

cw_bufqm_t *
bufq_get(cw_bufq_t *a_bufq)
{
	return NULL; /* XXX */
}

cw_bufqm_t *
bufq_tryget(cw_bufq_t *a_bufq)
{
	return NULL; /* XXX */
}

cw_bufqm_t *
bufq_timedget(cw_bufq_t *a_bufq, const struct timespec *a_timeout)
{
	return NULL; /* XXX */
}

void
bufq_put(cw_bufq_t *a_bufq, cw_bufqm_t *a_bufqm)
{
}

/* bufqm. */
void
bufqm_bufe_new(cw_bufqm_t *a_bufqm, cw_opaque_dealloc_t *a_dealloc, const void
    *a_dealloc_arg, cw_bufe_t *a_bufe, cw_bufqmt_t a_event)
{
}

void
bufqm_bufm_new(cw_bufqm_t *a_bufqm, cw_opaque_dealloc_t *a_dealloc, const void
    *a_dealloc_arg, cw_bufm_t *a_bufm, cw_bufqmt_t a_event)
{
}

void
bufqm_delete(cw_bufqm_t *a_bufqm)
{
}

cw_bufqmt_t
bufqm_event(cw_bufqm_t *a_bufqm)
{
	return BUFQMT_NONE; /* XXX */
}

cw_bufe_t *
bufqm_bufe(cw_bufqm_t *a_bufqm)
{
	return NULL; /* XXX */
}

cw_bufm_t *
bufqm_bufm(cw_bufqm_t *a_bufqm)
{
	return NULL; /* XXX */
}
