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
 * bufb : Buffer block.  Internally, buf data are composed of bufb's, which
 *        store buf data in a paged buffer gap format.
 * bufc : Character.  Characters are merely 8 bit values.
 * bufm : Marker.  Markers are used as handles for many buf operations.
 * bufh : History.  The bufh class provides an abstract history mechanism to the
 *        buf class and implements infinite undo and redo.
 *
 * buf's and everything associated with them are implicitly synchronized.
 *
 * Buffer/character position numbering starts at 1.
 *
 * bufc pos:  0           1     0   1   2   3         0   1   2
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
 * mentioned, but internally, the difference matters at bufb boundaries.  When
 * inserting, we use buffer positions, and when reading/writing/removing, we use
 * character positions.
 *
 * Buffer position N and character position N - 1 are always in the same bufb,
 * so internally we use character positions everywhere.  Special care must be
 * given to buffer position 1, since there is no actual character 0.
 *
 * Position rules:
 *
 * *) bufb's start counting at 0; it's up to the buf code to translate.
 * *) bpos refers to buffer position.
 * *) cpos refers to character position.
 * *) If a position isn't specified as bpos or cpos, then it is bpos.
 *
 ******************************************************************************/

#include "../include/nxe.h"

/*
 * Prototypes.
 */
/* bufb. */
static void	bufb_p_new(cw_bufb_t *a_bufb);
static void	bufb_p_gap_move(cw_bufb_t *a_bufb, cw_uint32_t a_pos);
static cw_bufc_t bufb_p_get(cw_bufb_t *a_bufb, cw_uint32_t a_pos);
static void	bufb_p_insert(cw_bufb_t *a_bufb, cw_uint32_t a_pos, const
    cw_bufc_t *a_data, cw_uint64_t a_count);
static void	bufb_p_split(cw_bufb_t *a_bufb, cw_bufb_t *a_after, cw_uint32_t
    a_pos);
static void	bufb_p_remove(cw_bufb_t *a_bufb, cw_uint32_t a_pos,
    cw_uint32_t a_count);
static cw_bufc_t bufb_p_bufc_at_cpos(cw_bufb_t *a_bufb, cw_uint64_t a_cpos);
static cw_uint64_t bufb_p_line_at_pos(cw_bufb_t *a_bufb, cw_uint64_t
    a_pos);

#define	bufb_p_gap_len_get(a_bufb) (a_bufb)->gap_len
#define	bufb_p_len_get(a_bufb)						\
	(_CW_BUFB_NCHAR - bufb_p_gap_len_get(a_bufb))
#define	bufb_p_nlines_get(a_bufb) ((cw_uint64_t)(a_bufb)->nlines)
#define	bufb_p_ecpos_get(a_bufb) (a_bufb)->ecpos
#define	bufb_p_ecpos_set(a_bufb, a_ecpos)				\
	(a_bufb)->ecpos = (a_ecpos) + bufb_p_len_get(a_bufb)
#define	bufb_p_eline_get(a_bufb) (a_bufb)->eline
#define	bufb_p_eline_set(a_bufb, a_eline)				\
	(a_bufb)->eline = (a_eline) + bufb_p_nlines_get(a_bufb)

/* bufh. */
static void	bufh_p_new(cw_bufh_t *a_bufh);
static void	bufh_p_delete(cw_bufh_t *a_bufh);

/* buf. */
static cw_uint64_t buf_p_bufb_at_cpos(cw_buf_t *a_buf, cw_uint64_t a_cpos);
static cw_uint64_t buf_p_line_at_cpos(cw_buf_t *a_buf, cw_uint64_t a_cpos);
static void	buf_p_bufb_cache_update(cw_buf_t *a_buf, cw_uint64_t a_cpos);
static void	buf_p_bufm_insert(cw_buf_t *a_buf, cw_bufm_t *a_bufm);

/* bufb. */
static void
bufb_p_new(cw_bufb_t *a_bufb)
{
	_cw_check_ptr(a_bufb);
	_cw_assert(sizeof(cw_bufb_t) == _CW_BUFB_SIZE);

	a_bufb->nlines = 0;
	a_bufb->gap_off = 0;
	a_bufb->gap_len = _CW_BUFB_NCHAR;
}

static void
bufb_p_gap_move(cw_bufb_t *a_bufb, cw_uint32_t a_pos)
{
	_cw_check_ptr(a_bufb);
	_cw_assert(a_pos <= bufb_p_len_get(a_bufb));

	/* Move the gap if it isn't already at the point of insertion. */
	if (a_bufb->gap_off != a_pos) {
		if (a_bufb->gap_off < a_pos) {
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
			 *                   a_pos
			 *                   |
			 *                   v
			 * oooooooXXXXXXXXXXX________oo
			 */
			memmove(&a_bufb->data[a_bufb->gap_off],
			    &a_bufb->data[a_bufb->gap_off + a_bufb->gap_len],
			    (a_pos - a_bufb->gap_off) * sizeof(cw_bufc_t));
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
			 *     a_pos
			 *     |
			 *     v
			 * oooo___________XXXXXXXXXoooo
			 */
			memmove(&a_bufb->data[a_bufb->gap_len + a_pos],
			    &a_bufb[a_pos],
			    (a_bufb->gap_off - a_pos) * sizeof(cw_bufc_t));
		}
		a_bufb->gap_off = a_pos;
	}
}

static cw_bufc_t
bufb_p_get(cw_bufb_t *a_bufb, cw_uint32_t a_pos)
{
	cw_bufc_t	retval;

	_cw_check_ptr(a_bufb);
	_cw_assert(a_pos < bufb_p_len_get(a_bufb));

	if (a_pos < a_bufb->gap_off)
		retval = a_bufb->data[a_pos];
	else
		retval = a_bufb->data[a_pos + a_bufb->gap_len];

	return retval;
}

static void
bufb_p_insert(cw_bufb_t *a_bufb, cw_uint32_t a_pos, const cw_bufc_t *a_data,
    cw_uint64_t a_count)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_bufb);
	_cw_assert(a_count <= a_bufb->gap_len);
	_cw_assert(a_pos + a_count <= _CW_BUFB_NCHAR);

	bufb_p_gap_move(a_bufb, a_pos);
	for (i = 0; i < a_count; i++) {
		a_bufb->data[a_pos + i] = a_data[i];
		if (a_data[i].c == '\n')
			a_bufb->nlines++;
	}
	a_bufb->gap_off += a_count;
	a_bufb->gap_len -= a_count;
}

/*
 * Split a_bufb into two bufb's at position a_pos.  a_after is assumed to have
 * enough space.
 */
static void
bufb_p_split(cw_bufb_t *a_bufb, cw_bufb_t *a_after, cw_uint32_t a_pos)
{
	_cw_check_ptr(a_bufb);
	_cw_check_ptr(a_after);
	_cw_assert(bufb_p_gap_len_get(a_after) >= _CW_BUFB_NCHAR -
	    a_bufb->gap_off - a_bufb->gap_len);
	_cw_assert(a_pos <= _CW_BUFB_NCHAR - a_bufb->gap_len);

	bufb_p_gap_move(a_bufb, a_pos);
	bufb_p_insert(a_after, 0, &a_bufb->data[a_bufb->gap_off +
	    a_bufb->gap_len], _CW_BUFB_NCHAR - a_bufb->gap_off -
	    a_bufb->gap_len);

	a_bufb->gap_len = _CW_BUFB_NCHAR - a_bufb->gap_off;
	a_bufb->nlines -= bufb_p_nlines_get(a_after);
}

static void
bufb_p_remove(cw_bufb_t *a_bufb, cw_uint32_t a_pos, cw_uint32_t a_count)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_bufb);
	_cw_assert(a_pos + a_count <= bufb_p_len_get(a_bufb));

	bufb_p_gap_move(a_bufb, a_pos);
	for (i = 0; i < a_count; i++) {
		if (a_bufb->data[a_bufb->gap_off + a_bufb->gap_len + i].c ==
		    '\n')
			a_bufb->nlines--;
	}
	a_bufb->gap_len += a_count;
}

static cw_bufc_t
bufb_p_bufc_at_cpos(cw_bufb_t *a_bufb, cw_uint64_t a_cpos)
{
	_cw_check_ptr(a_bufb);
	_cw_assert(a_cpos <= a_bufb->ecpos);
	
	/*
	 * We can assume that ecpos is valid.
	 */
	return bufb_p_get(a_bufb, a_cpos + bufb_p_len_get(a_bufb) -
	    a_bufb->ecpos);
}

static cw_uint64_t
bufb_p_line_at_cpos(cw_bufb_t *a_bufb, cw_uint64_t a_cpos)
{
	cw_uint64_t	i, count, retval;

	_cw_assert(a_cpos <= a_bufb->ecpos);
	_cw_assert(a_cpos > a_bufb->ecpos - bufb_p_len_get(a_bufb));

	/*
	 * We can assume that ecpos and eline are valid.
	 *
	 * Iterate upward, since it is faster on many architectures.  Avoid the
	 * gap.
	 */
	retval = a_bufb->eline - a_bufb->nlines;
	count = a_cpos - a_bufb->ecpos + bufb_p_len_get(a_bufb);

	for (i = 0; i < count && i < a_bufb->gap_off; i++) {
		if (a_bufb->data[i].c == '\n')
			retval++;
	}

	for (i = a_bufb->gap_off + a_bufb->gap_len; i < count; i++) {
		if (a_bufb->data[i].c == '\n')
			retval++;
	}

	return retval;
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
/*  	bufb_p_ecpos_set(retval->bufb_vec[0], 0); */
/*  	bufb_p_eline_set(retval->bufb_vec[0], 1); */

	retval->ncached = 0;

	/* Initialize history. */
	retval->hist_active = FALSE;
	/* XXX */
/*  	bufh_p_new(&retval->hist); */

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

	/* XXX */
/*  	bufh_p_delete(&a_buf->hist); */

	/*
	 * Set the buf pointers of all objects that point to this one to NULL,
	 * so that they won't try to disconnect during destruction.  All objects
	 * that reference this one effectively become invalid, but they can (and
	 * should) be destroyed even though this base buf is gone.
	 */
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

static cw_uint64_t
buf_p_bufb_at_cpos(cw_buf_t *a_buf, cw_uint64_t a_cpos)
{
	cw_uint64_t	index, first, last;

	/*
	 * Make sure the cache reaches far enough to be useful for the
	 * search.
	 */
	buf_p_bufb_cache_update(a_buf, a_cpos);

	/*
	 * Do a binary search for the correct bufb using the bufb character
	 * position cache.  "first" is the index of the first element in the
	 * range to search, and "last" is one plus the index of the last element
	 * in the range to search.
	 */
	first = 0;
	last = a_buf->bufb_count;
	if (a_cpos < bufb_p_ecpos_get(a_buf->bufb_vec[first]))
		index = first;
	else if (a_cpos == a_buf->len)
		index = last - 1;
	else {
		for (;;) {
			index = (first + last) >> 1;

			_cw_assert(index < a_buf->bufb_count);
			if (bufb_p_ecpos_get(a_buf->bufb_vec[index]) < a_cpos) {
				first = index + 1;
			} else if (bufb_p_ecpos_get(a_buf->bufb_vec[index - 1])
			    > a_cpos) {
				last = index;
			} else
				break;
		}
	}
	_cw_assert(index < a_buf->bufb_count);

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

	index = buf_p_bufb_at_cpos(a_buf, a_cpos);
	retval = bufb_p_line_at_cpos(a_buf->bufb_vec[index], a_cpos);
	RETURN:
	return retval;
}

static void
buf_p_bufb_cache_update(cw_buf_t *a_buf, cw_uint64_t a_cpos)
{
	cw_uint64_t	ecpos, eline;

	_cw_assert(a_cpos <= a_buf->len);

	/*
	 * Update the cache so that it can be used for searches up to and
	 * including a_cpos.
	 */
	if (a_buf->ncached > 0) {
		ecpos = bufb_p_ecpos_get(a_buf->bufb_vec[a_buf->ncached - 1]);
		eline = bufb_p_eline_get(a_buf->bufb_vec[a_buf->ncached - 1]);
	} else {
		ecpos = 0;
		eline = 1;
	}
	for (; ecpos <= a_cpos && a_buf->ncached < a_buf->bufb_count;
	     a_buf->ncached++) {
		bufb_p_ecpos_set(a_buf->bufb_vec[a_buf->ncached], ecpos);
		ecpos = bufb_p_ecpos_get(a_buf->bufb_vec[a_buf->ncached]);

		bufb_p_eline_set(a_buf->bufb_vec[a_buf->ncached], eline);
		eline = bufb_p_eline_get(a_buf->bufb_vec[a_buf->ncached]);
	}
}

static void
buf_p_bufm_insert(cw_buf_t *a_buf, cw_bufm_t *a_bufm)
{
	cw_bufm_t	*bufm;

	ql_foreach(bufm, &a_buf->bufms, link) {
		if (a_bufm->cpos >= bufm->cpos) {
			ql_after_insert(bufm, a_bufm, link);
			return;
		}
	}
	ql_head_insert(&a_buf->bufms, a_bufm, link);
}

/* bufm. */
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
	retval->cpos = 0;
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

	a_to->cpos = a_from->cpos;

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
bufm_line(cw_bufm_t *a_bufm)
{
	cw_uint64_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);
	retval = buf_p_line_at_cpos(a_bufm->buf, a_bufm->cpos);
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
		if (a_bufm->cpos + a_amount > a_bufm->buf->len)
			a_amount = a_bufm->buf->len;
	} else {
		if (a_amount * -1 > a_bufm->cpos)
			a_amount = -a_bufm->cpos;
	}

	a_bufm->cpos += a_amount;

	/*
	 * XXX We should be able to do better than this, since most relative
	 * seeks will be very nearby, meaning that we don't always need to
	 * re-insert into the bufm list.
	 */
	ql_remove(&a_bufm->buf->bufms, a_bufm, link);
	buf_p_bufm_insert(a_bufm->buf, a_bufm);

	retval = a_bufm->cpos;

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

	a_bufm->cpos = a_pos - 1;

	/* Make sure not to go out of buf bounds. */
	if (a_bufm->cpos > a_bufm->buf->len)
		a_bufm->cpos = a_bufm->buf->len;

	ql_remove(&a_bufm->buf->bufms, a_bufm, link);
	buf_p_bufm_insert(a_bufm->buf, a_bufm);

	retval = a_bufm->cpos;

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
	retval = a_bufm->cpos + 1;
	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

cw_bufc_t
bufm_bufc_get(cw_bufm_t *a_bufm)
{
	cw_bufc_t	retval;
	cw_uint64_t	index;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	mtx_lock(&a_bufm->buf->mtx);

	index = buf_p_bufb_at_cpos(a_bufm->buf, a_bufm->cpos);
	retval = bufb_p_bufc_at_cpos(a_bufm->buf->bufb_vec[index],
	    a_bufm->cpos);

	mtx_unlock(&a_bufm->buf->mtx);

	return retval;
}

void
bufm_bufc_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	_cw_error("XXX Not implemented");
}

void
bufm_bufc_insert(cw_bufm_t *a_bufm, const cw_bufc_t *a_data, cw_uint64_t
    a_count)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	/* XXX Temporary hack! */
	bufb_p_insert(a_bufm->buf->bufb_vec[0], 0, a_data, a_count);
	a_bufm->buf->len = a_count;
	
/*  	_cw_error("XXX Not implemented"); */
}
