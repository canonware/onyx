/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
	cw_uint32_t magic;
#endif

	cw_bool_t is_malloced;

	/* stil this stilt is part of. */
	cw_stil_t *stil;

	/*
	 * Hash of cached stiln references.  Keys are (cw_stilnk_t *),
	 * direct hashed; values are (cw_stiltn_t *).
	 */
	cw_dch_t stiln_dch;

	/*
	 * Hash of external references to the local VM, used for mark and
	 * sweep garbage collection.  Keys are (cw_stilo_t *); values are
	 * (cw_stiloe_t *). References need not be looked at directly, since
	 * the value field in the hash table is all we need to know.
	 */
	cw_dch_t roots_dch;

	/*
	 * Tokenizer state.  If a token is broken across two or more input
	 * strings, data are copied to an internal buffer, and state machine
	 * state is preserved so that the buffered data need not be
	 * processed again.
	 */

	/*
	 * If greater than _CW_STIL_BUFC_SIZE, tok_buffer is a buf.
	 * Otherwise, it is a character array.
	 */
	cw_uint32_t index;

	union {
		cw_buf_t buf;
		cw_uint8_t str[_CW_STIL_BUFC_SIZE];
	}       tok_buffer;
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
	}       state;

	union {
		struct {
			enum {
				_CW_STILS_NUMBER_POS,
				_CW_STILS_NUMBER_NEG
			}       sign;
			cw_uint32_t base;
			cw_sint32_t point_offset;
			cw_uint32_t begin_offset;
		}       number;
		struct {
			cw_uint32_t paren_depth;
			cw_uint8_t hex_val;
		}       string;
		struct {
			cw_bool_t is_literal;
			cw_bool_t is_immediate;
		}       name;
	}       meta;
	cw_bool_t procedure_depth;
};

struct cw_stiltn_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t magic;
#endif

	/* Number of simple references to this object. */
	cw_uint32_t ref_count;

	/*
	 * Key.  This is a copy of the stilnk in the main stil's names hash.
	 * The value is stored as the data pointer in the hash table.
	 */
	cw_stilnk_t key;
};

cw_stilt_t	*stilt_new(cw_stilt_t *a_stilt, cw_stil_t *a_stil);

void		stilt_delete(cw_stilt_t *a_stilt);

cw_bool_t	stilt_interp_str(cw_stilt_t *a_stilt, const char *a_str,
    cw_uint32_t a_len);

cw_bool_t	stilt_interp_buf(cw_stilt_t *a_stilt, cw_buf_t *a_buf);

cw_bool_t	stilt_detach_str(cw_stilt_t *a_stilt, const char *a_str,
    cw_uint32_t a_len);

cw_bool_t	stilt_detach_buf(cw_stilt_t *a_stilt, cw_buf_t *a_buf);

const cw_stiln_t	*stiltn_ref(cw_stilt_t *a_stilt, const cw_uint8_t
    *a_name, cw_uint32_t a_len, cw_bool_t a_force);

void		stiltn_unref(cw_stilt_t *a_stilt, const cw_stiln_t *a_stiln);
