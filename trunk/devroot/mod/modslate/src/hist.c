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
 * system), buffer history can become quite long.  Therefore, the buffer history
 * mechanism aims to be as compact as reasonably possible, without resorting to
 * excessive complexity.  Internally, the history mechanism is a combination of
 * a state machine that maintains enough state to be able to play the history in
 * either direction, along with a log of buffer changes.
 *
 * User input that does not involve explicit point movement causes the history
 * log to grow by approximately one byte per inserted/deleted character.  Log
 * records have a fixed overhead of 9 bytes.
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
 * If there are redo records, it may be that one record that is partly
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

/* History record header length. */
#define HST_HDR_LEN	9

/* History record tag values. */
#define HST_TAG_GRP_BEG	1
#define HST_TAG_GRP_END	2
#define HST_TAG_POS	3
#define HST_TAG_INS	4
#define HST_TAG_YNK	5
#define HST_TAG_REM	6
#define HST_TAG_DEL	7

typedef struct
{
    cw_uint8_t tag;
    cw_uint64_t aux; /* Host byte order, always valid. */
    union
    {
	cw_uint64_t aux; /* Host or network byte order, depending on need. */
	cw_uint8_t str[8];
    } u;
    cw_bufv_t bufv[2];
} cw_hist_hdr_t;

/* Convenience macros that define and set up a bufv for a history record on the
 * stack.  They can be used as such:
 *
 * {
 *     HIST_HDR(hdr);
 *     HIST_HDR_INIT(hdr);
 *
 *     hdr.tag = HST_TAG_POS;
 *     hdr.u.aux = cw_htonq(a_hist->hbpos);
 *
 *     mkr_before_insert(&a_hist->hcur, hdr.bufv, 2);
 * }
 */
#define HIST_HDR(a_var)							\
    cw_hist_hdr_t a_var

#define HIST_HDR_INIT(a_var)						\
    a_var.bufv[0].data = &( a_var.tag );				\
    a_var.bufv[0].len = sizeof(a_var.tag);				\
									\
    a_var.bufv[1].data = a_var.u.str;					\
    a_var.bufv[1].len = sizeof(a_var.u.aux)

#ifdef CW_BUF_DUMP
static void
hist_p_bufv_print(const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    const char hchars[] = "0123456789abcdef";
    cw_uint32_t i, j;
    cw_uint8_t c;

    for (i = 0; i < a_bufvcnt; i++)
    {
	for (j = 0; j < a_bufv[i].len; j++)
	{
	    c = a_bufv[i].data[j];
	    if (isprint(c))
	    {
		fprintf(stderr, "%c", c);
	    }
	    else
	    {
		fprintf(stderr, "\\x%c%c", hchars[c >> 4], hchars[c & 0xf]);
	    }
	}
    }
}
#endif

CW_P_INLINE cw_uint8_t
hist_p_tag_invert(cw_uint8_t a_tag)
{
    cw_uint8_t retval;

    cw_assert(a_tag == HST_TAG_INS
	      || a_tag == HST_TAG_YNK
	      || a_tag == HST_TAG_REM
	      || a_tag == HST_TAG_DEL);

    switch (a_tag)
    {
	case HST_TAG_INS:
	{
	    retval = HST_TAG_REM;
	    break;
	}
	case HST_TAG_YNK:
	{
	    retval = HST_TAG_DEL;
	    break;
	}
	case HST_TAG_REM:
	{
	    retval = HST_TAG_INS;
	    break;
	}
	case HST_TAG_DEL:
	{
	    retval = HST_TAG_YNK;
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return retval;
}

/* Get the current undo header.  Various code relies on the fact that the header
 * is bracketed by a_b..a_a after this call. */
CW_P_INLINE void
hist_p_undo_header_get(cw_mkr_t *a_a, cw_mkr_t *a_b, cw_hist_hdr_t *a_hdr)
{
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    cw_assert(mkr_pos(a_a) > 1);

    mkr_dup(a_b, a_a);
    mkr_seek(a_b, -HST_HDR_LEN, BUFW_REL);
    bufv = mkr_range_get(a_b, a_a, &bufvcnt);

    bufv_copy(a_hdr->bufv, 2, bufv, bufvcnt, 0);
    a_hdr->aux = cw_ntohq(a_hdr->u.aux);
}

/* Get the current redo header.  Various code relies on the fact that the header
 * is bracketed by a_a..a_b after this call. */
CW_P_INLINE void
hist_p_redo_header_get(cw_mkr_t *a_a, cw_mkr_t *a_b, cw_hist_hdr_t *a_hdr)
{
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    mkr_dup(a_b, a_a);
    mkr_seek(a_b, HST_HDR_LEN, BUFW_REL);
    cw_assert(mkr_pos(a_a) + HST_HDR_LEN == mkr_pos(a_b));
    bufv = mkr_range_get(a_a, a_b, &bufvcnt);

    bufv_copy(a_hdr->bufv, 2, bufv, bufvcnt, 0);
    a_hdr->aux = cw_ntohq(a_hdr->u.aux);
}

/* Flush redo state, if any. */
CW_P_INLINE void
hist_p_redo_flush(cw_hist_t *a_hist)
{
    if (mkr_pos(&a_hist->hcur) != buf_len(&a_hist->h) + 1)
    {
	mkr_seek(&a_hist->htmp, 0, BUFW_EOB);
	mkr_remove(&a_hist->hcur, &a_hist->htmp);
    }
}

/* Insert a position record into the undo log. */
CW_P_INLINE void
hist_p_pos(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos)
{
    HIST_HDR(hdr);

    cw_assert(&a_hist->h != NULL);
    cw_assert(mkr_pos(&a_hist->hcur) == buf_len(&a_hist->h) + 1);
    cw_assert(a_bpos != a_hist->hbpos);

    HIST_HDR_INIT(hdr);

    if (a_hist->hbpos == 0)
    {
	/* There's no need for an initial position record. */
	a_hist->hbpos = a_bpos;
	goto RETURN;
    }

    /* Record header. */
    hdr.tag = HST_TAG_POS;
    /* Old position. */
    hdr.aux = a_hist->hbpos;
    hdr.u.aux = cw_htonq(hdr.aux);

    mkr_before_insert(&a_hist->hcur, hdr.bufv, 2);

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
    cw_uint8_t *p, c;
    cw_mkr_t tmkr;
    cw_bufv_t bufv;
    HIST_HDR(undo_hdr);
    HIST_HDR(redo_hdr);

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    HIST_HDR_INIT(undo_hdr);
    HIST_HDR_INIT(redo_hdr);

    mkr_new(&tmkr, a_buf);

    /* Iteratively undo a_count times. */
    for (retval = 0;
	 retval < a_count && mkr_pos(&a_hist->hcur) > 1;
	 )
    {
	hist_p_undo_header_get(&a_hist->hcur, &a_hist->htmp, &undo_hdr);

	switch (undo_hdr.tag)
	{
	    case HST_TAG_INS:
	    case HST_TAG_YNK:
	    case HST_TAG_REM:
	    case HST_TAG_DEL:
	    {
		/* Read (or synthesize) the undo header, redo header, and
		 * character, then remove them from the history buffer.  Then
		 * re-insert them in their new order and state.
		 *
		 * There are four possible variations.  In all cases, hcur is
		 * positioned before the character, and htmp is positioned after
		 * the redo header (if it exists and is being re-used).  This
		 * allows an unconditional call to mkr_remove(). */

		/* Get or synthesize the redo header. */
		if (mkr_pos(&a_hist->hcur) != buf_len(&a_hist->h) + 1)
		{
		    /* Get the redo header and check if it is the inverse of the
		     * undo header. */
		    hist_p_redo_header_get(&a_hist->hcur, &a_hist->htmp,
					   &redo_hdr);

		    if (redo_hdr.tag == hist_p_tag_invert(undo_hdr.tag))
		    {
			/* Update the redo header state. */
			redo_hdr.aux++;
			redo_hdr.u.aux = cw_htonq(redo_hdr.aux);

			/* The hist_p_redo_header_get() call above already moved
			 * htmp to the proper location for the mkr_remove() call
			 * below. */
		    }
		    else
		    {
			/* Synthesize the redo header. */
			redo_hdr.tag = hist_p_tag_invert(undo_hdr.tag);
			redo_hdr.aux = 1;
			redo_hdr.u.aux = cw_htonq(redo_hdr.aux);

			/* Move htmp to the proper location for the mkr_remove()
			 * call below. */
			mkr_dup(&a_hist->htmp, &a_hist->hcur);
		    }
		}
		else
		{
		    /* Synthesize the redo header. */
		    redo_hdr.tag = hist_p_tag_invert(undo_hdr.tag);
		    redo_hdr.aux = 1;
		    redo_hdr.u.aux = cw_htonq(redo_hdr.aux);

		    /* Move htmp to the proper location for the mkr_remove()
		     * call below. */
		    mkr_dup(&a_hist->htmp, &a_hist->hcur);
		}

		/* Set c and move hcur to the appropriate position. */
		mkr_seek(&a_hist->hcur, -10, BUFW_REL);
		p = mkr_after_get(&a_hist->hcur);
		c = *p;

		/* Remove the character and header(s). */
		mkr_remove(&a_hist->hcur, &a_hist->htmp);

		/* Insert the character. */
		bufv.data = &c;
		bufv.len = sizeof(c);
		mkr_after_insert(&a_hist->hcur, &bufv, 1);

		/* Insert the redo header. */
		mkr_after_insert(&a_hist->hcur, redo_hdr.bufv, 2);

		/* Insert the undo header, if necessary. */
		if (undo_hdr.aux > 1)
		{
		    undo_hdr.aux--;
		    undo_hdr.u.aux = cw_htonq(undo_hdr.aux);

		    mkr_before_insert(&a_hist->hcur, undo_hdr.bufv, 2);
		}

		/* Actually take action, now that the log is updated. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);
		switch (undo_hdr.tag)
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

		if (a_hist->gdepth == 0)
		{
		    retval++;
		}
		break;
	    }
	    case HST_TAG_GRP_BEG:
	    {
		a_hist->gdepth--;
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}
		mkr_seek(&a_hist->hcur, -HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_GRP_END:
	    {
		a_hist->gdepth++;
		mkr_seek(&a_hist->hcur, -HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_POS:
	    {
		/* Read the undo record. */
		hist_p_undo_header_get(&a_hist->hcur, &a_hist->htmp, &undo_hdr);

		/* Set up the redo record. */
		redo_hdr.tag = HST_TAG_POS;
		redo_hdr.aux = a_hist->hbpos;
		redo_hdr.u.aux = cw_htonq(redo_hdr.aux);

		/* Update hbpos. */
		a_hist->hbpos = undo_hdr.aux;

		/* Remove the undo record. */
		mkr_remove(&a_hist->htmp, &a_hist->hcur);

		/* Insert the redo record. */
		mkr_after_insert(&a_hist->hcur, redo_hdr.bufv, 2);

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

    mkr_delete(&tmkr);
    return retval;
}

cw_uint64_t
hist_redo(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_mkr_t *a_mkr,
	  cw_uint64_t a_count)
{
    cw_uint64_t retval;
    cw_bool_t redid;
    cw_uint8_t *p, c;
    cw_mkr_t tmkr;
    cw_bufv_t bufv;
    HIST_HDR(undo_hdr);
    HIST_HDR(redo_hdr);

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    HIST_HDR_INIT(undo_hdr);
    HIST_HDR_INIT(redo_hdr);

    mkr_new(&tmkr, a_buf);

    /* Iteratively redo a_count times. */
    for (retval = 0, redid = FALSE;
	 retval < a_count && mkr_pos(&a_hist->hcur) < buf_len(&a_hist->h) + 1;
	 )
    {
	/* Make a note that the redo loop was executed at least once, so that a
	 * correct decision about whether an incomplete group was redone can be
	 * made after the loop terminates. */
	redid = TRUE;

	hist_p_redo_header_get(&a_hist->hcur, &a_hist->htmp, &redo_hdr);

	switch (redo_hdr.tag)
	{
	    case HST_TAG_INS:
	    case HST_TAG_YNK:
	    case HST_TAG_REM:
	    case HST_TAG_DEL:
	    {
		/* Read (or synthesize) the redo header, undo header, and
		 * character, then remove them from the history buffer.  Then
		 * re-insert them in their new order and state.
		 *
		 * There are four possible variations.  In all cases, hcur is
		 * positioned after the character, and htmp is positioned before
		 * the undo header (if it exists and is being re-used).  This
		 * allows an unconditional call to mkr_remove(). */

		/* Get or synthesize the undo header. */
		if (mkr_pos(&a_hist->hcur) > 1)
		{
		    /* Get the undo header and check if it is the inverse of the
		     * redo header. */
		    hist_p_undo_header_get(&a_hist->hcur, &a_hist->htmp,
					   &undo_hdr);

		    if (undo_hdr.tag == hist_p_tag_invert(redo_hdr.tag))
		    {
			/* Update the undo header state. */
			undo_hdr.aux++;
			undo_hdr.u.aux = cw_htonq(undo_hdr.aux);

			/* The hist_p_undo_header_get() call above already moved
			 * htmp to the proper location for the mkr_remove() call
			 * below. */
		    }
		    else
		    {
			/* Synthesize the undo header. */
			undo_hdr.tag = hist_p_tag_invert(redo_hdr.tag);
			undo_hdr.aux = 1;
			undo_hdr.u.aux = cw_htonq(undo_hdr.aux);

			/* Move htmp to the proper location for the mkr_remove()
			 * call below. */
			mkr_dup(&a_hist->htmp, &a_hist->hcur);
		    }
		}
		else
		{
		    /* Synthesize the undo header. */
		    undo_hdr.tag = hist_p_tag_invert(redo_hdr.tag);
		    undo_hdr.aux = 1;
		    undo_hdr.u.aux = cw_htonq(undo_hdr.aux);

		    /* Move htmp to the proper location for the mkr_remove()
		     * call below. */
		    mkr_dup(&a_hist->htmp, &a_hist->hcur);
		}
		
		/* Set c and move hcur to the appropriate position. */
		mkr_seek(&a_hist->hcur, 10, BUFW_REL);
		p = mkr_before_get(&a_hist->hcur);
		c = *p;

		/* Remove the character and header(s). */
		mkr_remove(&a_hist->htmp, &a_hist->hcur);

		/* Insert the character. */
		bufv.data = &c;
		bufv.len = sizeof(c);
		mkr_before_insert(&a_hist->hcur, &bufv, 1);

		/* Insert the undo header. */
		mkr_before_insert(&a_hist->hcur, undo_hdr.bufv, 2);

		/* Insert the redo header, if necessary. */
		if (redo_hdr.aux > 1)
		{
		    redo_hdr.aux--;
		    redo_hdr.u.aux = cw_htonq(redo_hdr.aux);

		    mkr_after_insert(&a_hist->hcur, redo_hdr.bufv, 2);
		}

		/* Actually take action, now that the log is updated. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);
		switch (redo_hdr.tag)
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

		if (a_hist->gdepth == 0)
		{
		    retval++;
		}
		break;
	    }
	    case HST_TAG_GRP_BEG:
	    {
		a_hist->gdepth++;
		mkr_seek(&a_hist->hcur, HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_GRP_END:
	    {
		a_hist->gdepth--;
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}
		mkr_seek(&a_hist->hcur, HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_POS:
	    {
		HIST_HDR(hdr);

		HIST_HDR_INIT(hdr);

		/* Read the redo record. */
		hist_p_redo_header_get(&a_hist->hcur, &a_hist->htmp, &redo_hdr);

		/* Set up the undo record. */
		undo_hdr.tag = HST_TAG_POS;
		undo_hdr.aux = a_hist->hbpos;
		undo_hdr.u.aux = cw_htonq(undo_hdr.aux);

		/* Update hbpos. */
		a_hist->hbpos = redo_hdr.aux;

		/* Remove the redo record. */
		mkr_remove(&a_hist->hcur, &a_hist->htmp);

		/* Insert the undo record. */
		mkr_before_insert(&a_hist->hcur, undo_hdr.bufv, 2);

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

    if (a_hist->gdepth > 0
	&& redid
	&& mkr_pos(&a_hist->hcur) == buf_len(&a_hist->h) + 1)
    {
	/* There is an incomplete group that was redone.  Increment the redo
	 * count to indicate that something was redone. */
	retval++;
    }

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
    HIST_HDR(hdr);

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    HIST_HDR_INIT(hdr);

    hist_p_redo_flush(a_hist);

    /* If the position of a_mkr is not the same as the last saved postion,
     * insert a position record before creating the group begin record, so that
     * when the group is undone, the position ends up where the user would
     * expect it to. */
    if (a_mkr != NULL && mkr_pos(a_mkr) != a_hist->hbpos)
    {
	hist_p_pos(a_hist, a_buf, mkr_pos(a_mkr));
    }

    hdr.tag = HST_TAG_GRP_BEG;
    hdr.aux = 0;
    hdr.u.aux = 0;

    mkr_before_insert(&a_hist->hcur, hdr.bufv, 2);

    a_hist->gdepth++;
}

cw_bool_t
hist_group_end(cw_hist_t *a_hist, cw_buf_t *a_buf)
{
    cw_bool_t retval;
    HIST_HDR(hdr);

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);

    HIST_HDR_INIT(hdr);

    if (a_hist->gdepth == 0)
    {
	/* Group ends must always be matched by beginnings. */
	retval = TRUE;
	goto RETURN;
    }

    hist_p_redo_flush(a_hist);

    hdr.tag = HST_TAG_GRP_END;
    hdr.aux = 0;
    hdr.u.aux = 0;

    mkr_before_insert(&a_hist->hcur, hdr.bufv, 2);

    a_hist->gdepth--;

    retval = FALSE;
    RETURN:
    return retval;
}

static void
hist_p_ins_ynk_rem_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
		       const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt,
		       cw_uint8_t a_tag)
{
    cw_mkr_t tmkr;
    cw_uint64_t cnt;
    cw_bufv_t *bufv;
    cw_uint32_t i, bufvcnt;
    HIST_HDR(undo_hdr);

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_bufv);

    HIST_HDR_INIT(undo_hdr);

    hist_p_redo_flush(a_hist);

    /* Record a position change if necessary. */
    if (a_hist->hbpos != a_bpos)
    {
	hist_p_pos(a_hist, a_buf, a_bpos);
    }

    mkr_new(&tmkr, &a_hist->h);

    cnt = 0;
    for (i = 0; i < a_bufvcnt; i++)
    {
	cnt += (cw_uint64_t) a_bufv[i].len;
    }

    /* Update the recorded position. */
    switch (a_tag)
    {
	case HST_TAG_INS:
	{
	    a_hist->hbpos -= cnt;
	    break;
	}
	case HST_TAG_REM:
	{
	    a_hist->hbpos += cnt;
	    break;
	}
	case HST_TAG_YNK:
	case HST_TAG_DEL:
	{
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    /* Get the undo header, or synthesize one.  Remove header if extending an
     * existing record. */
    if (mkr_pos(&a_hist->hcur) > 1)
    {
	/* Get the undo header and check if it is a remove record. */
	hist_p_undo_header_get(&a_hist->hcur, &a_hist->htmp, &undo_hdr);

	if (undo_hdr.tag == a_tag)
	{
	    /* Remove the header. */
	    mkr_remove(&a_hist->htmp, &a_hist->hcur);

	    /* Update the data count. */
	    undo_hdr.aux += cnt;
	    undo_hdr.u.aux = cw_htonq(undo_hdr.aux);
	}
	else
	{
	    /* Synthesize the header. */
	    undo_hdr.tag = a_tag;
	    undo_hdr.aux = cnt;
	    undo_hdr.u.aux = cw_htonq(undo_hdr.aux);

	    /* Move htmp to the proper location to insert the data into the
	     * log. */
	    mkr_dup(&a_hist->htmp, &a_hist->hcur);
	}
    }
    else
    {
	/* Synthesize the header. */
	undo_hdr.tag = a_tag;
	undo_hdr.aux = cnt;
	undo_hdr.u.aux = cw_htonq(undo_hdr.aux);
    }

    /* Insert the data. */
    mkr_before_insert(&a_hist->htmp, a_bufv, a_bufvcnt);

    /* Reverse the data, if the record type requires it. */
    switch (a_tag)
    {
	case HST_TAG_YNK:
	case HST_TAG_REM:
	{
	    break;
	}
	case HST_TAG_INS:
	case HST_TAG_DEL:
	{
	    /* Now that there is space for the data, do a bufv_rcopy() to get
	     * the data written in the proper order. */
	    mkr_dup(&tmkr, &a_hist->htmp);
	    mkr_seek(&tmkr, -(cw_sint64_t)cnt, BUFW_REL);
	    bufv = mkr_range_get(&tmkr, &a_hist->htmp, &bufvcnt);
	    if (bufv != NULL)
	    {
		bufv_rcopy(bufv, bufvcnt, a_bufv, a_bufvcnt, 0);
	    }

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    /* Write the undo record header. */
    mkr_before_insert(&a_hist->hcur, undo_hdr.bufv, 2);

    mkr_delete(&tmkr);
}

void
hist_ins(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HST_TAG_REM);
}

void
hist_ynk(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HST_TAG_DEL);
}

void
hist_rem(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HST_TAG_INS);
}

void
hist_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HST_TAG_YNK);
}

#ifdef CW_BUF_DUMP
void
hist_dump(cw_hist_t *a_hist, const char *a_beg, const char *a_mid,
	  const char *a_end)
{
    const char *beg, *mid, *end;
    HIST_HDR(hdr);
    cw_bufv_t *bufv;
    cw_mkr_t tmkr, ttmkr;
    cw_uint32_t bufvcnt;
#ifdef CW_HIST_DUMP
    char *tbeg, *tmid;
#endif

    HIST_HDR_INIT(hdr);

    beg = (a_beg != NULL) ? a_beg : "";
    mid = (a_mid != NULL) ? a_mid : beg;
    end = (a_end != NULL) ? a_end : mid;

    mkr_new(&tmkr, &a_hist->h);
    mkr_new(&ttmkr, &a_hist->h);

    fprintf(stderr, "%shist: %p\n", beg, a_hist);

#ifdef CW_HIST_DUMP
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
#endif

    /* hbpos. */
    fprintf(stderr, "%s|-> hbpos: %llu\n", mid, a_hist->hbpos);

    /* gdepth. */
    fprintf(stderr, "%s|-> gdepth: %u\n", mid, a_hist->gdepth);

#ifdef CW_HIST_DUMP
    /* Allocator state. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> dealloc: %p\n", mid, a_hist->dealloc);
    fprintf(stderr, "%s|-> arg: %p\n", mid, a_hist->arg);
#endif

    /* Undo. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s| [undo]:\n", mid);
    mkr_dup(&tmkr, &a_hist->hcur);
    while (mkr_pos(&tmkr) > 1)
    {
	hist_p_undo_header_get(&tmkr, &ttmkr, &hdr);
	
	switch (hdr.tag)
	{
	    case HST_TAG_GRP_BEG:
	    {
		fprintf(stderr, "%s|         B\n", mid);
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_GRP_END:
	    {
		fprintf(stderr, "%s|         E\n", mid);
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_POS:
	    {
		fprintf(stderr, "%s|         (%llu)P\n", mid, hdr.aux);
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_INS:
	    {
		fprintf(stderr, "%s|         (", mid);
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")%lluI\n", hdr.aux);
		break;
	    }
	    case HST_TAG_YNK:
	    {
		fprintf(stderr, "%s|         (", mid);
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")%lluY\n", hdr.aux);
		break;
	    }
	    case HST_TAG_REM:
	    {
		fprintf(stderr, "%s|         (", mid);
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")%lluR\n", hdr.aux);
		break;
	    }
	    case HST_TAG_DEL:
	    {
		fprintf(stderr, "%s|         (", mid);
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")%lluD\n", hdr.aux);
		break;
	    }
	    default:
	    {
		fprintf(stderr, "%s|         X\n", mid);
		mkr_seek(&tmkr, -1, BUFW_REL);
	    }
	}
    }

    /* Redo. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s| [redo]:\n", mid);
    mkr_dup(&tmkr, &a_hist->hcur);
    while (mkr_pos(&tmkr) < buf_len(&a_hist->h) + 1)
    {
	hist_p_redo_header_get(&tmkr, &ttmkr, &hdr);
	
	switch (hdr.tag)
	{
	    case HST_TAG_GRP_BEG:
	    {
		fprintf(stderr, "%s|         B\n", mid);
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_GRP_END:
	    {
		fprintf(stderr, "%s|         E\n", mid);
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_POS:
	    {
		fprintf(stderr, "%s|         P(%llu)\n", mid, hdr.aux);
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_INS:
	    {
		fprintf(stderr, "%s|         I%llu(", mid, hdr.aux);
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")\n");
		break;
	    }
	    case HST_TAG_YNK:
	    {
		fprintf(stderr, "%s|         Y%llu(\n", mid, hdr.aux);
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")\n");
		break;
	    }
	    case HST_TAG_REM:
	    {
		fprintf(stderr, "%s|         R%llu(", mid, hdr.aux);
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")\n");
		break;
	    }
	    case HST_TAG_DEL:
	    {
		fprintf(stderr, "%s|         D%llu(", mid, hdr.aux);
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")\n");
		break;
	    }
	    default:
	    {
		fprintf(stderr, "%s|         X\n", mid);
		mkr_seek(&tmkr, 1, BUFW_REL);
	    }
	}
    }
    fprintf(stderr, "%sV\n", end);

    mkr_delete(&ttmkr);
    mkr_delete(&tmkr);
}
#endif

#ifdef CW_BUF_VALIDATE
void
hist_validate(cw_hist_t *a_hist, cw_buf_t *a_buf)
{
    cw_mkr_t tmkr, ttmkr;
    HIST_HDR(hdr);

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);

    HIST_HDR_INIT(hdr);

    mkr_new(&tmkr, &a_hist->h);
    mkr_new(&ttmkr, &a_hist->h);

    /* Validate h. */
    buf_validate(&a_hist->h);

    /* Validate that hcur and htmp are in h. */
    cw_assert(a_hist->hcur.bufp->buf == &a_hist->h);
    cw_assert(a_hist->htmp.bufp->buf == &a_hist->h);

    /* Validate that hpbos is a legal bpos in the data buf. */
    cw_assert(a_hist->hbpos >= 1
	      || buf_len(&a_hist->h) == 0);
    cw_assert(a_hist->hbpos <= buf_len(a_buf) + 1);

    /* Iterate through the undo history and validate record consistency. */
    mkr_dup(&tmkr, &a_hist->hcur);
    while (mkr_pos(&tmkr) > 1)
    {
	hist_p_undo_header_get(&tmkr, &ttmkr, &hdr);
	
	switch (hdr.tag)
	{
	    case HST_TAG_GRP_BEG:
	    {
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_GRP_END:
	    {
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_POS:
	    {
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_INS:
	    {
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		break;
	    }
	    case HST_TAG_YNK:
	    {
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		break;
	    }
	    case HST_TAG_REM:
	    {
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		break;
	    }
	    case HST_TAG_DEL:
	    {
		mkr_seek(&tmkr, -HST_HDR_LEN, BUFW_REL);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }

    /* Iterate through the redo history and validate record consistency. */
    mkr_dup(&tmkr, &a_hist->hcur);
    while (mkr_pos(&tmkr) < buf_len(&a_hist->h) + 1)
    {
	hist_p_redo_header_get(&tmkr, &ttmkr, &hdr);
	
	switch (hdr.tag)
	{
	    case HST_TAG_GRP_BEG:
	    {
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_GRP_END:
	    {
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_POS:
	    {
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		break;
	    }
	    case HST_TAG_INS:
	    {
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		break;
	    }
	    case HST_TAG_YNK:
	    {
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		break;
	    }
	    case HST_TAG_REM:
	    {
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		break;
	    }
	    case HST_TAG_DEL:
	    {
		mkr_seek(&tmkr, HST_HDR_LEN, BUFW_REL);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }

    mkr_delete(&ttmkr);
    mkr_delete(&tmkr);
}
#endif
