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

#include "libonyx/libonyx.h"
#include "../include/modprompt.h"

#include <errno.h>
#include <histedit.h>

#define CW_PROMPT_STRLEN 80
#define CW_BUFFER_SIZE 512

struct cw_modprompt_synth_s {
#ifdef CW_DBG
#define CW_MODPROMPT_SYNTH_MAGIC 0x32ad81a5
    cw_uint32_t magic;
#endif
    cw_nxo_t modload_hook;
    cw_nxo_t *thread;
    cw_nxo_threadp_t threadp;
    cw_bool_t continuation;
#ifdef CW_THREADS
    cw_thd_t *read_thd;
    volatile cw_bool_t quit;
    volatile cw_bool_t resize;

    cw_mtx_t mtx;
    cw_bool_t have_data; /* The writer wrote data. */
    cw_cnd_t put_cnd; /* The writer (libedit thread) waits on this. */
    cw_bool_t want_data; /* The reader wants data. */
    cw_cnd_t get_cnd; /* The reader (interpreter thread) waits on this. */
#endif

    cw_uint8_t *buffer;
    cw_uint32_t buffer_size;
    cw_uint32_t buffer_count;

    EditLine *el;
    History *hist;
    HistEvent hevent;
    cw_uint8_t prompt_str[CW_PROMPT_STRLEN];
};

/* Globals. */
/* This must be global so that the signal handler can get to it. */
static struct cw_modprompt_synth_s *synth;

/* Function prototypes. */
static cw_nxoe_t *
modprompt_synth_ref_iter(void *a_data, cw_bool_t a_reset);

static void
modprompt_synth_delete(void *a_data);

static cw_sint32_t
modprompt_read(void *a_data, cw_nxo_t *a_file, cw_uint32_t a_len,
	       cw_uint8_t *r_str);

static void
modprompt_promptstring(struct cw_modprompt_synth_s *a_synth);

static char *
modprompt_prompt(EditLine *a_el);

static void
modprompt_handlers_install(void);

static void
modprompt_signal_handle(int a_signal);

#ifdef CW_THREADS
static void *
modprompt_entry(void *a_arg);
#endif

void
modprompt_init(void *a_arg, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *file;
    char *editor;
#ifdef CW_THREADS
    sigset_t set, oset;
#endif

    synth = (struct cw_modprompt_synth_s *)
	cw_calloc(1, sizeof(struct cw_modprompt_synth_s));

    /* Initialize stdin.  Only convert the initial thread's stdin, since it
     * isn't safe for multiple threads to use the synthetic file.  If the
     * application is crazy enough use the initial thread's stdin in another
     * thread, crashes are likely. */
    file = nxo_thread_stdin_get(a_thread);
    nxo_file_new(file, TRUE);
    nxo_file_synthetic(file, modprompt_read, NULL, modprompt_synth_ref_iter,
		       modprompt_synth_delete, synth);
    nxo_attr_set(file, NXOA_EXECUTABLE);

    /* The interpreter is currently executing a hook that holds a reference to
     * the dynamically loaded module.  Keep a reference to it, so that this
     * module will not be unloaded until after the synthetic file object has
     * been destroyed. */
    estack = nxo_thread_estack_get(a_thread);
    nxo_no_new(&synth->modload_hook);
    nxo_dup(&synth->modload_hook, nxo_stack_get(estack));

    /* Finish initializing synth. */
    synth->thread = a_thread;
    nxo_threadp_new(&synth->threadp);
#ifdef CW_THREADS
    mtx_new(&synth->mtx);
    cnd_new(&synth->put_cnd);
    cnd_new(&synth->get_cnd);
#endif

    /* Initialize the command editor. */
    synth->hist = history_init();
    history(synth->hist, &synth->hevent, H_SETSIZE, 512);

    synth->el = el_init("onyx", stdin, stdout, stderr);
    el_set(synth->el, EL_HIST, history, synth->hist);
    el_set(synth->el, EL_PROMPT, modprompt_prompt);
    el_set(synth->el, EL_CLIENTDATA, synth);

    editor = getenv("ONYX_EDITOR");
    if (editor == NULL || (strcmp(editor, "emacs") && strcmp(editor, "vi")))
    {
	/* Default to emacs key bindings, since they're more intuitive to the
	 * uninitiated. */
	editor = "emacs";
    }
    el_set(synth->el, EL_EDITOR, editor);
#ifdef CW_DBG
    synth->magic = CW_MODPROMPT_SYNTH_MAGIC;
#endif

#ifdef CW_THREADS
    /* Mask all signals that the read thread handles, so that they're always
     * delivered there. */
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTSTP);
    sigaddset(&set, SIGSTOP);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGCONT);
    sigaddset(&set, SIGWINCH);
    thd_sigmask(SIG_BLOCK, &set, &oset);
    synth->read_thd = thd_new(modprompt_entry, synth, TRUE);
#else
    /* Install signal handlers. */
    modprompt_handlers_install();
#endif

    /* stdin is now a synthetic file.  This does not change any file objects
     * which may already be on estack, but under normal circumstances, this
     * module is loaded by the bootstrap code, and stdin is not pushed onto
     * estack and executed until after the bootstrap code has been completely
     * executed.
     *
     * If this module is loaded later on, once stdin is being read from, loading
     * this module will have no apparent effect unless something like the
     * following code is recursed into:
     *
     *   stdin cvx eval
     *
     * Strange things may happen though, since the file object already on estack
     * may have already buffered data, but the data will not be evaluated until
     * EOF is returned by the synthetic stdin. */
}

static cw_nxoe_t *
modprompt_synth_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_modprompt_synth_s *synth = (struct cw_modprompt_synth_s *) a_data;

    cw_check_ptr(synth);
    cw_dassert(synth->magic == CW_MODPROMPT_SYNTH_MAGIC);

    if (a_reset)
    {
	retval = nxo_nxoe_get(&synth->modload_hook);
    }
    else
    {
	retval = NULL;
    }

    return retval;
}

static void
modprompt_synth_delete(void *a_data)
{
    struct cw_modprompt_synth_s *synth = (struct cw_modprompt_synth_s *) a_data;

    cw_check_ptr(synth);
    cw_dassert(synth->magic == CW_MODPROMPT_SYNTH_MAGIC);

#ifdef CW_THREADS
    /* Tell the read thread to start shutting down. */
    mtx_lock(&synth->mtx);
    synth->quit = TRUE;
    cnd_signal(&synth->put_cnd);
    mtx_unlock(&synth->mtx);
    /* Wait for the read thread to exit. */
    thd_join(synth->read_thd);
    /* Clean up. */
    mtx_delete(&synth->mtx);
    cnd_delete(&synth->put_cnd);
    cnd_delete(&synth->get_cnd);
#endif

    if (synth->buffer != NULL)
    {
	cw_free(synth->buffer);
    }

    /* Clean up the command editor. */
    el_end(synth->el);
    history_end(synth->hist);

    cw_free(synth);
}

#ifdef CW_THREADS
static cw_sint32_t
modprompt_read(void *a_data, cw_nxo_t *a_file, cw_uint32_t a_len,
	       cw_uint8_t *r_str)
{
    cw_sint32_t retval;
    struct cw_modprompt_synth_s *synth = (struct cw_modprompt_synth_s *) a_data;

    cw_check_ptr(synth);
    cw_dassert(synth->magic == CW_MODPROMPT_SYNTH_MAGIC);
    cw_assert(a_len > 0);

    mtx_lock(&synth->mtx);

    if (synth->buffer_count == 0)
    {
	/* Tell the main thread to read more data, then wait for it. */
	synth->want_data = TRUE;
	cnd_signal(&synth->put_cnd);
	synth->have_data = FALSE;
	while (synth->have_data == FALSE)
	{
	    cnd_wait(&synth->get_cnd, &synth->mtx);
	}
    }
    cw_assert(synth->buffer_count > 0);

    /* Return as much of the data as possible. */
    if (synth->buffer_count > a_len)
    {
	/* There are more data than we can return. */
	retval = a_len;
	memcpy(r_str, synth->buffer, a_len);
	synth->buffer_count -= a_len;
	memmove(synth->buffer, &synth->buffer[a_len], synth->buffer_count);
    }
    else
    {
	/* Return all the data. */
	retval = synth->buffer_count;
	memcpy(r_str, synth->buffer, synth->buffer_count);
	synth->buffer_count = 0;
    }

    mtx_unlock(&synth->mtx);
    return retval;
}
#else
static cw_sint32_t
modprompt_read(void *a_data, cw_nxo_t *a_file, cw_uint32_t a_len,
	       cw_uint8_t *r_str)
{
    cw_sint32_t retval;
    struct cw_modprompt_synth_s *synth = (struct cw_modprompt_synth_s *) a_data;
    const char *str;
    int count = 0;

    cw_check_ptr(synth);
    cw_dassert(synth->magic == CW_MODPROMPT_SYNTH_MAGIC);
    cw_assert(a_len > 0);

    if (synth->buffer_count == 0)
    {
	/* Update the promptstring if necessary. */
	modprompt_promptstring(synth);

	/* Read more data. */
	while ((str = el_gets(synth->el, &count)) == NULL)
	{
	    /* An interrupted system call (EINTR) caused an error in
	     * el_gets(). */
	}
	cw_assert(count > 0);

	/* Update the command line history. */
	if ((nxo_thread_deferred(synth->thread) == FALSE)
	    && (nxo_thread_state(synth->thread) == THREADTS_START))
	{
	    /* Completion of a history element.  Insert it, taking care to avoid
	     * simple (non-continued) duplicates and empty lines (simple
	     * carriage returns). */
	    if (synth->continuation)
	    {
		history(synth->hist, &synth->hevent, H_ENTER, str);
		synth->continuation = FALSE;
	    }
	    else
	    {
		if ((history(synth->hist, &synth->hevent, H_FIRST) == -1
		     || strcmp(str, synth->hevent.str)) && strlen(str) > 1)
		{
		    history(synth->hist, &synth->hevent, H_ENTER, str);
		}
	    }
	}
	else
	{
	    /* Continuation.  Append it to the current history element. */
	    history(synth->hist, &synth->hevent, H_ADD, str);
	    synth->continuation = TRUE;
	}

	/* Copy the data to the synth buffer. */
	if (count > synth->buffer_size)
	{
	    /* Expand the buffer. */
	    if (synth->buffer == NULL)
	    {
		synth->buffer = (cw_uint8_t *) cw_malloc(count);
	    }
	    else
	    {
		synth->buffer = (cw_uint8_t *) cw_realloc(synth->buffer, count);
	    }
	    synth->buffer_size = count;
	}
	memcpy(synth->buffer, str, count);
	synth->buffer_count = count;
    }
    cw_assert(synth->buffer_count > 0);

    /* Return as much of the data as possible. */
    if (synth->buffer_count > a_len)
    {
	/* There are more data than we can return. */
	retval = a_len;
	memcpy(r_str, synth->buffer, a_len);
	synth->buffer_count -= a_len;
	memmove(synth->buffer, &synth->buffer[a_len], synth->buffer_count);
    }
    else
    {
	/* Return all the data. */
	retval = synth->buffer_count;
	memcpy(r_str, synth->buffer, synth->buffer_count);
	synth->buffer_count = 0;
    }

    return retval;
}
#endif

static void
modprompt_promptstring(struct cw_modprompt_synth_s *a_synth)
{
    /* Call promptstring if the conditions are right.  Take lots of care not to
     * let an error in promptstring cause recursion into the error handling
     * machinery. */
    if ((nxo_thread_deferred(synth->thread) == FALSE)
	&& (nxo_thread_state(synth->thread) == THREADTS_START))
    {
	cw_uint8_t *pstr;
	cw_uint32_t plen, maxlen;
	cw_nxo_t *nxo;
	cw_nxo_t *stack;
	static const cw_uint8_t code[] =
	    "{$promptstring where {\n"
	    "pop\n"
	    /* Save the current contents of errordict into promptdict. */
	    "$promptdict errordict dict copy def\n"
	    /* Temporarily reconfigure errordict. */
	    "errordict $handleerror {} put\n"
	    "errordict $stop $stop load put\n"
	    /* Actually call promptstring. */
	    "{promptstring} stopped {`'} if\n"
	    /* Restore errordict's configuration. */
	    "promptdict errordict copy pop\n"
	    /* Remove the definition of promptdict. */
	    "$promptdict where {$promptdict undef} if\n"
	    "}{`'} ifelse} start";

	stack = nxo_thread_ostack_get(synth->thread);

	/* Push the prompt onto the data stack. */
	nxo_thread_interpret(synth->thread, &synth->threadp, code,
			     sizeof(code) - 1);
	nxo_thread_flush(synth->thread, &synth->threadp);

	/* Get the actual prompt string. */
	nxo = nxo_stack_get(stack);
	if (nxo == NULL)
	{
	    nxo_thread_nerror(synth->thread, NXN_stackunderflow);
	    maxlen = 0;
	}
	else if (nxo_type_get(nxo) != NXOT_STRING)
	{
	    nxo_thread_nerror(synth->thread, NXN_typecheck);
	    maxlen = 0;
	}
	else
	{
	    pstr = nxo_string_get(nxo);
	    plen = nxo_string_len_get(nxo);

	    /* Copy the prompt string to a global buffer. */
	    maxlen = (plen > CW_PROMPT_STRLEN - 1)
		? CW_PROMPT_STRLEN - 1 : plen;
	    strncpy(synth->prompt_str, pstr, maxlen);
	}

	synth->prompt_str[maxlen] = '\0';

	/* Pop the prompt string off the data stack. */
	nxo_stack_pop(stack);
    }
}

static char *
modprompt_prompt(EditLine *a_el)
{
    struct cw_modprompt_synth_s *synth;

    el_get(a_el, EL_CLIENTDATA, (void **)&synth);

    cw_check_ptr(synth);
    cw_dassert(synth->magic == CW_MODPROMPT_SYNTH_MAGIC);

    if ((nxo_thread_deferred(synth->thread))
	|| (nxo_thread_state(synth->thread) != THREADTS_START))
    {
	/* One or both of:
	 *
	 * - Continuation of a string or similarly parsed token.
	 * - Execution is deferred due to unmatched {}'s.
	 *
	 * Don't print a prompt. */
	synth->prompt_str[0] = '\0';
    }

    return synth->prompt_str;
}

static void
modprompt_handlers_install(void)
{
    struct sigaction action;
#ifdef CW_THREADS
    sigset_t set, oset;
#endif

    /* Set up a signal handler for various signals that are important to an
     * interactive program. */
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = modprompt_signal_handle;
    sigemptyset(&action.sa_mask);
#define HANDLER_INSTALL(a_signal)					\
    if (sigaction((a_signal), &action, NULL) == -1)			\
    {									\
	fprintf(stderr, "Error in sigaction(%s, ...): %s\n",		\
		#a_signal, strerror(errno));				\
	abort();							\
    }
    HANDLER_INSTALL(SIGHUP);
    HANDLER_INSTALL(SIGWINCH);
    HANDLER_INSTALL(SIGTSTP);
    HANDLER_INSTALL(SIGCONT);
    HANDLER_INSTALL(SIGINT);
    HANDLER_INSTALL(SIGQUIT);
    HANDLER_INSTALL(SIGTERM);
#undef HANDLER_INSTALL

#ifdef CW_THREADS
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTSTP);
    sigaddset(&set, SIGSTOP);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGCONT);
    sigaddset(&set, SIGWINCH);
    thd_sigmask(SIG_UNBLOCK, &set, &oset);
#endif
}

static void
modprompt_signal_handle(int a_signal)
{
    switch (a_signal)
    {
	case SIGWINCH:
	{
 	    el_resize(synth->el);
	    break;
	}
	case SIGTSTP:
	{
	    /* Signal the process group. */
	    kill(0, SIGSTOP);
	    break;
	}
	case SIGCONT:
	{
	    break;
	}
	case SIGHUP:
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
	{
	    el_end(synth->el);
	    exit(0);
	}
	default:
	{
	    fprintf(stderr, "Unexpected signal %d\n", a_signal);
	    abort();
	}
    }
}

#ifdef CW_THREADS
static void *
modprompt_entry(void *a_arg)
{
    struct cw_modprompt_synth_s *synth = (struct cw_modprompt_synth_s *) a_arg;
    const char *str;
    int count = 0;

    cw_check_ptr(synth);
    cw_dassert(synth->magic == CW_MODPROMPT_SYNTH_MAGIC);

    /* Install signal handlers. */
    modprompt_handlers_install();

    mtx_lock(&synth->mtx);
    for (;;)
    {
	/* Wait for the interpreter thread to request data. */
	while (synth->want_data == FALSE && synth->quit == FALSE)
	{
	    cnd_wait(&synth->put_cnd, &synth->mtx);
	}
	synth->want_data = FALSE;
	if (synth->quit)
	{
	    break;
	}

	/* Update the promptstring if necessary. */
	modprompt_promptstring(synth);

	/* Read data. */
	cw_assert(synth->buffer_count == 0);
	while ((str = el_gets(synth->el, &count)) == NULL)
	{
	    /* An interrupted system call (EINTR) caused an error in el_gets().
	     * Check to see if we should quit. */
	}
	cw_assert(count > 0);

	/* Update the command line history. */
	if ((nxo_thread_deferred(synth->thread) == FALSE)
	    && (nxo_thread_state(synth->thread) == THREADTS_START))
	{
	    /* Completion of a history element.  Insert it, taking care to avoid
	     * simple (non-continued) duplicates and empty lines (simple
	     * carriage returns). */
	    if (synth->continuation)
	    {
		history(synth->hist, &synth->hevent, H_ENTER, str);
		synth->continuation = FALSE;
	    }
	    else
	    {
		if ((history(synth->hist, &synth->hevent, H_FIRST) == -1
		     || strcmp(str, synth->hevent.str)) && strlen(str) > 1)
		{
		    history(synth->hist, &synth->hevent, H_ENTER, str);
		}
	    }
	}
	else
	{
	    /* Continuation.  Append it to the current history element. */
	    history(synth->hist, &synth->hevent, H_ADD, str);
	    synth->continuation = TRUE;
	}

	/* Copy the data to the synth buffer. */
	if (count > synth->buffer_size)
	{
	    /* Expand the buffer. */
	    if (synth->buffer == NULL)
	    {
		synth->buffer = (cw_uint8_t *) cw_malloc(count);
	    }
	    else
	    {
		synth->buffer = (cw_uint8_t *) cw_realloc(synth->buffer, count);
	    }
	    synth->buffer_size = count;
	}
	memcpy(synth->buffer, str, count);
	synth->buffer_count = count;

	/* Tell the interpreter thread that there are data available. */
	synth->have_data = TRUE;
	cnd_signal(&synth->get_cnd);
    }
    mtx_unlock(&synth->mtx);

    return NULL;
}
#endif
