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
 * bufc : Character.  Characters consist of 8 bit values and 24 bits of
 *        attributes.
 * bufm : Marker.  Markers are used as handles for many buf operations.
 *
 ******************************************************************************
 *
 * Internal buffer representation:
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
 ******************************************************************************
 *
 * Undo/redo history:
 *
 * Since nxe supports infinite undo (within the memory limitations of the
 * system), buffer history can become quite long.  Therefore, the buffer history
 * mechanism aims to be as compact as possible, at the expense of complexity.
 * Internally, the history mechanism is a combination of a state machine that
 * maintains enough state to be able to play the history in either direction,
 * along with a compact log of buffer changes.
 *
 * User input that does not involve explicit point movement causes the history
 * log to grow by approximately one byte per inserted/deleted character
 * (transitions between insert/delete cost 1 byte).  A position change in
 * between buffer modifications costs 9 bytes of history log space.  Each group
 * start/end boundary costs 1 byte of history log space.
 *
 * For the history log records to be so compact, additional state must be
 * available when playing the history.  Specifically, the following state is
 * necessary:
 *
 * *) Buffer position.
 * *) Number of character insertions/deletions in undo and redo directions.
 * *) Log state (none, insert, remove).
 *
 * Following are some examples of history logs/states.  They use the following
 * notation:
 *
 * B      : Group begin.
 * E      : Group end.
 * (int)P : Buffer position (64 bits, unsigned).
 * (s)I   : Insert string s before point.
 * (s)Y   : Insert (yank) string s after point.
 * (s)R   : Remove string s before point.
 * (s)K   : Remove (kill) string s after point.
 *
 * History growth -------->
 *
 * Example:
 *   (3)P(hello)I(olleh)R(salutations)I
 *
 * Translation:
 *   User started at bpos 3, typed "hello", backspaced through "hello", then
 *   typed "salutations".  Following is what the text looks like at each undo
 *   stup:
 *
 *   -------------
 *     salutations
 *     salutation
 *     salutatio
 *     salutati
 *     salutat
 *     saluta
 *     salut
 *     salu
 *     sal
 *     sa
 *     s
 *     
 *     h
 *     he
 *     hel
 *     hell
 *     hello
 *     hell
 *     hel
 *     he
 *     h
 *
 *   -------------
 *
 * Example:
 *   B(42)P(This is a string.)KEB(It gets replaced by this one.)YE
 *
 * Translation:
 *   At bpos 42, the user cut "This is a string.", then pasted
 *   "It gets replaced by this one.".  Following is what the text looks like at
 *   each undo step:
 *
 *   -------------------------------
 *     It gets replaced by this one.
 *
 *     This is a string.
 *   -------------------------------
 *
 * Example:
 *   (1)P(This string is more than 32 char)I(acters long.)I
 *
 * Translation:
 *   The user typed in a string that was long enough to require a new log
 *   header.  Logically, this is no different than if the entire string could
 *   have been encoded as a single record.
 *
 * Example:
 *   B(42)P(Hello)IB(Goodbye)IE(Really)IEE
 *
 * Translation:
 *   Through some programmatic means, the user inserted three strings in such a
 *   way that nested groups were created.  The entire record is undone in a
 *   single step.
 *
 * Example:
 *   B(42)P(Hello)I
 *
 * Translation:
 *   The user created an explicit group begin record, then inserted "Hello" at
 *   bpos 42.  The history state machine realizes that there is an unmatched
 *   group begin (it keeps a group nesting counter), so the entire string
 *   "Hello" will be removed as a single undo operation.  Redo will also cause
 *   the entire string to be inserted as a single redo operation.
 *
 *   Although it is allowable for a group begin record to be unmatched, it is
 *   not allowable for a group end record to be unmatched.  So, the following
 *   log is impossible:
 *
 *     E(25)P(Hello)I
 *
 ******************************************************************************/

#include "../include/nxe.h"

/*
 * The upper 3 bits are used to denote the record type, and the lower 5 bits are
 * used to record the number of characters for insert/delete.  5 bits isn't
 * much, but it makes for very compact storage in the common case, and doesn't
 * waste much space in the worst case.  In order to make the most of the 5 bits,
 * they are interpreted to encode the numbers 1 to 32, since 0 is never needed.
 */
#define	HDR_TAG_MASK		0xe0
#define	HDR_CNT_MASK		(~HDR_TAG_MASK)
#define	HDR_TAG_GRP_BEG		0x20
#define	HDR_TAG_GRP_END		0x40
#define	HDR_TAG_POS		0x60
#define	HDR_TAG_INS		0x80
#define	HDR_TAG_YNK		0xa0
#define	HDR_TAG_REM		0xc0
#define	HDR_TAG_DEL		0xe0

#define	hdr_tag_get(a_hdr)	((a_hdr) & HDR_TAG_MASK)
#define	hdr_tag_set(a_hdr, a_tag)					\
	(a_hdr) = ((a_hdr) & HDR_CNT_MASK) | (a_tag)
#define	hdr_cnt_get(a_hdr)	(((a_hdr) & HDR_CNT_MASK) + 1)
#define	hdr_cnt_set(a_hdr, a_cnt)					\
	(a_hdr) = ((a_hdr) & HDR_TAG_MASK) | ((a_cnt) - 1)


/*
 * Prototypes.
 */

/* buf. */
static cw_uint64_t buf_p_pos_b2a(cw_buf_t *a_buf, cw_uint64_t a_bpos);
static cw_uint64_t buf_p_pos_a2b(cw_buf_t *a_buf, cw_uint64_t a_apos);
static cw_uint64_t buf_p_lines_rel_forward_count(cw_buf_t *a_buf, cw_uint64_t
    a_apos_beg, cw_uint64_t a_nlines);
static cw_uint64_t buf_p_lines_rel_backward_count(cw_buf_t *a_buf, cw_uint64_t
    a_apos_beg, cw_uint64_t a_nlines);
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

	/* Move past a_nlines '\n' characters, taking care to avoid the gap. */
	for (apos = a_apos_beg, nlines = 0; nlines < a_nlines && apos <
	    a_buf->gap_off; apos++) {
		if (bufc_char_get(a_buf->b[apos]) == '\n')
			nlines++;
	}

	if (apos == a_buf->gap_off)
		apos += a_buf->gap_len;
	for (; nlines < a_nlines; apos++) {
		if (bufc_char_get(a_buf->b[apos]) == '\n')
			nlines++;
	}

	return apos;
}

static cw_uint64_t
buf_p_lines_rel_backward_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
    cw_uint64_t a_nlines)
{
	cw_uint64_t	apos, nlines;

	/* Move past a_nlines '\n' characters, taking care to avoid the gap. */
	for (apos = a_apos_beg, nlines = 0; nlines < a_nlines && apos >=
	    a_buf->gap_off + a_buf->gap_len; apos--) {
		if (bufc_char_get(a_buf->b[apos]) == '\n')
			nlines++;
	}

	if (apos == a_buf->gap_off + a_buf->gap_len - 1)
		apos -= a_buf->gap_len;
	for (; nlines < a_nlines; apos--) {
		if (bufc_char_get(a_buf->b[apos]) == '\n')
			nlines++;
	}

	/*
	 * apos is now just before the '\n' but we need to return the apos after
	 * the '\n'.  Add 1 to apos, then make sure it isn't in the gap.
	 */
	apos++;
	if (apos == a_buf->gap_off)
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
	     bufm = ql_next(&a_buf->bufms, bufm, link))
		bufm->apos += a_adjust;
/*  	fprintf(stderr, "%s:%u:%s(): Got here\n", __FILE__, __LINE__, */
/*  	    __FUNCTION__); */

	/* Backward. */
	for (bufm = ql_prev(&a_buf->bufms, a_bufm, link);
	     bufm != NULL && bufm->apos >= a_beg_apos && bufm->apos <
	     a_end_apos;
	     bufm = ql_prev(&a_buf->bufms, bufm, link))
		bufm->apos += a_adjust;
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
	retval->hist_buf = NULL;

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

	if (a_buf->hist_buf != NULL) {
		/* XXX Clean up history. */
	}

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

	if (a_buf->hist_buf != NULL)
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

	if (a_active == TRUE && a_buf->hist_buf == NULL) {
		_cw_error("XXX Not implemented");
	} else if (a_active == FALSE && a_buf->hist_buf != NULL) {
		_cw_error("XXX Not implemented");
	}
}

cw_bool_t
buf_undo(cw_buf_t *a_buf, cw_bufm_t *a_bufm)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	_cw_error("XXX Not implemented");
	return TRUE; /* XXX */
}

cw_bool_t
buf_redo(cw_buf_t *a_buf, cw_bufm_t *a_bufm)
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

/*  	fprintf(stderr, "%s:%u:%s(): Got here\n", __FILE__, __LINE__, */
/*  	    __FUNCTION__); */
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
		 * Move forward from BOB to just past a_offset '\n' characters.
		 * For example, if seeking forward 2:
		 *
		 *  \/
		 *   hello\ngoodbye\nyadda\nblah
		 *                  /\
		 */
		a_bufm->apos = buf_p_lines_rel_forward_count(a_bufm->buf,
		    buf_p_pos_b2a(a_bufm->buf, 1), a_offset);

		/* Set the line number. */
		a_bufm->line = 1 + a_offset;

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
			if (a_offset > 0 && a_bufm->line + a_offset >
			    a_bufm->buf->nlines) {
				/*
				 * Attempt to move to or after EOB.  Move to
				 * EOB.
				 */
				bufm_seek(a_bufm, 0, BUFW_END);
				break;
			}
			/*
			 * Move forward from the current position to just past
			 * a_offset '\n' characters.  Fore example, if seeking
			 * forward 2:
			 *
			 *            \/
			 *   hello\ngoodbye\nyadda\nblah
			 *                         /\
			 */
			apos = buf_p_lines_rel_forward_count(a_bufm->buf,
			    a_bufm->apos, a_offset);

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
			a_bufm->line += a_offset;

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
		 *
		 * Stopping short of the '\n' is done to make forward and
		 * backward line seeking reflexive.
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

	if (a_bufm->msgq != NULL) {
		/* Send a message. */
		_cw_error("XXX Not implemented");
	}

	retval = a_bufm->apos; /* XXX */

	return retval;
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
	cw_uint64_t	retval, bpos;
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

	if (a_bufm->msgq != NULL) {
		/* Send a message. */
		_cw_error("XXX Not implemented");
	}

	return retval;
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

cw_bool_t
bufm_before_get(cw_bufm_t *a_bufm, cw_bufc_t *r_bufc)
{
	cw_bool_t	retval;
	cw_uint64_t	bpos;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

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

	bpos = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);

	/* Make sure the marker isn't at EOB. */
	if (bpos == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	*r_bufc = a_bufm->buf->b[a_bufm->apos];

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
bufm_before_set(cw_bufm_t *a_bufm, cw_char_t a_char)
{
	cw_bool_t	retval;
	cw_uint64_t	apos, bpos;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	bpos = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);

	/* Make sure the marker isn't at BOB. */
	if (bpos == 1) {
		retval = TRUE;
		goto RETURN;
	}

	bpos--;
	apos = buf_p_pos_b2a(a_bufm->buf, bpos);
	bufc_char_set(a_bufm->buf->b[apos], a_char);

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
bufm_after_set(cw_bufm_t *a_bufm, cw_char_t a_char)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	/* Make sure the marker isn't at EOB. */
	if (buf_p_pos_a2b(a_bufm->buf, a_bufm->apos) == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	bufc_char_set(a_bufm->buf->b[a_bufm->apos], a_char);

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
bufm_before_attrs_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
	cw_bool_t	retval;
	cw_uint64_t	apos, bpos;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	bpos = buf_p_pos_a2b(a_bufm->buf, a_bufm->apos);

	/* Make sure the marker isn't at BOB. */
	if (bpos == 1) {
		retval = TRUE;
		goto RETURN;
	}

	bpos--;
	apos = buf_p_pos_b2a(a_bufm->buf, bpos);
	bufc_attrs_copy(a_bufm->buf->b[apos], a_bufc);

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
bufm_after_attrs_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	/* Make sure the marker isn't at EOB. */
	if (buf_p_pos_a2b(a_bufm->buf, a_bufm->apos) == a_bufm->buf->len + 1) {
		retval = TRUE;
		goto RETURN;
	}

	bufc_attrs_copy(a_bufm->buf->b[a_bufm->apos], a_bufc);

	retval = FALSE;
	RETURN:
	return retval;
}

void
bufm_before_insert(cw_bufm_t *a_bufm, const cw_char_t *a_str, cw_uint64_t
    a_count)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	bufm_p_insert(a_bufm, FALSE, a_str, a_count);
}

void
bufm_after_insert(cw_bufm_t *a_bufm, const cw_char_t *a_str, cw_uint64_t
    a_count)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	bufm_p_insert(a_bufm, TRUE, a_str, a_count);
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

	_cw_error("XXX Not implemented");
}
