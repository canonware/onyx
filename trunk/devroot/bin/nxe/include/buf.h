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

typedef struct cw_bufc_s cw_bufc_t;
typedef struct cw_bufm_s cw_bufm_t;
typedef struct cw_bufe_s cw_bufe_t;
typedef struct cw_bufhi_s cw_bufhi_t;
typedef struct cw_bufh_s cw_bufh_t;
typedef struct cw_bufb_s cw_bufb_t;
typedef struct cw_buf_s cw_buf_t;

/* This structure is not opaque; its internals can be mucked with. */
struct cw_bufc_s {
	cw_uint8_t	c:8;

	cw_uint32_t	fg:8;		/* Foreground color. */
	cw_uint32_t	bg:8;		/* Background color. */
	cw_bool_t	bold:1;		/* Bold if TRUE. */
	cw_bool_t	emph:1;		/* Emphasized (italic) if TRUE. */
	cw_bool_t	ul:1;		/* Underlined if TRUE. */
};

struct cw_bufm_s {
#ifdef _CW_DBG
	cw_uint32_t	magic;
#define _CW_BUFM_MAGIC	0x2e84a3c9
#endif

	ql_elm(cw_bufm_t) link;
	cw_buf_t	*buf;

	cw_opaque_dealloc_t *dealloc;
	const void	*arg;

	cw_uint64_t	cpos;

	/* Message queue to send notifications to. */
	cw_msgq_t	*msgq;
};

struct cw_bufhi_s {
	qs_elm(cw_bufhi_t) link;

	enum {
		BUFH_GROUP_START,
		BUFH_GROUP_END,
		BUFH_SEEK,
		BUFH_INSERT,
		BUFH_REMOVE
	}		mod;
	union {
		struct {
			cw_uint64_t	from;
			cw_uint64_t	to;
		}	seek;
		struct {
			cw_bufc_t	c;
		}	insert;
	}		data;
};

struct cw_bufh_s {
	qs_head(cw_bufhi_t) undo;
	qs_head(cw_bufhi_t) redo;
};

/*
 * Make each bufb structure exactly one page, which avoids memory fragmentation
 * with most malloc implementations.
 */
#ifndef PAGESIZE
#define	PAGESIZE		4096
#endif
#define	_CW_BUFB_SIZE		PAGESIZE
#define	_CW_BUFB_OVERHEAD	28
#define	_CW_BUFB_DATA		(_CW_BUFB_SIZE - _CW_BUFB_OVERHEAD)
#define	_CW_BUFB_NCHAR		(_CW_BUFB_DATA / sizeof(cw_bufc_t))
struct cw_bufb_s {
	/*
	 * The epos and eline fields aren't maintained by the bufb code at all;
	 * for them to be valid, they must be explicitly set by external logic.
	 */
	cw_uint64_t	ecpos;			/* Last character position. */
	cw_uint64_t	eline;			/* Last line number. */
	cw_uint32_t	nlines;			/* Number of newlines. */
	cw_uint32_t	gap_off;		/* Gap offset, in bufc's. */
	cw_uint32_t	gap_len;		/* Gap length, in bufc's. */
	cw_bufc_t	data[_CW_BUFB_NCHAR];	/* Text data, with gap. */
};

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
	const void	*arg;

	/* Implicit lock. */
	cw_mtx_t	mtx;

	/* Actual data. */
	cw_uint64_t	len;		/* Number of characters. */
	cw_uint64_t	bufb_count;	/* Number of valid bufb's. */
	cw_uint64_t	bufb_veclen;	/* Number of elements in bufb_vec. */
	cw_bufb_t	**bufb_vec;	/* Vector of bufb pointers. */
	cw_uint64_t	ncached;	/* # of bufb's w/ valid ecpos/eline. */

	/* Ordered list of all marks. */
	ql_head(cw_bufm_t) bufms;

	/* History. */
	cw_bool_t	hist_active:1;
	cw_bufh_t	hist;
};

/* buf. */
cw_buf_t *buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
    cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc, void
    *a_arg);
void	buf_delete(cw_buf_t *a_buf);

cw_uint64_t buf_count(cw_buf_t *a_buf);

cw_bool_t buf_hist_active_get(cw_buf_t *a_buf);
void	buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active);
cw_bool_t buf_undo(cw_buf_t *a_buf);
cw_bool_t buf_redo(cw_buf_t *a_buf);
void	buf_hist_group_beg(cw_buf_t *a_buf);
void	buf_hist_group_end(cw_buf_t *a_buf);
void	buf_hist_flush(cw_buf_t *a_buf);

/* bufm. */
cw_bufm_t *bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf, cw_msgq_t *a_msgq);
void	bufm_dup(cw_bufm_t *a_to, cw_bufm_t *a_from);
void	bufm_delete(cw_bufm_t *a_bufm);
cw_buf_t *bufm_buf(cw_bufm_t *a_bufm);
cw_uint64_t bufm_line(cw_bufm_t *a_bufm);

cw_uint64_t bufm_rel_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount);
cw_uint64_t bufm_abs_seek(cw_bufm_t *a_bufm, cw_uint64_t a_pos);
cw_uint64_t bufm_pos(cw_bufm_t *a_bufm);

cw_bufc_t bufm_bufc_get(cw_bufm_t *a_bufm);
void	bufm_bufc_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc);
/* XXX Provide accessors for individual bufc fields. */
void	bufm_bufc_insert(cw_bufm_t *a_bufm, const cw_bufc_t *a_data, cw_uint64_t
    a_count);
void	bufm_uint8_insert(cw_bufm_t *a_bufm, const cw_uint8_t *a_data,
    cw_uint64_t a_count);
void	bufm_remove(cw_bufm_t *a_start, cw_bufm_t *a_end);
