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
 * Each bufp caches its bpos and line to speed up many operations.  buf
 * modifications can invalidate the values cached in bufp's, so pointers to the
 * end points of the ranges with valid caches are maintained.  At any given
 * time, the ranges of bufp's with valid caches may look something like:
 *
 *    0           1           2           3           4           5
 * /------\    /------\    /------\    /------\    /------\    /------\
 * | bufp |<-->| bufp |<-->| bufp |<-->| bufp |<-->| bufp |<-->| bufp |
 * \------/    \------/    \------/    \------/    \------/    \------/
 *                ^                       ^
 *    |-----------|                       |-----------------------|
 *            bob_cached              eob_cached
 *
 * In this example, bufp 2 is the only one without a valid cache, relative to
 * either the begin or end of the buffer.  Note that it is possible (though
 * somewhat atypical) for the following to happen:
 *
 *    0           1           2           3           4           5
 * /------\    /------\    /------\    /------\    /------\    /------\
 * | bufp |<-->| bufp |<-->| bufp |<-->| bufp |<-->| bufp |<-->| bufp |
 * \------/    \------/    \------/    \------/    \------/    \------/
 *                            ^                       ^
 *                            |                       |
 *    |-----------------------+-----------------------|
 *                            |                   bob_cached
 *                            |-----------------------------------|
 *                        eob_cached
 *
 * In this case, bufp's 2, 3, and 4 have valid caches relative to both the begin
 * and end of the buffer.
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
 ******************************************************************************/

#include "../include/modslate.h"

/* Prototypes. */
/* XXX */

/* A simplified version of bufv_copy() that counts '\n' characters that are
 * copied, and returns that rather than the number of elements copied. */
CW_INLINE cw_uint64_t
bufv_p_copy(cw_bufv_t *a_to, cw_uint32_t a_to_len, const cw_bufv_t *a_fr,
	    cw_uint32_t a_fr_len)
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
static cw_bufp_t *
bufp_p_new(cw_buf_t *a_buf)
{
    cw_bufp_t *retval;

    /* Allocate. */
    retval = (cw_bufp_t *) cw_opaque_alloc(a_buf->alloc, a_buf->arg,
					   sizeof(cw_bufp_t));

    /* Don't bother initializing index, bpos, or line, since they are explicitly
     * set later. */

    /* Initialize length and line count. */
    retval->len = 0;
    retval->nlines = 0;

    /* Initialize buffer gap. */
    retval->gap_off = 0;
    retval->gap_len = CW_BUFP_SIZE;

    /* Allocate buffer. */
    retval->b = (cw_uint8_t *) cw_opaque_alloc(a_buf->alloc, a_buf->arg,
					       CW_BUFP_SIZE);

    /* Initialize marker tree and list. */
    rb_tree_new(&retval->mtree, node);
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
	rb_first(&a_bufp->mtree, node, first);
	cw_assert(first == NULL);
    }
#endif
    cw_assert(ql_first(&a_bufp->mlist) == NULL);

    cw_opaque_dealloc(a_bufp->buf->dealloc, a_bufp->buf->arg, a_bufp->b,
		      CW_BUFP_SIZE);

    cw_opaque_dealloc(a_bufp->buf->dealloc, a_bufp->buf->arg, a_bufp,
		      sizeof(cw_bufp_t));
}

CW_INLINE cw_uint64_t
bufp_p_index_get(cw_bufp_t *a_bufp)
{
    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    return a_bufp->index;
}

CW_INLINE void
bufp_p_index_set(cw_bufp_t *a_bufp, cw_uint64_t a_index)
{
    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    a_bufp->index = a_index;
}

CW_INLINE cw_uint64_t
bufp_p_bpos_get(cw_bufp_t *a_bufp)
{
    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    return a_bufp->bpos;
}

CW_INLINE void
bufp_p_bpos_set(cw_bufp_t *a_bufp, cw_uint64_t a_bpos)
{
    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    a_bufp->bpos = a_bpos;
}

CW_INLINE cw_uint64_t
bufp_p_line_get(cw_bufp_t *a_bufp)
{
    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    return a_bufp->line;
}

CW_INLINE void
bufp_p_line_set(cw_bufp_t *a_bufp, cw_uint64_t a_line)
{
    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    a_bufp->line = a_line;
}

static void
bufp_p_cache_validate(cw_bufp_t *a_bufp)
{
    cw_buf_t *buf;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    /* If the cached values for bpos and line are not valid, update them in the
     * quickest way possible.  This can be achieved by working forward or
     * backward in the array of bufp's, so start at the closest bufp with a
     * valid cache and work toward this bufp. */
    buf = a_bufp->buf;
    if (buf->bob_cached < a_bufp->index && buf->eob_cached > a_bufp->index)
    {
	cw_uint64_t bpos, line;

	/* Invalid cache. */
	if (a_bufp->index - buf->bob_cached <= buf->eob_cached - a_bufp->index)
	{
	    /* Work forward. */
	    bpos = buf->bufps[buf->bob_cached]->bpos;
	    line = buf->bufps[buf->bob_cached]->line;
	    while (buf->bob_cached < a_bufp->index)
	    {
		buf->bob_cached++;

		/* Update bpos. */
		buf->bufps[buf->bob_cached]->bpos = bpos;
		bpos += ((cw_uint64_t) CW_BUFP_SIZE
			 - (cw_uint64_t) buf->bufps[buf->bob_cached]->gap_len);

		/* Update line. */
		buf->bufps[buf->bob_cached]->line = line;
		line += ((cw_uint64_t) buf->bufps[buf->bob_cached]->nlines);
	    }
	}
	else
	{
	    /* Work backward. */

	    /* Take care to avoid looking past the end of the bufp array. */
	    if (buf->eob_cached < buf->nbufps)
	    {
		bpos = buf->bufps[buf->eob_cached]->bpos;
		line = buf->bufps[buf->eob_cached]->line;
	    }
	    else
	    {
		bpos = buf->len + 1;
		line = buf->nlines;
	    }

	    while (buf->eob_cached > a_bufp->index)
	    {
		buf->eob_cached--;

		/* Update bpos. */
		bpos -= ((cw_uint64_t) CW_BUFP_SIZE
			 - (cw_uint64_t) buf->bufps[buf->eob_cached]->gap_len);
		buf->bufps[buf->eob_cached]->bpos = bpos;

		/* Update line. */
		line -= ((cw_uint64_t) buf->bufps[buf->eob_cached]->nlines);
		buf->bufps[buf->eob_cached]->line = line;
	    }
	}
    }
}

static cw_uint32_t
bufp_p_pos_b2p(cw_bufp_t *a_bufp, cw_uint64_t a_bpos)
{
    cw_uint32_t ppos, rel_bpos;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);
    cw_assert(a_bpos > 0);
    cw_assert(a_bpos <= a_bufp->buf->len + 1);

    /* Make sure the cached bpos for this bufp is valid before using it for
     * calculations below. */
    bufp_p_cache_validate(a_bufp);

    /* Calculate the offset into bufp up front. */
    cw_assert(a_bpos >= a_bufp->bpos);
    rel_bpos = a_bpos - a_bufp->bpos;

    if (rel_bpos <= a_bufp->gap_off)
    {
	ppos = rel_bpos;
    }
    else
    {
	ppos = rel_bpos + a_bufp->gap_len;
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
	      || a_ppos >= a_bufp->gap_off + a_bufp->gap_len);
    /* The bufp's cached bpos must be valid. */
    cw_assert(a_bufp->index >= a_bufp->buf->bob_cached
	      || a_bufp->index <= a_bufp->buf->eob_cached);

    if (a_ppos <= a_bufp->gap_off)
    {
	bpos = a_ppos + a_bufp->bpos;
    }
    else
    {
	bpos = a_ppos - a_bufp->gap_len;
    }

    return bpos;
}

/* buf. */
static cw_uint64_t
buf_p_bpos_before_lf_validate(cw_buf_t *a_buf, cw_uint64_t a_offset)
{
    /* Find the bufp that the \n resides in.  If the cache is valid for the
     * resulting bufp, it is possible to do a binary search of the bufp array,
     * whether:
     *
     *   Beginning cached range: 0 <= bufp <= a_buf->bob_cached
     *
     * or
     *
     *   Ending cached range: a_buf->eob_cached <= bufp <= a_buf->bufps
     *
     * If the resulting bufp does not have a valid cache, start at
     * buf->bufbs[bob_cached] and extend the cached range until the resulting
     * bufp is sure to be <= a_buf->bob_cached.  It may be that starting at
     * eob_cached and working backward would be faster, and it would even be
     * possible to heuristically determine which end to start from, but the fact
     * that this function is finding the bpos *before* a \n means that it the
     * operation is a forward seek.  Therefore, working from bob_cached is
     * likely to result in a more useful cache for future operations, since all
     * bufp's between the original position and the seek position will then have
     * a valid cache. */

    cw_error("XXX Not implemented");
}

static cw_uint64_t
buf_p_bpos_after_lf_validate(cw_buf_t *a_buf, cw_uint64_t a_offset)
{
    /* Find the bufp that the \n resides in.  If the cache is valid for the
     * resulting bufp, it is possible to do a binary search of the bufp array,
     * whether:
     *
     *   Beginning cached range: 0 <= bufp <= a_buf->bob_cached
     *
     * or
     *
     *   Ending cached range: a_buf->eob_cached <= bufp <= a_buf->bufps
     *
     * If the resulting bufp does not have a valid cache, start at
     * buf->bufbs[eob_cached] and extend the cached range until the resulting
     * bufp is sure to be >= a_buf->eob_cached.  It may be that starting at
     * bob_cached and working foreward would be faster, and it would even be
     * possible to heuristically determine which end to start from, but the fact
     * that this function is finding the bpos *after* a \n means that it the
     * operation is a backward seek.  Therefore, working from eob_cached is
     * likely to result in a more useful cache for future operations, since all
     * bufp's between the original position and the seek position will then have
     * a valid cache.
     *
     * An important side effect of working backward is that the cache is
     * guaranteed to be valid for the bpos *after* the \n (even if the \n is the
     * last character in the bufp), which is what this function finds, after
     * all.  If a different search algorithm is used, it must also make sure
     * that the cache is valid for the resulting bpos. */

    cw_error("XXX Not implemented");
}

static void
buf_p_bufp_cache_validate(cw_buf_t *a_buf, cw_bufp_t *a_bufp)
{
    /* If the cache for a_bufp is not valid, validate it in the quickest way
     * possible, whether that means working forward from bob_cached, or backward
     * from eob_cached. */
    cw_error("XXX Not implemented");
}

static cw_uint32_t
buf_p_bpos_cache_validate(cw_buf_t *a_buf, cw_uint64_t a_bpos)
{
    /* Find the bufp that a_bpos resides in.  If the cache is valid for the
     * resulting bufp, it is possible to do a binary search of the bufp array,
     * whether:
     *
     *   Beginning cached range: 0 <= bufp <= a_buf->bob_cached
     *
     * or
     *
     *   Ending cached range: a_buf->eob_cached <= bufp <= a_buf->bufps
     *
     * If the resulting bufp does not have a valid cache, validate it in the
     * quickest way possible, whether that means working forward from
     * bob_cached, or backward from eob_cached.  Start from the end whose bpos
     * is closest to a_bpos.  Determining which end to start from is somewhat
     * heuristic, since bufp's are only guaranteed to be an average of least 50%
     * full.  However, the variability of bufp fullness is small enough that
     * even if the non-optimal end is started from, a worst case scenario only
     * approaches a factor of 4 more work. */
    cw_error("XXX Not implemented");
}

cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
	cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc,
	void *a_arg)
{
    cw_buf_t *retval;

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

    /* Initialize initial bufp. */
    retval->bufps = (cw_bufp_t **) cw_opaque_alloc(a_alloc, a_arg,
						   sizeof(cw_bufp_t *));
    retval->nbufps = 1;
    retval->bufps[0] = bufp_p_new(retval);
    bufp_p_index_set(retval->bufps[0], 0);
    bufp_p_bpos_set(retval->bufps[0], 1);
    bufp_p_line_set(retval->bufps[0], 1);

    /* Initialize cache. */
    retval->bob_cached = 0;
    retval->eob_cached = 0;

    /* Initialize bufv. */
    retval->bufv = (cw_bufv_t *) cw_opaque_alloc(a_alloc, a_arg,
						 2 * sizeof(cw_bufv_t));

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
    cw_uint32_t i;

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

    if (a_buf->hist != NULL)
    {
	hist_delete(a_buf->hist);
    }

    cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf->bufv,
		      a_buf->nbufps * sizeof(cw_bufv_t));

    for (i = 0; i < a_buf->nbufps; i++)
    {
	bufp_p_delete(a_buf->bufps[i]);
    }

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
    cw_assert(a_mkr->bufp->index >= a_mkr->bufp->buf->bob_cached
	      || a_mkr->bufp->index <= a_mkr->bufp->buf->eob_cached);

    return (a_mkr->bufp->bpos + a_mkr->ppos);
}

static cw_uint64_t
mkr_p_line(cw_mkr_t *a_mkr)
{
    cw_assert(a_mkr->bufp->index >= a_mkr->bufp->buf->bob_cached
	      || a_mkr->bufp->index <= a_mkr->bufp->buf->eob_cached);

    return (a_mkr->bufp->line + a_mkr->pline);
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
	if (a_a->bufp->index < a_b->bufp->index)
	{
	    retval = -1;
	}
	else if (a_a->bufp->index > a_b->bufp->index)
	{
	    retval = 1;
	}
	else
	{
	    retval = 0;
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
    rb_insert(&bufp->mtree, a_mkr, mkr_p_comp, cw_mkr_t, node);

    /* Insert into list.  Make sure that the tree and list orders are the same.
     */
    rb_next(&bufp->mtree, a_mkr, cw_mkr_t, node, next);
    if (next != NULL)
    {
	ql_before_insert(&bufp->mlist, next, a_mkr, link);
    }
    else
    {
	ql_head_insert(&bufp->mlist, a_mkr, link);
    }
}

static void
mkr_p_remove(cw_mkr_t *a_mkr)
{
    cw_bufp_t *bufp = a_mkr->bufp;

    rb_remove(&bufp->mtree, a_mkr, cw_mkr_t, node);
    ql_remove(&bufp->mlist, a_mkr, link);
}

void
mkr_l_insert(cw_mkr_t *a_mkr, cw_bool_t a_record, cw_bool_t a_after,
	     const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_error("XXX Not implemented");
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

    bufp = a_buf->bufps[0];
    a_mkr->bufp = bufp;
    a_mkr->ppos = bufp_p_pos_b2p(bufp, 1);
    a_mkr->pline = 0;
    rb_node_new(&bufp->mtree, a_mkr, node);
    ql_elm_new(a_mkr, link);

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
    cw_uint32_t pindex;
    cw_buf_t *buf;

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
		bpos = buf_p_bpos_before_lf_validate(buf, a_offset);
	    }
	    break;
	}
	case BUFW_REL:
	{
	    buf_p_bufp_cache_validate(buf, a_mkr->bufp);
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
		    bpos = buf_p_bpos_before_lf_validate(buf,
							 mkr_p_line(a_mkr) - 1
							 + a_offset);
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
		    bpos = buf_p_bpos_after_lf_validate(buf, mkr_p_line(a_mkr)
							- a_offset + 1);
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
		bpos = buf_p_bpos_after_lf_validate(buf,
						    buf->nlines + a_offset + 1);
	    }
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    /* Get the index of the bufp that contains bpos. */
    pindex = buf_p_bpos_cache_validate(buf, bpos);

    /* Remove a_mkr from the tree and list of the bufp at its old location. */
    mkr_p_remove(a_mkr);

    /* Update the internal state of a_mkr. */
    a_mkr->bufp = buf->bufps[pindex];
    a_mkr->ppos = bpos - a_mkr->bufp->bpos;
    a_mkr->pline = 1 + a_offset - a_mkr->bufp->line;

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

    bufp_p_cache_validate(a_mkr->bufp);
    retval = mkr_p_line(a_mkr);

    return retval;
}

cw_uint64_t
mkr_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
    cw_uint64_t bpos;
    cw_uint32_t pindex;
    cw_buf_t *buf;

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
	    buf_p_bufp_cache_validate(buf, a_mkr->bufp);
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

    /* Get the index of the bufp that contains bpos. */
    pindex = buf_p_bpos_cache_validate(buf, bpos);

    /* Remove a_mkr from the tree and list of the bufp at its old location. */
    mkr_p_remove(a_mkr);

    /* Update the internal state of a_mkr. */
    a_mkr->bufp = buf->bufps[pindex];
    a_mkr->ppos = bpos - a_mkr->bufp->bpos;
    a_mkr->pline = 1 + a_offset - a_mkr->bufp->line;

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

    bufp_p_cache_validate(a_mkr->bufp);
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
	offset = a_mkr->ppos - a_mkr->bufp->gap_len;
    }

    if (offset == 0)
    {
	if (a_mkr->bufp->index == 0)
	{
	    /* There is no character before BOB. */
	    retval = NULL;
	}
	else
	{
	    cw_bufp_t *bufp;

	    /* The character is in the previous bufp. */
	    bufp = a_mkr->bufp->buf->bufps[a_mkr->bufp->index - 1];

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
	if (a_mkr->ppos <= a_mkr->bufp->gap_off + a_mkr->bufp->gap_len)
	{
	    /* Before gap. */
	    offset = a_mkr->ppos - 1;
	}
	else
	{
	    /* After gap. */
	    offset = a_mkr->ppos - 1 - a_mkr->bufp->gap_len;
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
	offset = a_mkr->ppos - a_mkr->bufp->gap_len;
    }
    
    return &a_mkr->bufp->b[offset];
}

cw_bufv_t *
mkr_range_get(const cw_mkr_t *a_start, const cw_mkr_t *a_end,
	      cw_uint32_t *r_bufvcnt)
{
    cw_error("XXX Not implemented");
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
