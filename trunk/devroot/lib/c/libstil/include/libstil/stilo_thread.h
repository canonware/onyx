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
#define	_CW_STILO_THREAD_BUFFER_SIZE	  8
#else
#define	_CW_STILO_THREAD_BUFFER_SIZE	256
#endif

typedef struct cw_stilo_thread_entry_s cw_stilo_thread_entry_t;
typedef struct cw_stilo_threadp_s cw_stilo_threadp_t;
typedef struct cw_stiloe_thread_s cw_stiloe_thread_t;

struct cw_stilo_thread_entry_s {
	cw_stilo_t	*thread;
	cw_thd_t	*thd;
	cw_mtx_t	lock;
	cw_cnd_t	done_cnd;
	cw_cnd_t	join_cnd;
	cw_bool_t	done:1;
	cw_bool_t	gone:1;
	cw_bool_t	detached:1;
	cw_bool_t	joined:1;
};

typedef struct cw_stiloe_threadts_s cw_stiloe_threadts_t;
typedef enum {
	THREADTS_START,
	THREADTS_LT_CONT,
	THREADTS_GT_CONT,
	THREADTS_RB_CONT,
	THREADTS_SLASH_CONT,
	THREADTS_COMMENT,
	THREADTS_INTEGER,
	THREADTS_INTEGER_RADIX,
	THREADTS_ASCII_STRING,
	THREADTS_ASCII_STRING_NEWLINE_CONT,
	THREADTS_ASCII_STRING_PROT_CONT,
	THREADTS_ASCII_STRING_CRLF_CONT,
	THREADTS_ASCII_STRING_HEX_CONT,
	THREADTS_ASCII_STRING_HEX_FINISH,
	THREADTS_LIT_STRING,
	THREADTS_LIT_STRING_NEWLINE_CONT,
	THREADTS_LIT_STRING_PROT_CONT,
	THREADTS_HEX_STRING,
	THREADTS_BASE64_STRING,
	THREADTS_BASE64_STRING_PAD,
	THREADTS_BASE64_STRING_TILDE,
	THREADTS_BASE64_STRING_FINISH,
	THREADTS_NAME
} cw_stilo_threadts_t;

struct cw_stilo_threadp_s {
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

struct cw_stiloe_thread_s {
	cw_stiloe_t	stiloe;

	/*
	 * stil this thread is part of.
	 */
	cw_stil_t	*stil;

	/*
	 * Self, for the self operator, and as a convenience in various places
	 * internally.
	 */
	cw_stilo_t	self;

	/*
	 * Used by stilo_thread_thread(), stilo_thread_detach(), and
	 * stilo_thread_join().
	 */
	cw_stilo_thread_entry_t *entry;

	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;

	/*
	 * TRUE  : New array, dict, file, and string objects are implicitly
	 *         locked.
	 * FALSE : No implicit locking for new objects.
	 */
	cw_bool_t	locking:1;

	/*
	 * Stacks.
	 */
	cw_stilo_t	estack;	/* Execution stack. */
	cw_stilo_t	ostack;	/* Operand stack. */
	cw_stilo_t	dstack;	/* Dictionary stack. */
	cw_stilo_t	tstack;	/* Temp stack. */

	/*
	 * Local dictionaries.
	 */
	cw_stilo_t	currenterror;
	cw_stilo_t	errordict;
	cw_stilo_t	userdict;
	cw_stilo_t	threaddict;

	/*
	 * Tokenizer state.  If a token is broken across two or more input
	 * strings, data are copied to an internal buffer, and state machine
	 * state is preserved so that the buffered data need not be processed
	 * again.
	 */

	/* Current scanner state. */
	cw_stilo_threadts_t state;

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
	 * Pointer to the token buffer.  As long as index is less than
	 * _CW_STILO_THREAD_BUFFER_SIZE, tok_str actually points to buffer.
	 * Otherwise, adequate space is allocated (using exponential doubling),
	 * and the contents of tok_buffer are copied to the allocated buffer.
	 *
	 * If a temporary buffer is allocated, it is discarded as soon as the
	 * token is handled.  That is, tok_buffer is used for every token until
	 * (if) tok_buffer overflows.
	 */
	cw_uint8_t	*tok_str;
	cw_uint32_t	buffer_len;	/* Only valid if buffer overflowed. */
	cw_uint8_t	buffer[_CW_STILO_THREAD_BUFFER_SIZE];

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
 * stilo_threade.
 */
cw_stiln_t stilo_threade_stiln(cw_stilo_threade_t a_threade);

/*
 * stilo_threadp.
 */
cw_stilo_threadp_t *stilo_threadp_new(cw_stilo_threadp_t *a_threadp);
void	stilo_threadp_delete(cw_stilo_threadp_t *a_threadp, cw_stilo_t
    *a_thread);
void	stilo_threadp_position_get(cw_stilo_threadp_t *a_threadp, cw_uint32_t
    *r_line, cw_uint32_t *r_column);
void	stilo_threadp_position_set(cw_stilo_threadp_t *a_threadp, cw_uint32_t
    a_line, cw_uint32_t a_column);

/*
 * stilo_thread.
 */
void	stilo_thread_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil);
void	stilo_thread_start(cw_stilo_t *a_stilo);
void	stilo_thread_exit(cw_stilo_t *a_stilo);

void	stilo_thread_self(cw_stilo_t *a_stilo, cw_stilo_t *r_self);
void	stilo_thread_thread(cw_stilo_t *a_stilo);
void	stilo_thread_detach(cw_stilo_t *a_stilo);
void	stilo_thread_join(cw_stilo_t *a_stilo);

cw_stilo_threadts_t stilo_thread_state(cw_stilo_t *a_stilo);
cw_bool_t stilo_thread_deferred(cw_stilo_t *a_stilo);

void	stilo_thread_reset(cw_stilo_t *a_stilo);
void	stilo_thread_loop(cw_stilo_t *a_stilo);

void	stilo_thread_interpret(cw_stilo_t *a_stilo, cw_stilo_threadp_t
    *a_threadp, const cw_uint8_t *a_str, cw_uint32_t a_len);
void	stilo_thread_flush(cw_stilo_t *a_stilo, cw_stilo_threadp_t *a_threadp);
cw_uint32_t stilo_thread_token(cw_stilo_t *a_stilo, cw_stilo_threadp_t
    *a_threadp, const cw_uint8_t *a_str, cw_uint32_t a_len);
void	stilo_thread_error(cw_stilo_t *a_stilo, cw_stilo_threade_t a_threade);

cw_bool_t stilo_thread_dstack_search(cw_stilo_t *a_stilo, cw_stilo_t *a_key,
    cw_stilo_t *r_value);

#ifndef _CW_USE_INLINES
cw_stil_t *stilo_thread_stil_get(cw_stilo_t *a_stilo);

cw_stilo_t *stilo_thread_ostack_get(cw_stilo_t *a_stilo);
cw_stilo_t *stilo_thread_dstack_get(cw_stilo_t *a_stilo);
cw_stilo_t *stilo_thread_estack_get(cw_stilo_t *a_stilo);
cw_stilo_t *stilo_thread_tstack_get(cw_stilo_t *a_stilo);
#endif

cw_stilo_t *stilo_thread_threaddict_get(cw_stilo_t *a_stilo);
cw_stilo_t *stilo_thread_userdict_get(cw_stilo_t *a_stilo);
cw_stilo_t *stilo_thread_errordict_get(cw_stilo_t *a_stilo);
cw_stilo_t *stilo_thread_currenterror_get(cw_stilo_t *a_stilo);

cw_bool_t stilo_thread_currentlocking(cw_stilo_t *a_stilo);
void	stilo_thread_setlocking(cw_stilo_t *a_stilo, cw_bool_t a_locking);

#if (defined(_CW_USE_INLINES) || defined(_STILO_THREAD_C_))
_CW_INLINE cw_stil_t *
stilo_thread_stil_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return thread->stil;
}

_CW_INLINE cw_stilo_t *
stilo_thread_ostack_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return &thread->ostack;
}

_CW_INLINE cw_stilo_t *
stilo_thread_dstack_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return &thread->dstack;
}

_CW_INLINE cw_stilo_t *
stilo_thread_estack_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return &thread->estack;
}

_CW_INLINE cw_stilo_t *
stilo_thread_tstack_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return &thread->tstack;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_STILO_THREAD_C_)) */
