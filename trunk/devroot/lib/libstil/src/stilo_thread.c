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

#define _STILO_THREAD_C_

#include "../include/libstil/libstil.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "../include/libstil/currenterror_l.h"
#include "../include/libstil/errordict_l.h"
#include "../include/libstil/threaddict_l.h"
#include "../include/libstil/stil_l.h"
#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_array_l.h"
#include "../include/libstil/stilo_operator_l.h"
#include "../include/libstil/stilo_thread_l.h"

cw_stiln_t
stilo_threade_stiln(cw_stilo_threade_t a_threade)
{
	static const cw_stiln_t threade_stiln[] = {
		0,
		STILN_dstackunderflow,
		STILN_estackoverflow,
		STILN_interrupt,
		STILN_invalidaccess,
		STILN_invalidcontext,
		STILN_invalidexit,
		STILN_invalidfileaccess,
		STILN_ioerror,
		STILN_limitcheck,
		STILN_rangecheck,
		STILN_stackunderflow,
		STILN_syntaxerror,
		STILN_timeout,
		STILN_typecheck,
		STILN_undefined,
		STILN_undefinedfilename,
		STILN_undefinedresult,
		STILN_unmatchedfino,
		STILN_unmatchedmark,
		STILN_unregistered
	};
	_cw_assert(sizeof(threade_stiln) / sizeof(cw_stiln_t) ==
	    STILO_THREADE_LAST + 1);
	_cw_assert(a_threade > 0 && a_threade <= STILO_THREADE_LAST);
	return threade_stiln[a_threade];
}

#define _CW_STILO_THREAD_GETC(a_i)					\
	a_thread->tok_str[(a_i)]

#define _CW_STILO_THREAD_PUTC(a_c)					\
	do {								\
		if (a_thread->index >= _CW_STILO_THREAD_BUFFER_SIZE)	\
			stiloe_p_thread_tok_str_expand(a_thread);	\
		a_thread->tok_str[a_thread->index] = (a_c);		\
		a_thread->index++;					\
	} while (0)

/*
 * Make a note that a '\n' was just seen.  The line and column counters will be
 * updated before the next character is seen.
 */
#define _CW_STILO_THREAD_NEWLINE()					\
		newline = 1

static cw_uint32_t	stiloe_p_thread_feed(cw_stiloe_thread_t *a_thread,
    cw_stilo_threadp_t *a_threadp, cw_uint32_t a_token, const cw_uint8_t *a_str,
    cw_uint32_t a_len);
static void		stiloe_p_thread_tok_str_expand(cw_stiloe_thread_t
    *a_thread);
static void		stiloe_p_thread_syntax_error(cw_stiloe_thread_t
    *a_thread, cw_stilo_threadp_t *a_threadp, cw_uint8_t *a_prefix, cw_uint8_t
    *a_suffix, cw_uint8_t a_c);
static void		stiloe_p_thread_reset(cw_stiloe_thread_t *a_thread);
static void		stiloe_p_thread_procedure_accept(cw_stiloe_thread_t
    *a_thread);
static void		stiloe_p_thread_name_accept(cw_stiloe_thread_t
    *a_thread);

#ifdef _LIBSTIL_DBG
#define _CW_STILO_THREADP_MAGIC 0xdfe76a68
#endif

/*
 * stilo_threadp.
 */
cw_stilo_threadp_t *
stilo_threadp_new(cw_stilo_threadp_t *a_threadp)
{
	cw_stilo_threadp_t	*retval;

	if (a_threadp != NULL) {
		retval = a_threadp;
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_stilo_threadp_t
		    *)_cw_malloc(sizeof(cw_stilo_threadp_t));
		retval->is_malloced = TRUE;
	}

	retval->line = 1;
	retval->column = 0;

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILO_THREADP_MAGIC;
#endif
	return retval;
}

void
stilo_threadp_delete(cw_stilo_threadp_t *a_threadp, cw_stilo_t *a_thread)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_thread);
	_cw_assert(a_thread->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_thread->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	if (thread->state != THREADTS_START) {
		/*
		 * It's possible that the last token seen hasn't been accepted
		 * yet.  Reset the internal state so that this won't screw
		 * things up later.
		 */
		stiloe_p_thread_reset(thread);
	}

	if (a_threadp->is_malloced)
		_cw_free(a_threadp);
#ifdef _LIBSTIL_DBG
	else
		memset(a_threadp, 0x5a, sizeof(cw_stilo_threadp_t));
#endif
}

void
stilo_threadp_position_get(cw_stilo_threadp_t *a_threadp, cw_uint32_t *r_line,
    cw_uint32_t *r_column)
{
	_cw_check_ptr(a_threadp);
	_cw_assert(a_threadp->magic == _CW_STILO_THREADP_MAGIC);

	*r_line = a_threadp->line;
	*r_column = a_threadp->column;
}

void
stilo_threadp_position_set(cw_stilo_threadp_t *a_threadp, cw_uint32_t a_line,
    cw_uint32_t a_column)
{
	_cw_check_ptr(a_threadp);
	_cw_assert(a_threadp->magic == _CW_STILO_THREADP_MAGIC);

	a_threadp->line = a_line;
	a_threadp->column = a_column;
}

/*
 * stilo_thread.
 */
void
stilo_thread_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil)
{
	cw_stiloe_thread_t	*thread;
	cw_stilo_t		*stilo;

	thread = (cw_stiloe_thread_t *)_cw_malloc(sizeof(cw_stiloe_thread_t));
	memset(thread, 0, sizeof(cw_stiloe_thread_t));

	stiloe_l_new(&thread->stiloe, STILOT_THREAD, FALSE);

	/*
	 * Set things to a state that won't cause the GC (or any thread-related
	 * operators) to puke.
	 */

	/* Fake up a stilo for self. */
	stilo_l_new(&thread->self, STILOT_THREAD);
	thread->self.o.stiloe = (cw_stiloe_t *)thread;

	thread->stil = a_stil;
	thread->tok_str = thread->buffer;

	stilo_no_new(&thread->estack);
	stilo_no_new(&thread->ostack);
	stilo_no_new(&thread->dstack);
	stilo_no_new(&thread->tstack);
	stilo_no_new(&thread->currenterror);
	stilo_no_new(&thread->errordict);
	stilo_no_new(&thread->userdict);
	stilo_no_new(&thread->threaddict);

	/*
	 * Register this thread with the interpreter so that the GC will be able
	 * to get to it.
	 */
	stil_l_thread_insert(a_stil, &thread->self);

	/*
	 * Register with the GC so that this thread will be iterated on during
	 * GC.
	 */
	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)thread;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_THREAD;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)thread);

	/*
	 * Finish setting up the internals.
	 */
	stilo_stack_new(&thread->estack, a_stil, FALSE);
	stilo_stack_new(&thread->ostack, a_stil, FALSE);
	stilo_stack_new(&thread->dstack, a_stil, FALSE);
	stilo_stack_new(&thread->tstack, a_stil, FALSE);
	
	currenterror_l_populate(&thread->currenterror, a_stilo);
	errordict_l_populate(&thread->errordict, a_stilo);
	stilo_dict_new(&thread->userdict, a_stil, FALSE,
	    _LIBSTASH_USERDICT_HASH);
	threaddict_l_populate(&thread->threaddict, a_stilo);

	/*
	 * Push threaddict, systemdict, globaldict, and userdict onto
	 * the dictionary stack.
	 */
	stilo = stilo_stack_push(&thread->dstack);
	stilo_dup(stilo, &thread->threaddict);

	stilo = stilo_stack_push(&thread->dstack);
	stilo_dup(stilo, stil_systemdict_get(a_stil));

	stilo = stilo_stack_push(&thread->dstack);
	stilo_dup(stilo, stil_globaldict_get(a_stil));

	stilo = stilo_stack_push(&thread->dstack);
	stilo_dup(stilo, &thread->userdict);

	/* Execute the thread initialization hook if set. */
	if (stil_l_thread_init(a_stil) != NULL)
		stil_l_thread_init(a_stil)(&thread->self);
}

void
stiloe_l_thread_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_thread_t	*thread;

	thread = (cw_stiloe_thread_t *)a_stiloe;

	_cw_check_ptr(thread);
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	if (thread->tok_str != thread->buffer) {
		/*
		 * This shouldn't happen, since it indicates that there is an
		 * unaccepted token.  However, it's really the caller's fault,
		 * so just clean up.
		 */
		_cw_free(thread->tok_str);
	}

	_CW_STILOE_FREE(thread);
}

cw_stiloe_t *
stiloe_l_thread_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_thread_t	*thread;

	thread = (cw_stiloe_thread_t *)a_stiloe;

	if (a_reset)
		thread->ref_iter = 0;

	for (retval = NULL; retval == NULL; thread->ref_iter++) {
		switch (thread->ref_iter) {
		case 0:
			retval = stilo_stiloe_get(&thread->estack);
			break;
		case 1:
			retval = stilo_stiloe_get(&thread->ostack);
			break;
		case 2:
			retval = stilo_stiloe_get(&thread->dstack);
			break;
		case 3:
			retval = stilo_stiloe_get(&thread->tstack);
			break;
		case 4:
			retval = stilo_stiloe_get(&thread->currenterror);
			break;
		case 5:
			retval = stilo_stiloe_get(&thread->errordict);
			break;
		case 6:
			retval = stilo_stiloe_get(&thread->userdict);
			break;
		case 7:
			retval = stilo_stiloe_get(&thread->threaddict);
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
stilo_l_thread_print(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *depth, *thread, *stdout_stilo;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(depth, ostack, a_thread);
	STILO_STACK_DOWN_GET(thread, ostack, a_thread, depth);
	if (stilo_type_get(depth) != STILOT_INTEGER || stilo_type_get(thread)
	    != STILOT_THREAD) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stdout_stilo = stil_stdout_get(stilo_thread_stil_get(a_thread));

	error = stilo_file_output(stdout_stilo, "-thread-");

	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_npop(ostack, 2);
}

void
stilo_thread_start(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;
	cw_stilo_t		*start;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	start = stilo_stack_push(&thread->estack);
	stilo_operator_new(start, systemdict_start, STILN_start);
	stilo_attrs_set(start, STILOA_EXECUTABLE);

	stilo_thread_loop(a_stilo);
}

void
stilo_thread_exit(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	stil_l_thread_remove(thread->stil, &thread->self);
}

void
stilo_thread_self(cw_stilo_t *a_stilo, cw_stilo_t *r_self)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	_cw_check_ptr(r_self);

	stilo_dup(r_self, &thread->self);
}

static void *
stilo_p_thread_entry(void *a_arg)
{
	cw_stilo_thread_entry_t	*arg = (cw_stilo_thread_entry_t *)a_arg;

	/* Run. */
	stilo_thread_loop(arg->thread);

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
		stil_l_thread_remove(stilo_thread_stil_get(arg->thread),
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
stilo_thread_thread(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;
	sigset_t		sig_mask, old_mask;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	thread->entry = (cw_stilo_thread_entry_t
	    *)_cw_malloc(sizeof(cw_stilo_thread_entry_t));

	thread->entry->thread = a_stilo;
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
	thread->entry->thd = thd_new(stilo_p_thread_entry, (void
	    *)thread->entry, TRUE);
	thd_sigmask(SIG_SETMASK, &old_mask, NULL);
}

void
stilo_thread_detach(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	_cw_check_ptr(thread->entry);

	mtx_lock(&thread->entry->lock);
	thread->entry->detached = TRUE;
	if (thread->entry->done) {
		/* The thread is already done, so wake it back up. */
		cnd_signal(&thread->entry->done_cnd);
	}
	mtx_unlock(&thread->entry->lock);
}

void
stilo_thread_join(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;
	cw_stilo_thread_entry_t	*entry;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	_cw_check_ptr(thread->entry);

	mtx_lock(&thread->entry->lock);
	thread->entry->joined = TRUE;
	if (thread->entry->done) {
		/* The thread is already done, so wake it back up. */
		cnd_signal(&thread->entry->done_cnd);
	}
	/* Wait for the thread to totally go away. */
	while (thread->entry->gone == FALSE)
		cnd_wait(&thread->entry->join_cnd, &thread->entry->lock);
	mtx_unlock(&thread->entry->lock);

	/* Clean up. */
	cnd_delete(&thread->entry->join_cnd);
	cnd_delete(&thread->entry->done_cnd);
	mtx_delete(&thread->entry->lock);
	thd_join(thread->entry->thd);
	entry = thread->entry;
	stil_l_thread_remove(stilo_thread_stil_get(a_stilo), a_stilo);
	_cw_free(entry);
}

cw_stilo_threadts_t
stilo_thread_state(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return thread->state;
}

cw_bool_t
stilo_thread_deferred(cw_stilo_t *a_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	if (thread->defer_count != 0)
		retval = TRUE;
	else
		retval = FALSE;

	return retval;
}

void
stilo_thread_reset(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	thread->defer_count = 0;
	stiloe_p_thread_reset(thread);
}

void
stilo_thread_loop(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;
	cw_stilo_t		*stilo, *tstilo;
	cw_uint32_t		sdepth, cdepth;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	for (sdepth = cdepth = stilo_stack_count(&thread->estack);
	     cdepth >= sdepth; cdepth = stilo_stack_count(&thread->estack)) {
		if (cdepth == _LIBSTIL_ESTACK_MAX + 1) {
			stilo_thread_error(a_stilo,
			    STILO_THREADE_ESTACKOVERFLOW);
		}

		stilo = stilo_stack_get(&thread->estack);
		if (stilo_attrs_get(stilo) == STILOA_LITERAL) {
			/* Always push literal objects onto the data stack. */
			tstilo = stilo_stack_push(&thread->ostack);
			stilo_dup(tstilo, stilo);
			stilo_stack_pop(&thread->estack);
			continue;
		}

		switch (stilo_type_get(stilo)) {
		case STILOT_BOOLEAN:
		case STILOT_CONDITION:
		case STILOT_DICT:
		case STILOT_FINO:
		case STILOT_INTEGER:
		case STILOT_MARK:
		case STILOT_MUTEX:
		case STILOT_STACK:
		case STILOT_THREAD:
			/*
			 * Always push the object onto the data stack, even
			 * though it isn't literal.
			 */
			tstilo = stilo_stack_push(&thread->ostack);
			stilo_dup(tstilo, stilo);
			stilo_stack_pop(&thread->estack);
			break;
		case STILOT_NULL:
			/* Do nothing. */
			stilo_stack_pop(&thread->estack);
			break;
		case STILOT_ARRAY: {
			cw_uint32_t	i, len;
			cw_stilo_t	*el;

			len = stilo_array_len_get(stilo);
			if (len == 0) {
				stilo_stack_pop(&thread->estack);
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
			el = stilo_stack_push(&thread->tstack);
			for (i = 0; i < len - 1; i++) {
				stilo_l_array_el_get(stilo, i, el);
				if (stilo_attrs_get(el) == STILOA_LITERAL) {
					/*
					 * Always push literal objects onto the
					 * data stack.
					 */
					tstilo =
					    stilo_stack_push(&thread->ostack);
					stilo_dup(tstilo, el);
					continue;
				}

				switch (stilo_type_get(el)) {
				case STILOT_ARRAY:
					/*
					 * Don't execute nested arrays.
					 */
					tstilo =
					    stilo_stack_push(&thread->ostack);
					stilo_dup(tstilo, el);
					break;
				case STILOT_OPERATOR:
					if (stilo_l_operator_fast_op_get(el)
					    == FALSE) {
						stilo_operator_f(el)(a_stilo);
						break;
					}

					/* Fast operator. */
					switch (stilo_l_operator_fast_op_stiln(
					    el)) {
					case STILN_add:
						systemdict_inline_add(a_stilo);
						break;
					case STILN_dup:
						systemdict_inline_dup(a_stilo);
						break;
					case STILN_exch:
						systemdict_inline_exch(a_stilo);
						break;
					case STILN_index:
						systemdict_inline_index(
						    a_stilo);
						break;
					case STILN_pop:
						systemdict_inline_pop(a_stilo);
						break;
					case STILN_roll:
						systemdict_inline_roll(a_stilo);
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
					tstilo =
					    stilo_stack_push(&thread->estack);
					stilo_dup(tstilo, el);
					stilo_thread_loop(a_stilo);
				}
			}

			/*
			 * If recursion is possible and likely, make tail
			 * recursion safe by replacing the array with its last
			 * element before executing the last element.
			 */
			stilo_l_array_el_get(stilo, i, el);
			if ((stilo_attrs_get(el) == STILOA_LITERAL) ||
			    (stilo_type_get(el) == STILOT_ARRAY)) {
				/*
				 * Always push literal objects and nested arrays
				 * onto the data stack.
				 */
				tstilo = stilo_stack_push(&thread->ostack);
				stilo_dup(tstilo, el);
				stilo_stack_pop(&thread->estack);
			} else {
				/* Possible recursion. */
				stilo_dup(stilo, el);
			}
			stilo_stack_pop(&thread->tstack);
			break;
		}
		case STILOT_STRING: {
			cw_stilo_threadp_t	threadp;

			/*
			 * Use the string as a source of code.
			 */
			stilo_threadp_new(&threadp);
			stilo_string_lock(stilo);
			stilo_thread_interpret(a_stilo, &threadp,
			    stilo_string_get(stilo),
			    stilo_string_len_get(stilo));
			stilo_string_unlock(stilo);
			stilo_thread_flush(a_stilo, &threadp);
			stilo_threadp_delete(&threadp, a_stilo);
			stilo_stack_pop(&thread->estack);

			break;
		}
		case STILOT_NAME: {
			cw_stilo_t	*name;

			/*
			 * Search for a value associated with the name in the
			 * dictionary stack, put it on the execution stack, and
			 * execute it.
			 */
			name = stilo_stack_push(&thread->tstack);
			stilo_dup(name, stilo);
			if (stilo_thread_dstack_search(a_stilo, name, stilo)) {
				stilo_thread_error(a_stilo,
				    STILO_THREADE_UNDEFINED);
				stilo_stack_pop(&thread->estack);
			}
			stilo_stack_pop(&thread->tstack);
			break;
		}
		case STILOT_OPERATOR:
			if (stilo_l_operator_fast_op_get(stilo) == FALSE) {
				stilo_operator_f(stilo)(a_stilo);
				stilo_stack_pop(&thread->estack);
				break;
			}

			/* Fast operator. */
			switch (stilo_l_operator_fast_op_stiln(stilo)) {
			case STILN_add:
				systemdict_inline_add(a_stilo);
				break;
			case STILN_dup:
				systemdict_inline_dup(a_stilo);
				break;
			case STILN_exch:
				systemdict_inline_exch(a_stilo);
				break;
			case STILN_index:
				systemdict_inline_index(a_stilo);
				break;
			case STILN_pop:
				systemdict_inline_pop(a_stilo);
				break;
			case STILN_roll:
				systemdict_inline_roll(a_stilo);
				break;
			default:
				_cw_not_reached();
			}

			stilo_stack_pop(&thread->estack);
			break;
		case STILOT_FILE: {
			cw_stilo_threadp_t	threadp;
			cw_sint32_t	nread;
			cw_uint8_t	buffer[_LIBSTIL_FILE_EVAL_READ_SIZE];

			stilo_threadp_new(&threadp);
			/*
			 * Read data from the file and interpret it until an EOF
			 * (0 byte read).
			 */
			for (nread = stilo_file_read(stilo,
			    _LIBSTIL_FILE_EVAL_READ_SIZE, buffer); nread > 0;
			    nread = stilo_file_read(stilo,
			    _LIBSTIL_FILE_EVAL_READ_SIZE, buffer)) {
				stilo_thread_interpret(a_stilo, &threadp,
				    buffer, nread);
			}
			stilo_thread_flush(a_stilo, &threadp);
			stilo_threadp_delete(&threadp, a_stilo);

			stilo_stack_pop(&thread->estack);
			break;
		}
		case STILOT_HOOK: {
			cw_stilo_threade_t	error;
			
			error = stilo_hook_exec(stilo, a_stilo);
			if (error)
				stilo_thread_error(a_stilo, error);

			stilo_stack_pop(&thread->estack);
			break;
		}
		default:
			_cw_not_reached();
		}
	}
}

void
stilo_thread_interpret(cw_stilo_t *a_stilo, cw_stilo_threadp_t *a_threadp, const
    cw_uint8_t *a_str, cw_uint32_t a_len)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	stiloe_p_thread_feed(thread, a_threadp, 0, a_str, a_len);
}

void
stilo_thread_flush(cw_stilo_t *a_stilo, cw_stilo_threadp_t *a_threadp)
{
	cw_stiloe_thread_t	*thread;
	static const cw_uint8_t	str[] = "\n";

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	stiloe_p_thread_feed(thread, a_threadp, 0, str, sizeof(str) - 1);
}

cw_uint32_t
stilo_thread_token(cw_stilo_t *a_stilo, cw_stilo_threadp_t *a_threadp, const
    cw_uint8_t *a_str, cw_uint32_t a_len)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return stiloe_p_thread_feed(thread, a_threadp, 1, a_str, a_len);
}

void
stilo_thread_error(cw_stilo_t *a_stilo, cw_stilo_threade_t a_error)
{
	cw_stiloe_thread_t	*thread;
	cw_stilo_t	*stilo, *errordict, *key, *handler;
	cw_stiln_t	stiln;
	cw_uint32_t	defer_count;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	/* Shut off deferral temporarily. */
	defer_count = thread->defer_count;
	thread->defer_count = 0;

	/*
	 * Get errordict.  We can't throw an undefined error here because it
	 * would go infinitely recursive.
	 */
	errordict = stilo_stack_push(&thread->tstack);
	key = stilo_stack_push(&thread->tstack);
	stilo_name_new(key, thread->stil, stiln_str(STILN_errordict),
	    stiln_len(STILN_errordict), TRUE);
	if (stilo_thread_dstack_search(a_stilo, key, errordict)) {
		/*
		 * We failed to find errordict.  Fall back to the one originally
		 * defined in the thread.
		 */
		stilo_dup(errordict, &thread->errordict);
	}

	/*
	 * Find handler corresponding to error.
	 */
	stiln = stilo_threade_stiln(a_error);
	stilo_name_new(key, thread->stil, stiln_str(stiln), stiln_len(stiln),
	    TRUE);

	/*
	 * Push the object being executed onto ostack unless this is an
	 * interrupt or timeout.
	 */
	switch (a_error) {
	case STILO_THREADE_INTERRUPT:
	case STILO_THREADE_TIMEOUT:
		break;
	default:
		stilo = stilo_stack_push(&thread->ostack);
		stilo_dup(stilo, stilo_stack_get(&thread->estack));
	}

	/*
	 * Get the handler for this particular error and push it onto estack.
	 * We could potentially throw another error here without going
	 * infinitely recursive, but it's not worth the risk.  After all, the
	 * user has done some really hokey management of errordict if this
	 * happens.
	 */
	handler = stilo_stack_push(&thread->estack);
	if (stilo_dict_lookup(errordict, key, handler)) {
		/*
		 * Ignore the error, since the only alternative is to blow
		 * up (or potentially go infinitely recursive).
		 */
		stilo_stack_npop(&thread->tstack, 2);
		stilo_stack_pop(&thread->estack);
		goto IGNORE;
	}
	stilo_stack_npop(&thread->tstack, 2);

	/* Execute the handler. */
	stilo_thread_loop(a_stilo);

	IGNORE:
	/* Turn deferral back on. */
	thread->defer_count = defer_count;
}

cw_bool_t
stilo_thread_dstack_search(cw_stilo_t *a_stilo, cw_stilo_t *a_key, cw_stilo_t
    *r_value)
{
	cw_bool_t		retval;
	cw_stiloe_thread_t	*thread;
	cw_stilo_t		*dict;
	cw_uint32_t		i, depth;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	/*
	 * Iteratively search the dictionaries on the dictionary stack for
	 * a_key.
	 */
	for (i = 0, depth = stilo_stack_count(&thread->dstack), dict = NULL; i
	    < depth; i++) {
		dict = stilo_stack_down_get(&thread->dstack, dict);
		if (stilo_dict_lookup(dict, a_key, r_value) == FALSE) {
			/* Found. */
			retval = FALSE;
			goto RETURN;
		}
	}

	retval = TRUE;
	RETURN:
	return retval;
}

cw_stilo_t *
stilo_thread_threaddict_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return &thread->threaddict;
}

cw_stilo_t *
stilo_thread_userdict_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return &thread->userdict;
}

cw_stilo_t *
stilo_thread_errordict_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return &thread->errordict;
}

cw_stilo_t *
stilo_thread_currenterror_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return &thread->currenterror;
}

cw_bool_t
stilo_thread_currentlocking(cw_stilo_t *a_stilo)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	return thread->locking;
}

void
stilo_thread_setlocking(cw_stilo_t *a_stilo, cw_bool_t a_locking)
{
	cw_stiloe_thread_t	*thread;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	thread = (cw_stiloe_thread_t *)a_stilo->o.stiloe;
	_cw_assert(thread->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(thread->stiloe.type == STILOT_THREAD);

	thread->locking = a_locking;
}

static cw_uint32_t
stiloe_p_thread_feed(cw_stiloe_thread_t *a_thread, cw_stilo_threadp_t
    *a_threadp, cw_uint32_t a_token, const cw_uint8_t *a_str, cw_uint32_t a_len)
{
	cw_uint32_t	retval, i, newline;
	cw_uint8_t	c;
	cw_stilo_t	*stilo;
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
				stiloe_p_thread_syntax_error(a_thread,
				    a_threadp, "", "", c);
				if (a_token)
					goto RETURN;
				break;
			case '<': case '>': case '(': case ')': case '[':
			case ']':
				_CW_STILO_THREAD_PUTC(c);
				token = TRUE;
				a_thread->m.m.action = ACTION_EXECUTE;
				stiloe_p_thread_name_accept(a_thread);
				break;
			case '{':
				a_thread->defer_count++;
				stilo = stilo_stack_push(&a_thread->ostack);
				/*
				 * Leave the stilo as notype in order to
				 * differentiate from normal marks.
				 */
				break;
			case '}':
				if (a_thread->defer_count > a_token) {
					token = TRUE;
					a_thread->defer_count--;
					stiloe_p_thread_procedure_accept(
					    a_thread);
				} else {
					/* Missing '{'. */
					stiloe_p_thread_syntax_error(a_thread,
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
				_CW_STILO_THREAD_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* Swallow. */
				break;
			case '+':
				a_thread->state = THREADTS_INTEGER;
				a_thread->m.n.b_off = 1;
				_CW_STILO_THREAD_PUTC(c);
				break;
			case '-':
				a_thread->state = THREADTS_INTEGER;
				a_thread->m.n.b_off = 1;
				_CW_STILO_THREAD_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				a_thread->state = THREADTS_INTEGER;
				a_thread->m.n.b_off = 0;
				_CW_STILO_THREAD_PUTC(c);
				break;
			default:
				a_thread->state = THREADTS_NAME;
				a_thread->m.m.action = ACTION_EXECUTE;
				_CW_STILO_THREAD_PUTC(c);
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
				_CW_STILO_THREAD_NEWLINE();

				stiloe_p_thread_syntax_error(a_thread,
				    a_threadp, "", "/", c);
				if (a_token)
					goto RETURN;
				break;
			case '\0': case '\t': case '\f': case '\r': case ' ':
			case '(': case ')': case '`': case '\'': case '<':
			case '>': case '[': case ']': case '{': case '}':
			case '%':
				stiloe_p_thread_syntax_error(a_thread,
				    a_threadp, "", "/", c);
				if (a_token)
					goto RETURN;
				break;
			default:
				a_thread->state = THREADTS_NAME;
				a_thread->m.m.action = ACTION_LITERAL;
				_CW_STILO_THREAD_PUTC(c);
				break;
			}
			break;
		case THREADTS_COMMENT:
			_cw_assert(a_thread->index == 0);

			switch (c) {
			case '\n':
				_CW_STILO_THREAD_NEWLINE();
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
				_CW_STILO_THREAD_PUTC(c);
				break;
			case '#': {
				cw_uint32_t	i, digit;

				/*
				 * Convert the string to a base (interpreted as
				 * base 10).
				 */
				a_thread->m.n.base = 0;

				for (i = 0; i < a_thread->index; i++) {
					digit = _CW_STILO_THREAD_GETC(
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
				_CW_STILO_THREAD_PUTC(c);
				break;
			}
			case '\n':
				restart = TRUE; /* Inverted below. */
				_CW_STILO_THREAD_NEWLINE();
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
					cw_stiloi_t	val;

					/* Integer. */

					/*
					 * Convert string to integer.  Do the
					 * conversion before mucking with the
					 * stack in case there is an exception.
					 */
					a_thread->tok_str[a_thread->index] =
					    '\0';
					errno = 0;

#if (_CW_STILOI_SIZEOF == 8)
					val = strtoll(a_thread->tok_str, NULL,
					    10);
#else
					val = strtol(a_thread->tok_str, NULL,
					    10);
#endif
					if ((errno == ERANGE) &&
#if (_CW_STILOI_SIZEOF == 8)
					    ((val == LLONG_MIN) || (val ==
					    LLONG_MAX))
#else
					    ((val == LONG_MIN) || (val ==
					    LONG_MAX))
#endif
					    ) {
						stiloe_p_thread_reset(a_thread);
						stilo_thread_error(
						    &a_thread->self,
						    STILO_THREADE_RANGECHECK);
					} else {
						token = TRUE;
						stilo = stilo_stack_push(
						    &a_thread->ostack);
						stilo_integer_new(stilo, val);
						stiloe_p_thread_reset(a_thread);
					}
				} else {
					/* No number specified, so a name. */
					token = TRUE;
					a_thread->m.m.action = ACTION_EXECUTE;
					stiloe_p_thread_name_accept(a_thread);
				}
				if (restart)
					goto RESTART;
				break;
			default:
				/* Not a number character. */
				a_thread->m.m.action = ACTION_EXECUTE;
				a_thread->state = THREADTS_NAME;
				_CW_STILO_THREAD_PUTC(c);
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
				_CW_STILO_THREAD_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (a_thread->m.n.base <=
				    ((cw_uint32_t)(c - '0'))) {
					/* Too big for this base. */
					a_thread->state = THREADTS_NAME;
					a_thread->m.m.action = ACTION_EXECUTE;
				}
				_CW_STILO_THREAD_PUTC(c);
				break;
			case '\n':
				restart = TRUE; /* Inverted below. */
				_CW_STILO_THREAD_NEWLINE();
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
					cw_stiloi_t	val;

					/* Integer. */

					/*
					 * Convert string to integer.  Do the
					 * conversion before mucking with the
					 * stack in case there is an exception.
					 */
					a_thread->tok_str[a_thread->index] =
					    '\0';
					errno = 0;

#if (_CW_STILOI_SIZEOF == 8)
					val = strtoll(&a_thread->tok_str[
					    a_thread->m.n.b_off], NULL,
					    a_thread->m.n.base);
#else
					val = strtol(&a_thread->tok_str[
					    a_thread->m.n.b_off], NULL,
					    a_thread->m.n.base);
#endif
					if ((errno == ERANGE) &&
#if (_CW_STILOI_SIZEOF == 8)
					    ((val == LLONG_MIN) || (val ==
					    LLONG_MAX))
#else
					    ((val == LONG_MIN) || (val ==
					    LONG_MAX))
#endif
					    ) {
						stiloe_p_thread_reset(a_thread);
						stilo_thread_error(
						    &a_thread->self,
						    STILO_THREADE_RANGECHECK);
					} else {
						token = TRUE;
						stilo = stilo_stack_push(
						    &a_thread->ostack);
						stilo_integer_new(stilo, val);
						stiloe_p_thread_reset(a_thread);
					}
				} else {
					/* No number specified, so a name. */
					token = TRUE;
					a_thread->m.m.action = ACTION_EXECUTE;
					stiloe_p_thread_name_accept(a_thread);
				}
				if (restart)
					goto RESTART;
				break;
			default:
				/* Not a number character. */
				a_thread->m.m.action = ACTION_EXECUTE;
				a_thread->state = THREADTS_NAME;
				_CW_STILO_THREAD_PUTC(c);
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
				_CW_STILO_THREAD_PUTC(c);
				break;
			case '\'':
				a_thread->m.s.q_depth--;
				if (a_thread->m.s.q_depth == 0) {
					token = TRUE;
					stilo =
					    stilo_stack_push(&a_thread->ostack);
					stilo_string_new(stilo, a_thread->stil,
					    a_thread->locking, a_thread->index);
					stilo_string_set(stilo, 0,
					    a_thread->tok_str, a_thread->index);

					stiloe_p_thread_reset(a_thread);
				} else
					_CW_STILO_THREAD_PUTC(c);
				break;
			case '\r':
				a_thread->state = THREADTS_STRING_NEWLINE_CONT;
				break;
			case '\n':
				_CW_STILO_THREAD_NEWLINE();
				/* Fall through. */
			default:
				_CW_STILO_THREAD_PUTC(c);
				break;
			}
			break;
		case THREADTS_STRING_NEWLINE_CONT:
			/* All cases in the switch statement do this. */
			_CW_STILO_THREAD_PUTC('\n');
			a_thread->state = THREADTS_STRING;
			switch (c) {
			case '\n':
				_CW_STILO_THREAD_NEWLINE();
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
				_CW_STILO_THREAD_PUTC(c);
				break;
			case 'n':
				a_thread->state = THREADTS_STRING;
				_CW_STILO_THREAD_PUTC('\n');
				break;
			case 'r':
				a_thread->state = THREADTS_STRING;
				_CW_STILO_THREAD_PUTC('\r');
				break;
			case 't':
				a_thread->state = THREADTS_STRING;
				_CW_STILO_THREAD_PUTC('\t');
				break;
			case 'b':
				a_thread->state = THREADTS_STRING;
				_CW_STILO_THREAD_PUTC('\b');
				break;
			case 'f':
				a_thread->state = THREADTS_STRING;
				_CW_STILO_THREAD_PUTC('\f');
				break;
			case 'x':
				a_thread->state = THREADTS_STRING_HEX_CONT;
				break;
			case '\r':
				a_thread->state = THREADTS_STRING_CRLF_CONT;
				break;
			case '\n':
				_CW_STILO_THREAD_NEWLINE();

				/* Ignore. */
				a_thread->state = THREADTS_STRING;
				break;
			default:
				a_thread->state = THREADTS_STRING;
				_CW_STILO_THREAD_PUTC('\\');
				_CW_STILO_THREAD_PUTC(c);
				break;
			}
			break;
		case THREADTS_STRING_CRLF_CONT:
			switch (c) {
			case '\n':
				_CW_STILO_THREAD_NEWLINE();

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
				stiloe_p_thread_syntax_error(a_thread,
				    a_threadp, "(", "\\x", c);
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
					val =
					    (a_thread->m.s.hex_val
					    - '0') << 4;
					break;
				case 'a': case 'b': case 'c': case 'd':
				case 'e': case 'f':
					val = ((a_thread->m.s.hex_val
					    - 'a') + 10) << 4;
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
				_CW_STILO_THREAD_PUTC(val);
				break;
			}
			default: {
				cw_uint8_t	suffix[] = "\\x?";

				suffix[2] = a_thread->m.s.hex_val;
				stiloe_p_thread_syntax_error(a_thread,
				    a_threadp, "(", suffix, c);
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
				_CW_STILO_THREAD_NEWLINE();
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
					stiloe_p_thread_name_accept(a_thread);
				} else {
					switch (a_thread->m.m.action) {
					case ACTION_LITERAL:
						stiloe_p_thread_syntax_error(
						    a_thread, a_threadp, "/",
						    "", c);
						break;
					case ACTION_EVALUATE:
						stiloe_p_thread_syntax_error(
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
				_CW_STILO_THREAD_PUTC(c);
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
stiloe_p_thread_tok_str_expand(cw_stiloe_thread_t *a_thread)
{
	if (a_thread->index == _CW_STILO_THREAD_BUFFER_SIZE) {
		/*
		 * First overflow, initial expansion needed.
		 */
		a_thread->tok_str = (cw_uint8_t *)_cw_malloc(a_thread->index *
		    2);
		a_thread->buffer_len = a_thread->index * 2;
		memcpy(a_thread->tok_str, a_thread->buffer,
		    a_thread->index);
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
stiloe_p_thread_syntax_error(cw_stiloe_thread_t *a_thread, cw_stilo_threadp_t
    *a_threadp, cw_uint8_t *a_prefix, cw_uint8_t *a_suffix, cw_uint8_t a_c)
{
	cw_stilo_t	*stilo, *currenterror, *key, *val;
	cw_uint32_t	line, column;

	stilo = stilo_stack_push(&a_thread->ostack);

	stilo_string_new(stilo, a_thread->stil, a_thread->locking,
	    strlen(a_prefix) + a_thread->index + strlen(a_suffix) + 1);
	stilo_attrs_set(stilo, STILOA_EXECUTABLE);

	/* Prefix. */
	stilo_string_set(stilo, 0, a_prefix, strlen(a_prefix));

	/* Main text. */
	stilo_string_set(stilo, strlen(a_prefix), a_thread->tok_str,
	    a_thread->index);

	/* Suffix. */
	stilo_string_set(stilo, strlen(a_prefix) + a_thread->index, a_suffix,
	    strlen(a_suffix));

	/* Current character. */
	stilo_string_set(stilo, strlen(a_prefix) + a_thread->index +
	    strlen(a_suffix), &a_c, 1);

	stiloe_p_thread_reset(a_thread);

	/*
	 * Set line and column in currenterror.  Look up currenterror on dstack,
	 * since there is the possibility that the user has done something silly
	 * like undef it.
	 */
	stilo_threadp_position_get(a_threadp, &line, &column);

	currenterror = stilo_stack_push(&a_thread->tstack);
	key = stilo_stack_push(&a_thread->tstack);
	val = stilo_stack_push(&a_thread->tstack);

	stilo_name_new(key, a_thread->stil,
	    stiln_str(STILN_currenterror), stiln_len(STILN_currenterror), TRUE);
	if (stilo_thread_dstack_search(&a_thread->self, key, currenterror)) {
		/* Couldn't find currenterror.  Don't record line and column. */
		goto ERROR;
	}

	stilo_name_new(key, a_thread->stil, stiln_str(STILN_line),
	    stiln_len(STILN_line), TRUE);
	stilo_integer_new(val, (cw_stiloi_t)line);
	stilo_dict_def(currenterror, a_thread->stil, key, val);

	stilo_name_new(key, a_thread->stil, stiln_str(STILN_column),
	    stiln_len(STILN_column), TRUE);
	/*
	 * If the syntax error happened at a newline, the column number won't
	 * be correct, so use 0.
	 */
	if (column == -1)
		stilo_integer_new(val, 0);
	else
		stilo_integer_new(val, (cw_stiloi_t)column);
	stilo_dict_def(currenterror, a_thread->stil, key, val);

	ERROR:
	stilo_stack_npop(&a_thread->tstack, 3);

	/* Finally, throw a syntaxerror. */
	stilo_thread_error(&a_thread->self, STILO_THREADE_SYNTAXERROR);
}

static void
stiloe_p_thread_reset(cw_stiloe_thread_t *a_thread)
{
	a_thread->state = THREADTS_START;
	if (a_thread->index > _CW_STILO_THREAD_BUFFER_SIZE) {
		_cw_free(a_thread->tok_str);
		a_thread->tok_str = a_thread->buffer;
	}
	a_thread->index = 0;
}

static void
stiloe_p_thread_procedure_accept(cw_stiloe_thread_t *a_thread)
{
	cw_stilo_t	*tstilo, *stilo;
	cw_uint32_t	nelements, i, depth;

	/* Find the no "mark". */
	for (i = 0, depth = stilo_stack_count(&a_thread->ostack), stilo = NULL;
	     i < depth; i++) {
		stilo = stilo_stack_down_get(&a_thread->ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_NO)
			break;
	}
	_cw_assert(i < depth);

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	tstilo = stilo_stack_push(&a_thread->tstack);
	stilo_array_new(tstilo, a_thread->stil, a_thread->locking, nelements);
	stilo_attrs_set(tstilo, STILOA_EXECUTABLE);

	/*
	 * Traverse down the stack, moving stilo's to the array.
	 */
	for (i = nelements, stilo = NULL; i > 0; i--) {
		stilo = stilo_stack_down_get(&a_thread->ostack, stilo);
		stilo_array_el_set(tstilo, stilo, i - 1);
	}

	/* Pop the stilo's off the stack now. */
	stilo_stack_npop(&a_thread->ostack, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stilo_stack_push(&a_thread->ostack);
	stilo_dup(stilo, tstilo);
	stilo_stack_pop(&a_thread->tstack);
}

static void
stiloe_p_thread_name_accept(cw_stiloe_thread_t *a_thread)
{
	cw_stilo_t	*stilo;

	switch (a_thread->m.m.action) {
	case ACTION_EXECUTE:
		if (a_thread->defer_count == 0) {
			/*
			 * Find the the value associated with the name in the
			 * dictionary stack, push it onto the execution stack,
			 * and run the execution loop.
			 */
			stilo = stilo_stack_push(&a_thread->estack);
			stilo_name_new(stilo, a_thread->stil,
			    a_thread->tok_str, a_thread->index, FALSE);
			stilo_attrs_set(stilo, STILOA_EXECUTABLE);

			stiloe_p_thread_reset(a_thread);
			stilo_thread_loop(&a_thread->self);
		} else {
			/* Push the name object onto the data stack. */
			stilo = stilo_stack_push(&a_thread->ostack);
			stilo_name_new(stilo, a_thread->stil, a_thread->tok_str,
			    a_thread->index, FALSE);
			stilo_attrs_set(stilo, STILOA_EXECUTABLE);
			stiloe_p_thread_reset(a_thread);
		}
		break;
	case ACTION_LITERAL:
		/* Push the name object onto the data stack. */
		stilo = stilo_stack_push(&a_thread->ostack);
		stilo_name_new(stilo, a_thread->stil, a_thread->tok_str,
		    a_thread->index, FALSE);
		stiloe_p_thread_reset(a_thread);
		break;
	case ACTION_EVALUATE: {
		cw_stilo_t	*key;

		/*
		 * Find the value associated with the name in the dictionary
		 * stack and push the value onto the data stack.
		 */
		key = stilo_stack_push(&a_thread->estack);
		stilo_name_new(key, a_thread->stil, a_thread->tok_str,
		    a_thread->index, FALSE);
		stiloe_p_thread_reset(a_thread);

		stilo = stilo_stack_push(&a_thread->ostack);
		if (stilo_thread_dstack_search(&a_thread->self, key, stilo)) {
			stilo_stack_pop(&a_thread->ostack);
			stilo_thread_error(&a_thread->self,
			    STILO_THREADE_UNDEFINED);
		}
		stilo_stack_pop(&a_thread->estack);

		break;
	}
	default:
		_cw_not_reached();
	}
}
