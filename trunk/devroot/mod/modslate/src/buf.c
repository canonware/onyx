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
 * This file contains a paged buffer gap implementation of buffers for use in
 * the slate text editor.  The code is broken up into the following classes:
 *
 * buf  : Main buffer class.
 * bufp : Buffer page.  Each bufb has limited expansion/contraction.
 * mkr  : Marker.  Markers are used as handles for many buf operations.
 * ext  : Extent.  Extents denote buf regions, and are internally composed of
 *        two mkr's.
 *
 * A paged buffer gap is used rather than a single buffer gap in order to
 * provide reasonable scalability with buffer size, even if there are many
 * extents.  Buffer text insertions and deletions become linearly more expensive
 * as the number of markers past the gap increases.  By breaking the buffer into
 * pages, the cost of text insertions and deletions only has to take into
 * account the markers in the affected page.  Pathological worst case scenarios
 * (all markers in one bufp) would have the same scaling problems as a single
 * buffer gap, but it is unrealistic to have more markers than a constant factor
 * times the size of a buffer page.
 *
 * Another advantage to a paged buffer gap is that the cost of gap movement is
 * bounded by the (fixed) page size rather than by the size of the buffer.
 *
 * Unfortunately, the cost of inserting or removing a (randomly placed) page
 * increases linearly with the number of pages.  This is because it is necessary
 * to be able to quickly determine the validity of cached position and line
 * values associated with a page.  The only reasonable way to do this is to keep
 * track of which ranges (sets) of pages have valid caches, and the cheapest way
 * to determine membership in those sets is to number the pages and check if a
 * page's index number falls within a range that is known to have a valid
 * cache.
 *
 * So, there are two aspects in which the paged buffer gap algorithms scale
 * linearly for the common case:
 *
 * 1) M: the number of markers in a page.  This number is approximately
 *    proportional to page size for a buffer that contains markers that are
 *    approximately evenly distributed (typically true).
 *
 * 2) P: the number of pages.
 *
 * Therefore, the cost of paged buffer gap algorithms scales by (M + P).  M is
 * basically fixed, so as a buffer grows, P grows.  Page size must be chosen
 * such that M doesn't cause scaling problems by itself.  Additionally, page
 * size should be low enough that a single page doesn't impose a significant
 * memory burden on the application.  Wisely choosing a page size should allow
 * interactive buffer manipulation of buffers several gigabytes large.
 *
 ******************************************************************************
 *
 * Buffer position numbering starts at 1.  Buffer position 0 is invalid.
 *
 * Position rules:
 *
 * *) ppos refers to absolute position within a bufp.
 * *) bpos refers to buffer position.
 * *) If a position isn't specified as ppos or bpos, then it is bpos.
 *
 * Internal buffer page representation:
 *
 * ppos:   0   1   2   3   4   5   6   7     0   1   2   3   4   5   6   7
 *         |   |   |   |   |   |   |   |     |   |   |   |   |   |   |   |
 *         v   v   v   v   v   v   v   v     v   v   v   v   v   v   v   v
 *       /---+---+---+---+---+---+---+---\ /---+---+---+---+---+---+---+---\
 *       | A | B | \n|:::|:::|:::| C | D | | \n| E |:::|:::|:::|:::| \n| F |
 *       \---+---+---+---+---+---+---+---/ \---+---+---+---+---+---+---+---/
 *       ^   ^   ^               ^   ^     ^   ^                       ^   ^
 *       |   |   |               |   |     |   |                       |   |
 * bpos: 1   2   3               4   5     6   7                       8   9
 *       |   |   |               |   |     |   |                       |   |
 * line: 1   1   1               2   2     2   3                       4   4
 *       |   |   |               |   |     |   |                       |   |
 * pline:0   0   0               1   1     0   1                       1   2
 *
 *       bufp->bpos: 1                     bufp->bpos: 6
 *       bufp->line: 1                     bufp->line: 2
 *
 ******************************************************************************
 *
 * Under normal circumstances, it would make the most sense from a human point
 * of view to store the buffer gap offset and length, but since moving buf's
 * bufp_cur must be efficient, we instead store the buffer gap offset and total
 * number of valid characters.  The buffer gap length can be derived from this,
 * since the page size is fixed at CW_BUFP_SIZE.
 *
 *         /------- len ------\
 *        /                    \
 * |-----------|           |-------|
 *
 * |--------- CW_BUFP_SIZE --------|
 * /---+---+---+---+---+---+---+---\
 * | A | B | C |:::|:::|:::| D | E |
 * \---+---+---+---+---+---+---+---/
 *   ^           ^           ^
 *   |           |           |
 *   0           gap_off     gap_off + (CW_BUFP_SIZE - len)
 *
 ******************************************************************************
 *
 * bufp's, mkr's, and ext's are all organized using both red-black trees and
 * doubly linked lists, so that random access and iteration are fast.
 *
 * Searching for a bufp by bpos or line is somewhat tricky, since not all nodes
 * necessarily have valid caches.  However, it is still possible to do such
 * searches by taking a bit of extra care in the node comparison function.
 * Since each bufp has an index number, nodes that fall outside the range of
 * those with valid caches are known to be greater than the result for searches
 * within the range starting at BOB, and less than the result for searches
 * within the range ending at EOB.
 *
 ******************************************************************************
 *
 * Each bufp keeps track of its bpos and line to speed up many operations.  buf
 * modifications can require the values stored in bufp's to be converted between
 * being relative to BOB/EOB.  At any given time, the ranges of bufp's with
 * valid caches may look something like:
 *
 *    0           1           2           3           4           5
 * /------\    /------\    /------\    /------\    /------\    /------\
 * | bufp |<-->| bufp |<-->| bufp |<-->| bufp |<-->| bufp |<-->| bufp |
 * \------/    \------/    \------/    \------/    \------/    \------/
 *  |           |           |  ^        |           |           |
 * ||-----------|-----------|  |        |-----------|-----------|-----|
 *  1                          bufp_cur                               len + 1
 *
 * In this example, bufp's 0..2 know their positions relative to BOB, and bufp's
 * 3..5 know their positions relative to EOB.  If a subsequent modification is
 * made in bufp 4, bufp_cur must be moved to point to it, which means converting
 * bufp's 3..4 to store their positions relative to BOB:
 *
 *    0           1           2           3           4           5
 * /------\    /------\    /------\    /------\    /------\    /------\
 * | bufp |<-->| bufp |<-->| bufp |<-->| bufp |<-->| bufp |<-->| bufp |
 * \------/    \------/    \------/    \------/    \------/    \------/
 *  |           |           |           |           |  ^        |
 * ||-----------|-----------|-----------|-----------|  |        |-----|
 *  1                                                  bufp_cur       len + 1
 *
 ******************************************************************************
 *
 * Each buffer page keeps a tree/list of markers that point to the page.  Marker
 * position is maintained as ppos, so that when data are inserted or deleted,
 * only the markers within the modified range must be updated.  If marker
 * position were maintained as bpos, then all markers after the modified range
 * would also have to be updated.
 *
 * A down side to this approach is that when the gap is moved, the markers that
 * point into the moved range of text must be updated.  However, this typically
 * happens much less often than insertions and deletions.
 *
 ******************************************************************************
 *
 * Extents keep track of buffer ranges, and are typically used to associate
 * attributes (primarily for color syntax highlighting) with those ranges.  The
 * end points of an extent are denoted by markers, which are no different than
 * other markers.  This is the primary motivation for making marker operations
 * scalable, since a typical buffer has only a handfull of markers, until
 * extents come into play, at which time the number of markers can quickly
 * become huge.
 *
 * An extent's end points are each open or closed:
 *
 *   * Open : Insertion at the end point causes the text to go outside the
 *            extent.
 *
 *   * Closed : Insertion at the end point causes the text to go inside the
 *              extent.
 *
 * This gives rise to four open/closed combinations:
 *
 *   * Closed-closed (default)
 *
 *   * Closed-open
 *
 *   * Open-closed
 *
 *   * Open-open (can't be zero-length)
 *
 * The behavior of the various cases is as expected, except that zero-length
 * open-open extents are not allowed to exist.  If buffer operations shrink an
 * open-open extent to zero length, it is converted to closed-open.
 *
 * An extent can be detachable, which means that if the extent shrinks to zero
 * length, it is detached from the buffer.
 *
 ******************************************************************************
 *
 * Extents are ordered two different ways.  For extents A and B, where beg(X) is
 * the beginning position of X and end(X) is the ending position of X:
 *
 *   * Forward order : if ((beg(A) < beg(B))
 *                         || (start(A) == start(B) && end(A) > end(B)))
 *                     {
 *                         A < B
 *                     }
 *
 *   * Reverse order : if ((end(A) < end(B))
 *                         || (end(A) == end(B) && beg(A) > beg(B)))
 *                     {
 *                         A < B
 *                     }
 *
 * Following are examples of extents shown in forward- and reverse-order
 * (abbreviated as f-order and r-order):
 *
 *   f-order :
 *
 *     A : |---------|
 *     B : |-------|
 *     C :   |---------|
 *     D :   |-------|
 *     E :   |-----|
 *     F :   |---|
 *     G :       |-------|
 *     H :       |-----|
 *     I :         |-----------|
 *     J :           |---|
 *     K :             |---|
 *     L :               |-------|
 *
 *   r-order :
 *
 *     F :   |---|
 *     E :   |-----|
 *     B : |-------|
 *     D :   |-------|
 *     A : |---------|
 *     H :       |-----|
 *     C :   |---------|
 *     J :           |---|
 *     G :       |-------|
 *     K :             |---|
 *     I :         |-----------|
 *     L :               |-------|
 *
 * Maintaining both orderings makes it possible to quickly determine the set of
 * extents that overlap any range of the buffer.  Most importantly though, the
 * two orderings make it possible to quickly divide the buffer into fragments,
 * where each fragment is completely overlapped by a particular set of extents.
 * This is used when displaying the buffer.
 *
 ******************************************************************************
 *
 * In order to avoid excessive internal fragmentation, buffer pages are
 * coalesced during data deletion, such that for every pair of consecutive
 * pages, both are at least 25% full.  In addition, special care is taken during
 * insertion to assure that this requirement is never violated when inserting
 * new pages.
 *
 ******************************************************************************/

#include "../include/modslate.h"

/* Prototypes. */
/* XXX */
static cw_sint32_t
mkr_p_comp(cw_mkr_t *a_a, cw_mkr_t *a_b);

/* bufv. */

/* A simplified version of bufv_copy() that counts '\n' characters that are
 * copied, and returns that rather than the number of elements copied. */
CW_INLINE cw_uint32_t
bufv_p_copy(cw_bufv_t *a_to, cw_uint32_t a_to_len, const cw_bufv_t *a_fr,
	    cw_uint32_t a_fr_len)
{
    cw_uint32_t retval, to_el, fr_el, to_off, fr_off;

    cw_check_ptr(a_to);
    cw_check_ptr(a_fr);

    retval = 0;
    to_el = 0;
    to_off = 0;
    /* Iterate over bufv elements. */
    for (fr_el = 0; fr_el < a_fr_len; fr_el++)
    {
	/* Iterate over bufv element contents. */
	for (fr_off = 0; fr_off < a_fr[fr_el].len; fr_off++)
	{
	    a_to[to_el].data[to_off] = a_fr[fr_el].data[fr_off];

	    /* Count newlines. */
	    if (a_to[to_el].data[to_off] == '\n')
	    {
		retval++;
	    }

	    /* Increment the position to copy to. */
	    to_off++;
	    if (to_off == a_to[to_el].len)
	    {
		to_off = 0;
		to_el++;
		if (to_el == a_to_len)
		{
		    goto RETURN;
		}
	    }
	}
    }

    RETURN:
    return retval;
}

cw_uint64_t
bufv_copy(cw_bufv_t *a_to, cw_uint32_t a_to_len, const cw_bufv_t *a_fr,
	  cw_uint32_t a_fr_len, cw_uint64_t a_maxlen)
{
    cw_uint64_t retval;
    cw_uint32_t to_el, fr_el, to_off, fr_off;

    cw_check_ptr(a_to);
    cw_check_ptr(a_fr);

    retval = 0;
    to_el = 0;
    to_off = 0;
    /* Iterate over bufv elements. */
    for (fr_el = 0; fr_el < a_fr_len; fr_el++)
    {
	/* Iterate over bufv element contents. */
	for (fr_off = 0; fr_off < a_fr[fr_el].len; fr_off++)
	{
	    a_to[to_el].data[to_off] = a_fr[fr_el].data[fr_off];

	    /* Copy no more than a_maxlen elements (unless a_maxlen is 0). */
	    retval++;
	    if (retval == a_maxlen)
	    {
		goto RETURN;
	    }

	    /* Increment the position to copy to. */
	    to_off++;
	    if (to_off == a_to[to_el].len)
	    {
		to_off = 0;
		to_el++;
		if (to_el == a_to_len)
		{
		    goto RETURN;
		}
	    }
	}
    }

    RETURN:
    return retval;
}

/* bufp. */
CW_INLINE cw_uint64_t
bufp_p_bpos(cw_bufp_t *a_bufp)
{
    cw_uint64_t retval;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    if (a_bufp->bob_relative)
    {
	retval = a_bufp->bpos;
    }
    else
    {
	retval = a_bufp->buf->len + 1 - a_bufp->bpos;
    }

    return retval;
}

CW_INLINE cw_uint64_t
bufp_p_line(cw_bufp_t *a_bufp)
{
    cw_uint64_t retval;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    if (a_bufp->bob_relative)
    {
	retval = a_bufp->line;
    }
    else
    {
	retval = a_bufp->buf->nlines + 1 - a_bufp->line;
    }

    return retval;
}

static cw_sint32_t
bufp_p_comp(cw_bufp_t *a_a, cw_bufp_t *a_b)
{
    cw_sint32_t retval;

    if (bufp_p_bpos(a_a) < bufp_p_bpos(a_b))
    {
	retval = -1;
    }
    else
    {
	/* New bufp's are empty when they are inserted by mkr_l_insert().  As a
	 * result, this function will claim that a_a is greater than a_b.  This
	 * is important, since the empty bufp must come after the non-empty
	 * bufp. */
	retval = 1;
    }

    return retval;
}

/* Adjust ppos field of a range of mkr's. */
static void
bufp_p_mkrs_ppos_adjust(cw_bufp_t *a_bufp, cw_sint32_t a_adjust,
			cw_uint32_t a_beg_ppos, cw_uint32_t a_end_ppos)
{
    cw_mkr_t *mkr, key;

    cw_check_ptr(a_bufp);
    cw_assert(a_beg_ppos < a_end_ppos);

    /* Find the first affected mkr. */
    key.ppos = a_beg_ppos;
    rb_nsearch(&a_bufp->mtree, &key, mkr_p_comp, cw_mkr_t, mnode, mkr);

    for (;
	 mkr != NULL && mkr->ppos < a_end_ppos;
	 mkr = ql_next(&a_bufp->mlist, mkr, mlink))
    {
	mkr->ppos += a_adjust;
    }
}

static void
bufp_p_gap_move(cw_bufp_t *a_bufp, cw_uint32_t a_ppos)
{
    cw_assert(a_ppos < CW_BUFP_SIZE);

    /* Move the gap if it isn't already where it needs to be. */
    if (a_bufp->gap_off != a_ppos)
    {
	if (a_bufp->gap_off < a_ppos)
	{
	    /* Move the gap forward.
	     *
	     * o: data
	     * M: move
	     * _: gap
	     *
	     * ooooooo________MMMMMMMMMMMoo
	     *                   ^
	     *                   |
	     *                   a_ppos
	     *                   |
	     *                   v
	     * oooooooMMMMMMMMMMM________oo */
	    memmove(&a_bufp->b[a_bufp->gap_off],
		    &a_bufp->b[a_bufp->gap_off + (CW_BUFP_SIZE - a_bufp->len)],
		    (a_ppos - a_bufp->gap_off));

	    /* Adjust the ppos of all mkr's with ppos in the moved region. */
	    bufp_p_mkrs_ppos_adjust(a_bufp, -(CW_BUFP_SIZE - a_bufp->len),
				    a_bufp->gap_off + (CW_BUFP_SIZE
						       - a_bufp->len),
				    a_ppos + (CW_BUFP_SIZE - a_bufp->len));
	}
	else
	{
	    /* Move the gap backward.
	     *
	     * o: data
	     * M: move
	     * _: gap
	     *
	     * ooooMMMMMMMMM___________oooo
	     *     ^
	     *     |
	     *     a_ppos
	     *     |
	     *     v
	     * oooo___________MMMMMMMMMoooo */
	    memmove(&a_bufp->b[(CW_BUFP_SIZE - a_bufp->len) + a_ppos],
		    &a_bufp->b[a_ppos],
		    (a_bufp->gap_off - a_ppos));

	    /* Adjust the ppos of all mkr's with ppos in the moved region. */
	    bufp_p_mkrs_ppos_adjust(a_bufp, CW_BUFP_SIZE - a_bufp->len, a_ppos,
				    a_bufp->gap_off);
	}
	a_bufp->gap_off = a_ppos;
    }
}

static cw_bufp_t *
bufp_p_new(cw_buf_t *a_buf)
{
    cw_bufp_t *retval;

    /* Allocate. */
    retval = (cw_bufp_t *) cw_opaque_alloc(a_buf->alloc, a_buf->arg,
					   sizeof(cw_bufp_t));

    /* Don't bother initializing bob_relative, bpos, or line, since they are
     * explicitly set later. */

    /* Initialize length and line count. */
    retval->len = 0;
    retval->nlines = 0;

    /* Initialize buffer gap. */
    retval->gap_off = 0;

    /* Allocate buffer. */
    retval->b = (cw_uint8_t *) cw_opaque_alloc(a_buf->alloc, a_buf->arg,
					       CW_BUFP_SIZE);

    /* Initialize marker tree and list. */
    rb_tree_new(&retval->mtree, mnode);
    ql_new(&retval->mlist);

#ifdef CW_DBG
    retval->magic = CW_BUFP_MAGIC;
#endif

    return retval;
}

static void
bufp_p_delete(cw_bufp_t *a_bufp)
{
    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);
#ifdef CW_DBG
    {
	cw_mkr_t *first;
	rb_first(&a_bufp->mtree, mnode, first);
	cw_assert(first == NULL);
    }
#endif
    cw_assert(ql_first(&a_bufp->mlist) == NULL);
    cw_assert(qr_next(a_bufp, plink) == NULL);

    cw_opaque_dealloc(a_bufp->buf->dealloc, a_bufp->buf->arg, a_bufp->b,
		      CW_BUFP_SIZE);

    cw_opaque_dealloc(a_bufp->buf->dealloc, a_bufp->buf->arg, a_bufp,
		      sizeof(cw_bufp_t));
}

static cw_uint32_t
bufp_p_pos_b2p(cw_bufp_t *a_bufp, cw_uint64_t a_bpos)
{
    cw_uint32_t ppos, rel_bpos;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);
    cw_assert(a_bpos > 0);
    cw_assert(a_bpos <= a_bufp->buf->len + 1);

    /* Calculate the offset into bufp up front. */
    cw_assert(a_bpos >= bufp_p_bpos(a_bufp));
    rel_bpos = a_bpos - bufp_p_bpos(a_bufp);

    if (rel_bpos <= a_bufp->gap_off)
    {
	ppos = rel_bpos;
    }
    else
    {
	ppos = rel_bpos + (CW_BUFP_SIZE - a_bufp->len);
    }

    return ppos;
}

static cw_uint32_t
bufp_p_pos_p2b(cw_bufp_t *a_bufp, cw_uint64_t a_ppos)
{
    cw_uint32_t bpos;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);
    cw_assert(a_ppos <= a_bufp->gap_off
	      || a_ppos >= a_bufp->gap_off + (CW_BUFP_SIZE - a_bufp->len));

    if (a_ppos <= a_bufp->gap_off)
    {
	bpos = a_ppos + bufp_p_bpos(a_bufp);
    }
    else
    {
	bpos = a_ppos - (CW_BUFP_SIZE - a_bufp->len) + bufp_p_bpos(a_bufp);
    }

    return bpos;
}

/* buf. */
static cw_sint32_t
buf_p_bufp_at_bpos_comp(cw_bufp_t *a_key, cw_bufp_t *a_bufp)
{
    cw_sint32_t retval;
    cw_uint64_t bpos = bufp_p_bpos(a_bufp);

    if (a_key->bpos < bpos)
    {
	retval = -1;
    }
    else if (a_key->bpos < bpos + a_bufp->len)
    {
	retval = 0;
    }
    else
    {
	retval = 1;
    }

    return retval;
}

static cw_bufp_t *
buf_p_bufp_at_bpos(cw_buf_t *a_buf, cw_uint64_t a_bpos)
{
    cw_bufp_t *retval, key;

    /* Initialize enough of key for searching. */
    key.bpos = a_bpos;

    rb_search(&a_buf->ptree, &key, buf_p_bufp_at_bpos_comp, pnode, retval);
    cw_check_ptr(retval);

    return retval;
}

static cw_sint32_t
buf_p_bpos_lf_comp(cw_bufp_t *a_key, cw_bufp_t *a_bufp)
{
    cw_sint32_t retval;
    cw_uint64_t line = bufp_p_line(a_bufp);

    if (a_key->line < line)
    {
	retval = -1;
    }
    else if (a_key->line < line + a_bufp->nlines)
    {
	retval = 0;
    }
    else
    {
	retval = 1;
    }

    return retval;
}

static cw_uint64_t
buf_p_bpos_before_lf(cw_buf_t *a_buf, cw_uint64_t a_line, cw_bufp_t **r_bufp)
{
    cw_uint32_t ppos, nlines;
    cw_uint64_t retval, bufp_line;
    cw_bufp_t *bufp, key;

    /* Initialize enough of key for searching. */
    key.line = a_line;

    rb_search(&a_buf->ptree, &key, buf_p_bpos_lf_comp, pnode, bufp);
    cw_check_ptr(bufp);

    bufp_line = bufp_p_line(bufp);

    /* Before the gap. */
    for (ppos = nlines = 0; ppos < bufp->gap_off; ppos++)
    {
	if (bufp->b[ppos] == '\n')
	{
	    nlines++;
	    if (nlines + bufp_line == a_line)
	    {
		retval = bufp_p_bpos(bufp) + ppos;
		goto DONE;
	    }
	}
    }

    /* After the gap. */
    for (ppos += (CW_BUFP_SIZE - bufp->len);; ppos++)
    {
	cw_assert(ppos < CW_BUFP_SIZE);

	if (bufp->b[ppos] == '\n')
	{
	    nlines++;
	    if (nlines + bufp_line == a_line)
	    {
		retval = bufp_p_bpos(bufp) + ppos - (CW_BUFP_SIZE - bufp->len);
		goto DONE;
	    }
	}
    }

    DONE:
    *r_bufp = bufp;
    return retval;
}

static cw_uint64_t
buf_p_bpos_after_lf(cw_buf_t *a_buf, cw_uint64_t a_line, cw_bufp_t **r_bufp)
{
    cw_uint64_t bpos;
    cw_bufp_t *bufp;

    bpos = buf_p_bpos_before_lf(a_buf, a_line, &bufp);

    /* Move forward one position.  This could involve moving to the next
     * bufp. */
    if (bpos - bufp_p_bpos(bufp) + 1 >= bufp->len)
    {
	/* Move to the next bufp. */
	bufp = ql_next(&bufp->buf->plist, bufp, plink);
    }

    *r_bufp = bufp;
    return bpos + 1;
}

static void
buf_p_bufp_cur_set(cw_buf_t *a_buf, cw_bufp_t *a_bufp)
{
    cw_assert(a_bufp->buf == a_buf);

    if (a_buf->bufp_cur != a_bufp)
    {
	cw_bufp_t *bufp;
	cw_uint64_t bpos, line;

	if (bufp_p_bpos(a_bufp) > bufp_p_bpos(a_buf->bufp_cur))
	{
	    /* Move forward. */
	    bufp = a_buf->bufp_cur;
	    bpos = bufp->bpos;
	    line = bufp->line;
	    do
	    {
		bpos += bufp->len;
		line += bufp->line;

		bufp = ql_next(&a_buf->plist, bufp, plink);
		cw_check_ptr(bufp);
		cw_assert(bufp->bob_relative == FALSE);

		bufp->bob_relative = TRUE;
		bufp->bpos = bpos;
		bufp->line = line;
	    } while (bufp != a_bufp);
	    a_buf->bufp_cur = bufp;
	}
	else
	{
	    /* Move backward. */
	    bufp = a_buf->bufp_cur;
	    bpos = bufp->line;
	    line = bufp->line;
	    do
	    {
		bufp = ql_prev(&a_buf->plist, bufp, plink);
		cw_check_ptr(bufp);
		cw_assert(bufp->bob_relative);

		bpos += bufp->len;
		line += bufp->line;

		bufp->bob_relative = FALSE;
		bufp->bpos = bpos;
		bufp->line = line;
	    } while (bufp != a_bufp);
	    a_buf->bufp_cur = bufp;
	}
    }
}

/* bufv resizing must be done manually. */
static void
buf_p_bufp_insert(cw_buf_t *a_buf, cw_bufp_t *a_bufp)
{
    cw_bufp_t *next;

    /* Insert into tree. */
    rb_insert(&a_buf->ptree, a_bufp, bufp_p_comp, cw_bufp_t, pnode);

    /* Insert into list. */
    rb_next(&a_buf->ptree, a_bufp, cw_bufp_t, pnode, next);
    if (next != NULL)
    {
	ql_before_insert(&a_buf->plist, next, a_bufp, plink);
    }
    else
    {
	ql_head_insert(&a_buf->plist, a_bufp, plink);
    }
}

/* bufv resizing must be done manually. */
#ifdef NOT_YET
static void
buf_p_bufp_remove(cw_buf_t *a_buf, cw_bufp_t *a_bufp)
{
    rb_remove(&a_buf->ptree, a_bufp, cw_bufp_t, pnode);
    ql_remove(&a_buf->plist, a_bufp, plink);
}
#endif

cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
	cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc,
	void *a_arg)
{
    cw_buf_t *retval;
    cw_bufp_t *bufp;

    /* Allocate buf. */
    if (a_buf != NULL)
    {
	retval = a_buf;
/* 	memset(retval, 0, sizeof(cw_buf_t)); */
	retval->alloced = FALSE;
    }
    else
    {
	retval = (cw_buf_t *) cw_opaque_alloc(a_alloc, a_arg, sizeof(cw_buf_t));
/* 	memset(retval, 0, sizeof(cw_buf_t)); */
	retval->alloced = TRUE;
    }

    /* Initialize internal allocator pointers. */
    retval->alloc = a_alloc;
    retval->realloc = a_realloc;
    retval->dealloc = a_dealloc;
    retval->arg = a_arg;

    /* Set size. */
    retval->len = 0;
    retval->nlines = 1;

    /* Initialize bufp tree and list. */
    rb_tree_new(&retval->ptree, pnode);
    ql_new(&retval->plist);
    
    /* Initialize and insert initial bufp. */
    bufp = bufp_p_new(retval);
    bufp->bob_relative = TRUE;
    bufp->bpos = 1;
    bufp->line = 1;
    buf_p_bufp_insert(retval, bufp);

    /* Initialize current bufp. */
    retval->bufp_cur = bufp;

    /* Initialize bufv to have two elements, since the buf starts out with one
     * bufp. */
    retval->bufv = (cw_bufv_t *) cw_opaque_alloc(a_alloc, a_arg,
						 2 * sizeof(cw_bufv_t));
    retval->bufvcnt = 2;

    /* Initialize extent trees and lists. */
    rb_tree_new(&retval->ftree, fnode);
    ql_new(&retval->flist);
    rb_tree_new(&retval->rtree, rnode);
    ql_new(&retval->rlist);

    /* Initialize history. */
    retval->hist = NULL;

#ifdef CW_DBG
    retval->magic = CW_BUF_MAGIC;
#endif

    return retval;
}

void
buf_delete(cw_buf_t *a_buf)
{
    cw_bufp_t *bufp;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);
#ifdef CW_DBG
    {
	cw_ext_t *first;
	rb_first(&a_buf->ftree, fnode, first);
	cw_assert(first == NULL);
    }
#endif
    cw_assert(ql_first(&a_buf->flist) == NULL);
#ifdef CW_DBG
    {
	cw_ext_t *first;
	rb_first(&a_buf->rtree, rnode, first);
	cw_assert(first == NULL);
    }
#endif
    cw_assert(ql_first(&a_buf->rlist) == NULL);

    /* Delete history if it exists. */
    if (a_buf->hist != NULL)
    {
	hist_delete(a_buf->hist);
    }

    /* Iteratively delete bufp's. */
    for (bufp = ql_last(&a_buf->plist, plink);
	 bufp != NULL;
	 bufp = ql_last(&a_buf->plist, plink))
    {
	ql_remove(&a_buf->plist, bufp, plink);
	bufp_p_delete(bufp);
    }

    /* Delete the bufv array. */
    cw_assert(a_buf->bufvcnt >= 2);
    cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf->bufv,
		      a_buf->bufvcnt * sizeof(cw_bufv_t));

    if (a_buf->alloced)
    {
	cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf, sizeof(cw_buf_t));
    }
#ifdef CW_DBG
    else
    {
	memset(a_buf, 0x5a, sizeof(cw_buf_t));
    }
#endif
}

cw_uint64_t
buf_len(const cw_buf_t *a_buf)
{
    cw_uint64_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    retval = a_buf->len;

    return retval;
}

cw_uint64_t
buf_nlines(const cw_buf_t *a_buf)
{
    cw_uint64_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    retval = a_buf->nlines;

    return retval;
}

cw_bool_t
buf_hist_active_get(const cw_buf_t *a_buf)
{
    cw_bool_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_buf->hist != NULL)
    {
	retval = TRUE;
    }
    else
    {
	retval = FALSE;
    }

    return retval;
}

void
buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active)
{
    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_active == TRUE && a_buf->hist == NULL)
    {
	a_buf->hist = hist_new(a_buf->alloc, a_buf->realloc, a_buf->dealloc,
			       a_buf->arg);
    }
    else if (a_active == FALSE && a_buf->hist != NULL)
    {
	hist_delete(a_buf->hist);
	a_buf->hist = NULL;
    }
}

cw_bool_t
buf_undoable(const cw_buf_t *a_buf)
{
    cw_bool_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_buf->hist == NULL)
    {
	retval = FALSE;
	goto RETURN;
    }

    retval = hist_undoable(a_buf->hist, a_buf);

    RETURN:
    return retval;
}

cw_bool_t
buf_redoable(const cw_buf_t *a_buf)
{
    cw_bool_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_buf->hist == NULL)
    {
	retval = FALSE;
	goto RETURN;
    }

    retval = hist_redoable(a_buf->hist, a_buf);

    RETURN:
    return retval;
}

cw_uint64_t
buf_undo(cw_buf_t *a_buf, cw_mkr_t *a_mkr, cw_uint64_t a_count)
{
    cw_uint64_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_buf->hist == NULL)
    {
	retval = 0;
	goto RETURN;
    }

    retval = hist_undo(a_buf->hist, a_buf, a_mkr, a_count);

    RETURN:
    return retval;
}

cw_uint64_t
buf_redo(cw_buf_t *a_buf, cw_mkr_t *a_mkr, cw_uint64_t a_count)
{
    cw_uint64_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_buf->hist == NULL)
    {
	retval = 0;
	goto RETURN;
    }

    retval = hist_redo(a_buf->hist, a_buf, a_mkr, a_count);

    RETURN:
    return retval;
}

void
buf_hist_flush(cw_buf_t *a_buf)
{
    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_buf->hist != NULL)
    {
	hist_flush(a_buf->hist, a_buf);
    }
}

void
buf_hist_group_beg(cw_buf_t *a_buf, cw_mkr_t *a_mkr)
{
    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_buf->hist != NULL)
    {
	hist_group_beg(a_buf->hist, a_buf, a_mkr);
    }
}

cw_bool_t
buf_hist_group_end(cw_buf_t *a_buf)
{
    cw_bool_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_buf->hist != NULL)
    {
	retval = hist_group_end(a_buf->hist, a_buf);
    }
    else
    {
	retval = TRUE;
    }

    return retval;
}

/* mkr. */
static cw_uint64_t
mkr_p_bpos(cw_mkr_t *a_mkr)
{
    return (bufp_p_bpos(a_mkr->bufp) + a_mkr->ppos);
}

static cw_uint64_t
mkr_p_line(cw_mkr_t *a_mkr)
{
    return (bufp_p_line(a_mkr->bufp) + a_mkr->pline);
}

static cw_sint32_t
mkr_p_comp(cw_mkr_t *a_a, cw_mkr_t *a_b)
{
    cw_sint32_t retval;

    cw_check_ptr(a_a);
    cw_dassert(a_a->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_a->bufp);
    cw_check_ptr(a_b);
    cw_dassert(a_b->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_b->bufp);
    cw_assert(a_a->bufp->buf == a_b->bufp->buf);

    if (a_a->bufp == a_b->bufp)
    {
	if (a_a->ppos < a_b->ppos)
	{
	    retval = -1;
	}
	else if (a_a->ppos > a_b->ppos)
	{
	    retval = 1;
	}
	else
	{
	    retval = 0;
	}
    }
    else
    {
	/* XXX Does this code ever get used anywhere? */
	if (bufp_p_bpos(a_a->bufp) < bufp_p_bpos(a_b->bufp))
	{
	    retval = -1;
	}
	else
	{
	    retval = 1;
	}
    }

    return retval;
}

static void
mkr_p_insert(cw_mkr_t *a_mkr)
{
    cw_bufp_t *bufp = a_mkr->bufp;
    cw_mkr_t *next;

    /* Insert into tree. */
    rb_insert(&bufp->mtree, a_mkr, mkr_p_comp, cw_mkr_t, mnode);

    /* Insert into list.  Make sure that the tree and list orders are the same.
     */
    rb_next(&bufp->mtree, a_mkr, cw_mkr_t, mnode, next);
    if (next != NULL)
    {
	ql_before_insert(&bufp->mlist, next, a_mkr, mlink);
    }
    else
    {
	ql_head_insert(&bufp->mlist, a_mkr, mlink);
    }
}

static void
mkr_p_remove(cw_mkr_t *a_mkr)
{
    cw_bufp_t *bufp = a_mkr->bufp;

    rb_remove(&bufp->mtree, a_mkr, cw_mkr_t, mnode);
    ql_remove(&bufp->mlist, a_mkr, mlink);
}

/* Insert data into a single bufp, without moving the gap.  This function
 * assumes that the bufp internals are consistent, and that the data will
 * fit. */
/* XXX CW_INLINE? */
static void
mkr_p_raw_insert(cw_bufp_t *a_bufp, const cw_bufv_t *a_bufv,
		 cw_uint32_t a_bufvcnt, cw_uint32_t a_count)
{
    cw_uint32_t nlines;
    cw_bufv_t bufv;

    /* Insert. */
    bufv.data = &a_bufp->b[a_bufp->gap_off];
    bufv.len = CW_BUFP_SIZE - a_bufp->len;
    nlines = bufv_p_copy(&bufv, 1, a_bufv, a_bufvcnt);

    /* Shrink the gap. */
    a_bufp->gap_off += a_count;

    /* Adjust the buf's length and line count. */
    a_bufp->len += a_count;
    a_bufp->nlines += nlines;
}

/* Insert data into a single bufp.  This function assumes that the bufp
 * internals are consistent, and that the data will fit. */
static void
mkr_p_simple_insert(cw_mkr_t *a_mkr, cw_bool_t a_after, const cw_bufv_t *a_bufv,
		    cw_uint32_t a_bufvcnt, cw_uint32_t a_count)
{
    cw_uint32_t nlines;
    cw_mkr_t *first, *mkr;
    cw_buf_t *buf;
    cw_bufp_t *bufp;

    cw_assert(a_count <= CW_BUFP_SIZE - a_mkr->bufp->len);

    bufp = a_mkr->bufp;
    buf = bufp->buf;

    /* Move bufp_cur. */
    buf_p_bufp_cur_set(buf, bufp);

    /* Move the gap. */
    bufp_p_gap_move(bufp, a_mkr->ppos);

    /* Insert. */
    mkr_p_raw_insert(bufp, a_bufv, a_bufvcnt, a_count);

    /* Find the first mkr that is at the same ppos as a_mkr.  This may be
     * a_mkr. */
    for (first = a_mkr, mkr = ql_prev(&bufp->mlist, a_mkr, mlink);
	 mkr != NULL && mkr->ppos == a_mkr->ppos;
	 mkr = ql_prev(&bufp->mlist, mkr, mlink))
    {
	first = mkr;
    }

    /* If inserting after a_mkr, move a_mkr before the data just inserted. */
    if (a_after)
    {
	if (first == a_mkr)
	{
	    /* Adjust first, since a_mkr won't need adjusted. */
	    first = ql_next(&bufp->mlist, first, mlink);
	}
	else
	{
	    /* Remove, then reinsert a_mkr. */
	    mkr_p_remove(a_mkr);
	    a_mkr->ppos = bufp_p_pos_b2p(bufp,
					 bufp_p_pos_p2b(bufp, a_mkr->ppos)
					 - a_count);
	    mkr_p_insert(a_mkr);

	    cw_assert(ql_next(&bufp->mlist, a_mkr, mlink) == first);
	}
    }

    if (nlines > 0)
    {
	/* Adjust line. */
	if (a_after == FALSE)
	{
	    /* Adjust line for all mkr's at the same position. */
	    for (mkr = first;
		 mkr != NULL && mkr->ppos == a_mkr->ppos;
		 mkr = ql_next(&bufp->mlist, mkr, mlink))
	    {
		mkr->pline += nlines;
	    }
	}
	else
	{
	    /* Move past mkr's at the same position. */
	    for (mkr = a_mkr; /* a_mkr may be farther than first. */
		 mkr != NULL && mkr->ppos == a_mkr->ppos;
		 mkr = ql_next(&bufp->mlist, mkr, mlink))
	    {
		/* Do nothing. */
	    }
	}

	/* Adjust line for all following mkr's. */
	for (;
	     mkr != NULL;
	     mkr = ql_next(&bufp->mlist, mkr, mlink))
	{
	    mkr->pline += nlines;
	}
    }
}

static void
mkr_p_slide_before_insert(cw_mkr_t *a_mkr, cw_bufp_t *a_prevp,
			  const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt,
			  cw_uint32_t a_count)
{
    cw_buf_t *buf;
    cw_bufp_t *bufp;

    bufp = a_mkr->bufp;
    buf = bufp->buf;

    /* The data won't fit in this bufp, but enough data can be slid to the next
     * bufp to make room.  The data inserted may be split across the two bufp's
     * as well.
     *
     **************************************************************************
     *           I
     * [X   ][YYYY]
     *           ^
     *
     * [XYYY][YI  ]
     *
     **************************************************************************
     *         IIIII
     * [X   ][YY  ]
     *         ^
     *
     * [XYII][IIIY]
     *
     **************************************************************************
     *         II
     * [X   ][YYY ]
     *         ^
     * [XYII][   Y]
     *
     **************************************************************************/

}

static void
mkr_p_slide_after_insert(cw_mkr_t *a_mkr, cw_bufp_t *a_nextp,
			 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt,
			 cw_uint32_t a_count)
{
/*      cw_bufp_t *bufp; */

/*      bufp = a_mkr->bufp; */
/*      buf = bufp->buf; */

}

/* The data won't fit in this bufp, but enough data can be slid to the next
     * bufp to make room.  The data to be inserted may be split across the two
     * bufp's as well.  This case must be handled specially (as opposed to
     * simply splitting the bufp), because otherwise very sparse bufp's (as bad
     * as one character per bufp) could result.  Since coalescing is only done
     * during deletion, such fragmentation would potentially exist forever
     * after.  The sliding code assures that for each insertion operation, new
     * bufp's are on average at least 50% full, yet the algorithmic cost of
     * sliding is strongly bounded (O(1)).
     *
     * Buffer operations tend toward high locality, so on the assumption that
     * future operations will tend to be at a_mkr, leave as much gap space as
     * possible in the bufp that contains a_mkr.  Doing so decreases the
     * likelihood of having to do another slide operation soon after this one.
     *
     * In the following diagrams, bufp's are delimited by [], existing data are
     * Y and Z, data being inserted are I, and ^ points to the insertion point.
     *
     **************************************************************************
     *
     *     IIIIIIII
     * [YYYY    ][ZZZZ    ]
     *     ^
     *
     * [YYYIIIII][IIIYZZZZ]
     *
     **************************************************************************
     *
     *      IIII
     * [YYYYYY  ][ZZZZ    ]
     *      ^
     *
     * [YYYYIIII][YY  ZZZZ]
     *
     **************************************************************************
     *
     *        II
     * [YYYYYYYY][ZZZZ    ]
     *        ^
     *
     * [YYYYYYII][YY   ZZZZ]
     *
     **************************************************************************
     *
     *         II
     * [YYYYYYYY][ZZZZ    ]
     *         ^
     *
     * [YYYYYYYI][I   ZZZZ]
     *
     **************************************************************************
     *
     *      II
     * [YYYYYYYY][ZZZ     ]
     *      ^
     *
     * [YYYYII  ][YYYY ZZZ]
     *
     **************************************************************************/

/* a_bufv won't fit in the a_mkr's bufp, so split it. */
/* XXX Doesn't move a_mkr if a_after is FALSE. */
static void
mkr_p_split_insert(cw_mkr_t *a_mkr, cw_bool_t a_after, const cw_bufv_t *a_bufv,
		   cw_uint32_t a_bufvcnt, cw_uint32_t a_count)
{
    cw_uint32_t i, nextra, cnt, v;
    cw_buf_t *buf;
    cw_bufp_t *bufp, *nextp, *pastp;
    cw_mkr_t *mkr, *mmkr;
    cw_bufv_t bufv_to, bufv_from, vsplit, vremain;

    bufp = a_mkr->bufp;
    buf = bufp->buf;
    /* Keep track of the bufp past the range of bufp's being operated on.  This
     * might be NULL, so can only be used as an interation terminator. */
    pastp = ql_next(&buf->plist, bufp, plink);

    /* Move bufp's gap to the split point. */
    bufp_p_gap_move(bufp, a_mkr->ppos);

    /* Create nextp and insert it just after bufp. */
    nextp = bufp_p_new(buf);
    nextp->bob_relative = TRUE;
    nextp->bpos = bufp->bpos + bufp->len;
    nextp->line = bufp->line + bufp->nlines;
    buf_p_bufp_insert(buf, nextp);
    cw_assert(ql_next(&buf->plist, bufp, plink) == nextp);

    /* Insert the data after bufp's gap to the same offset in nextp. */
    bufv_to.data = &nextp->b[bufp->gap_off + (CW_BUFP_SIZE - bufp->len)];
    bufv_to.len = bufp->len - bufp->gap_off;
    bufv_from.data = &bufp->b[bufp->gap_off + (CW_BUFP_SIZE - bufp->len)];
    bufv_from.len = CW_BUFP_SIZE - (bufp->gap_off
				    + (CW_BUFP_SIZE - bufp->len));
    nextp->nlines = bufv_p_copy(&bufv_to, 1, &bufv_from, 1);

    /* Subtract nextp's nlines from bufp's nlines. */
    bufp->nlines -= nextp->nlines;

    /* Starting at the end of bufp's marker list, remove the markers and insert
     * them into nextp until all markers that are in the gap have been moved. */
    for (mkr = ql_last(&bufp->mlist, mlink);
	 mkr->ppos >= bufp->gap_off;)
    {
	/* Get the previous mkr before removing mkr from the list. */
	mmkr = mkr;
	mkr = ql_prev(&bufp->mlist, mkr, mlink);

	mkr_p_remove(mmkr);
	mmkr->bufp = nextp;
	mkr_p_insert(mmkr);
    }

    /* Adjust bufp's len. */
    bufp->len = bufp->gap_off;

    /* Adjust bufp_cur. */
    buf->bufp_cur = nextp;

    /* Check if splitting bufp provided enough space.  If not, calculate how
     * many more bufp's are needed, then insert them. */
    if (a_count > (cw_uint64_t) ((CW_BUFP_SIZE - bufp->len)
				 + (CW_BUFP_SIZE - nextp->len)))
    {
	cw_bufp_t *newp;

	/* Splitting bufp didn't provide enough space.  Calculate how many more
	 * bufp's are needed. */
	nextra = (a_count - (cw_uint64_t) ((CW_BUFP_SIZE - bufp->len)
					   + (CW_BUFP_SIZE - nextp->len)))
	    / (cw_uint64_t) CW_BUFP_SIZE;

	if ((a_count - (cw_uint64_t) ((CW_BUFP_SIZE - bufp->len)
				      + (CW_BUFP_SIZE - nextp->len)))
	    % (cw_uint64_t) CW_BUFP_SIZE != 0)
	{
	    nextra++;
	}

	/* Insert extra bufp's. */
	for (i = 0; i < nextra; i++)
	{
	    newp = bufp_p_new(buf);
	    newp->bob_relative = TRUE;
	    newp->bpos = nextp->bpos;
	    newp->line = nextp->line;

	    buf_p_bufp_insert(buf, bufp);
	}
    }
    else
    {
	nextra = 0;
    }

    /* Resize bufv to make enough room for all of the bufp's that were just
     * inserted (including the extra one from splitting). */
    buf->bufv = (cw_bufv_t *) cw_opaque_realloc(buf->realloc, buf->bufv,
						buf->arg,
						buf->bufvcnt
						* sizeof(cw_bufv_t),
						(buf->bufvcnt
						 + (nextra + 1) * 2)
						* sizeof(cw_bufv_t));
    buf->bufvcnt += (nextra + 1) * 2;

    /* Iteratively call mkr_p_simple_insert(), taking care never to insert more
     * data than will fit in the bufp being inserted into.  The approach taken
     * here is to use the elements of a_bufv directly, exept when an element
     * would overflow the space available in the bufp being inserted into.  In
     * that case, the offending element is iteratively broken into pieces and
     * inserted into bufp's, until it has been completely inserted.  In the
     * worst case, this means three calls to mkr_p_simple_insert() per bufp.
     *
     * The following diagrams show examples of various cases of how a_bufv can
     * be split up.  For the purposes of these diagrams, all characters,
     * including the brackets, represent bytes.  Thus [0 ] is a 4 byte array
     * (the 0 denotes the array element offset).
     *
     * bufv [0         ][1 ][2 ][3 ][4  ][5    ][6][7][8][][][11   ]
     * bufp [0 ][1     ][2     ][3     ][4     ][5     ][6     ][7 ]
     *
     * bufp   | # ins | Explanation
     * -------+-------+-----------------------------------------------------
     * [0..1] |     1 | bufv[0] is split, but it is the only thing being
     *        |       | inserted into bufp[0..1].  Note that a bufv element
     *        |       | could span more than two bufp's.
     * -------+-------+-----------------------------------------------------
     *    [2] |     1 | bufv[1..2] fit exactly into bufp[2].
     * -------+-------+-----------------------------------------------------
     *    [3] |     2 | bufv[3] is inserted directly, but bufv[4] is split.
     * -------+-------+-----------------------------------------------------
     *    [4] |     2 | bufv[4] is split, and bufv[5] is inserted directly.
     * -------+-------+-----------------------------------------------------
     *    [5] |     2 | Same case as for bufp[3].
     * -------+-------+-----------------------------------------------------
     *    [6] |     3 | bufv[8] is split, bufv[9..10] are inserted directly,
     *        |       | and bufv[11] is split.
     * -------+-------+-----------------------------------------------------
     *    [7] |     1 | Same case as for bufp[1].
     * -------+-------+-----------------------------------------------------
     *
     * Iterate over bufp's being inserted into.  bufp starts out pointing to the
     * original bufp on which insertion was attempted before the split. */
    v = 0;
    vremain.len = 0;
    for (; bufp != pastp; bufp = ql_next(&buf->plist, bufp, plink))
    {
	/* Insert remainder, if any. */
	if (vremain.len != 0)
	{
	    vsplit.data = vremain.data;
	    if (CW_BUFP_SIZE - bufp->len < vremain.len)
	    {
		vsplit.len = CW_BUFP_SIZE - bufp->len;
	    }
	    else
	    {
		vsplit.len = vremain.len;
	    }
	    vremain.data = &vremain.data[vsplit.len];
	    vremain.len -= vsplit.len;

	    mkr_p_raw_insert(bufp, &vsplit, 1, vsplit.len);

	    if (CW_BUFP_SIZE - bufp->len == 0)
	    {
		/* No more space in this bufp. */
		continue;
	    }
	}

	/* Iterate over a_bufv elements and insert their contents into the
	 * current bufp. */
	cnt = 0;
	for (i = v; i < a_bufvcnt; i++)
	{
	    if (cnt + a_bufv[i].len > CW_BUFP_SIZE - bufp->len)
	    {
		break;
	    }
	    cnt += a_bufv[i].len;
	}
	if (cnt != 0)
	{
	    mkr_p_raw_insert(bufp, &a_bufv[v], i - v, cnt);
	    v = i;
	}

	/* Split a_bufv[v] if bufp isn't full and there are more data. */
	if (CW_BUFP_SIZE - bufp->len != 0 && v < a_bufvcnt)
	{
	    vsplit.data = a_bufv[v].data;
	    vsplit.len = CW_BUFP_SIZE - bufp->len;
	    vremain.data = &vsplit.data[vsplit.len];
	    vremain.len = a_bufv[v].len - vsplit.len;

	    mkr_p_raw_insert(bufp, &vsplit, 1, vsplit.len);
	}
    }
}

/* XXX What if inserting before first position in a_mkr->bufp? */
void
mkr_l_insert(cw_mkr_t *a_mkr, cw_bool_t a_record, cw_bool_t a_after,
	     const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_uint64_t cnt;
    cw_uint32_t i;
    cw_buf_t *buf;
    cw_bufp_t *bufp;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    bufp = a_mkr->bufp;
    buf = bufp->buf;

    /* Record the undo information before inserting so that the apos is still
     * unmodified. */
    if (buf->hist != NULL && a_record)
    {
	if (a_after)
	{
	    hist_ynk(buf->hist, buf, bufp_p_pos_p2b(a_mkr->bufp, a_mkr->ppos),
		     a_bufv, a_bufvcnt);
	}
	else
	{
	    hist_ins(buf->hist, buf, bufp_p_pos_p2b(a_mkr->bufp, a_mkr->ppos),
		     a_bufv, a_bufvcnt);
	}
    }

    /* Determine the total number of characters to be inserted. */
    cnt = 0;
    for (i = 0; i < a_bufvcnt; i++)
    {
	cnt += a_bufv[i].len;
    }

    /* Depending on how much data are to be inserted, there are three
     * different algorithms: simple, slide, and split. */
    if (cnt <= CW_BUFP_SIZE - bufp->len)
    {
	mkr_p_simple_insert(a_mkr, a_after, a_bufv, a_bufvcnt, cnt);
    }
    else
    {
	cw_bufp_t *prevp, *nextp;

	prevp = ql_next(&buf->plist, bufp, plink);
	nextp = ql_next(&buf->plist, bufp, plink);

	/* Try sliding backward, then forward, then both directions.  If none of
	 * the slides would make enough room, splitting is guaranteed not to
	 * violate the requirement that any two consecutive bufps must both be
	 * at least 25% full.  Thus, there is never a need to do bufp coalescing
	 * after insertion.  In addition, by not trying to slide both directions
	 * unless necessary, there is never the risk of leaving the center bufp
	 * empty after sliding and insertion. */
	if (prevp != NULL && cnt <= (CW_BUFP_SIZE - bufp->len)
	    + (CW_BUFP_SIZE - prevp->len))
	{
	    mkr_p_slide_before_insert(a_mkr, prevp, a_bufv, a_bufvcnt, cnt);
	}
	else if (nextp != NULL && cnt <= (CW_BUFP_SIZE - bufp->len)
	    + (CW_BUFP_SIZE - nextp->len))
	{
	    mkr_p_slide_after_insert(a_mkr, nextp, a_bufv, a_bufvcnt, cnt);
	}
	else if (prevp != NULL && nextp != NULL
		 && cnt <= (CW_BUFP_SIZE - bufp->len)
		 + (CW_BUFP_SIZE - prevp->len)
		 + (CW_BUFP_SIZE - nextp->len))
	{
/* 	    mkr_p_slide_both_insert(a_mkr, prevp, nextp, a_bufv, a_bufvcnt, */
/* 				    cnt); */
	}
	else
	{
	    mkr_p_split_insert(a_mkr, a_after, a_bufv, a_bufvcnt, cnt);
	}
    }
}

void
mkr_l_remove(cw_mkr_t *a_start, cw_mkr_t *a_end, cw_bool_t a_record)
{
    cw_error("XXX Not implemented");
}

void
mkr_new(cw_mkr_t *a_mkr, cw_buf_t *a_buf)
{
    cw_bufp_t *bufp;

    cw_check_ptr(a_mkr);
    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    bufp = ql_first(&a_buf->plist);
    a_mkr->bufp = bufp;
    a_mkr->ppos = bufp_p_pos_b2p(bufp, 1);
    a_mkr->pline = 0;
    rb_node_new(&bufp->mtree, a_mkr, mnode);
    ql_elm_new(a_mkr, mlink);

    mkr_p_insert(a_mkr);

#ifdef CW_DBG
    a_mkr->magic = CW_MKR_MAGIC;
#endif
}

void
mkr_dup(cw_mkr_t *a_to, const cw_mkr_t *a_from)
{
    cw_check_ptr(a_to);
    cw_dassert(a_to->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_from);
    cw_dassert(a_from->magic == CW_MKR_MAGIC);
    cw_assert(a_to->bufp->buf == a_from->bufp->buf);

    mkr_p_remove(a_to);

    a_to->bufp = a_from->bufp;
    a_to->ppos = a_from->ppos;
    a_to->pline = a_from->pline;

    mkr_p_insert(a_to);
}

void
mkr_delete(cw_mkr_t *a_mkr)
{
    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    mkr_p_remove(a_mkr);

#ifdef CW_DBG
    memset(a_mkr, 0x5a, sizeof(cw_mkr_t));
#endif
}

cw_buf_t *
mkr_buf(const cw_mkr_t *a_mkr)
{
    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    return a_mkr->bufp->buf;
}

cw_uint64_t
mkr_line_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
    cw_uint64_t bpos;
    cw_buf_t *buf;
    cw_bufp_t *bufp = NULL;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    buf = a_mkr->bufp->buf;

    switch (a_whence)
    {
	case BUFW_BOB:
	{
	    if (a_offset <= 0)
	    {
		/* Attempt to move to or before BOB.  Move to BOB. */
		bpos = 1;
	    }
	    else if (a_offset >= buf->nlines)
	    {
		/* Attempt to move to or past EOB.  Move to EOB. */
		bpos = buf->len + 1;
	    }
	    else
	    {
		/* Move forward from BOB to just short of a_offset '\n'
		 * characters.  For example, if seeking forward 2:
		 *
		 *  \/
		 *   hello\ngoodbye\nyadda\nblah
		 *                /\
		 */
		bpos = buf_p_bpos_before_lf(buf, a_offset, &bufp);
	    }
	    break;
	}
	case BUFW_REL:
	{
	    if (a_offset > 0)
	    {
		if (mkr_p_line(a_mkr) + a_offset > buf->nlines)
		{
		    /* Attempt to move to or after EOB.  Move to EOB. */
		    bpos = buf->len + 1;
		}
		else
		{
		    /* Move forward from the current position to just short of
		     * a_offset '\n' characters.  For example, if seeking
		     * forward 2:
		     *
		     *            \/
		     *   hello\ngoodbye\nyadda\nblah
		     *                       /\
		     */
		    bpos = buf_p_bpos_before_lf(buf,
						mkr_p_line(a_mkr) - 1
						+ a_offset,
						&bufp);
		}
	    }
	    else if (a_offset < 0)
	    {
		if (-a_offset >= mkr_p_line(a_mkr))
		{
		    /* Attempt to move to or before BOB.  Move to BOB. */
		    bpos = 1;
		}
		else
		{
		    /* Move backward from the current position to just short
		     * of a_offset '\n' characters.  For example, if
		     * seeking backward 2:
		     *
		     *                     \/
		     *   hello\ngoodbye\nyadda\nblah
		     *         /\
		     */
		    bpos = buf_p_bpos_after_lf(buf,
					       mkr_p_line(a_mkr) - a_offset + 1,
					       &bufp);
		}
	    }
	    else
	    {
		/* Do nothing. */
		bpos = mkr_p_bpos(a_mkr);
		goto RETURN;
	    }
	    break;
	}
	case BUFW_EOB:
	{
	    if (a_offset >= 0)
	    {
		/* Attempt to move to or after EOB.  Move to EOB. */
		bpos = buf->len + 1;
	    }
	    else if (-a_offset >= buf->nlines)
	    {
		/* Attempt to move to or past BOB.  Move to BOB. */
		bpos = 1;
	    }
	    else
	    {
		/* Move backward from EOB to just short of a_offset '\n'
		 * characters.  For example if seeking backward 2:
		 *
		 *                             \/
		 *   hello\ngoodbye\nyadda\nblah
		 *                  /\
		 */
		bpos = buf_p_bpos_after_lf(buf, buf->nlines + a_offset + 1,
					   &bufp);
	    }
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    /* If not already known, get the bufp that contains bpos. */
    if (bufp == NULL)
    {
	bufp = buf_p_bufp_at_bpos(buf, bpos);
    }

    /* Remove a_mkr from the tree and list of the bufp at its old location. */
    mkr_p_remove(a_mkr);

    /* Update the internal state of a_mkr. */
    a_mkr->bufp = bufp;
    a_mkr->ppos = bpos - bufp_p_bpos(bufp);
    a_mkr->pline = 1 + a_offset - bufp_p_line(bufp);

    /* Insert a_mkr into the bufp's tree and list. */
    mkr_p_insert(a_mkr);

    RETURN:
    return bpos;
}

cw_uint64_t
mkr_line(cw_mkr_t *a_mkr)
{
    cw_uint64_t retval;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    retval = mkr_p_line(a_mkr);

    return retval;
}

cw_uint64_t
mkr_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
    cw_uint64_t bpos;
    cw_buf_t *buf;
    cw_bufp_t *bufp;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    buf = a_mkr->bufp->buf;

    /* Determine the bpos to seek to.  Take care not to go out of bounds. */
    switch (a_whence)
    {
	case BUFW_BOB:
	{
	    if (a_offset < 0)
	    {
		/* Attempt to move to or before BOB.  Move to BOB. */
		bpos = 1;
	    }
	    else if (a_offset > buf->len)
	    {
		/* Attempt to move to or past EOB.  Move to EOB. */
		bpos = buf->len + 1;
	    }
	    else
	    {
		bpos = a_offset + 1;
	    }
	    break;
	}
	case BUFW_REL:
	{
	    bpos = mkr_p_bpos(a_mkr);
	    if (a_offset > 0)
	    {
		if (bpos + a_offset > buf->len + 1)
		{
		    /* Attempt to move to or after EOB.  Move to EOB. */
		    bpos = buf->len + 1;
		}
		else
		{
		    bpos += a_offset;
		}
	    }
	    else if (a_offset < 0)
	    {
		if (-a_offset >= bpos)
		{
		    /* Attempt to move to or before BOB.  Move to BOB. */
		    bpos = 1;
		}
		else
		{
		    bpos += a_offset;
		}
	    }
	    else
	    {
		/* Do nothing. */
		goto RETURN;
	    }
	    break;
	}
	case BUFW_EOB:
	{
	    if (a_offset > 0)
	    {
		/* Attempt to move to or after EOB.  Move to EOB. */
		bpos = buf->len + 1;
	    }
	    else if (-a_offset >= buf->len)
	    {
		/* Attempt to move to or past BOB.  Move to BOB. */
		bpos = 1;
	    }
	    else
	    {
		bpos = buf->len + 1 + a_offset;
	    }
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    /* Get the bufp that contains bpos. */
    bufp = buf_p_bufp_at_bpos(buf, bpos);

    /* Remove a_mkr from the tree and list of the bufp at its old location. */
    mkr_p_remove(a_mkr);

    /* Update the internal state of a_mkr. */
    a_mkr->bufp = bufp;
    a_mkr->ppos = bpos - bufp_p_bpos(bufp);
    a_mkr->pline = 1 + a_offset - bufp_p_line(bufp);

    /* Insert a_mkr into the bufp's tree and list. */
    mkr_p_insert(a_mkr);

    RETURN:
    return bpos;
}

cw_uint64_t
mkr_pos(const cw_mkr_t *a_mkr)
{
    cw_uint64_t retval;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    retval = bufp_p_pos_p2b(a_mkr->bufp, a_mkr->ppos);

    return retval;
}

cw_uint8_t *
mkr_before_get(const cw_mkr_t *a_mkr)
{
    cw_uint8_t *retval;
    cw_uint32_t offset;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    /* Determine offset of a_mkr. */
    if (a_mkr->ppos <= a_mkr->bufp->gap_off)
    {
	/* Before gap. */
	offset = a_mkr->ppos;
    }
    else
    {
	/* After gap. */
	offset = a_mkr->ppos - (CW_BUFP_SIZE - a_mkr->bufp->len);
    }

    if (offset == 0)
    {
	cw_bufp_t *bufp;

	if ((bufp = ql_prev(&a_mkr->bufp->buf->plist, a_mkr->bufp, plink))
	    == NULL)
	{
	    /* There is no character before BOB. */
	    retval = NULL;
	}
	else
	{
	    /* The character is in the previous bufp. */
	    if (bufp->len > bufp->gap_off)
	    {
		/* Last character in raw buffer. */
		retval = &bufp->b[CW_BUFP_SIZE - 1];
	    }
	    else
	    {
		/* Last character in raw buffer before gap. */
		retval = &bufp->b[bufp->len - 1];
	    }
	}
    }
    else
    {
	/* Determine offset of character before a_mkr. */
	if (a_mkr->ppos <= a_mkr->bufp->gap_off + (CW_BUFP_SIZE
						   + a_mkr->bufp->len))
	{
	    /* Before gap. */
	    offset = a_mkr->ppos - 1;
	}
	else
	{
	    /* After gap. */
	    offset = a_mkr->ppos - 1 - (CW_BUFP_SIZE - a_mkr->bufp->len);
	}
	retval = &a_mkr->bufp->b[offset];
    }

    return retval;
}

cw_uint8_t *
mkr_after_get(const cw_mkr_t *a_mkr)
{
    cw_uint32_t offset;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    if (a_mkr->ppos <= a_mkr->bufp->gap_off)
    {
	offset = a_mkr->ppos;
    }
    else
    {
	offset = a_mkr->ppos - (CW_BUFP_SIZE - a_mkr->bufp->len);
    }
    
    return &a_mkr->bufp->b[offset];
}

cw_bufv_t *
mkr_range_get(const cw_mkr_t *a_start, const cw_mkr_t *a_end,
	      cw_uint32_t *r_bufvcnt)
{
    cw_bufv_t *retval;
    const cw_mkr_t *start, *end;
    cw_uint32_t alg;

    cw_check_ptr(a_start);
    cw_dassert(a_start->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_end);
    cw_dassert(a_end->magic == CW_MKR_MAGIC);
    cw_check_ptr(r_bufvcnt);
    cw_assert(a_start->bufp->buf == a_end->bufp->buf);

    /* Set start/end from a_start/a_end so that start is before end.  At the
     * same time, determine what algorithm will be used to construct the bufv.
     * There are three basic cases that must be handled:
     *
     * 1) start and end point to the same bpos, and therefore the range is
     *    zero-length.  retval = NULL and *r_bufvcnt = 0;
     *
     * 2) start and end are in the same bufp, on the same/opposite sides of the
     *    gap.
     *
     * 3) start and end are in different bufp's.  This has sub-cases:
     *
     *    a) start is before/after the gap.
     *
     *    b) There are zero or more bufp's in between start's bufp and end's
     *       bufp.  For each of those bufp's:
     *
     *       i) The gap splits the bufp into two regions.
     *
     *       ii) The gap is at the beginnning/end of the bufp, leaving the data
     *           contiguous.
     *
     *    c) end is before/after the gap.
     */
    if (a_start->bufp == a_end->bufp)
    {
	if (a_start->ppos == a_end->ppos)
	{
	    alg = 1;
	}
	else
	{
	    if (a_start->ppos < a_end->ppos)
	    {
		start = a_start;
		end = a_end;
	    }
	    else
	    {
		start = a_end;
		end = a_start;
	    }
	    alg = 2;
	}
    }
    else
    {
	if (bufp_p_bpos(a_start->bufp) < bufp_p_bpos(a_end->bufp))
	{
	    start = a_start;
	    end = a_end;
	}
	else
	{
	    start = a_end;
	    end = a_start;
	}
	alg = 3;
    }

    switch (alg)
    {
	case 1:
	{
	    /* Zero-length range. */
	    retval = NULL;
	    *r_bufvcnt = 0;
	    break;
	}
	case 2:
	{
	    cw_bufp_t *bufp = start->bufp;

	    /* One bufp involved. */
	    retval = bufp->buf->bufv;

	    if (start->ppos < bufp->gap_off
		&& end->ppos >= bufp->gap_off + (CW_BUFP_SIZE - bufp->len))
	    {
		/* Two ranges. */
		retval[0].data = &bufp->b[start->ppos];
		retval[0].len = bufp->gap_off - start->ppos;

		retval[1].data = &bufp->b[bufp->gap_off
					  + (CW_BUFP_SIZE - bufp->len)];
		retval[1].len = end->ppos - (bufp->gap_off
					     + (CW_BUFP_SIZE - bufp->len));

		*r_bufvcnt = 2;
	    }
	    else
	    {
		/* One range. */
		retval[0].data = &bufp->b[start->ppos];
		retval[0].len = end->ppos - start->ppos;

		*r_bufvcnt = 1;
	    }
	    break;
	}
	case 3:
	{
	    cw_buf_t *buf;
	    cw_bufp_t *bufp;
	    cw_uint32_t bufvcnt;

	    /* Two or more bufp's involved. */
	    buf = start->bufp->buf;
	    retval = buf->bufv;

	    /* First bufp. */
	    bufp = start->bufp;
	    retval[0].data = &bufp->b[start->ppos];
	    if (start->ppos < start->bufp->gap_off && bufp->gap_off < bufp->len)
	    {
		/* Two ranges. */
		retval[0].len = bufp->gap_off - start->ppos;

		retval[1].data = &bufp->b[bufp->gap_off
					  + (CW_BUFP_SIZE - bufp->len)];
		retval[1].len = CW_BUFP_SIZE - (bufp->gap_off
						+ (CW_BUFP_SIZE - bufp->len));

		bufvcnt = 2;
	    }
	    else
	    {
		/* One range. */
		if (start->ppos < bufp->gap_off)
		{
		    /* Before gap. */
		    retval[0].len = bufp->len - start->ppos;
		}
		else
		{
		    /* After gap. */
		    retval[0].len = CW_BUFP_SIZE - start->ppos;
		}

		bufvcnt = 1;
	    }

	    /* Intermediate bufp's. */
	    for (bufp = ql_next(&buf->plist, bufp, plink);
		 bufp != end->bufp;
		 bufp = ql_next(&buf->plist, bufp, plink))
	    {
		cw_check_ptr(bufp);

		if (bufp->gap_off == 0)
		{
		    /* Gap at beginning. */
		    retval[bufvcnt].data = &bufp->b[bufp->gap_off];
		    retval[bufvcnt].len = bufp->len;

		    bufvcnt++;
		}
		else if (bufp->gap_off == bufp->len)
		{
		    /* Gap at end. */
		    retval[bufvcnt].data = &bufp->b[0];
		    retval[bufvcnt].len = bufp->len;

		    bufvcnt++;
		}
		else
		{
		    /* Gap splits bufp into two regions. */
		    retval[bufvcnt].data = &bufp->b[0];
		    retval[bufvcnt].len = bufp->gap_off;

		    retval[bufvcnt + 1].data = &bufp->b[bufp->gap_off
							+ (CW_BUFP_SIZE
							   - bufp->len)];
		    retval[bufvcnt + 1].len = CW_BUFP_SIZE - (bufp->gap_off
							      + (CW_BUFP_SIZE
								 - bufp->len));

		    bufvcnt += 2;
		}
	    }

	    /* Last bufp. */
	    retval[bufvcnt].data = &bufp->b[end->ppos];
	    if (end->ppos < end->bufp->gap_off
		&& bufp->gap_off < bufp->len)
	    {
		/* Two ranges. */
		retval[bufvcnt].len = bufp->gap_off - end->ppos;

		retval[bufvcnt + 1].data = &bufp->b[bufp->gap_off
						    + (CW_BUFP_SIZE
						       - bufp->len)];
		retval[bufvcnt + 1].len = CW_BUFP_SIZE - (bufp->gap_off
							  + (CW_BUFP_SIZE
							     - bufp->len));

		bufvcnt += 2;
	    }
	    else
	    {
		/* One range. */
		if (end->ppos < bufp->gap_off)
		{
		    /* Before gap. */
		    retval[bufvcnt].len = bufp->len - end->ppos;
		}
		else
		{
		    /* After gap. */
		    retval[bufvcnt].len = CW_BUFP_SIZE - end->ppos;
		}

		bufvcnt++;
	    }

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return retval;
}

void
mkr_before_insert(cw_mkr_t *a_mkr, const cw_bufv_t *a_bufv,
		   cw_uint32_t a_bufvcnt)
{
    mkr_l_insert(a_mkr, TRUE, FALSE, a_bufv, a_bufvcnt);
}

void
mkr_after_insert(cw_mkr_t *a_mkr, const cw_bufv_t *a_bufv,
		  cw_uint32_t a_bufvcnt)
{
    mkr_l_insert(a_mkr, TRUE, TRUE, a_bufv, a_bufvcnt);
}

void
mkr_remove(cw_mkr_t *a_start, cw_mkr_t *a_end)
{
    mkr_l_remove(a_start, a_end, TRUE);
}

/* ext. */
#ifdef NOT_YET
static cw_sint32_t
ext_p_fcomp(cw_ext_t *a_a, cw_ext_t *a_b)
{
    cw_error("XXX Not implemented");
}

static cw_sint32_t
ext_p_rcomp(cw_ext_t *a_a, cw_ext_t *a_b)
{
    cw_error("XXX Not implemented");
}
#endif

cw_ext_t *
ext_new(cw_ext_t *a_ext, cw_buf_t *a_buf)
{
    cw_error("XXX Not implemented");
}

void
ext_dup(cw_ext_t *a_to, cw_ext_t *a_from)
{
    cw_error("XXX Not implemented");
}

void
ext_delete(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

cw_buf_t *
ext_buf(const cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

const cw_mkr_t *
ext_beg_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_beg_set(cw_ext_t *a_ext, const cw_mkr_t *a_beg)
{
    cw_error("XXX Not implemented");
}

const cw_mkr_t *
ext_end_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_end_set(cw_ext_t *a_ext, const cw_mkr_t *a_end)
{
    cw_error("XXX Not implemented");
}

cw_bool_t
ext_beg_open_get(const cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_beg_open_set(cw_ext_t *a_ext, cw_bool_t a_beg_open)
{
    cw_error("XXX Not implemented");
}

cw_bool_t
ext_end_open_get(const cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_end_open_set(cw_ext_t *a_ext, cw_bool_t a_end_open)
{
    cw_error("XXX Not implemented");
}

cw_bool_t
ext_detachable_get(const cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_detachable_set(cw_ext_t *a_ext, cw_bool_t a_detachable)
{
    cw_error("XXX Not implemented");
}

cw_bool_t
ext_detached_get(const cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_detached_set(cw_ext_t *a_ext, cw_bool_t a_detached)
{
    cw_error("XXX Not implemented");
}

void
ext_detach(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

/* Get the first and last ext's that overlap a_mkr.  retval is the length of the
 * run.  r_beg and r_end are NULL if there are no extents overlapping the
 * run. */
cw_uint64_t
ext_run_get(const cw_mkr_t *a_mkr, cw_ext_t *r_beg, cw_ext_t *r_end)
{
    cw_error("XXX Not implemented");
}

/* Iterate in f-order. */
cw_ext_t *
ext_prev_get(const cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

cw_ext_t *
ext_next_get(const cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}
