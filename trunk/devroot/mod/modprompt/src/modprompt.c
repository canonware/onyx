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
    cw_uint8_t prompt_str[CW_PROMPT_STRLEN];
};

/* This has to be global due to the libedit API not providing a way to pass data
 * to the prompt function. */
static struct cw_modprompt_synth_s g_synth;

/* Function prototypes. */
static cw_nxoe_t *
modprompt_synth_ref_iter(void *a_data, cw_bool_t a_reset);

static void
modprompt_synth_delete(void *a_data, cw_nx_t *a_nx);

static cw_sint32_t
modprompt_read(void *a_data, cw_nxo_t *a_file, cw_uint32_t a_len,
	       cw_uint8_t *r_str);

static char *
modprompt_prompt(EditLine *a_el);

#ifdef CW_THREADS
static void *
modprompt_entry(void *a_arg);
#endif

void
modprompt_init(void *a_arg, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *file;
    struct cw_modprompt_synth_s *synth = &g_synth;
    char *editor;
#ifdef CW_THREADS
    sigset_t set, oset;
#endif

    /* Initialize stdin. */
    file = nxo_thread_stdin_get(a_thread);
    nxo_file_new(file, nxo_thread_nx_get(a_thread), TRUE);
    nxo_file_synthetic(file, modprompt_read, NULL, modprompt_synth_ref_iter,
		       modprompt_synth_delete, synth);
    nxo_attr_set(file, NXOA_EXECUTABLE);

    /* Initialize synth. */
#ifdef CW_DBG
    memset(synth, 0xa5, sizeof(struct cw_modprompt_synth_s));
#endif

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
    synth->continuation = FALSE;
#ifdef CW_THREADS
    synth->quit = FALSE;
    synth->resize = FALSE;
    mtx_new(&synth->mtx);
    synth->have_data = FALSE;
    cnd_new(&synth->put_cnd);
    synth->want_data = FALSE;
    cnd_new(&synth->get_cnd);
#endif
    synth->buffer = NULL;
    synth->buffer_size = 0;
    synth->buffer_count = 0;

    /* Initialize the command editor. */
    synth->hist = history_init();
    history(synth->hist, H_EVENT, 512);

    synth->el = el_init("onyx", stdin, stdout);
    el_set(synth->el, EL_HIST, history, synth->hist);
    el_set(synth->el, EL_PROMPT, modprompt_prompt);

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
modprompt_synth_delete(void *a_data, cw_nx_t *a_nx)
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

#ifdef CW_DBG
    memset(synth, 0x5a, sizeof(struct cw_modprompt_synth_s));
#endif
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
	    const HistEvent *hevent;

	    /* Completion of a history element.  Insert it, taking care to avoid
	     * simple (non-continued) duplicates and empty lines (simple
	     * carriage returns). */
	    if (synth->continuation)
	    {
		history(synth->hist, H_ENTER, str);
		synth->continuation = FALSE;
	    }
	    else
	    {
		hevent = history(synth->hist, H_FIRST);
		if ((hevent == NULL
		     || strcmp(str, hevent->str)) && strlen(str) > 1)
		{
		    history(synth->hist, H_ENTER, str);
		}
	    }
	}
	else
	{
	    /* Continuation.  Append it to the current history element. */
	    history(synth->hist, H_ADD, str);
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

static char *
modprompt_prompt(EditLine *a_el)
{
    struct cw_modprompt_synth_s *synth
	= (struct cw_modprompt_synth_s *) &g_synth;

    cw_check_ptr(synth);
    cw_dassert(synth->magic == CW_MODPROMPT_SYNTH_MAGIC);

    if ((nxo_thread_deferred(synth->thread) == FALSE)
	&& (nxo_thread_state(synth->thread) == THREADTS_START))
    {
	static const cw_uint8_t code[] = "promptstring";
	cw_uint8_t *pstr;
	cw_uint32_t plen, maxlen;
	cw_nxo_t *nxo;
	cw_nxo_t *stack;

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
	    maxlen
		= (plen > CW_PROMPT_STRLEN - 1) ? CW_PROMPT_STRLEN - 1 : plen;
	    strncpy(synth->prompt_str, pstr, maxlen);
	}

	synth->prompt_str[maxlen] = '\0';

	/* Pop the prompt string off the data stack. */
	nxo_stack_pop(stack);
    }
    else
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

#ifdef CW_THREADS
static void *
modprompt_entry(void *a_arg)
{
    struct cw_modprompt_synth_s *synth = (struct cw_modprompt_synth_s *) a_arg;
    const char *str;
    int count = 0;

    cw_check_ptr(synth);
    cw_dassert(synth->magic == CW_MODPROMPT_SYNTH_MAGIC);

    /* Set up signal handlers. */
#ifdef NOT_YET /* XXX This makes things break.  Try again with newer libedit. */
    el_set(synth->el, EL_SIGNAL, 1);
#endif

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

	/* Read data. */
	cw_assert(synth->buffer_count == 0);
	while ((str = el_gets(synth->el, &count)) == NULL)
	{
	    /* An interrupted system call (EINTR) caused an error in
	     * el_gets().  Check to see if we should quit. */
	}
	cw_assert(count > 0);

	/* Update the command line history. */
	if ((nxo_thread_deferred(synth->thread) == FALSE)
	    && (nxo_thread_state(synth->thread) == THREADTS_START))
	{
	    const HistEvent *hevent;

	    /* Completion of a history element.  Insert it, taking care to avoid
	     * simple (non-continued) duplicates and empty lines (simple
	     * carriage returns). */
	    if (synth->continuation)
	    {
		history(synth->hist, H_ENTER, str);
		synth->continuation = FALSE;
	    }
	    else
	    {
		hevent = history(synth->hist, H_FIRST);
		if ((hevent == NULL
		     || strcmp(str, hevent->str)) && strlen(str) > 1)
		{
		    history(synth->hist, H_ENTER, str);
		}
	    }
	}
	else
	{
	    /* Continuation.  Append it to the current history element. */
	    history(synth->hist, H_ADD, str);
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
