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

typedef enum {
	/* Interpreter errors. */
	STILTE_DICTSTACKOVERFLOW,	/* dstack too deep. */
	STILTE_DICTSTACKUNDERFLOW,	/* No poppable dictionary on dstack. */
	STILTE_EXECSTACKOVERFLOW,	/* estack too deep. */
	STILTE_INTERRUPT,		/* Interrupt. */
	STILTE_INVALIDACCESS,		/* Permission error. */
	STILTE_INVALIDCONTEXT,		/* Bad thread context. */
	STILTE_INVALIDEXIT,		/* exit operator called outside loop. */
	STILTE_INVALIDFILEACCESS,	/* Insufficient file permissions. */
	STILTE_IOERROR,			/* read()/write()/etc. error. */
	STILTE_LIMITCHECK,		/* Value outside legal range. */
	STILTE_RANGECHECK,		/* Out of bounds string or array use. */
	STILTE_STACKOVERFLOW,		/* ostack too full. */
	STILTE_STACKUNDERFLOW,		/* Not enough objects on ostack. */
	STILTE_SYNTAXERROR,		/* Scanner syntax error. */
	STILTE_TIMEOUT,			/* Timeout. */
	STILTE_TYPECHECK,		/* Incorrect argument type. */
	STILTE_UNDEFINED,		/* Object not found in dstack. */
	STILTE_UNDEFINEDFILENAME,	/* Bad filename. */
	STILTE_UNDEFINEDRESOURCE,	/* Resource not found. */
	STILTE_UNDEFINEDRESULT,
	STILTE_UNMATCHEDMARK,		/* No mark on ostack. */
	STILTE_UNREGISTERED,		/* Other non-enumerated error. */
	STILTE_VMERROR			/* Out of memory. */
} cw_stilte_t;

typedef struct cw_stilts_s cw_stilts_t;
typedef enum {
	STILTTS_START,
	STILTTS_LT_CONT,
	STILTTS_GT_CONT,
	STILTTS_SLASH_CONT,
	STILTTS_COMMENT,
	STILTTS_INTEGER,
	STILTTS_INTEGER_RADIX,
	STILTTS_ASCII_STRING,
	STILTTS_ASCII_STRING_NEWLINE_CONT,
	STILTTS_ASCII_STRING_PROT_CONT,
	STILTTS_ASCII_STRING_CRLF_CONT,
	STILTTS_ASCII_STRING_HEX_CONT,
	STILTTS_ASCII_STRING_HEX_FINISH,
	STILTTS_LIT_STRING,
	STILTTS_LIT_STRING_NEWLINE_CONT,
	STILTTS_LIT_STRING_PROT_CONT,
	STILTTS_HEX_STRING,
	STILTTS_BASE64_STRING,
	STILTTS_BASE64_STRING_PAD,
	STILTTS_BASE64_STRING_TILDE,
	STILTTS_BASE64_STRING_FINISH,
	STILTTS_NAME
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
	cw_stils_t	estack;	/* Execution stack. */
	cw_stils_t	ostack;	/* Operand stack. */
	cw_stils_t	dstack;	/* Dictionary stack. */
	cw_stils_t	tstack;	/* Temp stack. */

	/*
	 * Local dictionaries.
	 */
	cw_stilo_t	userdict;
	cw_stilo_t	errordict;

	/*
	 * Number of objects to pop off ostack if there is an error.
	 */
	cw_uint32_t	hedge;

	/*
	 * Tokenizer state.  If a token is broken across two or more input
	 * strings, data are copied to an internal buffer, and state machine
	 * state is preserved so that the buffered data need not be processed
	 * again.
	 */

	/* Current scanner state. */
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
			cw_uint32_t	base;
			cw_uint32_t	b_off;	/* Depends on sign, radix. */
		}       n;	/* number. */
		struct {
			cw_uint8_t	hex_val;
		}	s;	/* string. */
		struct {
			cw_uint32_t	nodd;
		}	b;	/* base 64 string. */
		struct {
			enum {
				ACTION_EXECUTE,
				ACTION_LITERAL,
				ACTION_EVALUATE
			}	action;
		}	m;	/* name. */
	}       m;
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
#define		stilt_start(a_stilt) systemdict_start(a_stilt)
#define		stilt_executive(a_stilt) systemdict_executive(a_stilt)

#define		stilt_state(a_stilt) (a_stilt)->state
#define		stilt_deferred(a_stilt) ((a_stilt)->defer_count ? TRUE : FALSE)
void		stilt_loop(cw_stilt_t *a_stilt);
void		stilt_interpret(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
void		stilt_flush(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts);
void		stilt_detach(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
void		stilt_error(cw_stilt_t *a_stilt, cw_stilte_t a_error);

cw_bool_t	stilt_dict_stack_search(cw_stilt_t *a_stilt, cw_stilo_t *a_key,
    cw_stilo_t *r_value);

#define		stilt_stil_get(a_stilt) (a_stilt)->stil
#define		stilt_name_hash_get(a_stilt) &(a_stilt)->name_hash

#define		stilt_stdin_get(a_stilt) stil_stdin_get((a_stilt)->stil)
#define		stilt_stdout_get(a_stilt) stil_stdout_get((a_stilt)->stil)
#define		stilt_stderr_get(a_stilt) stil_stderr_get((a_stilt)->stil)

#define		stilt_ostack_get(a_stilt) (&((a_stilt)->ostack))
#define		stilt_dstack_get(a_stilt) (&((a_stilt)->dstack))
#define		stilt_estack_get(a_stilt) (&((a_stilt)->estack))
#define		stilt_tstack_get(a_stilt) (&((a_stilt)->tstack))

#define		stilt_systemdict_get(a_stilt)				\
	(stil_systemdict_get((a_stilt)->stil))
#define		stilt_globaldict_get(a_stilt)				\
	(stil_globaldict_get((a_stilt)->stil))
#define		stilt_userdict_get(a_stilt) (&((a_stilt)->userdict))
#define		stilt_errordict_get(a_stilt) (&((a_stilt)->errordict))

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
#define		stilt_malloc(a_stilt, a_size)				\
	stilat_malloc_e(&(a_stilt)->stilat, (a_size), __FILE__, __LINE__)
#define		stilt_free(a_stilt, a_ptr)				\
	stilat_free_e(&(a_stilt)->stilat, (a_ptr), __FILE__, __LINE__)
#else
#define		stilt_malloc(a_stilt, a_size)				\
	stilat_malloc_e(&(a_stilt)->stilat, (a_size), NULL, 0)
#define		stilt_free(a_stilt, a_ptr)				\
	stilat_free_e(&(a_stilt)->stilat, (a_ptr), NULL, 0)
#endif

#define		stilt_chi_get(a_stilt)					\
	stilat_chi_get(&(a_stilt)->stilat)
#define		stilt_chi_put(a_stilt, a_chi)				\
	stilat_chi_put(&(a_stilt)->stilat, (a_chi))

#define		stilt_stilsc_get(a_stilt)				\
	stilat_stilsc_get(&(a_stilt)->stilat)
#define		stilt_stilsc_put(a_stilt, a_stilsc)			\
	stilat_stilsc_put(&(a_stilt)->stilat, (a_stilsc))

#define		stilt_dicto_get(a_stilt)				\
	stilat_dicto_get(&(a_stilt)->stilat)
#define		stilt_dicto_put(a_stilt, a_dicto)			\
	stilat_dicto_put(&(a_stilt)->stilat, (a_dicto))
