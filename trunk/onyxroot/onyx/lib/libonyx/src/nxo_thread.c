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

#define _THREAD_C_

#include "../include/libonyx/libonyx.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "../include/libonyx/currenterror_l.h"
#include "../include/libonyx/errordict_l.h"
#include "../include/libonyx/threaddict_l.h"
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_array_l.h"
#include "../include/libonyx/nxo_operator_l.h"
#include "../include/libonyx/nxo_thread_l.h"

cw_nxn_t
nxo_threade_nxn(cw_nxo_threade_t a_threade)
{
	static const cw_nxn_t threade_nxn[] = {
		0,
		NXN_dstackunderflow,
		NXN_estackoverflow,
		NXN_interrupt,
		NXN_invalidaccess,
		NXN_invalidcontext,
		NXN_invalidexit,
		NXN_invalidfileaccess,
		NXN_ioerror,
		NXN_limitcheck,
		NXN_rangecheck,
		NXN_stackunderflow,
		NXN_syntaxerror,
		NXN_timeout,
		NXN_typecheck,
		NXN_undefined,
		NXN_undefinedfilename,
		NXN_undefinedresult,
		NXN_unmatchedfino,
		NXN_unmatchedmark,
		NXN_unregistered
	};
	_cw_assert(sizeof(threade_nxn) / sizeof(cw_nxn_t) == NXO_THREADE_LAST +
	    1);
	_cw_assert(a_threade > 0 && a_threade <= NXO_THREADE_LAST);
	return threade_nxn[a_threade];
}

#define _CW_NXO_THREAD_GETC(a_i)					\
	a_thread->tok_str[(a_i)]

#define _CW_NXO_THREAD_PUTC(a_c)					\
	do {								\
		if (a_thread->index >= _CW_NXO_THREAD_BUFFER_SIZE)	\
			nxoe_p_thread_tok_str_expand(a_thread);		\
		a_thread->tok_str[a_thread->index] = (a_c);		\
		a_thread->index++;					\
	} while (0)

/*
 * Make a note that a '\n' was just seen.  The line and column counters will be
 * updated before the next character is seen.
 */
#define _CW_NXO_THREAD_NEWLINE()					\
		newline = 1

static void	nxo_p_thread_join(cw_nxoe_thread_t *a_nxoe);
static cw_uint32_t nxoe_p_thread_feed(cw_nxoe_thread_t *a_thread,
    cw_nxo_threadp_t *a_threadp, cw_uint32_t a_token, const cw_uint8_t *a_str,
    cw_uint32_t a_len);
static void	nxoe_p_thread_tok_str_expand(cw_nxoe_thread_t *a_thread);
static void	nxoe_p_thread_syntax_error(cw_nxoe_thread_t *a_thread,
    cw_nxo_threadp_t *a_threadp, cw_uint8_t *a_prefix, cw_uint8_t *a_suffix,
    cw_uint8_t a_c);
static void	nxoe_p_thread_reset(cw_nxoe_thread_t *a_thread);
static void	nxoe_p_thread_procedure_accept(cw_nxoe_thread_t *a_thread);
static void	nxoe_p_thread_name_accept(cw_nxoe_thread_t *a_thread);

#ifdef _LIBONYX_DBG
#define _CW_NXO_THREADP_MAGIC 0xdfe76a68
#endif

/*
 * nxo_threadp.
 */
cw_nxo_threadp_t *
nxo_threadp_new(cw_nxo_threadp_t *a_threadp)
{
	cw_nxo_threadp_t	*retval;

	if (a_threadp != NULL) {
		retval = a_threadp;
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_nxo_threadp_t
		    *)_cw_malloc(sizeof(cw_nxo_threadp_t));
		retval->is_malloced = TRUE;
	}

	retval->line = 1;
	retval->column = 0;

#ifdef _LIBONYX_DBG
	retval->magic = _CW_NXO_THREADP_MAGIC;
#endif
	return retval;
}

void
nxo_threadp_delete(cw_nxo_threadp_t *a_threadp, cw_nxo_t *a_thread)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_thread);
	_cw_assert(a_thread->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_thread->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	if (thread->state != THREADTS_START) {
		/*
		 * It's possible that the last token seen hasn't been accepted
		 * yet.  Reset the internal state so that this won't screw
		 * things up later.
		 */
		nxoe_p_thread_reset(thread);
	}

	if (a_threadp->is_malloced)
		_cw_free(a_threadp);
#ifdef _LIBONYX_DBG
	else
		memset(a_threadp, 0x5a, sizeof(cw_nxo_threadp_t));
#endif
}

void
nxo_threadp_position_get(cw_nxo_threadp_t *a_threadp, cw_uint32_t *r_line,
    cw_uint32_t *r_column)
{
	_cw_check_ptr(a_threadp);
	_cw_assert(a_threadp->magic == _CW_NXO_THREADP_MAGIC);

	*r_line = a_threadp->line;
	*r_column = a_threadp->column;
}

void
nxo_threadp_position_set(cw_nxo_threadp_t *a_threadp, cw_uint32_t a_line,
    cw_uint32_t a_column)
{
	_cw_check_ptr(a_threadp);
	_cw_assert(a_threadp->magic == _CW_NXO_THREADP_MAGIC);

	a_threadp->line = a_line;
	a_threadp->column = a_column;
}

/*
 * nxo_thread.
 */
void
nxo_thread_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx)
{
	cw_nxoe_thread_t	*thread;
	cw_nxo_t		*nxo;

	thread = (cw_nxoe_thread_t *)_cw_malloc(sizeof(cw_nxoe_thread_t));
	memset(thread, 0, sizeof(cw_nxoe_thread_t));

	nxoe_l_new(&thread->nxoe, NXOT_THREAD, FALSE);

	/*
	 * Set things to a state that won't cause the GC (or any thread-related
	 * operators) to puke.
	 */

	/* Fake up a nxo for self. */
	nxo_l_new(&thread->self, NXOT_THREAD);
	thread->self.o.nxoe = (cw_nxoe_t *)thread;

	thread->nx = a_nx;
	thread->tok_str = thread->buffer;

	nxo_no_new(&thread->estack);
	nxo_no_new(&thread->ostack);
	nxo_no_new(&thread->dstack);
	nxo_no_new(&thread->tstack);
	nxo_no_new(&thread->currenterror);
	nxo_no_new(&thread->errordict);
	nxo_no_new(&thread->userdict);
	nxo_no_new(&thread->threaddict);

	/*
	 * Register this thread with the interpreter so that the GC will be able
	 * to get to it.
	 */
	nx_l_thread_insert(a_nx, &thread->self);

	/*
	 * Register with the GC so that this thread will be iterated on during
	 * GC.
	 */
	memset(a_nxo, 0, sizeof(cw_nxo_t));
	a_nxo->o.nxoe = (cw_nxoe_t *)thread;
#ifdef _LIBONYX_DBG
	a_nxo->magic = _CW_NXO_MAGIC;
#endif
	a_nxo->type = NXOT_THREAD;

	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)thread);

	/*
	 * Finish setting up the internals.
	 */
	nxo_stack_new(&thread->estack, a_nx, FALSE);
	nxo_stack_new(&thread->ostack, a_nx, FALSE);
	nxo_stack_new(&thread->dstack, a_nx, FALSE);
	nxo_stack_new(&thread->tstack, a_nx, FALSE);
	
	currenterror_l_populate(&thread->currenterror, a_nxo);
	errordict_l_populate(&thread->errordict, a_nxo);
	nxo_dict_new(&thread->userdict, a_nx, FALSE,
	    _LIBSTASH_USERDICT_HASH);
	threaddict_l_populate(&thread->threaddict, a_nxo);

	/*
	 * Push threaddict, systemdict, globaldict, and userdict onto
	 * the dictionary stack.
	 */
	nxo = nxo_stack_push(&thread->dstack);
	nxo_dup(nxo, &thread->threaddict);

	nxo = nxo_stack_push(&thread->dstack);
	nxo_dup(nxo, nx_systemdict_get(a_nx));

	nxo = nxo_stack_push(&thread->dstack);
	nxo_dup(nxo, nx_globaldict_get(a_nx));

	nxo = nxo_stack_push(&thread->dstack);
	nxo_dup(nxo, &thread->userdict);

	/* Execute the thread initialization hook if set. */
	if (nx_l_thread_init(a_nx) != NULL)
		nx_l_thread_init(a_nx)(&thread->self);
}

void
nxoe_l_thread_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx)
{
	cw_nxoe_thread_t	*thread;

	thread = (cw_nxoe_thread_t *)a_nxoe;

	_cw_check_ptr(thread);
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	if (thread->tok_str != thread->buffer) {
		/*
		 * This shouldn't happen, since it indicates that there is an
		 * unaccepted token.  However, it's really the caller's fault,
		 * so just clean up.
		 */
		_cw_free(thread->tok_str);
	}

	if (thread->entry != NULL) {
		/*
		 * The thread wasn't joined or detached.  This will never happen
		 * except at interpreter shutdown, so we can safely join the
		 * thread to clean things up.
		 */
		nxo_p_thread_join(thread);
	}
	_CW_NXOE_FREE(thread);
}

cw_nxoe_t *
nxoe_l_thread_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	cw_nxoe_t		*retval;
	cw_nxoe_thread_t	*thread;

	thread = (cw_nxoe_thread_t *)a_nxoe;

	if (a_reset)
		thread->ref_iter = 0;

	for (retval = NULL; retval == NULL; thread->ref_iter++) {
		switch (thread->ref_iter) {
		case 0:
			retval = nxo_nxoe_get(&thread->estack);
			break;
		case 1:
			retval = nxo_nxoe_get(&thread->ostack);
			break;
		case 2:
			retval = nxo_nxoe_get(&thread->dstack);
			break;
		case 3:
			retval = nxo_nxoe_get(&thread->tstack);
			break;
		case 4:
			retval = nxo_nxoe_get(&thread->currenterror);
			break;
		case 5:
			retval = nxo_nxoe_get(&thread->errordict);
			break;
		case 6:
			retval = nxo_nxoe_get(&thread->userdict);
			break;
		case 7:
			retval = nxo_nxoe_get(&thread->threaddict);
			break;
		default:
			retval = NULL;
			goto RETURN;
		}
	}

	RETURN:
	return retval;
}

void
nxo_l_thread_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *thread, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(thread, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(thread)
	    != NXOT_THREAD) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nx_stdout_get(nxo_thread_nx_get(a_thread));

	error = nxo_file_output(stdout_nxo, "-thread-");

	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
nxo_thread_start(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;
	cw_nxo_t		*start;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	start = nxo_stack_push(&thread->estack);
	nxo_operator_new(start, systemdict_start, NXN_start);
	nxo_attrs_set(start, NXOA_EXECUTABLE);

	nxo_thread_loop(a_nxo);
}

void
nxo_thread_exit(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	nx_l_thread_remove(thread->nx, &thread->self);
}

void
nxo_thread_self(cw_nxo_t *a_nxo, cw_nxo_t *r_self)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	_cw_check_ptr(r_self);

	nxo_dup(r_self, &thread->self);
}

static void *
nxo_p_thread_entry(void *a_arg)
{
	cw_nxo_thread_entry_t	*arg = (cw_nxo_thread_entry_t *)a_arg;

	/* Run. */
	nxo_thread_start(arg->thread);

	/* Wait to be joined or detated, if not already so. */
	mtx_lock(&arg->lock);
	arg->done = TRUE;
	while (arg->detached == FALSE && arg->joined == FALSE) {
		cnd_wait(&arg->done_cnd, &arg->lock);
	}
	if (arg->detached) {
		mtx_unlock(&arg->lock);

		/* Clean up. */
		cnd_delete(&arg->join_cnd);
		cnd_delete(&arg->done_cnd);
		mtx_delete(&arg->lock);
		nx_l_thread_remove(nxo_thread_nx_get(arg->thread),
		    arg->thread);
		thd_delete(arg->thd);
		_cw_free(arg);
	} else if (arg->joined) {
		/* Wake the joiner back up. */
		cnd_signal(&arg->join_cnd);
		/* We're done.  The joiner will clean up. */
		arg->gone = TRUE;
		mtx_unlock(&arg->lock);
	} else
		_cw_not_reached();

	return NULL;
}

void
nxo_thread_thread(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;
	sigset_t		sig_mask, old_mask;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	thread->entry = (cw_nxo_thread_entry_t
	    *)_cw_malloc(sizeof(cw_nxo_thread_entry_t));

	thread->entry->thread = a_nxo;
	mtx_new(&thread->entry->lock);
	cnd_new(&thread->entry->done_cnd);
	cnd_new(&thread->entry->join_cnd);
	thread->entry->done = FALSE;
	thread->entry->gone = FALSE;
	thread->entry->detached = FALSE;
	thread->entry->joined = FALSE;

	/*
	 * Block all signals during thread creation, so that the thread doesn't
	 * swallow signals.  Doing this here rather than in the new thread
	 * itself avoids a race condition where signals can be delivered to the
	 * new thread.
	 */
	sigfillset(&sig_mask);
	thd_sigmask(SIG_BLOCK, &sig_mask, &old_mask);
	thread->entry->thd = thd_new(nxo_p_thread_entry, (void
	    *)thread->entry, TRUE);
	thd_sigmask(SIG_SETMASK, &old_mask, NULL);
}

void
nxo_thread_detach(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	_cw_check_ptr(thread->entry);

	mtx_lock(&thread->entry->lock);
	thread->entry->detached = TRUE;
	if (thread->entry->done) {
		/* The thread is already done, so wake it back up. */
		cnd_signal(&thread->entry->done_cnd);
	}
	mtx_unlock(&thread->entry->lock);
}

static void
nxo_p_thread_join(cw_nxoe_thread_t *a_nxoe)
{
	mtx_lock(&a_nxoe->entry->lock);
	a_nxoe->entry->joined = TRUE;
	if (a_nxoe->entry->done) {
		/* The thread is already done, so wake it back up. */
		cnd_signal(&a_nxoe->entry->done_cnd);
	}
	/* Wait for the thread to totally go away. */
	while (a_nxoe->entry->gone == FALSE)
		cnd_wait(&a_nxoe->entry->join_cnd, &a_nxoe->entry->lock);
	mtx_unlock(&a_nxoe->entry->lock);

	/* Clean up. */
	cnd_delete(&a_nxoe->entry->join_cnd);
	cnd_delete(&a_nxoe->entry->done_cnd);
	mtx_delete(&a_nxoe->entry->lock);
	thd_join(a_nxoe->entry->thd);
	_cw_free(a_nxoe->entry);
	a_nxoe->entry = NULL;
}

void
nxo_thread_join(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	_cw_check_ptr(thread->entry);

	nxo_p_thread_join(thread);
	nx_l_thread_remove(nxo_thread_nx_get(a_nxo), a_nxo);
}

cw_nxo_threadts_t
nxo_thread_state(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	return thread->state;
}

cw_bool_t
nxo_thread_deferred(cw_nxo_t *a_nxo)
{
	cw_bool_t		retval;
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	if (thread->defer_count != 0)
		retval = TRUE;
	else
		retval = FALSE;

	return retval;
}

void
nxo_thread_reset(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	thread->defer_count = 0;
	nxoe_p_thread_reset(thread);
}

void
nxo_thread_loop(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;
	cw_nxo_t		*nxo, *tnxo;
	cw_uint32_t		sdepth, cdepth;
#ifdef _LIBONYX_DBG
	cw_uint32_t		tdepth;
#endif

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

#ifdef _LIBONYX_DBG
	/*
	 * The assertions about stack depth in this function check for tstack
	 * leaks in operators.
	 */
	tdepth = nxo_stack_count(&thread->tstack);
#endif

	for (sdepth = cdepth = nxo_stack_count(&thread->estack);
	     cdepth >= sdepth; cdepth = nxo_stack_count(&thread->estack)) {
		if (cdepth == _LIBONYX_ESTACK_MAX + 1) {
			nxo_thread_error(a_nxo,
			    NXO_THREADE_ESTACKOVERFLOW);
		}

		nxo = nxo_stack_get(&thread->estack);
		if (nxo_attrs_get(nxo) == NXOA_LITERAL) {
			/* Always push literal objects onto the data stack. */
			tnxo = nxo_stack_push(&thread->ostack);
			nxo_dup(tnxo, nxo);
			nxo_stack_pop(&thread->estack);
			continue;
		}

		switch (nxo_type_get(nxo)) {
		case NXOT_BOOLEAN:
		case NXOT_CONDITION:
		case NXOT_DICT:
		case NXOT_FINO:
		case NXOT_INTEGER:
		case NXOT_MARK:
		case NXOT_MUTEX:
		case NXOT_STACK:
		case NXOT_THREAD:
			/*
			 * Always push the object onto the data stack, even
			 * though it isn't literal.
			 */
			tnxo = nxo_stack_push(&thread->ostack);
			nxo_dup(tnxo, nxo);
			nxo_stack_pop(&thread->estack);
			break;
		case NXOT_NULL:
			/* Do nothing. */
			nxo_stack_pop(&thread->estack);
			break;
		case NXOT_ARRAY: {
			cw_uint32_t	i, len;
			cw_nxo_t	*el;

			len = nxo_array_len_get(nxo);
			if (len == 0) {
				nxo_stack_pop(&thread->estack);
				break;
			}

			/*
			 * Iterate through the array and execute each element in
			 * turn.  The generic algorithm is encapsulated in the
			 * last part of the if..else if..else statement, but the
			 * overhead of the pushing, recursion, and popping is
			 * excessive for the common cases of a simple object or
			 * operator.  Therefore, check for the most common
			 * simple cases and handle them specially.
			 */
			el = nxo_stack_push(&thread->tstack);
			for (i = 0; i < len - 1; i++) {
				nxo_l_array_el_get(nxo, i, el);
				if (nxo_attrs_get(el) == NXOA_LITERAL) {
					/*
					 * Always push literal objects onto the
					 * data stack.
					 */
					tnxo = nxo_stack_push(&thread->ostack);
					nxo_dup(tnxo, el);
					continue;
				}

				switch (nxo_type_get(el)) {
				case NXOT_ARRAY:
					/*
					 * Don't execute nested arrays.
					 */
					tnxo = nxo_stack_push(&thread->ostack);
					nxo_dup(tnxo, el);
					break;
				case NXOT_OPERATOR:
					if (nxo_l_operator_fast_op_get(el)
					    == FALSE) {
						nxo_operator_f(el)(a_nxo);
						break;
					}

					/* Fast operator. */
					switch (nxo_l_operator_fast_op_nxn(
					    el)) {
					case NXN_add:
						systemdict_inline_add(a_nxo);
						break;
					case NXN_dup:
						systemdict_inline_dup(a_nxo);
						break;
					case NXN_exch:
						systemdict_inline_exch(a_nxo);
						break;
					case NXN_index:
						systemdict_inline_index(a_nxo);
						break;
					case NXN_pop:
						systemdict_inline_pop(a_nxo);
						break;
					case NXN_roll:
						systemdict_inline_roll(a_nxo);
						break;
					default:
						   _cw_not_reached();
					}
					break;
				default:
					/*
					 * Not a simple common case, so use the
					 * generic algorithm.
					 */
					tnxo = nxo_stack_push(&thread->estack);
					nxo_dup(tnxo, el);
					nxo_thread_loop(a_nxo);
				}
				_cw_assert(nxo_stack_count(&thread->tstack) ==
				    tdepth + 1);
			}

			/*
			 * If recursion is possible and likely, make tail
			 * recursion safe by replacing the array with its last
			 * element before executing the last element.
			 */
			nxo_l_array_el_get(nxo, i, el);
			if ((nxo_attrs_get(el) == NXOA_LITERAL) ||
			    (nxo_type_get(el) == NXOT_ARRAY)) {
				/*
				 * Always push literal objects and nested arrays
				 * onto the data stack.
				 */
				tnxo = nxo_stack_push(&thread->ostack);
				nxo_dup(tnxo, el);
				nxo_stack_pop(&thread->estack);
			} else {
				/* Possible recursion. */
				nxo_dup(nxo, el);
			}
			nxo_stack_pop(&thread->tstack);
			break;
		}
		case NXOT_STRING: {
			cw_nxo_threadp_t	threadp;

			/*
			 * Use the string as a source of code.
			 */
			nxo_threadp_new(&threadp);
			nxo_string_lock(nxo);
			nxo_thread_interpret(a_nxo, &threadp,
			    nxo_string_get(nxo), nxo_string_len_get(nxo));
			nxo_string_unlock(nxo);
			nxo_thread_flush(a_nxo, &threadp);
			nxo_threadp_delete(&threadp, a_nxo);
			nxo_stack_pop(&thread->estack);

			break;
		}
		case NXOT_NAME: {
			cw_nxo_t	*name;

			/*
			 * Search for a value associated with the name in the
			 * dictionary stack, put it on the execution stack, and
			 * execute it.
			 */
			name = nxo_stack_push(&thread->tstack);
			nxo_dup(name, nxo);
			if (nxo_thread_dstack_search(a_nxo, name, nxo)) {
				nxo_thread_error(a_nxo, NXO_THREADE_UNDEFINED);
				nxo_stack_pop(&thread->estack);
			}
			nxo_stack_pop(&thread->tstack);
			break;
		}
		case NXOT_OPERATOR:
			if (nxo_l_operator_fast_op_get(nxo) == FALSE) {
				nxo_operator_f(nxo)(a_nxo);
				nxo_stack_pop(&thread->estack);
				break;
			}

			/* Fast operator. */
			switch (nxo_l_operator_fast_op_nxn(nxo)) {
			case NXN_add:
				systemdict_inline_add(a_nxo);
				break;
			case NXN_dup:
				systemdict_inline_dup(a_nxo);
				break;
			case NXN_exch:
				systemdict_inline_exch(a_nxo);
				break;
			case NXN_index:
				systemdict_inline_index(a_nxo);
				break;
			case NXN_pop:
				systemdict_inline_pop(a_nxo);
				break;
			case NXN_roll:
				systemdict_inline_roll(a_nxo);
				break;
			default:
				_cw_not_reached();
			}

			nxo_stack_pop(&thread->estack);
			break;
		case NXOT_FILE: {
			cw_nxo_threadp_t	threadp;
			cw_sint32_t	nread;
			cw_uint8_t	buffer[_LIBONYX_FILE_EVAL_READ_SIZE];

			nxo_threadp_new(&threadp);
			/*
			 * Read data from the file and interpret it until an EOF
			 * (0 byte read).
			 */
			for (nread = nxo_file_read(nxo,
			    _LIBONYX_FILE_EVAL_READ_SIZE, buffer); nread > 0;
			    nread = nxo_file_read(nxo,
			    _LIBONYX_FILE_EVAL_READ_SIZE, buffer)) {
				nxo_thread_interpret(a_nxo, &threadp, buffer,
				    nread);
			}
			nxo_thread_flush(a_nxo, &threadp);
			nxo_threadp_delete(&threadp, a_nxo);

			nxo_stack_pop(&thread->estack);
			break;
		}
		case NXOT_HOOK: {
			cw_nxo_threade_t	error;
			
			error = nxo_hook_eval(nxo, a_nxo);
			if (error)
				nxo_thread_error(a_nxo, error);

			nxo_stack_pop(&thread->estack);
			break;
		}
		default:
			_cw_not_reached();
		}
		_cw_assert(nxo_stack_count(&thread->tstack) == tdepth);
	}
}

void
nxo_thread_interpret(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp, const
    cw_uint8_t *a_str, cw_uint32_t a_len)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	nxoe_p_thread_feed(thread, a_threadp, 0, a_str, a_len);
}

void
nxo_thread_flush(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp)
{
	cw_nxoe_thread_t	*thread;
	static const cw_uint8_t	str[] = "\n";

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	nxoe_p_thread_feed(thread, a_threadp, 0, str, sizeof(str) - 1);
}

cw_uint32_t
nxo_thread_token(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp, const
    cw_uint8_t *a_str, cw_uint32_t a_len)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	return nxoe_p_thread_feed(thread, a_threadp, 1, a_str, a_len);
}

void
nxo_thread_error(cw_nxo_t *a_nxo, cw_nxo_threade_t a_error)
{
	cw_nxoe_thread_t	*thread;
	cw_nxo_t		*nxo, *errordict, *key, *handler;
	cw_nxn_t		nxn;
	cw_uint32_t		defer_count;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	/* Shut off deferral temporarily. */
	defer_count = thread->defer_count;
	thread->defer_count = 0;

	/*
	 * Get errordict.  We can't throw an undefined error here because it
	 * would go infinitely recursive.
	 */
	errordict = nxo_stack_push(&thread->tstack);
	key = nxo_stack_push(&thread->tstack);
	nxo_name_new(key, thread->nx, nxn_str(NXN_errordict),
	    nxn_len(NXN_errordict), TRUE);
	if (nxo_thread_dstack_search(a_nxo, key, errordict)) {
		/*
		 * We failed to find errordict.  Fall back to the one originally
		 * defined in the thread.
		 */
		nxo_dup(errordict, &thread->errordict);
	}

	/*
	 * Find handler corresponding to error.
	 */
	nxn = nxo_threade_nxn(a_error);
	nxo_name_new(key, thread->nx, nxn_str(nxn), nxn_len(nxn), TRUE);

	/*
	 * Push the object being executed onto ostack unless this is an
	 * interrupt or timeout.
	 */
	switch (a_error) {
	case NXO_THREADE_INTERRUPT:
	case NXO_THREADE_TIMEOUT:
		break;
	default:
		nxo = nxo_stack_push(&thread->ostack);
		nxo_dup(nxo, nxo_stack_get(&thread->estack));
	}

	/*
	 * Get the handler for this particular error and push it onto estack.
	 * We could potentially throw another error here without going
	 * infinitely recursive, but it's not worth the risk.  After all, the
	 * user has done some really hokey management of errordict if this
	 * happens.
	 */
	handler = nxo_stack_push(&thread->estack);
	if (nxo_dict_lookup(errordict, key, handler)) {
		/*
		 * Ignore the error, since the only alternative is to blow
		 * up (or potentially go infinitely recursive).
		 */
		nxo_stack_npop(&thread->tstack, 2);
		nxo_stack_pop(&thread->estack);
		goto IGNORE;
	}
	nxo_stack_npop(&thread->tstack, 2);

	/* Execute the handler. */
	nxo_thread_loop(a_nxo);

	IGNORE:
	/* Turn deferral back on. */
	thread->defer_count = defer_count;
}

cw_bool_t
nxo_thread_dstack_search(cw_nxo_t *a_nxo, cw_nxo_t *a_key, cw_nxo_t
    *r_value)
{
	cw_bool_t		retval;
	cw_nxoe_thread_t	*thread;
	cw_nxo_t		*dict;
	cw_uint32_t		i, depth;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	/*
	 * Iteratively search the dictionaries on the dictionary stack for
	 * a_key.
	 */
	for (i = 0, depth = nxo_stack_count(&thread->dstack), dict = NULL; i
	    < depth; i++) {
		dict = nxo_stack_down_get(&thread->dstack, dict);
		if (nxo_dict_lookup(dict, a_key, r_value) == FALSE) {
			/* Found. */
			retval = FALSE;
			goto RETURN;
		}
	}

	retval = TRUE;
	RETURN:
	return retval;
}

cw_nxo_t *
nxo_thread_threaddict_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->threaddict;
}

cw_nxo_t *
nxo_thread_userdict_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->userdict;
}

cw_nxo_t *
nxo_thread_errordict_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->errordict;
}

cw_nxo_t *
nxo_thread_currenterror_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	return &thread->currenterror;
}

cw_bool_t
nxo_thread_currentlocking(cw_nxo_t *a_nxo)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	return thread->locking;
}

void
nxo_thread_setlocking(cw_nxo_t *a_nxo, cw_bool_t a_locking)
{
	cw_nxoe_thread_t	*thread;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);

	thread = (cw_nxoe_thread_t *)a_nxo->o.nxoe;
	_cw_assert(thread->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(thread->nxoe.type == NXOT_THREAD);

	thread->locking = a_locking;
}

static cw_uint32_t
nxoe_p_thread_feed(cw_nxoe_thread_t *a_thread, cw_nxo_threadp_t *a_threadp,
    cw_uint32_t a_token, const cw_uint8_t *a_str, cw_uint32_t a_len)
{
	cw_uint32_t	retval, i, newline;
	cw_uint8_t	c;
	cw_nxo_t	*nxo;
	cw_bool_t	token;

	if (a_token) {
		token = FALSE;
		a_thread->defer_count++;
	}

	/*
	 * All the grossness surrounding newline avoids doing any branches when
	 * calculating the line and column number.  This may be overzealous
	 * optimization, but the logic is relatively simple.  We do the update
	 * of both line and column number here so that they are correct at all
	 * times during the main part of the loop.
	 */
	for (i = newline = 0; i < a_len; i++, a_threadp->line += newline,
	    a_threadp->column = ((a_threadp->column + 1) * !newline), newline =
	    0) {
		c = a_str[i];

		/*
		 * If a special character causes the end of the previous token,
		 * the state machine builds the object, then restarts the state
		 * machine without incrementing the input character index.  This
		 * is done in order to avoid having to duplicate the
		 * THREADTS_START code.
		 */
		RESTART:

		switch (a_thread->state) {
		case THREADTS_START:
			_cw_assert(a_thread->index == 0);

			if (a_token) {
				/*
				 * token is TRUE if a token has been accepted.
				 * We look for the situation where token is TRUE
				 * and a_thread->defer_count is only 1
				 * (artificially raised).  If these conditions
				 * are met, then we've managed to scan an entire
				 * token, as defined by the token operator.
				 */
				if (token && a_thread->defer_count == 1) {
					/*
					 * Return the offset of the next
					 * character.
					 */
					retval = i;
					goto RETURN;
				}
			}

			switch (c) {
			case '`':
				a_thread->state = THREADTS_STRING;
				a_thread->m.s.q_depth = 1;
				break;
			case '\'':
				nxoe_p_thread_syntax_error(a_thread, a_threadp,
				    "", "", c);
				if (a_token)
					goto RETURN;
				break;
			case '<': case '>': case '(': case ')': case '[':
			case ']':
				_CW_NXO_THREAD_PUTC(c);
				token = TRUE;
				a_thread->m.m.action = ACTION_EXECUTE;
				nxoe_p_thread_name_accept(a_thread);
				break;
			case '{':
				a_thread->defer_count++;
				nxo = nxo_stack_push(&a_thread->ostack);
				/*
				 * Leave the nxo as notype in order to
				 * differentiate from normal marks.
				 */
				break;
			case '}':
				if (a_thread->defer_count > a_token) {
					token = TRUE;
					a_thread->defer_count--;
					nxoe_p_thread_procedure_accept(
					    a_thread);
				} else {
					/* Missing '{'. */
					nxoe_p_thread_syntax_error(a_thread,
					    a_threadp, "", "", c);
					if (a_token)
						goto RETURN;
				}
				break;
			case '/':
				a_thread->state = THREADTS_SLASH_CONT;
				break;
			case '%':
				a_thread->state = THREADTS_COMMENT;
				break;
			case '\n':
				_CW_NXO_THREAD_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* Swallow. */
				break;
			case '+':
				a_thread->state = THREADTS_INTEGER;
				a_thread->m.n.b_off = 1;
				_CW_NXO_THREAD_PUTC(c);
				break;
			case '-':
				a_thread->state = THREADTS_INTEGER;
				a_thread->m.n.b_off = 1;
				_CW_NXO_THREAD_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				a_thread->state = THREADTS_INTEGER;
				a_thread->m.n.b_off = 0;
				_CW_NXO_THREAD_PUTC(c);
				break;
			default:
				a_thread->state = THREADTS_NAME;
				a_thread->m.m.action = ACTION_EXECUTE;
				_CW_NXO_THREAD_PUTC(c);
				break;
			}
			break;
		case THREADTS_SLASH_CONT:
			_cw_assert(a_thread->index == 0);

			switch (c) {
			case '/':
				a_thread->state = THREADTS_NAME;
				a_thread->m.m.action = ACTION_EVALUATE;
				break;
			case '\n':
				_CW_NXO_THREAD_NEWLINE();

				nxoe_p_thread_syntax_error(a_thread, a_threadp,
				    "", "/", c);
				if (a_token)
					goto RETURN;
				break;
			case '\0': case '\t': case '\f': case '\r': case ' ':
			case '(': case ')': case '`': case '\'': case '<':
			case '>': case '[': case ']': case '{': case '}':
			case '%':
				nxoe_p_thread_syntax_error(a_thread, a_threadp,
				    "", "/", c);
				if (a_token)
					goto RETURN;
				break;
			default:
				a_thread->state = THREADTS_NAME;
				a_thread->m.m.action = ACTION_LITERAL;
				_CW_NXO_THREAD_PUTC(c);
				break;
			}
			break;
		case THREADTS_COMMENT:
			_cw_assert(a_thread->index == 0);

			switch (c) {
			case '\n':
				_CW_NXO_THREAD_NEWLINE();
				/* Fall through. */
			case '\f':
				/* Fall through. */
			case '\r':
				a_thread->state = THREADTS_START;
				break;
			default:
				break;
			}
			break;
		case THREADTS_INTEGER: {
			cw_bool_t	restart = FALSE;

			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				_CW_NXO_THREAD_PUTC(c);
				break;
			case '#': {
				cw_uint32_t	i, digit;

				/*
				 * Convert the string to a base (interpreted as
				 * base 10).
				 */
				a_thread->m.n.base = 0;

				for (i = 0; i < a_thread->index; i++) {
					digit = _CW_NXO_THREAD_GETC(
					    a_thread->m.n.b_off + i) - '0';

					if (a_thread->index -
					    a_thread->m.n.b_off - i == 2)
						digit *= 10;
					a_thread->m.n.base += digit;

					if (((digit != 0) && ((a_thread->index -
					    a_thread->m.n.b_off - i) > 2)) ||
					    (a_thread->m.n.base > 36)) {
						/*
						 * Base too large. Set base to 0
						 * so that the check for too
						 * small a base catches this.
						 */
						a_thread->m.n.base = 0;
						break;
					}
				}

				if (a_thread->m.n.base < 2) {
					/*
					 * Base too small (or too large, as
					 * detected in the for loop above).
					 */
					a_thread->state = THREADTS_NAME;
					a_thread->m.m.action = ACTION_EXECUTE;
				} else {
					a_thread->m.n.b_off = a_thread->index +
					    1;
					a_thread->state =
					    THREADTS_INTEGER_RADIX;
				}
				_CW_NXO_THREAD_PUTC(c);
				break;
			}
			case '\n':
				restart = TRUE; /* Inverted below. */
				_CW_NXO_THREAD_NEWLINE();
				/* Fall through. */
			case '(': case ')': case '`': case '\'': case '<':
			case '>': case '[': case ']': case '{': case '}':
			case '/': case '%':
				/* New token. */
				/*
				 * Invert, in case we fell through from
				 * above.
				 */
				restart = !restart;
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				if (a_thread->index > a_thread->m.n.b_off) {
					cw_nxoi_t	val;

					/* Integer. */

					/*
					 * Convert string to integer.  Do the
					 * conversion before mucking with the
					 * stack in case there is an exception.
					 */
					a_thread->tok_str[a_thread->index] =
					    '\0';
					errno = 0;

#if (_CW_NXOI_SIZEOF == 8)
					val = strtoll(a_thread->tok_str, NULL,
					    10);
#else
					val = strtol(a_thread->tok_str, NULL,
					    10);
#endif
					if ((errno == ERANGE) &&
#if (_CW_NXOI_SIZEOF == 8)
					    ((val == LLONG_MIN) || (val ==
					    LLONG_MAX))
#else
					    ((val == LONG_MIN) || (val ==
					    LONG_MAX))
#endif
					    ) {
						nxoe_p_thread_reset(a_thread);
						nxo_thread_error(
						    &a_thread->self,
						    NXO_THREADE_RANGECHECK);
					} else {
						token = TRUE;
						nxo = nxo_stack_push(
						    &a_thread->ostack);
						nxo_integer_new(nxo, val);
						nxoe_p_thread_reset(a_thread);
					}
				} else {
					/* No number specified, so a name. */
					token = TRUE;
					a_thread->m.m.action = ACTION_EXECUTE;
					nxoe_p_thread_name_accept(a_thread);
				}
				if (restart)
					goto RESTART;
				break;
			default:
				/* Not a number character. */
				a_thread->m.m.action = ACTION_EXECUTE;
				a_thread->state = THREADTS_NAME;
				_CW_NXO_THREAD_PUTC(c);
				break;
			}
			break;
		}
		case THREADTS_INTEGER_RADIX: {
			cw_bool_t	restart = FALSE;

			switch (c) {
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y':
			case 'z':
				if (a_thread->m.n.base <= (10 +
				    ((cw_uint32_t)(c - 'a')))) {
					/* Too big for this base. */
					a_thread->state = THREADTS_NAME;
					a_thread->m.m.action = ACTION_EXECUTE;
				}
				_CW_NXO_THREAD_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (a_thread->m.n.base <=
				    ((cw_uint32_t)(c - '0'))) {
					/* Too big for this base. */
					a_thread->state = THREADTS_NAME;
					a_thread->m.m.action = ACTION_EXECUTE;
				}
				_CW_NXO_THREAD_PUTC(c);
				break;
			case '\n':
				restart = TRUE; /* Inverted below. */
				_CW_NXO_THREAD_NEWLINE();
				/* Fall through. */
			case '(': case ')': case '`': case '\'': case '<':
			case '>': case '[': case ']': case '{': case '}':
			case '/': case '%':
				/* New token. */
				/*
				 * Invert, in case we fell through from
				 * above.
				 */
				restart = !restart;
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				if (a_thread->index > a_thread->m.n.b_off) {
					cw_nxoi_t	val;

					/* Integer. */

					/*
					 * Convert string to integer.  Do the
					 * conversion before mucking with the
					 * stack in case there is an exception.
					 */
					a_thread->tok_str[a_thread->index] =
					    '\0';
					errno = 0;

#if (_CW_NXOI_SIZEOF == 8)
					val = strtoll(&a_thread->tok_str[
					    a_thread->m.n.b_off], NULL,
					    a_thread->m.n.base);
#else
					val = strtol(&a_thread->tok_str[
					    a_thread->m.n.b_off], NULL,
					    a_thread->m.n.base);
#endif
					if ((errno == ERANGE) &&
#if (_CW_NXOI_SIZEOF == 8)
					    ((val == LLONG_MIN) || (val ==
					    LLONG_MAX))
#else
					    ((val == LONG_MIN) || (val ==
					    LONG_MAX))
#endif
					    ) {
						nxoe_p_thread_reset(a_thread);
						nxo_thread_error(
						    &a_thread->self,
						    NXO_THREADE_RANGECHECK);
					} else {
						token = TRUE;
						nxo = nxo_stack_push(
						    &a_thread->ostack);
						nxo_integer_new(nxo, val);
						nxoe_p_thread_reset(a_thread);
					}
				} else {
					/* No number specified, so a name. */
					token = TRUE;
					a_thread->m.m.action = ACTION_EXECUTE;
					nxoe_p_thread_name_accept(a_thread);
				}
				if (restart)
					goto RESTART;
				break;
			default:
				/* Not a number character. */
				a_thread->m.m.action = ACTION_EXECUTE;
				a_thread->state = THREADTS_NAME;
				_CW_NXO_THREAD_PUTC(c);
				break;
			}
			break;
		}
		case THREADTS_STRING:
			/* The CRLF code jumps here if there was no LF. */
			STRING_CONTINUE:

			switch (c) {
			case '\\':
				a_thread->state = THREADTS_STRING_PROT_CONT;
				break;
			case '`':
				a_thread->m.s.q_depth++;
				_CW_NXO_THREAD_PUTC(c);
				break;
			case '\'':
				a_thread->m.s.q_depth--;
				if (a_thread->m.s.q_depth == 0) {
					token = TRUE;
					nxo = nxo_stack_push(&a_thread->ostack);
					nxo_string_new(nxo, a_thread->nx,
					    a_thread->locking, a_thread->index);
					nxo_string_set(nxo, 0,
					    a_thread->tok_str, a_thread->index);

					nxoe_p_thread_reset(a_thread);
				} else
					_CW_NXO_THREAD_PUTC(c);
				break;
			case '\r':
				a_thread->state = THREADTS_STRING_NEWLINE_CONT;
				break;
			case '\n':
				_CW_NXO_THREAD_NEWLINE();
				/* Fall through. */
			default:
				_CW_NXO_THREAD_PUTC(c);
				break;
			}
			break;
		case THREADTS_STRING_NEWLINE_CONT:
			/* All cases in the switch statement do this. */
			_CW_NXO_THREAD_PUTC('\n');
			a_thread->state = THREADTS_STRING;
			switch (c) {
			case '\n':
				_CW_NXO_THREAD_NEWLINE();
				break;
			default:
				/*
				 * '\r' was not followed by a '\n'.  Translate
				 * the '\r' to a '\n' and jump back up to the
				 * string scanning state to scan c again.
				 */
				goto STRING_CONTINUE;
			}
			break;
		case THREADTS_STRING_PROT_CONT:
			switch (c) {
			case '`': case '\'': case '\\':
				a_thread->state = THREADTS_STRING;
				_CW_NXO_THREAD_PUTC(c);
				break;
			case 'n':
				a_thread->state = THREADTS_STRING;
				_CW_NXO_THREAD_PUTC('\n');
				break;
			case 'r':
				a_thread->state = THREADTS_STRING;
				_CW_NXO_THREAD_PUTC('\r');
				break;
			case 't':
				a_thread->state = THREADTS_STRING;
				_CW_NXO_THREAD_PUTC('\t');
				break;
			case 'b':
				a_thread->state = THREADTS_STRING;
				_CW_NXO_THREAD_PUTC('\b');
				break;
			case 'f':
				a_thread->state = THREADTS_STRING;
				_CW_NXO_THREAD_PUTC('\f');
				break;
			case 'x':
				a_thread->state = THREADTS_STRING_HEX_CONT;
				break;
			case '\r':
				a_thread->state = THREADTS_STRING_CRLF_CONT;
				break;
			case '\n':
				_CW_NXO_THREAD_NEWLINE();

				/* Ignore. */
				a_thread->state = THREADTS_STRING;
				break;
			default:
				a_thread->state = THREADTS_STRING;
				_CW_NXO_THREAD_PUTC('\\');
				_CW_NXO_THREAD_PUTC(c);
				break;
			}
			break;
		case THREADTS_STRING_CRLF_CONT:
			switch (c) {
			case '\n':
				_CW_NXO_THREAD_NEWLINE();

				/* Ignore. */
				a_thread->state = THREADTS_STRING;
				break;
			default:
				goto STRING_CONTINUE;
			}
			break;
		case THREADTS_STRING_HEX_CONT:
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
				a_thread->state = THREADTS_STRING_HEX_FINISH;
				a_thread->m.s.hex_val = c;
				break;
			default:
				nxoe_p_thread_syntax_error(a_thread, a_threadp,
				    "(", "\\x", c);
				if (a_token)
					goto RETURN;
			}
			break;
		case THREADTS_STRING_HEX_FINISH:
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':{
				cw_uint8_t	val;

				a_thread->state = THREADTS_STRING;
				switch (a_thread->m.s.hex_val) {
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					val = (a_thread->m.s.hex_val - '0') <<
					    4;
					break;
				case 'a': case 'b': case 'c': case 'd':
				case 'e': case 'f':
					val = ((a_thread->m.s.hex_val - 'a') +
					    10) << 4;
					break;
				default:
					_cw_not_reached();
				}
				switch (c) {
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					val |= (c - '0');
					break;
				case 'a': case 'b': case 'c': case 'd':
				case 'e': case 'f':
					val |= ((c - 'a') + 10);
					break;
				default:
					_cw_not_reached();
				}
				_CW_NXO_THREAD_PUTC(val);
				break;
			}
			default: {
				cw_uint8_t	suffix[] = "\\x?";

				suffix[2] = a_thread->m.s.hex_val;
				nxoe_p_thread_syntax_error(a_thread, a_threadp,
				    "(", suffix, c);
				if (a_token)
					goto RETURN;
			}
			}
			break;
		case THREADTS_NAME: {
			cw_bool_t	restart = FALSE;

			switch (c) {
			case '\n':
				restart = TRUE;	/* Inverted below. */
				_CW_NXO_THREAD_NEWLINE();
				/* Fall through. */
			case '(': case ')': case '`': case '\'': case '"':
			case '<': case '>': case '[': case ']': case '{':
			case '}': case '/': case '%':
				/* New token. */
				/*
				 * Invert, in case we fell through from
				 * above.
				 */
				restart = !restart;
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* End of name. */
				if (a_thread->index > 0) {
					token = TRUE;
					nxoe_p_thread_name_accept(a_thread);
				} else {
					switch (a_thread->m.m.action) {
					case ACTION_LITERAL:
						nxoe_p_thread_syntax_error(
						    a_thread, a_threadp, "/",
						    "", c);
						break;
					case ACTION_EVALUATE:
						nxoe_p_thread_syntax_error(
						    a_thread, a_threadp, "//",
						    "", c);
						break;
					default:
						_cw_not_reached();
					}
					if (a_token)
						goto RETURN;
				}
				if (restart)
					goto RESTART;
				break;
			default:
				_CW_NXO_THREAD_PUTC(c);
				break;
			}
			break;
		}
		default:
			_cw_not_reached();
		}
	}

	retval = i;
	RETURN:
	if (a_token)
		a_thread->defer_count--;
	return retval;
}

static void
nxoe_p_thread_tok_str_expand(cw_nxoe_thread_t *a_thread)
{
	if (a_thread->index == _CW_NXO_THREAD_BUFFER_SIZE) {
		/*
		 * First overflow, initial expansion needed.
		 */
		a_thread->tok_str = (cw_uint8_t *)_cw_malloc(a_thread->index *
		    2);
		a_thread->buffer_len = a_thread->index * 2;
		memcpy(a_thread->tok_str, a_thread->buffer, a_thread->index);
	} else if (a_thread->index == a_thread->buffer_len) {
		cw_uint8_t	*t_str;

		/*
		 * Overflowed, and additional expansion needed.
		 */
		t_str = (cw_uint8_t *)_cw_malloc(a_thread->index * 2);
		a_thread->buffer_len = a_thread->index * 2;
		memcpy(t_str, a_thread->tok_str, a_thread->index);
		_cw_free(a_thread->tok_str);
		a_thread->tok_str = t_str;
	}
}

/*
 * Create a string that represents the code that caused the syntax error and
 * push it onto ostack.  This means that syntax errors cause two objects to be
 * pushed onto ostack rather than just the standard one, but if we don't do
 * this, the invalid code gets lost forever.
 */
static void
nxoe_p_thread_syntax_error(cw_nxoe_thread_t *a_thread, cw_nxo_threadp_t
    *a_threadp, cw_uint8_t *a_prefix, cw_uint8_t *a_suffix, cw_uint8_t a_c)
{
	cw_nxo_t	*nxo, *currenterror, *key, *val;
	cw_uint32_t	line, column;

	nxo = nxo_stack_push(&a_thread->ostack);

	nxo_string_new(nxo, a_thread->nx, a_thread->locking, strlen(a_prefix) +
	    a_thread->index + strlen(a_suffix) + 1);
	nxo_attrs_set(nxo, NXOA_EXECUTABLE);

	/* Prefix. */
	nxo_string_set(nxo, 0, a_prefix, strlen(a_prefix));

	/* Main text. */
	nxo_string_set(nxo, strlen(a_prefix), a_thread->tok_str,
	    a_thread->index);

	/* Suffix. */
	nxo_string_set(nxo, strlen(a_prefix) + a_thread->index, a_suffix,
	    strlen(a_suffix));

	/* Current character. */
	nxo_string_set(nxo, strlen(a_prefix) + a_thread->index +
	    strlen(a_suffix), &a_c, 1);

	nxoe_p_thread_reset(a_thread);

	/*
	 * Set line and column in currenterror.  Look up currenterror on dstack,
	 * since there is the possibility that the user has done something silly
	 * like undef it.
	 */
	nxo_threadp_position_get(a_threadp, &line, &column);

	currenterror = nxo_stack_push(&a_thread->tstack);
	key = nxo_stack_push(&a_thread->tstack);
	val = nxo_stack_push(&a_thread->tstack);

	nxo_name_new(key, a_thread->nx,
	    nxn_str(NXN_currenterror), nxn_len(NXN_currenterror), TRUE);
	if (nxo_thread_dstack_search(&a_thread->self, key, currenterror)) {
		/* Couldn't find currenterror.  Don't record line and column. */
		goto ERROR;
	}

	nxo_name_new(key, a_thread->nx, nxn_str(NXN_line), nxn_len(NXN_line),
	    TRUE);
	
	nxo_integer_new(val, (cw_nxoi_t)line);
	nxo_dict_def(currenterror, a_thread->nx, key, val);

	nxo_name_new(key, a_thread->nx, nxn_str(NXN_column),
	    nxn_len(NXN_column), TRUE);
	/*
	 * If the syntax error happened at a newline, the column number won't
	 * be correct, so use 0.
	 */
	if (column == -1)
		nxo_integer_new(val, 0);
	else
		nxo_integer_new(val, (cw_nxoi_t)column);
	nxo_dict_def(currenterror, a_thread->nx, key, val);

	ERROR:
	nxo_stack_npop(&a_thread->tstack, 3);

	/* Finally, throw a syntaxerror. */
	nxo_thread_error(&a_thread->self, NXO_THREADE_SYNTAXERROR);
}

static void
nxoe_p_thread_reset(cw_nxoe_thread_t *a_thread)
{
	a_thread->state = THREADTS_START;
	if (a_thread->index > _CW_NXO_THREAD_BUFFER_SIZE) {
		_cw_free(a_thread->tok_str);
		a_thread->tok_str = a_thread->buffer;
	}
	a_thread->index = 0;
}

static void
nxoe_p_thread_procedure_accept(cw_nxoe_thread_t *a_thread)
{
	cw_nxo_t	*tnxo, *nxo;
	cw_uint32_t	nelements, i, depth;

	/* Find the no "mark". */
	for (i = 0, depth = nxo_stack_count(&a_thread->ostack), nxo = NULL;
	     i < depth; i++) {
		nxo = nxo_stack_down_get(&a_thread->ostack, nxo);
		if (nxo_type_get(nxo) == NXOT_NO)
			break;
	}
	_cw_assert(i < depth);

	/*
	 * i is the index of the mark, and nxo points to the mark.  Set
	 * nelements accordingly.  When we pop the nxo's off the stack, we'll
	 * have to pop (nelements + 1) nxo's.
	 */
	nelements = i;

	tnxo = nxo_stack_push(&a_thread->tstack);
	nxo_array_new(tnxo, a_thread->nx, a_thread->locking, nelements);
	nxo_attrs_set(tnxo, NXOA_EXECUTABLE);

	/*
	 * Traverse down the stack, moving nxo's to the array.
	 */
	for (i = nelements, nxo = NULL; i > 0; i--) {
		nxo = nxo_stack_down_get(&a_thread->ostack, nxo);
		nxo_array_el_set(tnxo, nxo, i - 1);
	}

	/* Pop the nxo's off the stack now. */
	nxo_stack_npop(&a_thread->ostack, nelements + 1);

	/* Push the array onto the stack. */
	nxo = nxo_stack_push(&a_thread->ostack);
	nxo_dup(nxo, tnxo);
	nxo_stack_pop(&a_thread->tstack);
}

static void
nxoe_p_thread_name_accept(cw_nxoe_thread_t *a_thread)
{
	cw_nxo_t	*nxo;

	switch (a_thread->m.m.action) {
	case ACTION_EXECUTE:
		if (a_thread->defer_count == 0) {
			/*
			 * Find the the value associated with the name in the
			 * dictionary stack, push it onto the execution stack,
			 * and run the execution loop.
			 */
			nxo = nxo_stack_push(&a_thread->estack);
			nxo_name_new(nxo, a_thread->nx, a_thread->tok_str,
			    a_thread->index, FALSE);
			nxo_attrs_set(nxo, NXOA_EXECUTABLE);

			nxoe_p_thread_reset(a_thread);
			nxo_thread_loop(&a_thread->self);
		} else {
			/* Push the name object onto the data stack. */
			nxo = nxo_stack_push(&a_thread->ostack);
			nxo_name_new(nxo, a_thread->nx, a_thread->tok_str,
			    a_thread->index, FALSE);
			nxo_attrs_set(nxo, NXOA_EXECUTABLE);
			nxoe_p_thread_reset(a_thread);
		}
		break;
	case ACTION_LITERAL:
		/* Push the name object onto the data stack. */
		nxo = nxo_stack_push(&a_thread->ostack);
		nxo_name_new(nxo, a_thread->nx, a_thread->tok_str,
		    a_thread->index, FALSE);
		nxoe_p_thread_reset(a_thread);
		break;
	case ACTION_EVALUATE: {
		cw_nxo_t	*key;

		/*
		 * Find the value associated with the name in the dictionary
		 * stack and push the value onto the data stack.
		 */
		key = nxo_stack_push(&a_thread->estack);
		nxo_name_new(key, a_thread->nx, a_thread->tok_str,
		    a_thread->index, FALSE);
		nxoe_p_thread_reset(a_thread);

		nxo = nxo_stack_push(&a_thread->ostack);
		if (nxo_thread_dstack_search(&a_thread->self, key, nxo)) {
			nxo_stack_pop(&a_thread->ostack);
			nxo_thread_error(&a_thread->self,
			    NXO_THREADE_UNDEFINED);
		}
		nxo_stack_pop(&a_thread->estack);

		break;
	}
	default:
		_cw_not_reached();
	}
}
