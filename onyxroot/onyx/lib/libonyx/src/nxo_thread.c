/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#define CW_NXO_THREAD_C_

#include "../include/libonyx/libonyx.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#ifdef CW_REAL
#include <math.h>
#endif

#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_array_l.h"
#ifdef CW_REGEX
#include "../include/libonyx/nxo_regex_l.h"
#endif
#include "../include/libonyx/nxo_thread_l.h"

#define CW_NXO_THREAD_GETC(a_i) a_thread->tok_str[(a_i)]

#define CW_NXO_THREAD_PUTC(a_c)						\
    do									\
    {									\
	if (a_thread->index >= CW_NXO_THREAD_BUFFER_SIZE)		\
	{								\
	    nxoe_p_thread_tok_str_expand(a_thread);			\
	}								\
	a_thread->tok_str[a_thread->index] = (a_c);			\
	a_thread->index++;						\
    } while (0)

/* Make a note that a '\n' was just seen.  The line and column counters will be
 * updated before the next character is seen. */
#define CW_NXO_THREAD_NEWLINE() newline = 1

/* Prototype for automatically generated function. */
void
nxo_p_thread_nxcode(cw_nxo_t *a_thread);

static void
nxo_p_thread_start(cw_nxo_t *a_nxo);
#ifdef CW_THREADS
static void *
nxo_p_thread_entry(void *a_arg);
#endif
static uint32_t
nxoe_p_thread_feed(cw_nxoe_thread_t *a_thread, cw_nxo_threadp_t *a_threadp,
		   bool a_token, const char *a_str,
		   uint32_t a_len);
static void
nxoe_p_thread_tok_str_expand(cw_nxoe_thread_t *a_thread);
static void
nxoe_p_thread_syntax_error(cw_nxoe_thread_t *a_thread,
			   cw_nxo_threadp_t *a_threadp,
			   uint32_t a_defer_base, char *a_prefix,
			   char *a_suffix, int32_t a_c);
static void
nxoe_p_thread_reset(cw_nxoe_thread_t *a_thread);
static bool
nxoe_p_thread_integer_accept(cw_nxoe_thread_t *a_thread);
#ifdef CW_REAL
static bool
nxoe_p_thread_real_accept(cw_nxoe_thread_t *a_thread);
#endif
static void
nxoe_p_thread_procedure_accept(cw_nxoe_thread_t *a_thread,
			       cw_nxo_threadp_t *a_threadp);
static void
nxoe_p_thread_name_accept(cw_nxoe_thread_t *a_thread);

#ifdef CW_DBG
#define CW_NXO_THREADP_MAGIC 0xdfe76a68
#endif

/* nxo_threadp. */
void
nxo_threadp_new(cw_nxo_threadp_t *a_threadp)
{
    cw_check_ptr(a_threadp);

    a_threadp->origin = NULL;
    a_threadp->olen = 0;

    a_threadp->line = 1;
    a_threadp->column = 0;

#ifdef CW_DBG
    a_threadp->magic = CW_NXO_THREADP_MAGIC;
#endif
}

void
nxo_threadp_delete(cw_nxo_threadp_t *a_threadp, cw_nxo_t *a_thread)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_thread);
    cw_dassert(a_thread->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_thread->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    switch (thread->state)
    {
	case THREADTS_START:
	{
	    /* No problem. */
	    break;
	}
	case THREADTS_COMMENT:
	{
	    /* No problem. */
	    nxoe_p_thread_reset(thread);
	    break;
	}
	case THREADTS_STRING:
	case THREADTS_STRING_NEWLINE_CONT:
	case THREADTS_STRING_PROT_CONT:
	case THREADTS_STRING_CRLF_CONT:
	case THREADTS_STRING_CTRL_CONT:
	case THREADTS_STRING_HEX_CONT:
	case THREADTS_STRING_HEX_FINISH:
	{
	    cw_nxoe_thread_t *thread;

	    cw_check_ptr(a_thread);
	    cw_dassert(a_thread->magic == CW_NXO_MAGIC);

	    thread = (cw_nxoe_thread_t *) a_thread->o.nxoe;
	    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	    cw_assert(thread->nxoe.type == NXOT_THREAD);

	    nxoe_p_thread_syntax_error(thread, a_threadp, 0, "`", "", -1);
	    break;
	}
	case THREADTS_NAME_START:
	{
	    cw_nxoe_thread_t *thread;
	    char suffix[2] = "?";

	    cw_check_ptr(a_thread);
	    cw_dassert(a_thread->magic == CW_NXO_MAGIC);

	    thread = (cw_nxoe_thread_t *) a_thread->o.nxoe;
	    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
	    cw_assert(thread->nxoe.type == NXOT_THREAD);

	    switch (thread->m.m.action)
	    {
		case ACTION_EXECUTE:
		{
		    suffix[0] = '\0';
		    break;
		}
		case ACTION_EVALUATE:
		{
		    suffix[0] = '!';
		    break;
		}
#ifdef CW_OOP
		case ACTION_CALL:
		{
		    suffix[0] = ':';
		    break;
		}
		case ACTION_INVOKE:
		{
		    suffix[0] = ';';
		    break;
		}
		case ACTION_FETCH:
		{
		    suffix[0] = ',';
		    break;
		}
#endif
		case ACTION_LITERAL:
		{
		    suffix[0] = '$';
		    break;
		}
		case ACTION_IMMEDIATE:
		{
		    suffix[0] = '~';
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }

	    nxoe_p_thread_syntax_error(thread, a_threadp, 0, "", suffix, -1);
	    break;
	}
	case THREADTS_INTEGER:
	case THREADTS_INTEGER_RADIX:
	case THREADTS_NAME:
	{
	    /* Accept the name or integer. */
	    nxo_thread_flush(a_thread, a_threadp);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

#ifdef CW_DBG
    memset(a_threadp, 0x5a, sizeof(cw_nxo_threadp_t));
#endif
}

void
nxo_threadp_origin_get(const cw_nxo_threadp_t *a_threadp,
		       const char **r_origin, uint32_t *r_olen)
{
    cw_check_ptr(a_threadp);
    cw_dassert(a_threadp->magic == CW_NXO_THREADP_MAGIC);

    *r_origin = a_threadp->origin;
    *r_olen = a_threadp->olen;
}

void
nxo_threadp_origin_set(cw_nxo_threadp_t *a_threadp,
			 const char *a_origin, uint32_t a_olen)
{
    cw_check_ptr(a_threadp);
    cw_dassert(a_threadp->magic == CW_NXO_THREADP_MAGIC);

    a_threadp->origin = a_origin;
    a_threadp->olen = a_olen;
}

void
nxo_threadp_position_get(const cw_nxo_threadp_t *a_threadp, uint32_t *r_line,
			 uint32_t *r_column)
{
    cw_check_ptr(a_threadp);
    cw_dassert(a_threadp->magic == CW_NXO_THREADP_MAGIC);

    *r_line = a_threadp->line;
    *r_column = a_threadp->column;
}

void
nxo_threadp_position_set(cw_nxo_threadp_t *a_threadp, uint32_t a_line,
			 uint32_t a_column)
{
    cw_check_ptr(a_threadp);
    cw_dassert(a_threadp->magic == CW_NXO_THREADP_MAGIC);

    a_threadp->line = a_line;
    a_threadp->column = a_column;
}

/* nxo_thread. */
void
nxo_thread_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *nxo;

    thread = (cw_nxoe_thread_t *) nxa_calloc(1, sizeof(cw_nxoe_thread_t));

    nxoe_l_new(&thread->nxoe, NXOT_THREAD, false);

    /* Set things to a state that won't cause the GC (or any thread-related
     * operators) to puke. */

    /* Fake up a nxo for self. */
    nxo_p_new(&thread->self, NXOT_THREAD);
    thread->self.o.nxoe = (cw_nxoe_t *) thread;

    thread->nx = a_nx;
    thread->tok_str = thread->buffer;

    thread->maxestack = nx_maxestack_get(a_nx);

    thread->tailopt = nx_tailopt_get(a_nx);

    nxo_no_new(&thread->estack);
    nxo_no_new(&thread->istack);
    nxo_no_new(&thread->ostack);
    nxo_no_new(&thread->dstack);
#ifdef CW_OOP
    nxo_no_new(&thread->cstack);
#endif
    nxo_no_new(&thread->tstack);
    nxo_no_new(&thread->stdin_nxo);
    nxo_no_new(&thread->stdout_nxo);
    nxo_no_new(&thread->stderr_nxo);
    nxo_no_new(&thread->trapped_arg);
#ifdef CW_REGEX
    nxo_l_regex_cache_new(&thread->regex_cache);
#endif

    /* Register this thread with the interpreter so that the GC will be able to
     * get to it. */
    nx_l_thread_insert(a_nx, &thread->self);

    /* Register with the GC so that this thread will be iterated on during
     * GC. */
    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) thread;
#ifdef CW_DBG
    a_nxo->magic = CW_NXO_MAGIC;
#endif
    nxo_p_type_set(a_nxo, NXOT_THREAD);

    nxa_l_gc_register((cw_nxoe_t *) thread);

    /* Finish setting up the internals. */
    nxo_stack_new(&thread->estack, false, CW_LIBONYX_ESTACK_MINCOUNT);
    nxo_stack_new(&thread->istack, false, CW_LIBONYX_ISTACK_MINCOUNT);
    nxo_stack_new(&thread->ostack, false, CW_LIBONYX_OSTACK_MINCOUNT);
    nxo_stack_new(&thread->dstack, false, CW_LIBONYX_DSTACK_MINCOUNT);
#ifdef CW_OOP
    nxo_stack_new(&thread->cstack, false, CW_LIBONYX_CSTACK_MINCOUNT);
#endif
    nxo_stack_new(&thread->tstack, false, CW_LIBONYX_TSTACK_MINCOUNT);

    nxo_dup(&thread->stdin_nxo, nx_stdin_get(a_nx));
    nxo_dup(&thread->stdout_nxo, nx_stdout_get(a_nx));
    nxo_dup(&thread->stderr_nxo, nx_stderr_get(a_nx));

    /* Push threaddict, systemdict, and globaldict, onto the dictionary stack.
     * The embedded onyx initialization code creates userdict. */
    nxo = nxo_stack_push(&thread->dstack);
    nxo_dict_new(nxo, false, CW_LIBONYX_THREADDICT_HASH);

    nxo = nxo_stack_push(&thread->dstack);
    nxo_dup(nxo, nx_systemdict_get(a_nx));

    nxo = nxo_stack_push(&thread->dstack);
    nxo_dup(nxo, nx_globaldict_get(a_nx));

    /* Run per-thread embedded initialization code. */
    nxo_p_thread_nxcode(&thread->self);

    /* Execute the thread initialization hook if set. */
    if (nx_l_thread_init(a_nx) != NULL)
    {
	nx_l_thread_init(a_nx)(&thread->self);
    }
}

/* Called by thread start hook (if set). */
static void
nxo_p_thread_start(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *start;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    start = nxo_stack_push(&thread->estack);
    nxo_operator_new(start, systemdict_start, NXN_start);
    nxo_attr_set(start, NXOA_EXECUTABLE);

    nxo_thread_loop(a_nxo);
}

void
nxo_thread_start(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *start;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    if (nx_l_thread_start(thread->nx) != NULL)
    {
	nx_l_thread_start(thread->nx)(&thread->self, nxo_p_thread_start);
    }
    else
    {
	start = nxo_stack_push(&thread->estack);
	nxo_operator_new(start, systemdict_start, NXN_start);
	nxo_attr_set(start, NXOA_EXECUTABLE);

	nxo_thread_loop(a_nxo);
    }
}

void
nxo_thread_exit(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    nx_l_thread_remove(thread->nx, a_nxo);
}

#ifdef CW_THREADS
static void *
nxo_p_thread_entry(void *a_arg)
{
    cw_nxoe_thread_t *thread = (cw_nxoe_thread_t *) a_arg;

    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    /* Run. */
    nxo_thread_start(&thread->self);

    /* Wait to be joined or detached, if not already so. */
    mtx_lock(&thread->lock);
    thread->done = true;
    while (thread->detached == false && thread->joined == false)
    {
	cnd_wait(&thread->done_cnd, &thread->lock);
    }
    if (thread->detached)
    {
	mtx_unlock(&thread->lock);

	/* Clean up. */
	cnd_delete(&thread->join_cnd);
	cnd_delete(&thread->done_cnd);
	mtx_delete(&thread->lock);
	nx_l_thread_remove(thread->nx, &thread->self);
	thd_delete(thread->thd);
    }
    else if (thread->joined)
    {
	/* Wake the joiner back up. */
	cnd_signal(&thread->join_cnd);
	/* We're done.  The joiner will clean up. */
	thread->gone = true;
	mtx_unlock(&thread->lock);
    }
    else
    {
	cw_not_reached();
    }

    return NULL;
}

void
nxo_thread_thread(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    mtx_new(&thread->lock);
    cnd_new(&thread->done_cnd);
    cnd_new(&thread->join_cnd);
    thread->done = false;
    thread->gone = false;
    thread->detached = false;
    thread->joined = false;

    thread->thd = thd_new(nxo_p_thread_entry, (void *) thread, true);
}

void
nxo_thread_detach(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    mtx_lock(&thread->lock);
    thread->detached = true;
    if (thread->done)
    {
	/* The thread is already done, so wake it back up. */
	cnd_signal(&thread->done_cnd);
    }
    mtx_unlock(&thread->lock);
}

void
nxo_thread_join(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    mtx_lock(&thread->lock);
    thread->joined = true;
    if (thread->done)
    {
	/* The thread is already done, so wake it back up. */
	cnd_signal(&thread->done_cnd);
    }
    /* Wait for the thread to totally go away. */
    while (thread->gone == false)
    {
	cnd_wait(&thread->join_cnd, &thread->lock);
    }
    mtx_unlock(&thread->lock);

    /* Clean up. */
    cnd_delete(&thread->join_cnd);
    cnd_delete(&thread->done_cnd);
    mtx_delete(&thread->lock);
    thd_join(thread->thd);

    nx_l_thread_remove(thread->nx, a_nxo);
}
#endif

cw_nxo_threadts_t
nxo_thread_state(const cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    return thread->state;
}

bool
nxo_thread_deferred(cw_nxo_t *a_nxo)
{
    bool retval;
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    if (thread->defer_count != 0)
    {
	retval = true;
    }
    else
    {
	retval = false;
    }

    return retval;
}

void
nxo_thread_reset(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    thread->defer_count = 0;
    nxoe_p_thread_reset(thread);
}

void
nxo_thread_loop(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *nxo, *tnxo, *inxo;
#ifdef CW_DBG
    uint32_t tdepth;
#endif

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

#ifdef CW_DBG
    /* The assertions about stack depth in this function check for tstack leaks
     * in operators. */
    tdepth = nxo_stack_count(&thread->tstack);
#endif
    /* Push an integer onto istack that corresponds to the object on estack
     * we're executing. */
    inxo = nxo_stack_push(&thread->istack);
    nxo_integer_new(inxo, 0);
    cw_assert(nxo_stack_count(&thread->estack) ==
	      nxo_stack_count(&thread->istack));

    if (nxo_stack_count(&thread->estack) == thread->maxestack + 1)
    {
	nxo_thread_nerror(a_nxo, NXN_estackoverflow);
	nxo_stack_pop(&thread->estack);
	goto DONE;
    }

    RESTART:
    nxo = nxo_stack_get(&thread->estack);
    switch (nxo_attr_get(nxo))
    {
	case NXOA_LITERAL:
	{
	    /* Always push literal objects onto the operand stack. */
	    tnxo = nxo_stack_push(&thread->ostack);
	    nxo_dup(tnxo, nxo);
	    nxo_stack_pop(&thread->estack);
	    break;
	}
	case NXOA_EXECUTABLE:
	case NXOA_EVALUABLE:
	{
	    switch (nxo_type_get(nxo))
	    {
		case NXOT_ARRAY:
		{
		    uint32_t i, len, tailopt;
#ifdef CW_THREADS
		    bool alocking;
#endif
		    cw_nxo_t *el;
		    cw_nxoa_t attr;

		    len = nxo_array_len_get(nxo);
		    if (len == 0)
		    {
			nxo_stack_pop(&thread->estack);
			break;
		    }

		    /* Iterate through the array and execute each element in
		     * turn.  The generic algorithm is simply to push an element
		     * onto estack and recurse, but the overhead of the pushing,
		     * recursion, and popping is excessive for the common cases
		     * of a simple object or operator.  Therefore, check for the
		     * most common simple cases and handle them specially. */

#ifdef CW_THREADS
		    alocking = nxo_l_array_locking(nxo);
		    if (alocking)
		    {
			nxo_l_array_lock(nxo);
		    }
#endif

		    /* thread->tailopt may change between now and when it is
		     * last used for execution of this array, so a copy must be
		     * made and used in order avoid problems that could ensue
		     * otherwise. */
		    for (i = 0, tailopt = thread->tailopt;
			 i < len - tailopt;
			 i++)
		    {
			el = nxo_l_array_el_get(nxo, i);
			attr = nxo_attr_get(el);
			if (attr == NXOA_LITERAL)
			{
			    /* Always push literal objects onto the operand
			     * stack. */
			    tnxo = nxo_stack_push(&thread->ostack);
			    nxo_dup(tnxo, el);
			    continue;
			}

			/* Set the execution index. */
			nxo_integer_set(inxo, i);

			cw_assert(nxo_type_get(el) <= NXOT_LAST);
			switch (nxo_type_get(el))
			{
			    case NXOT_ARRAY:
			    {
				/* Only execute nested arrays that have the
				 * evaluable attribute. */
				if (attr == NXOA_EVALUABLE)
				{
				    tnxo = nxo_stack_push(&thread->estack);
				    nxo_dup(tnxo, el);
#ifdef CW_THREADS
				    if (alocking)
				    {
					nxo_l_array_unlock(nxo);
					nxo_thread_loop(a_nxo);
					nxo_l_array_lock(nxo);
				    }
				    else
#endif
				    {
					nxo_thread_loop(a_nxo);
				    }
				}
				else
				{
				    tnxo = nxo_stack_push(&thread->ostack);
				    nxo_dup(tnxo, el);
				}
				break;
			    }
			    case NXOT_BOOLEAN:
#ifdef CW_OOP
			    case NXOT_CLASS:
#endif
#ifdef CW_THREADS
			    case NXOT_CONDITION:
#endif
			    case NXOT_DICT:
			    case NXOT_FINO:
#ifdef CW_OOP
			    case NXOT_INSTANCE:
#endif
			    case NXOT_INTEGER:
			    case NXOT_MARK:
#ifdef CW_THREADS
			    case NXOT_MUTEX:
#endif
			    case NXOT_PMARK:
#ifdef CW_REAL
			    case NXOT_REAL:
#endif
#ifdef CW_REGEX
			    case NXOT_REGEX:
			    case NXOT_REGSUB:
#endif
			    case NXOT_STACK:
			    case NXOT_THREAD:
			    {
				/* Always push the object onto the operand
				 * stack, even though it isn't literal. */
				tnxo = nxo_stack_push(&thread->ostack);
				nxo_dup(tnxo, el);
				break;
			    }
			    case NXOT_FILE:
			    {
				cw_nxo_threadp_t threadp;
				int32_t nread;
				char buffer[CW_LIBONYX_FILE_EVAL_READ_SIZE];

#ifdef CW_THREADS
				if (alocking)
				{
				    nxo_l_array_unlock(nxo);
				}
#endif
				nxo_threadp_new(&threadp);
				{
				    const char *origin;
				    uint32_t olen;

				    nxo_file_origin_get(el, &origin, &olen);
				    nxo_threadp_origin_set(&threadp, origin,
							   olen);
				}
				/* Read data from the file and interpret it
				 * until an EOF (0 byte read). */
				for (nread = nxo_file_read(el,
							   CW_LIBONYX_FILE_EVAL_READ_SIZE,
							   buffer);
				     nread > 0;
				     nread = nxo_file_read(el,
							   CW_LIBONYX_FILE_EVAL_READ_SIZE,
							   buffer))
				{
				    nxo_thread_interpret(a_nxo, &threadp,
							 buffer, nread);
				}
				/* Do not flush, so that syntax errors get
				 * caught. */
				nxo_threadp_delete(&threadp, a_nxo);
#ifdef CW_THREADS
				if (alocking)
				{
				    nxo_l_array_lock(nxo);
				}
#endif
				break;
			    }
#ifdef CW_HANDLE
			    case NXOT_HANDLE:
			    {
				cw_nxo_t *handle;

				handle = nxo_stack_push(&thread->tstack);
				nxo_dup(handle, el);
#ifdef CW_THREADS
				if (alocking)
				{
				    nxo_l_array_unlock(nxo);
				    nxo_handle_eval(handle, a_nxo);
				    nxo_stack_pop(&thread->tstack);
				    nxo_l_array_lock(nxo);
				}
				else
#endif
				{
				    nxo_handle_eval(handle, a_nxo);
				    nxo_stack_pop(&thread->tstack);
				}
				break;
			    }
#endif
			    case NXOT_NAME:
			    {
				/* There is no way to evaluate the result of the
				 * name lookup without growing estack, so set up
				 * estack and recurse. */
				tnxo = nxo_stack_push(&thread->estack);
				nxo_dup(tnxo, el);
#ifdef CW_THREADS
				if (alocking)
				{
				    nxo_l_array_unlock(nxo);
				    nxo_thread_loop(a_nxo);
				    nxo_l_array_lock(nxo);
				}
				else
#endif
				{
				    nxo_thread_loop(a_nxo);
				}
				break;
			    }
			    case NXOT_NULL:
			    {
				/* Do nothing. */
				break;
			    }
			    case NXOT_OPERATOR:
			    {
#ifdef CW_THREADS
				if (alocking)
				{
				    nxo_l_array_unlock(nxo);
				    nxo_operator_f(el)(a_nxo);
				    nxo_l_array_lock(nxo);
				}
				else
#endif
				{
				    nxo_operator_f(el)(a_nxo);
				}
				break;
			    }
			    case NXOT_STRING:
			    {
				cw_nxo_t *string;
				cw_nxo_threadp_t threadp;

				/* Use the string as a source of code. */
				string = nxo_stack_push(&thread->tstack);
				nxo_dup(string, el);
#ifdef CW_THREADS
				if (alocking)
				{
				    nxo_l_array_unlock(nxo);
				}
#endif
				nxo_threadp_new(&threadp);
				nxo_string_lock(string);
				nxo_thread_interpret(a_nxo, &threadp,
						     nxo_string_get(string),
						     nxo_string_len_get(string));
				nxo_string_unlock(string);
				nxo_thread_flush(a_nxo, &threadp);
				nxo_threadp_delete(&threadp, a_nxo);
#ifdef CW_THREADS
				if (alocking)
				{
				    nxo_l_array_lock(nxo);
				}
#endif
				nxo_stack_pop(&thread->tstack);
				break;
			    }
			    default:
			    {
				cw_not_reached();
			    }
			}
			cw_assert(nxo_stack_count(&thread->tstack) == tdepth);
		    }

		    /* Set the index back to 0 now that we're not executing an
		     * array any more. */
		    nxo_integer_set(inxo, 0);

		    if (tailopt)
		    {
			/* Optimize tail calls.  If recursion is possible and
			 * likely, make tail recursion safe by replacing the
			 * array with its last element before executing the last
			 * element. */
			el = nxo_l_array_el_get(nxo, i);
			attr = nxo_attr_get(el);
			if ((attr == NXOA_LITERAL)
			    || (nxo_type_get(el) == NXOT_ARRAY
				&& attr == NXOA_EXECUTABLE))
			{
			    /* Always push literal objects and nested executable
			     * (not evaluable) arrays onto the operand stack. */
			    tnxo = nxo_stack_push(&thread->ostack);
			    nxo_dup(tnxo, el);
#ifdef CW_THREADS
			    if (alocking)
			    {
				nxo_l_array_unlock(nxo);
			    }
#endif
			    nxo_stack_pop(&thread->estack);
			}
			else
			{
			    /* Possible recursion. */
			    tnxo = nxo_stack_push(&thread->tstack);
			    nxo_dup(tnxo, el);
#ifdef CW_THREADS
			    if (alocking)
			    {
				nxo_l_array_unlock(nxo);
			    }
#endif
			    nxo_dup(nxo, tnxo);
			    nxo_stack_pop(&thread->tstack);
			    goto RESTART;
			}
		    }
		    else
		    {
			/* Do not optimize tail calls. */
#ifdef CW_THREADS
			if (alocking)
			{
			    nxo_l_array_unlock(nxo);
			}
#endif
			nxo_stack_pop(&thread->estack);
		    }
		    break;
		}
		case NXOT_BOOLEAN:
#ifdef CW_OOP
		case NXOT_CLASS:
#endif
#ifdef CW_THREADS
		case NXOT_CONDITION:
#endif
		case NXOT_DICT:
		case NXOT_FINO:
#ifdef CW_OOP
		case NXOT_INSTANCE:
#endif
		case NXOT_INTEGER:
		case NXOT_MARK:
#ifdef CW_THREADS
		case NXOT_MUTEX:
#endif
		case NXOT_PMARK:
#ifdef CW_REAL
		case NXOT_REAL:
#endif
#ifdef CW_REGEX
		case NXOT_REGEX:
		case NXOT_REGSUB:
#endif
		case NXOT_STACK:
		case NXOT_THREAD:
		{
		    /* Always push the object onto the operand stack, even
		     * though it isn't literal. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, nxo);
		    nxo_stack_pop(&thread->estack);
		    break;
		}
		case NXOT_FILE:
		{
		    cw_nxo_threadp_t threadp;
		    int32_t nread;
		    char buffer[CW_LIBONYX_FILE_EVAL_READ_SIZE];

		    nxo_threadp_new(&threadp);
		    {
			const char *origin;
			uint32_t olen;

			nxo_file_origin_get(nxo, &origin, &olen);
			nxo_threadp_origin_set(&threadp, origin, olen);
		    }
		    /* Read data from the file and interpret it until an EOF (0
		     * byte read). */
		    for (nread = nxo_file_read(nxo,
					       CW_LIBONYX_FILE_EVAL_READ_SIZE,
					       buffer);
			 nread > 0;
			 nread = nxo_file_read(nxo,
					       CW_LIBONYX_FILE_EVAL_READ_SIZE,
					       buffer))
		    {
			nxo_thread_interpret(a_nxo, &threadp, buffer, nread);
		    }
		    /* Do not flush, so that syntax errors get caught. */
		    nxo_threadp_delete(&threadp, a_nxo);

		    nxo_stack_pop(&thread->estack);
		    break;
		}
#ifdef CW_HANDLE
		case NXOT_HANDLE:
		{
		    nxo_handle_eval(nxo, a_nxo);

		    nxo_stack_pop(&thread->estack);
		    break;
		}
#endif
		case NXOT_NAME:
		{
		    uint32_t tailopt;
		    cw_nxo_t *name, *value;
		    cw_nxn_t error;

		    /* Search for a value associated with the name in the
		     * dictionary stack, and put it on the execution stack, in
		     * preparation for restarting the enclosing switch statement
		     * (or recursing, if tail optimization is disabled).  Thus,
		     * nested name lookups work correctly. */

		    /* Store a copy of tailopt for later use.  This is only
		     * strictly necessary for cases in which the interpreter
		     * recurses, but that never happens here.  Still, it's
		     * cleaner. */
		    tailopt = thread->tailopt;
		    if (tailopt)
		    {
			name = nxo_stack_push(&thread->tstack);
			nxo_dup(name, nxo);
			value = nxo;
		    }
		    else
		    {
			name = nxo;
			value = nxo_stack_push(&thread->estack);
		    }

		    /* Handle the name. */
		    cw_assert(nxo_attr_get(name) == NXOA_EXECUTABLE
			      || nxo_attr_get(name) == NXOA_EVALUABLE);
		    if (nxo_thread_dstack_search(a_nxo, name, value))
		    {
			error = NXN_undefined;
			goto NAME_ERROR;
		    }

		    /* Finish up.  Restart the outer switch statement, unless
		     * tail optimization is disabled; recurse in that case. */
		    if (tailopt)
		    {
			nxo_stack_pop(&thread->tstack);
			goto RESTART;
		    }
		    else
		    {
			nxo_thread_loop(a_nxo);
			nxo_stack_pop(&thread->estack);
			break;
		    }

		    NAME_ERROR:
		    if (tailopt)
		    {
			nxo_stack_pop(&thread->tstack);
		    }
		    else
		    {
			nxo_stack_pop(&thread->estack);
		    }
		    nxo_thread_nerror(a_nxo, error);
		    nxo_stack_pop(&thread->estack);
		    break;
		}
		case NXOT_NULL:
		{
		    /* Do nothing. */
		    nxo_stack_pop(&thread->estack);
		    break;
		}
		case NXOT_OPERATOR:
		{
		    nxo_operator_f(nxo)(a_nxo);
		    nxo_stack_pop(&thread->estack);
		    break;
		}
		case NXOT_STRING:
		{
		    cw_nxo_threadp_t threadp;

		    /* Use the string as a source of code. */
		    nxo_threadp_new(&threadp);
#ifdef CW_THREADS
		    nxo_string_lock(nxo);
#endif
		    nxo_thread_interpret(a_nxo, &threadp, nxo_string_get(nxo),
					 nxo_string_len_get(nxo));
#ifdef CW_THREADS
		    nxo_string_unlock(nxo);
#endif
		    nxo_thread_flush(a_nxo, &threadp);
		    nxo_threadp_delete(&threadp, a_nxo);
		    nxo_stack_pop(&thread->estack);
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }
	    cw_assert(nxo_stack_count(&thread->tstack) == tdepth);
	    break;
	}
#ifdef CW_OOP
	case NXOA_CALLABLE:
	{
	    cw_nxo_t *value, *instance, *class_, *cnxo;
	    uint32_t cdepth;
	    cw_nxn_t error;

	    /* Create space for value. */
	    value = nxo_stack_push(&thread->estack);

	    /* Get instance. */
	    instance = nxo_stack_get(&thread->ostack);
	    if (instance == NULL)
	    {
		error = NXN_stackunderflow;
		goto CALLABLE_ERROR;
	    }
	    switch (nxo_type_get(instance))
	    {
		case NXOT_CLASS:
		{
		    class_ = instance;
		    break;
		}
		case NXOT_INSTANCE:
		{
		    class_ = nxo_instance_isa_get(instance);
		    break;
		}
		default:
		{
		    error = NXN_typecheck;
		    goto CALLABLE_ERROR;
		}
	    }
	    if (nxo_thread_class_hier_search(a_nxo, class_, nxo, value))
	    {
		error = NXN_undefined;
		goto CALLABLE_ERROR;
	    }

	    /* Record cstack depth, so that cstack can be cleaned up later. */
	    cdepth = nxo_stack_count(&thread->cstack);

	    /* Push the class or instance onto cstack and pop it off ostack
	     * before recursing. */
	    cnxo = nxo_stack_push(&thread->cstack);
	    nxo_dup(cnxo, instance);
	    nxo_stack_pop(&thread->ostack);

	    /* Recurse. */
	    nxo_thread_loop(a_nxo);

	    /* Clean up. */
	    nxo_stack_pop(&thread->estack);
	    nxo_stack_npop(&thread->cstack,
			   nxo_stack_count(&thread->cstack) - cdepth);
	    break;

	    CALLABLE_ERROR:
	    nxo_stack_pop(&thread->estack);
	    nxo_thread_nerror(a_nxo, error);
	    nxo_stack_pop(&thread->estack);
	    break;
	}
	case NXOA_INVOKABLE:
	{
	    cw_nxo_t *value, *cnxo, *class_;
	    cw_nxn_t error;

	    /* Create space for value. */
	    value = nxo_stack_push(&thread->estack);

	    /* Get context for lookup. */
	    cnxo = nxo_stack_get(&thread->cstack);
	    if (cnxo == NULL)
	    {
		error = NXN_cstackunderflow;
		goto INVOKABLE_ERROR;
	    }
	    switch (nxo_type_get(cnxo))
	    {
		case NXOT_CLASS:
		{
		    class_ = cnxo;
		    break;
		}
		case NXOT_INSTANCE:
		{
		    class_ = nxo_instance_isa_get(cnxo);
		    break;
		}
		default:
		{
		    error = NXN_typecheck;
		    goto INVOKABLE_ERROR;
		}
	    }
	    if (nxo_thread_class_hier_search(a_nxo, class_, nxo, value))
	    {
		error = NXN_undefined;
		goto INVOKABLE_ERROR;
	    }

	    /* Recurse. */
	    nxo_thread_loop(a_nxo);

	    /* Clean up. */
	    nxo_stack_pop(&thread->estack);
	    break;

	    INVOKABLE_ERROR:
	    nxo_stack_pop(&thread->estack);
	    nxo_thread_nerror(a_nxo, error);
	    nxo_stack_pop(&thread->estack);
	    break;
	}
	case NXOA_FETCHABLE:
	{
	    cw_nxo_t *cnxo, *data, *value;
	    cw_nxn_t error;

	    cnxo = nxo_stack_get(&thread->cstack);
	    if (cnxo == NULL)
	    {
		error = NXN_cstackunderflow;
		goto FETCHABLE_ERROR;
	    }
	    switch (nxo_type_get(cnxo))
	    {
		case NXOT_CLASS:
		{
		    data = nxo_class_data_get(cnxo);
		    break;
		}
		case NXOT_INSTANCE:
		{
		    data = nxo_instance_data_get(cnxo);
		    break;
		}
		default:
		{
		    error = NXN_typecheck;
		    goto FETCHABLE_ERROR;
		}
	    }
	    if (nxo_type_get(data) != NXOT_DICT)
	    {
		error = NXN_undefined;
		goto FETCHABLE_ERROR;
	    }

	    value = nxo_stack_push(&thread->ostack);
	    if (nxo_dict_lookup(data, nxo, value))
	    {
		nxo_stack_pop(&thread->ostack);
		error = NXN_undefined;
		goto FETCHABLE_ERROR;
	    }

	    /* Clean up. */
	    nxo_stack_pop(&thread->estack);
	    break;

	    FETCHABLE_ERROR:
	    nxo_thread_nerror(a_nxo, error);
	    nxo_stack_pop(&thread->estack);
	    break;
	}
#endif
	default:
	{
	    cw_not_reached();
	}
    }
    DONE:

    /* Pop the execution index for this onyx stack frame. */
    nxo_stack_pop(&thread->istack);
    cw_assert(nxo_stack_count(&thread->estack) ==
	      nxo_stack_count(&thread->istack));
}

void
nxo_thread_interpret(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp, const
		     char *a_str, uint32_t a_len)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    nxoe_p_thread_feed(thread, a_threadp, false, a_str, a_len);
}

void
nxo_thread_flush(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp)
{
    cw_nxoe_thread_t *thread;
    static const char str[] = "\n";

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    nxoe_p_thread_feed(thread, a_threadp, false, str, sizeof(str) - 1);
}

void
nxo_thread_nerror(cw_nxo_t *a_nxo, cw_nxn_t a_nxn)
{
    nxo_thread_serror(a_nxo, nxn_str(a_nxn), nxn_len(a_nxn));
}

void
nxo_thread_serror(cw_nxo_t *a_nxo, const char *a_str, uint32_t a_len)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *errorname;
    uint32_t defer_count;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    /* Convert a_str to a name object on ostack. */
    errorname = nxo_stack_push(&thread->ostack);
    nxo_name_new(errorname, a_str, a_len, false);

    /* Shut off deferral temporarily.  It is possible for this C stack frame to
     * never be returned to, due to an exception (stop, quit, exit), in which
     * case, deferral will never be turned back on.  That's fine, since the user
     * is going to have to patch things up by hand in that case anyway. */
    defer_count = thread->defer_count;
    thread->defer_count = 0;

    /* Throw an error. */
    cw_onyx_code(a_nxo, "throw");

    /* Turn deferral back on. */
    thread->defer_count = defer_count;
}

bool
nxo_thread_dstack_search(cw_nxo_t *a_nxo, cw_nxo_t *a_key, cw_nxo_t *r_value)
{
    bool retval;
    cw_nxoe_thread_t *thread;
    cw_nxo_t *dict;
    uint32_t i, depth;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    /* Iteratively search the dictionaries on the dictionary stack for
     * a_key. */
    for (i = 0, depth = nxo_stack_count(&thread->dstack), dict = NULL;
	 i < depth;
	 i++)
    {
	dict = nxo_stack_nget(&thread->dstack, i);
	if (nxo_dict_lookup(dict, a_key, r_value) == false)
	{
	    /* Found. */
	    retval = false;
	    goto RETURN;
	}
    }

    retval = true;
    RETURN:
    return retval;
}

#ifdef CW_OOP
bool
nxo_thread_class_hier_search(cw_nxo_t *a_nxo, cw_nxo_t *a_class,
			     cw_nxo_t *a_key, cw_nxo_t *r_value)
{
    bool retval;
    cw_nxoe_thread_t *thread;
    cw_nxo_t *class_, *methods;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    cw_assert(nxo_type_get(a_class) == NXOT_CLASS
	      || nxo_type_get(a_class) == NXOT_NULL);

    /* Iteratively search a_class's class hierarchy for a_key. */
    for (class_ = a_class;
	 nxo_type_get(class_) == NXOT_CLASS;
	 class_ = nxo_class_super_get(class_))
    {
	methods = nxo_class_methods_get(class_);
	if (nxo_type_get(methods) == NXOT_DICT
	    && nxo_dict_lookup(methods, a_key, r_value) == false)
	{
	    /* Found. */
	    retval = false;
	    goto RETURN;
	}
    }

    retval = true;
    RETURN:
    return retval;
}
#endif

#ifdef CW_THREADS
bool
nxo_thread_currentlocking(const cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    return thread->locking;
}

void
nxo_thread_setlocking(cw_nxo_t *a_nxo, bool a_locking)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    thread->locking = a_locking;
}
#endif

void
nxo_thread_maxestack_set(cw_nxo_t *a_nxo, cw_nxoi_t a_maxestack)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(a_maxestack >= 0);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    thread->maxestack = a_maxestack;
}

void
nxo_thread_tailopt_set(cw_nxo_t *a_nxo, bool a_tailopt)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    if (a_tailopt)
    {
	thread->tailopt = 1;
    }
    else
    {
	thread->tailopt = 0;
    }
}

void
nxo_thread_stdin_set(cw_nxo_t *a_nxo, cw_nxo_t *a_stdin)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    cw_check_ptr(a_stdin);
    cw_assert(a_stdin->magic == CW_NXO_MAGIC);

    nxo_dup(&thread->stdin_nxo, a_stdin);
}

void
nxo_thread_stdout_set(cw_nxo_t *a_nxo, cw_nxo_t *a_stdout)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    cw_check_ptr(a_stdout);
    cw_assert(a_stdout->magic == CW_NXO_MAGIC);

    nxo_dup(&thread->stdout_nxo, a_stdout);
}

void
nxo_thread_stderr_set(cw_nxo_t *a_nxo, cw_nxo_t *a_stderr)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    cw_check_ptr(a_stderr);
    cw_assert(a_stderr->magic == CW_NXO_MAGIC);

    nxo_dup(&thread->stderr_nxo, a_stderr);
}

uint32_t
nxo_l_thread_token(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp, const
		   char *a_str, uint32_t a_len)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    return nxoe_p_thread_feed(thread, a_threadp, true, a_str, a_len);
}

static uint32_t
nxoe_p_thread_feed(cw_nxoe_thread_t *a_thread, cw_nxo_threadp_t *a_threadp,
		   bool a_token, const char *a_str,
		   uint32_t a_len)
{
    uint32_t retval, i, newline, defer_base;
    unsigned char c;
    cw_nxo_t *nxo;
    bool token;

    if (a_token)
    {
	/* Artificially inflate the defer count so that when we accept a token,
	 * it doesn't get evaluated. */
	defer_base = 1;
	a_thread->defer_count++;
	/* The value of token is only used when a_token is true, so only bother
	 * to initialize it in this case. */
	token = false;
    }
    else
    {
	defer_base = 0;
    }

    /* All the grossness surrounding newline avoids doing any branches when
     * calculating the line and column number.  This may be overzealous
     * optimization, but the logic is relatively simple.  We do the update of
     * both line and column number here so that they are correct at all times
     * during the main part of the loop. */
    for (i = newline = 0;
	 i < a_len;
	 i++, a_threadp->line += newline,
	     a_threadp->column = ((a_threadp->column + 1) * !newline),
	     newline = 0)
    {
	c = a_str[i];

	/* If a special character causes the end of the previous token, the
	 * state machine builds the object, then restarts the state machine
	 * without incrementing the input character index.  This is done in
	 * order to avoid having to duplicate the THREADTS_START code. */
	RESTART:

	switch (a_thread->state)
	{
	    case THREADTS_START:
	    {
		cw_assert(a_thread->index == 0);

		if (a_token)
		{
		    /* token is true if a token has been accepted.  We look for
		     * the situation where token is true and
		     * a_thread->defer_count is only 1 (artificially raised).
		     * If these conditions are met, then we've managed to scan
		     * an entire token, as defined by the token operator. */
		    if (token && a_thread->defer_count == 1)
		    {
			/* Return the offset of the next character. */
			retval = i;
			goto RETURN;
		    }
		}

		switch (c)
		{
		    case '`':
		    {
			a_thread->state = THREADTS_STRING;
			a_thread->m.s.q_depth = 1;
			break;
		    }
		    case '\'':
		    {
			nxoe_p_thread_syntax_error(a_thread, a_threadp,
						   defer_base, "", "", c);
			if (a_token)
			{
			    goto RETURN;
			}
			break;
		    }
		    case '<': case '>': case '(': case ')': case '[': case ']':
		    {
			CW_NXO_THREAD_PUTC(c);
			token = true;
			a_thread->m.m.action = ACTION_EXECUTE;
			nxoe_p_thread_name_accept(a_thread);
			break;
		    }
		    case '{':
		    {
			uint32_t line, column;

			a_thread->defer_count++;
			nxo = nxo_stack_push(&a_thread->ostack);
			nxo_pmark_new(nxo);
			nxo_threadp_position_get(a_threadp, &line, &column);
			nxo_pmark_line_set(nxo, line);
			break;
		    }
		    case '}':
		    {
			if (a_thread->defer_count > defer_base)
			{
			    token = true;
			    a_thread->defer_count--;
			    nxoe_p_thread_procedure_accept(a_thread, a_threadp);
			}
			else
			{
			    /* Missing '{'. */
			    nxoe_p_thread_syntax_error(a_thread, a_threadp,
						       defer_base, "", "", c);
			    if (a_token)
			    {
				goto RETURN;
			    }
			}
			break;
		    }
		    case '!':
		    {
			a_thread->state = THREADTS_NAME_START;
			a_thread->m.m.action = ACTION_EVALUATE;
			break;
		    }
#ifdef CW_OOP
		    case ':':
		    {
			a_thread->state = THREADTS_NAME_START;
			a_thread->m.m.action = ACTION_CALL;
			break;
		    }
		    case ';':
		    {
			a_thread->state = THREADTS_NAME_START;
			a_thread->m.m.action = ACTION_INVOKE;
			break;
		    }
		    case ',':
		    {
			a_thread->state = THREADTS_NAME_START;
			a_thread->m.m.action = ACTION_FETCH;
			break;
		    }
#endif
		    case '$':
		    {
			a_thread->state = THREADTS_NAME_START;
			a_thread->m.m.action = ACTION_LITERAL;
			break;
		    }
		    case '~':
		    {
			a_thread->state = THREADTS_NAME_START;
			a_thread->m.m.action = ACTION_IMMEDIATE;
			break;
		    }
		    case '#':
		    {
			a_thread->state = THREADTS_COMMENT;
			break;
		    }
		    case '\n':
		    {
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '\0': case '\t': case '\f': case '\r': case ' ':
		    {
			/* Swallow. */
			break;
		    }
		    case '+':
		    {
			a_thread->state = THREADTS_INTEGER;
			a_thread->m.n.mant_neg = false;
			a_thread->m.n.radix_base = 10;
			a_thread->m.n.whole = false;
#ifdef CW_REAL
			a_thread->m.n.frac = false;
			a_thread->m.n.exp = false;
#endif
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '-':
		    {
			a_thread->state = THREADTS_INTEGER;
			a_thread->m.n.mant_neg = true;
			a_thread->m.n.radix_base = 10;
			a_thread->m.n.whole = false;
#ifdef CW_REAL
			a_thread->m.n.frac = false;
			a_thread->m.n.exp = false;
#endif
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
#ifdef CW_REAL
		    case '.':
		    {
			a_thread->state = THREADTS_REAL_FRAC;
			a_thread->m.n.mant_neg = false;
			a_thread->m.n.radix_base = 10;
			a_thread->m.n.whole = false;
			a_thread->m.n.frac = false;
			a_thread->m.n.exp = false;
			break;
		    }
#endif
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			a_thread->state = THREADTS_INTEGER;
			a_thread->m.n.mant_neg = false;
			a_thread->m.n.radix_base = 10;
			a_thread->m.n.whole = true;
			a_thread->m.n.whole_off = 0;
#ifdef CW_REAL
			a_thread->m.n.frac = false;
			a_thread->m.n.exp = false;
#endif
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    default:
		    {
			a_thread->state = THREADTS_NAME;
			a_thread->m.m.action = ACTION_EXECUTE;
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
	    case THREADTS_COMMENT:
	    {
		cw_assert(a_thread->index == 0);

		switch (c)
		{
		    case '\n':
		    {
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '\f':
		    {
			/* Fall through. */
		    }
		    case '\r':
		    {
			a_thread->state = THREADTS_START;
			break;
		    }
		    default:
		    {
			break;
		    }
		}
		break;
	    }
	    case THREADTS_INTEGER:
	    {
		bool restart = false;

		switch (c)
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			if (a_thread->m.n.whole == false)
			{
			    a_thread->m.n.whole = true;
			    a_thread->m.n.whole_off = a_thread->index;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '@':
		    {
			uint32_t digit, ndigits;

			/* Convert the string to a base (interpreted as base
			 * 10). */
			if (a_thread->m.n.whole)
			{
			    a_thread->m.n.whole_len = a_thread->index
				- a_thread->m.n.whole_off;
			    ndigits = a_thread->m.n.whole_len;
			}
			else
			{
			    ndigits = 0;
			}
			switch (ndigits)
			{
			    case 2:
			    {
				digit =
				    CW_NXO_THREAD_GETC(a_thread->m.n.whole_off)
				    - '0';
				if (digit == 0)
				{
				    /* Leading '0' in radix not allowed. */
				    break;
				}
				a_thread->m.n.radix_base = digit * 10;

				digit
				    = CW_NXO_THREAD_GETC(a_thread->m.n.whole_off
							 + 1) - '0';
				a_thread->m.n.radix_base += digit;
				break;
			    }
			    case 1:
			    {
				digit =
				    CW_NXO_THREAD_GETC(a_thread->m.n.whole_off)
				    - '0';
				a_thread->m.n.radix_base = digit;
				break;
			    }
			    default:
			    {
				/* Too many, or not enough digits. */
				a_thread->m.n.radix_base = 0;
				break;
			    }
			}

			if (a_thread->m.n.radix_base < 2
			    || a_thread->m.n.radix_base > 36)
			{
			    /* Base too small or too large. */
			    a_thread->state = THREADTS_NAME;
			    a_thread->m.m.action = ACTION_EXECUTE;
			}
			else
			{
			    a_thread->m.n.whole = false;
			    a_thread->state = THREADTS_INTEGER_RADIX;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
#ifdef CW_REAL
		    case '.':
		    {
			a_thread->state = THREADTS_REAL_FRAC;
			if (a_thread->m.n.whole)
			{
			    a_thread->m.n.whole_len = a_thread->index
				- a_thread->m.n.whole_off;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case 'e': case 'E':
		    {
			a_thread->state = THREADTS_REAL_EXP;
			a_thread->m.n.exp_sign = false;
			a_thread->m.n.exp_neg = false;
			if (a_thread->m.n.whole)
			{
			    a_thread->m.n.whole_len = a_thread->index
				- a_thread->m.n.whole_off;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
#endif
		    case '\n':
		    {
			restart = true; /* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!':
#ifdef CW_OOP
		    case ':': case ';': case ',':
#endif
		    case '$': case '~': case '#':
		    {
			/* New token. */
			/* Invert, in case we fell through from above. */
			restart = !restart;
			/* Fall through. */
		    }
		    case '\0': case '\t': case '\f': case '\r': case ' ':
		    {
			if (a_thread->m.n.whole)
			{
			    a_thread->m.n.whole_len = a_thread->index
				- a_thread->m.n.whole_off;
			}
			if (nxoe_p_thread_integer_accept(a_thread))
			{
			    /* Conversion error.  Accept as a name. */
			    a_thread->m.m.action = ACTION_EXECUTE;
			    nxoe_p_thread_name_accept(a_thread);
			}
			token = true;
			if (restart)
			{
			    goto RESTART;
			}
			break;
		    }
		    default:
		    {
			/* Not a number character. */
			a_thread->state = THREADTS_NAME;
			a_thread->m.m.action = ACTION_EXECUTE;
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
	    case THREADTS_INTEGER_RADIX:
	    {
		bool restart = false;

		switch (c)
		{
		    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		    case 'y': case 'z':
		    {
			if (a_thread->m.n.whole == false)
			{
			    a_thread->m.n.whole = true;
			    a_thread->m.n.whole_off = a_thread->index;
			}
			if (a_thread->m.n.radix_base
			    <= (10 + ((uint32_t) (c - 'a'))))
			{
			    /* Too big for this base. */
			    a_thread->state = THREADTS_NAME;
			    a_thread->m.m.action = ACTION_EXECUTE;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		    case 'Y': case 'Z':
		    {
			if (a_thread->m.n.whole == false)
			{
			    a_thread->m.n.whole = true;
			    a_thread->m.n.whole_off = a_thread->index;
			}
			if (a_thread->m.n.radix_base
			    <= (10 + ((uint32_t) (c - 'A'))))
			{
			    /* Too big for this base. */
			    a_thread->state = THREADTS_NAME;
			    a_thread->m.m.action = ACTION_EXECUTE;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			if (a_thread->m.n.whole == false)
			{
			    a_thread->m.n.whole = true;
			    a_thread->m.n.whole_off = a_thread->index;
			}
			if (a_thread->m.n.radix_base
			    <= ((uint32_t) (c - '0')))
			{
			    /* Too big for this base. */
			    a_thread->state = THREADTS_NAME;
			    a_thread->m.m.action = ACTION_EXECUTE;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '\n':
		    {
			restart = true; /* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!':
#ifdef CW_OOP
		    case ':': case ';': case ',':
#endif
		    case '$': case '~': case '#':
		    {
			/* New token. */
			/* Invert, in case we fell through from above. */
			restart = !restart;
			/* Fall through. */
		    }
		    case '\0': case '\t': case '\f': case '\r': case ' ':
		    {
			if (a_thread->m.n.whole)
			{
			    a_thread->m.n.whole_len = a_thread->index
				- a_thread->m.n.whole_off;
			}
			if (a_thread->m.n.whole == false
			    || nxoe_p_thread_integer_accept(a_thread))
			{
			    /* Conversion error.  Accept as a name. */
			    a_thread->m.m.action = ACTION_EXECUTE;
			    nxoe_p_thread_name_accept(a_thread);
			}
			token = true;
			if (restart)
			{
			    goto RESTART;
			}
			break;
		    }
		    default:
		    {
			/* Not a number character. */
			a_thread->state = THREADTS_NAME;
			a_thread->m.m.action = ACTION_EXECUTE;
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
#ifdef CW_REAL
	    case THREADTS_REAL_FRAC:
	    {
		bool restart = false;

		switch (c)
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			if (a_thread->m.n.frac == false)
			{
			    a_thread->m.n.frac = true;
			    a_thread->m.n.frac_off = a_thread->index;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case 'e': case 'E':
		    {
			a_thread->state = THREADTS_REAL_EXP;
			a_thread->m.n.exp_sign = false;
			a_thread->m.n.exp_neg = false;
			if (a_thread->m.n.frac)
			{
			    a_thread->m.n.frac_len = a_thread->index
				- a_thread->m.n.frac_off;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '\n':
		    {
			restart = true; /* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!':
#ifdef CW_OOP
		    case ':': case ';': case ',':
#endif
		    case '$': case '~': case '#':
		    {
			/* New token. */
			/* Invert, in case we fell through from above. */
			restart = !restart;
			/* Fall through. */
		    }
		    case '\0': case '\t': case '\f': case '\r': case ' ':
		    {
			if (a_thread->m.n.frac)
			{
			    a_thread->m.n.frac_len = a_thread->index
				- a_thread->m.n.frac_off;
			}
			if (nxoe_p_thread_real_accept(a_thread))
			{
			    /* Conversion error.  Accept as a name. */
			    a_thread->m.m.action = ACTION_EXECUTE;
			    nxoe_p_thread_name_accept(a_thread);
			}
			token = true;
			if (restart)
			{
			    goto RESTART;
			}
			break;
		    }
		    default:
		    {
			/* Not a number character. */
			a_thread->state = THREADTS_NAME;
			a_thread->m.m.action = ACTION_EXECUTE;
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
	    case THREADTS_REAL_EXP:
	    {
		bool restart = false;

		switch (c)
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			if (a_thread->m.n.exp == false)
			{
			    a_thread->m.n.exp = true;
			    a_thread->m.n.exp_off = a_thread->index;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '+':
		    {
			if (a_thread->m.n.exp_sign == false
			    && a_thread->m.n.exp == false)
			{
			    a_thread->m.n.exp_sign = true;
			}
			else
			{
			    /* Sign specified more than once. */
			    a_thread->state = THREADTS_NAME;
			    a_thread->m.m.action = ACTION_EXECUTE;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '-':
		    {
			if (a_thread->m.n.exp_sign == false
			    && a_thread->m.n.exp == false)
			{
			    a_thread->m.n.exp_sign = true;
			    a_thread->m.n.exp_neg = true;
			}
			else
			{
			    /* Sign specified more than once. */
			    a_thread->state = THREADTS_NAME;
			    a_thread->m.m.action = ACTION_EXECUTE;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '\n':
		    {
			restart = true; /* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!':
#ifdef CW_OOP
		    case ':': case ';': case ',':
#endif
		    case '$': case '~': case '#':
		    {
			/* New token. */
			/* Invert, in case we fell through from above. */
			restart = !restart;
			/* Fall through. */
		    }
		    case '\0': case '\t': case '\f': case '\r': case ' ':
		    {
			if (a_thread->m.n.exp)
			{
			    a_thread->m.n.exp_len = a_thread->index
				- a_thread->m.n.exp_off;
			}
			if (a_thread->m.n.exp == false
			    || nxoe_p_thread_real_accept(a_thread))
			{
			    /* Conversion error.  Accept as a name. */
			    a_thread->m.m.action = ACTION_EXECUTE;
			    nxoe_p_thread_name_accept(a_thread);
			}
			token = true;
			if (restart)
			{
			    goto RESTART;
			}
			break;
		    }
		    default:
		    {
			/* Not a number character. */
			a_thread->state = THREADTS_NAME;
			a_thread->m.m.action = ACTION_EXECUTE;
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
#endif
	    case THREADTS_STRING:
	    {
		/* The CRLF code jumps here if there was no LF. */
		STRING_CONTINUE:

		switch (c)
		{
		    case '\\':
		    {
			a_thread->state = THREADTS_STRING_PROT_CONT;
			break;
		    }
		    case '`':
		    {
			a_thread->m.s.q_depth++;
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '\'':
		    {
			a_thread->m.s.q_depth--;
			if (a_thread->m.s.q_depth == 0)
			{
			    token = true;
			    nxo = nxo_stack_push(&a_thread->ostack);
#ifdef CW_THREADS
			    nxo_string_new(nxo, a_thread->locking,
					   a_thread->index);
#else
			    nxo_string_new(nxo, false, a_thread->index);
#endif
			    nxo_string_set(nxo, 0, a_thread->tok_str,
					   a_thread->index);

			    nxoe_p_thread_reset(a_thread);
			}
			else
			{
			    CW_NXO_THREAD_PUTC(c);
			}
			break;
		    }
		    case '\r':
		    {
			a_thread->state = THREADTS_STRING_NEWLINE_CONT;
			break;
		    }
		    case '\n':
		    {
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    default:
		    {
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
	    case THREADTS_STRING_NEWLINE_CONT:
	    {
		/* All cases in the switch statement do this. */
		CW_NXO_THREAD_PUTC('\n');
		a_thread->state = THREADTS_STRING;
		switch (c)
		{
		    case '\n':
		    {
			CW_NXO_THREAD_NEWLINE();
			break;
		    }
		    default:
		    {
			/* '\r' was not followed by a '\n'.  Translate the '\r'
			 * to a '\n' and jump back up to the string scanning
			 * state to scan c again. */
			goto STRING_CONTINUE;
		    }
		}
		break;
	    }
	    case THREADTS_STRING_PROT_CONT:
	    {
		switch (c)
		{
		    case '`': case '\'': case '\\':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '0':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\0');
			break;
		    }
		    case 'n':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\n');
			break;
		    }
		    case 'r':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\r');
			break;
		    }
		    case 't':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\t');
			break;
		    }
		    case 'a':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\a');
			break;
		    }
		    case 'b':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\b');
			break;
		    }
		    case 'e':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\x1b');
			break;
		    }
		    case 'f':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\f');
			break;
		    }
		    case 'c':
		    {
			a_thread->state = THREADTS_STRING_CTRL_CONT;
			break;
		    }
		    case 'x':
		    {
			a_thread->state = THREADTS_STRING_HEX_CONT;
			break;
		    }
		    case '\r':
		    {
			a_thread->state = THREADTS_STRING_CRLF_CONT;
			break;
		    }
		    case '\n':
		    {
			CW_NXO_THREAD_NEWLINE();

			/* Ignore. */
			a_thread->state = THREADTS_STRING;
			break;
		    }
		    default:
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC('\\');
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
	    case THREADTS_STRING_CRLF_CONT:
	    {
		switch (c)
		{
		    case '\n':
		    {
			CW_NXO_THREAD_NEWLINE();

			/* Ignore. */
			a_thread->state = THREADTS_STRING;
			break;
		    }
		    default:
		    {
			goto STRING_CONTINUE;
		    }
		}
		break;
	    }
	    case THREADTS_STRING_CTRL_CONT:
	    {
		switch (c)
		{
		    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		    case 'y': case 'z':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC(c - 'a' + 1);
			break;
		    }
		    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		    case 'Y': case 'Z':
		    {
			a_thread->state = THREADTS_STRING;
			CW_NXO_THREAD_PUTC(c - 'A' + 1);
			break;
		    }
		    default:
		    {
			nxoe_p_thread_syntax_error(a_thread, a_threadp,
						   defer_base, "`", "\\c", c);
			if (a_token)
			{
			    goto RETURN;
			}
		    }
		}
		break;
	    }
	    case THREADTS_STRING_HEX_CONT:
	    {
		switch (c)
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9': case 'a': case 'b':
		    case 'c': case 'd': case 'e': case 'f': case 'A': case 'B':
		    case 'C': case 'D': case 'E': case 'F':
		    {
			a_thread->state = THREADTS_STRING_HEX_FINISH;
			a_thread->m.s.hex_val = c;
			break;
		    }
		    default:
		    {
			nxoe_p_thread_syntax_error(a_thread, a_threadp,
						   defer_base, "`", "\\x", c);
			if (a_token)
			{
			    goto RETURN;
			}
		    }
		}
		break;
	    }
	    case THREADTS_STRING_HEX_FINISH:
	    {
		switch (c)
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9': case 'a': case 'b':
		    case 'c': case 'd': case 'e': case 'f': case 'A': case 'B':
		    case 'C': case 'D': case 'E': case 'F':
		    {
			unsigned char val;

			a_thread->state = THREADTS_STRING;
			switch (a_thread->m.s.hex_val)
			{
			    case '0': case '1': case '2': case '3': case '4':
			    case '5': case '6': case '7': case '8': case '9':
			    {
				val = (a_thread->m.s.hex_val - '0') << 4;
				break;
			    }
			    case 'a': case 'b': case 'c': case 'd': case 'e':
			    case 'f':
			    {
				val = ((a_thread->m.s.hex_val - 'a') + 10) << 4;
				break;
			    }
			    case 'A': case 'B': case 'C': case 'D': case 'E':
			    case 'F':
			    {
				val = ((a_thread->m.s.hex_val - 'A') + 10) << 4;
				break;
			    }
			    default:
			    {
				cw_not_reached();
			    }
			}
			switch (c)
			{
			    case '0': case '1': case '2': case '3': case '4':
			    case '5': case '6': case '7': case '8': case '9':
			    {
				val |= (c - '0');
				break;
			    }
			    case 'a': case 'b': case 'c': case 'd': case 'e':
			    case 'f':
			    {
				val |= ((c - 'a') + 10);
				break;
			    }
			    case 'A': case 'B': case 'C': case 'D': case 'E':
			    case 'F':
			    {
				val |= ((c - 'A') + 10);
				break;
			    }
			    default:
			    {
				cw_not_reached();
			    }
			}
			CW_NXO_THREAD_PUTC(val);
			break;
		    }
		    default:
		    {
			char suffix[] = "\\x?";

			suffix[2] = a_thread->m.s.hex_val;
			nxoe_p_thread_syntax_error(a_thread, a_threadp,
						   defer_base, "`", suffix, c);
			if (a_token)
			{
			    goto RETURN;
			}
		    }
		}
		break;
	    }
	    case THREADTS_NAME_START:
	    {
		cw_assert(a_thread->index == 0);

		switch (c)
		{
		    case '\n':
		    {
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '\0': case '\t': case '\f': case '\r': case ' ':
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!':
#ifdef CW_OOP
		    case ':': case ';': case ',':
#endif
		    case '$': case '~': case '#':
		    {
			char suffix[2] = "?";

			switch (a_thread->m.m.action)
			{
			    case ACTION_EXECUTE:
			    {
				suffix[0] = '\0';
				break;
			    }
			    case ACTION_EVALUATE:
			    {
				suffix[0] = '!';
				break;
			    }
#ifdef CW_OOP
			    case ACTION_CALL:
			    {
				suffix[0] = ':';
				break;
			    }
			    case ACTION_INVOKE:
			    {
				suffix[0] = ';';
				break;
			    }
			    case ACTION_FETCH:
			    {
				suffix[0] = ',';
				break;
			    }
#endif
			    case ACTION_LITERAL:
			    {
				suffix[0] = '$';
				break;
			    }
			    case ACTION_IMMEDIATE:
			    {
				suffix[0] = '~';
				break;
			    }
			    default:
			    {
				cw_not_reached();
			    }
			}

			nxoe_p_thread_syntax_error(a_thread, a_threadp,
						   defer_base, "", suffix, c);
			if (a_token)
			{
			    goto RETURN;
			}
			break;
		    }
		    default:
		    {
			a_thread->state = THREADTS_NAME;
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
	    case THREADTS_NAME:
	    {
		bool restart = false;

		switch (c)
		{
		    case '\n':
		    {
			restart = true;	/* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!':
#ifdef CW_OOP
		    case ':': case ';': case ',':
#endif
		    case '$': case '~': case '#':
		    {
			/* New token. */
			/* Invert, in case we fell through from above. */
			restart = !restart;
			/* Fall through. */
		    }
		    case '\0': case '\t': case '\f': case '\r': case ' ':
		    {
			/* End of name. */
			if (a_thread->index > 0)
			{
			    token = true;
			    nxoe_p_thread_name_accept(a_thread);
			}
			else
			{
			    char suffix[2] = "?";

			    switch (a_thread->m.m.action)
			    {
				case ACTION_EXECUTE:
				{
				    suffix[0] = '\0';
				    break;
				}
				case ACTION_EVALUATE:
				{
				    suffix[0] = '!';
				    break;
				}
#ifdef CW_OOP
				case ACTION_CALL:
				{
				    suffix[0] = ':';
				    break;
				}
				case ACTION_INVOKE:
				{
				    suffix[0] = ';';
				    break;
				}
				case ACTION_FETCH:
				{
				    suffix[0] = ',';
				    break;
				}
#endif
				case ACTION_LITERAL:
				{
				    suffix[0] = '$';
				    break;
				}
				case ACTION_IMMEDIATE:
				{
				    suffix[0] = '~';
				    break;
				}
				default:
				{
				    cw_not_reached();
				}
			    }

			    nxoe_p_thread_syntax_error(a_thread, a_threadp,
						       defer_base, "", suffix,
						       c);
			    if (a_token)
			    {
				goto RETURN;
			    }
			}
			if (restart)
			{
			    goto RESTART;
			}
			break;
		    }
		    default:
		    {
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }

    retval = i;
    RETURN:
    if (a_token)
    {
	a_thread->defer_count--;
    }
    return retval;
}

static void
nxoe_p_thread_tok_str_expand(cw_nxoe_thread_t *a_thread)
{
    if (a_thread->index == CW_NXO_THREAD_BUFFER_SIZE)
    {
	/* First overflow, initial expansion needed. */
	a_thread->tok_str = (char *) nxa_malloc(a_thread->index * 2);
	a_thread->buffer_len = a_thread->index * 2;
	memcpy(a_thread->tok_str, a_thread->buffer, a_thread->index);
    }
    else if (a_thread->index == a_thread->buffer_len)
    {
	char *t_str;

	/* Overflowed, and additional expansion needed. */
	t_str = (char *) nxa_malloc(a_thread->index * 2);
	a_thread->buffer_len = a_thread->index * 2;
	memcpy(t_str, a_thread->tok_str, a_thread->index);
	nxa_free(a_thread->tok_str, a_thread->index);
	a_thread->tok_str = t_str;
    }
}

/* Create a string that represents the code that caused the syntax error and
 * push it onto ostack.  This means that syntax errors cause two objects to be
 * pushed onto ostack rather than just the standard one, but if we don't do
 * this, the invalid code gets lost forever.
 *
 * If a_c is -1, no current character is inserted into the error string.  This
 * is necessary for syntax errors at EOF. */
static void
nxoe_p_thread_syntax_error(cw_nxoe_thread_t *a_thread,
			   cw_nxo_threadp_t *a_threadp,
			   uint32_t a_defer_base, char *a_prefix,
			   char *a_suffix, int32_t a_c)
{
    cw_nxo_t *nxo;
    const char *origin;
    uint32_t defer_count, olen, line, column;

    nxo = nxo_stack_push(&a_thread->ostack);

#ifdef CW_THREADS
    nxo_string_new(nxo, a_thread->locking,
		   strlen((char *) a_prefix) + a_thread->index
		   + strlen((char *) a_suffix) + ((a_c >= 0) ? 1 : 0));
#else
    nxo_string_new(nxo, false,
		   strlen((char *) a_prefix) + a_thread->index
		   + strlen((char *) a_suffix) + ((a_c >= 0) ? 1 : 0));
#endif
    nxo_attr_set(nxo, NXOA_EXECUTABLE);

    /* Prefix. */
    nxo_string_set(nxo, 0, a_prefix, strlen((char *) a_prefix));

    /* Main text. */
    nxo_string_set(nxo, strlen((char *) a_prefix), a_thread->tok_str,
		   a_thread->index);

    /* Suffix. */
    nxo_string_set(nxo, strlen((char *) a_prefix) + a_thread->index, a_suffix,
		   strlen((char *) a_suffix));

    /* Current character, if any. */
    if (a_c >= 0)
    {
	char c = (char) a_c;

	nxo_string_set(nxo, strlen((char *) a_prefix) + a_thread->index +
		       strlen((char *) a_suffix), &c, 1);
    }

    nxoe_p_thread_reset(a_thread);

    /* Push origin, line and column onto ostack.  They are used in the embedded
     * onyx code below to set origin, line, and column in currenterror. */
    nxo_threadp_origin_get(a_threadp, &origin, &olen);
    nxo_threadp_position_get(a_threadp, &line, &column);

    /* origin. */
    nxo = nxo_stack_push(&a_thread->ostack);
    if (origin != NULL)
    {
#ifdef CW_THREADS
	nxo_string_new(nxo, a_thread->locking, olen);
#else
	nxo_string_new(nxo, false, olen);
#endif
	nxo_string_set(nxo, 0, origin, olen);
    }
    else
    {
	nxo_null_new(nxo);
    }

    /* line. */
    nxo = nxo_stack_push(&a_thread->ostack);
    nxo_integer_new(nxo, (cw_nxoi_t) line);

    /* column. */
    nxo = nxo_stack_push(&a_thread->ostack);
    /* If the syntax error happened at a newline, the column number won't be
     * correct, so use 0. */
    if (column == -1)
    {
	nxo_integer_new(nxo, 0);
    }
    else
    {
	nxo_integer_new(nxo, (cw_nxoi_t) column);
    }

    /* Shut off deferral temporarily.  It is possible for this C stack frame to
     * never be returned to, due to an exception (stop, quit, exit), in which
     * case, deferral will never be turned back on.  That's fine, since the user
     * is going to have to patch things up by hand in that case anyway. */
    defer_count = a_thread->defer_count;
    a_thread->defer_count = 0;

    cw_onyx_code(&a_thread->self,
		 "currenterror begin"
		 " $column exch def $line exch def $origin exch def end"
		 " $syntaxerror throw");

    /* Turn deferral back on. */
    a_thread->defer_count = defer_count;
}

static void
nxoe_p_thread_reset(cw_nxoe_thread_t *a_thread)
{
    a_thread->state = THREADTS_START;
    if (a_thread->index > CW_NXO_THREAD_BUFFER_SIZE)
    {
	nxa_free(a_thread->tok_str, a_thread->buffer_len);
	a_thread->tok_str = a_thread->buffer;
    }
    a_thread->index = 0;
}

static bool
nxoe_p_thread_integer_accept(cw_nxoe_thread_t *a_thread)
{
    bool retval;

    if (a_thread->m.n.whole)
    {
	cw_nxo_t *nxo;
	cw_nxoi_t val;
	uint32_t i;
	uint64_t threshold, maxlast, sum, digit;
	char c;

	/* Determine threshold value at which overflow is a risk.  If the
	 * threshold is exceeded, then overflow occurred.  If the threshold
	 * value is reached and the next digit exceeds a certain value
	 * (maxlast), overflow occurred. */
	if (a_thread->m.n.mant_neg)
	{
	    threshold = 0x8000000000000000ULL;
	}
	else
	{
	    threshold = 0x7fffffffffffffffULL;
	}
	maxlast = threshold % a_thread->m.n.radix_base;
	threshold /= a_thread->m.n.radix_base;

	/* Iterate from right to left through the digits. */
	sum = 0;
	for (i = 0; i < a_thread->m.n.whole_len; i++)
	{
	    c = CW_NXO_THREAD_GETC(a_thread->m.n.whole_off + i);
	    switch (c)
	    {
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		{
		    digit = 10 + ((uint64_t) (c - 'a'));
		    break;
		}
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		{
		    digit = 10 + ((uint64_t) (c - 'A'));
		    break;
		}
		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
		{
		    digit = (uint64_t) (c - '0');
		    /* Fall through. */
		}
		default:
		{
		    break;
		}
	    }

	    if ((sum > threshold) || (sum == threshold && digit > maxlast))
	    {
		/* Overflow. */
		retval = true;
		goto RETURN;
	    }

	    sum *= a_thread->m.n.radix_base;
	    sum += digit;
	}

	if (a_thread->m.n.mant_neg)
	{
	    val = -sum;
	}
	else
	{
	    val = sum;
	}

	/* Create an integer on ostack. */
	nxo = nxo_stack_push(&a_thread->ostack);
	nxo_integer_new(nxo, val);
	nxoe_p_thread_reset(a_thread);
    }
    else
    {
	/* No number specified. */
	retval = true;
	goto RETURN;
    }

    retval = false;
    RETURN:
    return retval;
}

#ifdef CW_REAL
static bool
nxoe_p_thread_real_accept(cw_nxoe_thread_t *a_thread)
{
    bool retval;
    cw_nxo_t *nxo;
    cw_nxor_t val;

    /* Convert string to real.  Do the conversion before mucking with the stack
     * in case there is a conversion error.
     *
     * The state created by the scanner is enough to allow us to do the
     * conversion more efficiently than possible via strtod(), but the
     * conversion process is quite complex, so just use strtod().
     */
    a_thread->tok_str[a_thread->index] = '\0';
    errno = 0;
    val = strtod(a_thread->tok_str, NULL);
    if (errno == ERANGE && (val == HUGE_VAL || val == -HUGE_VAL))
    {
	retval = true;
	goto RETURN;
    }

    /* Create a real on ostack. */
    nxo = nxo_stack_push(&a_thread->ostack);
    nxo_real_new(nxo, val);
    nxoe_p_thread_reset(a_thread);

    retval = false;
    RETURN:
    return retval;
}
#endif

static void
nxoe_p_thread_procedure_accept(cw_nxoe_thread_t *a_thread,
			       cw_nxo_threadp_t *a_threadp)
{
    cw_nxo_t *tnxo, *nxo;
    uint32_t nelements, i, depth;

    /* Find the pmark. */
    for (i = 0, depth = nxo_stack_count(&a_thread->ostack), nxo = NULL;
	 i < depth;
	 i++)
    {
	nxo = nxo_stack_nget(&a_thread->ostack, i);
	if (nxo_type_get(nxo) == NXOT_PMARK)
	{
	    break;
	}
    }
    cw_assert(i < depth);

    /* i is the index of the mark, and nxo points to the mark.  Set nelements
     * accordingly.  When we pop the nxo's off the stack, we'll have to pop
     * (nelements + 1) nxo's. */
    nelements = i;

    tnxo = nxo_stack_push(&a_thread->tstack);
#ifdef CW_THREADS
    nxo_array_new(tnxo, a_thread->locking, nelements);
#else
    nxo_array_new(tnxo, false, nelements);
#endif
    {
	const char *origin;
	uint32_t olen;

	/* Set source origin information. */
	nxo_threadp_origin_get(a_threadp, &origin, &olen);
	if (origin != NULL)
	{
	    nxo_array_origin_set(tnxo, origin, olen, nxo_pmark_line_get(nxo));
	}
    }
    nxo_attr_set(tnxo, NXOA_EXECUTABLE);

    /* Iterate up the stack, dup'ing nxo's to the array. */
    for (i = 0; i < nelements; i++)
    {
	nxo = nxo_stack_nget(&a_thread->ostack, nelements - 1 - i);
	nxo_array_el_set(tnxo, nxo, i);
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
    cw_nxo_t *nxo;

    switch (a_thread->m.m.action)
    {
	case ACTION_EXECUTE:
	case ACTION_EVALUATE:
#ifdef CW_OOP
	case ACTION_CALL:
	case ACTION_INVOKE:
	case ACTION_FETCH:
#endif
	{
	    if (a_thread->defer_count == 0)
	    {
		/* Prepare for interpreter recursion by pushing a name object
		 * onto estack, then run the execution loop. */
		nxo = nxo_stack_push(&a_thread->estack);
		nxo_name_new(nxo, a_thread->tok_str, a_thread->index, false);
		nxo_attr_set(nxo, a_thread->m.m.action);

		nxoe_p_thread_reset(a_thread);
		nxo_thread_loop(&a_thread->self);
	    }
	    else
	    {
		/* Push the name object onto the operand stack. */
		nxo = nxo_stack_push(&a_thread->ostack);
		nxo_name_new(nxo, a_thread->tok_str, a_thread->index, false);
		nxo_attr_set(nxo, a_thread->m.m.action);
		nxoe_p_thread_reset(a_thread);
	    }
	    break;
	}
	case ACTION_LITERAL:
	{
	    /* Push the name object onto the operand stack. */
	    nxo = nxo_stack_push(&a_thread->ostack);
	    nxo_name_new(nxo, a_thread->tok_str, a_thread->index, false);
	    nxoe_p_thread_reset(a_thread);
	    break;
	}
	case ACTION_IMMEDIATE:
	{
	    cw_nxo_t *key;

	    /* Find the value associated with the name in the dictionary stack
	     * and push the value onto the operand stack. */
	    key = nxo_stack_push(&a_thread->tstack);
	    nxo_name_new(key, a_thread->tok_str, a_thread->index, false);
	    nxoe_p_thread_reset(a_thread);

	    nxo = nxo_stack_push(&a_thread->ostack);
	    if (nxo_thread_dstack_search(&a_thread->self, key, nxo))
	    {
		/* Push the name onto ostack before throwing the error.  This
		 * results in both the name and the source being pushed onto
		 * ostack, which is useful, depending on the source. */
		nxo_dup(nxo, key);

		/* Reset the deferral count before throwing the error. */
		a_thread->defer_count = 0;

		nxo_thread_nerror(&a_thread->self, NXN_undefined);
	    }
	    else if (nxo_type_get(nxo) == NXOT_ARRAY
		     && nxo_attr_get(nxo) == NXOA_EXECUTABLE)
	    {
		/* Set the evaluable attribute so that the array will still be
		 * executed when interpreted. */
		nxo_attr_set(nxo, NXOA_EVALUABLE);
	    }
	    nxo_stack_pop(&a_thread->tstack);

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
}
