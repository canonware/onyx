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
 * either direction, along with a log of buffer changes.  The history record
 * format is such that the history buffer can be stored to disk during an
 * incremental save, then used for recovery, in conjunction with the original
 * file.  This means that the history record format must allow forward and
 * backward traversal at all times.
 *
 * User input that does not involve explicit point movement causes the history
 * log to grow by approximately one byte per inserted/deleted character.  Log
 * records have a fixed overhead of 18 bytes.  A log record has the following
 * format:
 *
 *     T : Record tag.
 *     A : Record auxiliary data (data length or bpos), big endian (network byte
 *         order).
 *   ... : Data, if any.  Data are stored in the logical order that the user
 *         would type them in.
 *
 *     TAAAAAAAA...TAAAAAAAA
 *    /\         /\        /\
 *    ||         ||        ||
 *   hbeg       hcur      hend
 *
 * Three markers are used when traversing the history.  hbeg and hend bracket
 * the current record, and hcur is at the current position within the record for
 * insert, yank, remove, and delete records.
 *
 * The following notation is used in discussing the history buffer:
 *
 * Record : Meaning
 * =======:============================================
 * B      : Group begin.
 * E      : Group end.
 * P(n,m) : Buffer position change.
 * I(s)   : Insert string s before point.
 * Y(s)   : Insert (yank) string s after point.
 * R(s)   : Remove string s before point.
 * D(s)   : Remove (delete) string s after point.
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

/* histh. */
CW_P_INLINE void
histh_p_new(cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);
    
    a_histh->bufv[0].data = &a_histh->tag;
    a_histh->bufv[0].len = sizeof(a_histh->tag);

    a_histh->bufv[1].data = a_histh->u.str;
    a_histh->bufv[1].len = sizeof(a_histh->u.aux);

#ifdef CW_DBG
    a_histh->magic = CW_HISTH_MAGIC;
#endif
}

CW_P_INLINE void
histh_p_delete(cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

#ifdef CW_DBG
    a_histh->magic = 0;
#endif
}

CW_P_INLINE cw_bufv_t *
histh_p_bufv_get(cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    return &a_histh->bufv;
}

CW_P_INLINE cw_uint32_t
histh_p_bufvcnt_get(cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    return (sizeof(a_histh->bufv) / sizeof(cw_bufv_t));
}

CW_P_INLINE cw_uint8_t
histh_p_tag_get(cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    return a_histh->tag;
}

CW_P_INLINE void
histh_p_tag_set(cw_histh_t *a_histh, cw_uint8_t a_tag)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    a_histh->tag = a_tag;
}

CW_P_INLINE cw_uint64_t
histh_p_aux_get(cw_histh_t *a_histh)
{
    cw_uint64_t retval;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    retval = cw_ntohq(a_histh->u.aux);

    return retval;
}

CW_P_INLINE void
histh_p_aux_set(cw_histh_t *a_histh, cw_uint64_t a_aux)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    a_histh->u.aux = cw_htonq(a_aux);
}

/* Get the header that precedes a_a.  Various code relies on the fact that the
 * header is bracketed by a_b..a_a after this call. */
CW_P_INLINE void
histh_p_before_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b)
{
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(mkr_pos(a_a) > 1);

    mkr_dup(a_b, a_a);
    mkr_seek(a_b, -HISTH_LEN, BUFW_REL);
    bufv = mkr_range_get(a_b, a_a, &bufvcnt);

    bufv_copy(a_histh->bufv, 2, bufv, bufvcnt, 0);
    a_histh->aux = cw_ntohq(a_histh->u.aux);
}

/* Get the header that follows a_a.  Various code relies on the fact that the
 * header is bracketed by a_a..a_b after this call. */
CW_P_INLINE void
histh_p_after_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b)
{
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    mkr_dup(a_b, a_a);
    mkr_seek(a_b, HISTH_LEN, BUFW_REL);
    cw_assert(mkr_pos(a_a) + HISTH_LEN == mkr_pos(a_b));
    bufv = mkr_range_get(a_a, a_b, &bufvcnt);

    bufv_copy(a_histh->bufv, 2, bufv, bufvcnt, 0);
    a_histh->aux = cw_ntohq(a_histh->u.aux);
}

/* hist. */
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
    cw_histh_t hdr;

    cw_assert(&a_hist->h != NULL);
    cw_assert(mkr_pos(&a_hist->hcur) == buf_len(&a_hist->h) + 1);
    cw_assert(a_bpos != a_hist->hbpos);

    histh_p_new(&hdr);

    if (a_hist->hbpos == 0)
    {
	/* There's no need for an initial position record. */
	a_hist->hbpos = a_bpos;
	goto RETURN;
    }

    /* Record header. */
    hdr.tag = HISTH_TAG_POS;
    /* Old position. */
    hdr.aux = a_hist->hbpos;
    hdr.u.aux = cw_htonq(hdr.aux);

    mkr_before_insert(&a_hist->hcur, hdr.bufv, 2);

    /* Update hbpos now that the history record is complete. */
    a_hist->hbpos = a_bpos;

    RETURN:
    histh_p_delete(&hdr);
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
    cw_histh_t undo_hdr;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_bufv);

    histh_p_new(&undo_hdr);

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
	case HISTH_TAG_INS:
	{
	    a_hist->hbpos -= cnt;
	    break;
	}
	case HISTH_TAG_REM:
	{
	    a_hist->hbpos += cnt;
	    break;
	}
	case HISTH_TAG_YNK:
	case HISTH_TAG_DEL:
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
	case HISTH_TAG_YNK:
	case HISTH_TAG_REM:
	{
	    break;
	}
	case HISTH_TAG_INS:
	case HISTH_TAG_DEL:
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
    histh_p_delete(&hdr);
}

cw_hist_t *
hist_new(cw_opaque_alloc_t *a_alloc, cw_opaque_realloc_t *a_realloc,
	 cw_opaque_dealloc_t *a_dealloc, void *a_arg)
{
    cw_hist_t *retval;

    retval = (cw_hist_t *) cw_opaque_alloc(a_alloc, a_arg, sizeof(cw_hist_t));
    buf_new(&retval->h, a_alloc, a_realloc, a_dealloc, a_arg);
    mkr_new(&retval->hbeg, &retval->h);
    mkr_new(&retval->hcur, &retval->h);
    mkr_new(&retval->hend, &retval->h);
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

    mkr_delete(&a_hist->htmp);
    mkr_delete(&a_hist->hend);
    mkr_delete(&a_hist->hcur);
    mkr_delete(&a_hist->hbeg);
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
    cw_histh_t undo_hdr;
    cw_histh_t redo_hdr;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    histh_p_new(&undo_hdr);
    histh_p_new(&redo_hdr);

    mkr_new(&tmkr, a_buf);

    /* Iteratively undo a_count times. */
    for (retval = 0;
	 retval < a_count && mkr_pos(&a_hist->hcur) > 1;
	 )
    {
	hist_p_undo_header_get(&a_hist->hcur, &a_hist->htmp, &undo_hdr);

	switch (undo_hdr.tag)
	{
	    case HISTH_TAG_INS:
	    case HISTH_TAG_YNK:
	    case HISTH_TAG_REM:
	    case HISTH_TAG_DEL:
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
		    case HISTH_TAG_INS:
		    {
			bufv.data = &c;
			bufv.len = sizeof(c);
			mkr_l_insert(a_mkr, FALSE, FALSE, &bufv, 1);
			a_hist->hbpos++;
			break;
		    }
		    case HISTH_TAG_REM:
		    {
			mkr_dup(&tmkr, a_mkr);
			mkr_seek(&tmkr, -1, BUFW_REL);
			mkr_l_remove(&tmkr, a_mkr, FALSE);
			a_hist->hbpos--;
			break;
		    }
		    case HISTH_TAG_YNK:
		    {
			bufv.data = &c;
			bufv.len = sizeof(c);
			mkr_l_insert(a_mkr, FALSE, TRUE, &bufv, 1);
			break;
		    }
		    case HISTH_TAG_DEL:
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
	    case HISTH_TAG_GRP_BEG:
	    {
		a_hist->gdepth--;
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}
		mkr_seek(&a_hist->hcur, -HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_GRP_END:
	    {
		a_hist->gdepth++;
		mkr_seek(&a_hist->hcur, -HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		/* Read the undo record. */
		hist_p_undo_header_get(&a_hist->hcur, &a_hist->htmp, &undo_hdr);

		/* Set up the redo record. */
		redo_hdr.tag = HISTH_TAG_POS;
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
    hist_p_delete(&redo_hdr);
    hist_p_delete(&undo_hdr);
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
    cw_histh_t undo_hdr;
    cw_histh_t redo_hdr;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    histh_p_new(&undo_hdr);
    histh_p_new(&redo_hdr);

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
	    case HISTH_TAG_INS:
	    case HISTH_TAG_YNK:
	    case HISTH_TAG_REM:
	    case HISTH_TAG_DEL:
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
		    case HISTH_TAG_INS:
		    {
			bufv.data = &c;
			bufv.len = sizeof(c);
			mkr_l_insert(a_mkr, FALSE, FALSE, &bufv, 1);
			a_hist->hbpos++;
			break;
		    }
		    case HISTH_TAG_REM:
		    {
			mkr_dup(&tmkr, a_mkr);
			mkr_seek(&tmkr, -1, BUFW_REL);
			mkr_l_remove(&tmkr, a_mkr, FALSE);
			a_hist->hbpos--;
			break;
		    }
		    case HISTH_TAG_YNK:
		    {
			bufv.data = &c;
			bufv.len = sizeof(c);
			mkr_l_insert(a_mkr, FALSE, TRUE, &bufv, 1);
			break;
		    }
		    case HISTH_TAG_DEL:
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
	    case HISTH_TAG_GRP_BEG:
	    {
		a_hist->gdepth++;
		mkr_seek(&a_hist->hcur, HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_GRP_END:
	    {
		a_hist->gdepth--;
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}
		mkr_seek(&a_hist->hcur, HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		cw_histh_t hdr;

		histh_p_new(&hdr);

		/* Read the redo record. */
		hist_p_redo_header_get(&a_hist->hcur, &a_hist->htmp, &redo_hdr);

		/* Set up the undo record. */
		undo_hdr.tag = HISTH_TAG_POS;
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

    histh_p_delete(&redo_hdr);
    histh_p_delete(&undo_hdr);
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
    cw_histh_t hdr;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    histh_p_new(&hdr);

    hist_p_redo_flush(a_hist);

    /* If the position of a_mkr is not the same as the last saved postion,
     * insert a position record before creating the group begin record, so that
     * when the group is undone, the position ends up where the user would
     * expect it to. */
    if (a_mkr != NULL && mkr_pos(a_mkr) != a_hist->hbpos)
    {
	hist_p_pos(a_hist, a_buf, mkr_pos(a_mkr));
    }

    hdr.tag = HISTH_TAG_GRP_BEG;
    hdr.aux = 0;
    hdr.u.aux = 0;

    mkr_before_insert(&a_hist->hcur, hdr.bufv, 2);

    a_hist->gdepth++;

    histh_p_delete(&hdr);
}

cw_bool_t
hist_group_end(cw_hist_t *a_hist, cw_buf_t *a_buf)
{
    cw_bool_t retval;
    cw_histh_t hdr;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);

    histh_p_new(&hdr);

    if (a_hist->gdepth == 0)
    {
	/* Group ends must always be matched by beginnings. */
	retval = TRUE;
	goto RETURN;
    }

    hist_p_redo_flush(a_hist);

    hdr.tag = HISTH_TAG_GRP_END;
    hdr.aux = 0;
    hdr.u.aux = 0;

    mkr_before_insert(&a_hist->hcur, hdr.bufv, 2);

    a_hist->gdepth--;

    retval = FALSE;
    RETURN:
    histh_p_delete(&hdr);
    return retval;
}

void
hist_ins(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_REM);
}

void
hist_ynk(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_DEL);
}

void
hist_rem(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_INS);
}

void
hist_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_YNK);
}

#ifdef CW_BUF_DUMP
void
hist_dump(cw_hist_t *a_hist, const char *a_beg, const char *a_mid,
	  const char *a_end)
{
    const char *beg, *mid, *end;
    cw_histh_t hdr;
    cw_bufv_t *bufv;
    cw_mkr_t tmkr, ttmkr;
    cw_uint32_t bufvcnt;
#ifdef CW_HIST_DUMP
    char *tbeg, *tmid;
#endif

    histh_p_new(&hdr);

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
	    case HISTH_TAG_GRP_BEG:
	    {
		fprintf(stderr, "%s|         B\n", mid);
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_GRP_END:
	    {
		fprintf(stderr, "%s|         E\n", mid);
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		fprintf(stderr, "%s|         (%llu)P\n", mid, hdr.aux);
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_INS:
	    {
		fprintf(stderr, "%s|         (", mid);
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")%lluI\n", hdr.aux);
		break;
	    }
	    case HISTH_TAG_YNK:
	    {
		fprintf(stderr, "%s|         (", mid);
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")%lluY\n", hdr.aux);
		break;
	    }
	    case HISTH_TAG_REM:
	    {
		fprintf(stderr, "%s|         (", mid);
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")%lluR\n", hdr.aux);
		break;
	    }
	    case HISTH_TAG_DEL:
	    {
		fprintf(stderr, "%s|         (", mid);
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
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
	    case HISTH_TAG_GRP_BEG:
	    {
		fprintf(stderr, "%s|         B\n", mid);
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_GRP_END:
	    {
		fprintf(stderr, "%s|         E\n", mid);
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		fprintf(stderr, "%s|         P(%llu)\n", mid, hdr.aux);
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_INS:
	    {
		fprintf(stderr, "%s|         I%llu(", mid, hdr.aux);
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")\n");
		break;
	    }
	    case HISTH_TAG_YNK:
	    {
		fprintf(stderr, "%s|         Y%llu(\n", mid, hdr.aux);
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")\n");
		break;
	    }
	    case HISTH_TAG_REM:
	    {
		fprintf(stderr, "%s|         R%llu(", mid, hdr.aux);
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		mkr_dup(&ttmkr, &tmkr);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		bufv = mkr_range_get(&tmkr, &ttmkr, &bufvcnt);
		hist_p_bufv_print(bufv, bufvcnt);
		fprintf(stderr, ")\n");
		break;
	    }
	    case HISTH_TAG_DEL:
	    {
		fprintf(stderr, "%s|         D%llu(", mid, hdr.aux);
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
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
    histh_p_delete(&hdr);
}
#endif

#ifdef CW_BUF_VALIDATE
void
hist_validate(cw_hist_t *a_hist, cw_buf_t *a_buf)
{
    cw_mkr_t tmkr, ttmkr;
    cw_histh_t hdr;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);

    histh_p_new(&hdr);

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
	    case HISTH_TAG_GRP_BEG:
	    {
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_GRP_END:
	    {
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_INS:
	    {
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_YNK:
	    {
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_REM:
	    {
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
		mkr_seek(&tmkr, -(cw_sint64_t) hdr.aux, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_DEL:
	    {
		mkr_seek(&tmkr, -HISTH_LEN, BUFW_REL);
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
	    case HISTH_TAG_GRP_BEG:
	    {
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_GRP_END:
	    {
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_INS:
	    {
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_YNK:
	    {
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_REM:
	    {
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
		mkr_seek(&tmkr, hdr.aux, BUFW_REL);
		break;
	    }
	    case HISTH_TAG_DEL:
	    {
		mkr_seek(&tmkr, HISTH_LEN, BUFW_REL);
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
    histh_p_delete(&hdr);
}
#endif
