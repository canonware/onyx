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

#define	_CW_BUFB_SIZE	4096

typedef struct cw_buf_s cw_buf_t;
typedef struct cw_bufb_s cw_bufb_t;
typedef cw_uint8_t cw_bufc_t;
typedef struct cw_bufm_s cw_bufm_t;
typedef struct cw_bufe_s cw_bufe_t;
typedef struct cw_bufh_s cw_bufh_t;

struct cw_bufe_s {
	ql_elm(cw_bufe_t) link;

	cw_bufm_t	beg;
	cw_bufm_t	end;

	/* bufes are either open or closed at each end. */
	cw_bool_t	beg_open:1;
	cw_bool_t	end_open:1;

	/* Properties. */
	cw_uint32_t	foreground;
	cw_uint32_t	background;
	cw_bool_t	bold:1;
	cw_bool_t	italic:1;
	cw_bool_t	underline:1;

	cw_sint32_t	priority;	/* xemacs uses this for precedence. */
	cw_bool_t	duplicable;	/*
					 * If TRUE, save in undo history.  XXX
					 * Yikes, the history data structure
					 * needs modification to handle this!
					 */

	/* Message queue to send notifications to. */
	cw_mq_t		*mq;
};

struct cw_bufb_s {
	cw_uint32_t	gap_off;
	cw_uint32_t	gap_len;
	cw_bufc_t	data[_CW_BUFB_SIZE];
};

struct cw_bufh_s {
	qs_elm(cw_bufh_t) link;
	enum {
		BUFH_BOUNDARY,
		BUFH_SEEK,
		BUFH_INSERT,
		BUFH_REMOVE
	}		mod;
	union {
		struct {
			cw_uint64_t	offset;
		}	seek;
		struct {
			cw_bufc_t	c;
		}	insert;
	}		data;
};

struct cw_bufm_s {
	ql_elm(cw_bufm_t) link;

	cw_nxo_t	*buf;
	cw_uint64_t	offset;

	/* Message queue to send notifications to. */
	cw_mq_t		*mq;
};

struct cw_buf_s {
	cw_mtx_t	mtx;

	cw_bufe_t	narrowing;

	cw_bool_t	indirect;
	union {
		/* Indirect. */
		struct {
			ql_elm(cw_buf_t) link;
			cw_buf_t	*parent;
			cw_mq_t		mq;
		} i;
		/* Base buf. */
		struct {
			cw_uint64_t	nchars;
			cw_uint64_t	nbufbs;
			cw_bufb_t	*bufbs;

			qs_head(cw_bufh_t) undo;
			qs_head(cw_bufh_t) redo;

			ql_head(cw_buf_t) bufs;
			ql_head(cw_bufm_t) bufms;
			/* XXX xemacs keeps 2 lists; 1 forward, 1 reverse. */
			ql_head(cw_bufe_t) bufes;
		} b;
	} d;
};

/* buf. */
void	buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc, cw_opaque_dealloc_t
    *a_dealloc, void *a_arg);
void	buf_subbuf(cw_buf_t *a_buf, cw_bufe_t *a_bufe);
void	buf_delete(cw_buf_t *a_buf);
cw_buf_t *buf_buf(cw_buf_t *a_buf);

cw_uint64_t buf_count(cw_buf_t *a_buf);

cw_bool_t buf_undo_active_get(cw_buf_t *a_buf);
void	buf_undo_active_set(cw_buf_t *a_buf, cw_bool_t a_active);
cw_bool_t buf_undo(cw_buf_t *a_buf);
cw_bool_t buf_redo(cw_buf_t *a_buf);
void	buf_history_boundary(cw_buf_t *a_buf);
cw_bool_t buf_group_undo(cw_buf_t *a_buf);
cw_bool_t buf_group_redo(cw_buf_t *a_buf);
void	buf_history_flush(cw_buf_t *a_buf);

/* XXX Need extent iterators. */

/* bufm. */
void	bufm_new(cw_bufm_t *a_bufm, cw_buf_t *a_buf, cw_mq_t *a_mq);
void	bufm_dup(cw_bufm_t *a_bufm, cw_bufm_t *a_orig, cw_mq_t *a_mq);
void	bufm_delete(cw_bufm_t *a_bufm);
cw_buf_t *bufm_buf(cw_bufm_t *a_bufm);

cw_uint64_t bufm_rel_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount);
cw_uint64_t bufm_abs_seek(cw_bufm_t *a_bufm, cw_sint64_t a_amount);
cw_uint64_t bufm_pos(cw_bufm_t *a_bufm);

cw_bufc_t bufm_bufc_get(cw_bufm_t *a_bufm);
void	bufm_bufc_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc);
void	bufm_bufc_insert(cw_bufm_t *a_bufm, const cw_bufc_t *a_data, cw_uint64_t
    *a_count);

/*
 * XXX Need a way to get the extent stack for a location.  emacs uses iterators.
 * We can too, but we have to be able to handle races gracefully.
 */

/* bufe. */
void	bufe_new(cw_bufe_t *a_bufe, cw_bufm_t *a_beg, cw_bufm_t *a_end,
    cw_bool_t a_beg_open, cw_bool_t a_end_open, cw_mq_t *a_mq);
void	bufe_delete(cw_bufe_t *a_bufe);
cw_buf_t *bufe_buf(cw_bufm_t *a_bufm);

cw_bufm_t *bufe_beg(cw_bufe_t *a_bufe);
cw_bufm_t *bufe_end(cw_bufe_t *a_bufe);

void	bufe_remove(cw_bufe_t *a_bufe);

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

/* XXX Need a way to get the extent list for a region. */
