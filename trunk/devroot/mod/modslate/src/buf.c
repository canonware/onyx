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
 * bufp : Buffer page.  Each bufp is a fixed size.
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
 * There are two aspects in which the paged buffer gap algorithms scale linearly
 * for the common case:
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
 * ppos:   0   1   2   3   4   5   6   7     0   1   2   3   4   5   6   7   8
 *         |   |   |   |   |   |   |   |     |   |   |   |   |   |   |   |   |
 *         v   v   v   v   v   v   v   v     v   v   v   v   v   v   v   v   v
 *       /---+---+---+---+---+---+---+---\ /---+---+---+---+---+---+---+---\
 *       | A | B | \n|:::|:::|:::| C | D | | \n| E |:::|:::|:::|:::| \n| F |
 *       \---+---+---+---+---+---+---+---/ \---+---+---+---+---+---+---+---/
 *       ^   ^   ^               ^   ^     ^   ^                   ^   ^   ^
 *       |   |   |               |   |     |   |                   |   |   |
 * bpos: 1   2   3               4   5     6   7                   8   9  10
 *       |   |   |               |   |     |   |                   |   |   |
 * line: 1   1   1               2   2     2   3                   4   4   5
 *       |   |   |               |   |     |   |                   |   |   |
 * pline:0   0   0               1   1     0   1                   1   2   2
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
 * since the page size is fixed at buf->bufp_size.
 *
 *         /------- len ------\
 *        /                    \
 * |- gap_off -|           |-------|
 *
 * |-------- buf->bufp_size -------|
 * /---+---+---+---+---+---+---+---\
 * | A | B | C |:::|:::|:::| D | E |
 * \---+---+---+---+---+---+---+---/
 *   ^           ^           ^
 *   |           |           |
 *   0           gap_off     gap_off + (buf->bufp_size - len)
 *
 ******************************************************************************
 *
 * bufp's, mkr's, and ext's are all organized using both red-black trees and
 * doubly linked lists, so that random access and iteration are fast.
 *
 ******************************************************************************
 *
 * Each bufp keeps track of its bpos and line to speed up many operations.  buf
 * modifications can require the values stored in bufp's to be converted between
 * being relative to BOB/EOB.  At any given time, the ranges of bufp's with
 * caches relative to BOB versus EOB may look something like:
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
 * In order to avoid excessive internal fragmentation, buffer pages are
 * coalesced during data deletion, such that for every pair of consecutive
 * pages, they are on average more than 25% full.  In addition, special care is
 * taken during insertion to assure that this requirement is never violated when
 * inserting new pages.
 *
 ******************************************************************************
 *
 * Extents keep track of buffer ranges, and are typically used to associate
 * attributes (primarily for color syntax highlighting) with those ranges.  The
 * end points of an extent are denoted by markers.  This is the primary
 * motivation for making marker operations scalable, since a typical buffer has
 * only a handfull of markers, until extents come into play, at which time the
 * number of markers can quickly become huge.
 *
 * An extent's end points are each open or shut:
 *
 *   * Open : Insertion at the end point causes the text to go outside the
 *            extent.
 *
 *   * Shut : Insertion at the end point causes the text to go inside the
 *            extent.
 *
 * This gives rise to four open/shut combinations:
 *
 *   * Shut-shut (default)
 *
 *   * Shut-open
 *
 *   * Open-shut
 *
 *   * Open-open (can't be zero-length)
 *
 * The behavior of the various cases is as expected, except that zero-length
 * open-open extents are not allowed to exist.  If buffer operations shrink an
 * open-open extent to zero length, it is converted to shut-open.
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
 *                  1         2
 *         12345678901234567890123456789
 *     A : |---------|
 *     B : |-------|
 *     C :   |---------|
 *     D :   |-------|
 *     E :   |-----|
 *     F :   |---|
 *     G :         |-------|
 *     H :         |-----|
 *     I :           |-----------|
 *     J :             |---|
 *     K :               |---|
 *     L :                 |-------|
 *
 *   r-order :
 *                  1         2
 *         12345678901234567890123456789
 *     F :   |---|
 *     E :   |-----|
 *     B : |-------|
 *     D :   |-------|
 *     A : |---------|
 *     C :   |---------|
 *     H :         |-----|
 *     J :             |---|
 *     G :         |-------|
 *     K :               |---|
 *     I :           |-----------|
 *     L :                 |-------|
 *
 * Maintaining both orderings makes it possible to quickly determine the set of
 * extents that overlap any range of the buffer.  Most importantly though, the
 * two orderings make it possible to quickly divide the buffer into fragments,
 * where each fragment is completely overlapped by a particular set of extents.
 * This is used when displaying the buffer.
 *
 * Any single buffer operation (insert or remove) is not capable of re-ordering
 * extents.  At the most extreme, a remove is capable of converting (A > B) or
 * (A < B) to (A == B).  Similarly, an insertion is capable of converting (A ==
 * B) to (A < B) or (A > B).  However, this still has implications during insert
 * and remove operations due to the way extent ordering is handled, and the fact
 * that multiple buffer operations *can* re-order extents.
 *
 * No special maintenance is necessary for the markers that denote the ends of
 * extents; they are normal markers that stay well ordered at all times.
 * However, extent ordering is actually kept track of separately, and there are
 * situations during insertion into a buffer where extra work is necessary to
 * maintain extent ordering.  Specifically, when there are zero-length extents
 * at the insertion position, the extents (but not their markers) need to be
 * removed, then reinserted.
 *
 * In order to support detachable extents, it is necessary to iterate through
 * the MKRO_BEFORE and MKRO_AFTER markers at the deletion point and remove any
 * detachable extents that have shrunk to zero length.
 *
 ******************************************************************************/

/* Compile non-inlined functions if not using inlines. */
#define CW_BUF_C_

#include "../include/modslate.h"
#ifndef HAVE_ASPRINTF
#include "../../../lib/libonyx/src/asprintf.c"
#endif

#ifdef CW_BUF_DUMP
#include <ctype.h>

#define buf_p_rb_recurse_gen(a_fname, a_type, a_field)			\
void									\
a_fname(a_type *a_node, a_type *a_nil)					\
{									\
    if (a_node == a_nil)						\
    {									\
	fprintf(stderr, ".");						\
	return;								\
    }									\
    /* Self. */								\
    fprintf(stderr, "%p", a_node);					\
									\
    /* Left subtree. */							\
    if (a_node->a_field.rbn_left != a_nil)				\
    {									\
	fprintf(stderr, "[");						\
	a_fname(a_node->a_field.rbn_left, a_nil);			\
	fprintf(stderr, "]");						\
    }									\
    else								\
    {									\
	fprintf(stderr, ".");						\
    }									\
									\
    /* Right subtree. */						\
    if (a_node->a_field.rbn_right != a_nil)				\
    {									\
	fprintf(stderr, "<");						\
	a_fname(a_node->a_field.rbn_right, a_nil);			\
	fprintf(stderr, ">");						\
    }									\
    else								\
    {									\
	fprintf(stderr, ".");						\
    }									\
}
buf_p_rb_recurse_gen(buf_p_ptree_dump, cw_bufp_t, pnode)
buf_p_rb_recurse_gen(buf_p_mtree_dump, cw_mkr_t, mnode)
buf_p_rb_recurse_gen(buf_p_ftree_dump, cw_ext_t, fnode)
buf_p_rb_recurse_gen(buf_p_rtree_dump, cw_ext_t, rnode)

#define buf_p_ql_dump(a_ql, a_type, a_field)				\
    do									\
    {									\
	a_type *telm;							\
	ql_foreach(telm, a_ql, a_field)					\
	{								\
	    fprintf(stderr, " %p", telm);				\
	}								\
    } while (0)
#endif

/* Prototypes. */
/* bufp. */
static void
bufp_p_mkrs_ppos_adjust(cw_bufp_t *a_bufp, cw_sint32_t a_adjust,
			cw_uint32_t a_beg_ppos, cw_uint32_t a_end_ppos);
static void
bufp_p_mkrs_pline_adjust(cw_bufp_t *a_bufp, cw_sint32_t a_adjust,
			 cw_uint32_t a_beg_ppos);
static void
bufp_p_gap_move(cw_bufp_t *a_bufp, cw_uint32_t a_ppos);
static cw_bufp_t *
bufp_p_new(cw_buf_t *a_buf);
static cw_uint32_t
bufp_p_simple_insert(cw_bufp_t *a_bufp, const cw_bufv_t *a_bufv,
		     cw_uint32_t a_bufvcnt, cw_uint32_t a_count,
		     cw_bool_t a_reverse);
static void
bufp_p_delete(cw_bufp_t *a_bufp);
static cw_uint32_t
bufp_p_pos_p2r(cw_bufp_t *a_bufp, cw_uint32_t a_ppos);
static cw_uint32_t
bufp_p_pos_b2p(cw_bufp_t *a_bufp, cw_uint64_t a_bpos);
static cw_uint64_t
bufp_p_pos_p2b(cw_bufp_t *a_bufp, cw_uint32_t a_ppos);
static cw_uint32_t
bufp_p_ppos2pline(cw_bufp_t *a_bufp, cw_uint32_t a_ppos);
#ifdef CW_BUF_DUMP
static void
bufp_p_dump(cw_bufp_t *a_bufp, const char *a_beg, const char *a_mid,
	    const char *a_end);
#endif
#ifdef CW_BUF_VALIDATE
static void
bufp_p_validate(cw_bufp_t *a_bufp);
#endif

/* buf. */
static cw_sint32_t
buf_p_bufp_at_bpos_comp(cw_bufp_t *a_key, cw_bufp_t *a_bufp);
static cw_bufp_t *
buf_p_bufp_at_bpos(cw_buf_t *a_buf, cw_uint64_t a_bpos);
static cw_sint32_t
buf_p_bpos_lf_comp(cw_bufp_t *a_key, cw_bufp_t *a_bufp);
static cw_uint64_t
buf_p_bpos_before_lf(cw_buf_t *a_buf, cw_uint64_t a_lf, cw_bufp_t **r_bufp);
static cw_uint64_t
buf_p_bpos_after_lf(cw_buf_t *a_buf, cw_uint64_t a_lf, cw_bufp_t **r_bufp);
static void
buf_p_bufp_cur_set(cw_buf_t *a_buf, cw_bufp_t *a_bufp);
static cw_sint32_t
buf_p_bufp_insert_comp(cw_bufp_t *a_a, cw_bufp_t *a_b);
static cw_uint64_t
buf_p_bufv_insert(cw_buf_t *a_buf, cw_bufp_t *a_bufp, cw_bufp_t *a_pastp,
		  const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt);
static cw_uint64_t
buf_p_bufv_rinsert(cw_buf_t *a_buf, cw_bufp_t *a_bufp, cw_bufp_t *a_pastp,
		   const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt);
static void
buf_p_bufp_insert(cw_buf_t *a_buf, cw_bufp_t *a_bufp);
static void
buf_p_bufp_remove(cw_buf_t *a_buf, cw_bufp_t *a_bufp);
static void
buf_p_bufp_splice(cw_buf_t *a_buf, cw_bufp_t *a_start, cw_bufp_t *a_end);

/* mkr. */
static void
mkr_p_new(cw_mkr_t *a_mkr, cw_buf_t *a_buf, cw_mkro_t a_order);
static void
mkr_p_dup(cw_mkr_t *a_to, const cw_mkr_t *a_from, cw_mkro_t a_order);
static cw_uint64_t
mkr_p_bpos(cw_mkr_t *a_mkr);
static cw_uint64_t
mkr_p_line(cw_mkr_t *a_mkr);
static cw_sint32_t
mkr_p_comp(cw_mkr_t *a_a, cw_mkr_t *a_b);
static void
mkr_p_insert(cw_mkr_t *a_mkr);
static void
mkr_p_remove(cw_mkr_t *a_mkr);
static cw_uint32_t
mkr_p_simple_insert(cw_mkr_t *a_mkr, cw_bool_t a_after, const cw_bufv_t *a_bufv,
		    cw_uint32_t a_bufvcnt, cw_uint32_t a_count,
		    cw_bool_t a_reverse);
static cw_uint32_t
mkr_p_before_slide_insert(cw_mkr_t *a_mkr, cw_bool_t a_after,
			  cw_bufp_t *a_prevp, const cw_bufv_t *a_bufv,
			  cw_uint32_t a_bufvcnt, cw_uint32_t a_count,
			  cw_bool_t a_reverse);
static cw_uint32_t
mkr_p_after_slide_insert(cw_mkr_t *a_mkr, cw_bool_t a_after, cw_bufp_t *a_nextp,
			 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt,
			 cw_uint32_t a_count, cw_bool_t a_reverse);
static cw_uint64_t
mkr_p_split_insert(cw_mkr_t *a_mkr, cw_bool_t a_after, const cw_bufv_t *a_bufv,
		   cw_uint32_t a_bufvcnt, cw_uint64_t a_count,
		   cw_bool_t a_reverse);

/* ext. */
static cw_sint32_t
ext_p_fcomp(cw_ext_t *a_a, cw_ext_t *a_b);
static cw_sint32_t
ext_p_rcomp(cw_ext_t *a_a, cw_ext_t *a_b);
static void
ext_p_insert(cw_ext_t *a_ext);
static void
ext_p_remove(cw_ext_t *a_ext);

/* bufv. */

/* A simplified version of bufv_copy() that counts '\n' characters that are
 * copied, and returns that rather than the number of elements copied. */
CW_P_INLINE cw_uint32_t
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

/* A simplified version of bufv_rcopy() that counts '\n' characters that are
 * copied, and returns that rather than the number of elements copied. */
CW_P_INLINE cw_uint32_t
bufv_p_rcopy(cw_bufv_t *a_to, cw_uint32_t a_to_len, const cw_bufv_t *a_fr,
	     cw_uint32_t a_fr_len)
{
    cw_uint32_t retval, to_el, fr_el, to_off, fr_off;

    cw_check_ptr(a_to);
    cw_check_ptr(a_fr);

    retval = 0;
    fr_el = a_fr_len - 1;
    fr_off = a_fr[fr_el].len - 1;
    /* Iterate over bufv elements. */
    for (to_el = 0; to_el < a_to_len; to_el++)
    {
	/* Iterate over bufv element contents. */
	for (to_off = 0; to_off < a_to[to_el].len; to_off++)
	{
	    /* If there is no more room in the current source bufv element,
	     * move on to the previous one. */
	    while (fr_off == 0xffffffff)
	    {
		fr_el--;
		if (fr_el == 0xffffffff)
		{
		    goto RETURN;
		}
		fr_off = a_fr[fr_el].len - 1;
	    }

	    a_to[to_el].data[to_off] = a_fr[fr_el].data[fr_off];

	    /* Count newlines. */
	    if (a_to[to_el].data[to_off] == '\n')
	    {
		retval++;
	    }

	    /* Decrement the position to copy from. */
	    fr_off--;
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
	    /* If there is no more room in the current destination bufv element,
	     * move on to the next one. */
	    while (to_off == a_to[to_el].len)
	    {
		to_off = 0;
		to_el++;
		if (to_el == a_to_len)
		{
		    goto RETURN;
		}
	    }

	    a_to[to_el].data[to_off] = a_fr[fr_el].data[fr_off];

	    /* Copy no more than a_maxlen elements (unless a_maxlen is 0). */
	    retval++;
	    if (retval == a_maxlen)
	    {
		goto RETURN;
	    }

	    /* Increment the position to copy to. */
	    to_off++;
	}
    }

    RETURN:
    return retval;
}

cw_uint64_t
bufv_rcopy(cw_bufv_t *a_to, cw_uint32_t a_to_len, const cw_bufv_t *a_fr,
	   cw_uint32_t a_fr_len, cw_uint64_t a_maxlen)
{
    cw_uint64_t retval;
    cw_uint32_t to_el, fr_el, to_off, fr_off;

    cw_check_ptr(a_to);
    cw_check_ptr(a_fr);

    retval = 0;
    fr_el = a_fr_len - 1;
    fr_off = a_fr[fr_el].len - 1;
    /* Iterate over bufv elements. */
    for (to_el = 0; to_el < a_to_len; to_el++)
    {
	/* Iterate over bufv element contents. */
	for (to_off = 0; to_off < a_to[to_el].len; to_off++)
	{
	    /* If there is no more room in the current source bufv element,
	     * move on to the previous one. */
	    while (fr_off == 0xffffffff)
	    {
		fr_el--;
		if (fr_el == 0xffffffff)
		{
		    goto RETURN;
		}
		fr_off = a_fr[fr_el].len - 1;
	    }

	    a_to[to_el].data[to_off] = a_fr[fr_el].data[fr_off];

	    /* Copy no more than a_maxlen elements (unless a_maxlen is 0). */
	    retval++;
	    if (retval == a_maxlen)
	    {
		goto RETURN;
	    }

	    /* Decrement the position to copy from. */
	    fr_off--;
	}
    }

    RETURN:
    return retval;
}

/* bufp. */
CW_P_INLINE cw_uint64_t
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

    cw_assert(retval != 0);
    return retval;
}

CW_P_INLINE cw_uint64_t
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
	retval = a_bufp->buf->nlines - a_bufp->line;
    }

    return retval;
}

/* Adjust ppos field of a range of mkr's, starting at a_beg_ppos (inclusive),
 * and ending at a_end_ppos (exclusive). */
static void
bufp_p_mkrs_ppos_adjust(cw_bufp_t *a_bufp, cw_sint32_t a_adjust,
			cw_uint32_t a_beg_ppos, cw_uint32_t a_end_ppos)
{
    cw_mkr_t *mkr, *prev, key;

    cw_check_ptr(a_bufp);
    cw_assert(a_adjust != 0);
    cw_assert(a_beg_ppos < a_end_ppos);

    /* Find the first affected mkr. */
    key.order = MKRO_BEFORE;
    key.ppos = a_beg_ppos;
#ifdef CW_DBG
    key.bufp = a_bufp;
    key.magic = CW_MKR_MAGIC;
#endif

    rb_nsearch(&a_bufp->mtree, &key, mkr_p_comp, cw_mkr_t, mnode, mkr);
    if (mkr != rb_tree_nil(&a_bufp->mtree))
    {
	/* Get prev before iterating forward, in preparation for backward
	 * iteration. */
	prev = ql_prev(&a_bufp->mlist, mkr, mlink);

	/* Iterate forward. */
	for (;
	     mkr != NULL && mkr->ppos < a_end_ppos;
	     mkr = ql_next(&a_bufp->mlist, mkr, mlink))
	{
	    mkr->ppos += a_adjust;
	}

	/* Iterate backward. */
	for (mkr = prev;
	     mkr != NULL && mkr->ppos == a_beg_ppos;
	     mkr = ql_prev(&a_bufp->mlist, mkr, mlink))
	{
	    mkr->ppos += a_adjust;
	}
    }
}

/* Adjust pline field of a range of mkr's, starting at a_beg_pline (inclusive),
 * and ending at a_end_pline (exclusive). */
static void
bufp_p_mkrs_pline_adjust(cw_bufp_t *a_bufp, cw_sint32_t a_adjust,
			 cw_uint32_t a_beg_ppos)
{
    cw_mkr_t *mkr;

    cw_check_ptr(a_bufp);
    cw_assert(a_beg_ppos <= a_bufp->buf->bufp_size + 1);

    for (mkr = ql_last(&a_bufp->mlist, mlink);
	 mkr != NULL && mkr->ppos >= a_beg_ppos;
	 mkr = ql_prev(&a_bufp->mlist, mkr, mlink))
    {
	mkr->pline += a_adjust;
    }
}

static void
bufp_p_gap_move(cw_bufp_t *a_bufp, cw_uint32_t a_ppos)
{
    cw_uint32_t bufp_size = a_bufp->buf->bufp_size;

    cw_assert(a_ppos < bufp_size
	      || (a_ppos == bufp_size && a_bufp->len == bufp_size));

    /* Move the gap if it isn't already where it needs to be. */
    if (a_bufp->gap_off != a_ppos)
    {
	if (a_bufp->len < bufp_size)
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
			&a_bufp->b[a_bufp->gap_off + (bufp_size - a_bufp->len)],
			(a_ppos - a_bufp->gap_off));

		/* Adjust the ppos of all mkr's with ppos in the moved
		 * region. */
		bufp_p_mkrs_ppos_adjust(a_bufp, -(bufp_size - a_bufp->len),
					a_bufp->gap_off + (bufp_size
							   - a_bufp->len),
					a_ppos + (bufp_size - a_bufp->len));
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
		memmove(&a_bufp->b[(bufp_size - a_bufp->len) + a_ppos],
			&a_bufp->b[a_ppos],
			(a_bufp->gap_off - a_ppos));

		/* Adjust the ppos of all mkr's with ppos in the moved
		 * region. */
		bufp_p_mkrs_ppos_adjust(a_bufp, bufp_size - a_bufp->len,
					a_ppos, a_bufp->gap_off);
	    }
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

    retval->buf = a_buf;

    /* Don't bother initializing bob_relative, bpos, or line, since they are
     * explicitly set later. */

    /* Initialize length and line count. */
    retval->len = 0;
    retval->nlines = 0;

    /* Initialize buffer gap. */
    retval->gap_off = 0;

    /* Allocate buffer. */
    retval->b = (cw_uint8_t *) cw_opaque_alloc(a_buf->alloc, a_buf->arg,
					       a_buf->bufp_size);

    /* Initialize marker tree and list. */
    rb_tree_new(&retval->mtree, mnode);
    ql_new(&retval->mlist);

    /* Initial bufp tree and list linkage. */
    rb_node_new(&a_buf->ptree, retval, pnode);
    ql_elm_new(retval, plink);

#ifdef CW_DBG
    retval->magic = CW_BUFP_MAGIC;
#endif

    return retval;
}

/* Insert data into a single bufp, without moving the gap, but do keep any mkr's
 * after the gap consistent.  This function assumes that data will fit.  The
 * cached bpos/line are not used, nor are they updated. */
static cw_uint32_t
bufp_p_simple_insert(cw_bufp_t *a_bufp, const cw_bufv_t *a_bufv,
		     cw_uint32_t a_bufvcnt, cw_uint32_t a_count,
		     cw_bool_t a_reverse)
{
    cw_uint32_t nlines;
    cw_bufv_t bufv;

    /* Insert. */
    bufv.data = &a_bufp->b[a_bufp->gap_off];
    bufv.len = a_bufp->buf->bufp_size - a_bufp->len;
    if (a_reverse == FALSE)
    {
	nlines = bufv_p_copy(&bufv, 1, a_bufv, a_bufvcnt);
    }
    else
    {
	nlines = bufv_p_rcopy(&bufv, 1, a_bufv, a_bufvcnt);
    }

    /* Shrink the gap. */
    a_bufp->gap_off += a_count;

    /* Adjust the buf's length and line count. */
    a_bufp->len += a_count;
    a_bufp->nlines += nlines;

    if (nlines > 0)
    {
	/* Adjust the line numbers of all mkr's after the gap. */
	bufp_p_mkrs_pline_adjust(a_bufp, nlines,
				 a_bufp->gap_off
				 + (a_bufp->buf->bufp_size - a_bufp->len));
    }

    return nlines;
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
	cw_assert(first == rb_tree_nil(&a_bufp->mtree));
    }
#endif
    cw_assert(ql_first(&a_bufp->mlist) == NULL);
    cw_assert(qr_next(a_bufp, plink) == a_bufp);

    cw_opaque_dealloc(a_bufp->buf->dealloc, a_bufp->buf->arg, a_bufp->b,
		      a_bufp->buf->bufp_size);

    cw_opaque_dealloc(a_bufp->buf->dealloc, a_bufp->buf->arg, a_bufp,
		      sizeof(cw_bufp_t));
}

/* Convert page position (ppos) to bpos, relative to the beginning of a_bufp. */
static cw_uint32_t
bufp_p_pos_p2r(cw_bufp_t *a_bufp, cw_uint32_t a_ppos)
{
    cw_uint32_t rpos;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);
    /* Check that a_ppos is one of the following:
     *
     * *) Before the gap.
     *
     * *) After the gap, but before the end of the bufp.
     *
     * *) Just past the end of the last bufp in the buf.
     */
    cw_assert(a_ppos < a_bufp->gap_off
	      || (a_ppos >= a_bufp->gap_off
		  + (a_bufp->buf->bufp_size - a_bufp->len)
		  && a_ppos < a_bufp->buf->bufp_size)
	      || (a_ppos == a_bufp->buf->bufp_size
		  && a_bufp == ql_last(&a_bufp->buf->plist, plink)));

    if (a_ppos < a_bufp->gap_off)
    {
	rpos = a_ppos;
    }
    else
    {
	rpos = a_ppos - (a_bufp->buf->bufp_size - a_bufp->len);
    }

    return rpos;
}

/* Convert bpos to ppos. */
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
    cw_assert(rel_bpos <= a_bufp->buf->bufp_size);

    if (rel_bpos < a_bufp->gap_off)
    {
	ppos = rel_bpos;
    }
    else
    {
	ppos = rel_bpos + (a_bufp->buf->bufp_size - a_bufp->len);
    }

    cw_assert(ppos < a_bufp->gap_off
	      || (ppos >= a_bufp->gap_off
		  + (a_bufp->buf->bufp_size - a_bufp->len)
		  && ppos < a_bufp->buf->bufp_size)
	      || (ppos == a_bufp->buf->bufp_size
		  && a_bufp == ql_last(&a_bufp->buf->plist, plink)));
    return ppos;
}

/* Convert ppos to bpos. */
static cw_uint64_t
bufp_p_pos_p2b(cw_bufp_t *a_bufp, cw_uint32_t a_ppos)
{
    return ((cw_uint64_t) bufp_p_pos_p2r(a_bufp, a_ppos) + bufp_p_bpos(a_bufp));
}

static cw_uint32_t
bufp_p_ppos2pline(cw_bufp_t *a_bufp, cw_uint32_t a_ppos)
{
    cw_uint32_t pline, i, bufp_size;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    bufp_size = a_bufp->buf->bufp_size;

    cw_assert(a_ppos <= a_bufp->gap_off
	      || a_ppos >= a_bufp->gap_off + (bufp_size - a_bufp->len)
	      || (a_ppos == bufp_size
		  && a_bufp == ql_last(&a_bufp->buf->plist, plink)));

    if (a_ppos < a_bufp->gap_off)
    {
	if (a_ppos * 2 <= a_bufp->len)
	{
	    /* Start from the beginning. */
	    for (i = pline = 0; i < a_ppos; i++)
	    {
		if (a_bufp->b[i] == '\n')
		{
		    pline++;
		}
	    }
	}
	else
	{
	    /* Start from the end. */
	    for (i = bufp_size - 1, pline = a_bufp->nlines;
		 i >= a_bufp->gap_off + (bufp_size - a_bufp->len);
		 i--)
	    {
		if (a_bufp->b[i] == '\n')
		{
		    pline--;
		}
	    }
	    for (i = a_bufp->gap_off - 1;
		 i >= a_ppos;
		 i--)
	    {
		if (a_bufp->b[i] == '\n')
		{
		    pline--;
		}
	    }
	}
    }
    else
    {
	if ((bufp_size - a_ppos) * 2 > a_bufp->len)
	{
	    /* Start from the beginning. */
	    for (i = pline = 0; i < a_bufp->gap_off; i++)
	    {
		if (a_bufp->b[i] == '\n')
		{
		    pline++;
		}
	    }
	    for (i = a_bufp->gap_off + (bufp_size - a_bufp->len);
		 i < a_ppos;
		 i++)
	    {
		if (a_bufp->b[i] == '\n')
		{
		    pline++;
		}
	    }
	}
	else
	{
	    /* Start from the end. */
	    for (i = bufp_size - 1, pline = a_bufp->nlines;
		 i >= a_ppos;
		 i--)
	    {
		if (a_bufp->b[i] == '\n')
		{
		    pline--;
		}
	    }
	}
    }

    return pline;
}

#ifdef CW_BUF_DUMP
static void
bufp_p_dump(cw_bufp_t *a_bufp, const char *a_beg, const char *a_mid,
	    const char *a_end)
{
    const char hchars[] = "0123456789abcdef";
    cw_uint32_t i;
    cw_mkr_t *tmkr;
    const char *beg, *mid, *end;
    char *tbeg, *tend;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    beg = (a_beg != NULL) ? a_beg : "";
    mid = (a_mid != NULL) ? a_mid : beg;
    end = (a_end != NULL) ? a_end : mid;

    fprintf(stderr, "%sbufp: %p\n", beg, a_bufp);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> buf: %p\n", mid, a_bufp->buf);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> bob_relative: %s\n", mid,
	    a_bufp->bob_relative ? "TRUE" : "FALSE");
    if (a_bufp->bob_relative)
    {
	fprintf(stderr, "%s|-> bpos: %llu\n", mid, a_bufp->bpos);
	fprintf(stderr, "%s|-> line: %llu\n", mid, a_bufp->line);
    }
    else
    {
	fprintf(stderr, "%s|-> bpos: %llu (%llu)\n", mid, a_bufp->bpos,
		bufp_p_bpos(a_bufp));
	fprintf(stderr, "%s|-> line: %llu (%llu)\n", mid, a_bufp->line,
		bufp_p_line(a_bufp));
    }

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> len: %u\n", mid, a_bufp->len);
    fprintf(stderr, "%s|-> nlines: %u\n", mid, a_bufp->nlines);
    fprintf(stderr, "%s|-> gap_off: %u\n", mid, a_bufp->gap_off);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> b: %p\n", mid, a_bufp->b);
    if (a_bufp->gap_off > 0)
    {
	fprintf(stderr, "%s|   b[0..%u]: \"", mid, a_bufp->gap_off - 1);
	for (i = 0; i < a_bufp->gap_off; i++)
	{
	    if (isprint(a_bufp->b[i]))
	    {
		fprintf(stderr, "%c", a_bufp->b[i]);
	    }
	    else
	    {
		fprintf(stderr, "\\x%c%c",
			hchars[a_bufp->b[i] >> 4],
			hchars[a_bufp->b[i] & 0xf]);
	    }
	}
	fprintf(stderr, "\"\n");
    }
    if (a_bufp->gap_off + (a_bufp->buf->bufp_size - a_bufp->len)
	< a_bufp->buf->bufp_size)
    {
	fprintf(stderr, "%s|   b[%u..%u]: \"", mid,
		a_bufp->gap_off + (a_bufp->buf->bufp_size - a_bufp->len),
		a_bufp->buf->bufp_size - 1);
	for (i = a_bufp->gap_off + (a_bufp->buf->bufp_size - a_bufp->len);
	     i < a_bufp->buf->bufp_size;
	     i++)
	{
	    if (isprint(a_bufp->b[i]))
	    {
		fprintf(stderr, "%c", a_bufp->b[i]);
	    }
	    else
	    {
		fprintf(stderr, "\\x%c%c",
			hchars[a_bufp->b[i] >> 4],
			hchars[a_bufp->b[i] & 0xf]);
	    }
	}
	fprintf(stderr, "\"\n");
    }

    fprintf(stderr, "%s|-> mlist:", mid);
    buf_p_ql_dump(&a_bufp->mlist, cw_mkr_t, mlink);
    fprintf(stderr, "\n");
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> mtree: ", mid);
    buf_p_mtree_dump(rb_root(&a_bufp->mtree), rb_tree_nil(&a_bufp->mtree));
    fprintf(stderr, "\n");

    i = 0;
    ql_foreach(tmkr, &a_bufp->mlist, mlink)
    {
	fprintf(stderr, "%s|\n", mid);
	asprintf(&tbeg, "%s| [%u] ", mid, i);
	if (tmkr != ql_last(&a_bufp->mlist, mlink))
	{
	    mkr_dump(tmkr, tbeg, NULL, NULL);
	}
	else
	{
	    asprintf(&tend, "%sV [%u] ", mid, i);
	    mkr_dump(tmkr, tbeg, NULL, tend);
	    free(tend);
	}
	free(tbeg);
	i++;
    }
}
#endif

#ifdef CW_BUF_VALIDATE
static void
bufp_p_validate(cw_bufp_t *a_bufp)
{
    cw_uint32_t i, nlines, bufp_size;
    cw_mkr_t *mkr, *tmkr;

    cw_check_ptr(a_bufp);
    cw_dassert(a_bufp->magic == CW_BUFP_MAGIC);

    bufp_size = a_bufp->buf->bufp_size;

    /* Validate consistency of len and gap_off. */
    cw_assert(a_bufp->gap_off <= a_bufp->len);
    cw_assert(a_bufp->len <= bufp_size);
    cw_assert(a_bufp->gap_off <= bufp_size);

    /* Validate nlines. */
    for (i = nlines = 0; i < a_bufp->gap_off; i++)
    {
	if (a_bufp->b[i] == '\n')
	{
	    nlines++;
	}
    }
    for (i = a_bufp->gap_off + (bufp_size - a_bufp->len);
	 i < bufp_size;
	 i++)
    {
	if (a_bufp->b[i] == '\n')
	{
	    nlines++;
	}
    }
    cw_assert(nlines == a_bufp->nlines);

    /* Iterate through mkr's. */
    ql_foreach(mkr, &a_bufp->mlist, mlink)
    {
	/* Make sure the mkr's point to this bufp. */
	cw_assert(mkr->bufp == a_bufp);

	/* Validate mkr. */
	mkr_validate(mkr);

	/* Validate consistent ordering of mtree and mlist. */
	rb_prev(&a_bufp->mtree, mkr, cw_mkr_t, mnode, tmkr);
	if (tmkr == rb_tree_nil(&a_bufp->mtree))
	{
	    tmkr = NULL;
	}
	cw_assert(ql_prev(&a_bufp->mlist, mkr, mlink) == tmkr);

	rb_next(&a_bufp->mtree, mkr, cw_mkr_t, mnode, tmkr);
	if (tmkr == rb_tree_nil(&a_bufp->mtree))
	{
	    tmkr = NULL;
	}
	cw_assert(ql_next(&a_bufp->mlist, mkr, mlink) == tmkr);

	/* Validate increasing order. */
	if (tmkr != NULL)
	{
	    cw_assert(mkr_p_comp(mkr, tmkr) <= 0);
	}
    }
}
#endif

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
    else if (a_key->bpos < bpos + (cw_uint64_t) a_bufp->len)
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
#ifdef CW_DBG
    key.magic = CW_BUFP_MAGIC;
#endif

    rb_search(&a_buf->ptree, &key, buf_p_bufp_at_bpos_comp, pnode, retval);
    if (retval == rb_tree_nil(&a_buf->ptree))
    {
	/* The only time this can happen is when searching for a bpos past EOB,
	 * or when searching for bpos 1 in an empty buffer. */
	if (a_bpos == 1)
	{
	    retval = ql_first(&a_buf->plist);
	}
	else
	{
	    cw_assert(a_bpos <= a_buf->len + 1);
	    retval = ql_last(&a_buf->plist, plink);
	}
    }

    return retval;
}

static cw_sint32_t
buf_p_bpos_lf_comp(cw_bufp_t *a_key, cw_bufp_t *a_bufp)
{
    cw_sint32_t retval;
    cw_uint64_t lf = a_key->line;
    cw_uint64_t line = bufp_p_line(a_bufp);

    if (lf < line)
    {
	retval = -1;
    }
    else if (lf < line + a_bufp->nlines)
    {
	retval = 0;
    }
    else
    {
	retval = 1;
    }

    return retval;
}

/* Return the bpos just before a line feed ('\n'), as well as the bufp that
 * contains it.  Line feeds are numbered as such:
 *
 * line:  1111 2222 3333 4 555
 * data:  abc\ndef\nghi\n\njkl
 * lf #:     1    2    3 4    5
 *
 */
static cw_uint64_t
buf_p_bpos_before_lf(cw_buf_t *a_buf, cw_uint64_t a_lf, cw_bufp_t **r_bufp)
{
    cw_uint32_t ppos, nlines, bufp_size;
    cw_uint64_t retval, bufp_line;
    cw_bufp_t *bufp, key;

    cw_assert(a_lf > 0);

    /* Initialize enough of key for searching.  Search for the bufp that
     * contains the '\n'.  The bpos just before the '\n' is always in the same
     * bufp. */
    key.line = a_lf;
#ifdef CW_DBG
    key.magic = CW_BUFP_MAGIC;
#endif

    rb_search(&a_buf->ptree, &key, buf_p_bpos_lf_comp, pnode, bufp);
    if (bufp == rb_tree_nil(&a_buf->ptree))
    {
	cw_assert(a_lf >= a_buf->nlines);
	bufp = ql_last(&a_buf->plist, plink);
	retval = a_buf->len + 1;
	goto DONE;
    }

    bufp_line = bufp_p_line(bufp);
    cw_assert(bufp_line + (cw_uint64_t) bufp->nlines >= a_lf + 1);

    /* Before the gap. */
    for (ppos = nlines = 0; ppos < bufp->gap_off; ppos++)
    {
	if (bufp->b[ppos] == '\n')
	{
	    nlines++;
	    if ((cw_uint64_t) nlines + bufp_line == a_lf + 1)
	    {
		retval = bufp_p_bpos(bufp) + (cw_uint64_t) ppos;
		goto DONE;
	    }
	}
    }

    /* After the gap. */
    bufp_size = bufp->buf->bufp_size;
    for (ppos += (bufp_size - bufp->len);; ppos++)
    {
	cw_assert(ppos < bufp_size);

	if (bufp->b[ppos] == '\n')
	{
	    nlines++;
	    if ((cw_uint64_t) nlines + bufp_line == a_lf + 1)
	    {
		retval = bufp_p_bpos(bufp)
		    + (cw_uint64_t) ppos
		    - (cw_uint64_t) (bufp_size - bufp->len);
		goto DONE;
	    }
	}
    }

    DONE:
    *r_bufp = bufp;
    return retval;
}

static cw_uint64_t
buf_p_bpos_after_lf(cw_buf_t *a_buf, cw_uint64_t a_lf, cw_bufp_t **r_bufp)
{
    cw_uint64_t bpos;
    cw_bufp_t *bufp;

    bpos = buf_p_bpos_before_lf(a_buf, a_lf, &bufp);

    if (bpos < a_buf->len + 1)
    {
	/* Move forward one position.  This could involve moving to the next
	 * bufp. */
	bpos++;
	if (bpos == bufp_p_bpos(bufp) + (cw_uint64_t) bufp->len)
	{
	    bufp = ql_next(&bufp->buf->plist, bufp, plink);
	}
    }

    *r_bufp = bufp;
    return bpos;
}

/* This function modifies the cached position information in the bufp list such
 * that a_bufp is the last bufp to store its position relative to BOB.  The
 * cached position of a_bufp must be correct when this function is called, but
 * there is no need for the other bufp's in the affected range to have accurate
 * cached positions. */
static void
buf_p_bufp_cur_set(cw_buf_t *a_buf, cw_bufp_t *a_bufp)
{
    cw_assert(a_bufp->buf == a_buf);

    if (a_buf->bufp_cur != a_bufp)
    {
	cw_bufp_t *bufp;
	cw_uint64_t bpos, line;

	cw_assert(bufp_p_bpos(a_bufp) != bufp_p_bpos(a_buf->bufp_cur));
//	fprintf(stderr, "%s:%d:%s(): %p(%llu) > %p(%llu) ?\n", __FILE__, __LINE__, __FUNCTION__, a_bufp, bufp_p_bpos(a_bufp), a_buf->bufp_cur, bufp_p_bpos(a_buf->bufp_cur));
	if (bufp_p_bpos(a_bufp) > bufp_p_bpos(a_buf->bufp_cur))
	{
	    /* Move forward. */
	    bufp = a_buf->bufp_cur;
	    bpos = bufp->bpos;
	    line = bufp->line;
	    do
	    {
		bpos += (cw_uint64_t) bufp->len;
		line += (cw_uint64_t) bufp->nlines;

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
	    bpos = a_buf->len + 1 - bufp->bpos;
	    line = a_buf->nlines - bufp->line;
	    do {
		bufp->bob_relative = FALSE;
		bufp->bpos = bpos;
		bufp->line = line;

		bufp = ql_prev(&a_buf->plist, bufp, plink);
		cw_check_ptr(bufp);
		cw_assert(bufp->bob_relative);

		bpos += (cw_uint64_t) bufp->len;
		line += (cw_uint64_t) bufp->nlines;
	    } while (bufp != a_bufp);
	    a_buf->bufp_cur = bufp;
	}
    }
}

/* This function inserts a_bufv into a series of contiguous bufp's.  The first
 * and last bufp's must have their gaps moved to the end and begin,
 * respectively, and the intermediate bufp's must be empty.  This allows
 * bufp_p_simple_insert() to be used.  a_buf->bufp_cur must be at or after
 * a_bufp so that cached positions can be updated during insertion. */
static cw_uint64_t
buf_p_bufv_insert(cw_buf_t *a_buf, cw_bufp_t *a_bufp, cw_bufp_t *a_pastp,
		  const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_uint64_t nlines, bpos, line;
    cw_uint32_t i, v, cnt;
    cw_bufp_t *bufp;
    cw_bufv_t vsplit, vremain;

    cw_assert(a_bufp->bob_relative);

    /* Iteratively call bufp_p_simple_insert(), taking care never to insert more
     * data than will fit in the bufp being inserted into.  The approach taken
     * here is to use the elements of a_bufv directly, exept when an element
     * would overflow the space available in the bufp being inserted into.  In
     * that case, the offending element is iteratively broken into pieces and
     * inserted into bufp's, until it has been completely inserted.  In the
     * worst case, this means three calls to bufp_p_simple_insert() per bufp.
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
    nlines = 0;
    v = 0;
    vremain.len = 0;
    for (bufp = a_bufp, bpos = bufp->bpos, line = bufp->line;
	 bufp != a_pastp;
	 bufp = ql_next(&a_buf->plist, bufp, plink))
    {
	/* Insert remainder, if any. */
	if (vremain.len != 0)
	{
	    vsplit.data = vremain.data;
	    if (a_buf->bufp_size - bufp->len < vremain.len)
	    {
		vsplit.len = a_buf->bufp_size - bufp->len;
	    }
	    else
	    {
		vsplit.len = vremain.len;
	    }
	    vremain.data = &vremain.data[vsplit.len];
	    vremain.len -= vsplit.len;

	    nlines += (cw_uint64_t) bufp_p_simple_insert(bufp, &vsplit, 1,
							 vsplit.len, FALSE);

	    if (a_buf->bufp_size - bufp->len == 0 || vremain.len != 0)
	    {
		/* No more space in this bufp, or no more remaining data in
		 * bufv[v]. */
		goto CONTINUE;
	    }
	}

	/* Iterate over a_bufv elements and insert their contents into the
	 * current bufp. */
	cnt = 0;
	for (i = v; i < a_bufvcnt; i++)
	{
	    if (cnt + a_bufv[i].len > a_buf->bufp_size - bufp->len)
	    {
		break;
	    }
	    cnt += a_bufv[i].len;
	}
	if (cnt != 0)
	{
	    nlines += (cw_uint64_t) bufp_p_simple_insert(bufp, &a_bufv[v],
							 i - v, cnt, FALSE);
	    v = i;
	}

	/* Split a_bufv[v] if bufp isn't full and there are more data. */
	if (a_buf->bufp_size - bufp->len != 0 && v < a_bufvcnt)
	{
	    vsplit.data = a_bufv[v].data;
	    vsplit.len = a_buf->bufp_size - bufp->len;
	    vremain.data = &vsplit.data[vsplit.len];
	    vremain.len = a_bufv[v].len - vsplit.len;
	    v++;

	    nlines += (cw_uint64_t) bufp_p_simple_insert(bufp, &vsplit, 1,
							 vsplit.len, FALSE);
	}

	CONTINUE:
	/* Update the cached position. */
	bufp->bob_relative = TRUE;
	bufp->bpos = bpos;
	bpos += (cw_uint64_t) bufp->len;
	bufp->line = line;
	line += (cw_uint64_t) bufp->nlines;
	a_buf->bufp_cur = bufp;
    }

    return nlines;
}

/* This function inserts a_bufv into a series of contiguous bufp's in reverse
 * order.  The first and last bufp's must have their gaps moved to the end and
 * begin, respectively, and the intermediate bufp's must be empty.  This allows
 * bufp_p_simple_insert() to be used.  a_buf->bufp_cur must be at or after
 * a_bufp so that cached positions can be updated during insertion.
 *
 * This function is the same as buf_p_bufv_insert(), except that the data in
 * a_bufv are inserted in reverse order.  See buf_p_bufv_insert() for a more
 * detailed algorithmic explanation. */
static cw_uint64_t
buf_p_bufv_rinsert(cw_buf_t *a_buf, cw_bufp_t *a_bufp, cw_bufp_t *a_pastp,
		   const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_uint64_t nlines, bpos, line;
    cw_uint32_t i, v, cnt;
    cw_bufp_t *bufp;
    cw_bufv_t vsplit, vremain;

    cw_assert(a_bufp->bob_relative);

    /* Iterate over bufp's being inserted into.  bufp starts out pointing to the
     * original bufp on which insertion was attempted before the split. */
    nlines = 0;
    v = a_bufvcnt - 1;
    vremain.len = 0;
    for (bufp = a_bufp, bpos = bufp->bpos, line = bufp->line;
	 bufp != a_pastp;
	 bufp = ql_next(&a_buf->plist, bufp, plink))
    {
	/* Insert remainder, if any. */
	if (vremain.len != 0)
	{
	    if (a_buf->bufp_size - bufp->len < vremain.len)
	    {
		vsplit.len = a_buf->bufp_size - bufp->len;
	    }
	    else
	    {
		vsplit.len = vremain.len;
	    }
	    vremain.len -= vsplit.len;
	    vsplit.data = &vremain.data[vremain.len];

	    nlines += (cw_uint64_t) bufp_p_simple_insert(bufp, &vsplit, 1,
							 vsplit.len, TRUE);

	    if (a_buf->bufp_size - bufp->len == 0 || vremain.len != 0)
	    {
		/* No more space in this bufp, or no more remaining data in
		 * bufv[v]. */
		goto CONTINUE;
	    }
	}

	/* Iterate over a_bufv elements and insert their contents into the
	 * current bufp. */
	cnt = 0;
	for (i = v; i != 0xffffffff; i--)
	{
	    if (cnt + a_bufv[i].len > a_buf->bufp_size - bufp->len)
	    {
		break;
	    }
	    cnt += a_bufv[i].len;
	}
	if (cnt != 0)
	{
	    nlines += (cw_uint64_t) bufp_p_simple_insert(bufp, &a_bufv[v],
							 v - i, cnt, TRUE);
	    v = i;
	}

	/* Split a_bufv[v] if bufp isn't full and there are more data. */
	if (a_buf->bufp_size - bufp->len != 0 && v != 0xffffffff)
	{
	    vsplit.len = a_buf->bufp_size - bufp->len;
	    vremain.len = a_bufv[v].len - vsplit.len;
	    vremain.data = a_bufv[v].data;
	    vsplit.data = &vremain.data[vremain.len];
	    v--;

	    nlines += (cw_uint64_t) bufp_p_simple_insert(bufp, &vsplit, 1,
							 vsplit.len, TRUE);
	}

	CONTINUE:
	/* Update the cached position. */
	bufp->bob_relative = TRUE;
	bufp->bpos = bpos;
	bpos += (cw_uint64_t) bufp->len;
	bufp->line = line;
	line += (cw_uint64_t) bufp->nlines;
	a_buf->bufp_cur = bufp;
    }

    return nlines;
}

static cw_sint32_t
buf_p_bufp_insert_comp(cw_bufp_t *a_a, cw_bufp_t *a_b)
{
    cw_sint32_t retval;

    if (a_b->bob_relative)
    {
	retval = 1;
    }
    else
    {
	retval = -1;
    }

    return retval;
}

/* Insert a_bufp just after a_buf->bufp_cur.
 *
 * bufv resizing must be done manually. */
static void
buf_p_bufp_insert(cw_buf_t *a_buf, cw_bufp_t *a_bufp)
{
    cw_bufp_t *next;

    /* Insert into tree. */
    rb_insert(&a_buf->ptree, a_bufp, buf_p_bufp_insert_comp, cw_bufp_t, pnode);

    /* Insert into list. */
    rb_next(&a_buf->ptree, a_bufp, cw_bufp_t, pnode, next);
    if (next != rb_tree_nil(&a_buf->ptree))
    {
	ql_before_insert(&a_buf->plist, next, a_bufp, plink);
    }
    else
    {
	ql_tail_insert(&a_buf->plist, a_bufp, plink);
    }
}

/* bufv resizing must be done manually. */
static void
buf_p_bufp_remove(cw_buf_t *a_buf, cw_bufp_t *a_bufp)
{
    rb_remove(&a_buf->ptree, a_bufp, cw_bufp_t, pnode);
    ql_remove(&a_buf->plist, a_bufp, plink);
}

/* bufv resizing must be done manually. */
static void
buf_p_bufp_splice(cw_buf_t *a_buf, cw_bufp_t *a_start, cw_bufp_t *a_end)
{
    cw_mkr_t *mkr;

    cw_assert(ql_next(&a_buf->plist, a_start, plink) == a_end);
    cw_assert(a_start == ql_prev(&a_buf->plist, a_end, plink));

//    fprintf(stderr, "%s:%d:%s() %p..%p\n", __FILE__, __LINE__, __FUNCTION__, a_start, a_end);
    /* Move a_start's gap to the end, and a_end's gap to the beginning. */
    bufp_p_gap_move(a_start, a_start->len);
    bufp_p_gap_move(a_end, 0);

    /* Copy a_end's data to a_start. */
    memcpy(&a_start->b[a_buf->bufp_size - a_end->len],
	   &a_end->b[a_buf->bufp_size - a_end->len],
	   a_end->len);

    /* Remove a_end. */
    buf_p_bufp_remove(a_buf, a_end);

    /* Move a_end's mkr's to a_start. */
    for (mkr = ql_first(&a_end->mlist);
	 mkr != NULL;
	 mkr = ql_first(&a_end->mlist))
    {
	mkr_p_remove(mkr);
	mkr->bufp = a_start;
	mkr->pline += a_start->nlines;
	rb_node_new(&a_start->mtree, mkr, mnode);
	mkr_p_insert(mkr);
    }

    /* Update a_start's internals. */
    a_start->len += a_end->len;
    a_start->nlines += a_end->nlines;

    /* Delete a_end. */
    if (a_end == a_buf->bufp_cur)
    {
	/* Move bufp_cur.  Since a_start comes before a_end, no changes to
	 * a_start's cached position are necessary. */
	a_buf->bufp_cur = a_start;
    }
    bufp_p_delete(a_end);
}

cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_uint32_t a_bufp_size, cw_opaque_alloc_t *a_alloc,
	cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc,
	void *a_arg)
{
    cw_buf_t *retval;
    cw_bufp_t *bufp;

    /* Allocate buf. */
    if (a_buf != NULL)
    {
	retval = a_buf;
#ifdef CW_DBG
 	memset(retval, 0xa5, sizeof(cw_buf_t));
#endif
	retval->alloced = FALSE;
    }
    else
    {
	retval = (cw_buf_t *) cw_opaque_alloc(a_alloc, a_arg, sizeof(cw_buf_t));
	retval->alloced = TRUE;
    }

    /* Set page size.  This is settable per buffer, so that for extremely large
     * buffers, it is possible to use a large page size that might be less than
     * ideal under normal circumstances. */
    retval->bufp_size = a_bufp_size;

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
	cw_assert(first == rb_tree_nil(&a_buf->ftree));
    }
#endif
    cw_assert(ql_first(&a_buf->flist) == NULL);
#ifdef CW_DBG
    {
	cw_ext_t *first;
	rb_first(&a_buf->rtree, rnode, first);
	cw_assert(first == rb_tree_nil(&a_buf->rtree));
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

#ifdef CW_BUF_DUMP
void
buf_dump(cw_buf_t *a_buf, const char *a_beg, const char *a_mid,
	 const char *a_end)
{
    cw_uint32_t i;
    cw_bufp_t *tbufp;
    const char *beg, *mid, *end;
    char *tbeg, *tmid, *tend;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    beg = (a_beg != NULL) ? a_beg : "";
    mid = (a_mid != NULL) ? a_mid : beg;
    end = (a_end != NULL) ? a_end : mid;

    fprintf(stderr, "%sbuf: %p\n", beg, a_buf);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> alloced: %s\n", mid,
	    a_buf->alloced ? "TRUE" : "FALSE");
    fprintf(stderr, "%s|-> alloc: %p\n", mid, a_buf->alloc);
    fprintf(stderr, "%s|-> realloc: %p\n", mid, a_buf->realloc);
    fprintf(stderr, "%s|-> dealloc: %p\n", mid, a_buf->dealloc);
    fprintf(stderr, "%s|-> arg: %p\n", mid, a_buf->arg);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> len: %llu\n", mid, a_buf->len);
    fprintf(stderr, "%s|-> nlines: %llu\n", mid, a_buf->nlines);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> ptree: ", mid);
    buf_p_ptree_dump(rb_root(&a_buf->ptree), rb_tree_nil(&a_buf->ptree));
    fprintf(stderr, "\n");
    fprintf(stderr, "%s|-> plist:", mid);
    buf_p_ql_dump(&a_buf->plist, cw_bufp_t, plink);
    fprintf(stderr, "\n");

    i = 0;
    ql_foreach(tbufp, &a_buf->plist, plink)
    {
	fprintf(stderr, "%s|\n", mid);
	asprintf(&tbeg, "%s| [%u] ", mid, i);
	bufp_p_dump(tbufp, tbeg, NULL, NULL);
	free(tbeg);
	i++;
    }

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> bufp_cur: %p\n", mid, a_buf->bufp_cur);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> bufv:", mid);
    for (i = 0; i < a_buf->bufvcnt; i++)
    {
	fprintf(stderr, " %p(%u)", a_buf->bufv[i].data, a_buf->bufv[i].len);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "%s|-> bufvcnt: %u\n", mid, a_buf->bufvcnt);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> ftree: ", mid);
    buf_p_ftree_dump(rb_root(&a_buf->ftree), rb_tree_nil(&a_buf->ftree));
    fprintf(stderr, "\n");
    fprintf(stderr, "%s|-> flist:", mid);
    buf_p_ql_dump(&a_buf->flist, cw_ext_t, rlink);
    fprintf(stderr, "\n");

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> rtree: ", mid);
    buf_p_rtree_dump(rb_root(&a_buf->rtree), rb_tree_nil(&a_buf->rtree));
    fprintf(stderr, "\n");
    fprintf(stderr, "%s|-> rlist:", mid);
    buf_p_ql_dump(&a_buf->rlist, cw_ext_t, rlink);
    fprintf(stderr, "\n");

    fprintf(stderr, "%s|\n", mid);
    if (a_buf->hist != NULL)
    {
	asprintf(&tbeg, "%s|-> ", mid);
	asprintf(&tmid, "%s|   ", mid);
	asprintf(&tend, "%sV   ", end);
	hist_dump(a_buf->hist, tbeg, tmid, tend);
	free(tbeg);
	free(tmid);
	free(tend);
    }
    else
    {
	fprintf(stderr, "%s\\-> hist: %p\n", end, a_buf->hist);
    }
}
#endif

#ifdef CW_BUF_VALIDATE
void
buf_validate(cw_buf_t *a_buf)
{
    cw_uint32_t nbufps;
    cw_uint64_t len, nlines;
    cw_bufp_t *bufp, *tbufp;
    cw_uint64_t nfexts, nrexts;
    cw_ext_t *ext, *text;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    /* Iterate through bufp's. */
    nbufps = 0;
    len = 0;
    nlines = 1;
    ql_foreach(bufp, &a_buf->plist, plink)
    {
	/* Count number of bufp's. */
	nbufps++;

	/* Make sure the bufp points to this buf. */
	cw_assert(bufp->buf == a_buf);

	/* Validate bufp. */
	bufp_p_validate(bufp);

	/* Validate consistent ordering of mtree and mlist. */
	rb_prev(&a_buf->ptree, bufp, cw_bufp_t, pnode, tbufp);
	if (tbufp == rb_tree_nil(&a_buf->ptree))
	{
	    tbufp = NULL;
	}
	cw_assert(ql_prev(&a_buf->plist, bufp, plink) == tbufp);

	rb_next(&a_buf->ptree, bufp, cw_bufp_t, pnode, tbufp);
	if (tbufp == rb_tree_nil(&a_buf->ptree))
	{
	    tbufp = NULL;
	}
	cw_assert(ql_next(&a_buf->plist, bufp, plink) == tbufp);

	/* Validate increasing order. */
	if (tbufp != NULL)
	{
	    if (bufp->bob_relative && tbufp->bob_relative)
	    {
		cw_assert(bufp->bpos + bufp->len == tbufp->bpos);
		cw_assert(bufp->line + bufp->nlines == tbufp->line);
	    }
	    else if (bufp->bob_relative == FALSE
		     && tbufp->bob_relative == FALSE)
	    {
		cw_assert(bufp->bpos - bufp->len == tbufp->bpos);
		cw_assert(bufp->line - bufp->nlines == tbufp->line);
	    }
	    else
	    {
		cw_assert(bufp->bob_relative && tbufp->bob_relative == FALSE);

		cw_assert(bufp->bpos + bufp->len - 1
			  + tbufp->bpos
			  == a_buf->len);
		cw_assert(bufp->line + bufp->nlines
		          + tbufp->line
			  == a_buf->nlines);
	    }
	}

	/* Sum bufp lengths and number of lines in order to validate buf len
	 * and nlines. */
	len += bufp->len;
	nlines += bufp->nlines;
    }

    /* Validate minimum number of bufp's (1). */
    cw_assert(nbufps >= 1);

    /* Validate bufvcnt. */
    cw_assert(a_buf->bufvcnt == 2 * nbufps);

    /* Validate len and nlines. */
    cw_assert(len == a_buf->len);
    cw_assert(nlines == a_buf->nlines);

    /* Validate consistency of bufp_cur versus that bufp's bob_relative, and the
     * previous and next bufp's' bob_relative. */
    cw_assert(a_buf->bufp_cur->bob_relative);
    if (ql_next(&a_buf->plist, a_buf->bufp_cur, plink) != NULL)
    {
	cw_assert(ql_next(&a_buf->plist, a_buf->bufp_cur, plink)->bob_relative
		  == FALSE);
    }

    /* Iterate through ext's in f-order. */
    nfexts = 0;
    ql_foreach(ext, &a_buf->flist, flink)
    {
	/* Count number of ext's, for later comparison with number in r-order.
	 * */
	nfexts++;

	/* Validate extent. */
	ext_validate(ext);

	/* Validate consistent ordering of ftree and flist. */
	rb_prev(&a_buf->ftree, ext, cw_ext_t, fnode, text);
	if (text == rb_tree_nil(&a_buf->ftree))
	{
	    text = NULL;
	}
	cw_assert(ql_prev(&a_buf->flist, ext, flink) == text);

	rb_next(&a_buf->ftree, ext, cw_ext_t, fnode, text);
	if (text == rb_tree_nil(&a_buf->ftree))
	{
	    text = NULL;
	}
	cw_assert(ql_next(&a_buf->flist, ext, flink) == text);

	/* Validate increasing order. */
	if (text != NULL)
	{
	    cw_assert(ext_p_fcomp(ext, text) == -1);
	}
    }

    /* Iterate through ext's in r-order. */
    nrexts = 0;
    ql_foreach(ext, &a_buf->rlist, rlink)
    {
	/* Count number of ext's, for later comparison with number in f-order.
	 * */
	nrexts++;

	/* Validate extent. */
	ext_validate(ext);

	/* Validate consistent ordering of rtree and rlist. */
	rb_prev(&a_buf->rtree, ext, cw_ext_t, rnode, text);
	if (text == rb_tree_nil(&a_buf->rtree))
	{
	    text = NULL;
	}
	cw_assert(ql_prev(&a_buf->rlist, ext, rlink) == text);

	rb_next(&a_buf->rtree, ext, cw_ext_t, rnode, text);
	if (text == rb_tree_nil(&a_buf->rtree))
	{
	    text = NULL;
	}
	cw_assert(ql_next(&a_buf->rlist, ext, rlink) == text);

	/* Validate increasing order. */
	if (text != NULL)
	{
	    cw_assert(ext_p_rcomp(ext, text) == -1);
	}
    }

    /* Validate equal number of ext's in f-order and r-order. */
    cw_assert(nfexts == nrexts);

    /* Validate hist, via hist_validate(). */
    if (a_buf->hist != NULL)
    {
	hist_validate(a_buf->hist, a_buf);
    }
}
#endif

/* mkr. */
static void
mkr_p_new(cw_mkr_t *a_mkr, cw_buf_t *a_buf, cw_mkro_t a_order)
{
    cw_bufp_t *bufp;

    cw_check_ptr(a_mkr);
    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    bufp = ql_first(&a_buf->plist);
#ifdef CW_DBG
    memset(a_mkr, 0xa5, sizeof(cw_mkr_t));
#endif
    a_mkr->bufp = bufp;
    a_mkr->order = a_order;
    a_mkr->ppos = bufp_p_pos_b2p(bufp, 1);
    a_mkr->pline = 0;
    rb_node_new(&bufp->mtree, a_mkr, mnode);
    ql_elm_new(a_mkr, mlink);

#ifdef CW_DBG
    a_mkr->magic = CW_MKR_MAGIC;
#endif

    mkr_p_insert(a_mkr);
}

static void
mkr_p_dup(cw_mkr_t *a_to, const cw_mkr_t *a_from, cw_mkro_t a_order)
{
    cw_check_ptr(a_to);
    cw_dassert(a_to->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_from);
    cw_dassert(a_from->magic == CW_MKR_MAGIC);
    cw_assert(a_to != a_from);
    cw_assert(a_to->bufp->buf == a_from->bufp->buf);

    mkr_p_remove(a_to);

    a_to->bufp = a_from->bufp;
    a_to->order = a_order;
    a_to->ppos = a_from->ppos;
    a_to->pline = a_from->pline;

    rb_node_new(&a_to->bufp->mtree, a_to, mnode);
    mkr_p_insert(a_to);
}

static cw_uint64_t
mkr_p_bpos(cw_mkr_t *a_mkr)
{
    return (bufp_p_bpos(a_mkr->bufp)
	    + (cw_uint64_t) bufp_p_pos_p2r(a_mkr->bufp, a_mkr->ppos));
}

static cw_uint64_t
mkr_p_line(cw_mkr_t *a_mkr)
{
    return (bufp_p_line(a_mkr->bufp) + (cw_uint64_t) a_mkr->pline);
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
    cw_assert(a_a->bufp == a_b->bufp);

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
	if (a_a->order == a_b->order)
	{
	    retval = 0;
	}
	else if (a_a->order < a_b->order)
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

    cw_assert(a_mkr->mnode.rbn_par == rb_tree_nil(&bufp->mtree));
    cw_assert(a_mkr->mnode.rbn_left == rb_tree_nil(&bufp->mtree));
    cw_assert(a_mkr->mnode.rbn_right == rb_tree_nil(&bufp->mtree));

    /* Insert into tree. */
    rb_insert(&bufp->mtree, a_mkr, mkr_p_comp, cw_mkr_t, mnode);

    /* Insert into list.  Make sure that the tree and list orders are the same.
     */
    rb_next(&bufp->mtree, a_mkr, cw_mkr_t, mnode, next);
    if (next != rb_tree_nil(&bufp->mtree))
    {
	ql_before_insert(&bufp->mlist, next, a_mkr, mlink);
    }
    else
    {
	ql_tail_insert(&bufp->mlist, a_mkr, mlink);
    }
}

static void
mkr_p_remove(cw_mkr_t *a_mkr)
{
    cw_bufp_t *bufp = a_mkr->bufp;

    rb_remove(&bufp->mtree, a_mkr, cw_mkr_t, mnode);
    ql_remove(&bufp->mlist, a_mkr, mlink);
}

/* Insert data into a single bufp.  This function assumes that the bufp
 * internals are consistent, and that the data will fit. */
static cw_uint32_t
mkr_p_simple_insert(cw_mkr_t *a_mkr, cw_bool_t a_after, const cw_bufv_t *a_bufv,
		    cw_uint32_t a_bufvcnt, cw_uint32_t a_count,
		    cw_bool_t a_reverse)
{
    cw_uint32_t nlines, bufp_size;
    cw_buf_t *buf;
    cw_bufp_t *bufp;
    cw_mkr_t *mkr;

    bufp = a_mkr->bufp;
    buf = bufp->buf;
    bufp_size = buf->bufp_size;

    cw_assert(a_count <= bufp_size - a_mkr->bufp->len);
    cw_assert(buf->bufp_cur == bufp);

    /* Move the gap. */
    bufp_p_gap_move(bufp, bufp_p_pos_p2r(bufp, a_mkr->ppos));

    /* Insert. */
    nlines = bufp_p_simple_insert(bufp, a_bufv, a_bufvcnt, a_count, a_reverse);

    /* Adjust markers at the insertion point. */
    if (a_after == FALSE)
    {
	/* Move MKRO_BEFORE markers at the insertion point in front of the data
	 * just inserted. */

	/* Skip MKRO_EITHER markers. */
	for (mkr = ql_prev(&bufp->mlist, a_mkr, mlink);
	     mkr != NULL && mkr->ppos == a_mkr->ppos
		 && mkr->order == MKRO_EITHER;
	     mkr = ql_prev(&bufp->mlist, mkr, mlink))
	{
	    /* Do nothing. */
	}

	/* Move MKRO_BEFORE markers. */
	for (;
	     mkr != NULL && mkr->ppos == a_mkr->ppos;
	     mkr = ql_prev(&bufp->mlist, mkr, mlink))
	{
	    cw_assert(mkr->order == MKRO_BEFORE);
	    mkr->ppos -= (bufp_size - bufp->len) + a_count;
	    mkr->pline -= nlines;
	}
    }
    else
    {
	cw_uint32_t ppos;

	/* Move MKRO_BEFORE and MKRO_EITHER markers at the insertion point in
	 * front of the data just inserted. */

	ppos = a_mkr->ppos;
	/* Iterate forward. */
	for (mkr = a_mkr;
	     mkr != NULL && mkr->ppos == ppos && mkr->order == MKRO_EITHER;
	     mkr = ql_next(&bufp->mlist, mkr, mlink))
	{
	    mkr->ppos -= (bufp_size - bufp->len) + a_count;
	    mkr->pline -= nlines;
	}

	/* Iterate backward. */
	for (mkr = ql_prev(&bufp->mlist, a_mkr, mlink);
	     mkr != NULL && mkr->ppos == ppos;
	     mkr = ql_prev(&bufp->mlist, mkr, mlink))
	{
	    mkr->ppos -= (bufp_size - bufp->len) + a_count;
	    mkr->pline -= nlines;
	}
    }

    return nlines;
}

static cw_uint32_t
mkr_p_before_slide_insert(cw_mkr_t *a_mkr, cw_bool_t a_after,
			  cw_bufp_t *a_prevp, const cw_bufv_t *a_bufv,
			  cw_uint32_t a_bufvcnt, cw_uint32_t a_count,
			  cw_bool_t a_reverse)
{
    cw_uint32_t nlines;
    cw_buf_t *buf;
    cw_bufp_t *bufp;
    cw_mkr_t *mkr, *mmkr;
    cw_bufv_t bufv;
    cw_uint32_t nmove, nmovelines, nslide;

    bufp = a_mkr->bufp;
    buf = bufp->buf;
    cw_assert(buf->bufp_cur == bufp);

    /* The data won't fit in this bufp, but enough data can be slid to the
     * previous bufp to make room. */

    /* Move the gap to the insertion point. */
    bufp_p_gap_move(bufp, bufp_p_pos_p2r(bufp, a_mkr->ppos));

    /* Move a_prevp's gap to the end. */
    bufp_p_gap_move(a_prevp, a_prevp->len);

    /* Copy all data that will fit to a_prevp. */
    if (buf->bufp_size - a_prevp->len >= bufp->gap_off)
    {
	/* All data can be moved to a_prevp. */
	nmove = bufp->gap_off;
    }
    else
    {
	/* Only some of the data can be moved to a_prevp. */
	nmove = buf->bufp_size - a_prevp->len;
    }
    nslide = bufp->gap_off - nmove;

    bufv.data = &bufp->b[0];
    bufv.len = nmove;
    nmovelines = bufp_p_simple_insert(a_prevp, &bufv, 1, nmove, FALSE);

    /* Move markers that belong with the data copied to a_prevp. */
    for (mkr = ql_first(&bufp->mlist);
	 mkr != NULL && mkr->ppos < nmove;
	 mkr = mmkr)
    {
	/* Get next before removing mkr. */
	mmkr = ql_next(&bufp->mlist, mkr, mlink);

	mkr_p_remove(mkr);
	mkr->bufp = a_prevp;
	mkr->ppos += a_prevp->gap_off - nmove;
	mkr->pline += a_prevp->nlines - nmovelines;
	rb_node_new(&a_prevp->mtree, mkr, mnode);
	mkr_p_insert(mkr);
    }

    if (nslide > 0)
    {
	/* Not all slid data fit in a_prevp. */

	/* Slide the remainder to the beginning of bufp. */
	memmove(&bufp->b[0], &bufp->b[nmove], nslide);

	/* Adjust bufp's internal state. */
	bufp->len -= nmove;
	bufp->nlines -= nmovelines;
	bufp->gap_off -= nmove;

	/* Adjust the ppos of markers associated with the slid data. */
	bufp_p_mkrs_ppos_adjust(bufp, -nmove, nmove, bufp->gap_off + nmove);
    }
    else
    {
	/* All slid data fit into a_prevp. */

	/* Adjust bufp's internal state. */
	bufp->len -= nmove;
	bufp->nlines -= nmovelines;
	bufp->gap_off -= nmove;
    }

    /* Adjust the line numbers for all mkr's in bufp. */
    bufp_p_mkrs_pline_adjust(bufp, -nmovelines, 0);

    /* Insert the data. */
    if (a_reverse == FALSE)
    {
	nlines = (cw_uint32_t) buf_p_bufv_insert(buf, a_prevp,
						 ql_next(&buf->plist, bufp,
							 plink),
						 a_bufv, a_bufvcnt);
    }
    else
    {
	nlines = (cw_uint32_t) buf_p_bufv_rinsert(buf, a_prevp,
						  ql_next(&buf->plist, bufp,
							  plink),
						  a_bufv, a_bufvcnt);
    }

    return nlines;
}

static cw_uint32_t
mkr_p_after_slide_insert(cw_mkr_t *a_mkr, cw_bool_t a_after, cw_bufp_t *a_nextp,
			 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt,
			 cw_uint32_t a_count, cw_bool_t a_reverse)
{
    cw_uint32_t nlines, bufp_size;
    cw_buf_t *buf;
    cw_bufp_t *bufp;
    cw_mkr_t *mkr, *mmkr;
    cw_bufv_t bufv;
    cw_uint32_t nmove, nmovelines, nslide;

    bufp = a_mkr->bufp;
    buf = bufp->buf;
    bufp_size = buf->bufp_size;
    cw_assert(buf->bufp_cur == bufp);

    /* The data won't fit in this bufp, but enough data can be slid to the next
     * bufp to make room. */

    /* Move the gap to the insertion point. */
    bufp_p_gap_move(bufp, bufp_p_pos_p2r(bufp, a_mkr->ppos));

    /* Move a_nextp's gap to the beginning. */
    bufp_p_gap_move(a_nextp, 0);

    /* Copy all data that will fit to a_nextp. */
    if (bufp_size - a_nextp->len >= bufp->len - bufp->gap_off)
    {
	/* All data can be moved to a_nextp. */
	nmove = bufp->len - bufp->gap_off;
    }
    else
    {
	/* Only some of the data can be moved to a_nextp. */
	nmove = bufp_size - a_nextp->len;
    }
    nslide = bufp->len - bufp->gap_off - nmove;

    bufv.data = &bufp->b[bufp_size - nmove];
    bufv.len = nmove;
    nmovelines = bufp_p_simple_insert(a_nextp, &bufv, 1, nmove, FALSE);

    /* Move markers that belong with the data copied to a_nextp. */
    for (mkr = ql_last(&bufp->mlist, mlink);
	 mkr != NULL && mkr->ppos >= bufp_size - nmove;
	 mkr = mmkr)
    {
	/* Get previous before removing mkr. */
	mmkr = ql_prev(&bufp->mlist, mkr, mlink);

	mkr_p_remove(mkr);
	mkr->bufp = a_nextp;
	mkr->ppos -= bufp_size - nmove;
	mkr->pline -= bufp->nlines - nmovelines;
	rb_node_new(&a_nextp->mtree, mkr, mnode);
	mkr_p_insert(mkr);
    }

    if (nslide > 0)
    {
	/* Not all slid data fit in a_nextp. */

	/* Slide the remainder to the end of bufp. */
	memmove(&bufp->b[bufp_size - nslide],
		&bufp->b[bufp->gap_off + (bufp_size - bufp->len)],
		nslide);

	/* Adjust bufp's internal state. */
	bufp->len -= nmove;
	bufp->nlines -= nmovelines;

	/* Adjust the internal state of markers associated with the slid
	 * data. */
	bufp_p_mkrs_ppos_adjust(bufp, nmove, bufp->gap_off,
				bufp_size - nmove);
    }
    else
    {
	/* All slid data fit in a_nextp. */

	/* Adjust bufp's internal state. */
	bufp->len -= nmove;
	bufp->nlines -= nmovelines;
    }

    /* Insert the data. */
    if (a_reverse == FALSE)
    {
	nlines = (cw_uint32_t) buf_p_bufv_insert(buf, bufp,
						 ql_next(&buf->plist, a_nextp,
							 plink),
						 a_bufv, a_bufvcnt);
    }
    else
    {
	nlines = (cw_uint32_t) buf_p_bufv_rinsert(buf, bufp,
						  ql_next(&buf->plist, a_nextp,
							  plink),
						  a_bufv, a_bufvcnt);
    }

    return nlines;
}

/* a_bufv won't fit in a_mkr's bufp, so split it. */
static cw_uint64_t
mkr_p_split_insert(cw_mkr_t *a_mkr, cw_bool_t a_after, const cw_bufv_t *a_bufv,
		   cw_uint32_t a_bufvcnt, cw_uint64_t a_count,
		   cw_bool_t a_reverse)
{
    cw_uint64_t nlines;
    cw_uint32_t i, nextra, bufp_size;
    cw_buf_t *buf;
    cw_bufp_t *bufp, *nextp, *pastp;
    cw_mkr_t *mkr, *mmkr;

    bufp = a_mkr->bufp;
    buf = bufp->buf;
    bufp_size = buf->bufp_size;
    cw_assert(buf->bufp_cur == bufp);

    /* Keep track of the bufp past the range of bufp's being operated on.  This
     * might be NULL, so can only be used as an iteration terminator. */
    pastp = ql_next(&buf->plist, bufp, plink);

    /* Move bufp's gap to the split point. */
    bufp_p_gap_move(bufp, bufp_p_pos_p2r(bufp, a_mkr->ppos));

    /* Create nextp and insert it just after bufp. */
    nextp = bufp_p_new(buf);
    nextp->bob_relative = TRUE;

    nextp->bpos = bufp->bpos; /* Not actual bpos, but needed for insertion. */
    nextp->line = bufp->line; /* Not actual line. */
    buf_p_bufp_insert(buf, nextp);
    cw_assert(ql_next(&buf->plist, bufp, plink) == nextp);

    /* Adjust bufp_cur. */
    buf->bufp_cur = nextp;

    /* Insert the data after bufp's gap to the same offset in nextp. */
    nextp->len = bufp->len - bufp->gap_off;
    nextp->nlines = bufp->nlines - a_mkr->pline;
    memcpy(&nextp->b[bufp->gap_off + (bufp_size - bufp->len)],
	   &bufp->b[bufp->gap_off + (bufp_size - bufp->len)],
	   nextp->len);

    /* Subtract nextp's nlines from bufp's nlines. */
    bufp->nlines -= nextp->nlines;

    /* Adjust nextp's cached position, now that the split data have been
     * moved. */
    nextp->bpos += bufp->len;
    nextp->line += bufp->nlines;

    /* Adjust bufp's len. */
    bufp->len = bufp->gap_off;

    /* Starting at the end of bufp's marker list, remove the markers and insert
     * them into nextp until all markers that are in the gap have been moved. */
    for (mkr = ql_last(&bufp->mlist, mlink);
	 mkr != NULL && mkr->ppos >= bufp->gap_off;
	 mkr = mmkr)
    {
	/* Get the previous mkr before removing mkr from the list. */
	mmkr = ql_prev(&bufp->mlist, mkr, mlink);

	mkr_p_remove(mkr);
	mkr->bufp = nextp;
	mkr->pline -= bufp->nlines;
	rb_node_new(&nextp->mtree, mkr, mnode);
	mkr_p_insert(mkr);
    }

    /* Check if splitting bufp provided enough space.  If not, calculate how
     * many more bufp's are needed, then insert them. */
    if (a_count > (cw_uint64_t) ((bufp_size - bufp->len)
				 + (bufp_size - nextp->len)))
    {
	cw_bufp_t *newp;

	/* Splitting bufp didn't provide enough space.  Calculate how many more
	 * bufp's are needed. */
	nextra = (a_count - (cw_uint64_t) ((bufp_size - bufp->len)
					   + (bufp_size - nextp->len)))
	    / (cw_uint64_t) bufp_size;

	if ((a_count - (cw_uint64_t) ((bufp_size - bufp->len)
				      + (bufp_size - nextp->len)))
	    % (cw_uint64_t) bufp_size != 0)
	{
	    nextra++;
	}

	/* Insert extra bufp's. */
	for (i = 0; i < nextra; i++)
	{
	    newp = bufp_p_new(buf);
	    newp->bob_relative = TRUE;
	    newp->bpos = bufp->bpos;
	    newp->line = bufp->line;

	    /* Temporarily set nextp->bob_relative to FALSE so that the
	     * buf_p_bufp_insert() calls will insert bufp's just before
	     * nextp. */
	    nextp->bob_relative = FALSE;
	    buf_p_bufp_insert(buf, newp);
	    cw_assert(ql_next(&buf->plist, newp, plink) == nextp);
	    nextp->bob_relative = TRUE;
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
						(buf->bufvcnt
						 + ((nextra + 1) * 2))
						* sizeof(cw_bufv_t),
						buf->bufvcnt
						* sizeof(cw_bufv_t));
    buf->bufvcnt += (nextra + 1) * 2;

    if (a_reverse == FALSE)
    {
	nlines = buf_p_bufv_insert(buf, bufp, pastp, a_bufv, a_bufvcnt);
    }
    else
    {
	nlines = buf_p_bufv_rinsert(buf, bufp, pastp, a_bufv, a_bufvcnt);
    }

    return nlines;
}

void
mkr_l_insert(cw_mkr_t *a_mkr, cw_bool_t a_record, cw_bool_t a_after,
	     const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt,
	     cw_bool_t a_reverse)
{
    cw_uint64_t cnt, nlines;
    cw_uint32_t i, bufp_size;
    cw_buf_t *buf;
    cw_bufp_t *bufp;
    cw_mkr_t *mkr, *mmkr;
    cw_ext_t *ext;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_assert(a_mkr->order == MKRO_EITHER);

    bufp = a_mkr->bufp;
    buf = bufp->buf;
    bufp_size = buf->bufp_size;

    /* Move bufp_cur. */
    buf_p_bufp_cur_set(buf, bufp);

    /* Record the undo information before inserting so that the position is
     * still unmodified. */
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
	cnt += (cw_uint64_t) a_bufv[i].len;
    }

    /* Find all the zero-length extents that will grow to non-zero length,
     * remove them from the extent trees/lists, and hold on to them so that they
     * can be re-inserted after the data insertion is complete. */

    /* Initialize the extent list. */
    ql_new(&buf->elist);

    /* Iterate forward. */
    for (mkr = ql_next(&bufp->mlist, a_mkr, mlink);
	 mkr != NULL && mkr->ppos == a_mkr->ppos;
	 mkr = mmkr)
    {
	cw_assert(mkr->order != MKRO_BEFORE);

	/* Get the next mkr before potentially removing mkr from the list. */
	mmkr = ql_next(&bufp->mlist, mkr, mlink);

	if (mkr->order == MKRO_AFTER && mkr->ext_end == FALSE)
	{
	    /* Get a pointer to the container extent. */
	    ext = (cw_ext_t *) ((void *) mkr - cw_offsetof(cw_ext_t, beg));
	    cw_dassert(ext->magic == CW_EXT_MAGIC);

	    if (ext->beg.bufp == ext->end.bufp
		&& ext->beg.ppos == ext->end.ppos)
	    {
		/* Remove the extent. */
		ext_p_remove(ext);

		/* Store the extent in the list for later re-insertion. */
		qr_remove(ext, elink);
		ql_tail_insert(&buf->elist, ext, elink);
	    }
	}
    }

    /* Iterate backward. */
    for (mkr = ql_prev(&bufp->mlist, a_mkr, mlink);
	 mkr != NULL && mkr->ppos == a_mkr->ppos;
	 mkr = mmkr)
    {
	cw_assert(mkr->order != MKRO_AFTER);

	/* Get the previous mkr before potentially removing mkr from the
	 * list. */
	mmkr = ql_prev(&bufp->mlist, mkr, mlink);

	if (mkr->order == MKRO_BEFORE && mkr->ext_end == FALSE)
	{
	    /* Get a pointer to the container extent. */
	    ext = (cw_ext_t *) ((void *) mkr - cw_offsetof(cw_ext_t, beg));
	    cw_dassert(ext->magic == CW_EXT_MAGIC);

	    if (ext->beg.bufp == ext->end.bufp
		&& ext->beg.ppos == ext->end.ppos)
	    {
		/* Remove the extent. */
		ext_p_remove(ext);

		/* Store the extent in the list for later re-insertion. */
		qr_remove(ext, elink);
		ql_tail_insert(&buf->elist, ext, elink);
	    }
	}
    }

    /* Depending on how much data are to be inserted, there are three
     * different algorithms: simple, slide, and split. */
    if (cnt <= bufp_size - bufp->len)
    {
	nlines = (cw_uint64_t) mkr_p_simple_insert(a_mkr, a_after, a_bufv,
						   a_bufvcnt,
						   (cw_uint32_t) cnt,
						   a_reverse);
    }
    else
    {
	cw_bufp_t *prevp, *nextp;

	prevp = ql_prev(&buf->plist, bufp, plink);
	nextp = ql_next(&buf->plist, bufp, plink);

	/* Try sliding backward, then forward.  If neither of the slides would
	 * make enough room, splitting is guaranteed not to violate the
	 * requirement that any two consecutive bufps must be on average more
	 * than 25% full.  Thus, there is never a need to do bufp coalescing
	 * after insertion. */
	if (prevp != NULL && cnt <= (bufp_size - bufp->len)
	    + (bufp_size - prevp->len))
	{
	    nlines = (cw_uint64_t) mkr_p_before_slide_insert(a_mkr, a_after,
							     prevp, a_bufv,
							     a_bufvcnt,
							     (cw_uint32_t) cnt,
							     a_reverse);
	}
	else if (nextp != NULL && cnt <= (bufp_size - bufp->len)
		 + (bufp_size - nextp->len))
	{
	    nlines = (cw_uint64_t) mkr_p_after_slide_insert(a_mkr, a_after,
							    nextp, a_bufv,
							    a_bufvcnt,
							    (cw_uint32_t) cnt,
							    a_reverse);
	}
	else
	{
	    nlines = mkr_p_split_insert(a_mkr, a_after, a_bufv, a_bufvcnt, cnt,
					a_reverse);
	}

	/* Adjust markers at the insertion point. */
	bufp = a_mkr->bufp;
	if (a_after == FALSE)
	{
	    /* Move MKRO_BEFORE markers at the insertion point in front of the
	     * data just inserted. */

	    /* Skip MKRO_EITHER markers. */
	    for (mkr = ql_prev(&bufp->mlist, a_mkr, mlink);
		 mkr != NULL && mkr->ppos == a_mkr->ppos
		     && mkr->order == MKRO_EITHER;
		 mkr = ql_prev(&bufp->mlist, mkr, mlink))
	    {
		/* Do nothing. */
	    }

	    if (mkr != NULL && mkr->ppos == a_mkr->ppos)
	    {
		/* Move MKRO_BEFORE markers. */

		/* Get the previous mkr before removing mkr from the list. */
		mmkr = ql_prev(&bufp->mlist, mkr, mlink);

		/* Seek.  Subsequent moves can be done via dup, which saves
		 * looking up the bufp to move the mkr to. */
		mkr_seek(mkr, -(cw_sint64_t) cnt, BUFW_REL);

		for (mkr = mmkr;
		     mkr != NULL && mkr->ppos == a_mkr->ppos;
		     mkr = mmkr)
		{
		    cw_assert(mkr->order == MKRO_BEFORE);

		    /* Get the previous mkr before removing mkr from the
		     * list. */
		    mmkr = ql_prev(&bufp->mlist, mkr, mlink);

		    /* Move mkr. */
		    mkr_p_dup(mkr, a_mkr, mkr->order);
		}
	    }
	}
	else
	{
	    cw_uint32_t ppos;
	    cw_mkr_t *prev;

	    /* Move MKRO_BEFORE and MKRO_EITHER markers at the insertion point
	     * in front of the data just inserted. */

	    /* Get the previous mkr before removing a_mkr from the list. */
	    prev = ql_prev(&bufp->mlist, a_mkr, mlink);

	    /* Get the next mkr before removing a_mkr from the list. */
	    mmkr = ql_next(&bufp->mlist, a_mkr, mlink);

	    ppos = a_mkr->ppos;

	    /* Seek.  Subsequent moves can be done via dup, which saves looking
	     * up the bufp to move the mkr to. */
	    mkr_seek(a_mkr, -(cw_sint64_t) cnt, BUFW_REL);

	    /* Iterate forward. */
	    for (mkr = mmkr;
		 mkr != NULL && mkr->ppos == ppos && mkr->order == MKRO_EITHER;
		 mkr = mmkr)
	    {
		/* Get the next mkr before removing mkr from the list. */
		mmkr = ql_next(&bufp->mlist, mkr, mlink);

		/* Move mkr. */
		mkr_p_dup(mkr, a_mkr, mkr->order);
	    }

	    /* Iterate backward. */
	    for (mkr = prev;
		 mkr != NULL && mkr->ppos == ppos;
		 mkr = mmkr)
	    {
		cw_assert(mkr->order != MKRO_AFTER);

		/* Get the previous mkr before removing mkr from the list. */
		mmkr = ql_prev(&bufp->mlist, mkr, mlink);

		/* Move mkr. */
		mkr_p_dup(mkr, a_mkr, mkr->order);
	    }
	}
    }

    buf->len += cnt;
    buf->nlines += (cw_uint64_t) nlines;

    /* Re-insert the extents that grew from zero length. */
    ql_foreach(ext, &buf->elist, elink)
    {
	ext_p_insert(ext);
    }
}

void
mkr_l_remove(cw_mkr_t *a_start, cw_mkr_t *a_end, cw_bool_t a_record)
{
    cw_buf_t *buf;
    cw_bufp_t *bufp, *nextp, *pastp;
    cw_mkr_t *start, *end, *mkr, *prev;
    cw_ext_t *ext;
    cw_uint32_t nrem = 0, bufp_size;
    cw_uint64_t start_bpos, end_bpos, rcount;

    cw_check_ptr(a_start);
    cw_dassert(a_start->magic == CW_MKR_MAGIC);
    cw_assert(a_start->order == MKRO_EITHER);
    cw_check_ptr(a_start->bufp);
    cw_check_ptr(a_end);
    cw_dassert(a_end->magic == CW_MKR_MAGIC);
    cw_assert(a_end->order == MKRO_EITHER);
    cw_check_ptr(a_end->bufp);
    cw_assert(a_start->bufp->buf == a_end->bufp->buf);
    cw_dassert(a_start->bufp->magic == CW_BUFP_MAGIC);

    /* Get bpos for start and end, since they are used more than once. */
    start_bpos = mkr_pos(a_start);
    end_bpos = mkr_pos(a_end);

    /* Determine which mkr actually comes first. */
    if (start_bpos < end_bpos)
    {
	start = a_start;
	end = a_end;
    }
    else if (start_bpos > end_bpos)
    {
	cw_uint64_t tbpos;

	start = a_end;
	end = a_start;

	tbpos = start_bpos;
	start_bpos = end_bpos;
	end_bpos = tbpos;
    }
    else
    {
	/* No data need to be removed. */
	return;
    }

    bufp = start->bufp;
    buf = bufp->buf;
    bufp_size = buf->bufp_size;

    /* Move bufp_cur. */
    buf_p_bufp_cur_set(buf, bufp);

    /* Calculate the number of elements being removed, since it is used more
     * than once. */
    rcount = end_bpos - start_bpos;

    /* Record undo information.  The ordering of a_start and a_end determines
     * whether this is a before/after removal. */
    if (buf->hist != NULL && a_record)
    {
	cw_uint32_t bufvcnt;

	mkr_range_get(start, end, &bufvcnt);

	if (start == a_start)
	{
	    hist_del(buf->hist, buf, start_bpos, buf->bufv, bufvcnt);
	}
	else
	{
	    hist_rem(buf->hist, buf, end_bpos, buf->bufv, bufvcnt);
	}
    }

    /* Remove data. */
    if (start->bufp == end->bufp)
    {
	cw_uint32_t gap_end, nlines;

	/* All data to be removed are in the same bufp. */

	/* Move gap to just before the data to be removed, then grow the gap. */
	bufp_p_gap_move(bufp, bufp_p_pos_p2r(bufp, start->ppos));
	bufp->len -= (cw_uint32_t) rcount;
	buf->len -= rcount;
	gap_end = bufp->gap_off + (bufp_size - bufp->len);

	/* Adjust pline for mkr's after the gap. */
	nlines = end->pline - start->pline;
	if (nlines > 0)
	{
	    bufp->nlines -= nlines;
	    buf->nlines -= (cw_uint64_t) nlines;

	    for (mkr = ql_last(&bufp->mlist, mlink);
		 mkr != NULL && mkr->ppos >= gap_end;
		 mkr = ql_prev(&bufp->mlist, mkr, mlink))
	    {
		mkr->pline -= nlines;
	    }
	}

	/* Move mkr's that are in the gap and adjust their pline, iterating
	 * forward, then backward. */
	prev = ql_prev(&bufp->mlist, start, mlink);
	for (mkr = start;
	     mkr != NULL && mkr->ppos < gap_end;
	     mkr = ql_next(&bufp->mlist, mkr, mlink))
	{
	    mkr->ppos = gap_end;
	    mkr->pline = start->pline;
	}
	for (mkr = prev;
	     mkr != NULL && mkr->ppos >= bufp->gap_off;
	     mkr = ql_prev(&bufp->mlist, mkr, mlink))
	{
	    mkr->ppos = gap_end;
	    mkr->pline = start->pline;
	}
    }
    else
    {
	cw_bufp_t *prevp;
	cw_uint32_t nlines;

	/* Data are to be removed from two or more bufp's.  Work backwards
	 * through the affected bufp's.  This order reduces the amount of work
	 * necessary in processing mkr's. */

	/*
	 * Last bufp.
	 */
	bufp = end->bufp;

	/* Move the gap just before end, then grow it backward to the beginning
	 * of bufp. */
	bufp_p_gap_move(bufp, bufp_p_pos_p2r(bufp, end->ppos));
	bufp->len -= bufp->gap_off;
	buf->len -= (cw_uint64_t) bufp->gap_off;
	cw_assert(bufp->bob_relative == FALSE);
	bufp->bpos -= (cw_uint64_t) bufp->gap_off;
	bufp->gap_off = 0;

	/* Adjust pline for mkr's at or after end. */
	nlines = end->pline;
	if (nlines > 0)
	{
	    bufp->nlines -= nlines;
	    buf->nlines -= (cw_uint64_t) nlines;
	    bufp->line -= (cw_uint64_t) nlines;

	    for (mkr = ql_last(&bufp->mlist, mlink);
		 mkr != NULL && mkr->ppos >= end->ppos;
		 mkr = ql_prev(&bufp->mlist, mkr, mlink))
	    {
		mkr->pline -= nlines;
	    }
	}

	/* Move mkr's that are in the gap and and adjust their pline. */
	for (mkr = ql_first(&bufp->mlist);
	     /* mkr != NULL && */ mkr->ppos < end->ppos;
	     mkr = ql_next(&bufp->mlist, mkr, mlink))
	{
	    mkr->ppos = end->ppos;
	    mkr->pline = end->pline;
	}

	/*
	 * Intermediate bufp's.
	 */
	for (bufp = ql_prev(&buf->plist, bufp, plink);
	     bufp != start->bufp;
	     bufp = prevp)
	{
	    cw_check_ptr(bufp);

	    /* Get the previous bufp before deleting this one. */
	    prevp = ql_prev(&buf->plist, bufp, plink);

	    /* Move mkr's. */
	    for (mkr = ql_first(&bufp->mlist);
		 mkr != NULL;
		 mkr = ql_first(&bufp->mlist))
	    {
		mkr_p_dup(mkr, end, mkr->order);
	    }

	    buf->len -= (cw_uint64_t) bufp->len;
	    buf->nlines -= (cw_uint64_t) bufp->nlines;

	    /* Remove and delete bufp. */
	    buf_p_bufp_remove(buf, bufp);
	    bufp_p_delete(bufp);
	    nrem++;
	}

	/*
	 * First bufp.
	 */

	/* Move the gap to just before the data to be removed, then grow the gap
	 * to the end of the bufp. */
	bufp_p_gap_move(bufp, bufp_p_pos_p2r(bufp, start->ppos));
	buf->len -= (cw_uint64_t) (bufp->len - bufp->gap_off);
	bufp->len -= bufp->len - bufp->gap_off;
	buf->nlines -= (cw_uint64_t) bufp->nlines - start->pline;
	bufp->nlines = start->pline;

	/* Move mkr's that are in the gap. */
	for (mkr = ql_last(&bufp->mlist, mlink);
	     mkr != NULL && mkr->ppos >= bufp->gap_off;
	     mkr = ql_last(&bufp->mlist, mlink))
	{
	    mkr_p_dup(mkr, end, mkr->order);
	}
    }

    /* Try to coalesce.  Start at the bufp preceding the first one affected by
     * the removal, then iteratively splice under-full pages until after the
     * bufp following the last one affected is considered.
     */
    bufp = ql_prev(&buf->plist, buf->bufp_cur, plink);
    if (bufp == NULL)
    {
	bufp = buf->bufp_cur;
    }
    /* Set pastp to point to the first bufp that shouldn't be considered, and
     * terminate the loop as soon as nextp reaches it. */
    pastp = ql_next(&buf->plist, end->bufp, plink);
    if (pastp != NULL)
    {
	pastp = ql_next(&buf->plist, pastp, plink);
    }
    for (nextp = ql_next(&buf->plist, bufp, plink);
	 nextp != pastp;
	 bufp = nextp, nextp = ql_next(&buf->plist, bufp, plink))
    {
	while (bufp->len + nextp->len <= bufp_size / 2
	       || bufp->len == 0
	       || nextp->len == 0)
	{
	    /* Splice bufp and nextp. */
	    buf_p_bufp_splice(buf, bufp, nextp);
	    nrem++;
	    nextp = ql_next(&buf->plist, bufp, plink);
	    if (nextp == pastp)
	    {
		goto DONE;
	    }
	}
    }
    DONE:

    /* Move bufp_cur, now that there is no danger of encountering a zero length
     * bufp (unless the buf is empty, in which case it doesn't matter). */
    buf_p_bufp_cur_set(buf, end->bufp);

    /* Resize bufv if any bufp's were removed. */
    if (nrem != 0)
    {
	buf->bufv = (cw_bufv_t *) cw_opaque_realloc(buf->realloc, buf->bufv,
						    buf->arg,
						    (buf->bufvcnt - (nrem * 2))
						    * sizeof(cw_bufv_t),
						    buf->bufvcnt
						    * sizeof(cw_bufv_t));
	buf->bufvcnt -= (nrem * 2);
    }

    /* Detach any detachable extents that just shrank to zero length.  Starting
     * at a_start, iterate forward, then backward.  Only consider markers that
     * are the beginning of extents; this still catches every zero-length
     * extent, and it saves some wasted effort for non-detachable extents and
     * non-zero-length extents. */
    bufp = a_start->bufp;

    /* Iterate forward. */
    for (mkr = ql_next(&bufp->mlist, a_start, mlink);
	 mkr != NULL && mkr->ppos == a_start->ppos;
	 mkr = ql_next(&bufp->mlist, mkr, mlink))
    {
	cw_assert(mkr->order != MKRO_BEFORE);

	if (mkr->order == MKRO_AFTER && mkr->ext_end == FALSE)
	{
	    /* Get a pointer to the container extent. */
	    ext = (cw_ext_t *) ((void *) mkr - cw_offsetof(cw_ext_t, beg));
	    cw_dassert(ext->magic == CW_EXT_MAGIC);

	    if (ext->detachable
		&& ext->beg.bufp == ext->end.bufp
		&& ext->beg.ppos == ext->end.ppos)
	    {
		/* Detatch the extent. */
		ext->attached = FALSE;
		ext_p_remove(ext);
		mkr_p_remove(&ext->beg);
		mkr_p_remove(&ext->end);
	    }
	}
    }

    /* Iterate backward. */
    for (mkr = ql_prev(&bufp->mlist, a_start, mlink);
	 mkr != NULL && mkr->ppos == a_start->ppos;
	 mkr = ql_prev(&bufp->mlist, mkr, mlink))
    {
	cw_assert(mkr->order != MKRO_AFTER);

	if (mkr->order == MKRO_BEFORE && mkr->ext_end == FALSE)
	{
	    /* Get a pointer to the container extent. */
	    ext = (cw_ext_t *) ((void *) mkr - cw_offsetof(cw_ext_t, beg));
	    cw_dassert(ext->magic == CW_EXT_MAGIC);

	    if (ext->detachable
		&& ext->beg.bufp == ext->end.bufp
		&& ext->beg.ppos == ext->end.ppos)
	    {
		/* Detatch the extent. */
		ext->attached = FALSE;
		ext_p_remove(ext);
		mkr_p_remove(&ext->beg);
		mkr_p_remove(&ext->end);
	    }
	}
    }
}

void
mkr_new(cw_mkr_t *a_mkr, cw_buf_t *a_buf)
{
    cw_check_ptr(a_mkr);
    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    mkr_p_new(a_mkr, a_buf, MKRO_EITHER);
}

void
mkr_dup(cw_mkr_t *a_to, const cw_mkr_t *a_from)
{
    mkr_p_dup(a_to, a_from, a_to->order);
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
    cw_uint64_t bpos, line;
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
		line = 1;
		bpos = 1;
	    }
	    else if (a_offset >= buf->nlines)
	    {
		/* Attempt to move to or past EOB.  Move to EOB. */
		line = buf->nlines;
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
		line = a_offset;
		bpos = buf_p_bpos_before_lf(buf, line, &bufp);
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
		    line = buf->nlines;
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
		    line = mkr_p_line(a_mkr) + a_offset - 1;
		    bpos = buf_p_bpos_before_lf(buf, line, &bufp);
		}
	    }
	    else if (a_offset < 0)
	    {
		if (-a_offset >= mkr_p_line(a_mkr))
		{
		    /* Attempt to move to or before BOB.  Move to BOB. */
		    line = 1;
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
		    line = mkr_p_line(a_mkr) + a_offset + 1;
		    bpos = buf_p_bpos_after_lf(buf, line - 1, &bufp);
		}
	    }
	    else
	    {
		/* Do nothing. */
		line = mkr_p_line(a_mkr);
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
		line = buf->nlines;
		bpos = buf->len + 1;
	    }
	    else if (-a_offset >= buf->nlines)
	    {
		/* Attempt to move to or past BOB.  Move to BOB. */
		line = 1;
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
		line = buf->nlines + a_offset + 1;
		bpos = buf_p_bpos_after_lf(buf, line - 1, &bufp);
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
    a_mkr->ppos = bufp_p_pos_b2p(bufp, bpos);
    a_mkr->pline = line - bufp_p_line(bufp);
    rb_node_new(&bufp->mtree, a_mkr, mnode);

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
    cw_uint32_t ppos, pline, bufp_size;
    cw_buf_t *buf;
    cw_bufp_t *bufp;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    buf = a_mkr->bufp->buf;
    bufp_size = buf->bufp_size;

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

    /* Determine the new values for a_mkr's internals. */
    bufp = buf_p_bufp_at_bpos(buf, bpos);
    ppos = bufp_p_pos_b2p(bufp, bpos);
    /* pline. */
    if (bufp == a_mkr->bufp)
    {
	cw_uint32_t rpos, orpos;

	orpos = bufp_p_pos_p2r(bufp, a_mkr->ppos);
	rpos = bufp_p_pos_p2r(bufp, ppos);

	/* If the distance between the old position and the new position isn't
	 * larger than the distance from the new position to either end of the
	 * bufp, calculate pline relative to the previous position. */
	if (a_offset <= rpos && a_offset <= bufp_size - rpos)
	{
	    cw_uint32_t lppos, hppos, i, nlines;

	    /* Determine low and high end of range. */
	    if (ppos > a_mkr->ppos)
	    {
		lppos = a_mkr->ppos;
		hppos = ppos;
	    }
	    else
	    {
		lppos = ppos;
		hppos = a_mkr->ppos;
	    }

	    /* Count newlines in range. */
	    nlines = 0;
	    if (hppos < bufp->gap_off
		|| lppos >= bufp->gap_off + (bufp_size - bufp->len))
	    {
		/* All data before or after the gap. */
		for (i = lppos; i < hppos; i++)
		{
		    if (bufp->b[i] == '\n')
		    {
			nlines++;
		    }
		}
	    }
	    else
	    {
		/* Data before and after the gap. */
		for (i = lppos; i < bufp->gap_off; i++)
		{
		    if (bufp->b[i] == '\n')
		    {
			nlines++;
		    }
		}

		for (i = bufp->gap_off + (bufp_size - bufp->len);
		     i < hppos;
		     i++)
		{
		    if (bufp->b[i] == '\n')
		    {
			nlines++;
		    }
		}
	    }

	    /* Finally, set pline. */
	    if (ppos > a_mkr->ppos)
	    {
		pline = a_mkr->pline + nlines;
	    }
	    else
	    {
		pline = a_mkr->pline - nlines;
	    }
	}
	else
	{
	    /* Relative calculation of pline isn't worthwhile. */
	    pline = bufp_p_ppos2pline(bufp, ppos);
	}
    }
    else
    {
	/* Determine pline without any attempt to use the previous position to
	 * speed things up. */
	pline = bufp_p_ppos2pline(bufp, ppos);
    }

    /* Move a_mkr, potentially to a different bufp. */
    mkr_p_remove(a_mkr);
    a_mkr->bufp = bufp;
    a_mkr->ppos = ppos;
    a_mkr->pline = pline;
    rb_node_new(&bufp->mtree, a_mkr, mnode);
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
    cw_buf_t *buf;
    cw_bufp_t *bufp;
    cw_uint32_t rpos;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    bufp = a_mkr->bufp;
    buf = bufp->buf;

    /* Determine offset of a_mkr. */
    rpos = bufp_p_pos_p2r(bufp, a_mkr->ppos);

    if (rpos == 0)
    {
	if ((bufp = ql_prev(&buf->plist, bufp, plink)) == NULL)
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
		retval = &bufp->b[buf->bufp_size - 1];
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
	rpos--;
	if (rpos < bufp->gap_off)
	{
	    /* Before gap. */
	    retval = &bufp->b[rpos];
	}
	else
	{
	    /* After gap. */
	    retval = &bufp->b[rpos + (buf->bufp_size - bufp->len)];
	}
    }

    return retval;
}

cw_uint8_t *
mkr_after_get(const cw_mkr_t *a_mkr)
{
    cw_uint8_t *retval;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    if (a_mkr->ppos == a_mkr->bufp->buf->bufp_size)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = &a_mkr->bufp->b[a_mkr->ppos];
    RETURN:
    return retval;
}

cw_bufv_t *
mkr_range_get(const cw_mkr_t *a_start, const cw_mkr_t *a_end,
	      cw_uint32_t *r_bufvcnt)
{
    cw_bufv_t *retval;
    cw_uint32_t bufvcnt;
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

    bufvcnt = 0;
    switch (alg)
    {
	case 1:
	{
	    /* Zero-length range. */
	    retval = NULL;
	    break;
	}
	case 2:
	{
	    cw_bufp_t *bufp = start->bufp;

	    /* One bufp involved. */
	    retval = bufp->buf->bufv;

	    if (start->ppos < bufp->gap_off
		&& end->ppos >= bufp->gap_off
		+ (bufp->buf->bufp_size - bufp->len))
	    {
		/* Two ranges. */
		retval[bufvcnt].data = &bufp->b[start->ppos];
		retval[bufvcnt].len = bufp->gap_off - start->ppos;
		bufvcnt++;

		if (end->ppos > bufp->gap_off
		    + (bufp->buf->bufp_size - bufp->len))
		{
		    /* Data after gap. */
		    retval[bufvcnt].data
			= &bufp->b[bufp->gap_off
				  + (bufp->buf->bufp_size - bufp->len)];
		    retval[bufvcnt].len
			= end->ppos - (bufp->gap_off
				       + (bufp->buf->bufp_size - bufp->len));
		    bufvcnt++;
		}
	    }
	    else
	    {
		/* One range. */
		retval[0].data = &bufp->b[start->ppos];
		retval[0].len = end->ppos - start->ppos;
		bufvcnt++;
	    }
	    break;
	}
	case 3:
	{
	    cw_buf_t *buf;
	    cw_bufp_t *bufp;

	    /* Two or more bufp's involved. */
	    buf = start->bufp->buf;
	    retval = buf->bufv;

	    /* First bufp. */
	    bufp = start->bufp;
	    if (start->ppos < bufp->gap_off)
	    {
		/* Before gap. */
		retval[bufvcnt].data = &bufp->b[start->ppos];
		retval[bufvcnt].len = bufp->gap_off - start->ppos;
		bufvcnt++;

		if (bufp->gap_off < bufp->len)
		{
		    /* Data after gap. */
		    retval[bufvcnt].data
			= &bufp->b[bufp->gap_off
				  + (bufp->buf->bufp_size - bufp->len)];
		    retval[bufvcnt].len = bufp->len - bufp->gap_off;
		    bufvcnt++;
		}
	    }
	    else
	    {
		/* After gap. */
		if (start->ppos < bufp->buf->bufp_size)
		{
		    retval[bufvcnt].data = &bufp->b[start->ppos];
		    retval[bufvcnt].len = bufp->buf->bufp_size - start->ppos;
		    bufvcnt++;
		}
	    }

	    /* Intermediate bufp's. */
	    for (bufp = ql_next(&buf->plist, bufp, plink);
		 bufp != end->bufp;
		 bufp = ql_next(&buf->plist, bufp, plink))
	    {
		cw_check_ptr(bufp);

		if (bufp->gap_off > 0)
		{
		    /* Data before gap. */
		    retval[bufvcnt].data = &bufp->b[0];
		    retval[bufvcnt].len = bufp->gap_off;
		    bufvcnt++;
		}

		if (bufp->gap_off < bufp->len)
		{
		    /* Data after gap. */
		    retval[bufvcnt].data
			= &bufp->b[bufp->gap_off
				  + (bufp->buf->bufp_size - bufp->len)];
		    retval[bufvcnt].len = bufp->len - bufp->gap_off;
		    bufvcnt++;
		}
	    }

	    /* Last bufp. */
	    if (end->ppos < end->bufp->gap_off)
	    {
		/* Before gap. */
		if (end->ppos > 0)
		{
		    retval[bufvcnt].data = &bufp->b[0];
		    retval[bufvcnt].len = end->ppos;
		    bufvcnt++;
		}
	    }
	    else
	    {
		/* After gap. */
		if (bufp->gap_off > 0)
		{
		    /* Data before gap. */
		    retval[bufvcnt].data = &bufp->b[0];
		    retval[bufvcnt].len = bufp->gap_off;
		    bufvcnt++;
		}

		if (bufp->gap_off < bufp->len
		    && end->ppos > bufp->gap_off
		    + (bufp->buf->bufp_size - bufp->len))
		{
		    /* Data after gap. */
		    retval[bufvcnt].data
			= &bufp->b[bufp->gap_off
				  + (bufp->buf->bufp_size - bufp->len)];
		    retval[bufvcnt].len
			= end->ppos
			- (bufp->gap_off + (bufp->buf->bufp_size - bufp->len));
		    bufvcnt++;
		}
	    }

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

#ifdef CW_DBG
    {
	cw_uint32_t i;

	for (i = 0; i < bufvcnt; i++)
	{
	    cw_assert(retval[i].len != 0);
	}
    }
#endif
    *r_bufvcnt = bufvcnt;
    return retval;
}

void
mkr_before_insert(cw_mkr_t *a_mkr, const cw_bufv_t *a_bufv,
		   cw_uint32_t a_bufvcnt)
{
    mkr_l_insert(a_mkr, TRUE, FALSE, a_bufv, a_bufvcnt, FALSE);
}

void
mkr_after_insert(cw_mkr_t *a_mkr, const cw_bufv_t *a_bufv,
		  cw_uint32_t a_bufvcnt)
{
    mkr_l_insert(a_mkr, TRUE, TRUE, a_bufv, a_bufvcnt, FALSE);
}

void
mkr_remove(cw_mkr_t *a_start, cw_mkr_t *a_end)
{
    mkr_l_remove(a_start, a_end, TRUE);
}

#ifdef CW_BUF_DUMP
void
mkr_dump(cw_mkr_t *a_mkr, const char *a_beg, const char *a_mid,
	 const char *a_end)
{
    const char *beg, *mid, *end;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    beg = (a_beg != NULL) ? a_beg : "";
    mid = (a_mid != NULL) ? a_mid : beg;
    end = (a_end != NULL) ? a_end : mid;

    fprintf(stderr, "%smkr: %p\n", beg, a_mkr);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> bufp: %p\n", mid, a_mkr->bufp);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> order: %s\n", mid,
	    (a_mkr->order == MKRO_BEFORE) ? "MKRO_BEFORE" :
	    (a_mkr->order == MKRO_EITHER) ? "MKRO_EITHER" : "MKRO_AFTER");
    if (a_mkr->order != MKRO_EITHER)
    {
	fprintf(stderr, "%s|-> ext_end: %s\n", mid,
		a_mkr->ext_end ? "TRUE" : "FALSE");
    }
    fprintf(stderr, "%s|-> ppos: %u\n", mid, a_mkr->ppos);
    fprintf(stderr, "%s\\-> pline: %u\n", end, a_mkr->pline);
}
#endif

#ifdef CW_BUF_VALIDATE
void
mkr_validate(cw_mkr_t *a_mkr)
{
    /* Validate consistency between order and ext_end. */
    if (a_mkr->order != MKRO_EITHER)
    {
	cw_ext_t *ext;

	if (a_mkr->ext_end == FALSE)
	{
	    /* Begin of extent. */
	    ext = (cw_ext_t *)(a_mkr - cw_offsetof(cw_ext_t, beg));
	}
	else
	{
	    /* End of extext. */
	    ext = (cw_ext_t *)(a_mkr - cw_offsetof(cw_ext_t, end));
	}

	cw_check_ptr(ext);
	cw_dassert(ext->magic == CW_EXT_MAGIC);
    }

    /* Validate that ppos isn't in the gap. */
    cw_assert(a_mkr->ppos <= a_mkr->bufp->gap_off
	      || (a_mkr->ppos >= a_mkr->bufp->gap_off
		  + (a_mkr->bufp->buf->bufp_size - a_mkr->bufp->len))
	      || (a_mkr->ppos == a_mkr->bufp->buf->bufp_size
		  && a_mkr->bufp == ql_last(&a_mkr->bufp->buf->plist, plink)));

    /* Validate pline. */
    cw_assert(bufp_p_ppos2pline(a_mkr->bufp, a_mkr->ppos) == a_mkr->pline);
}
#endif

/* ext. */
static cw_sint32_t
ext_p_fcomp(cw_ext_t *a_a, cw_ext_t *a_b)
{
    cw_sint32_t retval;
    cw_uint64_t abeg, bbeg;

    cw_check_ptr(a_a);
    cw_dassert(a_a->magic == CW_EXT_MAGIC);
    cw_check_ptr(a_a->beg.bufp->buf);
    cw_check_ptr(a_b);
    cw_dassert(a_b->magic == CW_EXT_MAGIC);
    cw_check_ptr(a_b->beg.bufp->buf);
    cw_assert(a_a->beg.bufp->buf == a_b->beg.bufp->buf);

    abeg = mkr_pos(&a_a->beg);
    bbeg = mkr_pos(&a_b->beg);
    if (abeg < bbeg)
    {
	retval = -1;
    }
    else if (abeg == bbeg)
    {
	cw_uint64_t aend, bend;

	aend = mkr_pos(&a_a->end);
	bend = mkr_pos(&a_b->end);
	if (aend > bend)
	{
	    retval = -1;
	}
	else if (aend == bend)
	{
	    retval = 0;
	}
	else
	{
	    retval = 1;
	}
    }
    else
    {
	retval = 1;
    }

    return retval;
}

static cw_sint32_t
ext_p_rcomp(cw_ext_t *a_a, cw_ext_t *a_b)
{
    cw_sint32_t retval;
    cw_uint64_t aend, bend;

    cw_check_ptr(a_a);
    cw_dassert(a_a->magic == CW_EXT_MAGIC);
    cw_check_ptr(a_a->beg.bufp->buf);
    cw_check_ptr(a_b);
    cw_dassert(a_b->magic == CW_EXT_MAGIC);
    cw_check_ptr(a_b->beg.bufp->buf);
    cw_assert(a_a->beg.bufp->buf == a_b->beg.bufp->buf);

    aend = mkr_pos(&a_a->end);
    bend = mkr_pos(&a_b->end);
    if (aend < bend)
    {
	retval = -1;
    }
    else if (aend == bend)
    {
	cw_uint64_t abeg, bbeg;

	abeg = mkr_pos(&a_a->beg);
	bbeg = mkr_pos(&a_b->beg);
	if (abeg > bbeg)
	{
	    retval = -1;
	}
	else if (abeg == bbeg)
	{
	    retval = 0;
	}
	else
	{
	    retval = 1;
	}
    }
    else
    {
	retval = 1;
    }

    return retval;
}

static void
ext_p_insert(cw_ext_t *a_ext)
{
    cw_buf_t *buf = a_ext->beg.bufp->buf;
    cw_ext_t *next;

    cw_assert(a_ext->fnode.rbn_par == rb_tree_nil(&buf->ftree));
    cw_assert(a_ext->fnode.rbn_left == rb_tree_nil(&buf->ftree));
    cw_assert(a_ext->fnode.rbn_right == rb_tree_nil(&buf->ftree));
    cw_assert(a_ext->rnode.rbn_par == rb_tree_nil(&buf->rtree));
    cw_assert(a_ext->rnode.rbn_left == rb_tree_nil(&buf->rtree));
    cw_assert(a_ext->rnode.rbn_right == rb_tree_nil(&buf->rtree));

    /* Insert into ftree. */
    rb_insert(&buf->ftree, a_ext, ext_p_fcomp, cw_ext_t, fnode);

    /* Insert into flist.  Make sure that the tree and list orders are the
     * same. */
    rb_next(&buf->ftree, a_ext, cw_ext_t, fnode, next);
    if (next != rb_tree_nil(&buf->ftree))
    {
	ql_before_insert(&buf->flist, next, a_ext, flink);
    }
    else
    {
	ql_tail_insert(&buf->flist, a_ext, flink);
    }

    /* Insert into rtree. */
    rb_insert(&buf->rtree, a_ext, ext_p_rcomp, cw_ext_t, rnode);

    /* Insert into rlist.  Make sure that the tree and list orders are the
     * same. */
    rb_next(&buf->rtree, a_ext, cw_ext_t, rnode, next);
    if (next != rb_tree_nil(&buf->rtree))
    {
	ql_before_insert(&buf->rlist, next, a_ext, rlink);
    }
    else
    {
	ql_tail_insert(&buf->rlist, a_ext, rlink);
    }
}

static void
ext_p_remove(cw_ext_t *a_ext)
{
    cw_buf_t *buf = a_ext->beg.bufp->buf;

    rb_remove(&buf->ftree, a_ext, cw_ext_t, fnode);
    ql_remove(&buf->flist, a_ext, flink);

    rb_remove(&buf->rtree, a_ext, cw_ext_t, rnode);
    ql_remove(&buf->rlist, a_ext, rlink);
}

cw_ext_t *
ext_new(cw_ext_t *a_ext, cw_buf_t *a_buf)
{
    cw_ext_t *retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    /* Allocate ext. */
    if (a_ext != NULL)
    {
	retval = a_ext;
#ifdef CW_DBG
 	memset(retval, 0xa5, sizeof(cw_ext_t));
#endif
	retval->alloced = FALSE;
    }
    else
    {
	retval = (cw_ext_t *) cw_opaque_alloc(a_buf->alloc, a_buf->arg,
					      sizeof(cw_ext_t));
	retval->alloced = TRUE;
    }

    /* Initialize detach state. */
    retval->attached = FALSE;
    retval->detachable = FALSE;

    /* Initialize markers.  Extents start out as zero-length shut-shut. */
    mkr_p_new(&retval->beg, a_buf, MKRO_BEFORE);
    retval->beg.ext_end = FALSE;
    mkr_p_new(&retval->end, a_buf, MKRO_AFTER);
    retval->end.ext_end = TRUE;

    /* Initialize extent tree and list linkage. */
    rb_node_new(&a_buf->ftree, retval, fnode);
    ql_elm_new(retval, flink);
    rb_node_new(&a_buf->rtree, retval, rnode);
    ql_elm_new(retval, rlink);

    /* Initialize extent stack linkage. */
    ql_elm_new(retval, elink);

#ifdef CW_DBG
    retval->magic = CW_EXT_MAGIC;
#endif

    return retval;
}

void
ext_dup(cw_ext_t *a_to, cw_ext_t *a_from)
{
    cw_check_ptr(a_to);
    cw_dassert(a_to->magic == CW_EXT_MAGIC);
    cw_check_ptr(a_from);
    cw_dassert(a_from->magic == CW_EXT_MAGIC);
    cw_assert(a_to != a_from);
    cw_assert(a_to->beg.bufp->buf == a_from->beg.bufp->buf);

    if (a_to->attached)
    {
	ext_p_remove(a_to);
    }

    a_to->attached = a_from->attached;
    a_to->detachable = a_from->detachable;
    mkr_dup(&a_to->beg, &a_from->beg);
    mkr_dup(&a_to->end, &a_from->beg);

    if (a_to->attached)
    {
	ext_p_insert(a_to);
    }
}

void
ext_delete(cw_ext_t *a_ext)
{
    cw_buf_t *buf;

    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    buf = a_ext->beg.bufp->buf;

    if (a_ext->attached)
    {
	ext_p_remove(a_ext);
    }
    mkr_delete(&a_ext->beg);
    mkr_delete(&a_ext->end);

    /* Remove this extent from any ring it may be in.  This makes it possible to
     * unconditionally insert and remove extents in the extent stack without
     * worries of trying to access extents that have already been deleted. */
    qr_remove(a_ext, elink);

    if (a_ext->alloced)
    {
	cw_opaque_dealloc(buf->dealloc, buf->arg, a_ext, sizeof(cw_ext_t));
    }
#ifdef CW_DBG
    else
    {
	memset(a_ext, 0x5a, sizeof(cw_ext_t));
    }
#endif
}

cw_buf_t *
ext_buf(const cw_ext_t *a_ext)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    return a_ext->beg.bufp->buf;
}

const cw_mkr_t *
ext_beg_get(cw_ext_t *a_ext)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    return &a_ext->beg;
}

void
ext_beg_set(cw_ext_t *a_ext, const cw_mkr_t *a_beg)
{
    cw_uint64_t bbpos, ebpos;

    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->attached)
    {
	ext_p_remove(a_ext);
    }

    ebpos = mkr_pos(&a_ext->end);
    bbpos = mkr_pos(a_beg);
    if (ebpos < bbpos)
    {
	/* Convert to zero-length, since the beginning is being moved after the
	 * end. */
	mkr_p_dup(&a_ext->end, a_beg, a_ext->end.order);
	ebpos = bbpos;
    }
    mkr_p_dup(&a_ext->beg, a_beg, a_ext->beg.order);

    if (a_ext->attached)
    {
	if (bbpos == ebpos && a_ext->detachable)
	{
	    /* Detach. */
	    a_ext->attached = FALSE;
	}
	else
	{
	    ext_p_insert(a_ext);
	}
    }
}

const cw_mkr_t *
ext_end_get(cw_ext_t *a_ext)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    return &a_ext->end;
}

void
ext_end_set(cw_ext_t *a_ext, const cw_mkr_t *a_end)
{
    cw_uint64_t bbpos, ebpos;

    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->attached)
    {
	ext_p_remove(a_ext);
    }

    bbpos = mkr_pos(&a_ext->beg);
    ebpos = mkr_pos(a_end);
    if (bbpos > ebpos)
    {
	/* Convert to zero-length, since the end is being moved before the
	 * beginning. */
	mkr_p_dup(&a_ext->beg, a_end, a_ext->beg.order);
	bbpos = ebpos;
    }
    mkr_p_dup(&a_ext->end, a_end, a_ext->end.order);

    if (a_ext->attached)
    {
	if (bbpos == ebpos && a_ext->detachable)
	{
	    /* Detach. */
	    a_ext->attached = FALSE;
	}
	else
	{
	    ext_p_insert(a_ext);
	}
    }
}

cw_bool_t
ext_beg_open_get(const cw_ext_t *a_ext)
{
    cw_bool_t retval;

    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->beg.order == MKRO_AFTER)
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
ext_beg_open_set(cw_ext_t *a_ext, cw_bool_t a_beg_open)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->beg.order != a_beg_open)
    {
	mkr_p_remove(&a_ext->beg);
	a_ext->beg.order = a_beg_open;
	mkr_p_insert(&a_ext->beg);
    }
}

cw_bool_t
ext_end_open_get(const cw_ext_t *a_ext)
{
    cw_bool_t retval;

    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->end.order == MKRO_BEFORE)
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
ext_end_open_set(cw_ext_t *a_ext, cw_bool_t a_end_open)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->end.order != a_end_open)
    {
	mkr_p_remove(&a_ext->end);
	a_ext->end.order = a_end_open;
	mkr_p_insert(&a_ext->end);
    }
}

cw_bool_t
ext_attached_get(const cw_ext_t *a_ext)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    return a_ext->attached;
}

void
ext_attach(cw_ext_t *a_ext)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->attached == FALSE)
    {
	ext_p_insert(a_ext);
	a_ext->attached = TRUE;
    }
}

void
ext_detach(cw_ext_t *a_ext)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->attached)
    {
	ext_p_remove(a_ext);
	a_ext->attached = FALSE;
    }
}

cw_bool_t
ext_detachable_get(const cw_ext_t *a_ext)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    return a_ext->detachable;
}

void
ext_detachable_set(cw_ext_t *a_ext, cw_bool_t a_detachable)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    if (a_ext->detachable == FALSE
	&& a_detachable
	&& mkr_pos(&a_ext->beg) == mkr_pos(&a_ext->end))
    {
	/* This extent meets the conditions for detachment, and is being set
	 * detachable.  Detach now. */
	ext_p_remove(a_ext);
    }

    a_ext->detachable = a_detachable;
}

/* Create the stack of extents that overlap the character either before or after
 * a_mkr, which can then be iterated on by ext_stack_down_get().  The stack is
 * in f-order, starting at the top of the stack. */
cw_uint32_t
ext_stack_init(const cw_mkr_t *a_mkr, cw_bool_t a_after)
{
    cw_uint32_t retval = 0;
    cw_ext_t *ext, *eext, key;
    cw_buf_t *buf;
    cw_bufp_t *bufp;
    cw_mkr_t tmkr;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);

    bufp = a_mkr->bufp;
    buf = bufp->buf;

    /* Initialize the extent stack. */
    ql_new(&buf->elist);

    /* Fake up a key for extent searching. */
    mkr_p_new(&tmkr, buf, MKRO_EITHER);
    mkr_dup(&tmkr, a_mkr);
    if (a_after == FALSE)
    {
	mkr_seek(&tmkr, -1LL, BUFW_REL);
	memcpy(&key.beg, &tmkr, sizeof(cw_mkr_t));
	memcpy(&key.end, a_mkr, sizeof(cw_mkr_t));
    }
    else
    {
	mkr_seek(&tmkr, 1LL, BUFW_REL);
	memcpy(&key.beg, a_mkr, sizeof(cw_mkr_t));
	memcpy(&key.end, &tmkr, sizeof(cw_mkr_t));
    }
#ifdef CW_DBG
    key.magic = CW_EXT_MAGIC;
#endif
    mkr_delete(&tmkr);

    /* Find the first extent in f-order. */
    rb_nsearch(&buf->ftree, &key, ext_p_fcomp, cw_ext_t, fnode, ext);
    if (ext != rb_tree_nil(&buf->ftree))
    {
	/* Iteratively add extents to the bottom of the stack. */
	for (;
	     ext != NULL
		 && ext->beg.bufp == bufp
		 && ext->beg.ppos == a_mkr->ppos;
	     ext = ql_next(&buf->flist, ext, flink))
	{
	    qr_remove(ext, elink);
	    ql_tail_insert(&buf->elist, ext, elink);
	    retval++;
	}
    }

    /* Find the first extent in r-order. */
    rb_nsearch(&buf->rtree, &key, ext_p_rcomp, cw_ext_t, rnode, ext);
    if (ext != rb_tree_nil(&buf->rtree))
    {
	cw_uint64_t bpos, ebpos, tbpos;

	/* Iteratively insert extents into the stack. */
	bpos = mkr_pos(a_mkr);
	for (;
	     ext != NULL
		 && (ebpos = mkr_pos(&ext->beg)) <= bpos;
	     ext = ql_next(&buf->rlist, ext, rlink))
	{
	    ql_remove(&buf->elist, ext, elink);
	    for (eext = ql_first(&buf->elist);
		 eext != NULL
		     && (((tbpos = mkr_pos(&eext->beg)) < ebpos)
			 || (tbpos == ebpos
			     && mkr_pos(&eext->end) < mkr_pos(&ext->end)));
		 eext = ql_next(&buf->elist, eext, elink))
	    {
		/* Skip. */
	    }

	    /* Insert. */
	    if (eext != NULL)
	    {
		ql_before_insert(&buf->elist, eext, ext, elink);
	    }
	    else
	    {
		ql_tail_insert(&buf->elist, ext, elink);
	    }
	    retval++;
	}
    }

    return retval;
}

/* Get the beginning and ending points of the fragment that a_mkr is contained
 * by.  If a_mkr falls on a fragment boundary, then it is considered to reside
 * in a zero-length fragment. */
void
ext_frag_get(const cw_mkr_t *a_mkr, cw_mkr_t *r_beg, cw_mkr_t *r_end)
{
    cw_buf_t *buf;
    cw_mkr_t *beg_mkr, *end_mkr;
    cw_ext_t *ext, key;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_assert(a_mkr->order == MKRO_EITHER);
    cw_check_ptr(r_beg);
    cw_dassert(r_beg->magic == CW_MKR_MAGIC);
    cw_assert(r_beg->order == MKRO_EITHER);
    cw_check_ptr(r_end);
    cw_dassert(r_end->magic == CW_MKR_MAGIC);
    cw_assert(r_end->order == MKRO_EITHER);

    /* Use f-order and r-order to find the nearest extent begins/ends on either
     * side of a_mkr.  rb_nsearch() makes it easy to find the following begin
     * and leading end.  In the case that a_mkr is not on a fragment boundary,
     * rb_nsearch() is guaranteed to return the first extent past a_mkr, which
     * means that the extent immediatly preceding rb_nsearch()'s return value is
     * the other extent of interest.  In the case where a_mkr is on a fragment
     * boundary, there is no need to even look further, since we know this to be
     * the case as soon as an extent's marker is found to be at a_mkr. */

    buf = a_mkr->bufp->buf;

    /* Fake up a key for extent searching. */
    memcpy(&key.beg, a_mkr, sizeof(cw_mkr_t));
    memcpy(&key.end, a_mkr, sizeof(cw_mkr_t));
#ifdef CW_DBG
    key.magic = CW_EXT_MAGIC;
#endif

    /* Find the first extent in f-order that starts after a_mkr. */
    rb_nsearch(&buf->ftree, &key, ext_p_fcomp, cw_ext_t, fnode, ext);
    if (ext != rb_tree_nil(&buf->ftree))
    {
	end_mkr = &ext->beg;

	if (r_end->bufp == a_mkr->bufp && r_end->ppos == a_mkr->ppos)
	{
	    /* Zero-length fragment. */
	    beg_mkr = end_mkr;
	    goto RETURN;
	}

	ext = ql_prev(&buf->flist, ext, flink);
	if (ext != NULL)
	{
	    beg_mkr = &ext->beg;
	}
	else
	{
	    end_mkr = NULL;
	}
    }
    else
    {
	end_mkr = NULL;
    }

    /* Find the first extent in r-order that ends before a_mkr. */
    rb_nsearch(&buf->rtree, &key, ext_p_rcomp, cw_ext_t, rnode, ext);
    if (ext != rb_tree_nil(&buf->rtree))
    {
	if (beg_mkr == NULL || mkr_p_bpos(&ext->end) > mkr_p_bpos(beg_mkr))
	{
	    beg_mkr = &ext->end;
	}

	if (beg_mkr->bufp == a_mkr->bufp && beg_mkr->ppos == a_mkr->ppos)
	{
	    /* Zero-length fragment. */
	    end_mkr = beg_mkr;
	    goto RETURN;
	}

	ext = ql_prev(&buf->rlist, ext, rlink);
	if (ext != NULL &&mkr_p_bpos(&ext->end) < mkr_p_bpos(end_mkr))
	{
	    end_mkr = beg_mkr;
	}
    }

    RETURN:
    mkr_p_dup(r_beg, beg_mkr, MKRO_EITHER);
    mkr_p_dup(r_end, end_mkr, MKRO_EITHER);
}

#ifdef CW_BUF_DUMP
void
ext_dump(cw_ext_t *a_ext, const char *a_beg, const char *a_mid,
	 const char *a_end)
{
    const char *beg, *mid, *end;
    char *tbeg, *tmid, *tend;

    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    beg = (a_beg != NULL) ? a_beg : "";
    mid = (a_mid != NULL) ? a_mid : beg;
    end = (a_end != NULL) ? a_end : mid;

    fprintf(stderr, "%sext: %p\n", beg, a_ext);

    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> alloced: %s\n", mid,
	    a_ext->alloced ? "TRUE" : "FALSE");
    fprintf(stderr, "%s|-> detachable: %s\n", mid,
	    a_ext->detachable ? "TRUE" : "FALSE");
    fprintf(stderr, "%s|-> attached: %s\n", mid,
	    a_ext->attached ? "TRUE" : "FALSE");

    fprintf(stderr, "%s|\n", mid);
    asprintf(&tbeg, "%s|-> beg: ", mid);
    asprintf(&tmid, "%s|        ", mid);
    mkr_dump(&a_ext->beg, tbeg, tmid, NULL);
    free(tbeg);
    fprintf(stderr, "%s|\n", mid);
    asprintf(&tbeg, "%s|-> end: ", mid);
    asprintf(&tend, "%sV        ", end);
    mkr_dump(&a_ext->end, tbeg, tmid, tend);
    free(tbeg);
    free(tmid);
    free(tend);
}
#endif

#ifdef CW_BUF_VALIDATE
void
ext_validate(cw_ext_t *a_ext)
{
    cw_check_ptr(a_ext);
    cw_dassert(a_ext->magic == CW_EXT_MAGIC);

    /* Validate consistency of attached and detachable. */
    if (a_ext->detachable == FALSE)
    {
	cw_assert(a_ext->attached);
    }
    else
    {
	if (mkr_pos(&a_ext->beg) == mkr_pos(&a_ext->end))
	{
	    cw_assert(a_ext->attached == FALSE);
	}
	else
	{
	    cw_assert(a_ext->attached);
	}
    }

    /* Validate beg and end. */
    mkr_validate(&a_ext->beg);
    mkr_validate(&a_ext->end);
}
#endif
