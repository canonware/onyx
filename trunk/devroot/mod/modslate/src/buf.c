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
 * This file contains a buffer gap implementation of buffers for use in the
 * slate text editor.  The code is broken up into the following classes:
 *
 * buf  : Main buffer class.
 * mkr : Marker.  Markers are used as handles for many buf operations.
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
 * The only significant impact that this has is on mkr_rang_get().  It is only
 * safe to treat the return value as a pointer to a string in the buffer if the
 * element size is 1.
 *
 ******************************************************************************/

#include "../include/modslate.h"

/* Prototypes. */
static cw_uint64_t
buf_p_pos_b2a(cw_buf_t *a_buf, cw_uint64_t a_bpos);

static cw_uint64_t
buf_p_pos_a2b(cw_buf_t *a_buf, cw_uint64_t a_apos);

static cw_uint64_t
buf_p_lines_rel_forward_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
			      cw_uint64_t a_nlines);

static cw_uint64_t
buf_p_lines_rel_backward_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
			       cw_uint64_t a_nlines);

static cw_uint64_t
buf_p_lines_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
		  cw_uint64_t a_apos_end);

static void
buf_p_mkrs_apos_adjust(cw_buf_t *a_buf, cw_mkr_t *a_mkr,
			cw_sint64_t a_adjust, cw_uint64_t a_beg_apos,
			cw_uint64_t a_end_apos);

static void
buf_p_gap_move(cw_buf_t *a_buf, cw_mkr_t *a_mkr, cw_uint64_t a_bpos);

static void
buf_p_grow(cw_buf_t *a_buf, cw_uint64_t a_minlen);

static void
buf_p_shrink(cw_buf_t *a_buf);

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

/* buf. */
static cw_uint64_t
buf_p_pos_b2a(cw_buf_t *a_buf, cw_uint64_t a_bpos)
{
    cw_uint64_t apos;

    cw_assert(a_bpos > 0);
    cw_assert(a_bpos <= a_buf->len + 1);

    if (a_bpos <= a_buf->gap_off)
    {
	apos = a_bpos - 1;
    }
    else
    {
	apos = a_bpos - 1 + a_buf->gap_len;
    }

    return apos;
}

static cw_uint64_t
buf_p_pos_a2b(cw_buf_t *a_buf, cw_uint64_t a_apos)
{
    cw_uint64_t bpos;

    cw_assert(a_apos <= a_buf->gap_off
	      || a_apos >= a_buf->gap_off + a_buf->gap_len);

    if (a_apos <= a_buf->gap_off)
    {
	bpos = a_apos + 1;
    }
    else
    {
	bpos = a_apos + 1 - a_buf->gap_len;
    }

    return bpos;
}

static cw_uint64_t
buf_p_lines_rel_forward_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
			      cw_uint64_t a_nlines)
{
    cw_uint64_t apos, nlines;

    /* Move to the "a_nlines"th '\n' character after a_apos_beg, taking care to
     * avoid the gap. */
    for (apos = a_apos_beg, nlines = 0; apos < a_buf->gap_off; apos++)
    {
	if (a_buf->b[apos] == '\n')
	{
	    nlines++;
	    if (nlines == a_nlines)
	    {
		goto DONE;
	    }
	}
    }

    if (apos == a_buf->gap_off)
    {
	/* Skip the gap. */
	apos += a_buf->gap_len;
    }

    for (;; apos++)
    {
	if (a_buf->b[apos] == '\n')
	{
	    nlines++;
	    if (nlines == a_nlines)
	    {
		goto DONE;
	    }
	}
    }

    DONE:
    return apos;
}

static cw_uint64_t
buf_p_lines_rel_backward_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg,
			       cw_uint64_t a_nlines)
{
    cw_uint64_t apos, nlines;

    /* Move past a_nlines '\n' characters, taking care to avoid the gap. */
    for (apos = a_apos_beg - 1, nlines = 0;
	 apos >= a_buf->gap_off + a_buf->gap_len;
	 apos--)
    {
	if (a_buf->b[apos] == '\n')
	{
	    nlines++;
	    if (nlines == a_nlines)
	    {
		goto DONE;
	    }
	}
    }

    if (apos == a_buf->gap_off + a_buf->gap_len - 1)
    {
	/* Skip the gap. */
	apos -= a_buf->gap_len;
    }

    for (;; apos--)
    {
	if (a_buf->b[apos] == '\n')
	{
	    nlines++;
	    if (nlines == a_nlines)
	    {
		goto DONE;
	    }
	}
    }

    DONE:
    /* apos is now at the '\n', but we need to return the apos after the '\n'.
     * Add 1 to apos, then make sure it isn't in the gap. */
    apos++;
    if (apos >= a_buf->gap_off && apos < a_buf->gap_off + a_buf->gap_len)
    {
	apos += a_buf->gap_len;
    }
    return apos;
}

static cw_uint64_t
buf_p_lines_count(cw_buf_t *a_buf, cw_uint64_t a_apos_beg, cw_uint64_t
		  a_apos_end)
{
    cw_uint64_t retval, apos;

    cw_assert(a_apos_beg <= a_buf->gap_off
	      || a_apos_beg >= a_buf->gap_off + a_buf->gap_len);
    cw_assert(a_apos_beg <= a_buf->len + a_buf->gap_len);
    cw_assert(a_apos_end <= a_buf->gap_off
	      || a_apos_end >= a_buf->gap_off + a_buf->gap_len);
    cw_assert(a_apos_end <= a_buf->len + a_buf->gap_len);
    cw_assert(a_apos_beg <= a_apos_end);

    retval = 0;

    /* Count the number of '\n' characters, taking care to avoid the gap. */
    for (apos = a_apos_beg;
	 apos < a_apos_end && apos < a_buf->gap_off;
	 apos++)
    {
	if (a_buf->b[apos] == '\n')
	{
	    retval++;
	}
    }

    if (apos == a_buf->gap_off)
    {
	apos += a_buf->gap_len;
    }
    for (; apos < a_apos_end; apos++)
    {
	if (a_buf->b[apos] == '\n')
	{
	    retval++;
	}
    }

    return retval;
}

static void
buf_p_mkrs_apos_adjust(cw_buf_t *a_buf, cw_mkr_t *a_mkr,
			cw_sint64_t a_adjust, cw_uint64_t a_beg_apos,
			cw_uint64_t a_end_apos)
{
    cw_mkr_t *mkr;

    cw_check_ptr(a_buf);
    cw_check_ptr(a_mkr);
    cw_assert(a_beg_apos < a_end_apos);

    /* Adjust apos field of affected mkr's. a_mkr is either in or adjacent to
     * the affected range.  Starting at a_mkr, go both directions until out of
     * the affected range or until past the beginning/end of the list.  Extra
     * care must be taken to ignore mkr's at the starting apos if a_mkr is
     * merely adjacent to the affected region. */

    /* Forward (including a_mkr). */
    for (mkr = a_mkr;
	 mkr != NULL && mkr->apos >= a_beg_apos && mkr->apos < a_end_apos;
	 mkr = ql_next(&a_buf->mkrs, mkr, link))
    {
	mkr->apos += a_adjust;
    }

    /* Backward. */
    for (mkr = ql_prev(&a_buf->mkrs, a_mkr, link);
	 mkr != NULL && mkr->apos == a_end_apos;
	 mkr = ql_prev(&a_buf->mkrs, mkr, link))
    {
	/* Ignore. */
    }

    for (;
	 mkr != NULL && mkr->apos >= a_beg_apos;
	 mkr = ql_prev(&a_buf->mkrs, mkr, link))
    {
	mkr->apos += a_adjust;
    }
}

static void
buf_p_gap_move(cw_buf_t *a_buf, cw_mkr_t *a_mkr, cw_uint64_t a_bpos)
{
    cw_uint64_t apos;

    cw_assert(a_bpos > 0);
    cw_assert(a_bpos <= a_buf->len + 1);

    apos = a_bpos - 1;

    /* Move the gap if it isn't already where it needs to be. */
    if (a_buf->gap_off != apos)
    {
	if (a_buf->gap_off < apos)
	{
	    /* Move the gap forward.
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
	     * oooooooXXXXXXXXXXX________oo */
	    memmove(&a_buf->b[a_buf->gap_off],
		    &a_buf->b[(a_buf->gap_off + a_buf->gap_len)],
		    (apos - a_buf->gap_off));

	    /* Adjust the apos of all mkr's with apos in the moved region. */
	    buf_p_mkrs_apos_adjust(a_buf, a_mkr, -a_buf->gap_len,
				    a_buf->gap_off + a_buf->gap_len,
				    apos + a_buf->gap_len);
	}
	else
	{
	    /* Move the gap backward.
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
	     * oooo___________XXXXXXXXXoooo */
	    memmove(&a_buf->b[(a_buf->gap_len + apos)],
		    &a_buf->b[apos],
		    (a_buf->gap_off - apos));

	    /* Adjust the apos of all mkr's with apos in the moved region. */
	    buf_p_mkrs_apos_adjust(a_buf, a_mkr, a_buf->gap_len, apos,
				    a_buf->gap_off);
	}
	a_buf->gap_off = apos;
    }
}

static void
buf_p_grow(cw_buf_t *a_buf, cw_uint64_t a_minlen)
{
    cw_uint64_t old_size, new_size;

    old_size = a_buf->len + a_buf->gap_len;

    for (new_size = old_size << 1; new_size < a_minlen; new_size <<= 1)
    {
	/* Iteratively double new_size until it is big enough to contain
	 * a_minlen elements. */
    }

    /* Move the gap to the end before reallocating. */
    buf_p_gap_move(a_buf, ql_last(&a_buf->mkrs, link), a_buf->len + 1);

    a_buf->b = (cw_uint8_t *) cw_opaque_realloc(a_buf->realloc, a_buf->arg,
						a_buf->b, new_size, old_size);

    /* Adjust the gap length. */
    a_buf->gap_len += new_size - old_size;
}

static void
buf_p_shrink(cw_buf_t *a_buf)
{
    cw_uint64_t old_size, new_size;

    old_size = a_buf->len + a_buf->gap_len;

    for (new_size = old_size;
	 (new_size >> 1) > a_buf->len && new_size > CW_BUF_MINELMS;
	 new_size >>= 1)
    {
	/* Iteratively halve new_size until the actual buffer size is between
	 * 25% (exclusive) and 50% (inclusive) of new_size, or until new_size is
	 * the minimum size. */
    }

    /* Only shrink the gap if the above loop determined that there is excessive
     * space being used by the gap. */
    if (old_size > new_size)
    {
	/* Move the gap to the end. */
	buf_p_gap_move(a_buf, ql_last(&a_buf->mkrs, link), a_buf->len + 1);

	/* Shrink the gap. */
	a_buf->b = (cw_uint8_t *) cw_opaque_realloc(a_buf->realloc,
						    a_buf->arg, a_buf->b,
						    new_size, old_size);

	/* Adjust the gap length. */
	a_buf->gap_len -= old_size - new_size;
    }
}

cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
	cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc,
	void *a_arg)
{
    cw_buf_t *retval;

    /* The memset() isn't strictly necessary, since we initialize all fields,
     * but it may clean up some clutter that could be confusing when
     * debugging. */
    if (a_buf != NULL)
    {
	retval = a_buf;
	memset(retval, 0, sizeof(cw_buf_t));
	retval->alloced = FALSE;
    }
    else
    {
	retval = (cw_buf_t *) cw_opaque_alloc(a_alloc, a_arg, sizeof(cw_buf_t));
	memset(retval, 0, sizeof(cw_buf_t));
	retval->alloced = TRUE;
    }

    retval->alloc = a_alloc;
    retval->realloc = a_realloc;
    retval->dealloc = a_dealloc;
    retval->arg = a_arg;

    retval->b = (cw_uint8_t *) cw_opaque_alloc(a_alloc, a_arg, CW_BUF_MINELMS);
    retval->len = 0;
    retval->nlines = 1;
    retval->gap_off = 0;
    retval->gap_len = CW_BUF_MINELMS;

    /* Initialize history. */
    retval->hist = NULL;

    /* Initialize lists. */
    ql_new(&retval->mkrs);
    ql_new(&retval->fexts);
    ql_new(&retval->rexts);

#ifdef CW_DBG
    retval->magic = CW_BUF_MAGIC;
#endif

    return retval;
}

void
buf_delete(cw_buf_t *a_buf)
{
    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);
    cw_assert(ql_first(&a_buf->mkrs) == NULL);

    if (a_buf->hist != NULL)
    {
	hist_delete(a_buf->hist);
    }

    cw_opaque_dealloc(a_buf->dealloc, a_buf->arg, a_buf->b,
		      (a_buf->len + a_buf->gap_len));

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
buf_len(cw_buf_t *a_buf)
{
    cw_uint64_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    retval = a_buf->len;

    return retval;
}

cw_uint64_t
buf_nlines(cw_buf_t *a_buf)
{
    cw_uint64_t retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    retval = a_buf->nlines;

    return retval;
}

cw_bool_t
buf_hist_active_get(cw_buf_t *a_buf)
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
buf_undoable(cw_buf_t *a_buf)
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
buf_redoable(cw_buf_t *a_buf)
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
void
mkr_l_insert(cw_mkr_t *a_mkr, cw_bool_t a_record, cw_bool_t a_after,
	      const cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt)
{
    cw_uint64_t i, count, nlines;
    cw_buf_t *buf;
    cw_mkr_t *first, *mkr;
    cw_bufv_t bufv;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    buf = a_mkr->buf;

    /* Record the undo information before inserting so that the apos is still
     * unmodified. */
    if (buf->hist != NULL && a_record)
    {
	if (a_after)
	{
	    hist_ynk(buf->hist, buf, buf_p_pos_a2b(buf, a_mkr->apos), a_bufv,
		     a_bufvcnt);
	}
	else
	{
	    hist_ins(buf->hist, buf, buf_p_pos_a2b(buf, a_mkr->apos), a_bufv,
		     a_bufvcnt);
	}
    }

    /* Determine the total number of elements to be inserted. */
    count = 0;
    for (i = 0; i < a_bufvcnt; i++)
    {
	count += a_bufv[i].len;
    }

    /* Make sure that the data will fit. */
    if (count >= buf->gap_len)
    {
	buf_p_grow(buf, buf->len + count);
    }

    /* Move the gap. */
    buf_p_gap_move(buf, a_mkr, buf_p_pos_a2b(buf, a_mkr->apos));

    /* Insert. */
    bufv.data = &buf->b[buf->gap_off];
    bufv.len = buf->gap_len;
    nlines = bufv_p_copy(&bufv, 1, a_bufv, a_bufvcnt);

    /* Shrink the gap. */
    buf->gap_off += count;
    buf->gap_len -= count;

    /* Adjust the buf's length and line count. */
    buf->len += count;
    buf->nlines += nlines;

    /* If there are multiple mkr's at the same position as a_mkr, make sure
     * that a_mkr is the first mkr in the mkr list, in order to simplify
     * later list iteration operations and allow moving a_mkr. */
    for (first = NULL, mkr = ql_prev(&buf->mkrs, a_mkr, link);
	 mkr != NULL && mkr->apos == a_mkr->apos;
	 mkr = ql_prev(&buf->mkrs, mkr, link))
    {
	first = mkr;
    }

    if (first != NULL)
    {
	ql_remove(&buf->mkrs, a_mkr, link);
	ql_before_insert(&buf->mkrs, first, a_mkr, link);
    }

    /* If inserting after a_mkr, move a_mkr before the data just inserted.
     * This relies on the mkr list re-ordering above, since moving a_mkr would
     * otherwise require re-insertion into the mkr list. */
    if (a_after)
    {
	a_mkr->apos = buf_p_pos_b2a(buf,
				     buf_p_pos_a2b(buf, a_mkr->apos) - count);
    }

    if (nlines > 0)
    {
	/* Adjust line. */
	if (a_after == FALSE)
	{
	    a_mkr->line += nlines;

	    /* Adjust line for all mkr's at the same position. */
	    for (mkr = ql_next(&buf->mkrs, a_mkr, link);
		 mkr != NULL && mkr->apos == a_mkr->apos;
		 mkr = ql_next(&buf->mkrs, mkr, link))
	    {
		mkr->line += nlines;
	    }
	}
	else
	{
	    /* Move past mkr's at the same position. */
	    for (mkr = ql_next(&buf->mkrs, a_mkr, link);
		 mkr != NULL && mkr->apos == a_mkr->apos;
		 mkr = ql_next(&buf->mkrs, mkr, link))
	    {
		/* Do nothing. */
	    }
	}

	/* Adjust line for all following mkr's. */
	for (;
	     mkr != NULL;
	     mkr = ql_next(&buf->mkrs, mkr, link))
	{
	    mkr->line += nlines;
	}
    }
}

void
mkr_l_remove(cw_mkr_t *a_start, cw_mkr_t *a_end, cw_bool_t a_record)
{
    cw_buf_t *buf;
    cw_mkr_t *start, *end, *mkr;
    cw_uint64_t start_bpos, end_bpos, rcount, nlines;
    cw_bufv_t bufv;

    cw_check_ptr(a_start);
    cw_dassert(a_start->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_start->buf);
    cw_check_ptr(a_end);
    cw_dassert(a_end->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_end->buf);
    cw_assert(a_start->buf == a_end->buf);

    if (a_start->apos < a_end->apos)
    {
	start = a_start;
	end = a_end;
    }
    else if (a_start->apos > a_end->apos)
    {
	start = a_end;
	end = a_start;
    }
    else
    {
	/* No data need to be removed. */
	return;
    }

    buf = start->buf;

    /* Get bpos for start and end, since they are used more than once. */
    start_bpos = buf_p_pos_a2b(buf, start->apos);
    end_bpos = buf_p_pos_a2b(buf, end->apos);

    /* Calculate the number of elements being removed, since it is used more
     * than once. */
    rcount = end_bpos - start_bpos;

    /* Move the gap. */
    buf_p_gap_move(buf, start, start_bpos);

    /* Record undo information, now that the gap has been moved and the elements
     * to be removed are contiguous.  The ordering of a_start and a_end
     * determines whether this is a before/after removal. */
    if (buf->hist != NULL && a_record)
    {
	bufv.data = &buf->b[start->apos];
	bufv.len = rcount;
	if (start == a_start)
	{
	    hist_del(buf->hist, buf, buf_p_pos_a2b(buf, start->apos), &bufv, 1);
	}
	else
	{
	    hist_rem(buf->hist, buf, buf_p_pos_a2b(buf, end->apos), &bufv, 1);
	}
    }

    /* Grow the gap. */
    buf->gap_len += rcount;

    /* Adjust apos for all mkr's before start that are at the same position. */
    for (mkr = ql_prev(&buf->mkrs, start, link);
	 mkr != NULL && mkr->apos == start->apos;
	 mkr = ql_prev(&buf->mkrs, mkr, link))
    {
	mkr->apos = end->apos;
    }

    /* Adjust apos and line for all mkr's from start (inclusive) to end
     * (exclusive). */
    for (mkr = start;
	 mkr->apos < end->apos;
	 mkr = ql_next(&buf->mkrs, mkr, link))
    {
	mkr->apos = end->apos;
	mkr->line = start->line;
    }
	
    nlines = end->line - start->line;

    /* Adjust the line for all mkr's after the gap. */
    for (;
	 mkr != NULL;
	 mkr = ql_next(&buf->mkrs, mkr, link))
    {
	mkr->line -= nlines;
    }

    /* Adjust the buf's len and nlines. */
    buf->len -= rcount;
    buf->nlines -= nlines;

    /* Try to shrink the gap. */
    buf_p_shrink(buf);
}

cw_mkr_t *
mkr_new(cw_mkr_t *a_mkr, cw_buf_t *a_buf)
{
    cw_mkr_t *retval;

    cw_check_ptr(a_buf);
    cw_dassert(a_buf->magic == CW_BUF_MAGIC);

    if (a_mkr == NULL)
    {
	retval = (cw_mkr_t *) cw_opaque_alloc(a_buf->alloc, a_buf->arg,
					       sizeof(cw_mkr_t));
	retval->malloced = TRUE;
    }
    else
    {
	retval = a_mkr;
	retval->malloced = FALSE;
    }

    ql_elm_new(retval, link);
    retval->buf = a_buf;
    retval->apos = buf_p_pos_b2a(a_buf, 1);
    retval->line = 1;

    ql_head_insert(&a_buf->mkrs, retval, link);

#ifdef CW_DBG
    retval->magic = CW_MKR_MAGIC;
#endif

    return retval;
}

void
mkr_dup(cw_mkr_t *a_to, cw_mkr_t *a_from)
{
    cw_check_ptr(a_to);
    cw_dassert(a_to->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_to->buf);
    cw_check_ptr(a_from);
    cw_dassert(a_from->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_from->buf);
    cw_assert(a_to->buf == a_from->buf);

    a_to->apos = a_from->apos;
    a_to->line = a_from->line;

    ql_remove(&a_to->buf->mkrs, a_to, link);
    ql_after_insert(a_from, a_to, link);
}

void
mkr_delete(cw_mkr_t *a_mkr)
{
    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

    if (a_mkr->malloced)
    {
	cw_opaque_dealloc(a_mkr->buf->dealloc, a_mkr->buf->arg, a_mkr,
			  sizeof(cw_mkr_t));
    }
#ifdef CW_DBG
    else
    {
	memset(a_mkr, 0x5a, sizeof(cw_mkr_t));
    }
#endif
}

cw_buf_t *
mkr_buf(cw_mkr_t *a_mkr)
{
    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    return a_mkr->buf;
}

cw_uint64_t
mkr_line_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
    cw_mkr_t *mkr;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    /* When checking for attempted seeking out of buf bounds, it is important to
     * handle the cases of movement to exactly BOB or EOB, since there is one
     * more line than there are '\n' characters, and the iteration algorithm
     * would have to check for BOB/EOB if it couldn't be sure that it would stay
     * in bounds. */
    switch (a_whence)
    {
	case BUFW_BEG:
	{
	    /* Make sure not to go out of buf bounds. */
	    if (a_offset <= 0)
	    {
		/* Attempt to move to or before BOB.  Move to BOB. */
		mkr_seek(a_mkr, 0, BUFW_BEG);
		break;
	    }
	    else if (a_offset >= a_mkr->buf->nlines)
	    {
		/* Attempt to move to or past EOB.  Move to EOB. */
		mkr_seek(a_mkr, 0, BUFW_END);
		break;
	    }

	    /* Move forward from BOB to just short of a_offset '\n'
	     * characters.  For example, if seeking forward 2:
	     *
	     *  \/
	     *   hello\ngoodbye\nyadda\nblah
	     *                /\
	     */
	    a_mkr->apos
		= buf_p_lines_rel_forward_count(a_mkr->buf,
						buf_p_pos_b2a(a_mkr->buf, 1),
						a_offset);

	    /* Set the line number. */
	    a_mkr->line = a_offset;

	    /* Relocate in the mkr list. */
	    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

	    for (mkr = ql_first(&a_mkr->buf->mkrs);
		 mkr != NULL && a_mkr->apos > mkr->apos;
		 mkr = ql_next(&a_mkr->buf->mkrs, mkr, link))
	    {
		/* Iterate until the end of the list is reached, or the apos of
		 * a mkr in the list is greater than that of the seeking
		 * mkr. */
	    }

	    if (mkr == NULL)
	    {
		/* Insert at end. */
		ql_tail_insert(&a_mkr->buf->mkrs, a_mkr, link);
	    }
	    else
	    {
		/* Insert before the last mkr looked at. */
		ql_before_insert(&a_mkr->buf->mkrs, mkr, a_mkr, link);
	    }
	    break;
	}
	case BUFW_REL:
	{
	    cw_uint64_t apos;
	    cw_bool_t relocate = FALSE;
		
	    if (a_offset > 0)
	    {
		/* Make sure not to go out of buf bounds. */
		if (a_mkr->line + a_offset > a_mkr->buf->nlines)
		{
		    /* Attempt to move to or after EOB.  Move to EOB. */
		    mkr_seek(a_mkr, 0, BUFW_END);
		    break;
		}
		/* Move forward from the current position to just short
		 * of a_offset '\n' characters.  Fore example, if
		 * seeking forward 2:
		 *
		 *            \/
		 *   hello\ngoodbye\nyadda\nblah
		 *                       /\
		 */
		apos = buf_p_lines_rel_forward_count(a_mkr->buf, a_mkr->apos,
						     a_offset);

		/* Relocate in the mkr list. */
		for (mkr = ql_next(&a_mkr->buf->mkrs, a_mkr, link);
		     mkr != NULL && apos > mkr->apos;
		     mkr = ql_next(&a_mkr->buf->mkrs, mkr, link))
		{
		    /* Iterate until the end of the list is reached, or the apos
		     * of a mkr in the list is greater than that of the seeking
		     * mkr. */
		    relocate = TRUE;
		}

		if (relocate)
		{
		    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

		    if (mkr == NULL)
		    {
			/* Insert at end. */
			ql_tail_insert(&a_mkr->buf->mkrs,
				       a_mkr, link);
		    }
		    else
		    {
			/* Insert before the last mkr looked at. */
			ql_before_insert(&a_mkr->buf->mkrs, mkr, a_mkr,
					 link);
		    }
		}

		/* Set the line number. */
		a_mkr->line += a_offset - 1;

		/* Set the apos of the mkr now that the old value isn't needed
		 * anymore. */
		a_mkr->apos = apos;
	    }
	    else if (a_offset < 0)
	    {
		/* Make sure not to go out of buf bounds. */
		if (-a_offset >= a_mkr->line)
		{
		    /* Attempt to move to or before BOB.  Move to BOB. */
		    mkr_seek(a_mkr, 0, BUFW_BEG);
		    break;
		}

		/* Move backward from the current position to just short
		 * of a_offset '\n' characters.  Fore example, if
		 * seeking backward 2:
		 *
		 *                     \/
		 *   hello\ngoodbye\nyadda\nblah
		 *         /\
		 */
		apos = buf_p_lines_rel_backward_count(a_mkr->buf, a_mkr->apos,
						      -a_offset);
			
		/* Relocate in the mkr list. */
		for (mkr = ql_prev(&a_mkr->buf->mkrs, a_mkr, link);
		     mkr != NULL && apos < mkr->apos;
		     mkr = ql_prev(&a_mkr->buf->mkrs, mkr, link))
		{
		    /* Iterate until the beginning of the list is reached, or
		     * the apos of a mkr in the list is less than that of the
		     * seeking mkr. */
		    relocate = TRUE;
		}

		if (relocate)
		{
		    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

		    if (mkr == NULL)
		    {
			/* Insert at beginning. */
			ql_head_insert(&a_mkr->buf->mkrs,
				       a_mkr, link);
		    }
		    else
		    {
			/* Insert after the last mkr looked at. */
			ql_after_insert(mkr, a_mkr, link);
		    }
		}

		/* Set the line number. */
		a_mkr->line += a_offset + 1;

		/* Set the apos of the mkr now that the old value isn't needed
		 * anymore. */
		a_mkr->apos = apos;
	    }

	    break;
	}
	case BUFW_END:
	{
	    /* Make sure not to go out of buf bounds. */
	    if (a_offset >= 0)
	    {
		/* Attempt to move to or after EOB.  Move to EOB. */
		mkr_seek(a_mkr, 0, BUFW_END);
		break;
	    }
	    else if (a_offset >= a_mkr->buf->nlines)
	    {
		/* Attempt to move to or past BOB.  Move to BOB. */
		mkr_seek(a_mkr, 0, BUFW_BEG);
		break;
	    }

	    /* Move backward from EOB to just short of a_offset '\n'
	     * characters.  For example if seeking backward 2:
	     *
	     *                             \/
	     *   hello\ngoodbye\nyadda\nblah
	     *                  /\
	     */
	    a_mkr->apos
		= buf_p_lines_rel_backward_count(
		    a_mkr->buf,
		    buf_p_pos_b2a(a_mkr->buf, a_mkr->buf->len + 1),
		    -a_offset);

	    /* Set the line number. */
	    a_mkr->line = a_mkr->buf->nlines + a_offset + 1;

	    /* Relocate in the mkr list. */
	    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

	    for (mkr = ql_last(&a_mkr->buf->mkrs, link);
		 mkr != NULL && a_mkr->apos < mkr->apos;
		 mkr = ql_prev(&a_mkr->buf->mkrs, mkr, link))
	    {
		/* Iterate until the beginning of the list is reached, or the
		 * apos of a mkr in the list is less than that of the seeking
		 * mkr. */
	    }

	    if (mkr == NULL)
	    {
		/* Insert at beginning. */
		ql_head_insert(&a_mkr->buf->mkrs, a_mkr, link);
	    }
	    else
	    {
		/* Insert after the last mkr looked at. */
		ql_after_insert(mkr, a_mkr, link);
	    }
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return buf_p_pos_a2b(a_mkr->buf, a_mkr->apos);
}

cw_uint64_t
mkr_line(cw_mkr_t *a_mkr)
{
    cw_uint64_t retval;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    retval = a_mkr->line;

    return retval;
}

cw_uint64_t
mkr_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
    cw_uint64_t bpos;
    cw_mkr_t *mkr;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    switch (a_whence)
    {
	case BUFW_BEG:
	{
	    /* Determine bpos and apos.  Make sure not to go out of buf
	     * bounds. */
	    if (a_offset < 0)
	    {
		bpos = 1;
	    }
	    else if (a_offset > a_mkr->buf->len)
	    {
		bpos = a_mkr->buf->len + 1;
	    }
	    else
	    {
		bpos = a_offset + 1;
	    }

	    a_mkr->apos = buf_p_pos_b2a(a_mkr->buf, bpos);

	    /* Relocate in the mkr list. */
	    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

	    for (mkr = ql_first(&a_mkr->buf->mkrs);
		 mkr != NULL && a_mkr->apos > mkr->apos;
		 mkr = ql_next(&a_mkr->buf->mkrs, mkr, link))
	    {
		/* Iterate until the end of the list is reached, or the apos of
		 * a mkr in the list is greater than that of the seeking
		 * mkr. */
	    }

	    if (mkr == NULL)
	    {
		/* Insert at end. */
		ql_tail_insert(&a_mkr->buf->mkrs, a_mkr, link);
	    }
	    else
	    {
		/* Insert before the last mkr looked at. */
		ql_before_insert(&a_mkr->buf->mkrs, mkr, a_mkr, link);
	    }

	    /* Count the number of newlines and set the line number
	     * accordingly. */
	    a_mkr->line = 1 + buf_p_lines_count(a_mkr->buf,
						 buf_p_pos_b2a(a_mkr->buf, 1),
						 a_mkr->apos);

	    break;
	}
	case BUFW_REL:
	{
	    cw_uint64_t apos;
	    cw_bool_t relocate = FALSE;

	    /* The algorithm differs substantially depending whether seeking
	     * forward or backward.  There is slight code duplication in the two
	     * branches, but this avoids repeated branches. */
	    bpos = buf_p_pos_a2b(a_mkr->buf, a_mkr->apos);
	    if (a_offset > 0)
	    {
		/* Determine bpos and apos.  Make sure not to go out of buf
		 * bounds. */
		if (bpos + a_offset > a_mkr->buf->len + 1)
		{
		    bpos = a_mkr->buf->len + 1;
		}
		else
		{
		    bpos += a_offset;
		}

		apos = buf_p_pos_b2a(a_mkr->buf, bpos);

		/* Relocate in the mkr list. */
		for (mkr = ql_next(&a_mkr->buf->mkrs, a_mkr, link);
		     mkr != NULL && apos > mkr->apos;
		     mkr = ql_next(&a_mkr->buf->mkrs, mkr, link))
		{
		    /* Iterate until the end of the list is reached, or the apos
		     * of a mkr in the list is greater than that of the seeking
		     * mkr. */
		    relocate = TRUE;
		}

		if (relocate)
		{
		    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

		    if (mkr == NULL)
		    {
			/* Insert at end. */
			ql_tail_insert(&a_mkr->buf->mkrs, a_mkr, link);
		    }
		    else
		    {
			/* Insert before the last mkr looked at. */
			ql_before_insert(&a_mkr->buf->mkrs, mkr, a_mkr,
					 link);
		    }
		}

		/* Count the number of newlines moved past and adjust the line
		 * number accordingly. */
		a_mkr->line += buf_p_lines_count(a_mkr->buf, a_mkr->apos,
						  apos);

		/* Set the apos of the mkr now that the old value isn't
		 * needed anymore. */
		a_mkr->apos = apos;
	    }
	    else if (a_offset < 0)
	    {
		/* Determine bpos and apos.  Make sure not to go out of buf
		 * bounds. */
		if (bpos <= -a_offset)
		{
		    bpos = 1;
		}
		else
		{
		    bpos += a_offset;
		}

		apos = buf_p_pos_b2a(a_mkr->buf, bpos);

		/* Relocate in the mkr list. */
		for (mkr = ql_prev(&a_mkr->buf->mkrs, a_mkr, link);
		     mkr != NULL && apos < mkr->apos;
		     mkr = ql_prev(&a_mkr->buf->mkrs, mkr, link))
		{
		    /* Iterate until the beginning of the list is reached, or
		     * the apos of a mkr in the list is less than that of the
		     * seeking mkr. */
		    relocate = TRUE;
		}

		if (relocate)
		{
		    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

		    if (mkr == NULL)
		    {
			/* Insert at beginning. */
			ql_head_insert(&a_mkr->buf->mkrs, a_mkr, link);
		    }
		    else
		    {
			/* Insert after the last mkr looked at. */
			ql_after_insert(mkr, a_mkr, link);
		    }
		}

		/* Count the number of newlines moved past and adjust the line
		 * number accordingly. */
		a_mkr->line -= buf_p_lines_count(a_mkr->buf, apos,
						  a_mkr->apos);

		/* Set the apos of the mkr now that the old value isn't needed
		 * anymore. */
		a_mkr->apos = apos;
	    }

	    break;
	}
	case BUFW_END:
	{
	    /* Determine bpos and apos.  Make sure not to go out of buf
	     * bounds. */
	    if (a_offset > 0)
	    {
		bpos = a_mkr->buf->len + 1;
	    }
	    else if (-a_offset >= a_mkr->buf->len)
	    {
		bpos = 1;
	    }
	    else
	    {
		bpos = a_mkr->buf->len + 1 + a_offset;
	    }

	    a_mkr->apos = buf_p_pos_b2a(a_mkr->buf, bpos);

	    /* Relocate in the mkr list. */
	    ql_remove(&a_mkr->buf->mkrs, a_mkr, link);

	    for (mkr = ql_last(&a_mkr->buf->mkrs, link);
		 mkr != NULL && a_mkr->apos < mkr->apos;
		 mkr = ql_prev(&a_mkr->buf->mkrs, mkr, link))
	    {
		/* Iterate until the beginning of the list is reached, or the
		 * apos of a mkr in the list is less than that of the seeking
		 * mkr. */
	    }

	    if (mkr == NULL)
	    {
		/* Insert at beginning. */
		ql_head_insert(&a_mkr->buf->mkrs, a_mkr, link);
	    }
	    else
	    {
		/* Insert after the last mkr looked at. */
		ql_after_insert(mkr, a_mkr, link);
	    }

	    /* Count the number of newlines and set the line number
	     * accordingly. */
	    a_mkr->line = 1 + buf_p_lines_count(a_mkr->buf,
						 buf_p_pos_b2a(a_mkr->buf, 1),
						 a_mkr->apos);

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return bpos;
}

cw_uint64_t
mkr_pos(cw_mkr_t *a_mkr)
{
    cw_uint64_t retval;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    retval = buf_p_pos_a2b(a_mkr->buf, a_mkr->apos);

    return retval;
}

cw_uint8_t *
mkr_before_get(cw_mkr_t *a_mkr)
{
    cw_uint8_t *retval;
    cw_uint64_t bpos;
    cw_buf_t *buf;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    buf = a_mkr->buf;

    bpos = buf_p_pos_a2b(buf, a_mkr->apos);

    /* Make sure the marker isn't at BOB. */
    if (bpos == 1)
    {
	retval = NULL;
	goto RETURN;
    }

    /* Don't use the marker's apos, in case it is next to the gap. */
    retval = &buf->b[buf_p_pos_b2a(buf, bpos - 1)];

    RETURN:
    return retval;
}

cw_uint8_t *
mkr_after_get(cw_mkr_t *a_mkr)
{
    cw_uint8_t *retval;
    cw_uint64_t bpos;
    cw_buf_t *buf;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->buf);

    buf = a_mkr->buf;

    bpos = buf_p_pos_a2b(buf, a_mkr->apos);

    /* Make sure the marker isn't at EOB. */
    if (bpos == buf->len + 1)
    {
	retval = NULL;
	goto RETURN;
    }

    retval = &buf->b[a_mkr->apos];

    RETURN:
    return retval;
}

cw_bufv_t *
mkr_range_get(cw_mkr_t *a_start, cw_mkr_t *a_end, cw_uint32_t *r_bufvcnt)
{
    cw_mkr_t *start, *end;
    cw_buf_t *buf;

    cw_check_ptr(a_start);
    cw_dassert(a_start->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_start->buf);
    cw_check_ptr(a_end);
    cw_dassert(a_end->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_end->buf);
    cw_assert(a_start->buf == a_end->buf);
    cw_check_ptr(r_bufvcnt);

    buf = a_start->buf;

    if (a_start->apos < a_end->apos)
    {
	start = a_start;
	end = a_end;
    }
    else if (a_start->apos > a_end->apos)
    {
	start = a_end;
	end = a_start;
    }
    else
    {
	/* There are no characters between the two mkr's. */
	*r_bufvcnt = 0;
	goto RETURN;
    }

    /* Set the bufv according to whether the range is split accros the gap. */
    if (buf->gap_off + buf->gap_len <= start->apos || buf->gap_off > end->apos)
    {
	/* Not split. */
	buf->bufv[0].data = &buf->b[start->apos];
	buf->bufv[0].len = end->apos - start->apos;

	*r_bufvcnt = 1;
    }
    else
    {
	/* Split. */
	buf->bufv[0].data = &buf->b[start->apos];
	buf->bufv[0].len = buf->gap_off - start->apos;

	buf->bufv[1].data = &buf->b[(buf->gap_off + buf->gap_len)];
	buf->bufv[1].len = end->apos - buf->gap_off - buf->gap_len;

	*r_bufvcnt = 2;
    }

    RETURN:
    return buf->bufv;
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
ext_buf(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

cw_uint64_t
ext_beg_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_beg_set(cw_ext_t *a_ext, cw_uint64_t a_beg)
{
    cw_error("XXX Not implemented");
}

cw_uint64_t
ext_end_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_end_set(cw_ext_t *a_ext, cw_uint64_t a_end)
{
    cw_error("XXX Not implemented");
}

cw_bool_t
ext_beg_open_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_beg_open_set(cw_ext_t *a_ext, cw_bool_t a_beg_open)
{
    cw_error("XXX Not implemented");
}

cw_bool_t
ext_end_open_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_end_open_set(cw_ext_t *a_ext, cw_bool_t a_end_open)
{
    cw_error("XXX Not implemented");
}

cw_bool_t
ext_detachable_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

void
ext_detachable_set(cw_ext_t *a_ext, cw_bool_t a_detachable)
{
    cw_error("XXX Not implemented");
}

cw_bool_t
ext_detached_get(cw_ext_t *a_ext)
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

cw_ext_t *
ext_before_get(cw_ext_t *a_ext, cw_mkr_t *a_mkr)
{
    cw_error("XXX Not implemented");
}

cw_ext_t *
ext_at_get(cw_ext_t *a_ext, cw_mkr_t *a_mkr)
{
    cw_error("XXX Not implemented");
}

cw_ext_t *
ext_after_get(cw_ext_t *a_ext, cw_mkr_t *a_mkr)
{
    cw_error("XXX Not implemented");
}

cw_ext_t *
ext_prev_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}

cw_ext_t *
ext_next_get(cw_ext_t *a_ext)
{
    cw_error("XXX Not implemented");
}
