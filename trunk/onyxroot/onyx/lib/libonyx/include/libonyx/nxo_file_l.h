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

/*
 * Size of stack-allocated buffer to use for nxo_file_readline().  If this
 * overflows, heap allocation is used.
 */
#ifdef _CW_DBG
#define	_CW_NXO_FILE_READLINE_BUFSIZE	 25
#else
#define	_CW_NXO_FILE_READLINE_BUFSIZE	100
#endif

typedef struct cw_nxoe_file_s cw_nxoe_file_t;

struct cw_nxoe_file_s {
	cw_nxoe_t	nxoe;
#ifdef _CW_THREADS
	/*
	 * Access is locked if this object has the locking bit set.
	 */
	cw_mtx_t	lock;
#endif
	cw_nx_t		*nx;

	enum {
		FILE_NONE,
#ifdef _CW_POSIX_FILE
		FILE_POSIX,
#endif
		FILE_SYNTHETIC
	}		mode;

	union {
		struct {
			cw_nxo_file_read_t	*read_f;
			cw_nxo_file_write_t	*write_f;
			cw_nxo_file_ref_iter_t	*ref_iter_f;
			cw_nxo_file_delete_t	*delete_f;
			void			*arg;
			cw_nxoi_t		position;
		}	s;
#ifdef _CW_POSIX_FILE
		struct {
			cw_sint32_t		fd;
			cw_bool_t		wrapped;
		}	p;
#endif
	}		f;
	/*
	 * Buffering.
	 */
	cw_uint8_t	*buffer;
	cw_uint32_t	buffer_size;
	enum {
		BUFFER_EMPTY,
		BUFFER_READ,
		BUFFER_WRITE
	}		buffer_mode;
	cw_uint32_t	buffer_offset;
};

cw_bool_t nxoe_l_file_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t
    a_iter);
cw_nxoe_t *nxoe_l_file_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
