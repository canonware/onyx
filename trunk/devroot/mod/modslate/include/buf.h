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

/* Define CW_BUF_DUMP to enable buf_dump(). */
#ifdef CW_DBG
#define CW_BUF_DUMP
#endif
/* #define CW_BUFP_DUMP */

/* Define CW_BUF_VALIDATE to enable buf_validate(). */
#ifdef CW_DBG
#define CW_BUF_VALIDATE
#endif
/* #define CW_BUFP_VALIDATE */

typedef struct cw_hist_s cw_hist_t;
typedef struct cw_ext_s cw_ext_t;
typedef struct cw_mkr_s cw_mkr_t;
typedef struct cw_bufp_s cw_bufp_t;
typedef struct cw_buf_s cw_buf_t;

/* Similar to struct iovec. */
typedef struct
{
    cw_uint8_t *data;
    cw_uint32_t len;
} cw_bufv_t;

/* Enumeration for seek operations. */
typedef enum
{
    /* Invalid. */
    BUFW_NONE,

    /* Offset from BOB, must be positive. */
    BUFW_BOB,

    /* Relative to marker, positive or negative. */
    BUFW_REL,

    /* Offset from EOB, must be negative. */
    BUFW_EOB
} cw_bufw_t;

/* Enumeration for marker ordering.  This is used for mkr ordering (to support
 * extents), as well as for deciding whether insertions at a mkr's position go
 * before or after the mkr. */
typedef enum
{
    /* The mkr stays before data inserted. */
    MKRO_BEFORE,

    /* The mkr stays before data inserted via mkr_after_insert(), and goes after
     * data inserted via mkr_before_insert(). */
    MKRO_EITHER,

    /* The mkr goes after data inserted at its position. */
    MKRO_AFTER
} cw_mkro_t;

struct cw_mkr_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_MKR_MAGIC 0x2e84a3c9
#endif

    /* bufp this marker is in. */
    cw_bufp_t *bufp;

    /* Marker type.  Normal markers are MKRO_EITHER; other values are for
     * extents. */
    cw_mkro_t order:2;

    /* If this marker is part of an extent, then this field denotes whether it
     * is the beginning or ending marker.  This is needed so that a pointer to
     * the marker can be used to get a pointer to the containing extent. */
    cw_bool_t ext_end:1;

    /* Gap movement can change this. */
    cw_uint32_t ppos;

    /* Line number, relative to the beginning of bufp (>= 0). */
    cw_uint32_t pline;

    /* Ordered mkr tree and list linkage. */
    rb_node(cw_mkr_t) mnode;
    ql_elm(cw_mkr_t) mlink;
};

struct cw_ext_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_EXT_MAGIC 0x8a94e34c
#endif

    /* Allocator state. */
    cw_bool_t alloced:1;

    /* A detachable extent is removed from the buffer if its size reaches 0.
     * Note that an extent is created detached, but not detachable. */
    cw_bool_t attached:1;
    cw_bool_t detachable:1;

    /* Beginning and ending markers. */
    cw_mkr_t beg;
    cw_mkr_t end;

    /* Forward- and reverse-ordered extent tree and list linkage. */
    rb_node(cw_ext_t) fnode;
    ql_elm(cw_ext_t) flink;
    rb_node(cw_ext_t) rnode;
    ql_elm(cw_ext_t) rlink;

    /* Extent stack linkage. */
    ql_elm(cw_ext_t) elink;
};

struct cw_bufp_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_BUFP_MAGIC 0xda7d87d3
#endif

    /* Parent buf. */
    cw_buf_t *buf;

    /* Cached position of the begin of the bufp, relative to the begin/end
     * of the entire buf.  If bob_relative is TRUE, bpos and line are relative
     * to BOB; otherwise they're relative to EOB. */
    cw_bool_t bob_relative;
    cw_uint64_t bpos;
    cw_uint64_t line;

    /* Length. */
    cw_uint32_t len;

    /* Number of newlines. */
    cw_uint32_t nlines;

    /* Gap offset, in elements. */
    cw_uint32_t gap_off;

    /* Text buffer, with gap. */
#ifdef XXX_NOT_YET
#define CW_BUFP_SIZE 65536
#else
#define CW_BUFP_SIZE 5
#endif
    cw_uint8_t *b;

    /* Tree and list of mkr's that point into the bufp.  Both of these are
     * ordered and kept up to date.  Random access is done via the tree, and
     * iteration is done via the list. */
    rb_tree(cw_mkr_t) mtree;
    ql_head(cw_mkr_t) mlist;

    /* bufp tree and list linkage. */
    rb_node(cw_bufp_t) pnode;
    ql_elm(cw_bufp_t) plink;
};

struct cw_buf_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_BUF_MAGIC 0x348279bd
#endif

    /* Allocator state. */
    cw_bool_t alloced;
    cw_opaque_alloc_t *alloc;
    cw_opaque_realloc_t *realloc;
    cw_opaque_dealloc_t *dealloc;
    void *arg;

    /* Length. */
    cw_uint64_t len;

    /* Number of lines (>= 1). */
    cw_uint64_t nlines;

    /* bufp tree and list. */
    rb_tree(cw_bufp_t) ptree;
    ql_head(cw_bufp_t) plist;

    /* Pointer to the last bufp that stores its position relative to BOB. */
    cw_bufp_t *bufp_cur;

    /* An array of bufv's with twice as many elements as there are bufp's.  This
     * is large enough to create a vector for the entire buf, even if all bufp's
     * are split by their gaps. */
    cw_bufv_t *bufv;
    cw_uint32_t bufvcnt;

    /* Extent trees and lists.  ftree and flist are ordered in forward order.
     * rtree and rlist are ordered in reverse order. */
    rb_tree(cw_ext_t) ftree;
    ql_head(cw_ext_t) flist;
    rb_tree(cw_ext_t) rtree;
    ql_head(cw_ext_t) rlist;

    /* Extent stack, also used for tracking extents that grow from zero
     * length. */
    ql_head(cw_ext_t) elist;

    /* History (undo/redo), if non-NULL. */
    cw_hist_t *hist;
};

/* bufv. */
cw_uint64_t
bufv_copy(cw_bufv_t *a_to, cw_uint32_t a_to_len, const cw_bufv_t *a_fr,
	  cw_uint32_t a_fr_len, cw_uint64_t a_maxlen);
cw_uint64_t
bufv_rcopy(cw_bufv_t *a_to, cw_uint32_t a_to_len, const cw_bufv_t *a_fr,
	   cw_uint32_t a_fr_len, cw_uint64_t a_maxlen);

/* buf. */
cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_opaque_alloc_t *a_alloc,
	cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc,
	void *a_arg);

void
buf_delete(cw_buf_t *a_buf);

cw_uint64_t
buf_len(const cw_buf_t *a_buf);

cw_uint64_t
buf_nlines(const cw_buf_t *a_buf);

cw_bool_t
buf_hist_active_get(const cw_buf_t *a_buf);

void
buf_hist_active_set(cw_buf_t *a_buf, cw_bool_t a_active);

cw_bool_t
buf_undoable(const cw_buf_t *a_buf);

cw_bool_t
buf_redoable(const cw_buf_t *a_buf);

cw_uint64_t
buf_undo(cw_buf_t *a_buf, cw_mkr_t *a_mkr, cw_uint64_t a_count);

cw_uint64_t
buf_redo(cw_buf_t *a_buf, cw_mkr_t *a_mkr, cw_uint64_t a_count);

void
buf_hist_flush(cw_buf_t *a_buf);

void
buf_hist_group_beg(cw_buf_t *a_buf, cw_mkr_t *a_mkr);

cw_bool_t
buf_hist_group_end(cw_buf_t *a_buf);

#ifdef CW_BUF_DUMP
void
buf_dump(cw_buf_t *a_buf, const char *a_beg, const char *a_mid,
	 const char *a_end);
#endif

#ifdef CW_BUF_VALIDATE
void
buf_validate(cw_buf_t *a_buf);
#endif

/* mkr. */
void
mkr_new(cw_mkr_t *a_mkr, cw_buf_t *a_buf);

void
mkr_dup(cw_mkr_t *a_to, const cw_mkr_t *a_from);

void
mkr_delete(cw_mkr_t *a_mkr);

cw_buf_t *
mkr_buf(const cw_mkr_t *a_mkr);

cw_uint64_t
mkr_line_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence);

cw_uint64_t
mkr_line(cw_mkr_t *a_mkr);

cw_uint64_t
mkr_seek(cw_mkr_t *a_mkr, cw_sint64_t a_offset, cw_bufw_t a_whence);

cw_uint64_t
mkr_pos(const cw_mkr_t *a_mkr);

cw_uint8_t *
mkr_before_get(const cw_mkr_t *a_mkr);

cw_uint8_t *
mkr_after_get(const cw_mkr_t *a_mkr);

cw_bufv_t *
mkr_range_get(const cw_mkr_t *a_start, const cw_mkr_t *a_end,
	      cw_uint32_t *r_iovcnt);

void
mkr_before_insert(cw_mkr_t *a_mkr, const cw_bufv_t *a_bufv,
		  cw_uint32_t a_bufvcnt);

void
mkr_after_insert(cw_mkr_t *a_mkr, const cw_bufv_t *a_bufv,
		 cw_uint32_t a_bufvcnt);

void
mkr_remove(cw_mkr_t *a_start, cw_mkr_t *a_end);

#ifdef CW_BUF_DUMP
void
mkr_dump(cw_mkr_t *a_mkr, const char *a_beg, const char *a_mid,
	 const char *a_end);
#endif

#ifdef CW_BUF_VALIDATE
void
mkr_validate(cw_mkr_t *a_mkr);
#endif

/* ext. */
cw_ext_t *
ext_new(cw_ext_t *a_ext, cw_buf_t *a_buf);

void
ext_dup(cw_ext_t *a_to, cw_ext_t *a_from);

void
ext_delete(cw_ext_t *a_ext);

cw_buf_t *
ext_buf(const cw_ext_t *a_ext);

const cw_mkr_t *
ext_beg_get(cw_ext_t *a_ext);

void
ext_beg_set(cw_ext_t *a_ext, const cw_mkr_t *a_beg);

const cw_mkr_t *
ext_end_get(cw_ext_t *a_ext);

void
ext_end_set(cw_ext_t *a_ext, const cw_mkr_t *a_end);

cw_bool_t
ext_beg_open_get(const cw_ext_t *a_ext);

void
ext_beg_open_set(cw_ext_t *a_ext, cw_bool_t a_beg_open);

cw_bool_t
ext_end_open_get(const cw_ext_t *a_ext);

void
ext_end_open_set(cw_ext_t *a_ext, cw_bool_t a_end_open);

cw_bool_t
ext_attached_get(const cw_ext_t *a_ext);

void
ext_attach(cw_ext_t *a_ext);

void
ext_detach(cw_ext_t *a_ext);

cw_bool_t
ext_detachable_get(const cw_ext_t *a_ext);

void
ext_detachable_set(cw_ext_t *a_ext, cw_bool_t a_detachable);

/* Create the stack of extents that overlap a_mkr, which can then be iterated on
 * by ext_stack_down_get().  The stack is in f-order, starting at the top of the
 * stack. */
cw_uint32_t
ext_stack_init(const cw_mkr_t *a_mkr);

/* Get the extent in the stack that is below a_ext.  If a_ext is NULL, the top
 * element is returned. */
/* XXX This should be inlined. */
cw_ext_t *
ext_stack_down_get(cw_ext_t *a_ext);

void
ext_frag_get(const cw_mkr_t *a_mkr, cw_mkr_t *r_beg, cw_mkr_t *r_end);

#ifdef CW_BUF_DUMP
void
ext_dump(cw_ext_t *a_ext, const char *a_beg, const char *a_mid,
	 const char *a_end);
#endif

#ifdef CW_BUF_VALIDATE
void
ext_validate(cw_ext_t *a_ext);
#endif
