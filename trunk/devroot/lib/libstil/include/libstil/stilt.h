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

#ifdef _LIBSTIL_DBG
#define	_CW_STILT_BUFFER_SIZE	  8
#else
#define	_CW_STILT_BUFFER_SIZE	256
#endif

typedef struct cw_stilt_thread_entry_s cw_stilt_thread_entry_t;

struct cw_stilt_thread_entry_s {
	cw_stilt_t	*stilt;
	cw_thd_t	*thd;
	cw_mtx_t	lock;
	cw_cnd_t	done_cnd;
	cw_cnd_t	join_cnd;
	cw_bool_t	done:1;
	cw_bool_t	gone:1;
	cw_bool_t	detached:1;
	cw_bool_t	joined:1;
};

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
#ifdef _LIBSTIL_DBG
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
#ifdef _LIBSTIL_DBG
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced:1;

	/*
	 * stil this stilt is part of.
	 */
	cw_stil_t	*stil;

	/*
	 * Linkage for list of stilt's.
	 */
	ql_elm(cw_stilt_t) link;

	/*
	 * Used by stilt_thread(), stilt_detach(), and stilt_join().
	 */
	cw_stilt_thread_entry_t *entry;

	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;

	/*
	 * TRUE  : Global allocation mode.
	 * FALSE : Local allocation mode.
	 */
	cw_bool_t	global:1;

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
	cw_stilo_t	derror;
	cw_stilo_t	errordict;
	cw_stilo_t	userdict;
	cw_stilo_t	threaddict;

	/* Current error value. */
	cw_stilte_t	error;

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
			cw_uint32_t	p_depth;
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
 * stilte.
 */
cw_stiln_t stilte_stiln(cw_stilte_t a_stilte);

/*
 * stilts.
 */
cw_stilts_t *stilts_new(cw_stilts_t *a_stilts);
void	stilts_delete(cw_stilts_t *a_stilts, cw_stilt_t *a_stilt);
void	stilts_position_get(cw_stilts_t *a_stilt, cw_uint32_t *r_line,
    cw_uint32_t *r_column);
void	stilts_position_set(cw_stilts_t *a_stilt, cw_uint32_t a_line,
    cw_uint32_t a_column);

/*
 * stilt.
 */
cw_stilt_t *stilt_new(cw_stilt_t *a_stilt, cw_stil_t *a_stil);
void	stilt_delete(cw_stilt_t *a_stilt);
void	stilt_start(cw_stilt_t *a_stilt);
#define	stilt_executive(a_stilt) systemdict_executive(a_stilt)

void	stilt_thread(cw_stilt_t *a_stilt);
void	stilt_detach(cw_stilt_t *a_stilt);
void	stilt_join(cw_stilt_t *a_stilt);

#define	stilt_state(a_stilt) (a_stilt)->state
#define	stilt_deferred(a_stilt) ((a_stilt)->defer_count ? TRUE : FALSE)
void	stilt_reset(cw_stilt_t *a_stilt);
void	stilt_undefer(cw_stilt_t *a_stilt);
void	stilt_loop(cw_stilt_t *a_stilt);
void	stilt_interpret(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts, const
    cw_uint8_t *a_str, cw_uint32_t a_len);
void	stilt_flush(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts);
cw_uint32_t stilt_token(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts, const
    cw_uint8_t *a_str, cw_uint32_t a_len);
void	stilt_error(cw_stilt_t *a_stilt, cw_stilte_t a_error);

cw_bool_t stilt_dict_stack_search(cw_stilt_t *a_stilt, cw_stilo_t *a_key,
    cw_stilo_t *r_value);

#define	stilt_stil_get(a_stilt) (a_stilt)->stil

#define	stilt_stdin_get(a_stilt) stil_stdin_get((a_stilt)->stil)
#define	stilt_stdout_get(a_stilt) stil_stdout_get((a_stilt)->stil)
#define	stilt_stderr_get(a_stilt) stil_stderr_get((a_stilt)->stil)

#define	stilt_ostack_get(a_stilt) (&((a_stilt)->ostack))
#define	stilt_dstack_get(a_stilt) (&((a_stilt)->dstack))
#define	stilt_estack_get(a_stilt) (&((a_stilt)->estack))
#define	stilt_tstack_get(a_stilt) (&((a_stilt)->tstack))

#define	stilt_threaddict_get(a_stilt) (&((a_stilt)->threaddict))
#define	stilt_systemdict_get(a_stilt)					\
	(stil_systemdict_get((a_stilt)->stil))
#define	stilt_globaldict_get(a_stilt)					\
	(stil_globaldict_get((a_stilt)->stil))
#define	stilt_userdict_get(a_stilt) (&((a_stilt)->userdict))
#define	stilt_errordict_get(a_stilt) (&((a_stilt)->errordict))
#define	stilt_derror_get(a_stilt) (&((a_stilt)->derror))

#define	stilt_error_get(a_stilt) (a_stilt)->error
#define	stilt_error_set(a_stilt, a_stilte) (a_stilt)->error = (a_stilte)

/*
 * If TRUE, allocation for the stilt is global.  Otherwise, allocation is
 * local.
 */
#define	stilt_currentglobal(a_stilt)					\
	(a_stilt)->global
#define	stilt_setglobal(a_stilt, a_global) do {				\
	(a_stilt)->global = (a_global);					\
} while (0);

#define	stilt_chi_get(a_stilt)						\
	stila_chi_get(stil_stila_get((a_stilt)->stil))
#define	stilt_chi_put(a_stilt, a_chi)					\
	stila_chi_put(stil_stila_get((a_stilt)->stil), (a_chi))

#define	stilt_stilsc_get(a_stilt)					\
	stila_stilsc_get(stil_stila_get((a_stilt)->stil))
#define	stilt_stilsc_put(a_stilt, a_stilsc)				\
	stila_stilsc_put(stil_stila_get((a_stilt)->stil), (a_stilsc))

#define	stilt_dicto_get(a_stilt)					\
	stila_dicto_get(stil_stila_get((a_stilt)->stil))
#define	stilt_dicto_put(a_stilt, a_dicto)				\
	stila_dicto_put(stil_stila_get((a_stilt)->stil), (a_dicto))
