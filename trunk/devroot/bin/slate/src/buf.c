/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * This file contains a buffer gap implementation of buffers for use in the
 * slate text editor.  The code is broken up into the following classes:
 *
 * buf  : Main buffer class.
 * bufm : Marker.  Markers are used as handles for many buf operations.
 *
 ******************************************************************************
 *
 * Internal buffer representation:
 *
 * Buffer position numbering starts at 1.
 *
 * Absolute position:  0   1   2   3   4   5   6   7   8
 *                     |   |   |   |   |   |   |   |   |
 *                     v   v   v   v   v   v   v   v   v
 *                   /---+---+---+---+---+---+---+---\
 *                   | A | B | C |:::|:::|:::| D | E |
 *                   \---+---+---+---+---+---+---+---/
 *                   ^   ^   ^               ^   ^   ^
 *                   |   |   |               |   |   |
 * Buffer position:  1   2   3               4   5   6
 *
 * Buffer position 0 is invalid.
 *
 * Position rules:
 *
 * *) apos refers to absolute position.
 * *) bpos refers to buffer position.
 * *) If a position isn't specified as apos or bpos, then it is bpos.
 *
 * The size of elements in the buffer can be changed on a per-buffer basis on
 * the fly.  This allows character attributes to be stored with the buffer data,
 * but doesn't waste space for buffers that don't utilize character attributes.
 * The only significant impact that this has is on bufm_rang_get().  It is only
 * safe to treat the return value as a pointer to a string in the buffer if the
 * element size is 1.
 *
 ******************************************************************************/

#include "../include/modslate.h"

/* Prototypes. */
static cw_uint64_t buf_p_pos_b2a(cw_buf_t *a_buf, cw_uint64_t a_bpos);
static cw_uint64_t buf_p_pos_a2b(cw_buf_t *a_buf, cw_uint64_t a_apos);
static cw_uint64_t buf_p_lines_rel_forward_count(cw_buf_t *a_buf, cw_uint64_t
    a_apos_beg, cw_uint64_t a_nlines);
static cw_uint64_t buf_p_lines_rel_backward_count(cw_buf_t *a_buf, cw_uint64_t
    a_apos_beg, cw_uint64_t a_nlines);
static cw_uint64_t buf_p_lines_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
    cw_uint64_t a_apos_end);
static void buf_p_bufms_apos_adjust(cw_buf_t *a_buf, cw_bufm_t *a_bufm,
    cw_sint64_t a_adjust, cw_uint64_t a_beg_apos, cw_uint64_t a_end_apos);
static void buf_p_gap_move(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t
    a_bpos);
static void buf_p_grow(cw_buf_t *a_buf, cw_uint64_t a_minlen);
static void buf_p_shrink(cw_buf_t *a_buf);

cw_uint64_t
bufv_copy(cw_bufv_t *a_to, cw_uint32_t a_to_len, cw_uint32_t a_to_sizeof,
    const cw_bufv_t *a_fr, cw_uint32_t a_fr_len, cw_uint32_t a_fr_sizeof,
    cw_uint64_t a_maxlen)
{
	cw_uint64_t	retval;
	cw_uint32_t	to_el, fr_el, to_off, fr_off, cpysizeof;

	_cw_check_ptr(a_to);
	_cw_check_ptr(a_fr);

	if (a_to_sizeof <= a_fr_sizeof)
		cpysizeof = a_to_sizeof;
	else
		cpysizeof = a_fr_sizeof;

	retval = 0;
	to_el = 0;
	to_off = 0;
	/* Iterate over bufv elements. */
	for (fr_el = 0; fr_el < a_fr_len; fr_el++) {
		/* Iterate over bufv element contents. */
		for (fr_off = 0; fr_off < a_fr[fr_el].len; fr_off++) {
			memcpy(&a_to[to_el].data[to_off],
			    &a_fr[fr_el].data[fr_off], cpysizeof);

			/*
			 * Copy no more than a_maxlen elements (unless a_maxlen
			 * is 0).
			 */
			retval++;
			if (retval == a_maxlen)
				goto RETURN;

			/* Increment the position to copy to. */
			to_off++;
			if (to_off == a_to[to_el].len) {
				to_off = 0;
				to_el++;
				if (to_el == a_to_len)
					goto RETURN;
			}
		}
	}

	RETURN:
	return retval;
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

static cw_uint64_t
buf_p_lines_rel_forward_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
    cw_uint64_t a_nlines)
{
	cw_uint64_t	apos, nlines;

	/*
	 * Move to the "a_nlines"th '\n' character after a_apos_beg, taking care
	 * to avoid the gap.
	 */
	for (apos = a_apos_beg, nlines = 0; apos < a_buf->gap_off; apos++) {
		if (a_buf->b[apos * a_buf->elmsize] == '\n') {
			nlines++;
			if (nlines == a_nlines)
				goto DONE;
		}
	}

	if (apos == a_buf->gap_off) {
		/* Skip the gap. */
		apos += a_buf->gap_len;
	}

	for (;; apos++) {
		if (a_buf->b[apos * a_buf->elmsize] == '\n') {
			nlines++;
			if (nlines == a_nlines)
				goto DONE;
		}
	}

	DONE:
	return apos;
}

static cw_uint64_t
buf_p_lines_rel_backward_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
    cw_uint64_t a_nlines)
{
	cw_uint64_t	apos, nlines;

	/* Move past a_nlines '\n' characters, taking care to avoid the gap. */
	for (apos = a_apos_beg - 1, nlines = 0; apos >= a_buf->gap_off +
	    a_buf->gap_len; apos--) {
		if (a_buf->b[apos * a_buf->elmsize] == '\n') {
			nlines++;
			if (nlines == a_nlines)
				goto DONE;
		}
	}

	if (apos == a_buf->gap_off + a_buf->gap_len - 1) {
		/* Skip the gap. */
		apos -= a_buf->gap_len;
	}

	for (;; apos--) {
		if (a_buf->b[apos * a_buf->elmsize] == '\n') {
			nlines++;
			if (nlines == a_nlines)
				goto DONE;
		}
	}

	DONE:
	/*
	 * apos is now at the '\n', but we need to return the apos
	 * after the '\n'.  Add 1 to apos, then make sure it isn't in the gap.
	 */
	apos++;
	if (apos >= a_buf->gap_off && apos < a_buf->gap_off + a_buf->gap_len)
		apos += a_buf->gap_len;
	return apos;
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
	_cw_assert(a_apos_beg <= a_apos_end);

	retval = 0;

	/* Count the number of '\n' characters, taking care to avoid the gap. */
	for (apos = a_apos_beg; apos < a_apos_end && apos < a_buf->gap_off;
	    apos++) {
		if (a_buf->b[apos * a_buf->elmsize] == '\n')
			retval++;
	}

	if (apos == a_buf->gap_off)
		apos += a_buf->gap_len;
	for (; apos < a_apos_end; apos++) {
		if (a_buf->b[apos * a_buf->elmsize] == '\n')
			retval++;
	}

	return retval;
}

static void
buf_p_bufms_apos_adjust(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_sint64_t
    a_adjust, cw_uint64_t a_beg_apos, cw_uint64_t a_end_apos)
{
	cw_bufm_t	*bufm;

	_cw_check_ptr(a_buf);
	_cw_check_ptr(a_bufm);
	_cw_assert(a_beg_apos < a_end_apos);

	/*
	 * Adjust apos field of affected bufm's. a_bufm is either in or adjacent
	 * to the affected range.  Starting at a_bufm, go both directions until
	 * out of the affected range or until past the beginning/end of the
	 * list.  Extra care must be taken to ignore bufm's at the starting apos
	 * if a_bufm is merely adjacent to the affected region.
	 */

	/* Forward (including a_bufm). */
	for (bufm = a_bufm;
	    bufm != NULL && bufm->apos >= a_beg_apos && bufm->apos <
	    a_end_apos;
	    bufm = ql_next(&a_buf->bufms, bufm, link)) {
		bufm->apos += a_adjust;
	}

	/* Backward. */
	for (bufm = ql_prev(&a_buf->bufms, a_bufm, link);
	    bufm != NULL && bufm->apos == a_end_apos;
	    bufm = ql_prev(&a_buf->bufms, bufm, link))
		; /* Ignore. */

	for (;
	    bufm != NULL && bufm->apos >= a_beg_apos;
	    bufm = ql_prev(&a_buf->bufms, bufm, link))
		bufm->apos += a_adjust;
}

static void
buf_p_gap_move(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t a_bpos)
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
			memmove(&a_buf->b[a_buf->gap_off * a_buf->elmsize],
			    &a_buf->b[(a_buf->gap_off + a_buf->gap_len) *
			    a_buf->elmsize],
			    (apos - a_buf->gap_off) * a_buf->elmsize);

			/*
			 * Adjust the apos of all bufm's with apos in the moved
			 * region.
			 */
			buf_p_bufms_apos_adjust(a_buf, a_bufm, -a_buf->gap_len,
			    a_buf->gap_off + a_buf->gap_len, apos +
			    a_buf->gap_len);
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
			memmove(&a_buf->b[(a_buf->gap_len + apos) *
			    a_buf->elmsize],
			    &a_buf->b[apos * a_buf->elmsize],
			    (a_buf->gap_off - apos) * a_buf->elmsize);

			/*
			 * Adjust the apos of all bufm's with apos in the moved
			 * region.
			 */
			buf_p_bufms_apos_adjust(a_buf, a_bufm, a_buf->gap_len,
			    apos, a_buf->gap_off);
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
		 * Iteratively double new_size until it is big enough to contain
		 * a_minlen elements.
		 */
	}

	/* Move the gap to the end before reallocating. */
	buf_p_gap_move(a_buf, ql_last(&a_buf->bufms, link), a_buf->len + 1);

	a_buf->b = (cw_uint8_t *)_cw_opaque_realloc(a_buf->realloc, a_buf->arg,
	    a_buf->b, new_size * a_buf->elmsize, old_size * a_buf->elmsize);

	/* Adjust the gap length. */
	a_buf->gap_len += new_size - old_size;
}

static void
buf_p_shrink(cw_buf_t *a_buf)
{
	cw_uint64_t	old_size, new_size;

	old_size = a_buf->len + a_buf->gap_len;

	for (new_size = old_size;
	    (new_size >> 1) > a_buf->len && new_size > _CW_BUF_MINELMS;
	    new_size >>= 1) {
		/*
		 * Iteratively halve new_size until the actual buffer size is
		 * between 25% (exclusive) and 50% (inclusive) of new_size, or
		 * until new_size is the minimum size.
		 */
	}

	/*
	 * Only shrink the gap if the above loop determined that there is
	 * excessive space being used by the gap.
	 */
	if (old_size > new_size) {
		/* Move the gap to the end. */
		buf_p_gap_move(a_buf, ql_last(&a_buf->bufms, link), a_buf->len +
		    1);

		/* Shrink the gap. */
		a_buf->b = (cw_uint8_t *)_cw_opaque_realloc(a_buf->realloc,
		    a_buf->arg, a_buf->b, new_size * a_buf->elmsize,
		    old_size * a_buf->elmsize);

		/* Adjust the gap length. */
		a_buf->gap_len -= old_size - new_size;
	}
}

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
		retval = (cw_buf_t *)_cw_opaque_alloc(a_alloc, a_arg,
		    sizeof(cw_buf_t));
		memset(retval, 0, sizeof(cw_buf_t));
		retval->alloced = TRUE;
	}

	retval->alloc = a_alloc;
	retval->realloc = a_realloc;
	retval->dealloc = a_dealloc;
	retval->arg = a_arg;

	mtx_new(&retval->mtx);

	retval->elmsize = 1;
	retval->b = (cw_uint8_t *)_cw_opaque_alloc(a_alloc, a_arg,
	    _CW_BUF_MINELMS * retval->elmsize);
	retval->len = 0;
	retval->nlines = 1;
	retval->gap_off = 0;
	retval->gap_len = _CW_BUF_MINELMS;

	/* Initialize history. */
	retval->hist = NULL;

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
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);
	_cw_assert(ql_first(&a_buf->bufms) == NULL);

	if (a_buf->hist != NULL)
		hist_delete(a_buf->hist);

	_cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf->b, (a_buf->len +
	    a_buf->gap_len) * a_buf->elmsize);
	
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

void
buf_lock(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	mtx_lock(&a_buf->mtx);
}

void
buf_unlock(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	mtx_unlock(&a_buf->mtx);
}

cw_uint32_t
buf_elmsize_get(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	return a_buf->elmsize;
}

void
buf_elmsize_set(cw_buf_t *a_buf, cw_uint32_t a_elmsize)
{
	cw_uint8_t	*b;
	cw_uint64_t	i, size;
	cw_uint32_t	min_elmsize;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);
	_cw_assert(a_elmsize > 0);

	if (a_elmsize == a_buf->elmsize) {
		/* Do nothing. */
		return;
	}

	size = a_buf->len + a_buf->gap_len;

	/* Move the gap to the end to make things easier. */
	buf_p_gap_move(a_buf, ql_last(&a_buf->bufms, link), a_buf->len + 1);

	/* Allocate the new buffer. */
	b = (cw_uint8_t *)_cw_opaque_alloc(a_buf->alloc, a_buf->arg, size *
	    a_elmsize);

	/* XXX Use bufv_copy(). */
	/*
	 * Iteratively move data from the old buffer to the new one.  Preserve
	 * as many bytes per element as possible, which is the lesser of the two
	 * element sizes.
	 */
	if (a_elmsize > a_buf->elmsize)
		min_elmsize = a_buf->elmsize;
	else
		min_elmsize = a_elmsize;

	for (i = 0; i < a_buf->len; i++) {
		memcpy(&b[i * a_elmsize], &a_buf->b[i * a_buf->elmsize],
		    min_elmsize);
	}

	/* Free the old buffer. */
	_cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf->b, size *
	    a_buf->elmsize);

	/* Update the buffer pointer. */
	a_buf->b = b;

	/* Update the element size. */
	a_buf->elmsize = a_elmsize;
}

cw_uint64_t
buf_len(cw_buf_t *a_buf)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	retval = a_buf->len;

	return retval;
}

cw_uint64_t
buf_nlines(cw_buf_t *a_buf)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	retval = a_buf->nlines;

	return retval;
}

cw_bool_t
buf_hist_active_get(cw_buf_t *a_buf)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->hist != NULL)
		retval = TRUE;
	else
		retval = FALSE;

	return retval;
}

void
buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_active == TRUE && a_buf->hist == NULL) {
		a_buf->hist = hist_new(a_buf->alloc, a_buf->realloc,
		    a_buf->dealloc, a_buf->arg);
	} else if (a_active == FALSE && a_buf->hist != NULL) {
		hist_delete(a_buf->hist);
		a_buf->hist = NULL;
	}
}

cw_bool_t
buf_undoable(cw_buf_t *a_buf)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->hist == NULL) {
		retval = FALSE;
		goto RETURN;
	}

	retval = hist_undoable(a_buf->hist, a_buf);

	RETURN:
	return retval;
}

cw_bool_t
buf_redoable(cw_buf_t *a_buf)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->hist == NULL) {
		retval = FALSE;
		goto RETURN;
	}

	retval = hist_redoable(a_buf->hist, a_buf);

	RETURN:
	return retval;
}

cw_uint64_t
buf_undo(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t a_count)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->hist == NULL) {
		retval = 0;
		goto RETURN;
	}

	retval = hist_undo(a_buf->hist, a_buf, a_bufm, a_count);

	RETURN:
	return retval;
}

cw_uint64_t
buf_redo(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t a_count)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->hist == NULL) {
		retval = 0;
		goto RETURN;
	}

	retval = hist_redo(a_buf->hist, a_buf, a_bufm, a_count);

	RETURN:
	return retval;
}

void
buf_hist_flush(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->hist != NULL)
		hist_flush(a_buf->hist, a_buf);
}

void
buf_hist_group_beg(cw_buf_t *a_buf, cw_bufm_t *a_bufm)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->hist != NULL)
		hist_group_beg(a_buf->hist, a_buf, a_bufm);
}

cw_bool_t
buf_hist_group_end(cw_buf_t *a_buf)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->hist != NULL)
		retval = hist_group_end(a_buf->hist, a_buf);
	else
		retval = TRUE;

	return retval;
}

/* bufm. */
void
bufm_l_insert(cw_bufm_t *a_bufm, cw_bool_t a_record, cw_bool_t a_after, const
    cw_uint8_t *a_str, cw_uint64_t a_len)
{
	cw_uint64_t	i, nlines;
	cw_buf_t	*buf;
	cw_bufm_t	*first, *bufm;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	buf = a_bufm->buf;

	/*
	 * Record the undo information before inserting so that the apos is
	 * still unmodified.
	 */
	if (buf->hist != NULL && a_record) {
		if (a_after) {
			hist_ynk(buf->hist, buf, buf_p_pos_a2b(buf,
			    a_bufm->apos), a_str, a_len);
		} else {
			hist_ins(buf->hist, buf, buf_p_pos_a2b(buf,
			    a_bufm->apos), a_str, a_len);
		}
	}

	/* Make sure that the string will fit. */
	if (a_len >= buf->gap_len)
		buf_p_grow(buf, buf->len + a_len);

	/* Move the gap. */
	buf_p_gap_move(buf, a_bufm, buf_p_pos_a2b(buf, a_bufm->apos));

	/* Insert. */
	for (i = nlines = 0; i < a_len; i++) {
		buf->b[(buf->gap_off + i) * buf->elmsize] = a_str[i];
		if (a_str[i] == '\n')
			nlines++;
	}

	/*
	 * If there are multiple bufm's at the same position as a_bufm, make
	 * sure that a_bufm is the first bufm in the bufm list, in order to
	 * simplify later list iteration operations and allow moving a_bufm.
	 */
	for (first = NULL, bufm = ql_prev(&buf->bufms, a_bufm, link);
	    bufm != NULL && bufm->apos == a_bufm->apos;
	    bufm = ql_prev(&buf->bufms, bufm, link))
		first = bufm;

	if (first != NULL) {
		ql_remove(&buf->bufms, a_bufm, link);
		ql_before_insert(&buf->bufms, first, a_bufm, link);
	}
	
	/*
	 * If inserting after a_bufm, move a_bufm before the data just inserted.
	 * This relies on the bufm list re-ordering above, since moving a_bufm
	 * would otherwise require re-insertion into the bufm list.
	 */
	if (a_after)
		a_bufm->apos = buf->gap_off;

	/* Shrink the gap. */
	buf->gap_off += a_len;
	buf->gap_len -= a_len;

	/* Adjust the buf's length and line count. */
	buf->len += a_len;
	buf->nlines += nlines;

	if (nlines > 0) {
		/* Adjust line. */
		if (a_after == FALSE) {
			a_bufm->line += nlines;

			/* Adjust line for all bufm's at the same position. */
			for (bufm = ql_next(&buf->bufms, a_bufm, link);
			     bufm != NULL && bufm->apos == a_bufm->apos;
			     bufm = ql_next(&buf->bufms, bufm, link))
				bufm->line += nlines;
		} else {
			/* Move past bufm's at the same position. */
			for (bufm = ql_next(&buf->bufms, a_bufm, link);
			     bufm != NULL && bufm->apos == a_bufm->apos;
			     bufm = ql_next(&buf->bufms, bufm, link))
				; /* Do nothing. */
		}

		/* Adjust line for all following bufm's. */
		for (;
		     bufm != NULL;
		     bufm = ql_next(&buf->bufms, bufm, link))
			bufm->line += nlines;
	}
}

void
bufm_l_remove(cw_bufm_t *a_start, cw_bufm_t *a_end, cw_bool_t a_record)
{
	cw_buf_t	*buf;
	cw_bufm_t	*start, *end, *bufm;
	cw_uint64_t	start_bpos, end_bpos, rcount, nlines;

	_cw_check_ptr(a_start);
	_cw_dassert(a_start->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_start->buf);
	_cw_check_ptr(a_end);
	_cw_dassert(a_end->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_end->buf);
	_cw_assert(a_start->buf == a_end->buf);

	if (a_start->apos < a_end->apos) {
		start = a_start;
		end = a_end;
	} else if (a_start->apos > a_end->apos) {
		start = a_end;
		end = a_start;
	} else {
		/* No data need to be removed. */
		return;
	}

	buf = start->buf;

	/* Get bpos for start and end, since they are used more than once. */
	start_bpos = buf_p_pos_a2b(buf, start->apos);
	end_bpos = buf_p_pos_a2b(buf, end->apos);

	/*
	 * Calculate the number of elements being removed, since it is used more
	 * than once.
	 */
	rcount = end_bpos - start_bpos;

	/* Move the gap. */
	buf_p_gap_move(buf, start, start_bpos);

	/*
	 * Record undo information, now that the gap has been moved and the
	 * elements to be removed are contiguous.  The ordering of a_start and
	 * a_end determines whether this is a before/after removal.
	 */
	if (buf->hist != NULL && a_record) {
		if (start == a_start) {
			hist_del(buf->hist, buf, buf_p_pos_a2b(buf,
			    start->apos), &buf->b[start->apos * buf->elmsize],
			    rcount);
		} else {
			hist_rem(buf->hist, buf, buf_p_pos_a2b(buf, end->apos),
			    &buf->b[start->apos * buf->elmsize], rcount);
		}
	}

	/* Grow the gap. */
	buf->gap_len += rcount;

	/*
	 * Adjust apos for all bufm's before start that are at the same
	 * position.
	 */
	for (bufm = ql_prev(&buf->bufms, start, link);
	     bufm != NULL && bufm->apos == start->apos;
	     bufm = ql_prev(&buf->bufms, bufm, link))
		bufm->apos = end->apos;

	/*
	 * Adjust apos and line for all bufm's from start (inclusive) to end
	 * (exclusive).
	 */
	for (bufm = start;
	     bufm->apos < end->apos;
	     bufm = ql_next(&buf->bufms, bufm, link)) {
		bufm->apos = end->apos;
		bufm->line = start->line;
	}
	
	nlines = end->line - start->line;

	/*
	 * Adjust the line for all bufm's after the gap.
	 */
	for (;
	     bufm != NULL;
	     bufm = ql_next(&buf->bufms, bufm, link)) {
		bufm->line -= nlines;
	}

	/* Adjust the buf's len and nlines. */
	buf->len -= rcount;
	buf->nlines -= nlines;

	/* Try to shrink the gap. */
	buf_p_shrink(buf);
}

cw_bufm_t *
bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf)
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
	retval->apos = buf_p_pos_b2a(a_buf, 1);
	retval->line = 1;

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
	a_to->line = a_from->line;

	ql_remove(&a_to->buf->bufms, a_to, link);
	ql_after_insert(a_from, a_to, link);
}

void
bufm_delete(cw_bufm_t *a_bufm)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

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
	cw_bufm_t	*bufm;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	/*
	 * When checking for attempted seeking out of buf bounds, it is
	 * important to handle the cases of movement to exactly BOB or EOB,
	 * since there is one more line than there are '\n' characters, and the
	 * iteration algorithm would have to check for BOB/EOB if it couldn't be
	 * sure that it would stay in bounds.
	 */
	switch (a_whence) {
	case BUFW_BEG:
		/*
		 * Make sure not to go out of buf bounds.
		 */
		if (a_offset <= 0) {
			/* Attempt to move to or before BOB.  Move to BOB. */
			bufm_seek(a_bufm, 0, BUFW_BEG);
			break;
		} else if (a_offset >= a_bufm->buf->nlines) {
			/* Attempt to move to or past EOB.  Move to EOB. */
			bufm_seek(a_bufm, 0, BUFW_END);
			break;
		}

		/*
		 * Move forward from BOB to just short of a_offset '\n'
		 * characters.  For example, if seeking forward 2:
		 *
		 *  \/
		 *   hello\ngoodbye\nyadda\nblah
		 *                /\
		 */
		a_bufm->apos = buf_p_lines_rel_forward_count(a_bufm->buf,
		    buf_p_pos_b2a(a_bufm->buf, 1), a_offset);

		/* Set the line number. */
		a_bufm->line = a_offset;

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
		break;
	case BUFW_REL: {
		cw_uint64_t	apos;
		cw_bool_t	relocate = FALSE;
		
		if (a_offset > 0) {
			/*
			 * Make sure not to go out of buf bounds.
			 */
			if (a_bufm->line + a_offset > a_bufm->buf->nlines) {
				/*
				 * Attempt to move to or after EOB.  Move to
				 * EOB.
				 */
				bufm_seek(a_bufm, 0, BUFW_END);
				break;
			}
			/*
			 * Move forward from the current position to just short
			 * of a_offset '\n' characters.  Fore example, if
			 * seeking forward 2:
			 *
			 *            \/
			 *   hello\ngoodbye\nyadda\nblah
			 *                       /\
			 */
			apos = buf_p_lines_rel_forward_count(a_bufm->buf,
			    a_bufm->apos, a_offset);

			/*
			 * Relocate in the bufm list.
			 */
			for (bufm = ql_next(&a_bufm->buf->bufms, a_bufm, link);
			     bufm != NULL && apos > bufm->apos;
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

			/* Set the line number. */
			a_bufm->line += a_offset - 1;

			/*
			 * Set the apos of the bufm now that the old value isn't
			 * needed anymore.
			 */
			a_bufm->apos = apos;
		} else if (a_offset < 0) {
			/*
			 * Make sure not to go out of buf bounds.
			 */
			if (-a_offset >= a_bufm->line) {
				/*
				 * Attempt to move to or before BOB.  Move to
				 * BOB.
				 */
				bufm_seek(a_bufm, 0, BUFW_BEG);
				break;
			}

			/*
			 * Move backward from the current position to just short
			 * of a_offset '\n' characters.  Fore example, if
			 * seeking backward 2:
			 *
			 *                     \/
			 *   hello\ngoodbye\nyadda\nblah
			 *         /\
			 */
			apos = buf_p_lines_rel_backward_count(a_bufm->buf,
			    a_bufm->apos, -a_offset);
			
			/*
			 * Relocate in the bufm list.
			 */
			for (bufm = ql_prev(&a_bufm->buf->bufms, a_bufm, link);
			     bufm != NULL && apos < bufm->apos;
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

			/* Set the line number. */
			a_bufm->line += a_offset + 1;

			/*
			 * Set the apos of the bufm now that the old value isn't
			 * needed anymore.
			 */
			a_bufm->apos = apos;
		}

		break;
	}
	case BUFW_END:
		/*
		 * Make sure not to go out of buf bounds.
		 */
		if (a_offset >= 0) {
			/* Attempt to move to or after EOB.  Move to EOB. */
			bufm_seek(a_bufm, 0, BUFW_END);
			break;
		} else if (a_offset >= a_bufm->buf->nlines) {
			/* Attempt to move to or past BOB.  Move to BOB. */
			bufm_seek(a_bufm, 0, BUFW_BEG);
			break;
		}

		/*
		 * Move backward from EOB to just short of a_offset '\n'
		 * characters.  For example if seeking backward 2:
		 *
		 *                             \/
		 *   hello\ngoodbye\nyadda\nblah
		 *                  /\
		 */
		a_bufm->apos = buf_p_lines_rel_backward_count(a_bufm->buf,
		    buf_p_pos_b2a(a_bufm->buf, a_bufm->buf->len + 1),
		    -a_offset);

		/* Set the line number. */
		a_bufm->line = a_bufm->buf->nlines + a_offset + 1;

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
		break;
	default:
		_cw_not_reached();
	}

	return buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);
}

cw_uint64_t
bufm_line(cw_bufm_t *a_bufm)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	retval = a_bufm->line;

	return retval;
}

cw_uint64_t
bufm_seek(cw_bufm_t *a_bufm, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
	cw_uint64_t	bpos;
	cw_bufm_t	*bufm;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	switch (a_whence) {
	case BUFW_BEG:
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
	case BUFW_REL: {
		cw_uint64_t	apos;
		cw_bool_t	relocate = FALSE;

		/*
		 * The algorithm differs substantially depending whether seeking
		 * forward or backward.  There is slight code duplication in the
		 * two branches, but this avoids repeated branches.
		 */
		bpos = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);
		if (a_offset > 0) {
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
			for (bufm = ql_next(&a_bufm->buf->bufms, a_bufm, link);
			     bufm != NULL && apos > bufm->apos;
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
		} else if (a_offset < 0) {
			/*
			 * Determine bpos and apos.  Make sure not to go out of
			 * buf bounds.
			 */
			if (bpos <= -a_offset)
				bpos = 1;
			else
				bpos += a_offset;

			apos = buf_p_pos_b2a(a_bufm->buf, bpos);

			/*
			 * Relocate in the bufm list.
			 */
			for (bufm = ql_prev(&a_bufm->buf->bufms, a_bufm, link);
			     bufm != NULL && apos < bufm->apos;
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
			a_bufm->line -= buf_p_lines_count(a_bufm->buf,
			    apos, a_bufm->apos);

			/*
			 * Set the apos of the bufm now that the old value isn't
			 * needed anymore.
			 */
			a_bufm->apos = apos;
		}

		break;
	}
	case BUFW_END:
		/*
		 * Determine bpos and apos.  Make sure not to go out of buf
		 * bounds.
		 */
		if (a_offset > 0)
			bpos = a_bufm->buf->len + 1;
		else if (-a_offset >= a_bufm->buf->len)
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
	default:
		_cw_not_reached();
	}

	return bpos;
}

cw_uint64_t
bufm_pos(cw_bufm_t *a_bufm)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	retval = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);

	return retval;
}

cw_uint8_t *
bufm_before_get(cw_bufm_t *a_bufm)
{
	cw_uint8_t	*retval;
	cw_uint64_t	bpos;
	cw_buf_t	*buf;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	buf = a_bufm->buf;

	bpos = buf_p_pos_a2b(buf, a_bufm->apos);

	/* Make sure the marker isn't at BOB. */
	if (bpos == 1) {
		retval = NULL;
		goto RETURN;
	}

	/* Don't use the marker's apos, in case it is next to the gap. */
	retval = &buf->b[buf_p_pos_b2a(buf, bpos - 1) * buf->elmsize];

	RETURN:
	return retval;
}

cw_uint8_t *
bufm_after_get(cw_bufm_t *a_bufm)
{
	cw_uint8_t	*retval;
	cw_uint64_t	bpos;
	cw_buf_t	*buf;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	buf = a_bufm->buf;

	bpos = buf_p_pos_a2b(buf, a_bufm->apos);

	/* Make sure the marker isn't at EOB. */
	if (bpos == buf->len + 1) {
		retval = NULL;
		goto RETURN;
	}

	retval = &buf->b[a_bufm->apos * buf->elmsize];

	RETURN:
	return retval;
}

cw_bufv_t *
bufm_range_get(cw_bufm_t *a_start, cw_bufm_t *a_end, cw_uint32_t *r_bufvcnt)
{
	cw_bufm_t	*start, *end;
	cw_buf_t	*buf;

	_cw_check_ptr(a_start);
	_cw_dassert(a_start->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_start->buf);
	_cw_check_ptr(a_end);
	_cw_dassert(a_end->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_end->buf);
	_cw_assert(a_start->buf == a_end->buf);
	_cw_check_ptr(r_bufvcnt);

	buf = a_start->buf;

	if (a_start->apos < a_end->apos) {
		start = a_start;
		end = a_end;
	} else if (a_start->apos > a_end->apos) {
		start = a_end;
		end = a_start;
	} else {
		/* There are no characters between the two bufm's. */
		*r_bufvcnt = 0;
		goto RETURN;
	}

	/*
	 * Set the bufv according to whether the range is split accros the gap.
	 */
	if (buf->gap_off + buf->gap_len <= start->apos || buf->gap_off >
	    end->apos) {
		/* Not split. */
		buf->bufv[0].data = &buf->b[start->apos * buf->elmsize];
		buf->bufv[0].len = end->apos - start->apos;

		*r_bufvcnt = 1;
	} else {
		/* Split. */
		buf->bufv[0].data = &buf->b[start->apos * buf->elmsize];
		buf->bufv[0].len = buf->gap_off - start->apos;

		buf->bufv[1].data = &buf->b[(buf->gap_off + buf->gap_len)
		    * buf->elmsize];
		buf->bufv[1].len = end->apos - buf->gap_off - buf->gap_len;

		*r_bufvcnt = 2;
	}

	RETURN:
	return buf->bufv;
}

void
bufm_before_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_str, cw_uint64_t
    a_len)
{
	bufm_l_insert(a_bufm, TRUE, FALSE, a_str, a_len);
}

void
bufm_after_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_str, cw_uint64_t a_len)
{
	bufm_l_insert(a_bufm, TRUE, TRUE, a_str, a_len);
}

void
bufm_remove(cw_bufm_t *a_start, cw_bufm_t *a_end)
{
	bufm_l_remove(a_start, a_end, TRUE);
}
