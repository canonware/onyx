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
 * records have an overhead of 2, 4, or 18 bytes, depending on the record type,
 * and the amount of record data.  Log records have the following formats:
 *
 *     T : Record tag.
 *     A : Record auxiliary data (data length or bpos), big endian (network byte
 *         order).
 *   ... : Data, if any.  Data are stored in the logical order that the user
 *         would type them in.
 *
 * Sync (SYNC), group begin/end (GBEG, GEND):
 *   TT
 *
 * Position (POS):
 *   TAAAAAAAAAAAAAAAAT
 *
 * Short insert/yank/remove/delete (SINS, SYNK, SREM, SDEL):
 *   TA...AT
 *
 * Long insert/yank/remove/delete (LINS, LYNK, LREM, LDEL):
 *   TAAAAAAAA...AAAAAAAAT
 *
 * Three markers are used when traversing the history.  hbeg and hend bracket
 * the current record, and hcur is at the current position within the record.
 *
 *     TAAAAAAAA...AAAAAAAAT
 *    /\         /\        /\
 *    ||         ||        ||
 *   hbeg       hcur      hend
 *
 * The following notation is used in discussing the history buffer:
 *
 * Record : Meaning
 * =======:============================================
 * B      : Group begin.
 * E      : Group end.
 * P(n,m) : Buffer position change.
 * i(s)   : Insert short string s before point.
 * y(s)   : Insert (yank) short string s after point.
 * r(s)   : Remove short string s before point.
 * d(s)   : Remove (delete) short string s after point.
 * I(s)   : Insert long string s before point.
 * Y(s)   : Insert (yank) long string s after point.
 * R(s)   : Remove long string s before point.
 * D(s)   : Remove (delete) long string s after point.
 *
 ******************************************************************************/

#include "../include/modslate.h"
#include "../include/buf_l.h"
#ifdef CW_BUF_DUMP
#ifndef HAVE_ASPRINTF
#include "../../../lib/libonyx/src/asprintf.c"
#endif
#include <ctype.h>
#endif

/* Prototypes. */
/* histh. */
CW_P_INLINE void
histh_p_new(cw_histh_t *a_histh);
CW_P_INLINE void
histh_p_delete(cw_histh_t *a_histh);
CW_P_INLINE cw_bufv_t *
histh_p_bufv_get(cw_histh_t *a_histh);
CW_P_INLINE cw_uint32_t
histh_p_bufvcnt_get(const cw_histh_t *a_histh);
CW_P_INLINE cw_uint32_t
histh_p_bufvlen_get(const cw_histh_t *a_histh);
CW_P_INLINE void
histh_p_header_bufv_init(cw_histh_t *a_histh, cw_uint8_t a_tag);
CW_P_INLINE void
histh_p_footer_bufv_init(cw_histh_t *a_histh, cw_uint8_t a_tag);
CW_P_INLINE cw_uint8_t
histh_p_tag_get(const cw_histh_t *a_histh);
CW_P_INLINE void
histh_p_header_tag_set(cw_histh_t *a_histh, cw_uint8_t a_tag);
CW_P_INLINE void
histh_p_footer_tag_set(cw_histh_t *a_histh, cw_uint8_t a_tag);
CW_P_INLINE cw_uint8_t
histh_p_saux_get(const cw_histh_t *a_histh);
CW_P_INLINE void
histh_p_saux_set(cw_histh_t *a_histh, cw_uint8_t a_saux);
CW_P_INLINE cw_uint64_t
histh_p_aux_get(const cw_histh_t *a_histh);
CW_P_INLINE void
histh_p_aux_set(cw_histh_t *a_histh, cw_uint64_t a_aux);
CW_P_INLINE void
histh_p_header_before_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b,
			  cw_uint8_t a_tag);
CW_P_INLINE void
histh_p_footer_before_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b);
CW_P_INLINE void
histh_p_header_after_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b);
CW_P_INLINE void
histh_p_footer_after_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b,
			 cw_uint8_t a_tag);
CW_P_INLINE void
histh_p_record_prev(cw_histh_t *a_head, cw_histh_t *a_foot, cw_buf_t *a_buf,
		    cw_mkr_t *a_beg, cw_mkr_t *a_cur, cw_mkr_t *a_end,
		    cw_mkr_t *a_tmp);
CW_P_INLINE void
histh_p_record_next(cw_histh_t *a_head, cw_histh_t *a_foot, cw_buf_t *a_buf,
		    cw_mkr_t *a_beg, cw_mkr_t *a_cur, cw_mkr_t *a_end,
		    cw_mkr_t *a_tmp);
#ifdef CW_HIST_DUMP
static void
histh_p_dump(cw_histh_t *a_histh, const char *a_beg, const char *a_mid,
	     const char *a_end);
#endif

/* hist. */
#ifdef CW_BUF_DUMP
static void
hist_p_bufv_print(const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt);
#endif
static void
hist_p_record_prev(cw_hist_t *a_hist);
static void
hist_p_record_next(cw_hist_t *a_hist);
static void
hist_p_gap_prev(cw_hist_t *a_hist);
static void
hist_p_gap_next(cw_hist_t *a_hist);
CW_P_INLINE void
hist_p_redo_flush(cw_hist_t *a_hist);
CW_P_INLINE void
hist_p_pos(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos);
static void
hist_p_ins_ynk_rem_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
		       const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt,
		       cw_uint8_t a_tag);

/* histh. */
CW_P_INLINE void
histh_p_new(cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);

    a_histh->tag = HISTH_TAG_NONE;
    a_histh->u.aux = 0;

    a_histh->bufvcnt = 0;

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
histh_p_bufvcnt_get(const cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    return a_histh->bufvcnt;
}

CW_P_INLINE cw_uint32_t
histh_p_bufvlen_get(const cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    return a_histh->bufvlen;
}

CW_P_INLINE void
histh_p_header_bufv_init(cw_histh_t *a_histh, cw_uint8_t a_tag)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    switch (a_tag)
    {
	case HISTH_TAG_NONE:
	{
	    a_histh->bufvcnt = 0;
	    a_histh->bufvlen = 0;
	    break;
	}
	case HISTH_TAG_SYNC:
	case HISTH_TAG_GBEG:
	case HISTH_TAG_GEND:
	{
	    a_histh->bufv[0].data = &a_histh->tag;
	    a_histh->bufv[0].len = sizeof(a_histh->tag);
	    a_histh->bufvcnt = 1;
	    a_histh->bufvlen = sizeof(a_histh->tag);
	    break;
	}
	case HISTH_TAG_SINS:
	case HISTH_TAG_SYNK:
	case HISTH_TAG_SREM:
	case HISTH_TAG_SDEL:
	{
	    a_histh->bufv[0].data = &a_histh->tag;
	    a_histh->bufv[0].len = sizeof(a_histh->tag);
	    a_histh->bufv[1].data = a_histh->u.str;
	    a_histh->bufv[1].len = sizeof(a_histh->u.saux);
	    a_histh->bufvcnt = 2;
	    a_histh->bufvlen = sizeof(a_histh->tag) + sizeof(a_histh->u.saux);
	    break;
	}
	case HISTH_TAG_POS:
	case HISTH_TAG_LINS:
	case HISTH_TAG_LYNK:
	case HISTH_TAG_LREM:
	case HISTH_TAG_LDEL:
	{
	    a_histh->bufv[0].data = &a_histh->tag;
	    a_histh->bufv[0].len = sizeof(a_histh->tag);
	    a_histh->bufv[1].data = a_histh->u.str;
	    a_histh->bufv[1].len = sizeof(a_histh->u.aux);
	    a_histh->bufvcnt = 2;
	    a_histh->bufvlen = sizeof(a_histh->tag) + sizeof(a_histh->u.aux);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
}

CW_P_INLINE void
histh_p_footer_bufv_init(cw_histh_t *a_histh, cw_uint8_t a_tag)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    switch (a_tag)
    {
	case HISTH_TAG_NONE:
	{
	    a_histh->bufvcnt = 0;
	    a_histh->bufvlen = 0;
	    break;
	}
	case HISTH_TAG_SYNC:
	case HISTH_TAG_GBEG:
	case HISTH_TAG_GEND:
	{
	    a_histh->bufv[0].data = &a_histh->tag;
	    a_histh->bufv[0].len = sizeof(a_histh->tag);
	    a_histh->bufvcnt = 1;
	    a_histh->bufvlen = sizeof(a_histh->tag);
	    break;
	}
	case HISTH_TAG_SINS:
	case HISTH_TAG_SYNK:
	case HISTH_TAG_SREM:
	case HISTH_TAG_SDEL:
	{
	    a_histh->bufv[0].data = a_histh->u.str;
	    a_histh->bufv[0].len = sizeof(a_histh->u.saux);
	    a_histh->bufv[1].data = &a_histh->tag;
	    a_histh->bufv[1].len = sizeof(a_histh->tag);
	    a_histh->bufvcnt = 2;
	    a_histh->bufvlen = sizeof(a_histh->u.saux) + sizeof(a_histh->tag);
	    break;
	}
	case HISTH_TAG_POS:
	case HISTH_TAG_LINS:
	case HISTH_TAG_LYNK:
	case HISTH_TAG_LREM:
	case HISTH_TAG_LDEL:
	{
	    a_histh->bufv[0].data = a_histh->u.str;
	    a_histh->bufv[0].len = sizeof(a_histh->u.aux);
	    a_histh->bufv[1].data = &a_histh->tag;
	    a_histh->bufv[1].len = sizeof(a_histh->tag);
	    a_histh->bufvcnt = 2;
	    a_histh->bufvlen = sizeof(a_histh->u.aux) + sizeof(a_histh->tag);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
}

CW_P_INLINE cw_uint8_t
histh_p_tag_get(const cw_histh_t *a_histh)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    return a_histh->tag;
}

CW_P_INLINE void
histh_p_header_tag_set(cw_histh_t *a_histh, cw_uint8_t a_tag)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    a_histh->tag = a_tag;
    histh_p_header_bufv_init(a_histh, a_tag);
}

CW_P_INLINE void
histh_p_footer_tag_set(cw_histh_t *a_histh, cw_uint8_t a_tag)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);

    a_histh->tag = a_tag;
    histh_p_footer_bufv_init(a_histh, a_tag);
}

CW_P_INLINE cw_uint8_t
histh_p_saux_get(const cw_histh_t *a_histh)
{
    cw_uint8_t retval;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(a_histh->tag == HISTH_TAG_SINS
	      || a_histh->tag == HISTH_TAG_SYNK
	      || a_histh->tag == HISTH_TAG_SREM
	      || a_histh->tag == HISTH_TAG_SDEL);

    retval = a_histh->u.saux;

    return retval;
}

CW_P_INLINE void
histh_p_saux_set(cw_histh_t *a_histh, cw_uint8_t a_saux)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(a_histh->tag == HISTH_TAG_SINS
	      || a_histh->tag == HISTH_TAG_SYNK
	      || a_histh->tag == HISTH_TAG_SREM
	      || a_histh->tag == HISTH_TAG_SDEL);

    a_histh->u.saux = a_saux;
}

CW_P_INLINE cw_uint64_t
histh_p_aux_get(const cw_histh_t *a_histh)
{
    cw_uint64_t retval;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(a_histh->tag == HISTH_TAG_POS
	      || a_histh->tag == HISTH_TAG_LINS
	      || a_histh->tag == HISTH_TAG_LYNK
	      || a_histh->tag == HISTH_TAG_LREM
	      || a_histh->tag == HISTH_TAG_LDEL);

    retval = cw_ntohq(a_histh->u.aux);

    return retval;
}

CW_P_INLINE void
histh_p_aux_set(cw_histh_t *a_histh, cw_uint64_t a_aux)
{
    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(a_histh->tag == HISTH_TAG_POS
	      || a_histh->tag == HISTH_TAG_LINS
	      || a_histh->tag == HISTH_TAG_LYNK
	      || a_histh->tag == HISTH_TAG_LREM
	      || a_histh->tag == HISTH_TAG_LDEL);

    a_histh->u.aux = cw_htonq(a_aux);
}

/* Get the header that precedes a_a.  Various code relies on the fact that the
 * header is bracketed by a_b..a_a after this call. */
CW_P_INLINE void
histh_p_header_before_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b,
			  cw_uint8_t a_tag)
{
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(mkr_pos(a_a) > 1);
    cw_assert(mkr_pos(a_a) < buf_len(mkr_buf(a_a)) + 1);

    /* Prepare to copy the header into a_histh's bufv. */
    histh_p_header_bufv_init(a_histh, a_tag);

    /* Get a bufv containing the header. */
    mkr_dup(a_b, a_a);
    mkr_seek(a_b, -(cw_sint64_t)a_histh->bufvlen, BUFW_REL);
    bufv = mkr_range_get(a_b, a_a, &bufvcnt);
    cw_check_ptr(bufv);

    /* Copy the header to a_histh. */
    bufv_copy(a_histh->bufv, a_histh->bufvcnt, bufv, bufvcnt, 0);
}

/* Get the footer that precedes a_a.  Various code relies on the fact that the
 * footer is bracketed by a_b..a_a after this call. */
CW_P_INLINE void
histh_p_footer_before_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b)
{
    cw_uint8_t *tag;
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(mkr_pos(a_a) > 1);

    /* Get record type and prepare to copy the footer into a_histh's bufv. */
    tag = mkr_before_get(a_a);
    histh_p_footer_bufv_init(a_histh, *tag);

    /* Get a bufv containing the footer. */
    mkr_dup(a_b, a_a);
    mkr_seek(a_b, -(cw_sint64_t)a_histh->bufvlen, BUFW_REL);
    bufv = mkr_range_get(a_b, a_a, &bufvcnt);
    cw_check_ptr(bufv);

    /* Copy the footer to a_histh. */
    bufv_copy(a_histh->bufv, a_histh->bufvcnt, bufv, bufvcnt, 0);
}

/* Get the header that follows a_a.  Various code relies on the fact that the
 * header is bracketed by a_a..a_b after this call. */
CW_P_INLINE void
histh_p_header_after_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b)
{
    cw_uint8_t *tag;
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(mkr_pos(a_a) < buf_len(mkr_buf(a_a)) + 1);

    /* Get record type and prepare to copy the header into a_histh's bufv. */
    tag = mkr_after_get(a_a);
    histh_p_header_bufv_init(a_histh, *tag);

    /* Get a bufv containing the header. */
    mkr_dup(a_b, a_a);
    mkr_seek(a_b, a_histh->bufvlen, BUFW_REL);
    cw_assert(mkr_pos(a_a) + a_histh->bufvlen == mkr_pos(a_b));
    bufv = mkr_range_get(a_a, a_b, &bufvcnt);
    cw_check_ptr(bufv);

    /* Copy the header to a_histh. */
    bufv_copy(a_histh->bufv, a_histh->bufvcnt, bufv, bufvcnt, 0);
}

/* Get the footer that follows a_a.  Various code relies on the fact that the
 * footer is bracketed by a_a..a_b after this call. */
CW_P_INLINE void
histh_p_footer_after_get(cw_histh_t *a_histh, cw_mkr_t *a_a, cw_mkr_t *a_b,
			 cw_uint8_t a_tag)
{
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    cw_check_ptr(a_histh);
    cw_dassert(a_histh->magic == CW_HISTH_MAGIC);
    cw_assert(mkr_pos(a_a) > 1);
    cw_assert(mkr_pos(a_a) < buf_len(mkr_buf(a_a)) + 1);

    /* Prepare to copy the footer into a_histh's bufv. */
    histh_p_footer_bufv_init(a_histh, a_tag);

    /* Get a bufv containing the footer. */
    mkr_dup(a_b, a_a);
    mkr_seek(a_b, a_histh->bufvlen, BUFW_REL);
    cw_assert(mkr_pos(a_a) + a_histh->bufvlen == mkr_pos(a_b));
    bufv = mkr_range_get(a_a, a_b, &bufvcnt);
    cw_check_ptr(bufv);

    /* Copy the footer to a_histh. */
    bufv_copy(a_histh->bufv, a_histh->bufvcnt, bufv, bufvcnt, 0);
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
	    histh_p_header_tag_set(a_head, HISTH_TAG_NONE);
	    histh_p_footer_tag_set(a_foot, HISTH_TAG_NONE);
	    break;
	}

	/* Get footer. */
	histh_p_footer_before_get(a_foot, a_end, a_cur);

	switch (histh_p_tag_get(a_foot))
	{
	    case HISTH_TAG_SYNC:
	    {
		/* Ignore sync records. */
		again = TRUE;
		/* Fall through. */
	    }
	    case HISTH_TAG_GBEG:
	    case HISTH_TAG_GEND:
	    case HISTH_TAG_POS:
	    {
		/* Get header. */
		histh_p_header_before_get(a_head, a_cur, a_beg,
					  histh_p_tag_get(a_foot));

		break;
	    }
	    case HISTH_TAG_SINS:
	    case HISTH_TAG_SYNK:
	    case HISTH_TAG_SREM:
	    case HISTH_TAG_SDEL:
	    {
		/* Skip data. */
		mkr_dup(a_tmp, a_cur);
		mkr_seek(a_tmp, -(cw_sint64_t)histh_p_saux_get(a_foot),
			 BUFW_REL);

		/* Get header. */
		histh_p_header_before_get(a_head, a_tmp, a_beg,
					  histh_p_tag_get(a_foot));

		break;
	    }
	    case HISTH_TAG_LINS:
	    case HISTH_TAG_LYNK:
	    case HISTH_TAG_LREM:
	    case HISTH_TAG_LDEL:
	    {
		/* Skip data. */
		mkr_dup(a_tmp, a_cur);
		mkr_seek(a_tmp, -(cw_sint64_t)histh_p_aux_get(a_foot),
			 BUFW_REL);

		/* Get header. */
		histh_p_header_before_get(a_head, a_tmp, a_beg,
					  histh_p_tag_get(a_foot));

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
	    histh_p_header_tag_set(a_head, HISTH_TAG_NONE);
	    histh_p_footer_tag_set(a_foot, HISTH_TAG_NONE);
	    break;
	}

	/* Get header. */
	histh_p_header_after_get(a_head, a_beg, a_cur);

	switch (histh_p_tag_get(a_head))
	{
	    case HISTH_TAG_SYNC:
	    {
		/* Ignore sync records. */
		again = TRUE;
		/* Fall through. */
	    }
	    case HISTH_TAG_GBEG:
	    case HISTH_TAG_GEND:
	    case HISTH_TAG_POS:
	    {
		/* Get footer. */
		histh_p_footer_after_get(a_foot, a_cur, a_end,
					 histh_p_tag_get(a_head));

		break;
	    }
	    case HISTH_TAG_SINS:
	    case HISTH_TAG_SYNK:
	    case HISTH_TAG_SREM:
	    case HISTH_TAG_SDEL:
	    {
		/* Skip data. */
		mkr_dup(a_tmp, a_cur);
		mkr_seek(a_tmp, histh_p_saux_get(a_head), BUFW_REL);

		/* Get footer. */
		histh_p_footer_after_get(a_foot, a_tmp, a_end,
					 histh_p_tag_get(a_head));

		break;
	    }
	    case HISTH_TAG_LINS:
	    case HISTH_TAG_LYNK:
	    case HISTH_TAG_LREM:
	    case HISTH_TAG_LDEL:
	    {
		/* Skip data. */
		mkr_dup(a_tmp, a_cur);
		mkr_seek(a_tmp, histh_p_aux_get(a_head), BUFW_REL);

		/* Get footer. */
		histh_p_footer_after_get(a_foot, a_tmp, a_end,
					 histh_p_tag_get(a_head));

		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
}

#ifdef CW_HIST_DUMP
static void
histh_p_dump(cw_histh_t *a_histh, const char *a_beg, const char *a_mid,
	     const char *a_end)
{
    const char *beg, *mid, *end;
    static const char *tags[] =
    {
	"HISTH_TAG_NONE",
	"HISTH_TAG_SYNC",
	"HISTH_TAG_GBEG",
	"HISTH_TAG_GEND",
	"HISTH_TAG_POS",
	"HISTH_TAG_SINS",
	"HISTH_TAG_SYNK",
	"HISTH_TAG_SREM",
	"HISTH_TAG_SDEL",
	"HISTH_TAG_LINS",
	"HISTH_TAG_LYNK",
	"HISTH_TAG_LREM",
	"HISTH_TAG_LDEL"
    };

    beg = (a_beg != NULL) ? a_beg : "";
    mid = (a_mid != NULL) ? a_mid : beg;
    end = (a_end != NULL) ? a_end : mid;

    fprintf(stderr, "%shisth: %p\n", beg, a_histh);
    fprintf(stderr, "%s|\n", mid);
    switch (a_histh->tag)
    {
	case HISTH_TAG_NONE:
	case HISTH_TAG_SYNC:
	case HISTH_TAG_GBEG:
	case HISTH_TAG_GEND:
	{
	    fprintf(stderr, "%s\\-> tag: %s (%u)\n", mid,
		    tags[a_histh->tag], a_histh->tag);
	    break;
	}
	case HISTH_TAG_SINS:
	case HISTH_TAG_SYNK:
	case HISTH_TAG_SREM:
	case HISTH_TAG_SDEL:
	{
	    fprintf(stderr, "%s|-> tag: %s (%u)\n", mid,
		    tags[a_histh->tag], a_histh->tag);
	    fprintf(stderr, "%s|\n", mid);
	    fprintf(stderr, "%s\\-> saux: %u\n", mid,
		    histh_p_saux_get(a_histh));
	    break;
	}
	case HISTH_TAG_POS:
	case HISTH_TAG_LINS:
	case HISTH_TAG_LYNK:
	case HISTH_TAG_LREM:
	case HISTH_TAG_LDEL:
	{
	    fprintf(stderr, "%s|-> tag: %s (%u)\n", mid,
		    tags[a_histh->tag], a_histh->tag);
	    fprintf(stderr, "%s|\n", mid);
	    fprintf(stderr, "%s\\-> aux: %llu\n", mid,
		    histh_p_aux_get(a_histh));
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
}
#endif

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

/* Move to the gap previous to the current record in h. */
static void
hist_p_gap_prev(cw_hist_t *a_hist)
{
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_NONE);
    histh_p_footer_tag_set(&a_hist->hfoot, HISTH_TAG_NONE);

    mkr_dup(&a_hist->hcur, &a_hist->hbeg);
    mkr_dup(&a_hist->hend, &a_hist->hbeg);
}

/* Move to the gap following the current record in h. */
static void
hist_p_gap_next(cw_hist_t *a_hist)
{
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_NONE);
    histh_p_footer_tag_set(&a_hist->hfoot, HISTH_TAG_NONE);

    mkr_dup(&a_hist->hbeg, &a_hist->hend);
    mkr_dup(&a_hist->hcur, &a_hist->hend);
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
    if (mkr_pos(&a_hist->hcur) + histh_p_bufvlen_get(&a_hist->hfoot)
	< mkr_pos(&a_hist->hend))
    {
	mkr_dup(&a_hist->htmp, &a_hist->hend);
	mkr_seek(&a_hist->htmp,
		 -(cw_sint64_t)histh_p_bufvlen_get(&a_hist->hfoot), BUFW_REL);
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

    /* Initialize and insert header (old position). */
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_POS);
    histh_p_aux_set(&a_hist->hhead, a_hist->hbpos);
    mkr_before_insert(&a_hist->hend, histh_p_bufv_get(&a_hist->hhead),
		      histh_p_bufvcnt_get(&a_hist->hhead));

    /* Initialize and insert footer (new position). */
    histh_p_footer_tag_set(&a_hist->hfoot, HISTH_TAG_POS);
    histh_p_aux_set(&a_hist->hfoot, a_bpos);
    mkr_before_insert(&a_hist->hend, histh_p_bufv_get(&a_hist->hfoot),
		      histh_p_bufvcnt_get(&a_hist->hfoot));

    /* Relocate hbeg and hcur. */
    mkr_dup(&a_hist->hbeg, &a_hist->hend);
    mkr_dup(&a_hist->hcur, &a_hist->hend);

    /* Reset header and footer. */
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_NONE);
    histh_p_footer_tag_set(&a_hist->hhead, HISTH_TAG_NONE);

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
    cw_uint8_t tag_short;

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

    /* Calculate insertion data count. */
    cnt = 0;
    for (i = 0; i < a_bufvcnt; i++)
    {
	cnt += (cw_uint64_t) a_bufv[i].len;
    }

    /* Update the recorded position.  Also determine tag equivalence so that
     * short and long records can be dealt with similarly. */
    switch (a_tag)
    {
	case HISTH_TAG_LINS:
	{
	    a_hist->hbpos += cnt;
	    tag_short = HISTH_TAG_SINS;
	    break;
	}
	case HISTH_TAG_LREM:
	{
	    a_hist->hbpos -= cnt;
	    tag_short = HISTH_TAG_SREM;
	    break;
	}
	case HISTH_TAG_LYNK:
	{
	    tag_short = HISTH_TAG_SYNK;
	    break;
	}
	case HISTH_TAG_LDEL:
	{
	    tag_short = HISTH_TAG_SDEL;
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    /* Update the data count if extending an existing record.  Otherwise, create
     * a new empty record. */
    if (mkr_pos(&a_hist->hend) > 1
	&& (histh_p_tag_get(&a_hist->hhead) == tag_short
	    || histh_p_tag_get(&a_hist->hhead) == a_tag))
    {
	cw_uint64_t ocnt;

	/* Remove the header, footer, and redo data, in preparation for
	 * reconstruction. */
	mkr_dup(&a_hist->htmp, &a_hist->hbeg);
	mkr_seek(&a_hist->htmp, histh_p_bufvlen_get(&a_hist->hhead), BUFW_REL);
	mkr_remove(&a_hist->hbeg, &a_hist->htmp);
	mkr_remove(&a_hist->hcur, &a_hist->hend);

	/* Get the old data count.  This may be different from what the
	 * header/footer say, since any redo data have been removed. */
	ocnt = mkr_pos(&a_hist->hcur) - mkr_pos(&a_hist->hbeg);

	/* Create and insert the new header and footer.  Be careful to leave
	 * hcur in the correct location (just before the footer). */
	if (ocnt + cnt <= HISTH_SAUX_MAX)
	{
	    histh_p_header_tag_set(&a_hist->hhead, tag_short);
	    histh_p_saux_set(&a_hist->hhead, ocnt + cnt);

	    histh_p_footer_tag_set(&a_hist->hfoot, tag_short);
	    histh_p_saux_set(&a_hist->hfoot, ocnt + cnt);
	}
	else
	{
	    histh_p_header_tag_set(&a_hist->hhead, a_tag);
	    histh_p_aux_set(&a_hist->hhead, ocnt + cnt);

	    histh_p_footer_tag_set(&a_hist->hfoot, a_tag);
	    histh_p_aux_set(&a_hist->hfoot, ocnt + cnt);
	}
	mkr_after_insert(&a_hist->hbeg, histh_p_bufv_get(&a_hist->hhead),
			 histh_p_bufvcnt_get(&a_hist->hhead));
	mkr_before_insert(&a_hist->hend, histh_p_bufv_get(&a_hist->hfoot),
			  histh_p_bufvcnt_get(&a_hist->hfoot));
	mkr_seek(&a_hist->hcur,
		 -(cw_sint64_t)histh_p_bufvlen_get(&a_hist->hfoot), BUFW_REL);
    }
    else
    {
	/* Initialize header. */
	if (cnt <= HISTH_SAUX_MAX)
	{
	    histh_p_header_tag_set(&a_hist->hhead, tag_short);
	    histh_p_saux_set(&a_hist->hhead, cnt);
	}
	else
	{
	    histh_p_header_tag_set(&a_hist->hhead, a_tag);
	    histh_p_aux_set(&a_hist->hhead, cnt);
	}

	/* Relocate hbeg. */
	mkr_dup(&a_hist->hbeg, &a_hist->hend);

	/* Insert header. */
	mkr_after_insert(&a_hist->hbeg, histh_p_bufv_get(&a_hist->hhead),
			 histh_p_bufvcnt_get(&a_hist->hhead));

	/* Relocate hcur. */
	mkr_dup(&a_hist->hcur, &a_hist->hbeg);
	mkr_seek(&a_hist->hcur, histh_p_bufvlen_get(&a_hist->hhead), BUFW_REL);

	/* Initialize footer. */
	if (cnt <= HISTH_SAUX_MAX)
	{
	    histh_p_footer_tag_set(&a_hist->hfoot, tag_short);
	    histh_p_saux_set(&a_hist->hfoot, cnt);
	}
	else
	{
	    histh_p_footer_tag_set(&a_hist->hfoot, a_tag);
	    histh_p_aux_set(&a_hist->hfoot, cnt);
	}

	/* Insert footer. */
	mkr_after_insert(&a_hist->hcur, histh_p_bufv_get(&a_hist->hfoot),
			 histh_p_bufvcnt_get(&a_hist->hfoot));

	/* Relocate hend. */
	mkr_dup(&a_hist->hend, &a_hist->hcur);
	mkr_seek(&a_hist->hend, histh_p_bufvlen_get(&a_hist->hfoot), BUFW_REL);
    }

    /* Insert the data in the appropriate order for the record type. */
    switch (a_tag)
    {
	case HISTH_TAG_LINS:
	case HISTH_TAG_LDEL:
	{
	    /* Insert in forward order. */
	    mkr_before_insert(&a_hist->hcur, a_bufv, a_bufvcnt);
	    break;
	}
	case HISTH_TAG_LYNK:
	case HISTH_TAG_LREM:
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
hist_new(cw_mema_t *a_mema)
{
    cw_hist_t *retval;

    retval = (cw_hist_t *) cw_opaque_alloc(mema_alloc_get(a_mema),
					   mema_arg_get(a_mema),
					   sizeof(cw_hist_t));
    buf_new(&retval->h, CW_HIST_BUFP_SIZE, a_mema);
    histh_p_new(&retval->hhead);
    histh_p_new(&retval->hfoot);
    mkr_new(&retval->hbeg, &retval->h);
    mkr_new(&retval->hcur, &retval->h);
    mkr_new(&retval->hend, &retval->h);
    mkr_new(&retval->htmp, &retval->h);
    retval->hbpos = 0;
    retval->gdepth = 0;
    retval->mema = a_mema;
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
    cw_opaque_dealloc(mema_dealloc_get(a_hist->mema),
		      mema_arg_get(a_hist->mema), a_hist, sizeof(cw_hist_t));
}

cw_bool_t
hist_undoable(const cw_hist_t *a_hist, const cw_buf_t *a_buf)
{
    cw_bool_t retval;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);

    /* There is at least one undoable operation unless hcur is at or before the
     * beginning of the data in the first record. */
    if (mkr_pos(&a_hist->hcur) > 1 + histh_p_bufvlen_get(&a_hist->hhead))
    {
	retval = TRUE;
    }
    else
    {
	retval = FALSE;
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

    /* There is at least one redoable operation unless hcur is at or past the
     * end of the data in the last record. */
    if (mkr_pos(&a_hist->hcur) + histh_p_bufvlen_get(&a_hist->hfoot)
	< buf_len(&a_hist->h) + 1)
    {
	retval = TRUE;
    }
    else
    {
	retval = FALSE;
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

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    mkr_new(&tmkr, a_buf);

    /* Iteratively undo a_count times. */
    for (retval = 0;
	 retval < a_count && mkr_pos(&a_hist->hcur) > 1;
	 )
    {
	switch (histh_p_tag_get(&a_hist->hhead))
	{
	    case HISTH_TAG_NONE:
	    {
		/* Move to the previous record. */
		hist_p_record_prev(a_hist);
		if (histh_p_tag_get(&a_hist->hhead) == HISTH_TAG_NONE)
		{
		    /* No more records. */
		    goto RETURN;
		}
		break;
	    }
	    case HISTH_TAG_SYNC:
	    {
		/* Remove sync records. */
		mkr_remove(&a_hist->hbeg, &a_hist->hend);

		/* Move before the current record. */
		hist_p_gap_prev(a_hist);
		break;
	    }
	    case HISTH_TAG_GBEG:
	    {
		/* Decrease depth. */
		cw_assert(a_hist->gdepth != 0);
		a_hist->gdepth--;

		/* Increment the undo count, if the group depth dropped to 0. */
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}

		/* Move before the current record. */
		hist_p_gap_prev(a_hist);
		break;
	    }
	    case HISTH_TAG_GEND:
	    {
		/* Increase depth. */
		a_hist->gdepth++;

		/* Move before the current record. */
		hist_p_gap_prev(a_hist);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		/* Update hbpos. */
		a_hist->hbpos = histh_p_aux_get(&a_hist->hhead);

		/* Move. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);

		/* Move before the current record. */
		hist_p_gap_prev(a_hist);
		break;
	    }
	    case HISTH_TAG_SINS:
	    case HISTH_TAG_SYNK:
	    case HISTH_TAG_SREM:
	    case HISTH_TAG_SDEL:
	    case HISTH_TAG_LINS:
	    case HISTH_TAG_LYNK:
	    case HISTH_TAG_LREM:
	    case HISTH_TAG_LDEL:
	    {
		cw_assert(mkr_pos(&a_hist->hcur)
			  > mkr_pos(&a_hist->hbeg)
			  + histh_p_bufvlen_get(&a_hist->hhead));

		/* Move to the correct bpos in the buf. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);

		/* Take action. */
		if (a_hist->gdepth == 0)
		{
		    cw_bufv_t bufv;

		    /* Undo one character at a time, in order to simplify doing
		     * a_count undo operations. */

		    switch (histh_p_tag_get(&a_hist->hhead))
		    {
			case HISTH_TAG_SINS:
			case HISTH_TAG_LINS:
			{
			    /* Remove character. */
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, -1LL, BUFW_REL);
			    mkr_l_remove(&tmkr, a_mkr, FALSE);

			    /* Adjust bpos. */
			    a_hist->hbpos--;
			    break;
			}
			case HISTH_TAG_SYNK:
			case HISTH_TAG_LYNK:
			{
			    /* Remove character. */
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, 1, BUFW_REL);
			    mkr_l_remove(a_mkr, &tmkr, FALSE);
			    break;
			}
			case HISTH_TAG_SREM:
			case HISTH_TAG_LREM:
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
			case HISTH_TAG_SDEL:
			case HISTH_TAG_LDEL:
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

		    /* Increment the undo count. */
		    retval++;

		    /* Move to the previous record if all data in this record
		     * have already been undone. */
		    if (mkr_pos(&a_hist->hcur)
			== mkr_pos(&a_hist->hbeg)
			+ histh_p_bufvlen_get(&a_hist->hhead))
		    {
			hist_p_gap_prev(a_hist);
		    }
		}
		else
		{
		    cw_uint64_t ucnt;

		    /* Undo the entire record, since the non-zero group depth
		     * means there is no need to count the number of characters
		     * undone. */

		    /* Get the number of characters left in the record to
		     * undo.  htmp is moved as a side effect, which is relied on
		     * in some cases below. */
		    mkr_dup(&a_hist->htmp, &a_hist->hbeg);
		    mkr_seek(&a_hist->htmp, histh_p_bufvlen_get(&a_hist->hhead),
			     BUFW_REL);
		    ucnt = mkr_pos(&a_hist->hcur) - mkr_pos(&a_hist->htmp);

		    switch (histh_p_tag_get(&a_hist->hhead))
		    {
			case HISTH_TAG_SINS:
			case HISTH_TAG_LINS:
			{
			    /* Remove characters. */
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, -(cw_sint64_t)ucnt, BUFW_REL);
			    mkr_l_remove(&tmkr, a_mkr, FALSE);

			    /* Adjust bpos. */
			    a_hist->hbpos -= ucnt;
			    break;
			}
			case HISTH_TAG_SYNK:
			case HISTH_TAG_LYNK:
			{
			    /* Remove characters. */
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, ucnt, BUFW_REL);
			    mkr_l_remove(a_mkr, &tmkr, FALSE);
			    break;
			}
			case HISTH_TAG_SREM:
			case HISTH_TAG_LREM:
			{
			    cw_bufv_t *bufv;
			    cw_uint32_t bufvcnt;

			    /* Get bufv. */
			    bufv = mkr_range_get(&a_hist->htmp, &a_hist->hcur,
						 &bufvcnt);

			    /* Insert bufv. */
			    mkr_l_insert(a_mkr, FALSE, FALSE, bufv, bufvcnt,
					 FALSE);

			    /* Adjust bpos. */
			    a_hist->hbpos += ucnt;
			    break;
			}
			case HISTH_TAG_SDEL:
			case HISTH_TAG_LDEL:
			{
			    cw_bufv_t *bufv;
			    cw_uint32_t bufvcnt;

			    /* Get bufv. */
			    bufv = mkr_range_get(&a_hist->htmp, &a_hist->hcur,
						 &bufvcnt);

			    /* Insert bufv. */
			    mkr_l_insert(a_mkr, FALSE, TRUE, bufv, bufvcnt,
					 FALSE);
			    break;
			}
			default:
			{
			    cw_not_reached();
			}
		    }

		    /* Move to the previous record. */
		    hist_p_gap_prev(a_hist);
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
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
    cw_uint8_t *p;
    cw_mkr_t tmkr;

    cw_check_ptr(a_hist);
    cw_dassert(a_hist->magic == CW_HIST_MAGIC);
    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);

    mkr_new(&tmkr, a_buf);

    /* Iteratively redo a_count times. */
    for (retval = 0, redid = FALSE;
	 retval < a_count
	     && mkr_pos(&a_hist->hcur) + histh_p_bufvlen_get(&a_hist->hfoot)
	     < buf_len(&a_hist->h) + 1;
	 )
    {
	/* Make a note that the redo loop was executed at least once, so that a
	 * correct decision about whether an incomplete group was redone can be
	 * made after the loop terminates. */
	redid = TRUE;

	switch (histh_p_tag_get(&a_hist->hhead))
	{
	    case HISTH_TAG_NONE:
	    {
		/* Move to the next record. */
		hist_p_record_next(a_hist);
		if (histh_p_tag_get(&a_hist->hhead) == HISTH_TAG_NONE)
		{
		    /* No more records. */
		    goto RETURN;
		}
		break;
	    }
	    case HISTH_TAG_SYNC:
	    {
		/* Remove sync records. */
		mkr_remove(&a_hist->hbeg, &a_hist->hend);

		/* Move after the current record. */
		hist_p_gap_next(a_hist);
		break;
	    }
	    case HISTH_TAG_GBEG:
	    {
		/* Increase depth. */
		a_hist->gdepth++;

		/* Move after the current record. */
		hist_p_gap_next(a_hist);
		break;
	    }
	    case HISTH_TAG_GEND:
	    {
		/* Decrease depth. */
		cw_assert(a_hist->gdepth != 0);
		a_hist->gdepth--;

		/* Increment the redo count, if the group depth dropped to 0. */
		if (a_hist->gdepth == 0)
		{
		    retval++;
		}

		/* Move after the current record. */
		hist_p_gap_next(a_hist);
		break;
	    }
	    case HISTH_TAG_POS:
	    {
		/* Update hbpos. */
		a_hist->hbpos = histh_p_aux_get(&a_hist->hfoot);

		/* Move. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);

		/* Move after the current record. */
		hist_p_gap_next(a_hist);
		break;
	    }
	    case HISTH_TAG_SINS:
	    case HISTH_TAG_SYNK:
	    case HISTH_TAG_SREM:
	    case HISTH_TAG_SDEL:
	    case HISTH_TAG_LINS:
	    case HISTH_TAG_LYNK:
	    case HISTH_TAG_LREM:
	    case HISTH_TAG_LDEL:
	    {
		cw_assert(mkr_pos(&a_hist->hcur)
			  < mkr_pos(&a_hist->hend)
			  - histh_p_bufvlen_get(&a_hist->hfoot));

		/* Move to the correct bpos in the buf. */
		mkr_seek(a_mkr, a_hist->hbpos - 1, BUFW_BOB);

		/* Take action. */
		if (a_hist->gdepth == 0)
		{
		    cw_bufv_t bufv;

		    /* Redo one character at a time, in order to simplify doing
		     * a_count redo operations. */

		    switch(histh_p_tag_get(&a_hist->hhead))
		    {
			case HISTH_TAG_SINS:
			case HISTH_TAG_LINS:
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
			case HISTH_TAG_SYNK:
			case HISTH_TAG_LYNK:
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
			case HISTH_TAG_SREM:
			case HISTH_TAG_LREM:
			{
			    /* Remove character. */
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, -1LL, BUFW_REL);
			    mkr_l_remove(&tmkr, a_mkr, FALSE);

			    /* Adjust bpos. */
			    a_hist->hbpos--;
			    break;
			}
			case HISTH_TAG_SDEL:
			case HISTH_TAG_LDEL:
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

		    /* Increment the redo count. */
		    retval++;

		    /* Move to the next record if all data in this record have
		     * already been redone. */
		    if (mkr_pos(&a_hist->hcur)
			== mkr_pos(&a_hist->hend)
			- histh_p_bufvlen_get(&a_hist->hfoot))
		    {
			hist_p_gap_next(a_hist);
		    }
		}
		else
		{
		    cw_uint64_t rcnt;

		    /* Redo the entire record, since the non-zero group depth
		     * means there is no need to count the number of characters
		     * redone. */
		    
		    /* Get the number of characters left in the record to
		     * undo.  htmp is moved as a side effect, which is relied on
		     * in some cases below. */
		    mkr_dup(&a_hist->htmp, &a_hist->hend);
		    mkr_seek(&a_hist->htmp,
			     -(cw_sint64_t)histh_p_bufvlen_get(&a_hist->hhead),
			     BUFW_REL);
		    rcnt = mkr_pos(&a_hist->htmp) - mkr_pos(&a_hist->hcur);

		    switch(histh_p_tag_get(&a_hist->hhead))
		    {
			case HISTH_TAG_SINS:
			case HISTH_TAG_LINS:
			{
			    cw_bufv_t *bufv;
			    cw_uint32_t bufvcnt;

			    /* Get bufv. */
			    bufv = mkr_range_get(&a_hist->hcur, &a_hist->htmp,
						 &bufvcnt);

			    /* Insert bufv. */
			    mkr_l_insert(a_mkr, FALSE, FALSE, bufv, bufvcnt,
					 FALSE);

			    /* Adjust bpos. */
			    a_hist->hbpos += rcnt;
			    break;
			}
			case HISTH_TAG_SYNK:
			case HISTH_TAG_LYNK:
			{
			    cw_bufv_t *bufv;
			    cw_uint32_t bufvcnt;

			    /* Get bufv. */
			    bufv = mkr_range_get(&a_hist->htmp, &a_hist->hcur,
						 &bufvcnt);

			    /* Insert bufv. */
			    mkr_l_insert(a_mkr, FALSE, TRUE, bufv, bufvcnt,
					 FALSE);
			    break;
			}
			case HISTH_TAG_SREM:
			case HISTH_TAG_LREM:
			{
			    /* Remove characters. */
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, -(cw_sint64_t)rcnt, BUFW_REL);
			    mkr_l_remove(&tmkr, a_mkr, FALSE);

			    /* Adjust bpos. */
			    a_hist->hbpos -= rcnt;
			    break;
			}
			case HISTH_TAG_SDEL:
			case HISTH_TAG_LDEL:
			{
			    /* Remove characters. */
			    mkr_dup(&tmkr, a_mkr);
			    mkr_seek(&tmkr, rcnt, BUFW_REL);
			    mkr_l_remove(a_mkr, &tmkr, FALSE);
			    break;
			}
			default:
			{
			    cw_not_reached();
			}
		    }

		    /* Move to the next record. */
		    hist_p_gap_next(a_hist);
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }

    RETURN:
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
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_NONE);
    histh_p_footer_tag_set(&a_hist->hfoot, HISTH_TAG_NONE);
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

    /* Initialize and insert header. */
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_GBEG);
    mkr_before_insert(&a_hist->hend, histh_p_bufv_get(&a_hist->hhead),
		      histh_p_bufvcnt_get(&a_hist->hhead));

    /* Initialize and insert footer. */
    histh_p_footer_tag_set(&a_hist->hfoot, HISTH_TAG_GBEG);
    mkr_before_insert(&a_hist->hend, histh_p_bufv_get(&a_hist->hfoot),
		      histh_p_bufvcnt_get(&a_hist->hfoot));

    /* Relocate hbeg and hcur. */
    mkr_dup(&a_hist->hbeg, &a_hist->hend);
    mkr_dup(&a_hist->hcur, &a_hist->hend);

    /* Reset header and footer. */
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_NONE);
    histh_p_footer_tag_set(&a_hist->hhead, HISTH_TAG_NONE);

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

    /* Initialize and insert header. */
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_GEND);
    mkr_before_insert(&a_hist->hend, histh_p_bufv_get(&a_hist->hhead),
		      histh_p_bufvcnt_get(&a_hist->hhead));

    /* Initialize and insert footer. */
    histh_p_footer_tag_set(&a_hist->hfoot, HISTH_TAG_GEND);
    mkr_before_insert(&a_hist->hend, histh_p_bufv_get(&a_hist->hfoot),
		      histh_p_bufvcnt_get(&a_hist->hfoot));

    /* Relocate hbeg and hcur. */
    mkr_dup(&a_hist->hbeg, &a_hist->hend);
    mkr_dup(&a_hist->hcur, &a_hist->hend);

    /* Reset header and footer. */
    histh_p_header_tag_set(&a_hist->hhead, HISTH_TAG_NONE);
    histh_p_footer_tag_set(&a_hist->hhead, HISTH_TAG_NONE);

    /* Decrease depth. */
    cw_assert(a_hist->gdepth != 0);
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
			   HISTH_TAG_LINS);
}

void
hist_ynk(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_LYNK);
}

void
hist_rem(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_LREM);
}

void
hist_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos,
	 const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    hist_p_ins_ynk_rem_del(a_hist, a_buf, a_bpos, a_bufv, a_bufvcnt,
			   HISTH_TAG_LDEL);
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

    /* hhead. */
    fprintf(stderr, "%s|\n", mid);
    asprintf(&pbeg, "%s|-> hhead: ", mid);
    asprintf(&pmid, "%s|          ", mid);
    histh_p_dump(&a_hist->hhead, pbeg, pmid, NULL);
    free(pbeg);
    free(pmid);

    /* hfoot. */
    fprintf(stderr, "%s|\n", mid);
    asprintf(&pbeg, "%s|-> hfoot: ", mid);
    asprintf(&pmid, "%s|          ", mid);
    histh_p_dump(&a_hist->hfoot, pbeg, pmid, NULL);
    free(pbeg);
    free(pmid);

    /* Markers. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> hbeg: %p\n", mid, &a_hist->hbeg);
    fprintf(stderr, "%s|-> hcur: %p\n", mid, &a_hist->hcur);
    fprintf(stderr, "%s|-> hend: %p\n", mid, &a_hist->hend);
    fprintf(stderr, "%s|-> htmp: %p\n", mid, &a_hist->htmp);
#endif

    /* hbpos. */
    fprintf(stderr, "%s|-> hbpos: %llu\n", mid, a_hist->hbpos);

    /* gdepth. */
    fprintf(stderr, "%s|-> gdepth: %u\n", mid, a_hist->gdepth);

#ifdef CW_HIST_DUMP
    /* Allocator state. */
    fprintf(stderr, "%s|\n", mid);
    fprintf(stderr, "%s|-> mema: %p\n", mid, a_hist->mema);
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
	    histh_p_footer_before_get(&tfoot, &tend, &ttmp);
	    histh_p_header_after_get(&thead, &tbeg, &ttmp);
	}

	/* Iterate through records. */
	while (mkr_pos(&tend) > 1)
	{
	    switch (histh_p_tag_get(&thead))
	    {
		case HISTH_TAG_SYNC:
		{
		    fprintf(stderr, "%s|         S\n", mid);
		    break;
		}
		case HISTH_TAG_GBEG:
		{
		    fprintf(stderr, "%s|         B\n", mid);
		    break;
		}
		case HISTH_TAG_GEND:
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
		case HISTH_TAG_SINS:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%ui\n",
			    histh_p_saux_get(&thead));
		    break;
		}
		case HISTH_TAG_SYNK:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%uy\n",
			    histh_p_saux_get(&thead));
		    break;
		}
		case HISTH_TAG_SREM:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%ur\n",
			    histh_p_saux_get(&thead));
		    break;
		}
		case HISTH_TAG_SDEL:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%ud\n",
			    histh_p_saux_get(&thead));
		    break;
		}
		case HISTH_TAG_LINS:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%lluI\n",
			    histh_p_aux_get(&thead));
		    break;
		}
		case HISTH_TAG_LYNK:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%lluY\n",
			    histh_p_aux_get(&thead));
		    break;
		}
		case HISTH_TAG_LREM:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%lluR\n",
			    histh_p_aux_get(&thead));
		    break;
		}
		case HISTH_TAG_LDEL:
		{
		    fprintf(stderr, "%s|         (", mid);
		    bufv = mkr_range_get(&ttmp, &tcur, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")%lluD\n",
			    histh_p_aux_get(&thead));
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
	    histh_p_header_after_get(&thead, &tbeg, &ttmp);
	    histh_p_footer_before_get(&tfoot, &tend, &ttmp);
	}

	/* Iterate through records. */
	while (mkr_pos(&tbeg) < buf_len(&a_hist->h) + 1)
	{
	    switch (histh_p_tag_get(&thead))
	    {
		case HISTH_TAG_SYNC:
		{
		    fprintf(stderr, "%s|         S\n", mid);
		    break;
		}
		case HISTH_TAG_GBEG:
		{
		    fprintf(stderr, "%s|         B\n", mid);
		    break;
		}
		case HISTH_TAG_GEND:
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
		case HISTH_TAG_SINS:
		{
		    fprintf(stderr, "%s|         i%u(", mid,
			    histh_p_saux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_SYNK:
		{
		    fprintf(stderr, "%s|         y%u(\n", mid,
			    histh_p_saux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_SREM:
		{
		    fprintf(stderr, "%s|         r%u(", mid,
			    histh_p_saux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_SDEL:
		{
		    fprintf(stderr, "%s|         d%u(", mid,
			    histh_p_saux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_LINS:
		{
		    fprintf(stderr, "%s|         I%llu(", mid,
			    histh_p_aux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_LYNK:
		{
		    fprintf(stderr, "%s|         Y%llu(\n", mid,
			    histh_p_aux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_LREM:
		{
		    fprintf(stderr, "%s|         R%llu(", mid,
			    histh_p_aux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
		    break;
		}
		case HISTH_TAG_LDEL:
		{
		    fprintf(stderr, "%s|         D%llu(", mid,
			    histh_p_aux_get(&thead));
		    bufv = mkr_range_get(&tcur, &ttmp, &bufvcnt);
		    hist_p_bufv_print(bufv, bufvcnt);
		    fprintf(stderr, ")\n");
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
    cw_assert(mkr_buf(&a_hist->hbeg) == &a_hist->h);
    cw_assert(mkr_buf(&a_hist->hcur) == &a_hist->h);
    cw_assert(mkr_buf(&a_hist->hend) == &a_hist->h);
    cw_assert(mkr_buf(&a_hist->htmp) == &a_hist->h);

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
	    histh_p_footer_before_get(&tfoot, &tend, &ttmp);
	    histh_p_header_after_get(&thead, &tbeg, &ttmp);
	}

	/* Iterate through records. */
	while (mkr_pos(&tend) > 1)
	{
	    cw_assert(histh_p_tag_get(&thead) == histh_p_tag_get(&tfoot));

	    switch (histh_p_tag_get(&thead))
	    {
		case HISTH_TAG_SYNC:
		case HISTH_TAG_GBEG:
		case HISTH_TAG_GEND:
		{
		    break;
		}
		case HISTH_TAG_POS:
		{
		    cw_assert(histh_p_aux_get(&thead) != 0);
		    cw_assert(histh_p_aux_get(&tfoot) != 0);
		    break;
		}
		case HISTH_TAG_SINS:
		case HISTH_TAG_SYNK:
		case HISTH_TAG_SREM:
		case HISTH_TAG_SDEL:
		{
		    cw_assert(histh_p_saux_get(&thead)
			      == histh_p_saux_get(&tfoot));
		    cw_assert(mkr_pos(&ttmp) <= mkr_pos(&tcur));
		    break;
		}
		case HISTH_TAG_LINS:
		case HISTH_TAG_LYNK:
		case HISTH_TAG_LREM:
		case HISTH_TAG_LDEL:
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
	    histh_p_header_after_get(&thead, &tbeg, &ttmp);
	    histh_p_footer_before_get(&tfoot, &tend, &ttmp);
	}

	/* Iterate through records. */
	while (mkr_pos(&tbeg) < buf_len(&a_hist->h) + 1)
	{
	    cw_assert(histh_p_tag_get(&thead) == histh_p_tag_get(&tfoot));

	    switch (histh_p_tag_get(&thead))
	    {
		case HISTH_TAG_SYNC:
		case HISTH_TAG_GBEG:
		case HISTH_TAG_GEND:
		{
		    break;
		}
		case HISTH_TAG_POS:
		{
		    cw_assert(histh_p_aux_get(&thead) != 0);
		    cw_assert(histh_p_aux_get(&tfoot) != 0);
		    break;
		}
		case HISTH_TAG_SINS:
		case HISTH_TAG_SYNK:
		case HISTH_TAG_SREM:
		case HISTH_TAG_SDEL:
		{
		    cw_assert(histh_p_saux_get(&thead)
			      == histh_p_saux_get(&tfoot));
		    cw_assert(mkr_pos(&tcur) <= mkr_pos(&ttmp));
		    break;
		}
		case HISTH_TAG_LINS:
		case HISTH_TAG_LYNK:
		case HISTH_TAG_LREM:
		case HISTH_TAG_LDEL:
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
