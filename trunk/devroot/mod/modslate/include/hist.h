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

/* Define CW_HIST_DUMP to enable verbose internal history state dumping. */
#ifdef CW_BUF_DUMP
#define CW_HIST_DUMP
#endif

/* History record header. */
struct cw_histh_s
{
#ifdef CW_DBG
#define CW_HISTH_MAGIC 0x2389473d
    cw_uint32_t magic;
#endif
    /* Record tag.  Not an enumerated type since it needs to be only 8 bits. */
#define HISTH_TAG_NONE		 0
#define HISTH_TAG_SYNC		 1
#define HISTH_TAG_GBEG		 2
#define HISTH_TAG_GEND		 3
#define HISTH_TAG_POS		 4
#define HISTH_TAG_SINS		 5
#define HISTH_TAG_SYNK		 6
#define HISTH_TAG_SREM		 7
#define HISTH_TAG_SDEL		 8
#define HISTH_TAG_INS		 9
#define HISTH_TAG_YNK		10
#define HISTH_TAG_REM		11
#define HISTH_TAG_DEL		12
    cw_uint8_t tag;

    /* Auxiliary field (bpos or data byte count). */
    union
    {
	cw_uint8_t str[8];
	cw_uint8_t saux;
	cw_uint64_t aux; /* Always network byte order. */
    } u;

    /* Vector that is set up to contain the header data. */
    cw_bufv_t bufv[2];
    cw_uint32_t bufvcnt;
    /* Total byte count for bufv. */
    cw_uint32_t bufvlen;
};

struct cw_hist_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_HIST_MAGIC 0x353dd57a
#endif
    /* History buffer. */
    cw_buf_t h;

    /* Header and footer of current record in h. */
    cw_histh_t hhead;
    cw_histh_t hfoot;

    /* Marker at begin, current position, and end of current record in h. */
    cw_mkr_t hbeg;
    cw_mkr_t hcur;
    cw_mkr_t hend;

    /* Temporary marker in h. */
    cw_mkr_t htmp;

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
hist_undoable(const cw_hist_t *a_hist, const cw_buf_t *a_buf);

cw_bool_t
hist_redoable(const cw_hist_t *a_hist, const cw_buf_t *a_buf);

cw_uint64_t
hist_undo(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_mkr_t *a_mkr,
	  cw_uint64_t a_count);

cw_uint64_t
hist_redo(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_mkr_t *a_mkr,
	  cw_uint64_t a_count);

void
hist_flush(cw_hist_t *a_hist, cw_buf_t *a_buf);

void
hist_group_beg(cw_hist_t *a_hist, cw_buf_t *a_buf, cw_mkr_t *a_mkr);

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

#ifdef CW_BUF_DUMP
void
hist_dump(cw_hist_t *a_hist, const char *a_beg, const char *a_mid,
	  const char *a_end);
#endif

#ifdef CW_BUF_VALIDATE
void
hist_validate(cw_hist_t *a_hist, cw_buf_t *a_buf);
#endif
