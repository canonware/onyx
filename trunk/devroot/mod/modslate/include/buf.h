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

typedef struct cw_hist_s cw_hist_t;
typedef struct cw_bufe_s cw_bufe_t;
typedef struct cw_bufm_s cw_bufm_t;
typedef struct cw_buf_s cw_buf_t;

/* Similar to struct iovec, but with 64 bit lengths. */
typedef struct
{
    cw_uint8_t *data;
    cw_uint64_t len;
} cw_bufv_t;

/* Enumeration for seek operations. */
typedef enum
{
    /* Invalid. */
    BUFW_NONE,

    /* Offset from BOB, must be positive. */
    BUFW_BEG,

    /* Relative to marker, positive or negative. */
    BUFW_REL,

    /* Offset from EOB, must be negative. */
    BUFW_END
} cw_bufw_t;

struct cw_bufe_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_BUFE_MAGIC 0x8a94e34c
#endif

    /* Ordered bufe list linkage.  flink: forward, rlink: reverse. */
    ql_elm(cw_bufe_t) flink;
    ql_elm(cw_bufe_t) rlink;

    /* Buffer this extent is in. */
    cw_buf_t *buf;

    /* Allocator state. */
    cw_opaque_dealloc_t *dealloc;
    const void *arg;

    /* Gap movement can change this. */
    cw_uint64_t beg_apos;
    cw_uint64_t end_apos;

    /* Always kept up to date. */
    cw_uint64_t beg_line;
    cw_uint64_t end_line;

    /* Extents are either open or closed at each end. */
    cw_bool_t beg_open:1;
    cw_bool_t end_open:1;

    /* A detachable extent is removed from the buffer if its size reaches 0. */
    cw_bool_t detachable:1;
    cw_bool_t detached:1;
};

struct cw_bufm_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_BUFM_MAGIC 0x2e84a3c9
#endif

    /* Ordered bufm list linkage. */
    ql_elm(cw_bufm_t) link;

    /* Buffer this marker is in. */
    cw_buf_t *buf;

    /* Allocator state. */
    cw_opaque_dealloc_t *dealloc;
    const void *arg;

    /* Gap movement can change this. */
    cw_uint64_t apos;

    /* Always kept up to date. */
    cw_uint64_t line;
};

#define CW_BUF_MINELMS 4096
struct cw_buf_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_BUF_MAGIC 0x348279bd
#endif

    /* Allocator state. */
    cw_bool_t alloced:1;
    cw_opaque_alloc_t *alloc;
    cw_opaque_realloc_t *realloc;
    cw_opaque_dealloc_t *dealloc;
    void *arg;

    /* Internal buffer state. */

    /* Text buffer, with gap. */
    cw_uint8_t *b;

    /* Length. */
    cw_uint64_t len;

    /* Number of lines (>= 1). */
    cw_uint64_t nlines;

    /* Gap offset, in elements. */
    cw_uint64_t gap_off;

    /* Gap length, in elements. */
    cw_uint64_t gap_len;

    /* Returned by bufm_range_get(). */
    cw_bufv_t bufv[2];

    /* Ordered list of all markers. */
    ql_head(cw_bufm_t) bufms;

    /* Ordered lists of all extents, in forward and reverse order. */
    ql_head(cw_bufe_t) fbufes;
    ql_head(cw_bufe_t) rbufes;

    /* History (undo/redo), if non-NULL. */
    cw_hist_t *hist;
};

/* bufv. */
cw_uint64_t
bufv_copy(cw_bufv_t *a_to, cw_uint32_t a_to_len, const cw_bufv_t *a_fr,
	  cw_uint32_t a_fr_len, cw_uint64_t a_maxlen);

/* buf. */
cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
	cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc,
	void *a_arg);

void
buf_delete(cw_buf_t *a_buf);

cw_uint64_t
buf_len(cw_buf_t *a_buf);

cw_uint64_t
buf_nlines(cw_buf_t *a_buf);

cw_bool_t
buf_hist_active_get(cw_buf_t *a_buf);

void
buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active);

cw_bool_t
buf_undoable(cw_buf_t *a_buf);

cw_bool_t
buf_redoable(cw_buf_t *a_buf);

cw_uint64_t
buf_undo(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t a_count);

cw_uint64_t
buf_redo(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t a_count);

void
buf_hist_flush(cw_buf_t *a_buf);

void
buf_hist_group_beg(cw_buf_t *a_buf, cw_bufm_t *a_bufm);

cw_bool_t
buf_hist_group_end(cw_buf_t *a_buf);

/* bufm. */
cw_bufm_t *
bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf);

void
bufm_dup(cw_bufm_t *a_to, cw_bufm_t *a_from);

void
bufm_delete(cw_bufm_t *a_bufm);

cw_buf_t *
bufm_buf(cw_bufm_t *a_bufm);

cw_uint64_t
bufm_line_seek(cw_bufm_t *a_bufm, cw_sint64_t a_offset, cw_bufw_t a_whence);

cw_uint64_t
bufm_line(cw_bufm_t *a_bufm);

cw_uint64_t
bufm_seek(cw_bufm_t *a_bufm, cw_sint64_t a_offset, cw_bufw_t a_whence);

cw_uint64_t
bufm_pos(cw_bufm_t *a_bufm);

cw_uint8_t *
bufm_before_get(cw_bufm_t *a_bufm);

cw_uint8_t *
bufm_after_get(cw_bufm_t *a_bufm);

cw_bufv_t *
bufm_range_get(cw_bufm_t *a_start, cw_bufm_t *a_end, cw_uint32_t *r_iovcnt);

void
bufm_before_insert(cw_bufm_t *a_bufm, const cw_bufv_t *a_bufv,
		   cw_uint32_t a_bufvcnt);

void
bufm_after_insert(cw_bufm_t *a_bufm, const cw_bufv_t *a_bufv,
		  cw_uint32_t a_bufvcnt);

void
bufm_remove(cw_bufm_t *a_start, cw_bufm_t *a_end);

/* bufe. */
cw_bufe_t *
bufe_new(cw_bufe_t *a_bufe, cw_buf_t *a_buf);

void
bufe_dup(cw_bufe_t *a_to, cw_bufe_t *a_from);

void
bufe_delete(cw_bufe_t *a_bufe);

cw_buf_t *
bufe_buf(cw_bufe_t *a_bufe);

cw_uint64_t
bufe_beg_get(cw_bufe_t *a_bufe);

void
bufe_beg_set(cw_bufe_t *a_bufe, cw_uint64_t a_beg);

cw_uint64_t
bufe_end_get(cw_bufe_t *a_bufe);

void
bufe_end_set(cw_bufe_t *a_bufe, cw_uint64_t a_end);

cw_bool_t
bufe_beg_open_get(cw_bufe_t *a_bufe);

void
bufe_beg_open_set(cw_bufe_t *a_bufe, cw_bool_t a_beg_open);

cw_bool_t
bufe_end_open_get(cw_bufe_t *a_bufe);

void
bufe_end_open_set(cw_bufe_t *a_bufe, cw_bool_t a_end_open);

cw_bool_t
bufe_detachable_get(cw_bufe_t *a_bufe);

void
bufe_detachable_set(cw_bufe_t *a_bufe, cw_bool_t a_detachable);

cw_bool_t
bufe_detached_get(cw_bufe_t *a_bufe);

void
bufe_detached_set(cw_bufe_t *a_bufe, cw_bool_t a_detached);

void
bufe_detach(cw_bufe_t *a_bufe);

cw_bufe_t *
bufe_before_get(cw_bufe_t *a_bufe, cw_bufm_t *a_bufm);

cw_bufe_t *
bufe_at_get(cw_bufe_t *a_bufe, cw_bufm_t *a_bufm);

cw_bufe_t *
bufe_after_get(cw_bufe_t *a_bufe, cw_bufm_t *a_bufm);

cw_bufe_t *
bufe_prev_get(cw_bufe_t *a_bufe);

cw_bufe_t *
bufe_next_get(cw_bufe_t *a_bufe);
