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

typedef struct cw_bufm_s cw_bufm_t;
typedef struct cw_buf_s cw_buf_t;

/* Enumeration for seek operations. */
typedef enum {
	BUFW_NONE,	/* Invalid. */
	BUFW_BEG,	/* Offset from BOB, must be positive. */
	BUFW_REL,	/* Relative to marker, positive or negative. */
	BUFW_END	/* Offset from EOB, must be negative. */
} cw_bufw_t;

/* Buffer history record types and history state machine states. */
typedef enum {
	BUFH_NONE,	/* Start state. */
	BUFH_B,		/* Begin group. */
	BUFH_E,		/* End group. */
	BUFH_P,		/* Change buffer position. */
	BUFH_I,		/* Insert before. */
	BUFH_Y,		/* Insert after (yank). */
	BUFH_R,		/* Remove before. */
	BUFH_K		/* Remove after (kill). */
} cw_bufh_t;

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

	cw_msgq_t	*msgq;		/* Notify of manual marker movement. */
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

	cw_msgq_t	*msgq;		/* Notify of buffer changes. */

	cw_mtx_t	mtx;		/* Explicit lock. */

	/* Internal buffer state. */
	cw_uint32_t	elmsize;	/* Number of bytes per element, >= 1. */
	cw_uint8_t	*b;		/* Text buffer, with gap. */
	cw_uint64_t	len;		/* Length (also last valid cpos). */
	cw_uint64_t	nlines; 	/* Number of lines (>= 1). */
	cw_uint64_t	gap_off;	/* Gap offset, in elements. */
	cw_uint64_t	gap_len;	/* Gap length, in elements. */

	ql_head(cw_bufm_t) bufms;	/* Ordered list of all markers. */

	/* History (undo/redo) state. */
	cw_buf_t	*h;		/* History buffer, if non-NULL. */
	cw_bufm_t	hend;		/* Marker at end of h. */
	cw_bufm_t	hcur;		/* Marker at current position in h. */
	cw_bufh_t	hstate;		/* Current history state. */
	cw_uint64_t	hbpos;		/* Current history bpos. */
	cw_uint32_t	ucount;		/* # of undo chars in current record. */
	cw_uint32_t	rcount;		/* # of redo chars in current record. */
};

/* buf. */
cw_buf_t *buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
    cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc, void
    *a_arg);
void	buf_delete(cw_buf_t *a_buf);

void	buf_lock(cw_buf_t *a_buf);
void	buf_unlock(cw_buf_t *a_buf);

cw_uint32_t buf_elmsize_get(cw_buf_t *a_buf);
void	buf_elmsize_set(cw_buf_t *a_buf, cw_uint32_t a_elmsize);

cw_msgq_t *buf_msgq_get(cw_buf_t *a_buf);
void	buf_msgq_set(cw_buf_t *a_buf, cw_msgq_t *a_msgq);

cw_uint64_t buf_len(cw_buf_t *a_buf);
cw_uint64_t buf_nlines(cw_buf_t *a_buf);

cw_bool_t buf_hist_active_get(cw_buf_t *a_buf);
void	buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active);
cw_bool_t buf_undoable(cw_buf_t *a_buf);
cw_bool_t buf_redoable(cw_buf_t *a_buf);
cw_bool_t buf_undo(cw_buf_t *a_buf, cw_bufm_t *a_bufm);
cw_bool_t buf_redo(cw_buf_t *a_buf, cw_bufm_t *a_bufm);
void	buf_hist_group_beg(cw_buf_t *a_buf);
void	buf_hist_group_end(cw_buf_t *a_buf);
void	buf_hist_flush(cw_buf_t *a_buf);

/* bufm. */
cw_bufm_t *bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf, cw_msgq_t *a_msgq);
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
cw_uint8_t *bufm_range_get(cw_bufm_t *a_start, cw_bufm_t *a_end);

void	bufm_before_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_str,
    cw_uint64_t a_len);
void	bufm_after_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_str,
    cw_uint64_t a_len);

void	bufm_remove(cw_bufm_t *a_start, cw_bufm_t *a_end);
