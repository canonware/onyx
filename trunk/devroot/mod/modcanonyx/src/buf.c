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

/* bufb. */
static void
bufb_p_new(cw_bufb_t *a_bufb)
{
	_cw_check_ptr(a_bufb);
	_cw_assert(sizeof(cw_bufb_t) == _CW_BUFB_SIZE);

	a_bufb->gap_off = 0;
	a_bufb->gap_len = _CW_BUFB_DATA;

#ifdef _CW_DBG
	a_bufb->magic = _CW_BUFB_MAGIC;
#endif
}

static void
bufb_p_cache_set(cw_bufb_t *a_bufb, cw_uint64_t a_offset, cw_uint64_t a_line)
{
	_cw_check_ptr(a_bufb);
	_cw_dassert(a_bufb->magic == _CW_BUFB_MAGIC);

	a_bufb->offset = a_offset;
	a_bufb->line = a_line;
}

#if (0) /* Not used yet. */
static cw_uint64_t
bufb_p_len_get(cw_bufb_t *a_bufb)
{
	_cw_check_ptr(a_bufb);
	_cw_dassert(a_bufb->magic == _CW_BUFB_MAGIC);

	return _CW_BUFB_DATA - a_bufb->gap_len;
}

static cw_uint64_t
bufb_p_newlines_count(cw_bufb_t *a_bufb)
{
	cw_uint32_t	retval, i;

	_cw_check_ptr(a_bufb);
	_cw_dassert(a_bufb->magic == _CW_BUFB_MAGIC);

	/* Count the number of '\n' characters.  Avoid the gap. */
	for (i = retval = 0; i < a_bufb->gap_off; i++) {
		if (a_bufb->data[i] == '\n')
			retval++;
	}

	for (i = a_bufb->gap_off + a_bufb->gap_len; i < _CW_BUFB_DATA; i++) {
		if (a_bufb->data[i] == '\n')
			retval++;
	}

	return (cw_uint64_t)retval;
}
#endif

/* bufh. */
static void
bufh_p_new(cw_bufh_t *a_bufh)
{
	/* XXX */
}

#if (0) /* Not used yet. */
static void
bufh_p_delete(cw_bufh_t *a_bufh)
{
	/* XXX */
}
#endif

/* buf. */
cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc, cw_opaque_realloc_t
    *a_realloc, cw_opaque_dealloc_t *a_dealloc, void *a_arg)
{
	cw_buf_t	*retval;

	/*
	 * The memset() isn't strictly necessary, since we initialize all
	 * fields, but it may clean up some clutter that could be confusing when
	 * debugging.
	 */
	if (a_buf != NULL) {
		retval = a_buf;
		memset(retval, 0, sizeof(cw_buf_t));
		retval->alloced = FALSE;
	} else {
		retval = (cw_buf_t *)a_alloc(a_arg, sizeof(cw_buf_t), __FILE__,
		    __LINE__);
		memset(retval, 0, sizeof(cw_buf_t));
		retval->alloced = TRUE;
	}

	retval->alloc = a_alloc;
	retval->realloc = a_realloc;
	retval->dealloc = a_dealloc;
	retval->arg = a_arg;

	/* Set up the bufb vector. */
	retval->len = 0;
	retval->bufb_count = 1;
	retval->bufb_veclen = 1;
	retval->bufb_vec = (cw_bufb_t **)a_alloc(a_arg, sizeof(cw_bufb_t *),
	    __FILE__, __LINE__);

	/* Initialize a bufb. */
	retval->bufb_vec[0] = (cw_bufb_t *)a_alloc(a_arg, sizeof(cw_bufb_t),
	    __FILE__, __LINE__);
	bufb_p_new(retval->bufb_vec[0]);
	bufb_p_cache_set(retval->bufb_vec[0], 0, 1);

	retval->last_cached = 0;

	/* Initialize history. */
	retval->hist_active = FALSE;
	bufh_p_new(&retval->hist);

	/* Initialize lists. */
	ql_new(&retval->bufms);
	ql_new(&retval->bufes_fwd);
	ql_new(&retval->bufes_rev);
	ql_new(&retval->bufes_det);

#ifdef _CW_DBG
	retval->magic = _CW_BUF_MAGIC;
#endif

	return retval;
}

void
buf_delete(cw_buf_t *a_buf)
{
	cw_bufe_t	*bufe;
	cw_bufm_t	*bufm;
	cw_uint64_t	i;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	/*
	 * Set the buf pointers of all objects that point to this one to NULL,
	 * so that they won't try to disconnect during destruction.  All objects
	 * that reference this one effectively become invalid, but they can (and
	 * should) be destroyed even though this base buf is gone.
	 */
	ql_foreach(bufe, &a_buf->bufes_det, link) {
		bufe->buf = NULL;
	}

	ql_foreach(bufe, &a_buf->bufes_fwd, link) {
		bufe->buf = NULL;
	}

	ql_foreach(bufm, &a_buf->bufms, link) {
		bufm->buf = NULL;
	}

	/* Destroy the bufb vector. */
	for (i = 0; i < a_buf->bufb_count; i++) {
		_cw_opaque_dealloc(a_buf->dealloc, a_buf->arg,
		    a_buf->bufb_vec[i], sizeof(cw_bufb_t));
	}
	_cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf->bufb_vec,
	    sizeof(cw_bufb_t *) * a_buf->bufb_veclen);

	if (a_buf->alloced) {
		_cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf,
		    sizeof(cw_buf_t));
	}
#ifdef _CW_DBG
	else
		memset(a_buf, 0x5a, sizeof(cw_buf_t));
#endif
}

cw_uint64_t
buf_count(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);
	
	return a_buf->len;
}

cw_bool_t
buf_hist_active_get(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	return a_buf->hist_active;
}

void
buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

}

cw_bool_t
buf_undo(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	return TRUE; /* XXX */
}

cw_bool_t
buf_redo(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	return TRUE; /* XXX */
}

void
buf_hist_group_beg(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

}

void
buf_hist_group_end(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

}

void
buf_hist_flush(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

}

cw_bufe_t *
buf_bufe_next(cw_buf_t *a_buf, cw_bufe_t *a_bufe)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	return NULL; /* XXX */
}

cw_bufe_t *
buf_bufe_prev(cw_buf_t *a_buf, cw_bufe_t *a_bufe)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

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
