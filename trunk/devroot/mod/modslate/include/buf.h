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

struct cw_mkr_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_MKR_MAGIC 0x2e84a3c9
#endif

    /* bufp this marker is in. */
    cw_bufp_t *bufp;

    /* Gap movement can change this. */
    cw_uint64_t ppos;

    /* Line number, relative to the beginning of bufp (>= 0). */
    cw_uint64_t pline;

    /* Ordered mkr tree and list linkage. */
    rb_node(cw_mkr_t) node;
    ql_elm(cw_mkr_t) link;
};

struct cw_ext_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_EXT_MAGIC 0x8a94e34c
#endif

    /* Beginning and ending markers. */
    cw_mkr_t beg;
    cw_mkr_t end;

    /* Extents are either open or closed at each end. */
    cw_bool_t beg_open:1;
    cw_bool_t end_open:1;

    /* A detachable extent is removed from the buffer if its size reaches 0. */
    cw_bool_t detachable:1;
    cw_bool_t detached:1;

    /* Forward- and reverse-ordered ext tree and list linkage. */
    rb_node(cw_ext_t) fnode;
    ql_elm(cw_ext_t) flink;
    rb_node(cw_ext_t) rnode;
    ql_elm(cw_ext_t) rlink;
};

struct cw_bufp_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_BUFP_MAGIC 0xda7d87d3
#endif

    /* Parent buf. */
    cw_buf_t *buf;

    /* Offset into the buf's bufps array. */
    cw_uint32_t index;

    /* Cached position of the begin of the bufp, relative to the begin/end
     * of the entire buf.  The validity of these values is determined by the
     * bob_cached/eob_cached fields of the buf. */
    cw_uint64_t bpos;
    cw_uint64_t line;
    cw_uint64_t ebpos;
    cw_uint64_t eline;

    /* Length. */
    cw_uint32_t len;

    /* Number of newlines. */
    cw_uint32_t nlines;

    /* Gap offset, in elements. */
    cw_uint32_t gap_off;

    /* Gap length, in elements. */
    cw_uint32_t gap_len;

    /* Text buffer, with gap. */
#define CW_BUFP_SIZE 65536
    cw_uint8_t *b;

    /* Tree and list of mkr's that point into the bufp.  Both of these are
     * ordered and kept up to date.  Random access is done via the tree, and
     * iteration is done via the list. */
    rb_tree(cw_mkr_t) mtree;
    ql_head(cw_mkr_t) mlist;
};

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

    /* Length. */
    cw_uint64_t len;

    /* Number of lines (>= 1). */
    cw_uint64_t nlines;

    /* bufp tree and list. */
    rb_tree(cw_bufp_t) ptree;
    ql_head(cw_bufp_t) plist;
    /* Array of pointers to bufp's.  There are nbufps elements. */
//    cw_bufp_t **bufps;
//    cw_uint32_t nbufps;

    /* Pointers to the ends of the bufp ranges with valid caches. */
    cw_bufp_t *bob_cached;
    cw_bufp_t *eob_cached;

    /* Index of first and last bufp with valid caches.  The first bufp always
     * has a valid cache, which allows bob_cached to be unsigned.  If no bufp's
     * at the end have a valid cache, then eob_cached is set to nbufps. */
//    cw_uint32_t bob_cached;
//    cw_uint32_t eob_cached;

    /* An array of (2 * nbufps) bufv's.  This is large enough to create a vector
     * for the entire buf, even if all bufp's are split by their gaps. */
    cw_bufv_t *bufv;

    /* Extent trees and lists.  ftree and flist are ordered in forward order.
     * rtree and rlist are ordered in reverse order.
     *
     * For ext's A and B, where beg(X) is the beginning position of X and end(X)
     * is the ending position of X:
     *
     *   Forward order : if ((beg(A) < beg(B))
     *                       || (start(A) == start(B) && end(A) > end(B)))
     *                   {
     *                       A < B
     *                   }
     *
     *   Reverse order : if ((end(A) < end(B))
     *                       || (end(A) == end(B) && beg(A) > beg(B)))
     *                   {
     *                       A < B
     *                   }
     */
    rb_tree(cw_ext_t) ftree;
    ql_head(cw_ext_t) flist;
    rb_tree(cw_ext_t) rtree;
    ql_head(cw_ext_t) rlist;

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
ext_detachable_get(const cw_ext_t *a_ext);

void
ext_detachable_set(cw_ext_t *a_ext, cw_bool_t a_detachable);

cw_bool_t
ext_detached_get(const cw_ext_t *a_ext);

void
ext_detached_set(cw_ext_t *a_ext, cw_bool_t a_detached);

void
ext_detach(cw_ext_t *a_ext);

/* Get the first and last ext's that overlap a_mkr.  retval is the length of the
 * run.  r_beg and r_end are NULL if there are no extents overlapping the
 * run. */
cw_uint64_t
ext_run_get(const cw_mkr_t *a_mkr, cw_ext_t *r_beg, cw_ext_t *r_end);

/* Iterate in f-order. */
cw_ext_t *
ext_prev_get(const cw_ext_t *a_ext);

cw_ext_t *
ext_next_get(const cw_ext_t *a_ext);
