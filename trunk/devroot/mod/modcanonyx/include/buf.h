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

typedef struct cw_bufqm_s cw_bufqm_t;
typedef struct cw_bufq_s cw_bufq_t;
typedef struct cw_bufm_s cw_bufm_t;
typedef struct cw_bufe_s cw_bufe_t;
typedef struct cw_bufh_s cw_bufh_t;
typedef struct cw_bufb_s cw_bufb_t;
typedef cw_uint8_t cw_bufc_t;
typedef struct cw_buf_s cw_buf_t;

typedef enum {
	BUFQMT_NONE,

	BUFQMT_BUFE_CUT,
	BUFQMT_BUFE_PASTE,
	BUFQMT_BUFE_MODIFY,
	BUFQMT_BUFE_DETACH,

	BUFQMT_BUFM_MOVE
} cw_bufqmt_t;

struct cw_bufqm_s {
	cw_opaque_dealloc_t	*dealloc;
	const void		*dealloc_arg;

	cw_bufqmt_t		event;
	union {
		cw_bufe_t	*bufe;
		cw_bufm_t	*bufm;
	}			data;
};

struct cw_bufq_s {
	cw_mq_t		mq;
};

struct cw_bufm_s {
	ql_elm(cw_bufm_t) link;
	cw_buf_t	*buf;

	cw_opaque_dealloc_t *dealloc;
	const void	*arg;

	cw_uint64_t	offset;

	/* Message queue to send notifications to. */
	cw_bufq_t	*bufq;
};

struct cw_bufe_s {
	ql_elm(cw_bufe_t) link;
	cw_buf_t	*buf;

	cw_opaque_dealloc_t *dealloc;
	const void	*arg;

	cw_bufm_t	beg;
	cw_bufm_t	end;

	/* bufes are either open or closed at each end. */
	cw_bool_t	beg_open:1;
	cw_bool_t	end_open:1;

	/* Used for precedence in extent stacks. */
	cw_sint32_t	priority;

	/* If TRUE, save in undo/redo history. */
	cw_bool_t	duplicable:1;

	/*
	 * A detachable extent is removed from the buffer if its size reaches 0.
	 */
	cw_bool_t	detachable:1;
	cw_bool_t	detached:1;

	cw_uint32_t	foreground;
	cw_uint32_t	background;
	cw_bool_t	bold:1;
	cw_bool_t	italic:1;
	cw_bool_t	underline:1;

	/* Message queue to send notifications to. */
	cw_bufq_t	*bufq;
};

struct cw_bufh_s {
	qs_elm(cw_bufh_t) link;
	enum {
		BUFH_BOUNDARY,
		BUFH_SEEK,
		BUFH_INSERT,
		BUFH_REMOVE,
		BUFH_EXTENT
	}		mod;
	union {
		struct {
			cw_uint64_t	offset;
		}	seek;
		struct {
			cw_bufc_t	c;
		}	insert;
		struct {
			/*
			 * The internal bufe state isn't complete, so must be
			 * hanadled with care when restored.
			 *
			 * We use a pointer instead of embedding the bufe in
			 * order to keep bufh's small.
			 */
			cw_bufe_t	*bufe;
		}	extent;
	}		data;
};

struct cw_bufbh_s {
};

/*
 * Make each bufb structure exactly 4 K, which avoids memory fragmentation with
 * most malloc implementations.
 */
#define	_CW_BUFB_SIZE		4096
#ifdef _CW_DBG
#define	_CW_BUFB_OVERHEAD	  28
#else
#define	_CW_BUFB_OVERHEAD	  24
#endif
#define	_CW_BUFB_DATA	(_CW_BUFB_SIZE - _CW_BUFB_OVERHEAD)
struct cw_bufb_s {
#ifdef _CW_DBG
	cw_uint32_t	magic;
#define _CW_BUFB_MAGIC	0x28da3e88
#endif
	cw_uint64_t	offset;
	cw_uint64_t	line;
	cw_uint32_t	gap_off;
	cw_uint32_t	gap_len;
	cw_bufc_t	data[_CW_BUFB_DATA];
};

struct cw_buf_s {
#ifdef _CW_DBG
	cw_uint32_t	magic;
#define _CW_BUF_MAGIC	0x348279bd
#endif

	cw_bool_t	alloced:1;
	cw_opaque_alloc_t *alloc;
	cw_opaque_realloc_t *realloc;
	cw_opaque_dealloc_t *dealloc;
	const void	*arg;

	/* Actual data. */
	cw_uint64_t	len;
	cw_uint64_t	bufb_count;
	cw_uint64_t	bufb_veclen;
	cw_bufb_t	**bufb_vec;
	/*
	 * Offset of last bufb that has a valid cached offset and line.  The
	 * first bufb's cache is always valid.
	 */
	cw_uint64_t	last_cached;

	/* History. */
	cw_bool_t	hist_active:1;
	cw_bufh_t	hist;

	/* Ordered list of all marks. */
	ql_head(cw_bufm_t) bufms;

	/* Forward/reverse ordered lists of extents. */
	ql_head(cw_bufe_t) bufes_fwd;
	ql_head(cw_bufe_t) bufes_rev;

	/* Detached but not yet deleted. */
	ql_head(cw_bufe_t) bufes_det;
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

cw_bufe_t *buf_bufe_next(cw_buf_t *a_buf, cw_bufe_t *a_bufe);
cw_bufe_t *buf_bufe_prev(cw_buf_t *a_buf, cw_bufe_t *a_bufe);

/* bufm. */
void	bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf, cw_bufq_t *a_bufq);
void	bufm_dup(cw_bufm_t *a_bufm, const cw_bufm_t *a_orig, cw_bufq_t *a_bufq);
void	bufm_delete(cw_bufm_t *a_bufm);
cw_buf_t *bufm_buf(cw_bufm_t *a_bufm);
cw_uint64_t bufm_line(const cw_bufm_t *a_bufm);

cw_uint64_t bufm_rel_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount);
cw_uint64_t bufm_abs_seek(cw_bufm_t *a_bufm, cw_uint64_t a_amount);
cw_uint64_t bufm_pos(cw_bufm_t *a_bufm);

cw_bufc_t bufm_bufc_get(cw_bufm_t *a_bufm);
void	bufm_bufc_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc);
void	bufm_bufc_insert(cw_bufm_t *a_bufm, const cw_bufc_t *a_data, cw_uint64_t
    *a_count);

cw_bufe_t *bufm_bufe_down(cw_bufm_t *a_bufm, cw_bufe_t *a_bufe);
cw_bufe_t *bufm_bufe_up(cw_bufm_t *a_bufm, cw_bufe_t *a_bufe);

/* bufe. */
void	bufe_new(cw_bufe_t *a_bufe, cw_bufm_t *a_beg, cw_bufm_t *a_end,
    cw_bool_t a_beg_open, cw_bool_t a_end_open, cw_bool_t a_detachable,
    cw_bufq_t *a_bufq);
void	bufe_dup(cw_bufe_t *a_bufe, cw_bufe_t *a_orig, cw_bufq_t *a_bufq);
void	bufe_delete(cw_bufe_t *a_bufe);
cw_buf_t *bufe_buf(cw_bufm_t *a_bufm);

const cw_bufm_t *bufe_beg(cw_bufe_t *a_bufe);
const cw_bufm_t *bufe_end(cw_bufe_t *a_bufe);

cw_bufe_t *bufe_bufe_next(cw_bufe_t *a_bufe, cw_bufe_t *a_curr);
cw_bufe_t *bufe_bufe_prev(cw_bufe_t *a_bufe, cw_bufe_t *a_curr);

void	bufe_cut(cw_bufe_t *a_bufe);

cw_uint32_t bufe_foreground_get(cw_bufe_t *a_bufe);
void	bufe_foreground_set(cw_bufe_t *a_bufe, cw_uint32_t a_foreground);
cw_uint32_t bufe_background_get(cw_bufe_t *a_bufe);
void	bufe_background_set(cw_bufe_t *a_bufe, cw_uint32_t a_background);
cw_bool_t bufe_bold_get(cw_bufe_t *a_bufe);
void	bufe_bold_set(cw_bufe_t *a_bufe, cw_bool_t a_bold);
cw_bool_t bufe_italic_get(cw_bufe_t *a_bufe);
void	bufe_italic_set(cw_bufe_t *a_bufe, cw_bool_t a_italic);
cw_bool_t bufe_underline_get(cw_bufe_t *a_bufe);
void	bufe_underline_set(cw_bufe_t *a_bufe, cw_bool_t a_underline);

/* bufq. */
void	bufq_new(cw_bufq_t *a_bufq);
void	bufq_delete(cw_bufq_t *a_bufq);

cw_bufqm_t *bufq_get(cw_bufq_t *a_bufq);
cw_bufqm_t *bufq_tryget(cw_bufq_t *a_bufq);
cw_bufqm_t *bufq_timedget(cw_bufq_t *a_bufq, const struct timespec *a_timeout);
void	bufq_put(cw_bufq_t *a_bufq, cw_bufqm_t *a_bufqm);

/* bufqm. */
void	bufqm_bufe_new(cw_bufqm_t *a_bufqm, cw_opaque_dealloc_t *a_dealloc,
    const void *a_dealloc_arg, cw_bufe_t *a_bufe, cw_bufqmt_t a_event);
void	bufqm_bufm_new(cw_bufqm_t *a_bufqm, cw_opaque_dealloc_t *a_dealloc,
    const void *a_dealloc_arg, cw_bufm_t *a_bufm, cw_bufqmt_t a_event);
void	bufqm_delete(cw_bufqm_t *a_bufqm);

cw_bufqmt_t bufqm_event(cw_bufqm_t *a_bufqm);
cw_bufe_t *bufqm_bufe(cw_bufqm_t *a_bufqm);
cw_bufm_t *bufqm_bufm(cw_bufqm_t *a_bufqm);
