/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Defined in stilo.h to resolve a circular dependency. */
#if (0)
typedef struct cw_stilt_s cw_stilt_t;
#endif
typedef struct cw_stiltn_s cw_stiltn_t;

struct cw_stilt_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced;

	/* stil this stilt is part of. */
	cw_stil_t	*stil;

	/*
	 * Hash of cached stiln references.  Keys are (cw_stilnk_t *),
	 * direct hashed; values are (cw_stiltn_t *).
	 */
	cw_dch_t	stiln_dch;

	/*
	 * Hash of external references to the local VM, used for mark and
	 * sweep garbage collection.  Keys are (cw_stilo_t *); values are
	 * (cw_stiloe_t *). References need not be looked at directly, since
	 * the value field in the hash table is all we need to know.
	 */
	cw_dch_t	roots_dch;

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

	/*
	 * If greater than _CW_STIL_BUFC_SIZE, tok_buffer is a buf.
	 * Otherwise, it is a character array.
	 */
	cw_uint32_t	index;

	union {
		cw_buf_t	buf;
		cw_uint8_t	str[_CW_STIL_BUFC_SIZE];
	}	tok_buffer;
	enum {
		_CW_STILT_STATE_START,
		_CW_STILT_STATE_LT_CONT,
		_CW_STILT_STATE_GT_CONT,
		_CW_STILT_STATE_SLASH_CONT,
		_CW_STILT_STATE_COMMENT,
		_CW_STILT_STATE_NUMBER,
		_CW_STILT_STATE_ASCII_STRING,
		_CW_STILT_STATE_ASCII_STRING_NEWLINE_CONT,
		_CW_STILT_STATE_ASCII_STRING_PROT_CONT,
		_CW_STILT_STATE_ASCII_STRING_CRLF_CONT,
		_CW_STILT_STATE_ASCII_STRING_HEX_CONT,
		_CW_STILT_STATE_ASCII_STRING_HEX_FINISH,
		_CW_STILT_STATE_HEX_STRING,
		_CW_STILT_STATE_BASE85_STRING,
		_CW_STILT_STATE_BASE85_STRING_CONT,
		_CW_STILT_STATE_NAME
	}	state;

	union {
		struct {
			enum {
				_CW_STILS_NUMBER_POS,
				_CW_STILS_NUMBER_NEG
			}	sign;
			cw_uint32_t	base;
			cw_sint32_t	point_offset;
			cw_uint32_t	begin_offset;
		}       number;
		struct {
			cw_uint32_t	paren_depth;
			cw_uint8_t	hex_val;
		}	string;
		struct {
			cw_bool_t	is_literal;
			cw_bool_t	is_immediate;
		}	name;
	}       meta;
	cw_bool_t	procedure_depth;
};

struct cw_stiltn_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	/* Number of simple references to this object. */
	cw_uint32_t	ref_count;

	/*
	 * Key.  This is a copy of the stilnk in the main stil's names hash.
	 * The value is stored as the data pointer in the hash table.
	 */
	cw_stilnk_t	key;
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

#define		stilt_get_stil(a_stilt) (a_stilt)->stil

void		*stilt_malloc(cw_stilt_t *a_stilt, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
#define		_cw_stilt_malloc(a_stilt, a_size)			\
	stilt_malloc((a_stilt), (a_size), __FILE__, __LINE__)
#else
#define		_cw_stilt_malloc(a_stilt, a_size)			\
	stilt_malloc((a_stilt), (a_size), NULL, 0)
#endif

void		stilt_free(cw_stilt_t *a_stilt, void *a_ptr, const char
    *a_filename, cw_uint32_t a_line_num);
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
#define		_cw_stilt_free(a_stilt, a_ptr)				\
	stilt_free((a_stilt), (a_ptr), __FILE__, __LINE__)
#else
#define		_cw_stilt_free(a_stilt, a_ptr)				\
	stilt_free((a_stilt), (a_ptr), NULL, 0)
#endif

#define stilt_stdout_get(a_stilt) (a_stilt)->stdout_fd

#define stilt_data_stack_get(a_stilt) (&((a_stilt)->data_stils))

/* stiltn. */
const cw_stiln_t	*stiltn_ref(cw_stilt_t *a_stilt, const cw_uint8_t
    *a_name, cw_uint32_t a_len, cw_bool_t a_force);

void		stiltn_unref(cw_stilt_t *a_stilt, const cw_stiln_t *a_stiln);
