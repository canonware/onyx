/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

typedef struct cw_hist_s cw_hist_t;
typedef struct cw_bufm_s cw_bufm_t;
typedef struct cw_buf_s cw_buf_t;

/* Similar to struct iovec, but with 64 bit lengths. */
typedef struct {
	cw_uint8_t	*data;
	cw_uint64_t	len;
} cw_bufv_t;

/* Enumeration for seek operations. */
typedef enum {
	BUFW_NONE,	/* Invalid. */
	BUFW_BEG,	/* Offset from BOB, must be positive. */
	BUFW_REL,	/* Relative to marker, positive or negative. */
	BUFW_END	/* Offset from EOB, must be negative. */
} cw_bufw_t;

struct cw_bufm_s {
#ifdef _CW_DBG
	cw_uint32_t	magic;
#define _CW_BUFM_MAGIC	0x2e84a3c9
#endif

	ql_elm(cw_bufm_t) link;		/* Ordered bufm list linkage. */
	cw_buf_t	*buf;		/* NULL if buf has been deleted. */

	/* Allocator state. */
	cw_opaque_dealloc_t *dealloc;
	const void	*arg;

	cw_uint64_t	apos;		/* Gap movement can change this. */
	cw_uint64_t	line;		/* Always kept up to date. */
};

#define	_CW_BUF_MINELMS		4096
struct cw_buf_s {
#ifdef _CW_DBG
	cw_uint32_t	magic;
#define _CW_BUF_MAGIC	0x348279bd
#endif

	/* Allocator state. */
	cw_bool_t	alloced:1;
	cw_opaque_alloc_t *alloc;
	cw_opaque_realloc_t *realloc;
	cw_opaque_dealloc_t *dealloc;
	void		*arg;

	cw_mtx_t	mtx;		/* Explicit lock. */

	/* Internal buffer state. */
	cw_uint32_t	elmsize;	/* Number of bytes per element, >= 1. */
	cw_uint8_t	*b;		/* Text buffer, with gap. */
	cw_uint64_t	len;		/* Length. */
	cw_uint64_t	nlines; 	/* Number of lines (>= 1). */
	cw_uint64_t	gap_off;	/* Gap offset, in elements. */
	cw_uint64_t	gap_len;	/* Gap length, in elements. */

	cw_bufv_t	bufv[2];	/* Returned by bufm_range_get(). */

	ql_head(cw_bufm_t) bufms;	/* Ordered list of all markers. */

	cw_hist_t	*hist;		/* History (undo/redo), if non-NULL. */
};

/* bufv. */
cw_uint64_t bufv_copy(cw_bufv_t *a_to, cw_uint32_t a_to_len, cw_uint32_t
    a_to_sizeof, const cw_bufv_t *a_fr, cw_uint32_t a_fr_len, cw_uint32_t
    a_fr_sizeof, cw_uint64_t a_maxlen);

/* buf. */
cw_buf_t *buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
    cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc, void
    *a_arg);
void	buf_delete(cw_buf_t *a_buf);

void	buf_lock(cw_buf_t *a_buf);
void	buf_unlock(cw_buf_t *a_buf);

cw_uint32_t buf_elmsize_get(cw_buf_t *a_buf);
void	buf_elmsize_set(cw_buf_t *a_buf, cw_uint32_t a_elmsize);

cw_uint64_t buf_len(cw_buf_t *a_buf);
cw_uint64_t buf_nlines(cw_buf_t *a_buf);

cw_bool_t buf_hist_active_get(cw_buf_t *a_buf);
void	buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active);
cw_bool_t buf_undoable(cw_buf_t *a_buf);
cw_bool_t buf_redoable(cw_buf_t *a_buf);
cw_uint64_t buf_undo(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t a_count);
cw_uint64_t buf_redo(cw_buf_t *a_buf, cw_bufm_t *a_bufm, cw_uint64_t a_count);
void	buf_hist_flush(cw_buf_t *a_buf);
void	buf_hist_group_beg(cw_buf_t *a_buf, cw_bufm_t *a_bufm);
void	buf_hist_group_end(cw_buf_t *a_buf);

/* bufm. */
cw_bufm_t *bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf);
void	bufm_dup(cw_bufm_t *a_to, cw_bufm_t *a_from);
void	bufm_delete(cw_bufm_t *a_bufm);
cw_buf_t *bufm_buf(cw_bufm_t *a_bufm);

cw_uint64_t bufm_line_seek(cw_bufm_t *a_bufm, cw_sint64_t a_offset, cw_bufw_t
    a_whence);
cw_uint64_t bufm_line(cw_bufm_t *a_bufm);

cw_uint64_t bufm_seek(cw_bufm_t *a_bufm, cw_sint64_t a_offset, cw_bufw_t
    a_whence);
cw_uint64_t bufm_pos(cw_bufm_t *a_bufm);

cw_uint8_t *bufm_before_get(cw_bufm_t *a_bufm);
cw_uint8_t *bufm_after_get(cw_bufm_t *a_bufm);
cw_bufv_t *bufm_range_get(cw_bufm_t *a_start, cw_bufm_t *a_end, cw_uint32_t
    *r_iovcnt);

void	bufm_before_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_str,
    cw_uint64_t a_len);
void	bufm_after_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_str,
    cw_uint64_t a_len);

void	bufm_remove(cw_bufm_t *a_start, cw_bufm_t *a_end);
