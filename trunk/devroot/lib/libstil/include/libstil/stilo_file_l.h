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
 * Size of stack-allocated buffer to use for stilo_file_readline().  If this
 * overflows, heap allocation is used.
 */
#ifdef _LIBSTIL_DBG
#define	_CW_STILO_FILE_READLINE_BUFSIZE	 25
#else
#define	_CW_STILO_FILE_READLINE_BUFSIZE	100
#endif

typedef struct cw_stiloe_file_s cw_stiloe_file_t;

struct cw_stiloe_file_s {
	cw_stiloe_t	stiloe;
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
	cw_stilo_file_read_t	*read_f;
	cw_stilo_file_write_t	*write_f;
	void		*arg;
	cw_stiloi_t	position;
};

void	stiloe_l_file_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_file_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);
cw_stilte_t stilo_l_file_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);
