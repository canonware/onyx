/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

/* Size of stack-allocated buffer to use for nxo_file_readline().  If this
 * overflows, heap allocation is used. */
#ifdef CW_DBG
#define CW_NXO_FILE_READLINE_BUFSIZE 25
#else
#define CW_NXO_FILE_READLINE_BUFSIZE 100
#endif

typedef struct cw_nxoe_file_s cw_nxoe_file_t;

struct cw_nxoe_file_s
{
    cw_nxoe_t nxoe;
#ifdef CW_THREADS
    /* Access is locked if this object has the locking bit set. */
    cw_mtx_t lock;
#endif

    cw_uint8_t *origin;
    cw_uint32_t olen;

    enum
    {
	FILE_NONE,
#ifdef CW_POSIX_FILE
	FILE_POSIX,
#endif
	FILE_SYNTHETIC
    } mode:2;
    cw_bool_t nonblocking:1;

    union
    {
	struct
	{
	    cw_nxo_file_read_t *read_f;
	    cw_nxo_file_write_t *write_f;
	    cw_nxo_file_ref_iter_t *ref_iter_f;
	    cw_nxo_file_delete_t *delete_f;
	    void *arg;
	    cw_nxoi_t position;
	} s;
#ifdef CW_POSIX_FILE
	struct
	{
	    cw_sint32_t fd;
	    cw_bool_t close;
	} p;
#endif
    } f;
    /* Buffering. */
    cw_uint8_t *buffer;
    cw_uint32_t buffer_size;
    enum
    {
	BUFFER_EMPTY,
	BUFFER_READ,
	BUFFER_WRITE
    } buffer_mode;
    cw_uint32_t buffer_offset;
};

/* Private, but needed for the inlining of nxoe_l_file_delete(). */
cw_nxn_t
nxo_p_file_buffer_flush(cw_nxoe_file_t *a_file);

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_file_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_file_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_FILE_C_))
CW_INLINE cw_bool_t
nxoe_l_file_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter)
{
    cw_nxoe_file_t *file;

    file = (cw_nxoe_file_t *) a_nxoe;

    cw_check_ptr(file);
    cw_dassert(file->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(file->nxoe.type == NXOT_FILE);

    nxo_p_file_buffer_flush(file);
    if (file->buffer != NULL)
    {
	nxa_free(file->buffer, file->buffer_size);
    }
#ifdef CW_THREADS
    if (file->nxoe.locking)
    {
	mtx_delete(&file->lock);
    }
#endif
    if (file->origin != NULL)
    {
	nxa_free(file->origin, file->olen);
    }
    switch (file->mode)
    {
	case FILE_NONE:
	{
	    break;
	}
#ifdef CW_POSIX_FILE
	case FILE_POSIX:
	{
	    if (file->f.p.close)
	    {
		close(file->f.p.fd);
	    }
	    break;
	}
#endif
	case FILE_SYNTHETIC:
	{
	    if (file->f.s.delete_f != NULL)
	    {
		file->f.s.delete_f(file->f.s.arg);
	    }
	    break;
	}
    }

    nxa_free(file, sizeof(cw_nxoe_file_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_file_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_file_t *file;

    file = (cw_nxoe_file_t *) a_nxoe;

    if (file->mode == FILE_SYNTHETIC && file->f.s.ref_iter_f != NULL)
    {
	retval = file->f.s.ref_iter_f(file->f.s.arg, a_reset);
    }
    else
    {
	retval = NULL;
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_FILE_C_)) */
