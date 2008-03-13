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

#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#include "onyx.h"

/* Include generated code. */
#include "onyx_nxcode.c"

#define	_PROMPT_STRLEN	  80

struct nx_arg_s {
	cw_bool_t	quit;
	cw_bool_t	want_data;
	cw_bool_t	have_data;

	cw_mtx_t	mtx;
	cw_cnd_t	cl_cnd;
	cw_cnd_t	nx_cnd;

	cw_uint8_t	*buffer;
	cw_uint32_t	buffer_size;
	cw_uint32_t	buffer_count;
};

/*
 * Globals.  These are global due to the libedit API not providing a way to pass
 * them to the prompt function.
 */
cw_nxo_t	thread;
cw_nxo_threadp_t threadp;
EditLine	*el;
History		*hist;
cw_uint8_t	prompt_str[_PROMPT_STRLEN];

int		interactive_run(int argc, char **argv, char **envp);
char		*prompt(EditLine *a_el);
void		cl_read(struct nx_arg_s *a_arg);
void		*nx_entry(void *a_arg);
cw_sint32_t	nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len,
    cw_uint8_t *r_str);
void		signal_handle(int a_signal);
int		batch_run(int argc, char **argv, char **envp);
void		usage(const char *a_progname);
const char	*basename(const char *a_str);

int
main(int argc, char **argv, char **envp)
{
	int			retval;

	libstash_init();

	/*
	 * Run differently, depending on whether this is an interactive session.
	 */
	if (isatty(0) && argc == 1)
		retval = interactive_run(argc, argv, envp);
	else
		retval = batch_run(argc, argv, envp);

	libstash_shutdown();
	return retval;
}

int
interactive_run(int argc, char **argv, char **envp)
{
	/*
	 * Define promptstring in systemdict:
	 *   - promptstring <string>
	 *
	 * Define 'resume' in threaddict to continue after an error.
	 *
	 * Do not stop on error.  Instead, recursively evaluate stdin.  File
	 * bufferring can cause strange behavior, but at least the error will
	 * get handled first.
	 *
	 * Quit on estackoverflow in order to avoid infinite recursion.
	 *
	 * Print the product and version info.
	 *
	 * Push an executable stdin on ostack to prepare for the start operator.
	 */
	static const cw_uint8_t	code[] = "\n\
systemdict begin\n\
/promptstring {\n\
	count cvs `onyx:' exch catenate `> ' catenate\n\
} bind def\n\
end\n\
threaddict begin\n\
/resume //stop def\n\
end\n\
errordict begin\n\
	/stop {\n\
		stdin cvx stopped pop\n\
	} bind def\n\
end\n\
product print `, version ' print version print `.\n' print flush\n\
stdin cvx\n\
";
	struct nx_arg_s	arg;
	struct sigaction action;
	sigset_t	set, oset;
	cw_nx_t		nx;
	cw_nxo_t	*stdin_nxo;
	cw_thd_t	*nx_thd;
	char		*editor;

	/*
	 * Set up a signal handler for various signals that are important to an
	 * interactive program.
	 */
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = signal_handle;
	sigemptyset(&action.sa_mask);
#define	_HANDLER_INSTALL(a_signal)					\
	if (sigaction((a_signal), &action, NULL) == -1) {		\
		_cw_out_put_e("Error in sigaction([s], ...): [s]\n",	\
		    #a_signal, strerror(errno));			\
		abort();						\
	}
	_HANDLER_INSTALL(SIGHUP);
	_HANDLER_INSTALL(SIGWINCH);
	_HANDLER_INSTALL(SIGTSTP);
	_HANDLER_INSTALL(SIGCONT);
	_HANDLER_INSTALL(SIGINT);
	_HANDLER_INSTALL(SIGQUIT);
	_HANDLER_INSTALL(SIGTERM);
#undef	_HANDLER_INSTALL

	/*
	 * Initialize the command editor.
	 */
	hist = history_init();
	history(hist, H_EVENT, 512);

	el = el_init(basename(argv[0]), stdin, stdout);
	el_set(el, EL_HIST, history, hist);
	el_set(el, EL_PROMPT, prompt);

	editor = getenv("ONYX_EDITOR");
	if (editor == NULL || (strcmp(editor, "emacs") && strcmp(editor,
	    "vi"))) {
		/*
		 * Default to emacs key bindings, since they're more intuitive
		 * to the uninitiated.
		 */
		editor = "emacs";
	}
	el_set(el, EL_EDITOR, editor);

	/*
	 * Initialize the structure that is used for communication between this
	 * thread and the interpreter thread.
	 */
	arg.quit = FALSE;
	arg.want_data = FALSE;
	arg.have_data = FALSE;
	mtx_new(&arg.mtx);
	cnd_new(&arg.cl_cnd);
	cnd_new(&arg.nx_cnd);
	arg.buffer = NULL;
	arg.buffer_size = 0;
	arg.buffer_count = 0;

	/* Initialize the interpreter. */
	nx_new(&nx, NULL, argc, argv, envp);

	/* Set up the interactive wrapper for stdin. */
	stdin_nxo = nx_stdin_get(&nx);
	nxo_file_new(stdin_nxo, &nx, TRUE);
	nxo_file_interactive(stdin_nxo, nx_read, NULL, (void *)&arg);

	/* Create the initial thread. */
	nxo_thread_new(&thread, &nx);
	nxo_threadp_new(&threadp);

	/*
	 * Install custom operators and run embedded initialization code.
	 */
	onyx_ops_init(&thread);
	onyx_nxcode(&thread);

	/*
	 * Run embedded initialization code.
	 */
	nxo_thread_interpret(&thread, &threadp, code, sizeof(code) - 1);
	nxo_thread_flush(&thread, &threadp);

	/*
	 * Acquire the interlock mtx before creating the interpreter thread, so
	 * that we won't miss any cnd_signal()s from it.
	 */
	mtx_lock(&arg.mtx);

	/*
	 * Block signals that the main thread has handlers for while creating
	 * the thread that will run the interpreter, so that we'll be the only
	 * thread to receive signals.
	 */
	sigemptyset(&set);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGWINCH);
	sigaddset(&set, SIGTSTP);
	sigaddset(&set, SIGCONT);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGTERM);
	thd_sigmask(SIG_BLOCK, &set, &oset);
	nx_thd = thd_new(nx_entry, &arg, TRUE);
	thd_sigmask(SIG_SETMASK, &oset, NULL);

	/* Handle read requests from the interpreter thread. */
	cl_read(&arg);

	/*
	 * Release the interlock mtx, join the interpreter thd, then clean up.
	 */
	mtx_unlock(&arg.mtx);
	thd_join(nx_thd);

	mtx_delete(&arg.mtx);
	cnd_delete(&arg.cl_cnd);
	cnd_delete(&arg.nx_cnd);
	if (arg.buffer != NULL)
		_cw_free(arg.buffer);

	/* Clean up the command editor. */
	el_end(el);
	history_end(hist);

	nxo_threadp_delete(&threadp, &thread);
	nx_delete(&nx);
	return 0;
}

char *
prompt(EditLine *a_el)
{
	if ((nxo_thread_deferred(&thread) == FALSE) &&
	    (nxo_thread_state(&thread) == THREADTS_START)) {
		static const cw_uint8_t	code[] = "promptstring";
		cw_uint8_t		*pstr;
		cw_uint32_t		plen, maxlen;
		cw_nxo_t		*nxo;
		cw_nxo_t		*stack;

		stack = nxo_thread_ostack_get(&thread);

		/* Push the prompt onto the data stack. */
		nxo_thread_interpret(&thread, &threadp, code, sizeof(code) - 1);
		nxo_thread_flush(&thread, &threadp);

		/* Get the actual prompt string. */
		nxo = nxo_stack_get(stack);
		if (nxo == NULL) {
			nxo_thread_error(&thread, NXO_THREADE_STACKUNDERFLOW);
			maxlen = 0;
		} else if (nxo_type_get(nxo) != NXOT_STRING) {
			nxo_thread_error(&thread, NXO_THREADE_TYPECHECK);
			maxlen = 0;
		} else {
			pstr = nxo_string_get(nxo);
			plen = nxo_string_len_get(nxo);

			/* Copy the prompt string to a global buffer. */
			maxlen = (plen > _PROMPT_STRLEN - 1) ? _PROMPT_STRLEN -
			    1 : plen;
			strncpy(prompt_str, pstr, _PROMPT_STRLEN - 1);
		}

		prompt_str[maxlen] = '\0';

		/* Pop the prompt string off the data stack. */
		nxo_stack_pop(stack);
	} else {
		/*
		 * One or both of:
		 *
		 * - Continuation of a string or similarly parsed token.
		 * - Execution is deferred due to unmatched {}'s.
		 *
		 * Don't print a prompt.
		 */
		prompt_str[0] = '\0';
	}

	return prompt_str;
}

void
cl_read(struct nx_arg_s *a_arg)
{
	const char		*str;
	int			count = 0;
	static cw_bool_t	continuation = FALSE;
	struct nx_arg_s		*arg = (struct nx_arg_s *)a_arg;

	for (;;) {
		/* Wait for the interpreter thread to request data. */
		a_arg->want_data = FALSE;
		while (a_arg->want_data == FALSE && arg->quit == FALSE)
			cnd_wait(&a_arg->cl_cnd, &a_arg->mtx);
		if (a_arg->quit)
			break;

		/*
		 * Read data.
		 */
		_cw_assert(arg->buffer_count == 0);
		while ((str = el_gets(el, &count)) == NULL) {
			/*
			 * An interrupted system call (EINTR) caused an error in
			 * el_gets().
			 */
		}
		_cw_assert(count > 0);

		/*
		 * Update the command line history.
		 */
		if ((nxo_thread_deferred(&thread) == FALSE) &&
		    (nxo_thread_state(&thread) == THREADTS_START)) {
			const HistEvent	*hevent;

			/*
			 * Completion of a history element.  Insert it, taking
			 * care to avoid simple (non-continued) duplicates and
			 * empty lines (simple carriage returns).
			 */
			if (continuation) {
				history(hist, H_ENTER, str);
				continuation = FALSE;
			} else {
				hevent = history(hist, H_FIRST);
				if ((hevent == NULL || strcmp(str,
				    hevent->str)) && strlen(str) > 1)
					history(hist, H_ENTER, str);
			}
		} else {
			/*
			 * Continuation.  Append it to the current history
			 * element.
			 */
			history(hist, H_ADD, str);
			continuation = TRUE;
		}

		/*
		 * Copy the data to the arg buffer.
		 */
		if (count > arg->buffer_size) {
			/* Expand the buffer. */
			if (arg->buffer == NULL)
				arg->buffer = (cw_uint8_t *)_cw_malloc(count);
			else
				arg->buffer = (cw_uint8_t
				    *)_cw_realloc(arg->buffer, count);
			arg->buffer_size = count;
		}
		memcpy(arg->buffer, str, count);
		arg->buffer_count = count;

		/* Tell the interpreter thread that there are data available. */
		a_arg->have_data = TRUE;
		cnd_signal(&a_arg->nx_cnd);
	}
}

/*
 * This thread is started with all signals masked so that we don't have to worry
 * about signals from here on.
 */
void *
nx_entry(void *a_arg)
{
	struct nx_arg_s	*arg = (struct nx_arg_s *)a_arg;

	/* Run the interpreter such that it will not exit on errors. */
	nxo_thread_start(&thread);

	/* Tell the main thread to start shutting down. */
	mtx_lock(&arg->mtx);
	arg->quit = TRUE;
	cnd_signal(&arg->cl_cnd);
	mtx_unlock(&arg->mtx);

	return NULL;
}

cw_sint32_t
nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	struct nx_arg_s		*arg = (struct nx_arg_s *)a_arg;

	_cw_assert(a_len > 0);

	mtx_lock(&arg->mtx);

	if (arg->buffer_count == 0) {
		/*
		 * Tell the main thread to read more data, then wait for it.
		 */
		arg->want_data = TRUE;
		cnd_signal(&arg->cl_cnd);
		arg->have_data = FALSE;
		while (arg->have_data == FALSE)
			cnd_wait(&arg->nx_cnd, &arg->mtx);
	}
	_cw_assert(arg->buffer_count > 0);

	/*
	 * Return as much of the data as possible.
	 */
	if (arg->buffer_count > a_len) {
		/* There are more data than we can return. */
		retval = a_len;
		memcpy(r_str, arg->buffer, a_len);
		arg->buffer_count -= a_len;
		memmove(arg->buffer, &arg->buffer[a_len], arg->buffer_count);
	} else {
		/* Return all the data. */
		retval = arg->buffer_count;
		memcpy(r_str, arg->buffer, arg->buffer_count);
		arg->buffer_count = 0;
	}

	mtx_unlock(&arg->mtx);
	return retval;
}

void
signal_handle(int a_signal)
{
	switch (a_signal) {
	case SIGWINCH:
		/* XXX Doesn't return until cl_read() returns. */
		el_resize(el);
		break;
	case SIGTSTP:
		raise(SIGSTOP);
		break;
	case SIGCONT:
		break;
	case SIGHUP:
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
		exit(0);
	default:
		_cw_out_put_e("Unexpected signal [i]\n", a_signal);
		abort();
	}
}

int
batch_run(int argc, char **argv, char **envp)
{
	/*
	 * Die with an exit code of 1 on error.
	 */
	static const cw_uint8_t	code[] = "\n\
currenterror begin\n\
	/stop {\n\
		1 die\n\
	} def\n\
end\n\
";
	int		retval;
	cw_nxo_t	*file;
	int		c;
	cw_bool_t	opt_version = FALSE;
	cw_uint8_t	*opt_expression = NULL;
	cw_nx_t		nx;

	/*
	 * Parse command line arguments, but postpone taking any actions
	 * that require the onyx interpreter to be up and running.  This
	 * is necessary because we need to know how much of argv to pass
	 * to nx_new().
	 */
	c = getopt(argc, argv, "hVe:");
	switch (c) {
	case 'h':
		usage(basename(argv[0]));
		retval = 0;
		goto CLERROR;
	case 'V':
		opt_version = TRUE;
		break;
	case 'e': {
		if (argc != 3) {
			out_put(out_err, "[s]: Incorrect number of "
			    "arguments\n", basename(argv[0]));
			usage(basename(argv[0]));
			retval = 1;
			goto CLERROR;
		}

		opt_expression = optarg;
		break;
	}
	case -1:
		break;
	default:
		out_put(out_err, "[s]: Unrecognized option\n",
		    basename(argv[0]));
		usage(basename(argv[0]));
		retval = 1;
		goto CLERROR;
	}

	/*
	 * Do additional command line argument error checking.
	 */
	if ((optind < argc && (opt_expression != NULL || opt_version))
	    || (opt_version && opt_expression != NULL)) {
		out_put(out_err, "[s]: Incorrect number of arguments\n",
		    basename(argv[0]));
		usage(basename(argv[0]));
		retval = 1;
		goto CLERROR;
	}

	/*
	 * Since this is a non-interactive invocation, don't include all
	 * elements of argv, and wrap stdin.
	 */
	nx_new(&nx, NULL, argc - optind, &argv[optind], envp);
	nxo_thread_new(&thread, &nx);
	nxo_threadp_new(&threadp);

	/*
	 * Install custom operators and run embedded initialization code.
	 */
	onyx_ops_init(&thread);
	onyx_nxcode(&thread);

	/*
	 * Run embedded initialization code specific to non-interactive
	 * execution.
	 */
	nxo_thread_interpret(&thread, &threadp, code, sizeof(code) - 1);
	nxo_thread_flush(&thread, &threadp);

	/*
	 * Act on the command line arguments, now that the interpreter is
	 * initialized.
	 */
	if (opt_version) {
		static const cw_uint8_t	version[] =
		    "product print `, version ' print version print"
		    " `.\n' print flush";

		/*
		 * Print the version and exit.
		 */
		nxo_thread_interpret(&thread, &threadp, version,
		    sizeof(version) - 1);
		nxo_thread_flush(&thread, &threadp);
		retval = 0;
		goto RETURN;
	} else if (opt_expression != NULL) {
		cw_nxo_t	*string;
		cw_uint8_t	*str;

		/*
		 * Push the string onto the execution stack.
		 */
		string = nxo_stack_push(nxo_thread_ostack_get(&thread));
		nxo_string_new(string, &nx, FALSE,
		    strlen(opt_expression));
		nxo_attr_set(string, NXOA_EXECUTABLE);
		str = nxo_string_get(string);
		memcpy(str, opt_expression, nxo_string_len_get(string));
	} else if (optind < argc) {
		int	src_fd;

		/*
		 * Remaining command line arguments should be the name of a
		 * source file, followed by optional arguments.  Open the source
		 * file, wrap it in an onyx file object, and push it onto the
		 * execution stack.
		 */
		src_fd = open(argv[optind], O_RDONLY);
		if (src_fd == -1) {
			out_put(out_err, "[s]: Error in open(\"[s]\","
			    " O_RDONLY): [s]\n", basename(argv[0]),
			    argv[optind], strerror(errno));
			retval = 1;
			goto RETURN;
		}
		file = nxo_stack_push(nxo_thread_ostack_get(&thread));
		nxo_file_new(file, &nx, FALSE);
		nxo_attr_set(file, NXOA_EXECUTABLE);
		nxo_file_fd_wrap(file, src_fd);
	} else if (argc == 1) {
		/*
		 * No source file specified, and there there was no -e
		 * expression specified either, so treat stdin as the
		 * source.
		 *
		 * In other words, there were no arguments specified,
		 * and this isn't a tty.
		 */
		file = nxo_stack_push(nxo_thread_ostack_get(&thread));
		nxo_dup(file, nx_stdin_get(&nx));
		nxo_attr_set(file, NXOA_EXECUTABLE);
	} else
		_cw_not_reached();

	/* Run the interpreter non-interactively. */
	nxo_thread_start(&thread);

	retval = 0;
	RETURN:
	nxo_threadp_delete(&threadp, &thread);
	nx_delete(&nx);
	CLERROR:
	return retval;
}

void
usage(const char *a_progname)
{
	_cw_out_put("[s] usage:\n"
	    "    [s]\n"
	    "    [s] -h\n"
	    "    [s] -V\n"
	    "    [s] -e <expr>\n"
	    "    [s] <file> [[<args>]\n"
	    "\n"
	    "    Option    | Description\n"
	    "    ----------+------------------------------------\n"
	    "    -h        | Print usage and exit.\n"
	    "    -V        | Print version information and exit.\n"
	    "    -e <expr> | Execute <expr> as onyx code.\n",
	    a_progname, a_progname, a_progname, a_progname, a_progname,
	    a_progname);
}

/* Doesn't strip trailing '/' characters. */
const char *
basename(const char *a_str)
{
	const char	*retval = NULL;
	cw_uint32_t	i;

	_cw_check_ptr(a_str);

	i = strlen(a_str);
	if (i > 0) {
		for (i--; i > 0; i--) {
			if (a_str[i] == '/') {
				retval = &a_str[i + 1];
				break;
			}
		}
	}
	if (retval == NULL)
		retval = a_str;
	return retval;
}
