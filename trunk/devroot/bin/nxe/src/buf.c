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

#include "nxe.h"

/*
 * Prototypes.
 */
/* bufb. */
static void	bufb_p_new(cw_bufb_t *a_bufb);
static void	bufb_p_gap_move(cw_bufb_t *a_bufb, cw_uint32_t a_offset);
static cw_bufc_t bufb_p_get(cw_bufb_t *a_bufb, cw_uint32_t a_offset);
static void	bufb_p_insert(cw_bufb_t *a_bufb, cw_uint32_t a_offset, const
    cw_bufc_t *a_data, cw_uint64_t a_count);
static void	bufb_p_split(cw_bufb_t *a_bufb, cw_bufb_t *a_after, cw_uint32_t
    a_offset);
static void	bufb_p_remove(cw_bufb_t *a_bufb, cw_uint32_t a_offset,
    cw_uint32_t a_count);

/* bufh. */
static void	bufh_p_new(cw_bufh_t *a_bufh);
static void	bufh_p_delete(cw_bufh_t *a_bufh);

/* buf. */
/*  static void	buf_p_bufb_cache_update(cw_buf_t *a_buf, cw_uint64_t a_index); */

/* bufb. */
static void
bufb_p_new(cw_bufb_t *a_bufb)
{
	_cw_check_ptr(a_bufb);
	_cw_assert(sizeof(cw_bufb_t) == _CW_BUFB_SIZE);

	a_bufb->nlines = 0;
	a_bufb->gap_off = 0;
	a_bufb->gap_len = _CW_BUFB_DATA;
}

#define	bufb_p_len_get(a_bufb) (_CW_BUFB_DATA - (a_bufb)->gap_len)
#define	bufb_p_nlines_get(a_bufb) ((cw_uint64_t)(a_bufb)->nlines)
#define	bufb_p_eoff_get(a_bufb) (a_bufb)->eoff
#define	bufb_p_eoff_set(a_bufb, a_eoff)					\
	(a_bufb)->eoff = (a_eoff) + bufb_p_len_get(a_bufb)
#define	bufb_p_eline_get(a_bufb) (a_bufb)->eline
#define	bufb_p_eline_set(a_bufb, a_eline)				\
	(a_bufb)->eline = (a_eline) + bufb_p_nlines_get(a_bufb)

#define	bufb_p_gap_len_get(a_bufb) (a_bufb)->gap_len;

static void
bufb_p_gap_move(cw_bufb_t *a_bufb, cw_uint32_t a_offset)
{
	_cw_check_ptr(a_bufb);
	_cw_assert(a_offset <= bufb_p_len_get(a_bufb));

	/* Move the gap if it isn't already at the point of insertion. */
	if (a_bufb->gap_off != a_offset) {
		if (a_bufb->gap_off < a_offset) {
			/*
			 * Move the gap forward.
			 *
			 * o: data
			 * X: move
			 * _: gap
			 *
			 * ooooooo________XXXXXXXXXXXoo
			 *                   ^
			 *                   |
			 *                   a_offset
			 *                   |
			 *                   v
			 * oooooooXXXXXXXXXXX________oo
			 */
			memmove(&a_bufb->data[a_bufb->gap_off],
			    &a_bufb->data[a_bufb->gap_off + a_bufb->gap_len],
			    (a_offset - a_bufb->gap_off) * sizeof(cw_bufc_t));
		} else {
			/*
			 * Move the gap backward.
			 *
			 * o: data
			 * X: move
			 * _: gap
			 *
			 * ooooXXXXXXXXX___________oooo
			 *     ^
			 *     |
			 *     a_offset
			 *     |
			 *     v
			 * oooo___________XXXXXXXXXoooo

			 * a_offset + gap_len
			 */
			memmove(&a_bufb->data[a_bufb->gap_len + a_offset],
			    &a_bufb[a_offset],
			    (a_bufb->gap_off - a_offset) * sizeof(cw_bufc_t));
		}
		a_bufb->gap_off = a_offset;
	}
}

static cw_bufc_t
bufb_p_get(cw_bufb_t *a_bufb, cw_uint32_t a_offset)
{
	cw_bufc_t	retval;

	_cw_check_ptr(a_bufb);
	_cw_assert(a_offset < bufb_p_len_get(a_bufb));

	if (a_offset < a_bufb->gap_off)
		retval = a_bufb->data[a_offset];
	else
		retval = a_bufb->data[a_offset + a_bufb->gap_len];

	return retval;
}

static void
bufb_p_insert(cw_bufb_t *a_bufb, cw_uint32_t a_offset, const cw_bufc_t *a_data,
    cw_uint64_t a_count)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_bufb);
	_cw_assert(a_count <= a_bufb->gap_len);
	_cw_assert(a_offset + a_count <= _CW_BUFB_DATA);

	bufb_p_gap_move(a_bufb, a_offset);
	for (i = 0; i < a_count; i++) {
		a_bufb->data[a_offset + i] = a_data[i];
		if (a_data[i] == '\n')
			a_bufb->nlines++;
	}
	a_bufb->gap_off += a_count;
}

static void
bufb_p_remove(cw_bufb_t *a_bufb, cw_uint32_t a_offset, cw_uint32_t a_count)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_bufb);
	_cw_assert(a_offset + a_count <= bufb_p_len_get(a_bufb));
	
	bufb_p_gap_move(a_bufb, a_offset);
	for (i = 0; i < a_count; i++) {
		if (a_bufb->data[a_bufb->gap_off + a_bufb->gap_len + i] == '\n')
			a_bufb->nlines--;
	}
	a_bufb->gap_len += a_count;
}

/*
 * Split a_bufb into two bufb's at offset a_offset.  a_after starts out empty.
 */
static void
bufb_p_split(cw_bufb_t *a_bufb, cw_bufb_t *a_after, cw_uint32_t a_offset)
{
	_cw_check_ptr(a_bufb);
	_cw_assert(a_offset <= _CW_BUFB_DATA - a_bufb->gap_len);

	bufb_p_gap_move(a_bufb, a_offset);
	bufb_p_insert(a_after, 0, &a_bufb->data[a_bufb->gap_off +
	    a_bufb->gap_len], _CW_BUFB_DATA - a_bufb->gap_off -
	    a_bufb->gap_len);
	a_bufb->gap_len = _CW_BUFB_DATA - a_bufb->gap_off;

	a_bufb->nlines -= bufb_p_nlines_get(a_after);
}

/* bufh. */
static void
bufh_p_new(cw_bufh_t *a_bufh)
{
	_cw_error("XXX Not implemented");
	/* XXX */
}

static void
bufh_p_delete(cw_bufh_t *a_bufh)
{
	_cw_error("XXX Not implemented");
	/* XXX */
}

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
	bufb_p_eoff_set(retval->bufb_vec[0], 0);
	bufb_p_eline_set(retval->bufb_vec[0], 1);

	retval->last_cached = 0;

	/* Initialize history. */
	retval->hist_active = FALSE;
	/* XXX */
/*  	bufh_p_new(&retval->hist); */

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

	if (a_active == FALSE && a_buf->hist_active)
		buf_hist_flush(a_buf);

	a_buf->hist_active = a_active;
}

cw_bool_t
buf_undo(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	_cw_error("XXX Not implemented");
	return TRUE; /* XXX */
}

cw_bool_t
buf_redo(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	_cw_error("XXX Not implemented");
	return TRUE; /* XXX */
}

void
buf_hist_group_beg(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	_cw_error("XXX Not implemented");
}

void
buf_hist_group_end(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	_cw_error("XXX Not implemented");
}

void
buf_hist_flush(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	_cw_error("XXX Not implemented");
}

cw_bufe_t *
buf_bufe_next(cw_buf_t *a_buf, cw_bufe_t *a_bufe)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_bufe_t *
buf_bufe_prev(cw_buf_t *a_buf, cw_bufe_t *a_bufe)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

/* bufm. */
void
bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf, cw_bufq_t *a_bufq)
{
	_cw_error("XXX Not implemented");
}

void
bufm_dup(cw_bufm_t *a_bufm, const cw_bufm_t *a_orig, cw_bufq_t *a_bufq)
{
	_cw_error("XXX Not implemented");
}

void
bufm_delete(cw_bufm_t *a_bufm)
{
	_cw_error("XXX Not implemented");
}

cw_buf_t *
bufm_buf(cw_bufm_t *a_bufm)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_uint64_t
bufm_line(const cw_bufm_t *a_bufm)
{
	_cw_error("XXX Not implemented");
	return 0; /* XXX */
}

cw_uint64_t
bufm_rel_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount)
{
	_cw_error("XXX Not implemented");
	return 0; /* XXX */
}

cw_uint64_t
bufm_abs_seek(cw_bufm_t *a_bufm, cw_uint64_t a_amount)
{
	_cw_error("XXX Not implemented");
	return 0; /* XXX */
}

cw_uint64_t
bufm_pos(cw_bufm_t *a_bufm)
{
	_cw_error("XXX Not implemented");
	return 0; /* XXX */
}

cw_bufc_t
bufm_bufc_get(cw_bufm_t *a_bufm)
{
	_cw_error("XXX Not implemented");
	return 0; /* XXX */
}

void
bufm_bufc_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
	_cw_error("XXX Not implemented");
}

void
bufm_bufc_insert(cw_bufm_t *a_bufm, const cw_bufc_t *a_data, cw_uint64_t
    a_count)
{
	_cw_error("XXX Not implemented");
}

cw_bufe_t *
bufm_bufe_down(cw_bufm_t *a_bufm, cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_bufe_t *
bufm_bufe_up(cw_bufm_t *a_bufm, cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

/* bufe. */
void
bufe_new(cw_bufe_t *a_bufe, cw_bufm_t *a_beg, cw_bufm_t *a_end, cw_bool_t
    a_beg_open, cw_bool_t a_end_open, cw_bool_t a_detachable, cw_bufq_t *a_bufq)
{
	_cw_error("XXX Not implemented");
}

void
bufe_dup(cw_bufe_t *a_bufe, cw_bufe_t *a_orig, cw_bufq_t *a_bufq)
{
	_cw_error("XXX Not implemented");
}

void
bufe_delete(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
}

cw_buf_t *
bufe_buf(cw_bufm_t *a_bufm)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

const cw_bufm_t *
bufe_beg(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

const cw_bufm_t *
bufe_end(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_bufe_t *
bufe_bufe_next(cw_bufe_t *a_bufe, cw_bufe_t *a_curr)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_bufe_t *
bufe_bufe_prev(cw_bufe_t *a_bufe, cw_bufe_t *a_curr)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

void
bufe_cut(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
}

cw_uint32_t
bufe_foreground_get(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return 0; /* XXX */
}

void
bufe_foreground_set(cw_bufe_t *a_bufe, cw_uint32_t a_foreground)
{
	_cw_error("XXX Not implemented");
}

cw_uint32_t
bufe_background_get(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return 0; /* XXX */
}

void
bufe_background_set(cw_bufe_t *a_bufe, cw_uint32_t a_background)
{
	_cw_error("XXX Not implemented");
}

cw_bool_t
bufe_bold_get(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return TRUE; /* XXX */
}

void
bufe_bold_set(cw_bufe_t *a_bufe, cw_bool_t a_bold)
{
	_cw_error("XXX Not implemented");
}

cw_bool_t
bufe_italic_get(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return TRUE; /* XXX */
}

void
bufe_italic_set(cw_bufe_t *a_bufe, cw_bool_t a_italic)
{
	_cw_error("XXX Not implemented");
}

cw_bool_t
bufe_underline_get(cw_bufe_t *a_bufe)
{
	_cw_error("XXX Not implemented");
	return TRUE; /* XXX */
}

void
bufe_underline_set(cw_bufe_t *a_bufe, cw_bool_t a_underline)
{
	_cw_error("XXX Not implemented");
}

/* bufq. */
void
bufq_new(cw_bufq_t *a_bufq)
{
	_cw_error("XXX Not implemented");
}

void
bufq_delete(cw_bufq_t *a_bufq)
{
	_cw_error("XXX Not implemented");
}

cw_bufqm_t *
bufq_get(cw_bufq_t *a_bufq)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_bufqm_t *
bufq_tryget(cw_bufq_t *a_bufq)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_bufqm_t *
bufq_timedget(cw_bufq_t *a_bufq, const struct timespec *a_timeout)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

void
bufq_put(cw_bufq_t *a_bufq, cw_bufqm_t *a_bufqm)
{
	_cw_error("XXX Not implemented");
}

/* bufqm. */
void
bufqm_bufe_new(cw_bufqm_t *a_bufqm, cw_opaque_dealloc_t *a_dealloc, const void
    *a_dealloc_arg, cw_bufe_t *a_bufe, cw_bufqmt_t a_event)
{
	_cw_error("XXX Not implemented");
}

void
bufqm_bufm_new(cw_bufqm_t *a_bufqm, cw_opaque_dealloc_t *a_dealloc, const void
    *a_dealloc_arg, cw_bufm_t *a_bufm, cw_bufqmt_t a_event)
{
	_cw_error("XXX Not implemented");
}

void
bufqm_delete(cw_bufqm_t *a_bufqm)
{
	_cw_error("XXX Not implemented");
}

cw_bufqmt_t
bufqm_event(cw_bufqm_t *a_bufqm)
{
	_cw_error("XXX Not implemented");
	return BUFQMT_NONE; /* XXX */
}

cw_bufe_t *
bufqm_bufe(cw_bufqm_t *a_bufqm)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_bufm_t *
bufqm_bufm(cw_bufqm_t *a_bufqm)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}
