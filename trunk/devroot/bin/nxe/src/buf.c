/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * This file contains a buffer gap implementation of buffers for use in the nxe
 * text editor.  The code is broken up into the following classes:
 *
 * buf  : Main buffer class.
 * bufc : Character.  Characters are merely 8 bit values.
 * bufm : Marker.  Markers are used as handles for many buf operations.
 * bufh : History.  The bufh class provides an abstract history mechanism to the
 *        buf class and implements infinite undo and redo.
 *
 * buf's and everything associated with them are implicitly synchronized.
 *
 * Buffer/character position numbering starts at 1.
 *
 * Absolute position:  0   1   2   3   4   5   6   7   8
 *                                                     |
 * Character position: 1   2   3               4   5   |
 *                     |   |   |               |   |   |
 *                     v   v   v               v   v   v
 *                   /---+---+---+---+---+---+---+---\
 *                   | A | B | C |:::|:::|:::| D | E |
 *                   \---+---+---+---+---+---+---+---/
 *                   ^   ^   ^               ^   ^   ^
 *                   |   |   |               |   |   |
 * Buffer position:  1   2   3               4   5   6
 *
 * Position 0 is invalid, and there is one more buffer position than there are
 * character positions.  Externally, character positions are never mentioned.
 *
 * Position rules:
 *
 * *) bpos refers to buffer position.
 * *) cpos refers to character position.
 * *) apos refers to absolute position.
 * *) If a position isn't specified as bpos, cpos, or apos, then it is bpos.
 *
 ******************************************************************************/

#include "../include/nxe.h"

/*
 * Prototypes.
 */

/* bufh. */
static void	bufh_p_new(cw_bufh_t *a_bufh);
static void	bufh_p_delete(cw_bufh_t *a_bufh);

/* buf. */
static cw_uint64_t buf_p_pos_c2a(cw_buf_t *a_buf, cw_uint64_t a_cpos);
static cw_uint64_t buf_p_pos_a2b(cw_buf_t *a_buf, cw_uint64_t a_apos);
static cw_uint64_t buf_p_pos_a2c(cw_buf_t *a_buf, cw_uint64_t a_apos);
static cw_uint64_t buf_p_lines_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
    cw_uint64_t a_apos_end);
static void	buf_p_bufms_apos_adjust(cw_buf_t *a_buf, cw_bufm_t *a_bufm,
    cw_bool_t a_exclude, cw_sint64_t a_adjust, cw_uint64_t a_beg_apos,
    cw_uint64_t a_end_apos);
static void	buf_p_gap_move(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_bool_t
    a_exclude, cw_uint64_t a_bpos);
static void	buf_p_grow(cw_buf_t *a_buf, cw_uint64_t a_minlen);

/* bufm. */
static void	bufm_p_insert(cw_bufm_t *a_bufm, cw_bool_t a_exclude, const
    cw_char_t *a_str, cw_uint64_t a_count);

/*
 * Code.
 */

/* bufh. */
static void
bufh_p_new(cw_bufh_t *a_bufh)
{
	/* XXX */
}

static void
bufh_p_delete(cw_bufh_t *a_bufh)
{
	/* XXX */
}

/* buf. */
static cw_uint64_t
buf_p_pos_b2a(cw_buf_t *a_buf, cw_uint64_t a_bpos)
{
	cw_uint64_t	apos;

	_cw_assert(a_bpos > 0);
	_cw_assert(a_bpos <= a_buf->len + 1);

	if (a_bpos <= a_buf->gap_off)
		apos = a_bpos - 1;
	else
		apos = a_bpos - 1 + a_buf->gap_len;

	return apos;
}

static cw_uint64_t
buf_p_pos_c2a(cw_buf_t *a_buf, cw_uint64_t a_cpos)
{
	cw_uint64_t	apos;

	_cw_assert(a_cpos > 0);
	_cw_assert(a_cpos <= a_buf->len);

	if (a_cpos <= a_buf->gap_off)
		apos = a_cpos - 1;
	else
		apos = a_cpos - 1 + a_buf->gap_len;

	return apos;
}

static cw_uint64_t
buf_p_pos_a2b(cw_buf_t *a_buf, cw_uint64_t a_apos)
{
	cw_uint64_t	bpos;

	_cw_assert(a_apos <= a_buf->gap_off || a_apos >= a_buf->gap_off +
	    a_buf->gap_len);

	if (a_apos <= a_buf->gap_off)
		bpos = a_apos + 1;
	else
		bpos = a_apos + 1 - a_buf->gap_len;

	return bpos;
}

/* Same as buf_p_pos_a2b(). */
static cw_uint64_t
buf_p_pos_a2c(cw_buf_t *a_buf, cw_uint64_t a_apos)
{
	cw_uint64_t	cpos;

	_cw_assert(a_apos <= a_buf->gap_off || a_apos >= a_buf->gap_off +
	    a_buf->gap_len);

	if (a_apos <= a_buf->gap_off)
		cpos = a_apos + 1;
	else
		cpos = a_apos + 1 - a_buf->gap_len;

	return cpos;
}

static cw_uint64_t
buf_p_lines_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg, cw_uint64_t
    a_apos_end)
{
	cw_uint64_t	retval, apos;

	_cw_assert(a_apos_beg <= a_buf->gap_off || a_apos_beg >= a_buf->gap_off
	    + a_buf->gap_len);
	_cw_assert(a_apos_beg <= a_buf->len + a_buf->gap_len);
	_cw_assert(a_apos_end <= a_buf->gap_off || a_apos_end >= a_buf->gap_off
	    + a_buf->gap_len);
	_cw_assert(a_apos_end <= a_buf->len + a_buf->gap_len);

	retval = 0;

	/* Count the number of '\n' characters, taking care to avoid the gap. */
	for (apos = a_apos_beg; apos < a_apos_end && apos < a_buf->gap_off;
	    apos++) {
		if (bufc_char_get(a_buf->b[apos]) == '\n')
			retval++;
	}

	if (apos == a_buf->gap_off)
		apos += a_buf->gap_len;
	for (; apos < a_apos_end; apos++) {
		if (bufc_char_get(a_buf->b[apos]) == '\n')
			retval++;
	}

	return retval;
}

static void
buf_p_bufms_apos_adjust(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_bool_t a_exclude,
    cw_sint64_t a_adjust, cw_uint64_t a_beg_apos, cw_uint64_t a_end_apos)
{
	cw_bufm_t	*bufm;

	_cw_check_ptr(a_buf);
	_cw_check_ptr(a_bufm);
	_cw_assert(a_beg_apos < a_end_apos);

	/*
	 * Adjust apos field of affected bufm's. a_bufm is either in the
	 * affected range or adjacent to the empty region.  Starting at a_bufm,
	 * go both directions until out of the affected region or until past the
	 * beginning/end of the list.
	 */

	if (a_exclude == FALSE && a_bufm->apos >= a_beg_apos && a_bufm->apos <
	    a_end_apos) {
		a_bufm->apos += a_adjust;
	}

	/* Forward. */
	for (bufm = ql_next(&a_buf->bufms, a_bufm, link);
	     bufm != NULL && bufm->apos >= a_beg_apos && bufm->apos <
	     a_end_apos;
	     bufm = ql_next(&a_buf->bufms, bufm, link)) {
		fprintf(stderr, "%s:%u:%s(): Got here\n", __FILE__, __LINE__,
		    __FUNCTION__);
		bufm->apos += a_adjust;
	}

	/* Backward. */
	for (bufm = ql_prev(&a_buf->bufms, a_bufm, link);
	     bufm != NULL && bufm->apos >= a_beg_apos && bufm->apos <
	     a_end_apos;
	     bufm = ql_prev(&a_buf->bufms, bufm, link)) {
		fprintf(stderr, "%s:%u:%s(): Got here\n", __FILE__, __LINE__,
		    __FUNCTION__);
		bufm->apos += a_adjust;
	}
}

static void
buf_p_gap_move(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_bool_t a_exclude,
    cw_uint64_t a_bpos)
{
	cw_uint64_t	apos;

	_cw_assert(a_bpos > 0);
	_cw_assert(a_bpos <= a_buf->len + 1);

	apos = a_bpos - 1;

	/* Move the gap if it isn't already where it needs to be. */
	if (a_buf->gap_off != apos) {
		if (a_buf->gap_off < apos) {
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
			 *                   apos
			 *                   |
			 *                   v
			 * oooooooXXXXXXXXXXX________oo
			 */
			memmove(&a_buf->b[a_buf->gap_off],
			    &a_buf->b[a_buf->gap_off + a_buf->gap_len],
			    (apos - a_buf->gap_off) * sizeof(cw_bufc_t));

			/*
			 * Adjust the apos of all bufm's with apos in the moved
			 * region.
			 */
			buf_p_bufms_apos_adjust(a_buf, a_bufm, a_exclude,
			    -a_buf->gap_len, a_buf->gap_off + a_buf->gap_len,
			    apos + a_buf->gap_len);
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
			 *     apos
			 *     |
			 *     v
			 * oooo___________XXXXXXXXXoooo
			 */
			memmove(&a_buf->b[a_buf->gap_len + apos],
			    &a_buf->b[apos],
			    (a_buf->gap_off - apos) * sizeof(cw_bufc_t));

			/*
			 * Adjust the apos of all bufm's with apos in the moved
			 * region.
			 */
			buf_p_bufms_apos_adjust(a_buf, a_bufm, a_exclude,
			    a_buf->gap_len, apos, a_buf->gap_off);
		}
		a_buf->gap_off = apos;
	}
}

static void
buf_p_grow(cw_buf_t *a_buf, cw_uint64_t a_minlen)
{
	cw_uint64_t	old_size, new_size;

	old_size = a_buf->len + a_buf->gap_len;

	for (new_size = old_size << 1; new_size < a_minlen; new_size <<= 1) {
		/*
		 * Continue doubling the size of the buffer until it is big
		 * enough to contain a_minlen bufc's.
		 */
	}

	/* Move the gap to the end before reallocating. */
	buf_p_gap_move(a_buf, ql_last(&a_buf->bufms, link), FALSE, a_buf->len +
	    1);

	a_buf->b = (cw_bufc_t *)_cw_opaque_realloc(a_buf->realloc, a_buf->arg,
	    a_buf->b,new_size, old_size);

	/* Adjust the gap length. */
	a_buf->gap_len += new_size - old_size;
}

cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc, cw_opaque_realloc_t
    *a_realloc, cw_opaque_dealloc_t *a_dealloc, void *a_arg, cw_msgq_t *a_msgq)
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
		retval = (cw_buf_t *)_cw_opaque_alloc(a_alloc, a_arg,
		    sizeof(cw_buf_t));
		memset(retval, 0, sizeof(cw_buf_t));
		retval->alloced = TRUE;
	}

	retval->alloc = a_alloc;
	retval->realloc = a_realloc;
	retval->dealloc = a_dealloc;
	retval->arg = a_arg;

	retval->msgq = a_msgq;

	mtx_new(&retval->mtx);

	retval->b = (cw_bufc_t *)_cw_opaque_alloc(a_alloc, a_arg,
	    _CW_BUF_MINBUFCS * sizeof(cw_bufc_t));
	retval->len = 0;
	retval->nlines = 1;
	retval->gap_off = 0;
	retval->gap_len = _CW_BUF_MINBUFCS;

	/* Initialize history. */
	retval->hist_active = FALSE;
	bufh_p_new(&retval->hist);

	/* Initialize lists. */
	ql_new(&retval->bufms);

#ifdef _CW_DBG
	retval->magic = _CW_BUF_MAGIC;
#endif

	return retval;
}

void
buf_delete(cw_buf_t *a_buf)
{
	cw_bufm_t	*bufm;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	bufh_p_delete(&a_buf->hist);

	/*
	 * Set the buf pointers of all objects that point to this one to NULL,
	 * so that they won't try to disconnect during destruction.  All objects
	 * that reference this one effectively become invalid, but they can (and
	 * should) be destroyed even though this base buf is gone.
	 */
	ql_foreach(bufm, &a_buf->bufms, link) {
		bufm->buf = NULL;
	}

	_cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf->b, (a_buf->len +
	    a_buf->gap_len) * sizeof(cw_bufc_t));
	
	mtx_delete(&a_buf->mtx);

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
buf_len(cw_buf_t *a_buf)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	mtx_lock(&a_buf->mtx);
	retval = a_buf->len;
	mtx_unlock(&a_buf->mtx);

	return retval;
}

cw_bool_t
buf_hist_active_get(cw_buf_t *a_buf)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	mtx_lock(&a_buf->mtx);
	retval = a_buf->hist_active;
	mtx_unlock(&a_buf->mtx);

	return retval;
}

void
buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	mtx_lock(&a_buf->mtx);
	if (a_active == FALSE && a_buf->hist_active)
		buf_hist_flush(a_buf);
	a_buf->hist_active = a_active;
	mtx_unlock(&a_buf->mtx);
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

/* bufm. */
static void
bufm_p_insert(cw_bufm_t *a_bufm, cw_bool_t a_exclude, const cw_char_t *a_str,
    cw_uint64_t a_count)
{
	cw_uint64_t	i, nlines;

	/* Make sure that the string will fit. */
	if (a_count >= a_bufm->buf->gap_len)
		buf_p_grow(a_bufm->buf, a_bufm->buf->len + a_count);

	/* Move the gap. */
	buf_p_gap_move(a_bufm->buf, a_bufm, a_exclude,
	    buf_p_pos_a2b(a_bufm->buf, a_bufm->apos));

	/* Insert. */
	for (i = nlines = 0; i < a_count; i++) {
		a_bufm->buf->b[a_bufm->buf->gap_off + i] = a_str[i];
		if (bufc_char_get(a_str[i] == '\n'))
			nlines++;
	}

	/* Shrink the gap. */
	a_bufm->buf->gap_off += a_count;
	a_bufm->buf->gap_len -= a_count;

	/* Adjust the buf's length and line count. */
	a_bufm->buf->len += a_count;
	a_bufm->buf->nlines += nlines;

	if (nlines > 0) {
		cw_bufm_t	*bufm;

		/* Adjust line. */
		if (a_exclude == FALSE)
			a_bufm->line += nlines;

		/* Adjust line for all following bufm's. */
		for (bufm = ql_next(&a_bufm->buf->bufms, a_bufm, link);
		     bufm != NULL;
		     bufm = ql_next(&a_bufm->buf->bufms, bufm, link))
			bufm->line += nlines;
	}
}

cw_bufm_t *
bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf, cw_msgq_t *a_msgq)
{
	cw_bufm_t	*retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_bufm == NULL) {
		retval = (cw_bufm_t *)_cw_opaque_alloc(a_buf->alloc, a_buf->arg,
		    sizeof(cw_bufm_t));
		retval->dealloc = a_buf->dealloc;
		retval->arg = a_buf->arg;
	} else {
		retval = a_bufm;
		retval->dealloc = NULL;
		retval->arg = NULL;
	}

	ql_elm_new(retval, link);
	retval->buf = a_buf;
	retval->apos = 0;
	retval->line = 1;
	retval->msgq = a_msgq;

	ql_head_insert(&a_buf->bufms, retval, link);

#ifdef _CW_DBG
	retval->magic = _CW_BUFM_MAGIC;
#endif

	return retval;
}

void
bufm_dup(cw_bufm_t *a_to, cw_bufm_t *a_from)
{
	_cw_check_ptr(a_to);
	_cw_dassert(a_to->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_to->buf);
	_cw_check_ptr(a_from);
	_cw_dassert(a_from->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_from->buf);
	_cw_assert(a_to->buf == a_from->buf);

	a_to->apos = a_from->apos;

	ql_remove(&a_to->buf->bufms, a_to, link);
	ql_after_insert(a_from, a_to, link);

	if (a_to->msgq != NULL) {
		/* Send a message. */
		_cw_error("XXX Not implemented");
	}
}

void
bufm_delete(cw_bufm_t *a_bufm)
{
	_cw_check_ptr(a_bufm);

	if (a_bufm->buf != NULL)
		ql_remove(&a_bufm->buf->bufms, a_bufm, link);

	if (a_bufm->dealloc != NULL) {
		_cw_opaque_dealloc(a_bufm->dealloc, a_bufm->arg, a_bufm,
		    sizeof(cw_bufm_t));
	}
#ifdef _CW_DBG
	else
		memset(a_bufm, 0x5a, sizeof(cw_bufm_t));
#endif
}

cw_buf_t *
bufm_buf(cw_bufm_t *a_bufm)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	return a_bufm->buf;
}

cw_uint64_t
bufm_line_seek(cw_bufm_t *a_bufm, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);
	_cw_assert(a_offset > 0);

	mtx_lock(&a_bufm->buf->mtx);

	_cw_error("XXX Not implemented");

	if (a_bufm->msgq != NULL) {
		/* Send a message. */
		_cw_error("XXX Not implemented");
	}

	retval = a_bufm->apos; /* XXX */

	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

cw_uint64_t
bufm_line(cw_bufm_t *a_bufm)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);
	retval = a_bufm->line;
	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

cw_uint64_t
bufm_seek(cw_bufm_t *a_bufm, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	switch (a_whence) {
	case BUFW_BEG: {
		cw_uint64_t	bpos;
		cw_bufm_t	*bufm;

		/*
		 * Determine bpos and apos.  Make sure not to go out of buf
		 * bounds.
		 */
		if (a_offset < 0)
			bpos = 1;
		else if (a_offset > a_bufm->buf->len)
			bpos = a_bufm->buf->len + 1;
		else
			bpos = a_offset + 1;

		a_bufm->apos = buf_p_pos_b2a(a_bufm->buf, bpos);

		/*
		 * Relocate in the bufm list.
		 */
		ql_remove(&a_bufm->buf->bufms, a_bufm, link);

		for (bufm = ql_first(&a_bufm->buf->bufms);
		     bufm != NULL && a_bufm->apos > bufm->apos;
		     bufm = ql_next(&a_bufm->buf->bufms, bufm, link)) {
			/*
			 * Iterate until the end of the list is reached, or the
			 * apos of a bufm in the list is greater than that of
			 * the seeking bufm.
			 */
		}

		if (bufm == NULL) {
			/* Insert at end. */
			ql_tail_insert(&a_bufm->buf->bufms, a_bufm, link);
		} else {
			/* Insert before the last bufm looked at. */
			ql_before_insert(&a_bufm->buf->bufms, bufm, a_bufm,
			    link);
		}

		/*
		 * Count the number of newlines and set the line number
		 * accordingly.
		 */
		a_bufm->line = 1 + buf_p_lines_count(a_bufm->buf,
		    buf_p_pos_b2a(a_bufm->buf, 1), a_bufm->apos);

		break;
	}
	case BUFW_REL: {
		cw_uint64_t	apos, bpos;
		cw_bufm_t	*bufm;
		cw_bool_t	relocate = FALSE;

		/*
		 * The algorithm differs substantially depending whether seeking
		 * forward or backward.  There is slight code duplication in the
		 * two branches, but this avoids repeated branches.
		 */
		bpos = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);
		if (a_offset < 0) {
			/*
			 * Determine bpos and apos.  Make sure not to go out of
			 * buf bounds.
			 */
			if (bpos + a_offset < 1)
				bpos = 1;
			else
				bpos += a_offset;

			apos = buf_p_pos_b2a(a_bufm->buf, bpos);

			/*
			 * Relocate in the bufm list.
			 */
			for (bufm = ql_next(&a_bufm->buf->bufms, a_bufm, link);
			     bufm != NULL && a_bufm->apos > bufm->apos;
			     bufm = ql_next(&a_bufm->buf->bufms, bufm, link)) {
				/*
				 * Iterate until the end of the list is reached,
				 * or the apos of a bufm in the list is greater
				 * than that of the seeking bufm.
				 */
				relocate = TRUE;
			}

			if (relocate) {
				ql_remove(&a_bufm->buf->bufms, a_bufm, link);

				if (bufm == NULL) {
					/* Insert at end. */
					ql_tail_insert(&a_bufm->buf->bufms,
					    a_bufm, link);
				} else {
					/*
					 * Insert before the last bufm looked
					 * at.
					 */
					ql_before_insert(&a_bufm->buf->bufms,
					    bufm, a_bufm, link);
				}
			}

			/*
			 * Count the number of newlines moved past and adjust
			 * the line number accordingly.
			 */
			a_bufm->line += buf_p_lines_count(a_bufm->buf,
			    a_bufm->apos, apos);

			/*
			 * Set the apos of the bufm now that the old value isn't
			 * needed anymore.
			 */
			a_bufm->apos = apos;
		} else {
			/*
			 * Determine bpos and apos.  Make sure not to go out of
			 * buf bounds.
			 */
			if (bpos + a_offset > a_bufm->buf->len + 1)
				bpos = a_bufm->buf->len + 1;
			else
				bpos += a_offset;

			apos = buf_p_pos_b2a(a_bufm->buf, bpos);

			/*
			 * Relocate in the bufm list.
			 */
			for (bufm = ql_prev(&a_bufm->buf->bufms, a_bufm, link);
			     bufm != NULL && a_bufm->apos < bufm->apos;
			     bufm = ql_prev(&a_bufm->buf->bufms, bufm, link)) {
				/*
				 * Iterate until the beginning of the list is
				 * reached, or the apos of a bufm in the list is
				 * less than that of the seeking bufm.
				 */
				relocate = TRUE;
			}

			if (relocate) {
				ql_remove(&a_bufm->buf->bufms, a_bufm, link);

				if (bufm == NULL) {
					/* Insert at beginning. */
					ql_head_insert(&a_bufm->buf->bufms,
					    a_bufm, link);
				} else {
					/*
					 * Insert after the last bufm looked at.
					 */
					ql_after_insert(bufm, a_bufm, link);
				}
			}

			/*
			 * Count the number of newlines moved past and adjust
			 * the line number accordingly.
			 */
			a_bufm->line += buf_p_lines_count(a_bufm->buf,
			    apos, a_bufm->apos);

			/*
			 * Set the apos of the bufm now that the old value isn't
			 * needed anymore.
			 */
			a_bufm->apos = apos;
		}
		break;
	}
	case BUFW_END: {
		cw_uint64_t	bpos;
		cw_bufm_t	*bufm;

		/*
		 * Determine bpos and apos.  Make sure not to go out of buf
		 * bounds.
		 */
		if (a_offset > 0)
			bpos = a_bufm->buf->len + 1;
		else if (a_offset > a_bufm->buf->len)
			bpos = 1;
		else
			bpos = a_bufm->buf->len + 1 + a_offset;

		a_bufm->apos = buf_p_pos_b2a(a_bufm->buf, bpos);

		/*
		 * Relocate in the bufm list.
		 */
		ql_remove(&a_bufm->buf->bufms, a_bufm, link);

		for (bufm = ql_last(&a_bufm->buf->bufms, link);
		     bufm != NULL && a_bufm->apos < bufm->apos;
		     bufm = ql_prev(&a_bufm->buf->bufms, bufm, link)) {
			/*
			 * Iterate until the beginning of the list is reached,
			 * or the apos of a bufm in the list is less than that
			 * of the seeking bufm.
			 */
		}

		if (bufm == NULL) {
			/* Insert at beginning. */
			ql_head_insert(&a_bufm->buf->bufms, a_bufm, link);
		} else {
			/* Insert after the last bufm looked at. */
			ql_after_insert(bufm, a_bufm, link);
		}

		/*
		 * Count the number of newlines and set the line number
		 * accordingly.
		 */
		a_bufm->line = 1 + buf_p_lines_count(a_bufm->buf, 
		    buf_p_pos_b2a(a_bufm->buf, 1), a_bufm->apos);

		break;
	}
	default:
		_cw_not_reached();
	}

	if (a_bufm->msgq != NULL) {
		/* Send a message. */
		_cw_error("XXX Not implemented");
	}

	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

cw_uint64_t
bufm_pos(cw_bufm_t *a_bufm)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);
	retval = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);
	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

cw_bool_t
bufm_before_get(cw_bufm_t *a_bufm, cw_bufc_t *r_bufc)
{
	cw_bool_t	retval;
	cw_uint64_t	bpos;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	bpos = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);

	/* Make sure the marker isn't at BOB. */
	if (bpos == 1) {
		retval = TRUE;
		goto RETURN;
	}

	/* Don't use the marker's apos, in case it is next to the gap. */
	*r_bufc = a_bufm->buf->b[buf_p_pos_b2a(a_bufm->buf, bpos - 1)];

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_after_get(cw_bufm_t *a_bufm, cw_bufc_t *r_bufc)
{
	cw_bool_t	retval;
	cw_uint64_t	bpos;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	bpos = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);

	/* Make sure the marker isn't at EOB. */
	if (bpos == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	*r_bufc = a_bufm->buf->b[a_bufm->apos];

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_before_set(cw_bufm_t *a_bufm, cw_char_t a_char)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at BOB. */
	if (buf_p_pos_a2b(a_bufm->buf, a_bufm->apos) == 1) {
		retval = TRUE;
		goto RETURN;
	}

	_cw_error("XXX Not implemented");

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_after_set(cw_bufm_t *a_bufm, cw_char_t a_char)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at EOB. */
	if (buf_p_pos_a2b(a_bufm->buf, a_bufm->apos) == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	_cw_error("XXX Not implemented");

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_before_attrs_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at EOB. */
	if (buf_p_pos_a2b(a_bufm->buf, a_bufm->apos) == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	_cw_error("XXX Not implemented");

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_after_attrs_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at EOB. */
	if (buf_p_pos_a2b(a_bufm->buf, a_bufm->apos) == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	_cw_error("XXX Not implemented");

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

void
bufm_before_insert(cw_bufm_t *a_bufm, const cw_char_t *a_str, cw_uint64_t
    a_count)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	bufm_p_insert(a_bufm, FALSE, a_str, a_count);
	
	mtx_unlock(&a_bufm->buf->mtx);
}

void
bufm_after_insert(cw_bufm_t *a_bufm, const cw_char_t *a_str, cw_uint64_t
    a_count)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	bufm_p_insert(a_bufm, TRUE, a_str, a_count);

	mtx_unlock(&a_bufm->buf->mtx);
}

void
bufm_remove(cw_bufm_t *a_start, cw_bufm_t *a_end)
{
	_cw_check_ptr(a_start);
	_cw_dassert(a_start->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_start->buf);
	_cw_check_ptr(a_end);
	_cw_dassert(a_end->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_end->buf);
	_cw_assert(a_start->buf == a_end->buf);

	mtx_lock(&a_start->buf->mtx);

	_cw_error("XXX Not implemented");

	mtx_unlock(&a_start->buf->mtx);
}
