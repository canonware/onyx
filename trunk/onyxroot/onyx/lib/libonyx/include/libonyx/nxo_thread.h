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

#ifdef CW_DBG
#define	CW_NXO_THREAD_BUFFER_SIZE	  8
#else
#define	CW_NXO_THREAD_BUFFER_SIZE	256
#endif

typedef struct cw_nxo_threadp_s cw_nxo_threadp_t;
typedef struct cw_nxoe_thread_s cw_nxoe_thread_t;

typedef struct cw_nxoe_threadts_s cw_nxoe_threadts_t;
typedef enum {
	THREADTS_START,
	THREADTS_SLASH_CONT,
	THREADTS_COMMENT,
	THREADTS_INTEGER,
	THREADTS_INTEGER_RADIX,
	THREADTS_STRING,
	THREADTS_STRING_NEWLINE_CONT,
	THREADTS_STRING_PROT_CONT,
	THREADTS_STRING_CRLF_CONT,
	THREADTS_STRING_HEX_CONT,
	THREADTS_STRING_HEX_FINISH,
	THREADTS_NAME
} cw_nxo_threadts_t;

struct cw_nxo_threadp_s {
#ifdef CW_DBG
	cw_uint32_t	magic;
#endif
	/*
	 * Current line number (counting starts at 1 by convention) and column
	 * number (counting starts at 0).
	 */
	cw_uint32_t	line;
	cw_sint32_t	column;
};

struct cw_nxoe_thread_s {
	cw_nxoe_t	nxoe;

	/*
	 * nx this thread is part of.
	 */
	cw_nx_t		*nx;

	/*
	 * Self, for the self operator, and as a convenience in various places
	 * internally.
	 */
	cw_nxo_t	self;

#ifdef CW_THREADS
	/*
	 * Used by nxo_thread_thread(), nxo_thread_detach(), and
	 * nxo_thread_join().
	 */
	cw_thd_t	*thd;
	cw_mtx_t	lock;
	cw_cnd_t	done_cnd;
	cw_cnd_t	join_cnd;
	cw_bool_t	done:1;
	cw_bool_t	gone:1;
	cw_bool_t	detached:1;
	cw_bool_t	joined:1;
#endif

	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;

#ifdef CW_THREADS
	/*
	 * TRUE  : New array, dict, file, and string objects are implicitly
	 *         locked.
	 * FALSE : No implicit locking for new objects.
	 */
	cw_bool_t	locking:1;
#endif

	/*
	 * Stacks.
	 */
	cw_nxo_t	estack;	/* Execution stack. */
	cw_nxo_t	istack;	/* Execution index stack. */
	cw_nxo_t	ostack;	/* Operand stack. */
	cw_nxo_t	dstack;	/* Dictionary stack. */
	cw_nxo_t	tstack;	/* Temp stack. */

	/*
	 * Files.
	 */
	cw_nxo_t	stdin_nxo;
	cw_nxo_t	stdout_nxo;
	cw_nxo_t	stderr_nxo;

	/*
	 * Tokenizer state.  If a token is broken across two or more input
	 * strings, data are copied to an internal buffer, and state machine
	 * state is preserved so that the buffered data need not be processed
	 * again.
	 */

	/* Current scanner state. */
	cw_nxo_threadts_t state;

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
	 * CW_NXO_THREAD_BUFFER_SIZE, tok_str actually points to buffer.
	 * Otherwise, adequate space is allocated (using exponential doubling),
	 * and the contents of tok_buffer are copied to the allocated buffer.
	 *
	 * If a temporary buffer is allocated, it is discarded as soon as the
	 * token is handled.  That is, tok_buffer is used for every token until
	 * (if) tok_buffer overflows.
	 */
	cw_uint8_t	*tok_str;
	cw_uint32_t	buffer_len;	/* Only valid if buffer overflowed. */
	cw_uint8_t	buffer[CW_NXO_THREAD_BUFFER_SIZE];

	union {
		struct {
			cw_sint32_t	sign;	/* -1, 0 (not specified), 1. */
			cw_uint32_t	base;
			cw_uint32_t	b_off;	/* Depends on sign, radix. */
		}       n;	/* number. */
		struct {
			cw_uint32_t	q_depth;
			cw_uint8_t	hex_val;
		}	s;	/* string. */
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
 * nxo_threadp.
 */
void	nxo_threadp_new(cw_nxo_threadp_t *a_threadp);
void	nxo_threadp_delete(cw_nxo_threadp_t *a_threadp, cw_nxo_t *a_thread);
void	nxo_threadp_position_get(cw_nxo_threadp_t *a_threadp, cw_uint32_t
    *r_line, cw_uint32_t *r_column);
void	nxo_threadp_position_set(cw_nxo_threadp_t *a_threadp, cw_uint32_t
    a_line, cw_uint32_t a_column);

/*
 * nxo_thread.
 */
void	nxo_thread_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx);
void	nxo_thread_start(cw_nxo_t *a_nxo);
void	nxo_thread_exit(cw_nxo_t *a_nxo);

#ifdef CW_THREADS
void	nxo_thread_thread(cw_nxo_t *a_nxo);
void	nxo_thread_detach(cw_nxo_t *a_nxo);
void	nxo_thread_join(cw_nxo_t *a_nxo);
#endif

cw_nxo_threadts_t nxo_thread_state(cw_nxo_t *a_nxo);
cw_bool_t nxo_thread_deferred(cw_nxo_t *a_nxo);

void	nxo_thread_reset(cw_nxo_t *a_nxo);
void	nxo_thread_loop(cw_nxo_t *a_nxo);

void	nxo_thread_interpret(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp, const
    cw_uint8_t *a_str, cw_uint32_t a_len);
void	nxo_thread_flush(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp);
void	nxo_thread_nerror(cw_nxo_t *a_nxo, cw_nxn_t a_nxn);
void	nxo_thread_serror(cw_nxo_t *a_nxo, const cw_uint8_t *a_str, cw_uint32_t
    a_len);

cw_bool_t nxo_thread_dstack_search(cw_nxo_t *a_nxo, cw_nxo_t *a_key, cw_nxo_t
    *r_value);

#ifdef CW_THREADS
cw_bool_t nxo_thread_currentlocking(cw_nxo_t *a_nxo);
void	nxo_thread_setlocking(cw_nxo_t *a_nxo, cw_bool_t a_locking);
#else
#define	nxo_thread_currentlocking(a_nxo)	FALSE
#endif

#ifndef CW_USE_INLINES
cw_nx_t *nxo_thread_nx_get(cw_nxo_t *a_nxo);

cw_nxo_t *nxo_thread_ostack_get(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_thread_dstack_get(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_thread_estack_get(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_thread_istack_get(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_thread_tstack_get(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_thread_stdin_get(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_thread_stdout_get(cw_nxo_t *a_nxo);
cw_nxo_t *nxo_thread_stderr_get(cw_nxo_t *a_nxo);
#endif

#if (defined(CW_USE_INLINES) || defined(_NXO_THREAD_C_))
CW_INLINE cw_nx_t *
nxo_thread_nx_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return thread->nx;
}

CW_INLINE cw_nxo_t *
nxo_thread_ostack_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->ostack;
}

CW_INLINE cw_nxo_t *
nxo_thread_dstack_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->dstack;
}

CW_INLINE cw_nxo_t *
nxo_thread_estack_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->estack;
}

CW_INLINE cw_nxo_t *
nxo_thread_istack_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->istack;
}

CW_INLINE cw_nxo_t *
nxo_thread_tstack_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->tstack;
}

CW_INLINE cw_nxo_t *
nxo_thread_stdin_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->stdin_nxo;
}

CW_INLINE cw_nxo_t *
nxo_thread_stdout_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->stdout_nxo;
}

CW_INLINE cw_nxo_t *
nxo_thread_stderr_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->stderr_nxo;
}
#endif	/* (defined(CW_USE_INLINES) || defined(_NXO_THREAD_C_)) */
