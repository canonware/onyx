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

    a_histh->tag = HISTH_TAG_NONE;
    a_histh->u.aux = 0;

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

    return a_histh->bufv;
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
    mkr_seek(a_b, -(cw_sint64_t)HISTH_LEN, BUFW_REL);
    bufv = mkr_range_get(a_b, a_a, &bufvcnt);
    cw_check_ptr(bufv);

    bufv_copy(a_histh->bufv, 2, bufv, bufvcnt, 0);
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
    cw_check_ptr(bufv);

    bufv_copy(a_histh->bufv, 2, bufv, bufvcnt, 0);
}

/* Move to the previous record. */
CW_P_INLINE void
histh_p_record_prev(cw_histh_t *a_head, cw_histh_t *a_foot, cw_buf_t *a_buf,
		    cw_mkr_t *a_beg, cw_mkr_t *a_cur, cw_mkr_t *a_end,
		    cw_mkr_t *a_tmp)
{
    cw_bool_t again;

    /* Loop until a normal record is found, or there are no previous records. */
    for (again = TRUE; again;)
    {
	again = FALSE;

	/* Set a_end. */
	mkr_dup(a_end, a_beg);

	if (mkr_pos(a_end) == 1)
	{
	    /* No previous records. */
	    mkr_dup(a_cur, a_end);
	    histh_p_tag_set(a_head, HISTH_TAG_NONE);
	    histh_p_tag_set(a_foot, HISTH_TAG_NONE);
	    break;
	}

	/* Get footer. */
	histh_p_before_get(a_foot, a_end, a_cur);

	switch (histh_p_tag_get(a_foot))
	{
	    case HISTH_TAG_SYNC:
	    {
		/* Ignore sync records. */
		again = TRUE;
		/* Fall through. */
	    }
	    case HISTH_TAG_GRP_BEG:
	    case HISTH_TAG_GRP_END:
	    case HISTH_TAG_POS:
	    {
		/* Get header. */
		histh_p_before_get(a_head, a_cur, a_beg);

		break;
	    }
	    case HISTH_TAG_INS:
	    case HISTH_TAG_YNK:
	    case HISTH_TAG_REM:
	    case HISTH_TAG_DEL:
	    {
		/* Skip data. */
		mkr_dup(a_tmp, a_cur);
		mkr_seek(a_tmp, -(cw_sint64_t)histh_p_aux_get(a_foot),
			 BUFW_REL);

		/* Get header. */
		histh_p_before_get(a_head, a_tmp, a_beg);

		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
}

/* Move to the next record. */
CW_P_INLINE void
histh_p_record_next(cw_histh_t *a_head, cw_histh_t *a_foot, cw_buf_t *a_buf,
		    cw_mkr_t *a_beg, cw_mkr_t *a_cur, cw_mkr_t *a_end,
		    cw_mkr_t *a_tmp)
{
    cw_bool_t again;

    /* Loop until a normal record is found, or there are no following
     * records. */
    for (again = TRUE; again;)
    {
	again = FALSE;

	/* Set a_beg. */
	mkr_dup(a_beg, a_end);

	if (mkr_pos(a_beg) == buf_len(a_buf) + 1)
	{
	    /* No following records. */
	    mkr_dup(a_cur, a_beg);
	    histh_p_tag_set(a_head, HISTH_TAG_NONE);
	    histh_p_tag_set(a_foot, HISTH_TAG_NONE);
	    break;
	}

	/* Get header. */
	histh_p_after_get(a_head, a_beg, a_cur);

	switch (histh_p_tag_get(a_head))
	{
	    case HISTH_TAG_SYNC:
	    {
		/* Ignore sync records. */
		again = TRUE;
		/* Fall through. */
	    }
	    case HISTH_TAG_GRP_BEG:
	    case HISTH_TAG_GRP_END:
	    case HISTH_TAG_POS:
	    {
		/* Get footer. */
		histh_p_after_get(a_foot, a_cur, a_end);

		break;
	    }
	    case HISTH_TAG_INS:
	    case HISTH_TAG_YNK:
	    case HISTH_TAG_REM:
	    case HISTH_TAG_DEL:
	    {
		/* Skip data. */
		mkr_dup(a_tmp, a_cur);
		mkr_seek(a_tmp, histh_p_aux_get(a_head), BUFW_REL);

		/* Get footer. */
		histh_p_after_get(a_foot, a_tmp, a_end);

		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
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

/* Move to the previous record in h. */
static void
hist_p_record_prev(cw_hist_t *a_hist)
{
    histh_p_record_prev(&a_hist->hhead, &a_hist->hfoot, &a_hist->h,
			&a_hist->hbeg, &a_hist->hcur, &a_hist->hend,
			&a_hist->htmp);
}

/* Move to the next record in h. */
static void
hist_p_record_next(cw_hist_t *a_hist)
{
    histh_p_record_next(&a_hist->hhead, &a_hist->hfoot, &a_hist->h,
			&a_hist->hbeg, &a_hist->hcur, &a_hist->hend,
			&a_hist->htmp);
}

/* Flush redo state, if any. */
CW_P_INLINE void
hist_p_redo_flush(cw_hist_t *a_hist)
{
    /* Flush all records after the current one. */
    if (mkr_pos(&a_hist->hend) != buf_len(&a_hist->h) + 1)
    {
	mkr_seek(&a_hist->htmp, 0, BUFW_EOB);
	mkr_remove(&a_hist->hend, &a_hist->htmp);
    }

    /* Flush all data between hcur and the end of the current record. */
    if (mkr_pos(&a_hist->hcur) + HISTH_LEN < mkr_pos(&a_hist->hend))
    {
	mkr_dup(&a_hist->htmp, &a_hist->hend);
	mkr_seek(&a_hist->htmp, -(cw_sint64_t)HISTH_LEN, BUFW_REL);
	mkr_remove(&a_hist->hcur, &a_hist->hend);
    }
    else if (mkr_pos(&a_hist->hbeg) == mkr_pos(&a_hist->hend)
	     && mkr_pos(&a_hist->hbeg) > 1)
    {
	/* Move to the previous record in order to assure that appending to the
	 * history doesn't inadvertently create consecutive records of the same
	 * type. */
	hist_p_record_prev(a_hist);
    }
}

/* Insert a position record into the log.   */
CW_P_INLINE void
hist_p_pos(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos)
{
    cw_assert(&a_hist->h != NULL);
    cw_assert(mkr_pos(&a_hist->hend) == buf_len(&a_hist->h) + 1);
    cw_assert(a_bpos != a_hist->hbpos);

    if (a_hist->hbpos == 0)
    {
	/* There's no need for an initial position record. */
	a_hist->hbpos = a_bpos;
	return;
    }

    /* Initialize header (old position). */
    histh_p_tag_set(&a_hist->hhead, HISTH_TAG_POS);
    histh_p_aux_set(&a_hist->hhead, a_hist->hbpos);

    /* Relocate hbeg. */
    mkr_dup(&a_hist->hbeg, &a_hist->hend);

    /* Insert header. */
    mkr_after_insert(&a_hist->hbeg, histh_p_bufv_get(&a_hist->hhead),
		     histh_p_bufvcnt_get(&a_hist->hhead));

    /* Relocate hcur. */
    mkr_dup(&a_hist->hcur, &a_hist->hbeg);
    mkr_seek(&a_hist->hcur, HISTH_LEN, BUFW_REL);

    /* Initialize footer (new position). */
    histh_p_tag_set(&a_hist->hfoot, HISTH_TAG_POS);
    histh_p_aux_set(&a_hist->hfoot, a_bpos);

    /* Insert footer. */
    mkr_after_insert(&a_hist->hcur, histh_p_bufv_get(&a_hist->hfoot),
		     histh_p_bufvcnt_get(&a_hist->hfoot));

    /* Relocate hend. */
    mkr_dup(&a_hist->hend, &a_hist->hcur);
    mkr_seek(&a_hist->hend, HISTH_LEN, BUFW_REL);

    /* Update hbpos now that the history record is complete. */
    a_hist->hbpos = a_bpos;
}

static void
hist_p_ins_ynk_rem_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
		       const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt,
		       cw_uint8_t a_tag)
{
    cw_uint64_t cnt;
    cw_uint32_t i;

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
	    a_hist->hbpos += cnt;
	    break;
	}
	case HISTH_TAG_REM:
	{
	    a_hist->hbpos -= cnt;
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

    /* Update the data count if extending an existing record.  Otherwise, create
     * a new empty record. */
    if (mkr_pos(&a_hist->hend) > 1 && histh_p_tag_get(&a_hist->hhead) == a_tag)
    {
	/* Update the data count.  Be careful to leave hcur in the correct
	 * location (just before the footer). */

	/* Header. */
	histh_p_aux_set(&a_hist->hhead, histh_p_aux_get(&a_hist->hhead) + cnt);
	mkr_dup(&a_hist->htmp, &a_hist->hbeg);
	mkr_seek(&a_hist->htmp, HISTH_LEN, BUFW_REL);
	mkr_remove(&a_hist->hbeg, &a_hist->htmp);
	mkr_after_insert(&a_hist->hbeg, histh_p_bufv_get(&a_hist->hhead),
			 histh_p_bufvcnt_get(&a_hist->hhead));
	mkr_seek(&a_hist->htmp, HISTH_LEN, BUFW_REL);

	/* Footer. */
	histh_p_aux_set(&a_hist->hfoot, histh_p_aux_get(&a_hist->hfoot) + cnt);
	mkr_remove(&a_hist->hcur, &a_hist->hend);
	mkr_before_insert(&a_hist->hend, histh_p_bufv_get(&a_hist->hfoot),
			  histh_p_bufvcnt_get(&a_hist->hfoot));
	mkr_seek(&a_hist->hcur, -(cw_sint64_t)HISTH_LEN, BUFW_REL);
    }
    else
    {
	/* Initialize header. */
	histh_p_tag_set(&a_hist->hhead, a_tag);
	histh_p_aux_set(&a_hist->hhead, cnt);

	/* Relocate hbeg. */
	mkr_dup(&a_hist->hbeg, &a_hist->hend);

	/* Insert header. */
	mkr_after_insert(&a_hist->hbeg, histh_p_bufv_get(&a_hist->hhead),
			 histh_p_bufvcnt_get(&a_hist->hhead));

	/* Relocate hcur. */
	mkr_dup(&a_hist->hcur, &a_hist->hbeg);
	mkr_seek(&a_hist->hcur, HISTH_LEN, BUFW_REL);

	/* Initialize footer. */
	histh_p_tag_set(&a_hist->hfoot, a_tag);
	histh_p_aux_set(&a_hist->hfoot, cnt);

	/* Insert footer. */
	mkr_after_insert(&a_hist->hcur, histh_p_bufv_get(&a_hist->hfoot),
			 histh_p_bufvcnt_get(&a_hist->hfoot));

	/* Relocate hend. */
	mkr_dup(&a_hist->hend, &a_hist->hcur);
	mkr_seek(&a_hist->hend, HISTH_LEN, BUFW_REL);
    }

    /* Insert the data in the appropriate order for the record type. */
    switch (a_tag)
    {
	case HISTH_TAG_INS:
	case HISTH_TAG_DEL:
	{
	    /* Insert in forward order. */
	    mkr_before_insert(&a_hist->hcur, a_bufv, a_bufvcnt);
	    break;
	}
	case HISTH_TAG_YNK:
	case HISTH_TAG_REM:
	{
	    /* Insert in reverse order. */
	    mkr_l_insert(&a_hist->hcur, TRUE, FALSE, a_bufv, a_bufvcnt, TRUE);

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
}

cw_hist_t *
hist_new(cw_opaque_alloc_t *a_alloc, cw_opaque_realloc_t *a_realloc,
	 cw_opaque_dealloc_t *a_dealloc, void *a_arg)
{
    cw_hist_t *retval;

    retval = (cw_hist_t *) cw_opaque_alloc(a_alloc, a_arg, sizeof(cw_hist_t));
    buf_new(&retval->h, a_alloc, a_realloc, a_dealloc, a_arg);
    histh_p_new(&retval->hhead);
    histh_p_new(&retval->hfoot);
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
    histh_p_delete(&a_hist->hfoot);
    histh_p_delete(&a_hist->hhead);
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
    cw_uint8_t *p;
    cw_mkr_t tmkr;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    mkr_new(&tmkr, a_buf);

    if (histh_p_tag_get(&a_hist->hhead) == HISTH_TAG_NONE
	&& mkr_pos(&a_hist->hcur) > 1)
    {
	/* Move to the previous record, since we're currently at the end of the
	 * history. */
	hist_p_record_prev(a_hist);
    }

    /* Iteratively undo a_count times. */
    for (retval = 0;
	 retval < a_count && mkr_pos(&a_hist->hcur) > 1;
	 )
    {
	switch (histh_p_tag_get(&a_hist->hhead))
	{
	    case HISTH_TAG_INS:
	    case HISTH_TAG_YNK:
	    case HISTH_TAG_REM:
	    case HISTH_TAG_DEL:
	    {
		cw_assert(mkr_pos(&a_hist->hcur)
			  > mkr_pos(&a_hist->hbeg) + HISTH_LEN);

		/* Take action. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);
		switch (histh_p_tag_get(&a_hist->hhead))
		{
		    case HISTH_TAG_INS:
		    {
			/* Remove character. */
			mkr_dup(&tmkr, a_mkr);
			mkr_seek(&tmkr, -1, BUFW_REL);
			mkr_l_remove(&tmkr, a_mkr, FALSE);

			/* Adjust bpos. */
			a_hist->hbpos--;
			break;
		    }
		    case HISTH_TAG_YNK:
		    {
			/* Remove character. */
			mkr_dup(&tmkr, a_mkr);
			mkr_seek(&tmkr, 1, BUFW_REL);
			mkr_l_remove(a_mkr, &tmkr, FALSE);
			break;
		    }
		    case HISTH_TAG_REM:
		    {
			/* Get character. */
			p = mkr_before_get(&a_hist->hcur);

			/* Set up bufv. */
			bufv.data = p;
			bufv.len = sizeof(*p);

			/* Insert bufv. */
			mkr_l_insert(a_mkr, FALSE, FALSE, &bufv, 1, FALSE);

			/* Adjust bpos. */
			a_hist->hbpos++;
			break;
		    }
		    case HISTH_TAG_DEL:
		    {
			/* Get character. */
			p = mkr_before_get(&a_hist->hcur);

			/* Set up bufv. */
			bufv.data = p;
			bufv.len = sizeof(*p);

			/* Insert bufv. */
			mkr_l_insert(a_mkr, FALSE, TRUE, &bufv, 1, FALSE);
			break;
		    }
		    default:
		    {
			cw_not_reached();
		    }
		}

		/* Move to previous character. */
		mkr_seek(&a_hist->hcur, -1LL, BUFW_REL);

		/* Increment the undo count, if the group depth is 0. */
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}

		/* Move to the previous record if all data in this record have
		 * already been undone. */
		if (mkr_pos(&a_hist->hcur)
		    == mkr_pos(&a_hist->hbeg) + HISTH_LEN)
		{
		    hist_p_record_prev(a_hist);
		}
		break;
	    }
	    case HISTH_TAG_GRP_BEG:
	    {
		/* Decrease depth. */
		a_hist->gdepth--;

		/* Increment the undo count, if the group depth dropped to 0. */
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}

		/* Get the previous record. */
		hist_p_record_prev(a_hist);
		break;
	    }
	    case HISTH_TAG_GRP_END:
	    {
		/* Increase depth. */
		a_hist->gdepth++;

		/* Get the previous record. */
		hist_p_record_prev(a_hist);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		/* Update hbpos. */
		a_hist->hbpos = histh_p_aux_get(&a_hist->hhead);

		/* Move. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);

		/* Get the previous record. */
		hist_p_record_prev(a_hist);
		break;
	    }
	    case HISTH_TAG_SYNC:
	    {
		/* Remove sync records. */
		mkr_remove(&a_hist->hbeg, &a_hist->hend);

		/* Get the previous record. */
		hist_p_record_prev(a_hist);
		break;
	    }
	    case HISTH_TAG_NONE:
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
    cw_uint8_t *p;
    cw_mkr_t tmkr;
    cw_bufv_t bufv;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    mkr_new(&tmkr, a_buf);

    if (histh_p_tag_get(&a_hist->hhead) == HISTH_TAG_NONE
	&& mkr_pos(&a_hist->hcur) < buf_len(&a_hist->h) + 1)
    {
	/* Move to the next record, since we're currently at the begin of the
	 * history. */
	hist_p_record_next(a_hist);
    }

    /* Iteratively redo a_count times. */
    for (retval = 0, redid = FALSE;
	 retval < a_count && mkr_pos(&a_hist->hcur) < buf_len(&a_hist->h) + 1;
	 )
    {
	/* Make a note that the redo loop was executed at least once, so that a
	 * correct decision about whether an incomplete group was redone can be
	 * made after the loop terminates. */
	redid = TRUE;

	switch (histh_p_tag_get(&a_hist->hhead))
	{
	    case HISTH_TAG_INS:
	    case HISTH_TAG_YNK:
	    case HISTH_TAG_REM:
	    case HISTH_TAG_DEL:
	    {
		cw_assert(mkr_pos(&a_hist->hcur)
			  < mkr_pos(&a_hist->hend) - HISTH_LEN);

		/* Take action. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);
		switch(histh_p_tag_get(&a_hist->hhead))
		{
		    case HISTH_TAG_INS:
		    {
			/* Get character. */
			p = mkr_after_get(&a_hist->hcur);

			/* Set up bufv. */
			bufv.data = p;
			bufv.len = sizeof(*p);

			/* Insert bufv. */
			mkr_l_insert(a_mkr, FALSE, FALSE, &bufv, 1, FALSE);

			/* Adjust bpos. */
			a_hist->hbpos++;
			break;
		    }
		    case HISTH_TAG_YNK:
		    {
			/* Get character. */
			p = mkr_after_get(&a_hist->hcur);

			/* Set up bufv. */
			bufv.data = p;
			bufv.len = sizeof(*p);

			/* Insert bufv. */
			mkr_l_insert(a_mkr, FALSE, TRUE, &bufv, 1, FALSE);
			break;
		    }
		    case HISTH_TAG_REM:
		    {
			/* Remove character. */
			mkr_dup(&tmkr, a_mkr);
			mkr_seek(&tmkr, -1, BUFW_REL);
			mkr_l_remove(&tmkr, a_mkr, FALSE);

			/* Adjust bpos. */
			a_hist->hbpos--;
			break;
		    }
		    case HISTH_TAG_DEL:
		    {
			/* Remove character. */
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

		/* Move to the next character. */
		mkr_seek(&a_hist->hcur, 1LL, BUFW_REL);

		/* Increment the redo count, if the group depth is 0. */
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}

		/* Move to the next record if all data in this record have
		 * already been redone. */
		if (mkr_pos(&a_hist->hcur)
		    == mkr_pos(&a_hist->hend) - HISTH_LEN)
		{
		    hist_p_record_next(a_hist);
		}
		break;
	    }
	    case HISTH_TAG_GRP_BEG:
	    {
		/* Increase depth. */
		a_hist->gdepth++;

		/* Get the next record. */
		hist_p_record_next(a_hist);
		break;
	    }
	    case HISTH_TAG_GRP_END:
	    {
		/* Decrease depth. */
		a_hist->gdepth--;

		/* Increment the redo count, if the group depth dropped to 0. */
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}

		/* Get the next record. */
		hist_p_record_next(a_hist);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		/* Update hbpos. */
		a_hist->hbpos = histh_p_aux_get(&a_hist->hfoot);

		/* Move. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);

		/* Get the next record. */
		hist_p_record_next(a_hist);
		break;
	    }
	    case HISTH_TAG_SYNC:
	    {
		/* Remove sync records. */
		mkr_remove(&a_hist->hbeg, &a_hist->hend);

		/* Get the next record. */
		hist_p_record_next(a_hist);
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

    mkr_seek(&a_hist->hbeg, 0, BUFW_BOB);
    mkr_seek(&a_hist->hend, 0, BUFW_EOB);
    mkr_remove(&a_hist->hbeg, &a_hist->hend);
    histh_p_tag_set(&a_hist->hhead, HISTH_TAG_NONE);
    histh_p_tag_set(&a_hist->hfoot, HISTH_TAG_NONE);
    a_hist->hbpos = 0;
    a_hist->gdepth = 0;
}

void
hist_group_beg(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_mkr_t *a_mkr)
{
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

    /* Initialize header. */
    histh_p_tag_set(&a_hist->hhead, HISTH_TAG_GRP_BEG);
    histh_p_aux_set(&a_hist->hhead, 0);

    /* Relocate hbeg. */
    mkr_dup(&a_hist->hbeg, &a_hist->hend);

    /* Insert header. */
    mkr_after_insert(&a_hist->hbeg, histh_p_bufv_get(&a_hist->hhead),
		     histh_p_bufvcnt_get(&a_hist->hhead));

    /* Relocate hcur. */
    mkr_dup(&a_hist->hcur, &a_hist->hbeg);
    mkr_seek(&a_hist->hcur, HISTH_LEN, BUFW_REL);

    /* Initialize footer. */
    histh_p_tag_set(&a_hist->hfoot, HISTH_TAG_GRP_BEG);
    histh_p_aux_set(&a_hist->hfoot, 0);

    /* Insert footer. */
    mkr_after_insert(&a_hist->hcur, histh_p_bufv_get(&a_hist->hfoot),
		     histh_p_bufvcnt_get(&a_hist->hfoot));

    /* Relocate hend. */
    mkr_dup(&a_hist->hend, &a_hist->hcur);
    mkr_seek(&a_hist->hend, HISTH_LEN, BUFW_REL);

    /* Increase depth. */
    a_hist->gdepth++;
}

cw_bool_t
hist_group_end(cw_hist_t *a_hist, cw_buf_t *a_buf)
{
    cw_bool_t retval;

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

    /* Initialize header. */
    histh_p_tag_set(&a_hist->hhead, HISTH_TAG_GRP_BEG);
    histh_p_aux_set(&a_hist->hhead, 0);

    /* Relocate hbeg. */
    mkr_dup(&a_hist->hbeg, &a_hist->hend);

    /* Insert header. */
    mkr_after_insert(&a_hist->hbeg, histh_p_bufv_get(&a_hist->hhead),
		     histh_p_bufvcnt_get(&a_hist->hhead));

    /* Relocate hcur. */
    mkr_dup(&a_hist->hcur, &a_hist->hbeg);
    mkr_seek(&a_hist->hcur, HISTH_LEN, BUFW_REL);

    /* Initialize footer. */
    histh_p_tag_set(&a_hist->hfoot, HISTH_TAG_GRP_BEG);
    histh_p_aux_set(&a_hist->hfoot, 0);

    /* Insert footer. */
    mkr_after_insert(&a_hist->hcur, histh_p_bufv_get(&a_hist->hfoot),
		     histh_p_bufvcnt_get(&a_hist->hfoot));

    /* Relocate hend. */
    mkr_dup(&a_hist->hend, &a_hist->hcur);
    mkr_seek(&a_hist->hend, HISTH_LEN, BUFW_REL);

    /* Decrease depth. */
    a_hist->gdepth--;

    retval = FALSE;
    RETURN:
    return retval;
}

void
hist_ins(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_INS);
}

void
hist_ynk(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_YNK);
}

void
hist_rem(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_REM);
}

void
hist_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_DEL);
}

#ifdef CW_BUF_DUMP
void
hist_dump(cw_hist_t *a_hist, const char *a_beg, const char *a_mid,
	  const char *a_end)
{
    cw_histh_t thead, tfoot;
    cw_mkr_t tbeg, tcur, tend, ttmp;
    const char *beg, *mid, *end;
#ifdef CW_HIST_DUMP
    char *pbeg, *pmid;
#endif
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    histh_p_new(&thead);
    histh_p_new(&tfoot);

    beg = (a_beg != NULL) ? a_beg : "";
    mid = (a_mid != NULL) ? a_mid : beg;
    end = (a_end != NULL) ? a_end : mid;

    fprintf(stderr, "%shist: %p\n", beg, a_hist);

#ifdef CW_HIST_DUMP
    /* h. */
    fprintf(stderr, "%s|\n", mid);
    asprintf(&pbeg, "%s|-> h: ", mid);
    asprintf(&pmid, "%s|      ", mid);
    buf_dump(&a_hist->h, pbeg, pmid, NULL);
    free(pbeg);
    free(pmid);

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

    /* Initialize markers after buf_dump() in order to keep them from showing
     * up. */
    mkr_new(&tbeg, &a_hist->h);
    mkr_new(&tcur, &a_hist->h);
    mkr_new(&tend, &a_hist->h);
    mkr_new(&ttmp, &a_hist->h);

    /* Undo. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s| [undo]:\n", mid);
    mkr_dup(&tbeg, &a_hist->hbeg);
    mkr_dup(&tcur, &a_hist->hcur);
    mkr_dup(&tend, &a_hist->hend);
    if (mkr_pos(&tend) > 1)
    {
	if (mkr_pos(&tbeg) == mkr_pos(&tend))
	{
	    /* No current record.  Move back to previous one. */
	    histh_p_record_prev(&thead, &tfoot, &a_hist->h, &tbeg, &tcur, &tend,
				&ttmp);
	}
	else
	{
	    /* Get current record header/footer. */
	    histh_p_before_get(&tfoot, &tend, &ttmp);
	    histh_p_after_get(&thead, &tbeg, &ttmp);
	}

	/* Iterate through records. */
	while (mkr_pos(&tend) > 1)
	{
	    switch (histh_p_tag_get(&thead))
	    {
		case HISTH_TAG_GRP_BEG:
		{
		    fprintf(stderr, "%s|         B\n", mid);
		    break;
		}
		case HISTH_TAG_GRP_END:
		{
		    fprintf(stderr, "%s|         E\n", mid);
		    break;
		}
		case HISTH_TAG_POS:
		{
		    fprintf(stderr, "%s|         (%llu,%llu)P\n", mid,
			    histh_p_aux_get(&thead), histh_p_aux_get(&tfoot));
		    break;
		}
		case HISTH_TAG_INS:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%lluI\n",
			    histh_p_aux_get(&thead));
		    break;
		}
		case HISTH_TAG_YNK:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%lluY\n",
			    histh_p_aux_get(&thead));
		    break;
		}
		case HISTH_TAG_REM:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%lluR\n",
			    histh_p_aux_get(&thead));
		    break;
		}
		case HISTH_TAG_DEL:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%lluD\n",
			    histh_p_aux_get(&thead));
		    break;
		}
		case HISTH_TAG_SYNC:
		{
		    fprintf(stderr, "%s|         S\n", mid);
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }

	    /* Get previous record. */
	    histh_p_record_prev(&thead, &tfoot, &a_hist->h, &tbeg, &tcur, &tend,
				&ttmp);
	}
    }

    /* Redo. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s| [redo]:\n", mid);
    mkr_dup(&tbeg, &a_hist->hbeg);
    mkr_dup(&tcur, &a_hist->hcur);
    mkr_dup(&tend, &a_hist->hend);
    if (mkr_pos(&tbeg) < buf_len(&a_hist->h) + 1)
    {
	/* Initialize thead/tfoot. */
	if (mkr_pos(&tbeg) == mkr_pos(&tend))
	{
	    /* No current record.  Move on to next one. */
	    histh_p_record_next(&thead, &tfoot, &a_hist->h, &tbeg, &tcur, &tend,
				&ttmp);
	}
	else
	{
	    /* Get current record header/footer. */
	    histh_p_after_get(&thead, &tbeg, &ttmp);
	    histh_p_before_get(&tfoot, &tend, &ttmp);
	}

	/* Iterate through records. */
	while (mkr_pos(&tbeg) < buf_len(&a_hist->h) + 1)
	{
	    switch (histh_p_tag_get(&thead))
	    {
		case HISTH_TAG_GRP_BEG:
		{
		    fprintf(stderr, "%s|         B\n", mid);
		    break;
		}
		case HISTH_TAG_GRP_END:
		{
		    fprintf(stderr, "%s|         E\n", mid);
		    break;
		}
		case HISTH_TAG_POS:
		{
		    fprintf(stderr, "%s|         P(%llu,%llu)\n", mid,
			    histh_p_aux_get(&thead), histh_p_aux_get(&tfoot));
		    break;
		}
		case HISTH_TAG_INS:
		{
		    fprintf(stderr, "%s|         I%llu(", mid,
			    histh_p_aux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_YNK:
		{
		    fprintf(stderr, "%s|         Y%llu(\n", mid,
			    histh_p_aux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_REM:
		{
		    fprintf(stderr, "%s|         R%llu(", mid,
			    histh_p_aux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_DEL:
		{
		    fprintf(stderr, "%s|         D%llu(", mid,
			    histh_p_aux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_SYNC:
		{
		    fprintf(stderr, "%s|         S\n", mid);
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }

	    /* Get next record. */
	    histh_p_record_next(&thead, &tfoot, &a_hist->h, &tbeg, &tcur, &tend,
				&ttmp);
	}
    }
    fprintf(stderr, "%sV\n", end);

    mkr_delete(&ttmp);
    mkr_delete(&tend);
    mkr_delete(&tcur);
    mkr_delete(&tbeg);
    histh_p_delete(&tfoot);
    histh_p_delete(&thead);
}
#endif

#ifdef CW_BUF_VALIDATE
void
hist_validate(cw_hist_t *a_hist, cw_buf_t *a_buf)
{
    cw_histh_t thead, tfoot;
    cw_mkr_t tbeg, tcur, tend, ttmp;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);

    histh_p_new(&thead);
    histh_p_new(&tfoot);

    mkr_new(&tbeg, &a_hist->h);
    mkr_new(&tcur, &a_hist->h);
    mkr_new(&tend, &a_hist->h);
    mkr_new(&ttmp, &a_hist->h);

    /* Validate h. */
    buf_validate(&a_hist->h);

    /* Validate that hbeg, hcur, hend, and htmp are in h. */
    cw_assert(a_hist->hbeg.bufp->buf == &a_hist->h);
    cw_assert(a_hist->hcur.bufp->buf == &a_hist->h);
    cw_assert(a_hist->hend.bufp->buf == &a_hist->h);
    cw_assert(a_hist->htmp.bufp->buf == &a_hist->h);

    /* Validate that hpbos is a legal bpos in the data buf. */
    cw_assert(a_hist->hbpos >= 1
	      || buf_len(&a_hist->h) == 0);
    cw_assert(a_hist->hbpos <= buf_len(a_buf) + 1);

    /* Iterate through the undo history and validate record consistency. */
    mkr_dup(&tbeg, &a_hist->hbeg);
    mkr_dup(&tcur, &a_hist->hcur);
    mkr_dup(&tend, &a_hist->hend);
    if (mkr_pos(&tend) > 1)
    {
	if (mkr_pos(&tbeg) == mkr_pos(&tend))
	{
	    /* No current record.  Move back to previous one. */
	    histh_p_record_prev(&thead, &tfoot, &a_hist->h, &tbeg, &tcur, &tend,
				&ttmp);
	}
	else
	{
	    /* Get current record header/footer. */
	    histh_p_before_get(&tfoot, &tend, &ttmp);
	    histh_p_after_get(&thead, &tbeg, &ttmp);
	}

	/* Iterate through records. */
	while (mkr_pos(&tend) > 1)
	{
	    cw_assert(histh_p_tag_get(&thead) == histh_p_tag_get(&tfoot));

	    switch (histh_p_tag_get(&thead))
	    {
		case HISTH_TAG_GRP_BEG:
		case HISTH_TAG_GRP_END:
		case HISTH_TAG_SYNC:
		{
		    cw_assert(histh_p_aux_get(&thead) == 0);
		    cw_assert(histh_p_aux_get(&tfoot) == 0);
		    break;
		}
		case HISTH_TAG_POS:
		{
		    cw_assert(histh_p_aux_get(&thead) != 0);
		    cw_assert(histh_p_aux_get(&tfoot) != 0);
		    break;
		}
		case HISTH_TAG_INS:
		case HISTH_TAG_YNK:
		case HISTH_TAG_REM:
		case HISTH_TAG_DEL:
		{
		    cw_assert(histh_p_aux_get(&thead)
			      == histh_p_aux_get(&tfoot));
		    cw_assert(mkr_pos(&ttmp) <= mkr_pos(&tcur));
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }

	    /* Get previous record. */
	    histh_p_record_prev(&thead, &tfoot, &a_hist->h, &tbeg, &tcur, &tend,
				&ttmp);
	}
    }

    /* Iterate through the redo history and validate record consistency. */
    mkr_dup(&tbeg, &a_hist->hbeg);
    mkr_dup(&tcur, &a_hist->hcur);
    mkr_dup(&tend, &a_hist->hend);
    if (mkr_pos(&tbeg) < buf_len(&a_hist->h) + 1)
    {
	/* Initialize thead/tfoot. */
	if (mkr_pos(&tbeg) == mkr_pos(&tend))
	{
	    /* No current record.  Move on to next one. */
	    histh_p_record_next(&thead, &tfoot, &a_hist->h, &tbeg, &tcur, &tend,
				&ttmp);
	}
	else
	{
	    /* Get current record header/footer. */
	    histh_p_after_get(&thead, &tbeg, &ttmp);
	    histh_p_before_get(&tfoot, &tend, &ttmp);
	}

	/* Iterate through records. */
	while (mkr_pos(&tbeg) < buf_len(&a_hist->h) + 1)
	{
	    cw_assert(histh_p_tag_get(&thead) == histh_p_tag_get(&tfoot));

	    switch (histh_p_tag_get(&thead))
	    {
		case HISTH_TAG_GRP_BEG:
		case HISTH_TAG_GRP_END:
		case HISTH_TAG_SYNC:
		{
		    cw_assert(histh_p_aux_get(&thead) == 0);
		    cw_assert(histh_p_aux_get(&tfoot) == 0);
		    break;
		}
		case HISTH_TAG_POS:
		{
		    cw_assert(histh_p_aux_get(&thead) != 0);
		    cw_assert(histh_p_aux_get(&tfoot) != 0);
		    break;
		}
		case HISTH_TAG_INS:
		case HISTH_TAG_YNK:
		case HISTH_TAG_REM:
		case HISTH_TAG_DEL:
		{
		    cw_assert(histh_p_aux_get(&thead)
			      == histh_p_aux_get(&tfoot));
		    cw_assert(mkr_pos(&tcur) <= mkr_pos(&ttmp));
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }

	    /* Get next record. */
	    histh_p_record_next(&thead, &tfoot, &a_hist->h, &tbeg, &tcur, &tend,
				&ttmp);
	}
    }

    mkr_delete(&ttmp);
    mkr_delete(&tend);
    mkr_delete(&tcur);
    mkr_delete(&tbeg);
    histh_p_delete(&tfoot);
    histh_p_delete(&thead);
}
#endif
