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

typedef struct cw_stilts_s cw_stilts_t;
typedef enum {
	STATE_START			=  0,
	STATE_LT_CONT			=  1,
	STATE_GT_CONT			=  2,
	STATE_SLASH_CONT		=  3,
	STATE_COMMENT			=  4,
	STATE_NUMBER			=  5,
	STATE_ASCII_STRING		=  6,
	STATE_ASCII_STRING_NEWLINE_CONT	=  7,
	STATE_ASCII_STRING_PROT_CONT	=  8,
	STATE_ASCII_STRING_CRLF_CONT	=  9,
	STATE_ASCII_STRING_HEX_CONT	= 10,
	STATE_ASCII_STRING_HEX_FINISH	= 11,
	STATE_LIT_STRING		= 12,
	STATE_LIT_STRING_NEWLINE_CONT	= 13,
	STATE_LIT_STRING_PROT_CONT	= 14,
	STATE_HEX_STRING		= 15,
	STATE_BASE85_STRING		= 16,
	STATE_BASE85_STRING_CONT	= 17,
	STATE_NAME			= 18
} cw_stiltts_t;

struct cw_stilts_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced;

	/*
	 * Current line number (counting starts at 1 by convention) and column
	 * number (counting starts at 0).
	 */
	cw_uint32_t	line;
	cw_sint32_t	column;
};

struct cw_stilt_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced;

	/* stil this stilt is part of. */
	cw_stil_t	*stil;

	/* Allocator. */
	cw_stilat_t	stilat;

        /*
         * Thread-specific name cache hash (key: {name, len}, value:
         * (stiloe_name *)).  This hash table keeps track of name "values" that
         * are in existence within a particular local VM.
         */
	cw_dch_t	name_hash;

	/*
	 * Stacks.
	 */
	cw_stils_t	exec_stils;
	cw_stils_t	data_stils;
	cw_stils_t	dict_stils;

	/*
	 * File handles.
	 *
	 * XXX File handles should eventually be objects in threaddict.
	 */
	cw_sint32_t	stdout_fd;

	/*
	 * Tokenizer state.  If a token is broken across two or more input
	 * strings, data are copied to an internal buffer, and state machine
	 * state is preserved so that the buffered data need not be processed
	 * again.
	 */

	cw_stiltts_t	state;

	/*
	 * Every time a '{' token is encountered by the scanner, this value is
	 * incremented, and this value is decremented every time the scanner
	 * encounters a '}' token.  Execution of objects is deferred if this
	 * value is non-zero.
	 */
	cw_uint32_t	defer_count;

	/* Offset of first invalid character in tok_str. */
	cw_uint32_t	index;

	/*
	 * Position where the current token starts.
	 */
	cw_uint32_t	tok_line;
	cw_uint32_t	tok_column;

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

	union {
		struct {
			enum {
				SIGN_POS,
				SIGN_NEG
			}		sign;
			cw_uint32_t	base;
			cw_sint32_t	point_offset;
			cw_uint32_t	begin_offset;
		}       number;
		struct {
			cw_uint8_t	hex_val;
		}	string;
		struct {
			enum {
				ACTION_EXECUTE,
				ACTION_LITERAL,
				ACTION_EVALUATE
			}	action;
		}	name;
	}       meta;
};

/*
 * stilts.
 */
cw_stilts_t	*stilts_new(cw_stilts_t *a_stilts, cw_stilt_t *a_stilt);
void		stilts_delete(cw_stilts_t *a_stilts, cw_stilt_t *a_stilt);
void		stilts_position_get(cw_stilts_t *a_stilt, cw_uint32_t *r_line,
    cw_uint32_t *r_column);
void		stilts_position_set(cw_stilts_t *a_stilt, cw_uint32_t a_line,
    cw_uint32_t a_column);

/*
 * stilt.
 */
cw_stilt_t	*stilt_new(cw_stilt_t *a_stilt, cw_stil_t *a_stil);
void		stilt_delete(cw_stilt_t *a_stilt);
#define		stilt_state(a_stilt) (a_stilt)->state
#define		stilt_deferred(a_stilt) ((a_stilt)->defer_count ? TRUE : FALSE)

cw_bool_t	stilt_interp_str(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
cw_bool_t	stilt_interp_buf(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts,
    cw_buf_t *a_buf);
cw_bool_t	stilt_detach_str(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
cw_bool_t	stilt_detach_buf(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts,
    cw_buf_t *a_buf);
cw_bool_t	stilt_dict_stack_search(cw_stilt_t *a_stilt, cw_stilo_t *a_key,
    cw_stilo_t *r_value);


#define		stilt_stil_get(a_stilt) (a_stilt)->stil
#define		stilt_name_hash_get(a_stilt) &(a_stilt)->name_hash
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
