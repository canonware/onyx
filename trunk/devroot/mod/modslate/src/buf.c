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
 * ext  : Extent.  Extents denote buf regions.
 *
 ******************************************************************************
 *
 * Buffer position numbering starts at 1.  Buffer position 0 is invalid.
 *
 * Position rules:
 *
 * *) apos refers to absolute position.
 * *) bpos refers to buffer position.
 * *) If a position isn't specified as apos or bpos, then it is bpos.
 *
 * Internal buffer page representation:
 *
 * ppos:   0   1   2   3   4   5   6   7     0   1   2   3   4   5   6   7
 *         |   |   |   |   |   |   |   |     |   |   |   |   |   |   |   |
 *         v   v   v   v   v   v   v   v     v   v   v   v   v   v   v   v
 *       /---+---+---+---+---+---+---+---\ /---+---+---+---+---+---+---+---\
 *       | A | B | C |:::|:::|:::| D | E | | F | G |:::|:::|:::|:::| H | I |
 *       \---+---+---+---+---+---+---+---/ \---+---+---+---+---+---+---+---/
 *       ^   ^   ^               ^   ^     ^   ^                       ^   ^
 *       |   |   |               |   |     |   |                       |   |
 * bpos: 1   2   3               4   5     6   7                       8   9
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
    cw_error("XXX Not implemented");
}

static void
bufp_p_delete(cw_bufp_t *a_bufp)
{
    cw_error("XXX Not implemented");
}

CW_INLINE cw_uint64_t
bufp_p_bpos_get(cw_bufp_t *a_bufp)
{
    cw_error("XXX Not implemented");
}

CW_INLINE void
bufp_p_bpos_set(cw_bufp_t *a_bufp, cw_uint64_t a_bpos)
{
    cw_error("XXX Not implemented");
}

CW_INLINE cw_uint64_t
bufp_p_line_get(cw_bufp_t *a_bufp)
{
    cw_error("XXX Not implemented");
}

CW_INLINE void
bufp_p_line_set(cw_bufp_t *a_bufp, cw_uint64_t a_line)
{
    cw_error("XXX Not implemented");
}

static void
bufp_p_cache_validate(cw_bufp_t *a_bufp)
{
    cw_error("XXX Not implemented");
}

static cw_uint32_t
bufp_p_pos_b2p(cw_bufp_t *a_bufp, cw_uint64_t a_bpos)
{
    cw_error("XXX Not implemented");
}

static cw_uint32_t
bufp_p_pos_p2b(cw_bufp_t *a_bufp, cw_uint64_t a_ppos)
{
    cw_error("XXX Not implemented");
}

/* buf. */
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
	rb_first(&a_buf->ftree, rb_root(&a_buf->ftree), fnode, first);
	cw_assert(first == NULL);
    }
#endif
    cw_assert(ql_first(&a_buf->flist) == NULL);
#ifdef CW_DBG
    {
	cw_ext_t *first;
	rb_first(&a_buf->rtree, rb_root(&a_buf->rtree), rnode, first);
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
    rb_insert(&bufp->mkr_tree, a_mkr, mkr_p_comp, cw_mkr_t, node);

    /* Insert into list.  Make sure that the tree and list orders are the same.
     */
    rb_next(&bufp->mkr_tree, a_mkr, cw_mkr_t, node, next);
    if (next != NULL)
    {
	ql_before_insert(&bufp->mkr_list, next, a_mkr, link);
    }
    else
    {
	ql_head_insert(&bufp->mkr_list, a_mkr, link);
    }
}

static void
mkr_p_remove(cw_mkr_t *a_mkr)
{
    cw_bufp_t *bufp = a_mkr->bufp;

    rb_remove(&bufp->mkr_tree, a_mkr, cw_mkr_t, node);
    ql_remove(&bufp->mkr_list, a_mkr, link);
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
    a_mkr->line = 0;
    rb_node_new(&bufp->mkr_tree, a_mkr, node);
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
    cw_check_ptr(a_to->bufp);
    cw_check_ptr(a_from);
    cw_dassert(a_from->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_from->bufp);
    cw_assert(a_to->bufp->buf == a_from->bufp->buf);

    mkr_p_remove(a_to);

    a_to->bufp = a_from->bufp;
    a_to->ppos = a_from->ppos;
    a_to->line = a_from->line;

    mkr_p_insert(a_to);
}

void
mkr_delete(cw_mkr_t *a_mkr)
{
    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->bufp);

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
    cw_check_ptr(a_mkr->bufp);

    return a_mkr->bufp->buf;
}

cw_uint64_t
mkr_line_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
    cw_error("XXX Not implemented");
}

cw_uint64_t
mkr_line(cw_mkr_t *a_mkr)
{
    cw_uint64_t retval;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->bufp);

    bufp_p_cache_validate(a_mkr->bufp);
    retval = a_mkr->bufp->line + a_mkr->line;

    return retval;
}

cw_uint64_t
mkr_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence)
{
    cw_error("XXX Not implemented");
}

cw_uint64_t
mkr_pos(const cw_mkr_t *a_mkr)
{
    cw_uint64_t retval;

    cw_check_ptr(a_mkr);
    cw_dassert(a_mkr->magic == CW_MKR_MAGIC);
    cw_check_ptr(a_mkr->bufp);

    bufp_p_cache_validate(a_mkr->bufp);
    retval = a_mkr->bufp->bpos + bufp_p_pos_p2b(a_mkr->bufp, a_mkr->ppos);

    return retval;
}

cw_uint8_t *
mkr_before_get(const cw_mkr_t *a_mkr)
{
    cw_error("XXX Not implemented");
}

cw_uint8_t *
mkr_after_get(const cw_mkr_t *a_mkr)
{
    cw_error("XXX Not implemented");
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
