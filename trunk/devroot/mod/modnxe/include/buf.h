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

typedef cw_uint8_t cw_char_t;
typedef cw_uint32_t cw_bufc_t;
typedef struct cw_bufm_s cw_bufm_t;
typedef struct cw_buf_s cw_buf_t;

/*
 * The bits of a bufc are defined as follows:
 *
 * . : Unused.
 *
 * U : Underlined.
 *
 * I : Invisible.
 *
 * D : Dim.
 *
 * B : Bold.
 *
 * C : Colored (foreground and background are ignored if this is set).
 *
 * f : Foreground color.
 *
 * b : Background color.
 *
 * c : Character.
 *
 * ...UIDBC ffffffff bbbbbbbb cccccccc
 *
 */
#define	bufc_clear(a_bufc)		(a_bufc) = 0
#define	bufc_attrs_clear(a_bufc)	(a_bufc) &= 0xff
#define	bufc_attrs_copy(a_to, a_from)					\
	(a_to) = ((a_to) & 0xff) | ((a_from) & 0xffffff00)

#define	bufc_char_get(a_bufc)		((cw_char_t)((a_bufc) & 0xff))
#define	bufc_char_set(a_bufc, a_char)					\
	(a_bufc) = (((a_bufc) & 0xffffff00) | (a_char))

#define	bufc_fg_get(a_bufc)		(((a_bufc) >> 16) & 0xff)
#define	bufc_fg_set(a_bufc, a_fg) do {					\
	_cw_assert(((a_fg) & 0xffffff) == 0);				\
	(a_bufc) = ((a_bufc) & 0xff00ffff) | ((a_fg) << 16);		\
} while (0)

#define	bufc_bg_get(a_bufc)		(((a_bufc) >> 8) & 0xff)
#define	bufc_bg_set(a_bufc, a_bg) do {					\
	_cw_assert(((a_bg) & 0xffffff) == 0);				\
	(a_bufc) = ((a_bufc) & 0xffff00ff) | ((a_bg) << 8);		\
} while (0)

#define	bufc_colored_get(a_bufc)	((a_bufc) >> 24) & 1
#define	bufc_colored_set(a_bufc, a_colored) do {			\
	_cw_assert(((a_colored) & 0xfffffffe) = 0);			\
	(a_bufc) = ((a_bufc) & 0xfeffffff) | ((a_colored) << 24);	\
} while (0)

#define	bufc_bold_get(a_bufc)	((a_bufc) >> 25) & 1
#define	bufc_bold_set(a_bufc, a_bold) do {				\
	_cw_assert(((a_bold) & 0xfffffffe) = 0);			\
	(a_bufc) = ((a_bufc) & 0xfdffffff) | ((a_bold) << 25);		\
} while (0)

#define	bufc_dim_get(a_bufc)	((a_bufc) >> 26) & 1
#define	bufc_dim_set(a_bufc, a_dim) do {				\
	_cw_assert(((a_dim) & 0xfffffffe) = 0);				\
	(a_bufc) = ((a_bufc) & 0xfbffffff) | ((a_dim) << 26);		\
} while (0)

#define	bufc_invisible_get(a_bufc)	((a_bufc) >> 27) & 1
#define	bufc_invisible_set(a_bufc, a_invisible) do {			\
	_cw_assert(((a_invisible) & 0xfffffffe) = 0);			\
	(a_bufc) = ((a_bufc) & 0xf7ffffff) | ((a_invisible) << 27);	\
} while (0)

#define	bufc_underlined_get(a_bufc)	((a_bufc) >> 28) & 1
#define	bufc_underlined_set(a_bufc, a_underlined) do {			\
	_cw_assert(((a_underlined) & 0xfffffffe) = 0);			\
	(a_bufc) = ((a_bufc) & 0xefffffff) | ((a_underlined) << 28);	\
} while (0)

/* Enumeration for seek operations. */
typedef enum {
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

	cw_msgq_t	*msgq;		/* Notify of manual marker movement. */
};

#ifndef PAGESIZE
#define	PAGESIZE		4096
#endif
#define	_CW_BUF_MINBUFCS	(PAGESIZE / sizeof(cw_bufc_t))
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

	cw_msgq_t	*msgq;		/* Notify of buffer changes. */

	cw_mtx_t	mtx;		/* Implicit lock. */

	cw_bufc_t	*b;		/* Text buffer, with gap. */
	cw_uint64_t	len;		/* Length (also last valid cpos). */
	cw_uint64_t	nlines; 	/* Number of lines (>= 1). */
	cw_uint64_t	gap_off;	/* Gap offset, in bufc's. */
	cw_uint64_t	gap_len;	/* Gap length, in bufc's. */

	ql_head(cw_bufm_t) bufms;	/* Ordered list of all markers. */

	cw_uint8_t	*hist_buf;	/* History buffer, if non-NULL. */
	cw_uint64_t	hist_buflen;	/* Total size of hist_buf. */
	cw_uint64_t	hist_len;	/* Amount of hist_buf used. */
	
};

/* buf. */
cw_buf_t *buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
    cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc, void *a_arg,
    cw_msgq_t *a_msgq);
void	buf_delete(cw_buf_t *a_buf);

void	buf_lock(cw_buf_t *a_buf);
void	buf_unlock(cw_buf_t *a_buf);

cw_uint64_t buf_len(cw_buf_t *a_buf);
cw_uint64_t buf_nlines(cw_buf_t *a_buf);

cw_bool_t buf_hist_active_get(cw_buf_t *a_buf);
void	buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active);
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

cw_bool_t bufm_before_get(cw_bufm_t *a_bufm, cw_bufc_t *r_bufc);
cw_bool_t bufm_after_get(cw_bufm_t *a_bufm, cw_bufc_t *r_bufc);

cw_bool_t bufm_before_set(cw_bufm_t *a_bufm, cw_char_t a_char);
cw_bool_t bufm_after_set(cw_bufm_t *a_bufm, cw_char_t a_char);

cw_bool_t bufm_before_attrs_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc);
cw_bool_t bufm_after_attrs_set(cw_bufm_t *a_bufm, cw_bufc_t a_bufc);

void	bufm_before_insert(cw_bufm_t *a_bufm, const cw_char_t *a_str,
    cw_uint64_t a_count);
void	bufm_after_insert(cw_bufm_t *a_bufm, const cw_char_t *a_str,
    cw_uint64_t a_count);

void	bufm_remove(cw_bufm_t *a_start, cw_bufm_t *a_end);

/* Operators. */
void	nxe_buf(cw_nxo_t *a_thread);
void	nxe_buf_len(cw_nxo_t *a_thread);
void	nxe_buf_nlines(cw_nxo_t *a_thread);
void	nxe_marker(cw_nxo_t *a_thread);
