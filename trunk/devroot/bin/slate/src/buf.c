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
 ******************************************************************************
 *
 * Undo/redo history:
 *
 * Since slate supports infinite undo (within the memory limitations of the
 * system, or up to 4 GB of history storage, whichever is less), buffer history
 * can become quite long.  Therefore, the buffer history mechanism aims to be as
 * compact as possible, at the expense of complexity.  Internally, the history
 * mechanism is a combination of a state machine that maintains enough state to
 * be able to play the history in either direction, along with a compact log of
 * buffer changes.
 *
 * User input that does not involve explicit point movement causes the history
 * log to grow by approximately one byte per inserted/deleted character
 * (transitions between insert/delete cost 1 byte).  A position change in
 * between buffer modifications costs 9 bytes of history log space.  Each group
 * start/end boundary costs 1 byte of history log space.
 *
 * The following notation is used in discussing the history buffer:
 *
 * Undo : Inverse  : Meaning
 * =====:======:======================================================
 * B    : b    : Group begin.
 * E    : e    : Group end.
 * (n)P : p[n] : Buffer position change (64 bits, unsigned).
 * (s)I : r[s] : Insert string s before point.
 * (s)Y : k[s] : Insert (yank) string s after point.
 * (s)R : i[s] : Remove string s before point.
 * (s)K : y[s] : Remove (kill) string s after point.
 *
 * History growth -------->
 *
 * As the history is undone, it gets inverted, so that it becomes a redo log.
 * In general, the history buffer looks:
 *
 *   UUUUUUUUUUURRRRRRRR
 *             /\
 *             hcur
 *
 * where "U" is an undo record, and "R" is a redo record.  Records are kept
 * valid at all times, so that the only state necessary is:
 *
 * *) History buffer (h)
 * *) Current history buffer position (hcur), delimits undo and redo.
 * *) End of history.
 *
 * An undo record consists of data, followed by a record header, whereas a redo
 * record starts with a record header, followed by data.  This difference is
 * necessary since the header must be encountered first when reading the
 * history; undo and redo read in opposite directions.
 *
 * If there are redo records, it may be that one a record that is partly
 * undone/redone must be split into an undo and a redo portion.  In this case,
 * an extra byte is necessary to store the extra record header.  For example,
 * consider an insert record in its original state, partly undone, and
 * completely undone:
 *
 *   (hello)I
 *   (hel)Ir[lo]
 *   r[hello]
 *
 * Following are several examples of history logs and translations of their
 * meanings.  The /\ characters denote the current position:
 *
 * Example:
 *   (1,3)P(hello)I(olleh)R(salutations)I
 *                                      /\
 *
 * Translation:
 *   User started at bpos 3, typed "hello", backspaced through "hello", then
 *   typed "salutations".  Following is what the text looks like at each undo
 *   step:
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
 *   B(10,42)P(This is a string.)KEB(It gets replaced by this one.)YE
 *                                                                  /\
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
 *   (This string is more than 32 char)I(acters long.)I
 *                                                    /\
 *
 * Translation:
 *   The user typed in a string that was long enough to require a new log
 *   header.  Logically, this is no different than if the entire string could
 *   have been encoded as a single record.
 *
 * Example:
 *   B(5,42)P(Hello)IB(Goodbye)IE(Really)IEE
 *                                         /\
 *
 * Translation:
 *   Through some programmatic means, the user inserted three strings in such a
 *   way that nested groups were created.  The entire record is undone in a
 *   single step.
 *
 * Example:
 *   B(6,42)P(Hello)I
 *                  /\
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
 *     E(1,25)P(Hello)I
 *                    /\
 *
 * Example:
 *   (Hello)Ir[Goodbye]
 *          /\
 *
 * Translation:
 *   The user typed "HelloGoodbye", then undid "Goodbye" so that only "Hello"
 *   remains.
 *
 * Example:
 *   (Hello)Ii[olleH]
 *
 * Translation:
 *   The user typed "Hello", then backspaced through it, then undid the
 *   backspacing so that the end buffer contents are "Hello".
 *
 * Example:
 *  (Hell)Ir[o]i[olleH]
 *
 * Translation:
 *   The user typed "Hello", then backspaced through it, then undid the
 *   backspacing and the "o", so that the end buffer contents are "Hell".
 *
 ******************************************************************************/

#include "../include/modslate.h"

/*
 * The upper 3 bits are used to denote the record type, and the lower 5 bits are
 * used to record the number of characters for insert/delete.  5 bits isn't
 * much, but it makes for very compact storage in the common case, and doesn't
 * waste much space in the worst case.  In order to make the most of the 5 bits,
 * they are interpreted to encode the numbers 1 to 32, since 0 is never needed.
 *
 * The tag numbering is fragile, since bitwise xor is used in hdr_tag_inverse().
 */
#define	HDR_TAG_MASK		0xe0
#define	HDR_CNT_MASK		(~HDR_TAG_MASK)
#define	HDR_CNT_MAX		32
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
/* Only meant for HDR_TAG_{INS,YNK,REM,DEL}. */
#define	hdr_tag_inverse(a_hdr)	((a_hdr) ^ 0x40)
#define	hdr_cnt_get(a_hdr)	(((a_hdr) & HDR_CNT_MASK) + 1)
#define	hdr_cnt_set(a_hdr, a_cnt)					\
	(a_hdr) = ((a_hdr) & HDR_TAG_MASK) | ((a_cnt) - 1)

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
		if (a_buf->b[apos * a_buf->elmsize] == '\n')
			nlines++;
	}

	if (apos == a_buf->gap_off)
		apos += a_buf->gap_len;
	for (; nlines < a_nlines; apos++) {
		if (a_buf->b[apos * a_buf->elmsize] == '\n')
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
		if (a_buf->b[apos * a_buf->elmsize] == '\n')
			nlines++;
	}

	if (apos == a_buf->gap_off + a_buf->gap_len - 1)
		apos -= a_buf->gap_len;
	for (; nlines < a_nlines; apos--) {
		if (a_buf->b[apos * a_buf->elmsize] == '\n')
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

/*  	fprintf(stderr, "%s:%u:%s(): a_bufm->apos: %llu, " */
/*  	    "a_adjust: %lld, " */
/*  	    "a_beg_apos: %llu, a_end_apos: %llu\n", __FILE__, __LINE__, */
/*  	    __FUNCTION__, a_bufm->apos, a_adjust, */
/*  	    a_beg_apos, a_end_apos); */

	/*
	 * Adjust apos field of affected bufm's. a_bufm is either in or adjacent
	 * to the affected range.  Starting at a_bufm, go both directions until
	 * out of the affected range or until past the beginning/end of the
	 * list.  Extra care must be taken to ignore bufm's at the starting apos
	 * if a_bufm is merely adjacent to the affected region.
	 */

	/* Forward (including a_bufm). */
/*  	fprintf(stderr, "%s:%u:%s(): Forward from %llu:", __FILE__, */
/*  	    __LINE__, __FUNCTION__, a_bufm->apos); */
	for (bufm = a_bufm;
	    bufm != NULL && bufm->apos >= a_beg_apos && bufm->apos <
	    a_end_apos;
	    bufm = ql_next(&a_buf->bufms, bufm, link)) {
/*  		fprintf(stderr, " [%llu --> %llu]", bufm->apos, bufm->apos + */
/*  		    a_adjust); */
		bufm->apos += a_adjust;
	}
/*  	fprintf(stderr, "\n"); */

	/* Backward. */
/*  	fprintf(stderr, "%s:%u:%s(): Backward:", __FILE__, __LINE__, */
/*  	    __FUNCTION__); */
	for (bufm = ql_prev(&a_buf->bufms, a_bufm, link);
	    bufm != NULL && bufm->apos == a_end_apos;
	    bufm = ql_prev(&a_buf->bufms, bufm, link)) {
		/* Ignore. */
/*  		fprintf(stderr, " [%llu (ignore)]", bufm->apos); */
	}

	for (;
	    bufm != NULL && bufm->apos >= a_beg_apos;
	    bufm = ql_prev(&a_buf->bufms, bufm, link)) {
/*  		fprintf(stderr, " [%llu --> %llu]", bufm->apos, bufm->apos + */
/*  		    a_adjust); */
		bufm->apos += a_adjust;
	}
/*  	fprintf(stderr, "\n"); */
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

_CW_INLINE void
buf_p_hist_redo_flush(cw_buf_t *a_buf)
{
	/* Flush redo state, if any. */
	if (a_buf->hcur.apos != buf_p_pos_b2a(a_buf, a_buf->len + 1)) {
		bufm_seek(&a_buf->htmp, 0, BUFW_END);
		bufm_remove(&a_buf->hcur, &a_buf->htmp);
	}
}

_CW_INLINE void
buf_p_hist_pos(cw_buf_t *a_buf, cw_uint64_t a_bpos)
{
	cw_uint8_t	hdr;
	union {
		cw_uint64_t	bpos;
		cw_uint8_t	str[8];
	}	u;

	_cw_assert(a_buf->h != NULL);
	_cw_assert(bufm_pos(&a_buf->hcur) == a_buf->len + 1);
	_cw_assert(a_bpos != a_buf->hbpos);

	buf_p_hist_redo_flush(a_buf);

	/* Old position. */
	u.bpos = a_buf->hbpos;
	bufm_before_insert(&a_buf->hcur, u.str, sizeof(a_buf->hbpos));

	/* New position. */
	u.bpos = a_bpos;
	bufm_before_insert(&a_buf->hcur, u.str, sizeof(a_bpos));

	/* Record header. */
	hdr_tag_set(hdr, HDR_TAG_POS);
	bufm_before_insert(&a_buf->hcur, &hdr, sizeof(hdr));

	/* Update hbpos now that the history record is complete. */
	a_buf->hbpos = a_bpos;
}

/*
 * a_what is one of: HDR_TAG_INS, HDR_TAG_YNK, HDR_TAG_REM, HDR_TAG_DEL.
 */
static void
buf_p_hist_text(cw_buf_t *a_buf, cw_uint8_t a_what, cw_uint64_t a_apos, const
    cw_uint8_t *a_str, cw_uint64_t a_len)
{
	cw_uint8_t	*p, hdr, hdr_cnt;
	cw_uint64_t	i, bpos;

	_cw_assert(a_what == HDR_TAG_INS || a_what == HDR_TAG_YNK || a_what ==
	    HDR_TAG_REM || a_what == HDR_TAG_DEL);
	_cw_assert(a_buf->h != NULL);

	buf_p_hist_redo_flush(a_buf);

	/*
	 * Record a position change if necessary.
	 */
	bpos = buf_p_pos_a2b(a_buf, a_apos);
	if (a_buf->hbpos != bpos)
		buf_p_hist_pos(a_buf, bpos);

	i = 0;
	/*
	 * If the last history record is an insertion, and it still has room
	 * left, insert as much of a_str as will fit.
	 */
	if (bufm_pos(&a_buf->hcur) > 1) {
		p = bufm_before_get(&a_buf->hcur);
		if (hdr_tag_get(*p) == a_what && (hdr_cnt = hdr_cnt_get(*p)) <
		    HDR_CNT_MAX) {
			/*
			 * Determine how much of a_str to insert into the
			 * existing record.
			 */
			if (HDR_CNT_MAX - hdr_cnt >= a_len)
				i = a_len;
			else
				i = HDR_CNT_MAX - hdr_cnt;

			/* Update the header before moving hcur. */
			hdr_cnt_set(*p, hdr_cnt + i);

			/* Move htmp before the header and insert text. */
			bufm_dup(&a_buf->htmp, &a_buf->hcur);
			bufm_seek(&a_buf->htmp, -1, BUFW_REL);
			bufm_before_insert(&a_buf->htmp, a_str, i);
		}
	}

	/*
	 * While there is still >= HDR_CNT_MAX characters text to be inserted,
	 * insert full records.
	 */
	for (; a_len - i >= HDR_CNT_MAX; i += HDR_CNT_MAX) {
		bufm_before_insert(&a_buf->hcur, &a_str[i], HDR_CNT_MAX);
		hdr_tag_set(hdr, a_what);
		hdr_cnt_set(hdr, HDR_CNT_MAX);
		bufm_before_insert(&a_buf->hcur, &hdr, sizeof(hdr));
	}

	/*
	 * Insert any remaining characters.
	 */
	if (a_len - i > 0) {
		bufm_before_insert(&a_buf->hcur, &a_str[i], a_len - i);
		hdr_tag_set(hdr, a_what);
		hdr_cnt_set(hdr, a_len - i);
		bufm_before_insert(&a_buf->hcur, &hdr, sizeof(hdr));
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

	retval->msgq = NULL;

	mtx_new(&retval->mtx);

	retval->elmsize = 1;
	retval->b = (cw_uint8_t *)_cw_opaque_alloc(a_alloc, a_arg,
	    _CW_BUF_MINELMS * retval->elmsize);
	retval->len = 0;
	retval->nlines = 1;
	retval->gap_off = 0;
	retval->gap_len = _CW_BUF_MINELMS;

	/* Initialize history. */
	retval->h = NULL;

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

	if (a_buf->h != NULL) {
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

cw_msgq_t *
buf_msgq_get(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	return a_buf->msgq;
}

void
buf_msgq_set(cw_buf_t *a_buf, cw_msgq_t *a_msgq)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	a_buf->msgq = a_msgq;
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

	if (a_buf->h != NULL)
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

	if (a_active == TRUE && a_buf->h == NULL) {
		a_buf->h = buf_new(NULL, a_buf->alloc, a_buf->realloc,
		    a_buf->dealloc, a_buf->arg);
		bufm_new(&a_buf->hcur, a_buf->h, NULL);
		bufm_new(&a_buf->htmp, a_buf->h, NULL);
		a_buf->hbpos = 1;
	} else if (a_active == FALSE && a_buf->h != NULL) {
		bufm_delete(&a_buf->hcur);
		bufm_delete(&a_buf->htmp);
		buf_delete(a_buf->h);
		a_buf->h = NULL;
	}
}

cw_bool_t
buf_undoable(cw_buf_t *a_buf)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->h == NULL) {
		retval = FALSE;
		goto RETURN;
	}

	/* There is at least one undoable operation unless hcur is at BOB. */
	if (bufm_pos(&a_buf->hcur) == 1) {
		retval = FALSE;
		goto RETURN;
	}

	retval = TRUE;
	RETURN:
	return retval;
}

cw_bool_t
buf_redoable(cw_buf_t *a_buf)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->h == NULL) {
		retval = FALSE;
		goto RETURN;
	}

	/* There is at least one redoable operation unless hcur is at EOB. */
	if (a_buf->hcur.apos == buf_p_pos_b2a(a_buf, a_buf->len + 1)) {
		retval = FALSE;
		goto RETURN;
	}

	retval = TRUE;
	RETURN:
	return retval;
}

cw_uint64_t
buf_undo(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t a_count)
{
	cw_uint64_t	retval;
	cw_uint64_t	i;
	cw_uint32_t	gdepth;
  	cw_uint8_t	*p, uhdr, rhdr, c;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->h == NULL || bufm_pos(&a_buf->hcur) == 1) {
		retval = 0;
		goto RETURN;
	}

	/*
	 * Iteratively undo a_count times.
	 */
	for (retval = 0; retval < a_count; retval++) {
		/*
		 * Undo one character at a time (at least one character total)
		 * until the group depth is zero or the entire history has been
		 * undone.
		 */
		for (i = gdepth = 0;
		     (gdepth != 0 || i == 0) && bufm_pos(&a_buf->hcur) > 1;) {
			p = bufm_before_get(&a_buf->hcur);
			uhdr = *p;
			switch (hdr_tag_get(uhdr)) {
			case BUFH_I:
			case BUFH_Y:
			case BUFH_R:
			case BUFH_K:
				/*
				 * Read (or synthesize) the undo header, redo
				 * header, and character, then remove them from
				 * the history buffer.  Then re-insert them in
				 * their new order and state.
				 *
				 * Ther are four possible variations.  In all
				 * cases, hcur is positioned before the
				 * character, and htmp is positioned after the
				 * redo header (if it exists and is being
				 * re-used).  This allows an unconditional call
				 * to bufm_remove().
				 *
				 * rhdr's count is updated before the
				 * bufm_remove(), whereas hdr's count is updated
				 * later.  This is necessary because the count
				 * can never be 0.
				 *
				 * There are more efficient ways of doing this,
				 * but it requires more special case code.
				 */

				/*
				 * Set rhdr and move htmp to the appropriate
				 * position.
				 */
				bufm_dup(&a_buf->htmp, &a_buf->hcur);
				if (buf_redoable(a_buf)) {
					p = bufm_after_get(&a_buf->htmp);
					rhdr = *p;
					if (hdr_tag_inverse(hdr_tag_get(rhdr) ==
					    hdr_tag_get(uhdr))
					    && hdr_cnt_get(rhdr) <
					    HDR_CNT_MAX) {
						hdr_cnt_set(rhdr,
						    hdr_cnt_get(rhdr) + 1);
						bufm_seek(&a_buf->htmp, 1,
						    BUFW_REL);
					} else {
						/* Synthesize a redo header. */
						hdr_tag_set(rhdr,
						    hdr_tag_inverse(uhdr));
						hdr_cnt_set(rhdr, 1);
					}
				} else {
					/* Synthesize a redo header. */
					hdr_tag_set(rhdr,
					    hdr_tag_inverse(uhdr));
					hdr_cnt_set(rhdr, 1);
				}

				/*
				 * Set c and move hcur to the appropriate
				 * position.
				 */
				bufm_seek(&a_buf->hcur, -2, BUFW_REL);
				p = bufm_after_get(&a_buf->hcur);
				c = *p;

				/* Remove the character and header(s). */
				bufm_remove(&a_buf->hcur, &a_buf->htmp);

				/* Insert the character. */
				bufm_after_insert(&a_buf->hcur, &c, 1);
				/* Insert the redo header. */
				bufm_after_insert(&a_buf->hcur, &rhdr, 1);
				/* Insert the undo header if necessary. */
				if (hdr_cnt_get(uhdr) > 1) {
					hdr_cnt_set(uhdr, hdr_cnt_get(uhdr) -
					    1);
					bufm_before_insert(&a_buf->hcur, &uhdr,
					    1);
				}

				i++;
				break;
			case BUFH_B:
				gdepth--;
				bufm_seek(&a_buf->hcur, -1, BUFW_REL);
				break;
			case BUFH_E:
				gdepth++;
				bufm_seek(&a_buf->hcur, -1, BUFW_REL);
				break;
			case BUFH_P: {
				cw_uint64_t	from;
				union {
					cw_uint64_t	bpos;
					cw_uint8_t	str[8];
				}	u;

				/* Read the history record. */
				bufm_dup(&a_buf->htmp, &a_buf->hcur);
				bufm_seek(&a_buf->htmp, -9, BUFW_REL);
				p = bufm_range_get(&a_buf->htmp, &a_buf->hcur);
				bufm_dup(&a_buf->hcur, &a_buf->htmp);

				/* Swap from and to. */
				memcpy(u.str, p, 8);
				from = a_buf->hbpos;
				a_buf->hbpos = u.bpos;
				u.bpos = from;

				/* Invert the history record. */
				*p = p[8];
				memcpy(&p[1], u.str, 8);

				/* Move. */
				bufm_seek(a_bufm, a_buf->hbpos, BUFW_BEG);

				break;
			}
			case BUFH_NONE:
			default:
				_cw_not_reached();
			}
		}
	}

	RETURN:
	return retval;
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
	cw_uint8_t	hdr;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->h != NULL) {
		buf_p_hist_redo_flush(a_buf);

		hdr_tag_set(hdr, HDR_TAG_GRP_BEG);
		bufm_before_insert(&a_buf->hcur, &hdr, sizeof(hdr));
	}
}

void
buf_hist_group_end(cw_buf_t *a_buf)
{
	cw_uint8_t	hdr;

	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->h != NULL) {
		buf_p_hist_redo_flush(a_buf);

		hdr_tag_set(hdr, HDR_TAG_GRP_END);
		bufm_before_insert(&a_buf->hcur, &hdr, sizeof(hdr));
	}
}

void
buf_hist_flush(cw_buf_t *a_buf)
{
	_cw_check_ptr(a_buf);
	_cw_dassert(a_buf->magic == _CW_BUF_MAGIC);

	if (a_buf->h != NULL) {
		bufm_seek(&a_buf->hcur, 0, BUFW_BEG);
		bufm_seek(&a_buf->htmp, 0, BUFW_END);
		bufm_remove(&a_buf->hcur, &a_buf->htmp);
		a_buf->hbpos = 1;
	}
}

/* bufm. */
static void
bufm_p_insert(cw_bufm_t *a_bufm, cw_bool_t a_after, const cw_uint8_t *a_str,
    cw_uint64_t a_len)
{
	cw_uint64_t	i, nlines;
	cw_buf_t	*buf;
	cw_bufm_t	*first, *bufm;

	buf = a_bufm->buf;

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
		if (a_after == FALSE)
			a_bufm->line += nlines;

		/* Adjust line for all following bufm's. */
		for (bufm = ql_next(&buf->bufms, a_bufm, link);
		     bufm != NULL;
		     bufm = ql_next(&buf->bufms, bufm, link))
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
	retval->apos = buf_p_pos_b2a(a_buf, 1);
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

cw_uint8_t *
bufm_range_get(cw_bufm_t *a_start, cw_bufm_t *a_end)
{
	cw_uint8_t	*retval;
	cw_bufm_t	*start, *end;
	cw_buf_t	*buf;

	_cw_check_ptr(a_start);
	_cw_dassert(a_start->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_start->buf);
	_cw_check_ptr(a_end);
	_cw_dassert(a_end->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_end->buf);
	_cw_assert(a_start->buf == a_end->buf);

	buf = a_start->buf;

	if (a_start->apos < a_end->apos) {
		start = a_start;
		end = a_end;
	} else if (a_start->apos > a_end->apos) {
		start = a_end;
		end = a_start;
	} else {
		/* There are no characters between the two bufm's. */
		retval = NULL;
		goto RETURN;
	}

	/* Move the gap if any part of it is between the two bufm's. */
	if (buf->gap_off + buf->gap_len <= start->apos || buf->gap_off >
	    end->apos) {
		/*
		 * Do nothing.  Inverting the logic in the conditional above
		 * isn't worth the obfuscation it causes.
		 */
	} else {
		/* Move the gap to just past the end bufm. */
		buf_p_gap_move(buf, end, buf_p_pos_a2b(buf, end->apos));
	}

	retval = &buf->b[start->apos * buf->elmsize];

	RETURN:
	return retval;
}

void
bufm_before_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_str, cw_uint64_t
    a_len)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	/*
	 * Record the undo information before inserting so that the apos is
	 * still unmodified.
	 */
	if (a_bufm->buf->h != NULL) {
		buf_p_hist_text(a_bufm->buf, HDR_TAG_INS, a_bufm->apos, a_str,
		    a_len);
	}
	bufm_p_insert(a_bufm, FALSE, a_str, a_len);
}

void
bufm_after_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_str, cw_uint64_t a_len)
{
	_cw_check_ptr(a_bufm);
	_cw_dassert(a_bufm->magic == _CW_BUFM_MAGIC);
	_cw_check_ptr(a_bufm->buf);

	if (a_bufm->buf->h != NULL) {
		buf_p_hist_text(a_bufm->buf, HDR_TAG_YNK, a_bufm->apos, a_str,
		    a_len);
	}
	bufm_p_insert(a_bufm, TRUE, a_str, a_len);
}

void
bufm_remove(cw_bufm_t *a_start, cw_bufm_t *a_end)
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

/* 	fprintf(stderr, */
/* 	    "%s(): rcount %llu, bpos %llu<-->%llu, apos %llu<-->%llu\n", */
/* 	    __FUNCTION__, rcount, */
/* 	    start_bpos, end_bpos, */
/* 	    start->apos, end->apos); */

	/* Move the gap. */
	buf_p_gap_move(buf, start, start_bpos);

	/*
	 * Record undo information, now that the gap has been moved and the
	 * elements to be removed are contiguous.  The ordering of a_start and
	 * a_end determines whether this is a before/after removal.
	 */
	if (buf->h != NULL) {
		if (start == a_start) {
			buf_p_hist_text(buf, HDR_TAG_DEL, start_bpos,
			    &buf->b[start->apos * buf->elmsize], rcount);
		} else {
			buf_p_hist_text(buf, HDR_TAG_REM, start_bpos,
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
	     bufm = ql_prev(&buf->bufms, bufm, link)) {
/* 		fprintf(stderr, */
/* 		    "%s(): before: apos: %llu --> %llu\n", */
/* 		    __FUNCTION__, */
/* 		    bufm->apos, end->apos); */
		bufm->apos = end->apos;
	}

	/*
	 * Adjust apos and line for all bufm's from start (inclusive) to end
	 * (exclusive).
	 */
	for (bufm = start;
	     bufm->apos < end->apos;
	     bufm = ql_next(&buf->bufms, bufm, link)) {
/* 		fprintf(stderr, */
/* 		    "%s(): in: apos: %llu --> %llu, line: %llu --> %llu\n", */
/* 		    __FUNCTION__, */
/* 		    bufm->apos, end->apos, */
/* 		    bufm->line, start->line); */
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
/* 		fprintf(stderr, "%s(): after: line: %llu --> %llu\n", */
/* 		    __FUNCTION__, */
/* 		    bufm->line, bufm->line - nlines); */
		bufm->line -= nlines;
	}

	/* Adjust the buf's len and nlines. */
/* 	fprintf(stderr, "%s(): len: %llu --> %llu, lines: %llu --> %llu\n", */
/* 	    __FUNCTION__, */
/* 	    buf->len, buf->len - rcount, */
/* 	    buf->nlines, buf->nlines - nlines); */
	buf->len -= rcount;
	buf->nlines -= nlines;

	/* Try to shrink the gap. */
	buf_p_shrink(buf);
}
