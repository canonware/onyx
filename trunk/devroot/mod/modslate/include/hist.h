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
 ******************************************************************************/

struct cw_hist_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_HIST_MAGIC 0x353dd57a
#endif
    /* History buffer. */
    cw_buf_t h;

    /* Marker at current position in h. */
    cw_bufm_t hcur;

    /* Temporary marker. */
    cw_bufm_t htmp;

    /* Current history bpos (in the data buf). */
    cw_uint64_t hbpos;

    /* Current group depth. */
    cw_uint32_t gdepth;

    /* Allocator state. */
    cw_opaque_dealloc_t *dealloc;
    const void *arg;
};

cw_hist_t *
hist_new(cw_opaque_alloc_t *a_alloc, cw_opaque_realloc_t *a_realloc,
	 cw_opaque_dealloc_t *a_dealloc, void *a_arg);

void
hist_delete(cw_hist_t *a_hist);

cw_bool_t
hist_undoable(cw_hist_t *a_hist, cw_buf_t *a_buf);

cw_bool_t
hist_redoable(cw_hist_t *a_hist, cw_buf_t *a_buf);

cw_uint64_t
hist_undo(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_bufm_t *a_bufm,
	  cw_uint64_t a_count);

cw_uint64_t
hist_redo(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_bufm_t *a_bufm,
	  cw_uint64_t a_count);

void
hist_flush(cw_hist_t *a_hist, cw_buf_t *a_buf);

void
hist_group_beg(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_bufm_t *a_bufm);

cw_bool_t
hist_group_end(cw_hist_t *a_hist, cw_buf_t *a_buf);

void
hist_ins(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos, const
	 cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt);

void
hist_ynk(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos, const
	 cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt);

void
hist_rem(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos, const
	 cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt);

void
hist_del(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_uint64_t a_bpos, const
	 cw_bufv_t *a_bufv, cw_uint32_t a_bufvcnt);

void
hist_dump(cw_hist_t *a_hist, cw_buf_t *a_buf);
