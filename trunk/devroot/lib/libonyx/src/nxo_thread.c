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
#include "../include/libonyx/nxo_operator_l.h" /* XXX Temp debugging. */

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

#ifdef CW_THREADS
static void *
nxo_p_thread_entry(void *a_arg);
#endif
static cw_uint32_t
nxoe_p_thread_feed(cw_nxoe_thread_t *a_thread, cw_nxo_threadp_t *a_threadp,
		   cw_bool_t a_token, const cw_uint8_t *a_str,
		   cw_uint32_t a_len);
static void
nxoe_p_thread_tok_str_expand(cw_nxoe_thread_t *a_thread);
static void
nxoe_p_thread_syntax_error(cw_nxoe_thread_t *a_thread,
			   cw_nxo_threadp_t *a_threadp,
			   cw_uint32_t a_defer_base, cw_uint8_t *a_prefix,
			   cw_uint8_t *a_suffix, cw_sint32_t a_c);
static void
nxoe_p_thread_reset(cw_nxoe_thread_t *a_thread);
static cw_bool_t
nxoe_p_thread_integer_accept(cw_nxoe_thread_t *a_thread);
#ifdef CW_REAL
static cw_bool_t
nxoe_p_thread_real_accept(cw_nxoe_thread_t *a_thread);
#endif
static void
nxoe_p_thread_procedure_accept(cw_nxoe_thread_t *a_thread);
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
	    cw_uint8_t suffix[2] = "?";

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
nxo_threadp_position_get(const cw_nxo_threadp_t *a_threadp, cw_uint32_t *r_line,
			 cw_uint32_t *r_column)
{
    cw_check_ptr(a_threadp);
    cw_dassert(a_threadp->magic == CW_NXO_THREADP_MAGIC);

    *r_line = a_threadp->line;
    *r_column = a_threadp->column;
}

void
nxo_threadp_position_set(cw_nxo_threadp_t *a_threadp, cw_uint32_t a_line,
			 cw_uint32_t a_column)
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

    thread = (cw_nxoe_thread_t *) nxa_calloc(nx_nxa_get(a_nx), 1,
					     sizeof(cw_nxoe_thread_t));

    nxoe_l_new(&thread->nxoe, NXOT_THREAD, FALSE);

    /* Set things to a state that won't cause the GC (or any thread-related
     * operators) to puke. */

    /* Fake up a nxo for self. */
    nxo_p_new(&thread->self, NXOT_THREAD);
    thread->self.o.nxoe = (cw_nxoe_t *) thread;

    thread->nx = a_nx;
    thread->tok_str = thread->buffer;

    nxo_no_new(&thread->estack);
    nxo_no_new(&thread->istack);
    nxo_no_new(&thread->ostack);
    nxo_no_new(&thread->dstack);
    nxo_no_new(&thread->tstack);
    nxo_no_new(&thread->stdin_nxo);
    nxo_no_new(&thread->stdout_nxo);
    nxo_no_new(&thread->stderr_nxo);
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

    nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *) thread);

    /* Finish setting up the internals. */
    nxo_stack_new(&thread->estack, a_nx, FALSE);
    nxo_stack_new(&thread->istack, a_nx, FALSE);
    nxo_stack_new(&thread->ostack, a_nx, FALSE);
    nxo_stack_new(&thread->dstack, a_nx, FALSE);
    nxo_stack_new(&thread->tstack, a_nx, FALSE);

    nxo_dup(&thread->stdin_nxo, nx_stdin_get(a_nx));
    nxo_dup(&thread->stdout_nxo, nx_stdout_get(a_nx));
    nxo_dup(&thread->stderr_nxo, nx_stderr_get(a_nx));

    /* Push threaddict, systemdict, and globaldict, onto the dictionary stack.
     * The embedded onyx initialization code creates userdict. */
    nxo = nxo_stack_push(&thread->dstack);
    nxo_dict_new(nxo, a_nx, FALSE, CW_LIBONYX_THREADDICT_HASH);

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

    start = nxo_stack_push(&thread->estack);
    nxo_operator_new(start, systemdict_start, NXN_start);
    nxo_attr_set(start, NXOA_EXECUTABLE);

    nxo_thread_loop(a_nxo);
}

void
nxo_thread_exit(cw_nxo_t *a_nxo)
{
    nx_l_thread_remove(nxo_thread_nx_get(a_nxo), a_nxo);
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

    /* Wait to be joined or detated, if not already so. */
    mtx_lock(&thread->lock);
    thread->done = TRUE;
    while (thread->detached == FALSE && thread->joined == FALSE)
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
	thread->gone = TRUE;
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
    thread->done = FALSE;
    thread->gone = FALSE;
    thread->detached = FALSE;
    thread->joined = FALSE;

    thread->thd = thd_new(nxo_p_thread_entry, (void *) thread, TRUE);
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
    thread->detached = TRUE;
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
    thread->joined = TRUE;
    if (thread->done)
    {
	/* The thread is already done, so wake it back up. */
	cnd_signal(&thread->done_cnd);
    }
    /* Wait for the thread to totally go away. */
    while (thread->gone == FALSE)
    {
	cnd_wait(&thread->join_cnd, &thread->lock);
    }
    mtx_unlock(&thread->lock);

    /* Clean up. */
    cnd_delete(&thread->join_cnd);
    cnd_delete(&thread->done_cnd);
    mtx_delete(&thread->lock);
    thd_join(thread->thd);

    nx_l_thread_remove(nxo_thread_nx_get(a_nxo), a_nxo);
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

cw_bool_t
nxo_thread_deferred(cw_nxo_t *a_nxo)
{
    cw_bool_t retval;
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    if (thread->defer_count != 0)
    {
	retval = TRUE;
    }
    else
    {
	retval = FALSE;
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

#if (1) /* XXX */
void
nxo_thread_loop(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *el, *cnxo, *enxo, *inxo, *tnxo;
    cw_uint32_t sdepth, cdepth;
#ifdef CW_DBG
    cw_uint32_t tdepth;
#endif

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    /* Push an object onto tstack that can be used to store array elements. */
    el = nxo_stack_push(&thread->tstack);

#ifdef CW_DBG
    /* The assertions about stack depth in this function check for tstack leaks
     * in operators. */
    tdepth = nxo_stack_count(&thread->tstack);
#endif

    /* Push an integer onto istack that corresponds to the object on estack
     * we're about to evaluate. */
    inxo = nxo_stack_push(&thread->istack);
    nxo_integer_new(inxo, 0);
    cw_assert(nxo_stack_count(&thread->estack)
	      == nxo_stack_count(&thread->istack));

    /* In the following loop, cnxo points to the object currently being
     * considered for evaluation, and enxo points to the top object on estack.
     * It may well be that cnxo points to an element within an array that is
     * pointed to by enxo. */

    sdepth = cdepth = nxo_stack_count(&thread->estack);
    cnxo = enxo = nxo_stack_get(&thread->estack);
    while (cdepth >= sdepth)
    {
	RESTART:
	if (0) /* XXX */
	{
	    const char *type, *attr;
	    switch (nxo_type_get(cnxo))
	    {
		case NXOT_NO:
		{
		    type = "NO";
		    break;
		}
		case NXOT_ARRAY:
		{
		    type = "ARRAY";
		    break;
		}
		case NXOT_BOOLEAN:
		{
		    type = "BOOLEAN";
		    break;
		}
#ifdef CW_THREADS
		case NXOT_CONDITION:
		{
		    type = "CONDITION";
		    break;
		}
#endif
		case NXOT_DICT:
		{
		    type = "DICT";
		    break;
		}
		case NXOT_FILE:
		{
		    type = "FILE";
		    break;
		}
		case NXOT_FINO:
		{
		    type = "FINO";
		    break;
		}
		case NXOT_HOOK:
		{
		    type = "HOOK";
		    break;
		}
		case NXOT_INTEGER:
		{
		    fprintf(stderr, "%lld:\t\t", nxo_integer_get(cnxo));
		    type = "INTEGER";
		    break;
		}
		case NXOT_MARK:
		{
		    type = "MARK";
		    break;
		}
#ifdef CW_THREADS
		case NXOT_MUTEX:
		{
		    type = "MUTEX";
		    break;
		}
#endif
		case NXOT_NAME:
		{
		    type = "NAME";
		    write(2, nxo_name_str_get(cnxo), nxo_name_len_get(cnxo));
		    fprintf(stderr, ":\t\t");
		    break;
		}
		case NXOT_NULL:
		{
		    type = "NULL";
		    break;
		}
		case NXOT_OPERATOR:
		{
		    fprintf(stderr, "%s:\t\t", nxn_str(nxo_l_operator_nxn_get(cnxo)));
		    type = "OPERATOR";
		    break;
		}
		case NXOT_PMARK:
		{
		    type = "PMARK";
		    break;
		}
#ifdef CW_REAL
		case NXOT_REAL:
		{
		    type = "REAL";
		    break;
		}
#endif
#ifdef CW_REGEX
		case NXOT_REGEX:
		{
		    type = "REGEX";
		    break;
		}
		case NXOT_REGSUB:
		{
		    type = "REGSUB";
		    break;
		}
#endif
		case NXOT_STACK:
		{
		    type = "STACK";
		    break;
		}
		case NXOT_STRING:
		{
		    write(2, nxo_string_get(cnxo), nxo_string_len_get(cnxo));
		    fprintf(stderr, ":\t\t");
		    type = "STRING";
		    break;
		}
		case NXOT_THREAD:
		{
		    type = "THREAD";
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }
	    switch (nxo_attr_get(cnxo))
	    {
		case NXOA_LITERAL:
		{
		    attr = "LITERAL";
		    break;
		}
		case NXOA_EXECUTABLE:
		{
		    attr = "EXECUTABLE";
		    break;
		}
		case NXOA_EVALUATABLE:
		{
		    attr = "EVALUATABLE";
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }
	    fprintf(stderr,
		    "[sco]depth: %u/%u/%u, i: %lld, type: %s, attr: %s \n",
		    sdepth, cdepth, nxo_stack_count(&thread->ostack),
		    nxo_integer_get(inxo), type, attr);
	}
	cw_assert(enxo != el); /* XXX Eventually remove. */
	cw_assert(cdepth == nxo_stack_count(&thread->estack));

	/* Make sure that the execution stack depth is within acceptable
	 * limits. */
	if (cdepth == CW_LIBONYX_ESTACK_MAX + 1)
	{
	    cw_error("XXX Got here");
	    nxo_thread_nerror(a_nxo, NXN_estackoverflow);
	}

	/* Process the object under consideration. */
	cw_assert(enxo == nxo_stack_get(&thread->estack));
	switch(nxo_type_get(cnxo))
	{
	    case NXOT_BOOLEAN:
#ifdef CW_THREADS
	    case NXOT_CONDITION:
#endif
	    case NXOT_DICT:
	    case NXOT_FINO:
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
		/* Always push the object onto the operand stack, regardless of
		 * its attribute. */
		tnxo = nxo_stack_push(&thread->ostack);
		nxo_dup(tnxo, cnxo);
		break;
	    }
	    case NXOT_NULL:
	    {
		/* Do absolutely nothing, unless the null is literal. */
		if (nxo_attr_get(cnxo) == NXOA_LITERAL)
		{
		    /* Push the object onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, cnxo);
		}
		break;
	    }
	    case NXOT_ARRAY:
	    {
		if (nxo_attr_get(cnxo) == NXOA_LITERAL)
		{
		    /* Push the object onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, cnxo);
		}
		else
		{
		    cw_nxoa_t attr;
		    cw_nxoi_t i;


		    /* Don't do anything in the case of a zero-length array. */
		    if (nxo_array_len_get(enxo) == 0)
		    {
			break;
		    }

		    /* Prepare to evaluate an element of the array, according to
		     * the corresponding istack value.  Take care to optimize
		     * out tail recursion here. */
		    i = nxo_integer_get(inxo);
		    /* XXX Move assertions to beginning of block. */
//		    fprintf(stderr, "0x%8p: %lld/%u(%u)\n", &i, i,
//			    nxo_array_len_get(enxo), nxo_array_len_get(cnxo));
		    cw_assert(nxo_type_get(enxo) == NXOT_ARRAY);
		    cw_assert(nxo_type_get(cnxo) == NXOT_ARRAY);
		    cw_assert(enxo == cnxo); // XXX Bad assertion?
		    nxo_l_array_el_get(enxo, i, el);
		    cnxo = el;
		    attr = nxo_attr_get(cnxo);
		    if ((i == nxo_array_len_get(enxo) - 1)
			&& (attr == NXOA_EVALUATABLE
			    || (attr == NXOA_EXECUTABLE
				&& nxo_type_get(cnxo) != NXOT_ARRAY)))
		    {
			/* Possible tail recursion.  Replace the array with its
			 * last element, then restart the outer loop. */
			tnxo = nxo_stack_push(&thread->estack);
			nxo_dup(tnxo, cnxo);
			nxo_stack_remove(&thread->estack, enxo);
			enxo = cnxo = tnxo;
			nxo_integer_set(inxo, 0);
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__,
//				__FUNCTION__);
			goto RESTART;
		    }

		    if (nxo_type_get(cnxo) == NXOT_ARRAY)
		    {
			if (nxo_attr_get(cnxo) != NXOA_EVALUATABLE)
			{
			    /* Only nested arrays with the evaluatable attribute
			     * are evaluated.  Push this nested array onto
			     * ostack, since it isn't evaluatable. */
			    tnxo = nxo_stack_push(&thread->ostack);
			    nxo_dup(tnxo, cnxo);
			}
			else
			{
			    /* Set up for nested array evaluation. */
			    enxo = nxo_stack_push(&thread->estack);
			    cdepth++;
			    nxo_dup(enxo, cnxo);
			    cnxo = enxo;

			    inxo = nxo_stack_push(&thread->istack);
			    nxo_integer_new(inxo, 0); // XXX ???

//			    fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__,
//				    __FUNCTION__);
			    goto RESTART; // XXX Correct?
			}
		    }
		    else
		    {
			/* Restart the outer loop to evaluate the element of the
			 * array. */
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__,
//				__FUNCTION__);
			goto RESTART;
		    }
		}
		break;
	    }
	    case NXOT_STRING:
	    {
		if (nxo_attr_get(cnxo) == NXOA_LITERAL)
		{
		    /* Push the object onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, cnxo);
		}
		else
		{
		    cw_nxo_threadp_t threadp;

		    /* Use the string as a source of code. */
		    nxo_threadp_new(&threadp);
#ifdef CW_THREADS
		    nxo_string_lock(cnxo);
#endif
		    nxo_thread_interpret(a_nxo, &threadp, nxo_string_get(cnxo),
					 nxo_string_len_get(cnxo));
#ifdef CW_THREADS
		    nxo_string_unlock(cnxo);
#endif
		    nxo_thread_flush(a_nxo, &threadp);
		    nxo_threadp_delete(&threadp, a_nxo);
		}
		break;
	    }
	    case NXOT_NAME:
	    {
		if (nxo_attr_get(cnxo) == NXOA_LITERAL)
		{
		    /* Push the object onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, cnxo);
		}
		else
		{
		    cw_nxo_t *name, *val;

		    /* Do potentially nested lookup. */
		    name = nxo_stack_push(&thread->tstack);
		    val = nxo_stack_push(&thread->tstack);
		    nxo_dup(val, cnxo);
		    do {
			nxo_dup(name, val);
			if (nxo_thread_dstack_search(a_nxo, name, val))
			{
			    nxo_thread_nerror(a_nxo, NXN_undefined);
			    goto NAME_UNDEFINED;
			}
		    } while (nxo_type_get(val) == NXOT_NAME
			     && nxo_attr_get(val) != NXOA_LITERAL);
		    if (nxo_attr_get(val) == NXOA_LITERAL)
		    {
			/* Push onto ostack. */
			tnxo = nxo_stack_push(&thread->ostack);
			nxo_dup(tnxo, val);
		    }
		    else
		    {
			if (enxo == cnxo)
			{
			    /* Replace the name on estack and prepare for
			     * evaluation. */
			    nxo_dup(cnxo, val);
			    enxo = cnxo;
			}
			else
			{
			    /* Push onto estack and prepare for evaluation. */
			    cnxo = enxo = nxo_stack_push(&thread->estack);
			    cdepth++;
			    nxo_dup(enxo, val);

			    inxo = nxo_stack_push(&thread->istack);
			    nxo_integer_new(inxo, 0);
			}

			nxo_stack_npop(&thread->tstack, 2);
			goto RESTART;
		    }
		    NAME_UNDEFINED:
		    nxo_stack_npop(&thread->tstack, 2);
		}
		break;
	    }
	    case NXOT_OPERATOR:
	    {
		if (nxo_attr_get(cnxo) == NXOA_LITERAL)
		{
		    /* Push the object onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, cnxo);
		}
		else
		{
		    nxo_operator_f(cnxo)(a_nxo);
		}
		break;
	    }
	    case NXOT_FILE:
	    {
		if (nxo_attr_get(cnxo) == NXOA_LITERAL)
		{
		    /* Push the object onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, cnxo);
		}
		else
		{
		    cw_nxo_threadp_t threadp;
		    cw_sint32_t nread;
		    cw_uint8_t buffer[CW_LIBONYX_FILE_EVAL_READ_SIZE];

		    nxo_threadp_new(&threadp);
		    /* Read data from the file and interpret it until an EOF (0
		     * byte read). */
		    for (nread = nxo_file_read(cnxo,
					       CW_LIBONYX_FILE_EVAL_READ_SIZE,
					       buffer);
			 nread > 0;
			 nread = nxo_file_read(cnxo,
					       CW_LIBONYX_FILE_EVAL_READ_SIZE,
					       buffer))
		    {
			nxo_thread_interpret(a_nxo, &threadp, buffer, nread);
		    }
		    /* Do not flush, so that syntax errors get caught. */
		    nxo_threadp_delete(&threadp, a_nxo);
		}
		break;
	    }
	    case NXOT_HOOK:
	    {
		if (nxo_attr_get(cnxo) == NXOA_LITERAL)
		{
		    /* Push the object onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, cnxo);
		}
		else
		{
		    nxo_hook_eval(cnxo, a_nxo);
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}

	/* Unwind estack as far as possible.  Array evaluation must be
	 * continued, but other types can be unilaterally unwound.  The branches
	 * that break out of this loop are those that initiate additional
	 * evaluation. */
	cdepth = nxo_stack_count(&thread->estack);
	do
	{
	    if (enxo == cnxo)
	    {
		/* Pop the object off of estack, now that it has been processed.
		 * Clean up istack state as well. */
		nxo_stack_pop(&thread->estack);
		nxo_stack_pop(&thread->istack);
		cdepth--;
		if (cdepth == 0)
		{
		    break;
		}

		/* Re-initialize variables. */
		enxo = cnxo = nxo_stack_get(&thread->estack);
		inxo = nxo_stack_get(&thread->istack);

		/* If this estack frame is an array, restore cnxo to point to
		 * the element that was most recently evaluated. */
		if (nxo_type_get(enxo) == NXOT_ARRAY)
		{
		    /* XXX Inefficient, should skip right to next element. */
		    nxo_array_el_get(enxo, nxo_integer_get(inxo), el);
		    cnxo = el;
		}
	    }
	    else
	    {
		if (nxo_type_get(enxo) == NXOT_ARRAY)
		{
		    cw_nxoa_t attr;
		    cw_nxoi_t i;

		    /* Set up for evaluating the next element in enxo. */

		    i = nxo_integer_get(inxo);
//		    fprintf(stderr, "0x%8p: %lld/%u\n", &i, i,
//			    nxo_array_len_get(enxo));
		    i++;
		    if (i < nxo_array_len_get(enxo))
		    {
			nxo_integer_set(inxo, i);
			nxo_l_array_el_get(enxo, i, el);
			cnxo = el;
			attr = nxo_attr_get(cnxo);
			if ((i == nxo_array_len_get(enxo) - 1)
			    && (attr == NXOA_EVALUATABLE
				|| (attr == NXOA_EXECUTABLE
				    && nxo_type_get(cnxo) != NXOT_ARRAY)))
			{
			    /* Possible tail recursion.  Replace the array with
			     * its last element before the next iteration of the
			     * outer loop. */
			    tnxo = nxo_stack_push(&thread->estack);
			    nxo_dup(tnxo, cnxo);
			    nxo_stack_remove(&thread->estack, enxo);
			    enxo = cnxo = tnxo;
			    nxo_integer_set(inxo, 0);
//			    fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__,
//				    __FUNCTION__);
			    break;
			}

			if (nxo_type_get(cnxo) == NXOT_ARRAY)
			{
			    if (nxo_attr_get(cnxo) != NXOA_EVALUATABLE)
			    {
				/* Only nested arrays with the evaluatable
				 * attribute are evaluated.  Push this nested
				 * array onto ostack, since it isn't
				 * evaluatable. */
				tnxo = nxo_stack_push(&thread->ostack);
				nxo_dup(tnxo, cnxo);
			    }
			    else
			    {
				/* Set up for nested array evaluation. */
				enxo = nxo_stack_push(&thread->estack);
				cdepth++;
				nxo_dup(enxo, cnxo);
				cnxo = enxo;

				inxo = nxo_stack_push(&thread->istack);
				nxo_integer_new(inxo, 0); // XXX ???

//				fprintf(stderr, "%s:%d:%s()\n", __FILE__,
//					__LINE__, __FUNCTION__);
				break;
			    }
			}
			else
			{
			    /* Evaluate the element of the array in the outer
			     * loop. */
//			    fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__,
//				    __FUNCTION__);
			    break;
			}
		    }
		    else
		    {
			/* Set up for further unwinding, since the last element
			 * of the array was already evaluated. */
			cnxo = enxo;
		    }
		}
		else
		{
		    /* A new object was pushed onto estack for evaluation.  Set
		     * cnxo to point to it, in preparation for the next outer
		     * loop iteration. */
		    cnxo = enxo;
		    break;
		}
	    }
	} while (cdepth >= sdepth);

	cw_assert(nxo_stack_count(&thread->tstack) == tdepth);
    }

    /* Pop el. */
    nxo_stack_pop(&thread->tstack);

    /* Make sure that estack and istack are consistent. */
    cw_assert(nxo_stack_count(&thread->estack) ==
	      nxo_stack_count(&thread->istack));
}
#elif (0)
void
nxo_thread_loop(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *nxo, *tnxo, *inxo;
#ifdef CW_DBG
    cw_uint32_t tdepth;
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

    { /* XXX Remove braces. */
	AGAIN:
	if (nxo_stack_count(&thread->estack) == CW_LIBONYX_ESTACK_MAX + 1)
	{
	    nxo_thread_nerror(a_nxo, NXN_estackoverflow);
	}

	nxo = nxo_stack_get(&thread->estack);

	switch (nxo_type_get(nxo))
	{
	    case NXOT_BOOLEAN:
#ifdef CW_THREADS
	    case NXOT_CONDITION:
#endif
	    case NXOT_DICT:
	    case NXOT_FINO:
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
		/* Always push the object onto the operand stack, regardless of
		 * its attribute. */
		tnxo = nxo_stack_push(&thread->ostack);
		nxo_dup(tnxo, nxo);
		nxo_stack_pop(&thread->estack);
		break;
	    }
	    case NXOT_NULL:
	    {
		if (nxo_attr_get(nxo) == NXOA_LITERAL)
		{
		    /* Always push literal objects onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, nxo);
		    nxo_stack_pop(&thread->estack);
		}
		else
		{
		    /* Do nothing. */
		    nxo_stack_pop(&thread->estack);
		}
		break;
	    }
	    case NXOT_ARRAY:
	    {
		if (nxo_attr_get(nxo) == NXOA_LITERAL)
		{
		    /* Always push literal objects onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, nxo);
		    nxo_stack_pop(&thread->estack);
		}
		else
		{
		    cw_uint32_t i, len;
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
		    el = nxo_stack_push(&thread->tstack);
		    for (i = 0; i < len - 1; i++)
		    {
			nxo_l_array_el_get(nxo, i, el);

			/* Set the execution index. */
			nxo_integer_set(inxo, i);

			cw_assert(nxo_type_get(el) <= NXOT_LAST);
			switch (nxo_type_get(el))
			{
			    case NXOT_BOOLEAN:
#ifdef CW_THREADS
			    case NXOT_CONDITION:
#endif
			    case NXOT_DICT:
			    case NXOT_FINO:
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
				 * stack, regardless of its attribute. */
				tnxo = nxo_stack_push(&thread->ostack);
				nxo_dup(tnxo, el);
				break;
			    }
			    case NXOT_NULL:
			    {
				if (nxo_attr_get(el) == NXOA_LITERAL)
				{
				    /* Push onto ostack. */
				    tnxo = nxo_stack_push(&thread->ostack);
				    nxo_dup(tnxo, el);
				}
				break;
			    }
			    case NXOT_ARRAY:
			    {
				/* Only execute nested arrays that have the
				 * evaluatable attribute. */
				if (nxo_attr_get(el) == NXOA_EVALUATABLE)
				{
				    tnxo = nxo_stack_push(&thread->estack);
				    nxo_dup(tnxo, el);
				    nxo_thread_loop(a_nxo);
				}
				else
				{
				    tnxo = nxo_stack_push(&thread->ostack);
				    nxo_dup(tnxo, el);
				}
				break;
			    }
			    case NXOT_NAME:
			    {
				if (nxo_attr_get(el) == NXOA_LITERAL)
				{
				    /* Push onto ostack. */
				    tnxo = nxo_stack_push(&thread->ostack);
				    nxo_dup(tnxo, el);
				}
				else
				{
				    cw_nxo_t *name, *val;

				    /* Do potentially nested lookup. */
				    name = nxo_stack_push(&thread->tstack);
				    val = nxo_stack_push(&thread->tstack);
				    nxo_dup(val, el);
				    do {
					nxo_dup(name, val);
					if (nxo_thread_dstack_search(a_nxo,
								     name, val))
					{
					    nxo_thread_nerror(a_nxo,
							      NXN_undefined);
					    break; // XXX Is this correct?
					}
				    } while (nxo_type_get(val) == NXOT_NAME
					     && nxo_attr_get(val)
					     == NXOA_EXECUTABLE);
				    /* XXX Inefficient. */
				    if (nxo_attr_get(val) == NXOA_LITERAL)
				    {
					/* Push onto ostack. */
					tnxo = nxo_stack_push(&thread->ostack);
					nxo_dup(tnxo, val);
				    }
				    else
				    {
					tnxo = nxo_stack_push(&thread->estack);
					nxo_dup(tnxo, val);
					nxo_thread_loop(a_nxo);
				    }
				    nxo_stack_npop(&thread->tstack, 2);
				}
				break;
			    }
			    case NXOT_OPERATOR:
			    {
				if (nxo_attr_get(el) == NXOA_LITERAL)
				{
				    /* Push onto ostack. */
				    tnxo = nxo_stack_push(&thread->ostack);
				    nxo_dup(tnxo, el);
				}
				else
				{
				    /* Execute operator. */
				    nxo_operator_f(el)(a_nxo);
				}
				break;
			    }
			    default:
			    {
				if (nxo_attr_get(el) == NXOA_LITERAL)
				{
				    /* Push onto ostack. */
				    tnxo = nxo_stack_push(&thread->ostack);
				    nxo_dup(tnxo, el);
				}
				else
				{
				    /* Use the generic algorithm. */
				    tnxo = nxo_stack_push(&thread->estack);
				    nxo_dup(tnxo, el);
				    nxo_thread_loop(a_nxo);
				}
			    }
			}
			cw_assert(nxo_stack_count(&thread->tstack)
				  == tdepth + 1);
		    }

		    /* Set the index back to 0 now that we're not executing an
		     * array any more. */
		    nxo_integer_set(inxo, 0); /* XXX Wrong!  This should happen
					       * only if optimizing tail
					       * recursion. */

		    /* If recursion is possible and likely, make tail recursion
		     * safe by replacing the array with its last element before
		     * executing the last element. */
		    nxo_l_array_el_get(nxo, i, el);
		    attr = nxo_attr_get(el);
		    if ((attr == NXOA_LITERAL)
			|| (nxo_type_get(el) == NXOT_ARRAY
			    && attr == NXOA_EXECUTABLE))
		    {
			/* Always push literal objects and nested executable
			 * (not evaluatable) arrays onto the operand stack. */
			tnxo = nxo_stack_push(&thread->ostack);
			nxo_dup(tnxo, el);
			nxo_stack_pop(&thread->estack);
			nxo_stack_pop(&thread->tstack);
		    }
		    else
		    {
			/* Possible recursion. */
			/* XXX Again. */
			nxo_dup(nxo, el);
			nxo_stack_pop(&thread->tstack);
			goto AGAIN;
		    }
		}
		break;
	    }
	    case NXOT_STRING:
	    {
		if (nxo_attr_get(nxo) == NXOA_LITERAL)
		{
		    /* Always push literal objects onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, nxo);
		    nxo_stack_pop(&thread->estack);
		}
		else
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
		}
		break;
	    }
	    case NXOT_NAME:
	    {
		if (nxo_attr_get(nxo) == NXOA_LITERAL)
		{
		    /* Always push literal objects onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, nxo);
		    nxo_stack_pop(&thread->estack);
		}
		else
		{
		    cw_nxo_t *name;

		    /* XXX Do nested lookup in place! */
		    /* Search for a value associated with the name in the
		     * dictionary stack, and put it on the execution stack, in
		     * preparation for the next iteration through the execution
		     * loop.  Thus, nested name lookups work correctly. */
		    name = nxo_stack_push(&thread->tstack);
		    nxo_dup(name, nxo);
		    if (nxo_thread_dstack_search(a_nxo, name, nxo))
		    {
			nxo_thread_nerror(a_nxo, NXN_undefined);
			nxo_stack_pop(&thread->estack);
		    }
		    nxo_stack_pop(&thread->tstack);
		    /* XXX Again. */
		    goto AGAIN;
		}
		break;
	    }
	    case NXOT_OPERATOR:
	    {
		if (nxo_attr_get(nxo) == NXOA_LITERAL)
		{
		    /* Always push literal objects onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, nxo);
		    nxo_stack_pop(&thread->estack);
		}
		else
		{
		    /* Execute the operator. */
		    nxo_operator_f(nxo)(a_nxo);
		    nxo_stack_pop(&thread->estack);
		}
		break;
	    }
	    case NXOT_FILE:
	    {
		if (nxo_attr_get(nxo) == NXOA_LITERAL)
		{
		    /* Always push literal objects onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, nxo);
		    nxo_stack_pop(&thread->estack);
		}
		else
		{
		    cw_nxo_threadp_t threadp;
		    cw_sint32_t nread;
		    cw_uint8_t buffer[CW_LIBONYX_FILE_EVAL_READ_SIZE];

		    nxo_threadp_new(&threadp);
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
		}
		break;
	    }
	    case NXOT_HOOK:
	    {
		if (nxo_attr_get(nxo) == NXOA_LITERAL)
		{
		    /* Always push literal objects onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, nxo);
		    nxo_stack_pop(&thread->estack);
		}
		else
		{
		    nxo_hook_eval(nxo, a_nxo);
		    nxo_stack_pop(&thread->estack);
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
	cw_assert(nxo_stack_count(&thread->tstack) == tdepth);
    }

    /* Pop the execution index for this onyx stack frame. */
    nxo_stack_pop(&thread->istack);
    cw_assert(nxo_stack_count(&thread->estack) ==
	      nxo_stack_count(&thread->istack));
}
#else
void
nxo_thread_loop(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *nxo, *tnxo, *inxo;
    cw_uint32_t sdepth, cdepth;
#ifdef CW_DBG
    cw_uint32_t tdepth;
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

    for (sdepth = cdepth = nxo_stack_count(&thread->estack);
	 cdepth >= sdepth;
	 cdepth = nxo_stack_count(&thread->estack))
    {
	if (cdepth == CW_LIBONYX_ESTACK_MAX + 1)
	{
	    nxo_thread_nerror(a_nxo, NXN_estackoverflow);
	}

	nxo = nxo_stack_get(&thread->estack);
	if (nxo_attr_get(nxo) == NXOA_LITERAL)
	{
	    /* Always push literal objects onto the operand stack. */
	    tnxo = nxo_stack_push(&thread->ostack);
	    nxo_dup(tnxo, nxo);
	    nxo_stack_pop(&thread->estack);
	    continue;
	}

	switch (nxo_type_get(nxo))
	{
	    case NXOT_BOOLEAN:
#ifdef CW_THREADS
	    case NXOT_CONDITION:
#endif
	    case NXOT_DICT:
	    case NXOT_FINO:
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
		/* Always push the object onto the operand stack, even though it
		 * isn't literal. */
		tnxo = nxo_stack_push(&thread->ostack);
		nxo_dup(tnxo, nxo);
		nxo_stack_pop(&thread->estack);
		break;
	    }
	    case NXOT_NULL:
	    {
		/* Do nothing. */
		nxo_stack_pop(&thread->estack);
		break;
	    }
	    case NXOT_ARRAY:
	    {
		cw_uint32_t i, len;
		cw_nxo_t *el;
		cw_nxoa_t attr;

		len = nxo_array_len_get(nxo);
		if (len == 0)
		{
		    nxo_stack_pop(&thread->estack);
		    break;
		}

		/* Iterate through the array and execute each element in turn.
		 * The generic algorithm is simply to push an element onto
		 * estack and recurse, but the overhead of the pushing,
		 * recursion, and popping is excessive for the common cases of a
		 * simple object or operator.  Therefore, check for the most
		 * common simple cases and handle them specially. */
		el = nxo_stack_push(&thread->tstack);
		for (i = 0; i < len - 1; i++)
		{
		    nxo_l_array_el_get(nxo, i, el);
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
			     * evaluatable attribute. */
			    if (attr == NXOA_EVALUATABLE)
			    {
				tnxo = nxo_stack_push(&thread->estack);
				nxo_dup(tnxo, el);
				nxo_thread_loop(a_nxo);
			    }
			    else
			    {
				tnxo = nxo_stack_push(&thread->ostack);
				nxo_dup(tnxo, el);
			    }
			    break;
			}
			case NXOT_OPERATOR:
			{
			    nxo_operator_f(el)(a_nxo);
			    break;
			}
			default:
			{
			    /* Not a simple common case, so use the generic
			     * algorithm. */
			    tnxo = nxo_stack_push(&thread->estack);
			    nxo_dup(tnxo, el);
			    nxo_thread_loop(a_nxo);
			}
		    }
		    cw_assert(nxo_stack_count(&thread->tstack) == tdepth + 1);
		}

		/* Set the index back to 0 now that we're not executing an array
		 * any more. */
		nxo_integer_set(inxo, 0);

		/* If recursion is possible and likely, make tail recursion safe
		 * by replacing the array with its last element before executing
		 * the last element. */
		nxo_l_array_el_get(nxo, i, el);
		attr = nxo_attr_get(el);
		if ((attr == NXOA_LITERAL)
		    || (nxo_type_get(el) == NXOT_ARRAY
			&& attr == NXOA_EXECUTABLE))
		{
		    /* Always push literal objects and nested executable (not
		     * evaluatable) arrays onto the operand stack. */
		    tnxo = nxo_stack_push(&thread->ostack);
		    nxo_dup(tnxo, el);
		    nxo_stack_pop(&thread->estack);
		}
		else
		{
		    /* Possible recursion. */
		    nxo_dup(nxo, el);
		}
		nxo_stack_pop(&thread->tstack);
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
	    case NXOT_NAME:
	    {
		cw_nxo_t *name;

		/* Search for a value associated with the name in the dictionary
		 * stack, and put it on the execution stack, in preparation for
		 * the next iteration through the execution loop.  Thus, nested
		 * name lookups work correctly. */
		name = nxo_stack_push(&thread->tstack);
		nxo_dup(name, nxo);
		if (nxo_thread_dstack_search(a_nxo, name, nxo))
		{
		    nxo_thread_nerror(a_nxo, NXN_undefined);
		    nxo_stack_pop(&thread->estack);
		}
		nxo_stack_pop(&thread->tstack);
		break;
	    }
	    case NXOT_OPERATOR:
	    {
		nxo_operator_f(nxo)(a_nxo);
		nxo_stack_pop(&thread->estack);
		break;
	    }
	    case NXOT_FILE:
	    {
		cw_nxo_threadp_t threadp;
		cw_sint32_t nread;
		cw_uint8_t buffer[CW_LIBONYX_FILE_EVAL_READ_SIZE];

		nxo_threadp_new(&threadp);
		/* Read data from the file and interpret it until an EOF (0 byte
		 * read). */
		for (nread = nxo_file_read(nxo, CW_LIBONYX_FILE_EVAL_READ_SIZE,
					   buffer);
		     nread > 0;
		     nread = nxo_file_read(nxo, CW_LIBONYX_FILE_EVAL_READ_SIZE,
					   buffer))
		{
		    nxo_thread_interpret(a_nxo, &threadp, buffer, nread);
		}
		/* Do not flush, so that syntax errors get caught. */
		nxo_threadp_delete(&threadp, a_nxo);

		nxo_stack_pop(&thread->estack);
		break;
	    }
	    case NXOT_HOOK:
	    {
		nxo_hook_eval(nxo, a_nxo);

		nxo_stack_pop(&thread->estack);
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
	cw_assert(nxo_stack_count(&thread->tstack) == tdepth);
    }

    /* Pop the execution index for this onyx stack frame. */
    nxo_stack_pop(&thread->istack);
    cw_assert(nxo_stack_count(&thread->estack) ==
	      nxo_stack_count(&thread->istack));
}
#endif

void
nxo_thread_interpret(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp, const
		     cw_uint8_t *a_str, cw_uint32_t a_len)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    nxoe_p_thread_feed(thread, a_threadp, FALSE, a_str, a_len);
}

void
nxo_thread_flush(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp)
{
    cw_nxoe_thread_t *thread;
    static const cw_uint8_t str[] = "\n";

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    nxoe_p_thread_feed(thread, a_threadp, FALSE, str, sizeof(str) - 1);
}

void
nxo_thread_nerror(cw_nxo_t *a_nxo, cw_nxn_t a_nxn)
{
    nxo_thread_serror(a_nxo, nxn_str(a_nxn), nxn_len(a_nxn));
}

void
nxo_thread_serror(cw_nxo_t *a_nxo, const cw_uint8_t *a_str, cw_uint32_t a_len)
{
    cw_nxoe_thread_t *thread;
    cw_nxo_t *errorname;
    cw_uint32_t defer_count;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    /* Convert a_str to a name object on ostack. */
    errorname = nxo_stack_push(&thread->ostack);
    nxo_name_new(errorname, thread->nx, a_str, a_len, FALSE);

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

cw_bool_t
nxo_thread_dstack_search(cw_nxo_t *a_nxo, cw_nxo_t *a_key, cw_nxo_t *r_value)
{
    cw_bool_t retval;
    cw_nxoe_thread_t *thread;
    cw_nxo_t *dict;
    cw_uint32_t i, depth;

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
	dict = nxo_stack_down_get(&thread->dstack, dict);
	if (nxo_dict_lookup(dict, a_key, r_value) == FALSE)
	{
	    /* Found. */
	    retval = FALSE;
	    goto RETURN;
	}
    }

    retval = TRUE;
    RETURN:
    return retval;
}

#ifdef CW_THREADS
cw_bool_t
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
nxo_thread_setlocking(cw_nxo_t *a_nxo, cw_bool_t a_locking)
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

cw_uint32_t
nxo_l_thread_token(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp, const
		   cw_uint8_t *a_str, cw_uint32_t a_len)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    return nxoe_p_thread_feed(thread, a_threadp, TRUE, a_str, a_len);
}

static cw_uint32_t
nxoe_p_thread_feed(cw_nxoe_thread_t *a_thread, cw_nxo_threadp_t *a_threadp,
		   cw_bool_t a_token, const cw_uint8_t *a_str,
		   cw_uint32_t a_len)
{
    cw_uint32_t retval, i, newline, defer_base;
    cw_uint8_t c;
    cw_nxo_t *nxo;
    cw_bool_t token;

    if (a_token)
    {
	/* Artificially inflate the defer count so that when we accept a token,
	 * it doesn't get evaluated. */
	defer_base = 1;
	a_thread->defer_count++;
	/* The value of token is only used when a_token is true, so only bother
	 * to initialize it in this case. */
	token = FALSE;
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
		    /* token is TRUE if a token has been accepted.  We look for
		     * the situation where token is TRUE and
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
			token = TRUE;
			a_thread->m.m.action = ACTION_EXECUTE;
			nxoe_p_thread_name_accept(a_thread);
			break;
		    }
		    case '{':
		    {
			a_thread->defer_count++;
			nxo = nxo_stack_push(&a_thread->ostack);
			nxo_pmark_new(nxo);
			break;
		    }
		    case '}':
		    {
			if (a_thread->defer_count > defer_base)
			{
			    token = TRUE;
			    a_thread->defer_count--;
			    nxoe_p_thread_procedure_accept(
				a_thread);
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
			a_thread->m.n.mant_neg = FALSE;
			a_thread->m.n.radix_base = 10;
			a_thread->m.n.whole = FALSE;
#ifdef CW_REAL
			a_thread->m.n.frac = FALSE;
			a_thread->m.n.exp = FALSE;
#endif
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '-':
		    {
			a_thread->state = THREADTS_INTEGER;
			a_thread->m.n.mant_neg = TRUE;
			a_thread->m.n.radix_base = 10;
			a_thread->m.n.whole = FALSE;
#ifdef CW_REAL
			a_thread->m.n.frac = FALSE;
			a_thread->m.n.exp = FALSE;
#endif
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
#ifdef CW_REAL
		    case '.':
		    {
			a_thread->state = THREADTS_REAL_FRAC;
			a_thread->m.n.mant_neg = FALSE;
			a_thread->m.n.radix_base = 10;
			a_thread->m.n.whole = FALSE;
			a_thread->m.n.frac = FALSE;
			a_thread->m.n.exp = FALSE;
			break;
		    }
#endif
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			a_thread->state = THREADTS_INTEGER;
			a_thread->m.n.mant_neg = FALSE;
			a_thread->m.n.radix_base = 10;
			a_thread->m.n.whole = TRUE;
			a_thread->m.n.whole_off = 0;
#ifdef CW_REAL
			a_thread->m.n.frac = FALSE;
			a_thread->m.n.exp = FALSE;
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
		cw_bool_t restart = FALSE;

		switch (c)
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			if (a_thread->m.n.whole == FALSE)
			{
			    a_thread->m.n.whole = TRUE;
			    a_thread->m.n.whole_off = a_thread->index;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '@':
		    {
			cw_uint32_t digit, ndigits;

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
			    a_thread->m.n.whole = FALSE;
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
			a_thread->m.n.exp_sign = FALSE;
			a_thread->m.n.exp_neg = FALSE;
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
			restart = TRUE; /* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!': case '$':
		    case '~': case '#':
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
			token = TRUE;
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
		cw_bool_t restart = FALSE;

		switch (c)
		{
		    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		    case 'y': case 'z':
		    {
			if (a_thread->m.n.whole == FALSE)
			{
			    a_thread->m.n.whole = TRUE;
			    a_thread->m.n.whole_off = a_thread->index;
			}
			if (a_thread->m.n.radix_base
			    <= (10 + ((cw_uint32_t) (c - 'a'))))
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
			if (a_thread->m.n.whole == FALSE)
			{
			    a_thread->m.n.whole = TRUE;
			    a_thread->m.n.whole_off = a_thread->index;
			}
			if (a_thread->m.n.radix_base
			    <= (10 + ((cw_uint32_t) (c - 'A'))))
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
			if (a_thread->m.n.whole == FALSE)
			{
			    a_thread->m.n.whole = TRUE;
			    a_thread->m.n.whole_off = a_thread->index;
			}
			if (a_thread->m.n.radix_base
			    <= ((cw_uint32_t) (c - '0')))
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
			restart = TRUE; /* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!': case '$':
		    case '~': case '#':
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
			if (a_thread->m.n.whole == FALSE
			    || nxoe_p_thread_integer_accept(a_thread))
			{
			    /* Conversion error.  Accept as a name. */
			    a_thread->m.m.action = ACTION_EXECUTE;
			    nxoe_p_thread_name_accept(a_thread);
			}
			token = TRUE;
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
		cw_bool_t restart = FALSE;

		switch (c)
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			if (a_thread->m.n.frac == FALSE)
			{
			    a_thread->m.n.frac = TRUE;
			    a_thread->m.n.frac_off = a_thread->index;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case 'e': case 'E':
		    {
			a_thread->state = THREADTS_REAL_EXP;
			a_thread->m.n.exp_sign = FALSE;
			a_thread->m.n.exp_neg = FALSE;
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
			restart = TRUE; /* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!': case '$':
		    case '~': case '#':
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
			token = TRUE;
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
		cw_bool_t restart = FALSE;

		switch (c)
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
			if (a_thread->m.n.exp == FALSE)
			{
			    a_thread->m.n.exp = TRUE;
			    a_thread->m.n.exp_off = a_thread->index;
			}
			CW_NXO_THREAD_PUTC(c);
			break;
		    }
		    case '+':
		    {
			if (a_thread->m.n.exp_sign == FALSE
			    && a_thread->m.n.exp == FALSE)
			{
			    a_thread->m.n.exp_sign = TRUE;
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
			if (a_thread->m.n.exp_sign == FALSE
			    && a_thread->m.n.exp == FALSE)
			{
			    a_thread->m.n.exp_sign = TRUE;
			    a_thread->m.n.exp_neg = TRUE;
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
			restart = TRUE; /* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!': case '$':
		    case '~': case '#':
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
			if (a_thread->m.n.exp == FALSE
			    || nxoe_p_thread_real_accept(a_thread))
			{
			    /* Conversion error.  Accept as a name. */
			    a_thread->m.m.action = ACTION_EXECUTE;
			    nxoe_p_thread_name_accept(a_thread);
			}
			token = TRUE;
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
			    token = TRUE;
			    nxo = nxo_stack_push(&a_thread->ostack);
#ifdef CW_THREADS
			    nxo_string_new(nxo, a_thread->nx, a_thread->locking,
					   a_thread->index);
#else
			    nxo_string_new(nxo, a_thread->nx, FALSE,
					   a_thread->index);
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
			cw_uint8_t val;

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
			cw_uint8_t suffix[] = "\\x?";

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
		    case '[': case ']': case '{': case '}': case '!': case '$':
		    case '~': case '#':
		    {
			cw_uint8_t suffix[2] = "?";

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
		cw_bool_t restart = FALSE;

		switch (c)
		{
		    case '\n':
		    {
			restart = TRUE;	/* Inverted below. */
			CW_NXO_THREAD_NEWLINE();
			/* Fall through. */
		    }
		    case '(': case ')': case '`': case '\'': case '<': case '>':
		    case '[': case ']': case '{': case '}': case '!': case '$':
		    case '~': case '#':
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
			    token = TRUE;
			    nxoe_p_thread_name_accept(a_thread);
			}
			else
			{
			    cw_uint8_t suffix[2] = "?";

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
    cw_nxa_t *nxa;

    nxa = nx_nxa_get(a_thread->nx);

    if (a_thread->index == CW_NXO_THREAD_BUFFER_SIZE)
    {
	/* First overflow, initial expansion needed. */
	a_thread->tok_str = (cw_uint8_t *) nxa_malloc(nxa, a_thread->index * 2);
	a_thread->buffer_len = a_thread->index * 2;
	memcpy(a_thread->tok_str, a_thread->buffer, a_thread->index);
    }
    else if (a_thread->index == a_thread->buffer_len)
    {
	cw_uint8_t *t_str;

	/* Overflowed, and additional expansion needed. */
	t_str = (cw_uint8_t *) nxa_malloc(nxa, a_thread->index * 2);
	a_thread->buffer_len = a_thread->index * 2;
	memcpy(t_str, a_thread->tok_str, a_thread->index);
	nxa_free(nxa, a_thread->tok_str, a_thread->index);
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
			   cw_uint32_t a_defer_base, cw_uint8_t *a_prefix,
			   cw_uint8_t *a_suffix, cw_sint32_t a_c)
{
    cw_nxo_t *nxo;
    cw_uint32_t defer_count, line, column;

    nxo = nxo_stack_push(&a_thread->ostack);

#ifdef CW_THREADS
    nxo_string_new(nxo, a_thread->nx, a_thread->locking,
		   strlen((char *) a_prefix) + a_thread->index
		   + strlen((char *) a_suffix) + ((a_c >= 0) ? 1 : 0));
#else
    nxo_string_new(nxo, a_thread->nx, FALSE,
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
	cw_uint8_t c = (cw_uint8_t) a_c;

	nxo_string_set(nxo, strlen((char *) a_prefix) + a_thread->index +
		       strlen((char *) a_suffix), &c, 1);
    }

    nxoe_p_thread_reset(a_thread);

    /* Push line and column onto ostack.  They are used in the embedded onyx
     * code below to set line and column in currenterror. */
    nxo_threadp_position_get(a_threadp, &line, &column);

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
		 "currenterror begin $column exch def $line exch def end"
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
	nxa_free(nx_nxa_get(a_thread->nx), a_thread->tok_str,
		 a_thread->buffer_len);
	a_thread->tok_str = a_thread->buffer;
    }
    a_thread->index = 0;
}

static cw_bool_t
nxoe_p_thread_integer_accept(cw_nxoe_thread_t *a_thread)
{
    cw_bool_t retval;

    if (a_thread->m.n.whole)
    {
	cw_nxo_t *nxo;
	cw_nxoi_t val;
	cw_uint32_t i;
	cw_uint64_t threshold, maxlast, sum, digit;
	cw_uint8_t c;

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
		    digit = 10 + ((cw_uint64_t) (c - 'a'));
		    break;
		}
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		{
		    digit = 10 + ((cw_uint64_t) (c - 'A'));
		    break;
		}
		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
		{
		    digit = (cw_uint64_t) (c - '0');
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
		retval = TRUE;
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
	retval = TRUE;
	goto RETURN;
    }

    retval = FALSE;
    RETURN:
    return retval;
}

#ifdef CW_REAL
static cw_bool_t
nxoe_p_thread_real_accept(cw_nxoe_thread_t *a_thread)
{
    cw_bool_t retval;
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
	retval = TRUE;
	goto RETURN;
    }

    /* Create a real on ostack. */
    nxo = nxo_stack_push(&a_thread->ostack);
    nxo_real_new(nxo, val);
    nxoe_p_thread_reset(a_thread);

    retval = FALSE;
    RETURN:
    return retval;
}
#endif

static void
nxoe_p_thread_procedure_accept(cw_nxoe_thread_t *a_thread)
{
    cw_nxo_t *tnxo, *nxo;
    cw_uint32_t nelements, i, depth;

    /* Find the pmark. */
    for (i = 0, depth = nxo_stack_count(&a_thread->ostack), nxo = NULL;
	 i < depth;
	 i++)
    {
	nxo = nxo_stack_down_get(&a_thread->ostack, nxo);
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
    nxo_array_new(tnxo, a_thread->nx, a_thread->locking, nelements);
#else
    nxo_array_new(tnxo, a_thread->nx, FALSE, nelements);
#endif
    nxo_attr_set(tnxo, NXOA_EXECUTABLE);

    /* Traverse down the stack, moving nxo's to the array. */
    for (i = nelements, nxo = NULL; i > 0; i--)
    {
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
    cw_nxo_t *nxo;

    switch (a_thread->m.m.action)
    {
	case ACTION_EXECUTE:
	case ACTION_EVALUATE:
	{
	    if (a_thread->defer_count == 0)
	    {
		/* Find the the value associated with the name in the dictionary
		 * stack, push it onto the execution stack, and run the
		 * execution loop. */
		nxo = nxo_stack_push(&a_thread->estack);
		nxo_name_new(nxo, a_thread->nx, a_thread->tok_str,
			     a_thread->index, FALSE);
		nxo_attr_set(nxo, a_thread->m.m.action);

		nxoe_p_thread_reset(a_thread);
		nxo_thread_loop(&a_thread->self);
	    }
	    else
	    {
		/* Push the name object onto the operand stack. */
		nxo = nxo_stack_push(&a_thread->ostack);
		nxo_name_new(nxo, a_thread->nx, a_thread->tok_str,
			     a_thread->index, FALSE);
		nxo_attr_set(nxo, a_thread->m.m.action);
		nxoe_p_thread_reset(a_thread);
	    }
	    break;
	}
	case ACTION_LITERAL:
	{
	    /* Push the name object onto the operand stack. */
	    nxo = nxo_stack_push(&a_thread->ostack);
	    nxo_name_new(nxo, a_thread->nx, a_thread->tok_str, a_thread->index,
			 FALSE);
	    nxoe_p_thread_reset(a_thread);
	    break;
	}
	case ACTION_IMMEDIATE:
	{
	    cw_nxo_t *key;

	    /* Find the value associated with the name in the dictionary stack
	     * and push the value onto the operand stack. */
	    key = nxo_stack_push(&a_thread->tstack);
	    nxo_name_new(key, a_thread->nx, a_thread->tok_str, a_thread->index,
			 FALSE);
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
		/* Set the evaluatable attribute so that the array will still be
		 * executed when interpreted. */
		nxo_attr_set(nxo, NXOA_EVALUATABLE);
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
