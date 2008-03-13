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
	/*
	 * Access is locked if this object has the locking bit set.
	 */
	cw_mtx_t	lock;
	/*
	 * File descriptor number.  -1: Invalid, -2: Wrapped.
	 */
	cw_sint32_t	fd;
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
	/*
	 * Used for wrapped files.
	 */
	cw_nxo_file_read_t	*read_f;
	cw_nxo_file_write_t	*write_f;
	void		*arg;
	cw_nxoi_t	position;
};

void	nxoe_l_file_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx);
cw_nxoe_t *nxoe_l_file_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
