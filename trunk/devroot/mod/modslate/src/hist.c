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
 * Undo : Meaning
 * =====:============================================
 * B    : Group begin.
 * E    : Group end.
 * (n)P : Buffer position change (64 bits, unsigned).
 * (s)I : Insert string s before point.
 * (s)Y : Insert (yank) string s after point.
 * (s)R : Remove string s before point.
 * (s)K : Remove (kill) string s after point.
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
 * *) Current position in the data buffer.
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
 *   (hello)R
 *   (hel)RI[lo]
 *   I[hello]
 *
 * Following are several examples of history logs and translations of their
 * meanings.  The /\ characters denote the current position (hcur):
 *
 * Example:
 *   (3)P(hello)R(olleh)I(salutations)R
 *                                    /\
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
 *   B(42)P(This is a string.)YEB(It gets replaced by this one.)KE
 *                                                               /\
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
 *   (This string is more than 32 char)R(acters long.)R
 *                                                    /\
 *
 * Translation:
 *   The user typed in a string that was long enough to require a new log
 *   header.  Logically, this is no different than if the entire string could
 *   have been encoded as a single record.
 *
 * Example:
 *   B(42)P(Hello)RB(Goodbye)RE(Really)REE
 *                                       /\
 *
 * Translation:
 *   Through some programmatic means, the user inserted three strings in such a
 *   way that nested groups were created.  The entire record is undone in a
 *   single step.
 *
 * Example:
 *   B(42)P(Hello)R
 *                /\
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
 *     E(42)P(Hello)R
 *                  /\
 *
 * Example:
 *   (Hello)RI(Goodbye)
 *          /\
 *
 * Translation:
 *   The user typed "HelloGoodbye", then undid "Goodbye" so that only "Hello"
 *   remains.
 *
 * Example:
 *   (Hello)RR(olleH)
 *          /\
 *
 * Translation:
 *   The user typed "Hello", then backspaced through it, then undid the
 *   backspacing so that the end buffer contents are "Hello".
 *
 * Example:
 *  (Hell)RI(o)R(olleH)
 *        /\
 *
 * Translation:
 *   The user typed "Hello", then backspaced through it, then undid the
 *   backspacing and the "o", so that the end buffer contents are "Hell".
 *
 ******************************************************************************/

#include "../include/modslate.h"
#include "../include/buf_l.h"
#ifndef HAVE_ASPRINTF
#include "../../../lib/libonyx/src/asprintf.c"
#endif
#ifdef CW_BUF_DUMP
#include <ctype.h>
#endif

/* The upper 3 bits are used to denote the record type, and the lower 5 bits are
 * used to record the number of characters for insert/delete.  5 bits isn't
 * much, but it makes for very compact storage in the common case, and doesn't
 * waste much space in the worst case.  In order to make the most of the 5 bits,
 * they are interpreted to encode the numbers 1 to 32, since 0 is never needed.
 *
 * The tag numbering is fragile, since bitwise xor is used in
 * hst_tag_inverse(). */
#define HST_TAG_MASK    0xe0
#define HST_CNT_MASK (~HST_TAG_MASK)
#define HST_CNT_MAX 32
#define HST_TAG_GRP_BEG 0x20
#define HST_TAG_GRP_END 0x40
#define HST_TAG_POS     0x60
#define HST_TAG_INS     0x80
#define HST_TAG_YNK     0xa0
#define HST_TAG_REM     0xc0
#define HST_TAG_DEL     0xe0

#define hst_tag_get(a_hdr) ((a_hdr) & HST_TAG_MASK)
#define hst_tag_set(a_hdr, a_tag)					\
    (a_hdr) = ((a_hdr) & HST_CNT_MASK) | (a_tag)
/* Only meant for HST_TAG_{INS,YNK,REM,DEL}. */
#define hst_tag_inverse(a_hdr) ((a_hdr) ^ 0x40)
#define hst_cnt_get(a_hdr) (((a_hdr) & HST_CNT_MASK) + 1)
#define hst_cnt_set(a_hdr, a_cnt)					\
    (a_hdr) = ((a_hdr) & HST_TAG_MASK) | ((a_cnt) - 1)

CW_P_INLINE void
hist_p_redo_flush(cw_hist_t *a_hist)
{
    /* Flush redo state, if any. */
    if (mkr_pos(&a_hist->hcur) != buf_len(&a_hist->h) + 1)
    {
	mkr_seek(&a_hist->htmp, 0, BUFW_EOB);
	mkr_remove(&a_hist->hcur, &a_hist->htmp);
    }
}

CW_P_INLINE void
hist_p_pos(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos)
{
    cw_uint8_t hdr;
    union
    {
	cw_uint64_t bpos;
	cw_uint8_t str[8];
    } u;
    cw_bufv_t bufv;

    cw_assert(&a_hist->h != NULL);
    cw_assert(mkr_pos(&a_hist->hcur) == buf_len(&a_hist->h) + 1);
    cw_assert(a_bpos != a_hist->hbpos);

    if (a_hist->hbpos == 0)
    {
	/* There's no need for an initial position record. */
	a_hist->hbpos = a_bpos;
	goto RETURN;
    }

    /* Old position. */
    u.bpos = cw_htonq(a_hist->hbpos);
    bufv.data = u.str;
    bufv.len = sizeof(u.bpos);
    mkr_before_insert(&a_hist->hcur, &bufv, 1);
    /* Record header. */
    hst_tag_set(hdr, HST_TAG_POS);
    bufv.data = &hdr;
    bufv.len = sizeof(hdr);
    mkr_before_insert(&a_hist->hcur, &bufv, 1);

    /* Update hbpos now that the history record is complete. */
    a_hist->hbpos = a_bpos;

    RETURN:
}

cw_hist_t *
hist_new(cw_opaque_alloc_t *a_alloc, cw_opaque_realloc_t *a_realloc,
	 cw_opaque_dealloc_t *a_dealloc, void *a_arg)
{
    cw_hist_t *retval;

    retval = (cw_hist_t *) cw_opaque_alloc(a_alloc, a_arg, sizeof(cw_hist_t));
    buf_new(&retval->h, a_alloc, a_realloc, a_dealloc, a_arg);
    mkr_new(&retval->hcur, &retval->h);
    mkr_new(&retval->htmp, &retval->h);
    retval->hbpos = 0;
    retval->gdepth = 0;
    retval->dealloc = a_dealloc;
    retval->arg = a_arg;
#ifdef CW_DBG
    retval->magic = CW_HIST_MAGIC;
#endif

    return retval;
}

void
hist_delete(cw_hist_t *a_hist)
{
    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);

    mkr_delete(&a_hist->hcur);
    mkr_delete(&a_hist->htmp);
    buf_delete(&a_hist->h);
    cw_opaque_dealloc(a_hist->dealloc, a_hist->arg, a_hist, sizeof(cw_hist_t));
}

cw_bool_t
hist_undoable(const cw_hist_t *a_hist, const cw_buf_t *a_buf)
{
    cw_bool_t retval;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);

    /* There is at least one undoable operation unless hcur is at BOB. */
    if (mkr_pos(&a_hist->hcur) == 1)
    {
	retval = FALSE;
    }
    else
    {
	retval = TRUE;
    }

    return retval;
}

cw_bool_t
hist_redoable(const cw_hist_t *a_hist, const cw_buf_t *a_buf)
{
    cw_bool_t retval;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);

    /* There is at least one redoable operation unless hcur is at EOB. */
    if (mkr_pos(&a_hist->hcur) == buf_len(&a_hist->h) + 1)
    {
	retval = FALSE;
    }
    else
    {
	retval = TRUE;
    }

    return retval;
}

cw_uint64_t
hist_undo(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_mkr_t *a_mkr,
	  cw_uint64_t a_count)
{
    cw_uint64_t retval;
    cw_bool_t undid;
    cw_uint8_t *p, uhdr, rhdr, c;
    cw_mkr_t tmkr;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    mkr_new(&tmkr, a_buf);

    if (mkr_pos(&a_hist->hcur) == 1)
    {
	retval = 0;
	goto RETURN;
    }

    /* Iteratively undo a_count times. */
    for (retval = 0; retval < a_count; retval++)
    {
	/* Undo one character at a time (at least one character total) until the
	 * group depth is zero or the entire history has been undone. */
	for (undid = FALSE;
	     (a_hist->gdepth != 0 || undid == FALSE)
		 && mkr_pos(&a_hist->hcur) > 1;)
	{
	    p = mkr_before_get(&a_hist->hcur);
	    uhdr = *p;
	    switch (hst_tag_get(uhdr))
	    {
		case HST_TAG_INS:
		case HST_TAG_YNK:
		case HST_TAG_REM:
		case HST_TAG_DEL:
		{
		    /* Read (or synthesize) the undo header, redo header, and
		     * character, then remove them from the history buffer.
		     * Then re-insert them in their new order and state.
		     *
		     * There are four possible variations.  In all cases, hcur
		     * is positioned before the character, and htmp is
		     * positioned after the redo header (if it exists and is
		     * being re-used).  This allows an unconditional call to
		     * mkr_remove().
		     *
		     * rhdr's count is updated before the mkr_remove(), whereas
		     * uhdr's count is updated later.  This is necessary because
		     * the count can never be 0.
		     *
		     * There are more efficient ways of doing this, but it
		     * requires more special case code. */

		    /* Set rhdr and move htmp to the appropriate position. */
		    mkr_dup(&a_hist->htmp, &a_hist->hcur);
		    if (mkr_pos(&a_hist->hcur) != buf_len(&a_hist->h) + 1)
		    {
			p = mkr_after_get(&a_hist->htmp);
			rhdr = *p;
			if (hst_tag_get(rhdr)
			    == hst_tag_inverse(hst_tag_get(uhdr))
			    && hst_cnt_get(rhdr) < HST_CNT_MAX)
			{
			    hst_cnt_set(rhdr, hst_cnt_get(rhdr) + 1);
			    mkr_seek(&a_hist->htmp, 1, BUFW_REL);
			}
			else
			{
			    /* Synthesize a redo header. */
			    hst_tag_set(rhdr, hst_tag_inverse(uhdr));
			    hst_cnt_set(rhdr, 1);
			}
		    }
		    else
		    {
			/* Synthesize a redo header. */
			hst_tag_set(rhdr,
				    hst_tag_inverse(uhdr));
			hst_cnt_set(rhdr, 1);
		    }

		    /* Set c and move hcur to the appropriate position. */
		    mkr_seek(&a_hist->hcur, -2, BUFW_REL);
		    p = mkr_after_get(&a_hist->hcur);
		    c = *p;

		    /* Remove the character and header(s). */
		    mkr_remove(&a_hist->hcur, &a_hist->htmp);

		    /* Insert the character. */
		    bufv.data = &c;
		    bufv.len = sizeof(c);
		    mkr_after_insert(&a_hist->hcur, &bufv, 1);
		    /* Insert the redo header. */
		    bufv.data = &rhdr;
		    bufv.len = sizeof(rhdr);
		    mkr_after_insert(&a_hist->hcur, &bufv, 1);
		    /* Insert the undo header if necessary. */
		    if (hst_cnt_get(uhdr) > 1)
		    {
			hst_cnt_set(uhdr, hst_cnt_get(uhdr) - 1);
			bufv.data = &uhdr;
			bufv.len = sizeof(uhdr);
			mkr_before_insert(&a_hist->hcur, &bufv, 1);
		    }

		    /* Actually take action, now that the log is updated. */
		    mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);
		    switch (hst_tag_get(uhdr))
		    {
			case HST_TAG_INS:
			{
			    bufv.data = &c;
			    bufv.len = sizeof(c);
			    mkr_l_insert(a_mkr, FALSE, FALSE, &bufv, 1);
			    a_hist->hbpos++;
			    break;
			}
			case HST_TAG_REM:
			{
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, -1, BUFW_REL);
			    mkr_l_remove(&tmkr, a_mkr, FALSE);
			    a_hist->hbpos--;
			    break;
			}
			case HST_TAG_YNK:
			{
			    bufv.data = &c;
			    bufv.len = sizeof(c);
			    mkr_l_insert(a_mkr, FALSE, TRUE, &bufv, 1);
			    break;
			}
			case HST_TAG_DEL:
			{
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, 1, BUFW_REL);
			    mkr_l_remove(a_mkr, &tmkr, FALSE);
			    break;
			}
			default:
			{
			    cw_not_reached();
			}
		    }

		    undid = TRUE;
		    break;
		}
		case HST_TAG_GRP_BEG:
		{
		    a_hist->gdepth--;
		    mkr_seek(&a_hist->hcur, -1, BUFW_REL);
		    break;
		}
		case HST_TAG_GRP_END:
		{
		    a_hist->gdepth++;
		    mkr_seek(&a_hist->hcur, -1, BUFW_REL);
		    break;
		}
		case HST_TAG_POS:
		{
		    cw_uint64_t from;
		    union
		    {
			cw_uint64_t bpos;
			cw_uint8_t str[8];
		    } u;
		    cw_bufv_t *bufv, pbufv;
		    cw_uint32_t bufvcnt;

		    /* Set up the bufv. */
		    pbufv.data = u.str;
		    pbufv.len = 8;

		    /* Read the history record. */
		    mkr_dup(&a_hist->htmp, &a_hist->hcur);
		    mkr_seek(&a_hist->htmp, -9, BUFW_REL);
		    bufv = mkr_range_get(&a_hist->htmp, &a_hist->hcur,
					  &bufvcnt);

		    /* Swap from and to. */
		    bufv_copy(&pbufv, 1, bufv, bufvcnt, 0);
		    from = a_hist->hbpos;
		    a_hist->hbpos = cw_ntohq(u.bpos);
		    u.bpos = cw_htonq(from);

		    /* Invert the history record. */
		    mkr_seek(&a_hist->htmp, 1, BUFW_REL);
		    bufv = mkr_range_get(&a_hist->htmp, &a_hist->hcur,
					  &bufvcnt);
		    mkr_seek(&a_hist->htmp, -1, BUFW_REL);
		    bufv_copy(bufv, bufvcnt, &pbufv, 1, 0);
		    p = mkr_after_get(&a_hist->htmp);
		    *p = uhdr;
		    mkr_dup(&a_hist->hcur, &a_hist->htmp);

		    /* Move. */
		    mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);

		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }
	}
    }

    RETURN:
    mkr_delete(&tmkr);
    return retval;
}

cw_uint64_t
hist_redo(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_mkr_t *a_mkr,
	  cw_uint64_t a_count)
{
    cw_uint64_t retval;
    cw_bool_t redid;
    cw_uint8_t *p, uhdr, rhdr, c;
    cw_mkr_t tmkr;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    mkr_new(&tmkr, a_buf);

    if (mkr_pos(&a_hist->hcur) == buf_len(&a_hist->h) + 1)
    {
	retval = 0;
	goto RETURN;
    }

    /* Iteratively redo a_count times. */
    for (retval = 0; retval < a_count; retval++)
    {
	/* Undo one character at a time (at least one character total) until the
	 * group depth is zero or the entire history has been undone. */
	for (redid = FALSE;
	     (a_hist->gdepth != 0 || redid == FALSE)
		 && mkr_pos(&a_hist->hcur) < buf_len(&a_hist->h) + 1;)
	{
	    p = mkr_after_get(&a_hist->hcur);
	    rhdr = *p;
	    switch (hst_tag_get(rhdr))
	    {
		case HST_TAG_INS:
		case HST_TAG_YNK:
		case HST_TAG_REM:
		case HST_TAG_DEL:
		{
		    /* Read (or synthesize) the redo header, undo header, and
		     * character, then remove them from the history buffer.
		     * Then re-insert them in their new order and state.
		     *
		     * There are four possible variations.  In all cases, hcur
		     * is positioned after the character, and htmp is positioned
		     * before the undo header (if it exists and is being
		     * re-used).  This allows an unconditional call to
		     * mkr_remove().
		     *
		     * uhdr's count is updated before the mkr_remove(), whereas
		     * rhdr's count is updated later.  This is necessary because
		     * the count can never be 0 (only 1..32 are representable).
		     *
		     * There are more efficient ways of doing this, but it
		     * requires more special case code. */

		    /* Set uhdr and move htmp to the appropriate position. */
		    mkr_dup(&a_hist->htmp, &a_hist->hcur);
		    if (mkr_pos(&a_hist->hcur) != 1)
		    {
			p = mkr_before_get(&a_hist->htmp);
			uhdr = *p;
			if (hst_tag_get(uhdr)
			    == hst_tag_inverse(hst_tag_get(rhdr))
			    && hst_cnt_get(rhdr) < HST_CNT_MAX)
			{
			    hst_cnt_set(uhdr, hst_cnt_get(uhdr) + 1);
			    mkr_seek(&a_hist->htmp, -1, BUFW_REL);
			}
			else
			{
			    /* Synthesize an undo header. */
			    hst_tag_set(uhdr, hst_tag_inverse(rhdr));
			    hst_cnt_set(uhdr, 1);
			}
		    }
		    else
		    {
			/* Synthesize an undo header. */
			hst_tag_set(uhdr, hst_tag_inverse(rhdr));
			hst_cnt_set(uhdr, 1);
		    }

		    /* Set c and move hcur to the appropriate position. */
		    mkr_seek(&a_hist->hcur, 2, BUFW_REL);
		    p = mkr_before_get(&a_hist->hcur);
		    c = *p;

		    /* Remove the character and header(s). */
		    mkr_remove(&a_hist->htmp, &a_hist->hcur);

		    /* Insert the character. */
		    bufv.data = &c;
		    bufv.len = sizeof(c);
		    mkr_before_insert(&a_hist->hcur, &bufv, 1);
		    /* Insert the undo header. */
		    bufv.data = &uhdr;
		    bufv.len = sizeof(uhdr);
		    mkr_before_insert(&a_hist->hcur, &bufv, 1);
		    /* Insert the redo header if necessary. */
		    if (hst_cnt_get(rhdr) > 1)
		    {
			hst_cnt_set(rhdr, hst_cnt_get(rhdr) - 1);
			bufv.data = &rhdr;
			bufv.len = sizeof(rhdr);
			mkr_after_insert(&a_hist->hcur, &bufv, 1);
		    }

		    /* Actually take action, now that the log is updated. */
		    mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);
		    switch (hst_tag_get(rhdr))
		    {
			case HST_TAG_INS:
			{
			    bufv.data = &c;
			    bufv.len = sizeof(c);
			    mkr_l_insert(a_mkr, FALSE, FALSE, &bufv, 1);
			    a_hist->hbpos++;
			    break;
			}
			case HST_TAG_REM:
			{
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, -1, BUFW_REL);
			    mkr_l_remove(&tmkr, a_mkr, FALSE);
			    a_hist->hbpos--;
			    break;
			}
			case HST_TAG_YNK:
			{
			    bufv.data = &c;
			    bufv.len = sizeof(c);
			    mkr_l_insert(a_mkr, FALSE, TRUE, &bufv, 1);
			    break;
			}
			case HST_TAG_DEL:
			{
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, 1, BUFW_REL);
			    mkr_l_remove(a_mkr, &tmkr, FALSE);
			    break;
			}
			default:
			{
			    cw_not_reached();
			}
		    }

		    redid = TRUE;
		    break;
		}
		case HST_TAG_GRP_BEG:
		{
		    a_hist->gdepth++;
		    mkr_seek(&a_hist->hcur, 1, BUFW_REL);
		    break;
		}
		case HST_TAG_GRP_END:
		{
		    a_hist->gdepth--;
		    mkr_seek(&a_hist->hcur, 1, BUFW_REL);
		    break;
		}
		case HST_TAG_POS:
		{
		    cw_uint64_t from;
		    union
		    {
			cw_uint64_t bpos;
			cw_uint8_t str[8];
		    } u;
		    cw_bufv_t *bufv, pbufv;
		    cw_uint32_t bufvcnt;

		    /* Set up the bufv. */
		    pbufv.data = u.str;
		    pbufv.len = 8;

		    /* Read the history record. */
		    mkr_dup(&a_hist->htmp, &a_hist->hcur);
		    mkr_seek(&a_hist->hcur, 1, BUFW_REL);
		    mkr_seek(&a_hist->htmp, 9, BUFW_REL);
		    bufv = mkr_range_get(&a_hist->htmp, &a_hist->hcur,
					  &bufvcnt);
		    mkr_seek(&a_hist->hcur, -1, BUFW_REL);

		    /* Swap from and to. */
		    bufv_copy(&pbufv, 1, bufv, bufvcnt, 0);
		    from = a_hist->hbpos;
		    a_hist->hbpos = cw_ntohq(u.bpos);
		    u.bpos = cw_htonq(from);

		    /* Invert the history record. */
		    bufv = mkr_range_get(&a_hist->htmp, &a_hist->hcur,
					  &bufvcnt);
		    bufv_copy(bufv, bufvcnt, &pbufv, 1, 0);
		    p = mkr_before_get(&a_hist->htmp);
		    *p = rhdr;
		    mkr_dup(&a_hist->hcur, &a_hist->htmp);

		    /* Move. */
 		    mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);

		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }
	}
    }

    RETURN:
    mkr_delete(&tmkr);
    return retval;
}

void
hist_flush(cw_hist_t *a_hist, cw_buf_t *a_buf)
{
    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);

    mkr_seek(&a_hist->hcur, 0, BUFW_BOB);
    mkr_seek(&a_hist->htmp, 0, BUFW_EOB);
    mkr_remove(&a_hist->hcur, &a_hist->htmp);
    a_hist->hbpos = 0;
    a_hist->gdepth = 0;
}

void
hist_group_beg(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_mkr_t *a_mkr)
{
    cw_uint8_t hdr;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    hist_p_redo_flush(a_hist);

    /* If the position of a_mkr is not the same as the last saved postion,
     * insert a position record before creating the group begin record, so that
     * when the group is undone, the position ends up where the user would
     * expect it to. */
    if (a_mkr != NULL && mkr_pos(a_mkr) != a_hist->hbpos)
    {
	hist_p_pos(a_hist, a_buf, mkr_pos(a_mkr));
    }

    hst_tag_set(hdr, HST_TAG_GRP_BEG);
    bufv.data = &hdr;
    bufv.len = sizeof(hdr);
    mkr_before_insert(&a_hist->hcur, &bufv, 1);

    a_hist->gdepth++;
}

cw_bool_t
hist_group_end(cw_hist_t *a_hist, cw_buf_t *a_buf)
{
    cw_bool_t retval;
    cw_uint8_t hdr;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);

    if (a_hist->gdepth == 0)
    {
	/* Group ends must always be matched by beginnings. */
	retval = TRUE;
	goto RETURN;
    }

    hist_p_redo_flush(a_hist);

    hst_tag_set(hdr, HST_TAG_GRP_END);
    bufv.data = &hdr;
    bufv.len = sizeof(hdr);
    mkr_before_insert(&a_hist->hcur, &bufv, 1);

    a_hist->gdepth--;

    retval = FALSE;
    RETURN:
    return retval;
}

void
hist_ins(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos, const
	 cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_uint8_t *p, hdr, hst_cnt;
    cw_uint64_t i, j;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_bufv);

    hist_p_redo_flush(a_hist);

    /* Record a position change if necessary. */
    if (a_hist->hbpos != a_bpos)
    {
	hist_p_pos(a_hist, a_buf, a_bpos);
    }

    /* Iterate throught the bufv elements.  It is possible to unroll the nested
     * loops for a bit more efficiency, but at the cost of significantly more
     * code. */
    for (i = 0; i < a_bufvcnt; i++)
    {
	/* Update the recorded position. */
	a_hist->hbpos += a_bufv[i].len;

	j = 0;
	/*
	 * If the last history record is the same type, and it still has
	 * room left, insert as much data as will fit.
	 */
	if (mkr_pos(&a_hist->hcur) > 1)
	{
	    p = mkr_before_get(&a_hist->hcur);
	    if (hst_tag_get(*p) == HST_TAG_REM
		&& (hst_cnt = hst_cnt_get(*p)) < HST_CNT_MAX)
	    {
		/* Determine how much data to insert into the existing
		 * record. */
		if (HST_CNT_MAX - hst_cnt >= a_bufv[i].len)
		{
		    j = a_bufv[i].len;
		}
		else
		{
		    j = HST_CNT_MAX - hst_cnt;
		}

		/* Update the header before moving hcur. */
		hst_cnt_set(*p, hst_cnt + j);

		/* Move htmp before the header and insert text. */
		mkr_dup(&a_hist->htmp, &a_hist->hcur);
		mkr_seek(&a_hist->htmp, -1, BUFW_REL);
		bufv.data = a_bufv[i].data;
		bufv.len = j;
		mkr_before_insert(&a_hist->htmp, &bufv, 1);
	    }
	}

	/* While there are still >= HST_CNT_MAX characters to be inserted,
	 * insert full records. */
	for (; a_bufv[i].len - j >= HST_CNT_MAX; j += HST_CNT_MAX)
	{
	    bufv.data = &a_bufv[i].data[j];
	    bufv.len = HST_CNT_MAX;
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	    hst_tag_set(hdr, HST_TAG_REM);
	    hst_cnt_set(hdr, HST_CNT_MAX);
	    bufv.data = &hdr;
	    bufv.len = sizeof(hdr);
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	}

	/* Insert any remaining characters. */
	if (a_bufv[i].len - j > 0)
	{
	    bufv.data = &a_bufv[i].data[j];
	    bufv.len = a_bufv[i].len - j;
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	    hst_tag_set(hdr, HST_TAG_REM);
	    hst_cnt_set(hdr, a_bufv[i].len - j);
	    bufv.data = &hdr;
	    bufv.len = sizeof(hdr);
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	}
    }
}

void
hist_ynk(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos, const
	 cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_uint8_t *p, hdr, hst_cnt;
    cw_uint64_t i, j, k;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_bufv);

    hist_p_redo_flush(a_hist);

    /* Record a position change if necessary. */
    if (a_hist->hbpos != a_bpos)
    {
	hist_p_pos(a_hist, a_buf, a_bpos);
    }

    /* Iterate throught the bufv elements.  It is possible to unroll the nested
     * loops for a bit more efficiency, but at the cost of significantly more
     * code. */
    for (i = 0; i < a_bufvcnt; i++)
    {
	j = 0;
	/* If the last history record is the same type, and it still has room
	 * left, insert as much data as will fit. */
	if (mkr_pos(&a_hist->hcur) > 1)
	{
	    p = mkr_before_get(&a_hist->hcur);
	    if (hst_tag_get(*p) == HST_TAG_DEL
		&& (hst_cnt = hst_cnt_get(*p)) < HST_CNT_MAX)
	    {
		/* Determine how much data to insert into the existing
		 * record. */
		if (HST_CNT_MAX - hst_cnt >= a_bufv[i].len)
		{
		    j = a_bufv[i].len;
		}
		else
		{
		    j = HST_CNT_MAX - hst_cnt;
		}

		/* Update the header before moving hcur. */
		hst_cnt_set(*p, hst_cnt + j);

		/* Move htmp before the header and insert text. */
		mkr_dup(&a_hist->htmp, &a_hist->hcur);
		mkr_seek(&a_hist->htmp, -1, BUFW_REL);
		bufv.len = 1;
		for (k = 0; k < j; k++)
		{
		    bufv.data = &a_bufv[i].data[j - 1 - k];
		    mkr_before_insert(&a_hist->htmp, &bufv, 1);
		}
	    }
	}

	/* While there are still >= HST_CNT_MAX characters to be inserted,
	 * insert full records. */
	for (; a_bufv[i].len - j >= HST_CNT_MAX; j += HST_CNT_MAX)
	{
	    for (k = 0; k < HST_CNT_MAX; k++)
	    {
		bufv.data = &a_bufv[i].data[j + HST_CNT_MAX - 1 - k];
		mkr_before_insert(&a_hist->hcur, &bufv, 1);
	    }
	    hst_tag_set(hdr, HST_TAG_DEL);
	    hst_cnt_set(hdr, HST_CNT_MAX);
	    bufv.data = &hdr;
	    bufv.len = sizeof(hdr);
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	}

	/* Insert any remaining characters. */
	if (a_bufv[i].len - j > 0)
	{
	    bufv.len = 1;
	    for (k = 0; k < a_bufv[i].len - j; k++)
	    {
		bufv.data = &a_bufv[i].data[a_bufv[i].len - 1 - k];
		mkr_before_insert(&a_hist->hcur, &bufv, 1);
	    }
	    hst_tag_set(hdr, HST_TAG_DEL);
	    hst_cnt_set(hdr, a_bufv[i].len - j);
	    bufv.data = &hdr;
	    bufv.len = sizeof(hdr);
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	}
    }
}

void
hist_rem(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos, const
	 cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_uint8_t *p, hdr, hst_cnt;
    cw_uint64_t i, j, k;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_bufv);

    hist_p_redo_flush(a_hist);

    /* Record a position change if necessary. */
    if (a_hist->hbpos != a_bpos)
    {
	hist_p_pos(a_hist, a_buf, a_bpos);
    }

    /* Iterate throught the bufv elements.  It is possible to unroll the nested
     * loops for a bit more efficiency, but at the cost of significantly more
     * code. */
    for (i = 0; i < a_bufvcnt; i++)
    {
	/* Update the recorded position. */
	a_hist->hbpos -= a_bufv[i].len;

	j = 0;
	/* If the last history record is the same type, and it still has room
	 * left, insert as much data as will fit. */
	if (mkr_pos(&a_hist->hcur) > 1)
	{
	    p = mkr_before_get(&a_hist->hcur);
	    if (hst_tag_get(*p) == HST_TAG_INS
		&& (hst_cnt = hst_cnt_get(*p)) < HST_CNT_MAX)
	    {
		/* Determine how much data to insert into the existing
		 * record. */
		if (HST_CNT_MAX - hst_cnt >= a_bufv[i].len)
		{
		    j = a_bufv[i].len;
		}
		else
		{
		    j = HST_CNT_MAX - hst_cnt;
		}

		/* Update the header before moving hcur. */
		hst_cnt_set(*p, hst_cnt + j);

		/* Move htmp before the header and insert text. */
		mkr_dup(&a_hist->htmp, &a_hist->hcur);
		mkr_seek(&a_hist->htmp, -1, BUFW_REL);
		bufv.len = 1;
		for (k = 0; k < j; k++)
		{
		    bufv.data = &a_bufv[i].data[j - 1 - k];
		    mkr_before_insert(&a_hist->htmp, &bufv, 1);
		}
	    }
	}

	/* While there are still >= HST_CNT_MAX characters to be inserted,
	 * insert full records. */
	for (; a_bufv[i].len - j >= HST_CNT_MAX; j += HST_CNT_MAX)
	{
	    bufv.len = 1;
	    for (k = 0; k < HST_CNT_MAX; k++)
	    {
		bufv.data = &a_bufv[i].data[j + HST_CNT_MAX - 1 - k];
		mkr_before_insert(&a_hist->hcur, &bufv, 1);
	    }
	    hst_tag_set(hdr, HST_TAG_INS);
	    hst_cnt_set(hdr, HST_CNT_MAX);
	    bufv.data = &hdr;
	    bufv.len = sizeof(hdr);
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	}

	/* Insert any remaining characters. */
	if (a_bufv[i].len - j > 0)
	{
	    bufv.len = 1;
	    for (k = 0; k < a_bufv[i].len - j; k++)
	    {
		bufv.data = &a_bufv[i].data[a_bufv[i].len - 1 - k];
		mkr_before_insert(&a_hist->hcur, &bufv, 1);
	    }
	    hst_tag_set(hdr, HST_TAG_INS);
	    hst_cnt_set(hdr, a_bufv[i].len - j);
	    bufv.data = &hdr;
	    bufv.len = sizeof(hdr);
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	}
    }
}

void
hist_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos, const
	 cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_uint8_t *p, hdr, hst_cnt;
    cw_uint64_t i, j;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_bufv);

    hist_p_redo_flush(a_hist);

    /* Record a position change if necessary. */
    if (a_hist->hbpos != a_bpos)
    {
	hist_p_pos(a_hist, a_buf, a_bpos);
    }

    /* Iterate throught the bufv elements.  It is possible to unroll the nested
     * loops for a bit more efficiency, but at the cost of significantly more
     * code. */
    for (i = 0; i < a_bufvcnt; i++)
    {
	j = 0;
	/* If the last history record is the same type, and it still has room
	 * left, insert as much data as will fit. */
	if (mkr_pos(&a_hist->hcur) > 1)
	{
	    p = mkr_before_get(&a_hist->hcur);
	    if (hst_tag_get(*p) == HST_TAG_YNK
		&& (hst_cnt = hst_cnt_get(*p)) < HST_CNT_MAX)
	    {
		/* Determine how much data to insert into the existing
		 * record. */
		if (HST_CNT_MAX - hst_cnt >= a_bufv[i].len)
		{
		    j = a_bufv[i].len;
		}
		else
		{
		    j = HST_CNT_MAX - hst_cnt;
		}

		/* Update the header before moving hcur. */
		hst_cnt_set(*p, hst_cnt + j);

		/* Move htmp before the header and insert text. */
		mkr_dup(&a_hist->htmp, &a_hist->hcur);
		mkr_seek(&a_hist->htmp, -1, BUFW_REL);
		bufv.data = a_bufv[i].data;
		bufv.len = j;
		mkr_before_insert(&a_hist->htmp, &bufv, 1);
	    }
	}

	/* While there are still >= HST_CNT_MAX characters to be inserted,
	 * insert full records. */
	for (; a_bufv[i].len - j >= HST_CNT_MAX; j += HST_CNT_MAX)
	{
	    bufv.data = &a_bufv[i].data[j];
	    bufv.len = HST_CNT_MAX;
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	    hst_tag_set(hdr, HST_TAG_YNK);
	    hst_cnt_set(hdr, HST_CNT_MAX);
	    bufv.data = &hdr;
	    bufv.len = sizeof(hdr);
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	}

	/* Insert any remaining characters. */
	if (a_bufv[i].len - j > 0)
	{
	    bufv.data = &a_bufv[i].data[j];
	    bufv.len = a_bufv[i].len - j;
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	    hst_tag_set(hdr, HST_TAG_YNK);
	    hst_cnt_set(hdr, a_bufv[i].len - j);
	    bufv.data = &hdr;
	    bufv.len = sizeof(hdr);
	    mkr_before_insert(&a_hist->hcur, &bufv, 1);
	}
    }
}

#ifdef CW_BUF_DUMP
void
hist_dump(cw_hist_t *a_hist, const char *a_beg, const char *a_mid,
	  const char *a_end)
{
    const char hchars[] = "0123456789abcdef";
    const char *beg, *mid, *end;
    cw_uint8_t hdr, *p, c;
    union
    {
	cw_uint64_t bpos;
	cw_uint8_t str[8];
    } u;
    cw_bufv_t *bufv, pbufv, cbufv;
    cw_uint8_t text[32];
    cw_mkr_t tmkr, ttmkr;
    cw_sint32_t i;
    cw_uint32_t bufvcnt;
    char *tbeg, *tmid;

    beg = (a_beg != NULL) ? a_beg : "";
    mid = (a_mid != NULL) ? a_mid : beg;
    end = (a_end != NULL) ? a_end : mid;

    pbufv.data = u.str;
    pbufv.len = 8;

    cbufv.data = text;
    cbufv.len = 32;

    mkr_new(&tmkr, &a_hist->h);
    mkr_new(&ttmkr, &a_hist->h);

    fprintf(stderr, "%shist: %p\n", beg, a_hist);

    /* h. */
    fprintf(stderr, "%s|\n", mid);
    asprintf(&tbeg, "%s|-> h: ", mid);
    asprintf(&tmid, "%s|      ", mid);
    buf_dump(&a_hist->h, tbeg, tmid, NULL);
    free(tbeg);
    free(tmid);

    /* hcur. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> hcur: %p\n", mid, &a_hist->hcur);

    /* htmp. */
    fprintf(stderr, "%s|-> htmp: %p\n", mid, &a_hist->htmp);

    /* hbpos. */
    fprintf(stderr, "%s|-> hbpos: %llu\n", mid, a_hist->hbpos);

    /* gdepth. */
    fprintf(stderr, "%s|-> gdepth: %u\n", mid, a_hist->gdepth);

    /* Allocator state. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> dealloc: %p\n", mid, a_hist->dealloc);
    fprintf(stderr, "%s|-> arg: %p\n", mid, a_hist->arg);

    /* Undo. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s| [undo]: ", mid);
    mkr_dup(&tmkr, &a_hist->hcur);
    while (mkr_pos(&tmkr) > 1)
    {
	p = mkr_before_get(&tmkr);
	hdr = *p;
	switch (hst_tag_get(hdr))
	{
	    case HST_TAG_GRP_BEG:
	    {
		fprintf(stderr, "B");
		mkr_seek(&tmkr, -1, BUFW_REL);
		break;
	    }
	    case HST_TAG_GRP_END:
	    {
		fprintf(stderr, "E");
		mkr_seek(&tmkr, -1, BUFW_REL);
		break;
	    }
	    case HST_TAG_POS:
	    {
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -9, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&pbufv, 1, bufv, bufvcnt, 0);
		fprintf(stderr, "P(%llu)", cw_ntohq(u.bpos));
		break;
	    }
	    case HST_TAG_INS:
	    {
		fprintf(stderr, "I%d(", hst_cnt_get(hdr));
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -1 - (cw_sint64_t) hst_cnt_get(hdr), BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&cbufv, 1, bufv, bufvcnt, 0);
		for (i = 0; i < hst_cnt_get(hdr); i++)
		{
		    c = text[hst_cnt_get(hdr) - 1 - i];
		    if (isprint(c))
		    {
			fprintf(stderr, "%c", c);
		    }
		    else
		    {
			fprintf(stderr, "\\x%c%c",
				hchars[c >> 4], hchars[c & 0xf]);
		    }
		}
		fprintf(stderr, ")");
		break;
	    }
	    case HST_TAG_YNK:
	    {
		fprintf(stderr, "Y%d(", hst_cnt_get(hdr));
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -1 - (cw_sint64_t) hst_cnt_get(hdr), BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&cbufv, 1, bufv, bufvcnt, 0);
		for (i = 0; i < hst_cnt_get(hdr); i++)
		{
		    c = text[hst_cnt_get(hdr) - 1 - i];
		    if (isprint(c))
		    {
			fprintf(stderr, "%c", c);
		    }
		    else
		    {
			fprintf(stderr, "\\x%c%c",
				hchars[c >> 4], hchars[c & 0xf]);
		    }
		}
		fprintf(stderr, ")");
		break;
	    }
	    case HST_TAG_REM:
	    {
		fprintf(stderr, "R%d(", hst_cnt_get(hdr));
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -1 - (cw_sint64_t) hst_cnt_get(hdr), BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&cbufv, 1, bufv, bufvcnt, 0);
		for (i = 0; i < hst_cnt_get(hdr); i++)
		{
		    c = text[hst_cnt_get(hdr) - 1 - i];
		    if (isprint(c))
		    {
			fprintf(stderr, "%c", c);
		    }
		    else
		    {
			fprintf(stderr, "\\x%c%c",
				hchars[c >> 4], hchars[c & 0xf]);
		    }
		}
		fprintf(stderr, ")");
		break;
	    }
	    case HST_TAG_DEL:
	    {
		fprintf(stderr, "D%d(", hst_cnt_get(hdr));
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -1 - (cw_sint64_t) hst_cnt_get(hdr), BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&cbufv, 1, bufv, bufvcnt, 0);
		for (i = 0; i < hst_cnt_get(hdr); i++)
		{
		    c = text[hst_cnt_get(hdr) - 1 - i];
		    if (isprint(c))
		    {
			fprintf(stderr, "%c", c);
		    }
		    else
		    {
			fprintf(stderr, "\\x%c%c",
				hchars[c >> 4], hchars[c & 0xf]);
		    }
		}
		fprintf(stderr, ")");
		break;
	    }
	    default:
	    {
		fprintf(stderr, "X");
		mkr_seek(&tmkr, -1, BUFW_REL);
	    }
	}
    }
    fprintf(stderr, "\n");

    /* Redo. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%sV [redo]: ", end);
    mkr_dup(&tmkr, &a_hist->hcur);
    while (mkr_pos(&tmkr) < buf_len(&a_hist->h) + 1)
    {
	p = mkr_after_get(&tmkr);
	hdr = *p;
	switch (hst_tag_get(hdr))
	{
	    case HST_TAG_GRP_BEG:
	    {
		fprintf(stderr, "B");
		mkr_seek(&tmkr, 1, BUFW_REL);
		break;
	    }
	    case HST_TAG_GRP_END:
	    {
		fprintf(stderr, "E");
		mkr_seek(&tmkr, 1, BUFW_REL);
		break;
	    }
	    case HST_TAG_POS:
	    {
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, 9, BUFW_REL);
		mkr_seek(&ttmkr, 1, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&pbufv, 1, bufv, bufvcnt, 0);
		fprintf(stderr, "P(%llu)", cw_ntohq(u.bpos));
		break;
	    }
	    case HST_TAG_INS:
	    {
		fprintf(stderr, "I%d(", hst_cnt_get(hdr));
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, 1 + hst_cnt_get(hdr), BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&cbufv, 1, bufv, bufvcnt, 0);
		for (i = 0; i < hst_cnt_get(hdr); i++)
		{
		    c = text[i + 1];
		    if (isprint(c))
		    {
			fprintf(stderr, "%c", c);
		    }
		    else
		    {
			fprintf(stderr, "\\x%c%c",
				hchars[c >> 4], hchars[c & 0xf]);
		    }
		}

		fprintf(stderr, ")");
		break;
	    }
	    case HST_TAG_YNK:
	    {
		fprintf(stderr, "Y%d(", hst_cnt_get(hdr));
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, 1 + hst_cnt_get(hdr), BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&cbufv, 1, bufv, bufvcnt, 0);
		for (i = 0; i < hst_cnt_get(hdr); i++)
		{
		    c = text[i + 1];
		    if (isprint(c))
		    {
			fprintf(stderr, "%c", c);
		    }
		    else
		    {
			fprintf(stderr, "\\x%c%c",
				hchars[c >> 4], hchars[c & 0xf]);
		    }
		}		
		fprintf(stderr, ")");
		break;
	    }
	    case HST_TAG_REM:
	    {
		fprintf(stderr, "R%d(", hst_cnt_get(hdr));
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, 1 + hst_cnt_get(hdr), BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&cbufv, 1, bufv, bufvcnt, 0);
		for (i = 0; i < hst_cnt_get(hdr); i++)
		{
		    c = text[i + 1];
		    if (isprint(c))
		    {
			fprintf(stderr, "%c", c);
		    }
		    else
		    {
			fprintf(stderr, "\\x%c%c",
				hchars[c >> 4], hchars[c & 0xf]);
		    }
		}
		fprintf(stderr, ")");
		break;
	    }
	    case HST_TAG_DEL:
	    {
		fprintf(stderr, "D%d(", hst_cnt_get(hdr));
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, 1 + hst_cnt_get(hdr), BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		bufv_copy(&cbufv, 1, bufv, bufvcnt, 0);
		for (i = 0; i < hst_cnt_get(hdr); i++)
		{
		    c = text[i + 1];
		    if (isprint(c))
		    {
			fprintf(stderr, "%c", c);
		    }
		    else
		    {
			fprintf(stderr, "\\x%c%c",
				hchars[c >> 4], hchars[c & 0xf]);
		    }
		}
		fprintf(stderr, ")");
		break;
	    }
	    default:
	    {
		fprintf(stderr, "X");
		mkr_seek(&tmkr, 1, BUFW_REL);
	    }
	}
    }
    fprintf(stderr, "\n");

    mkr_delete(&ttmkr);
    mkr_delete(&tmkr);
}
#endif
