/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * This file contains an implementation of buffers for use in the nxe text
 * editor.  The code is broken up into the following classes:
 *
 * buf  : Main buffer class.
 * bufp : Buffer page.  Internally, buf data are composed of bufp's, which store
 *        buf data in a paged buffer gap format.
 * bufc : Character.  Characters are merely 8 bit values.
 * bufm : Marker.  Markers are used as handles for many buf operations.
 * bufh : History.  The bufh class provides an abstract history mechanism to the
 *        buf class and implements infinite undo and redo.
 *
 * buf's and everything associated with them are implicitly synchronized.
 *
 * Buffer/character position numbering starts at 1.
 *
 * bufp pos:  0           1     0   1   2   3         0   1   2
 *
 * Char pos:  1           2     3   4   5   6         7   8   9
 *            |           |     |   |   |   |         |   |   |
 *            v           v     v   v   v   v         v   v   v
 *          /---+---+---+---\ /---+---+---+---\ /---+---+---+---\
 *          | A |:::|:::| B | | C | D | E | F | |:::| G | H | I |
 *          \---+---+---+---/ \---+---+---+---/ \---+---+---+---/
 *          ^   ^           ^     ^   ^   ^   ^         ^   ^   ^
 *          |   |           |     |   |   |   |         |   |   |
 * Buf pos: 1   2           3     4   5   6   7         8   9  10
 *
 * Position 0 is invalid, and that there is one more buffer position than there
 * are character positions.  Externally, character positions are never
 * mentioned, but internally, the difference matters at bufp boundaries.  When
 * inserting, we use buffer positions, and when reading/writing/removing, we use
 * character positions.
 *
 * Buffer position N and character position N - 1 are always in the same bufp,
 * so internally we use character positions everywhere.  Special care must be
 * given to buffer position 1, since there is no actual character 0.
 *
 * Position rules:
 *
 * *) ppos (bufp position) start counting at 0; it's up to the buf code to
 *    translate.
 * *) bpos refers to buffer position.
 * *) cpos refers to character position.
 * *) If a position isn't specified as bpos, cpos, or ppos, then it is bpos.
 *
 ******************************************************************************/

#include "../include/nxe.h"

/*
 * Prototypes.
 */

/* bufp. */
static void	bufp_p_new(cw_bufp_t *a_bufp);
static void	bufp_p_gap_move(cw_bufp_t *a_bufp, cw_uint32_t a_ppos);
static cw_bufc_t *bufp_p_ppos_get(cw_bufp_t *a_bufp, cw_uint32_t a_ppos);
static void	bufp_p_insert(cw_bufp_t *a_bufp, cw_uint32_t a_ppos, const
    cw_bufc_t *a_data, cw_uint64_t a_len);
static void	bufp_p_split(cw_bufp_t *a_bufp, cw_bufp_t *a_after, cw_uint32_t
    a_ppos);
static void	bufp_p_remove(cw_bufp_t *a_bufp, cw_uint32_t a_ppos,
    cw_uint32_t a_len);
static cw_bufc_t *bufp_p_cpos_get(cw_bufp_t *a_bufp, cw_uint64_t a_cpos);
static cw_uint64_t bufp_p_line_at_cpos(cw_bufp_t *a_bufp, cw_uint64_t a_cpos);

#define	bufp_p_gap_len_get(a_bufp) (a_bufp)->gap_len
#define	bufp_p_len_get(a_bufp)						\
	(_CW_BUFP_NBUFC - bufp_p_gap_len_get(a_bufp))
#define	bufp_p_nlines_get(a_bufp) ((cw_uint64_t)(a_bufp)->nlines)
#define	bufp_p_ecpos_get(a_bufp) (a_bufp)->ecpos
#define	bufp_p_ecpos_set(a_bufp, a_ecpos)				\
	(a_bufp)->ecpos = (a_ecpos) + bufp_p_len_get(a_bufp)
#define	bufp_p_eline_get(a_bufp) (a_bufp)->eline
#define	bufp_p_eline_set(a_bufp, a_eline)				\
	(a_bufp)->eline = (a_eline) + bufp_p_nlines_get(a_bufp)

/* bufh. */
static void	bufh_p_new(cw_bufh_t *a_bufh);
static void	bufh_p_delete(cw_bufh_t *a_bufh);

/* buf. */
static cw_uint64_t buf_p_bufp_at_cpos(cw_buf_t *a_buf, cw_uint64_t a_cpos);
static cw_uint64_t buf_p_line_at_cpos(cw_buf_t *a_buf, cw_uint64_t a_cpos);
static void	buf_p_bufp_cache_update(cw_buf_t *a_buf, cw_uint64_t a_cpos);
static void	buf_p_bufm_insert(cw_buf_t *a_buf, cw_bufm_t *a_bufm);
static void	buf_p_bufms_update(cw_buf_t *a_buf, cw_bufm_t *a_valid,
    cw_sint64_t a_amount);

/* bufm. */
static void	bufm_p_bufp_cache(cw_bufm_t *a_bufm);
static void	bufm_p_rel_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount);
static void	bufm_p_insert(cw_bufm_t *a_bufm, const cw_char_t *a_str,
    cw_uint64_t a_count);

/*
 * Code.
 */

/* bufp. */
static void
bufp_p_new(cw_bufp_t *a_bufp)
{
	_cw_check_ptr(a_bufp);
	_cw_assert(sizeof(cw_bufp_t) == _CW_BUFP_SIZE);

	a_bufp->nlines = 0;
	a_bufp->gap_off = 0;
	a_bufp->gap_len = _CW_BUFP_NBUFC;
}

static void
bufp_p_gap_move(cw_bufp_t *a_bufp, cw_uint32_t a_ppos)
{
	_cw_check_ptr(a_bufp);
	_cw_assert(a_ppos <= bufp_p_len_get(a_bufp));

	/* Move the gap if it isn't already at the point of insertion. */
	if (a_bufp->gap_off != a_ppos) {
		if (a_bufp->gap_off < a_ppos) {
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
			 *                   a_ppos
			 *                   |
			 *                   v
			 * oooooooXXXXXXXXXXX________oo
			 */
			memmove(&a_bufp->data[a_bufp->gap_off],
			    &a_bufp->data[a_bufp->gap_off + a_bufp->gap_len],
			    (a_ppos - a_bufp->gap_off) * sizeof(cw_bufc_t));
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
			 *     a_ppos
			 *     |
			 *     v
			 * oooo___________XXXXXXXXXoooo
			 */
			memmove(&a_bufp->data[a_bufp->gap_len + a_ppos],
			    &a_bufp[a_ppos],
			    (a_bufp->gap_off - a_ppos) * sizeof(cw_bufc_t));
		}
		a_bufp->gap_off = a_ppos;
	}
}

static cw_bufc_t *
bufp_p_ppos_get(cw_bufp_t *a_bufp, cw_uint32_t a_ppos)
{
	cw_bufc_t	*retval;

	_cw_check_ptr(a_bufp);
	_cw_assert(a_ppos < bufp_p_len_get(a_bufp));

	if (a_ppos < a_bufp->gap_off)
		retval = &a_bufp->data[a_ppos];
	else
		retval = &a_bufp->data[a_ppos + a_bufp->gap_len];

	return retval;
}

static void
bufp_p_insert(cw_bufp_t *a_bufp, cw_uint32_t a_ppos, const cw_bufc_t *a_data,
    cw_uint64_t a_len)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_bufp);
	_cw_assert(a_len <= a_bufp->gap_len);
	_cw_assert(a_ppos + a_len <= _CW_BUFP_NBUFC);

	bufp_p_gap_move(a_bufp, a_ppos);
	for (i = 0; i < a_len; i++) {
		a_bufp->data[a_ppos + i] = a_data[i];
		if (bufc_char_get(a_data[i]) == '\n')
			a_bufp->nlines++;
	}
	a_bufp->gap_off += a_len;
	a_bufp->gap_len -= a_len;
}

/*
 * Split a_bufp into two bufp's at position a_ppos.  a_after is assumed to have
 * enough space.
 */
static void
bufp_p_split(cw_bufp_t *a_bufp, cw_bufp_t *a_after, cw_uint32_t a_ppos)
{
	_cw_check_ptr(a_bufp);
	_cw_check_ptr(a_after);
	_cw_assert(bufp_p_gap_len_get(a_after) >= _CW_BUFP_NBUFC -
	    a_bufp->gap_off - a_bufp->gap_len);
	_cw_assert(a_ppos <= _CW_BUFP_NBUFC - a_bufp->gap_len);

	bufp_p_gap_move(a_bufp, a_ppos);
	bufp_p_insert(a_after, 0, &a_bufp->data[a_bufp->gap_off +
	    a_bufp->gap_len], _CW_BUFP_NBUFC - a_bufp->gap_off -
	    a_bufp->gap_len);

	a_bufp->gap_len = _CW_BUFP_NBUFC - a_bufp->gap_off;
	a_bufp->nlines -= bufp_p_nlines_get(a_after);
}

static void
bufp_p_remove(cw_bufp_t *a_bufp, cw_uint32_t a_ppos, cw_uint32_t a_len)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_bufp);
	_cw_assert(a_ppos + a_len <= bufp_p_len_get(a_bufp));

	bufp_p_gap_move(a_bufp, a_ppos);
	for (i = 0; i < a_len; i++) {
		if (bufc_char_get(a_bufp->data[a_bufp->gap_off + a_bufp->gap_len
		    + i]) == '\n')
			a_bufp->nlines--;
	}
	a_bufp->gap_len += a_len;
}

static cw_bufc_t *
bufp_p_cpos_get(cw_bufp_t *a_bufp, cw_uint64_t a_cpos)
{
	_cw_check_ptr(a_bufp);
	_cw_assert(a_cpos <= a_bufp->ecpos);
	
	/*
	 * We can assume that ecpos is valid.
	 */
	return bufp_p_ppos_get(a_bufp, a_cpos + bufp_p_len_get(a_bufp) -
	    a_bufp->ecpos);
}

static cw_uint64_t
bufp_p_line_at_cpos(cw_bufp_t *a_bufp, cw_uint64_t a_cpos)
{
	cw_uint64_t	i, count, retval;

	_cw_assert(a_cpos <= a_bufp->ecpos);
	_cw_assert(a_cpos > a_bufp->ecpos - bufp_p_len_get(a_bufp));

	/*
	 * We can assume that ecpos and eline are valid.
	 *
	 * Iterate upward, since it is faster on many architectures.  Avoid the
	 * gap.
	 */
	retval = a_bufp->eline - a_bufp->nlines;
	count = a_cpos - a_bufp->ecpos + bufp_p_len_get(a_bufp);

	for (i = 0; i < count && i < a_bufp->gap_off; i++) {
		if (bufc_char_get(a_bufp->data[i]) == '\n')
			retval++;
	}

	for (i = a_bufp->gap_off + a_bufp->gap_len; i < count; i++) {
		if (bufc_char_get(a_bufp->data[i]) == '\n')
			retval++;
	}

	return retval;
}

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
buf_p_bufp_at_cpos(cw_buf_t *a_buf, cw_uint64_t a_cpos)
{
	cw_uint64_t	index, first, last;

	/*
	 * Make sure the cache reaches far enough to be useful for the
	 * search.
	 */
	buf_p_bufp_cache_update(a_buf, a_cpos);

	/*
	 * Do a binary search for the correct bufp using the bufp character
	 * position cache.  "first" is the index of the first element in the
	 * range to search, and "last" is one plus the index of the last element
	 * in the range to search.
	 */
	first = 0;
	last = a_buf->bufp_count;
	if (a_cpos < bufp_p_ecpos_get(a_buf->bufp_vec[first]))
		index = first;
	else if (a_cpos == a_buf->len)
		index = last - 1;
	else {
		for (;;) {
			index = (first + last) >> 1;

			_cw_assert(index < a_buf->bufp_count);
			if (bufp_p_ecpos_get(a_buf->bufp_vec[index]) < a_cpos) {
				first = index + 1;
			} else if (bufp_p_ecpos_get(a_buf->bufp_vec[index - 1])
			    > a_cpos) {
				last = index;
			} else
				break;
		}
	}
	_cw_assert(index < a_buf->bufp_count);

	return index;
}

static cw_uint64_t
buf_p_line_at_cpos(cw_buf_t *a_buf, cw_uint64_t a_cpos)
{
	cw_uint64_t	retval, index;

	_cw_assert(a_cpos <= a_buf->len);

	if (a_cpos == 0) {
		retval = 1;
		goto RETURN;
	}

	index = buf_p_bufp_at_cpos(a_buf, a_cpos);
	retval = bufp_p_line_at_cpos(a_buf->bufp_vec[index], a_cpos);
	RETURN:
	return retval;
}

static void
buf_p_bufp_cache_update(cw_buf_t *a_buf, cw_uint64_t a_cpos)
{
	cw_uint64_t	ecpos, eline;

	_cw_assert(a_cpos <= a_buf->len);

	/*
	 * Update the cache so that it can be used for searches up to and
	 * including a_cpos.
	 */
	if (a_buf->ncached > 0) {
		ecpos = bufp_p_ecpos_get(a_buf->bufp_vec[a_buf->ncached - 1]);
		eline = bufp_p_eline_get(a_buf->bufp_vec[a_buf->ncached - 1]);
	} else {
		ecpos = 0;
		eline = 1;
	}
	for (; ecpos <= a_cpos && a_buf->ncached < a_buf->bufp_count;
	     a_buf->ncached++) {
		bufp_p_ecpos_set(a_buf->bufp_vec[a_buf->ncached], ecpos);
		ecpos = bufp_p_ecpos_get(a_buf->bufp_vec[a_buf->ncached]);

		bufp_p_eline_set(a_buf->bufp_vec[a_buf->ncached], eline);
		eline = bufp_p_eline_get(a_buf->bufp_vec[a_buf->ncached]);
	}
}

static void
buf_p_bufm_insert(cw_buf_t *a_buf, cw_bufm_t *a_bufm)
{
	cw_bufm_t	*bufm;

	ql_foreach(bufm, &a_buf->bufms, link) {
		if (a_bufm->bpos >= bufm->bpos) {
			ql_after_insert(bufm, a_bufm, link);
			return;
		}
	}
	ql_head_insert(&a_buf->bufms, a_bufm, link);
}

/*
 * Update the position of all bufm's after a_valid, and invalidate their cached
 * bufp pointers as well.
 */
static void
buf_p_bufms_update(cw_buf_t *a_buf, cw_bufm_t *a_valid, cw_sint64_t a_amount)
{
	cw_bufm_t	*bufm;

	ql_reverse_foreach(bufm, &a_buf->bufms, link) {
		if (bufm == a_valid)
			break;
		bufm->bpos += a_amount;
		bufm->bufp = NULL;
	}
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
		retval = (cw_buf_t *)a_alloc(a_arg, sizeof(cw_buf_t), __FILE__,
		    __LINE__);
		memset(retval, 0, sizeof(cw_buf_t));
		retval->alloced = TRUE;
	}

	retval->alloc = a_alloc;
	retval->realloc = a_realloc;
	retval->dealloc = a_dealloc;
	retval->arg = a_arg;

	retval->msgq = a_msgq;

	mtx_new(&retval->mtx);

	/* Set up the bufp vector. */
	retval->len = 0;
	retval->bufp_count = 1;
	retval->bufp_veclen = 1;
	retval->bufp_vec = (cw_bufp_t **)_cw_opaque_alloc(a_alloc, a_arg,
	    sizeof(cw_bufp_t *));

	/* Initialize a bufp. */
	retval->bufp_vec[0] = (cw_bufp_t *)_cw_opaque_alloc(a_alloc, a_arg,
	    sizeof(cw_bufp_t));
	bufp_p_new(retval->bufp_vec[0]);

	retval->ncached = 0;

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
	cw_uint64_t	i;

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

	/* Destroy the bufp vector. */
	for (i = 0; i < a_buf->bufp_count; i++) {
		_cw_opaque_dealloc(a_buf->dealloc, a_buf->arg,
		    a_buf->bufp_vec[i], sizeof(cw_bufp_t));
	}
	_cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf->bufp_vec,
	    sizeof(cw_bufp_t *) * a_buf->bufp_veclen);

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
bufm_p_bufp_cache(cw_bufm_t *a_bufm)
{
	a_bufm->bufp = a_bufm->buf->bufp_vec[buf_p_bufp_at_cpos(a_bufm->buf,
	    a_bufm->bpos - 1)];
}

static void
bufm_p_rel_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount)
{
	a_bufm->bpos += a_amount;

	/*
	 * XXX We should be able to do better than this, since most relative
	 * seeks will be very nearby, meaning that we don't always need to
	 * re-insert into the bufm list.
	 */
	ql_remove(&a_bufm->buf->bufms, a_bufm, link);
	buf_p_bufm_insert(a_bufm->buf, a_bufm);

	if (a_bufm->msgq != NULL) {
		/* Send a message. */
		_cw_error("XXX Not implemented");
	}
}

static void
bufm_p_insert(cw_bufm_t *a_bufm, const cw_char_t *a_str, cw_uint64_t a_count)
{
	/*
	 * Get the bufp at which the insertion starts.  If the a_bufm points at
	 * the end of a full bufp, then first try to insert into the next bufp.
	 * If that bufp is full or doesn't exist, create a new bufp.
	 */
	if (a_bufm->bufp == NULL)
		bufm_p_bufp_cache(a_bufm);

	if (a_bufm->bpos > a_bufm->bufp->ecpos &&
	    bufp_p_gap_len_get(a_bufm->bufp) == 0) {
		/* At end of full bufp. */
		if (a_bufm->bpos > a_bufm->buf->len) {
			/* At EOB. */
			/* XXX Append a bufp. */
		} else {
			
		}
	}

	/*
	 * Check that there is enough room to insert the string into the bufp.
	 * If so, simply insert the string.
	 */


	/*
	 * There is not enough space in the bufp for the entire string.  Split
	 * the bufp.  Taking into account the space made available by the split,
	 * make sure there is enough space to insert the string, and if not,
	 * insert enough additional bufp's to make room for the string.
	 */

	/*
	 * The above code has guaranteed that there is enough space to insert
	 * the entire string.  Insert as much of the string as will fit in each
	 * consecutive bufp until the entire string has been inserted.  The text
	 * inserted into the first bufp is inserted at the end, but for all
	 * subsequent bufp's it is inserted at the beginning.
	 */

	_cw_error("XXX Not implemented");
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
	retval->bpos = 1;
	retval->bufp = NULL;
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

	a_to->bpos = a_from->bpos;

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
bufm_line_seek(cw_bufm_t *a_bufm, cw_uint64_t a_line)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);
	_cw_assert(a_line > 0);

	mtx_lock(&a_bufm->buf->mtx);

	_cw_error("XXX Not implemented");

	if (a_bufm->msgq != NULL) {
		/* Send a message. */
		_cw_error("XXX Not implemented");
	}

	retval = a_bufm->bpos; /* XXX */

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
	retval = buf_p_line_at_cpos(a_bufm->buf, a_bufm->bpos);
	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

cw_uint64_t
bufm_rel_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure not to go out of buf bounds. */
	if (a_amount >= 0) {
		if (a_bufm->bpos + a_amount > a_bufm->buf->len + 1)
			a_amount = a_bufm->buf->len + 1 - a_bufm->bpos;
	} else {
		if (a_amount * -1 + 1 > a_bufm->bpos)
			a_amount = -a_bufm->bpos + 1;
	}

	bufm_p_rel_seek(a_bufm, a_amount);
	retval = a_bufm->bpos;

	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

cw_uint64_t
bufm_abs_seek(cw_bufm_t *a_bufm, cw_uint64_t a_pos)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);
	_cw_assert(a_pos > 0);

	mtx_lock(&a_bufm->buf->mtx);

	a_bufm->bpos = a_pos;

	/* Make sure not to go out of buf bounds. */
	if (a_bufm->bpos > a_bufm->buf->len + 1)
		a_bufm->bpos = a_bufm->buf->len + 1;

	ql_remove(&a_bufm->buf->bufms, a_bufm, link);
	buf_p_bufm_insert(a_bufm->buf, a_bufm);

	if (a_bufm->msgq != NULL) {
		/* Send a message. */
		_cw_error("XXX Not implemented");
	}

	retval = a_bufm->bpos;

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
	retval = a_bufm->bpos;
	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

cw_bool_t
bufm_before_get(cw_bufm_t *a_bufm, cw_bufc_t *r_bufc)
{
	cw_bool_t	retval;
	cw_uint64_t	index;
	cw_bufc_t	*bufc;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at BOB. */
	if (a_bufm->bpos == 1) {
		retval = TRUE;
		goto RETURN;
	}

	index = buf_p_bufp_at_cpos(a_bufm->buf, a_bufm->bpos - 1);
	bufc = bufp_p_cpos_get(a_bufm->buf->bufp_vec[index], a_bufm->bpos - 1);
	*r_bufc = *bufc;

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_after_get(cw_bufm_t *a_bufm, cw_bufc_t *r_bufc)
{
	cw_bool_t	retval;
	cw_uint64_t	index;
	cw_bufc_t	*bufc;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at EOB. */
	if (a_bufm->bpos == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	index = buf_p_bufp_at_cpos(a_bufm->buf, a_bufm->bpos);
	bufc = bufp_p_cpos_get(a_bufm->buf->bufp_vec[index], a_bufm->bpos);
	*r_bufc = *bufc;

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_before_set(cw_bufm_t *a_bufm, cw_char_t a_char)
{
	cw_bool_t	retval;
	cw_uint64_t	index;
	cw_bufc_t	*bufc;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at BOB. */
	if (a_bufm->bpos == 1) {
		retval = TRUE;
		goto RETURN;
	}

	index = buf_p_bufp_at_cpos(a_bufm->buf, a_bufm->bpos - 1);
	bufc = bufp_p_cpos_get(a_bufm->buf->bufp_vec[index], a_bufm->bpos -
	    1);
	bufc_char_set(*bufc, a_char);

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_after_set(cw_bufm_t *a_bufm, cw_char_t a_char)
{
	cw_bool_t	retval;
	cw_uint64_t	index;
	cw_bufc_t	*bufc;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at EOB. */
	if (a_bufm->bpos == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	index = buf_p_bufp_at_cpos(a_bufm->buf, a_bufm->bpos);
	bufc = bufp_p_cpos_get(a_bufm->buf->bufp_vec[index], a_bufm->bpos);
	bufc_char_set(*bufc, a_char);

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_before_attrs_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
	cw_bool_t	retval;
	cw_uint64_t	index;
	cw_bufc_t	*bufc;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at BOB. */
	if (a_bufm->bpos == 1) {
		retval = TRUE;
		goto RETURN;
	}

	index = buf_p_bufp_at_cpos(a_bufm->buf, a_bufm->bpos - 1);
	bufc = bufp_p_cpos_get(a_bufm->buf->bufp_vec[index], a_bufm->bpos - 1);
	bufc_attrs_copy(*bufc, a_bufc);

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_bufm->buf->mtx);
	return retval;
}

cw_bool_t
bufm_after_attrs_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
	cw_bool_t	retval;
	cw_uint64_t	index;
	cw_bufc_t	*bufc;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	/* Make sure the marker isn't at EOB. */
	if (a_bufm->bpos == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	index = buf_p_bufp_at_cpos(a_bufm->buf, a_bufm->bpos);
	bufc = bufp_p_cpos_get(a_bufm->buf->bufp_vec[index], a_bufm->bpos);
	bufc_attrs_copy(*bufc, a_bufc);

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

	bufm_p_insert(a_bufm, a_str, a_count);

	/* Move the marker forward so that it is after the inserted text. */
	bufm_p_rel_seek(a_bufm, a_count);

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

	bufm_p_insert(a_bufm, a_str, a_count);
	
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
