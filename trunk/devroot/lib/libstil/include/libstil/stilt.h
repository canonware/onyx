/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#ifdef _LIBSTIL_DBG
#define	_CW_STILT_BUFFER_SIZE	  8
#else
#define	_CW_STILT_BUFFER_SIZE	256
#endif

struct cw_stilt_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced;

	/* stil this stilt is part of. */
	cw_stil_t	*stil;

	/* Allocator. */
	cw_stilat_t	stilat;

	/* Thread-specific name cache. */
	cw_stilnt_t	stilnt;

	/*
	 * Stacks.
	 */
	cw_stils_t	exec_stils;
	cw_stils_t	data_stils;
	cw_stils_t	dict_stils;

	/*
	 * File handles.
	 */
	cw_sint32_t	stdout_fd;

	/*
	 * Tokenizer state.  If a token is broken across two or more input
	 * strings, data are copied to an internal buffer, and state machine
	 * state is preserved so that the buffered data need not be
	 * processed again.
	 */

	/*
	 * Every time a '{' token is encountered by the scanner, this value is
	 * incremented, and this value is decremented every time the scanner
	 * encounters a '}' token.  Execution of objects is deferred if this
	 * value is non-zero.
	 */
	cw_uint32_t	defer_count;

	/*
	 * Current line number (counting starts at 1 by convention) and column
	 * number (counting starts at 0).
	 */
	cw_uint32_t	line;
	cw_sint32_t	column;
	
	/*
	 * Position where the current token starts.
	 */
	cw_uint32_t	tok_line;
	cw_uint32_t	tok_column;

	/* Offset of first invalid character in tok_str. */
	cw_uint32_t	index;

	/*
	 * Pointer to the token buffer.  As long as index is less than
	 * _CW_STILT_BUFFER_SIZE, tok_str actually points to buffer.
	 * Otherwise, adequate space is allocated (using exponential doubling),
	 * and the contents of tok_buffer are copied to the allocated buffer.
	 *
	 * If a temporary buffer is allocated, it is discarded as soon as the
	 * token is handled.  That is, tok_buffer is used for every token until
	 * (if) tok_buffer overflows.
	 */
	cw_uint8_t	*tok_str;
	cw_uint32_t	buffer_len;	/* Only valid if buffer overflowed. */
	cw_uint8_t	buffer[_CW_STILT_BUFFER_SIZE];

	enum {
		STATE_START,
		STATE_LT_CONT,
		STATE_GT_CONT,
		STATE_SLASH_CONT,
		STATE_COMMENT,
		STATE_NUMBER,
		STATE_ASCII_STRING,
		STATE_ASCII_STRING_NEWLINE_CONT,
		STATE_ASCII_STRING_PROT_CONT,
		STATE_ASCII_STRING_CRLF_CONT,
		STATE_ASCII_STRING_HEX_CONT,
		STATE_ASCII_STRING_HEX_FINISH,
		STATE_LIT_STRING,
		STATE_LIT_STRING_NEWLINE_CONT,
		STATE_LIT_STRING_PROT_CONT,
		STATE_HEX_STRING,
		STATE_BASE85_STRING,
		STATE_BASE85_STRING_CONT,
		STATE_NAME
	}	state;

	union {
		struct {
			enum {POS, NEG}	sign;
			cw_uint32_t	base;
			cw_sint32_t	point_offset;
			cw_uint32_t	begin_offset;
		}       number;
		struct {
			cw_uint8_t	hex_val;
		}	string;
		struct {
			enum {EXEC, LITERAL, EVAL} action;
		}	name;
	}       meta;
	cw_bool_t	procedure_depth;
};

cw_stilt_t	*stilt_new(cw_stilt_t *a_stilt, cw_stil_t *a_stil);
void		stilt_delete(cw_stilt_t *a_stilt);

void		stilt_get_position(cw_stilt_t *a_stilt, cw_uint32_t *r_line,
    cw_uint32_t *r_column);
void		stilt_set_position(cw_stilt_t *a_stilt, cw_uint32_t a_line,
    cw_uint32_t a_column);
	
cw_bool_t	stilt_interp_str(cw_stilt_t *a_stilt, const char *a_str,
    cw_uint32_t a_len);
cw_bool_t	stilt_interp_buf(cw_stilt_t *a_stilt, cw_buf_t *a_buf);
cw_bool_t	stilt_detach_str(cw_stilt_t *a_stilt, const char *a_str,
    cw_uint32_t a_len);
cw_bool_t	stilt_detach_buf(cw_stilt_t *a_stilt, cw_buf_t *a_buf);

#define		stilt_stil_get(a_stilt) (a_stilt)->stil
#define		stilt_stilnt_get(a_stilt) &(a_stilt)->stilnt
#define		stilt_stdout_get(a_stilt) (a_stilt)->stdout_fd
#define		stilt_data_stack_get(a_stilt) (&((a_stilt)->data_stils))

/*
 * If TRUE, allocation for the stilt is global.  Otherwise, allocation is
 * local.
 */
#define		stilt_currentglobal(a_stilt)				\
	stilat_currentglobal(&(a_stilt)->stilat)
#define		stilt_setglobal(a_stilt, a_global)			\
	stilat_setglobal(&(a_stilt)->stilat, (a_global))

#define		stilt_mem_get(a_stilt)					\
	stilat_mem_get(&(a_stilt)->stilat)

#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
#define		_cw_stilt_malloc(a_stilt, a_size)			\
	stilat_malloc(&(a_stilt)->stilat, (a_size), __FILE__, __LINE__)
#define		_cw_stilt_free(a_stilt, a_ptr)				\
	stilat_free(&(a_stilt)->stilat, (a_ptr), __FILE__, __LINE__)
#else
#define		_cw_stilt_malloc(a_stilt, a_size)			\
	stilat_malloc(&(a_stilt)->stilat, (a_size), NULL, 0)
#define		_cw_stilt_free(a_stilt, a_ptr)				\
	stilat_free(&(a_stilt)->stilat, (a_ptr), NULL, 0)
#endif

#define		_cw_stilt_chi_get(a_stilt)				\
	_cw_stilat_chi_get(&(a_stilt)->stilat)
#define		_cw_stilt_chi_put(a_stilt, a_chi)			\
	_cw_stilat_chi_put(&(a_stilt)->stilat, (a_chi))

#define		_cw_stilt_stilsc_get(a_stilt)				\
	_cw_stilat_stilsc_get(&(a_stilt)->stilat)
#define		_cw_stilt_stilsc_put(a_stilt, a_stilsc)			\
	_cw_stilat_stilsc_put(&(a_stilt)->stilat, (a_stilsc))

#define		_cw_stilt_dicto_get(a_stilt)				\
	_cw_stilat_dicto_get(&(a_stilt)->stilat)
#define		_cw_stilt_dicto_put(a_stilt, a_dicto)			\
	_cw_stilat_dicto_put(&(a_stilt)->stilat, (a_dicto))
