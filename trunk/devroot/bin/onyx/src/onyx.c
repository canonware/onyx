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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#include "onyx.h"

/* Include generated code. */
#ifdef _CW_POSIX
#include "onyx_nxcode.c"
#endif
#include "batch_nxcode.c"
#include "interactive_nxcode.c"

#define	_PROMPT_STRLEN	 80
#define	_BUFFER_SIZE	512

struct nx_read_arg_s {
#if (defined(_CW_USE_LIBEDIT) && defined(_CW_THREADS))
	cw_bool_t	quit;
	cw_bool_t	want_data;
	cw_bool_t	have_data;

	cw_mtx_t	mtx;
	cw_cnd_t	cl_cnd;
	cw_cnd_t	nx_cnd;
#else
	cw_bool_t	interactive;
	cw_nxo_t	*thread;
#endif
	cw_uint8_t	*buffer;
	cw_uint32_t	buffer_size;
	cw_uint32_t	buffer_count;
};

#ifndef _CW_POSIX_FILE
struct nx_write_arg_s {
	int	fd;
};
#endif

/*
 * Globals.  These are global due to the libedit API not providing a way to pass
 * them to the prompt function.
 */
cw_nxo_t	thread;
cw_nxo_threadp_t threadp;
#ifdef _CW_USE_LIBEDIT
EditLine	*el;
History		*hist;
cw_uint8_t	prompt_str[_PROMPT_STRLEN];
#endif

/*
 * Function prototypes.
 */
void		usage(const char *a_progname);
const char	*basename(const char *a_str);

struct nx_read_arg_s *stdin_init(cw_nx_t *a_nx, cw_nxo_t *a_thread, cw_bool_t
    a_interactive);
void		stdin_shutdown(void *a_arg, cw_nx_t *a_nx);
cw_sint32_t	nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len,
    cw_uint8_t *r_str);

int		interactive_run(int argc, char **argv, char **envp);
int		batch_run(int argc, char **argv, char **envp);

#ifdef _CW_USE_LIBEDIT
char		*prompt(EditLine *a_el);
void		signal_handle(int a_signal);
#endif

#if (defined(_CW_USE_LIBEDIT) && defined(_CW_THREADS))
void		cl_read(struct nx_read_arg_s *a_arg);
void		*nx_entry(void *a_arg);
#endif

#ifndef _CW_POSIX_FILE
void		stdout_init(cw_nx_t *a_nx);
void		stderr_init(cw_nx_t *a_nx);
cw_bool_t	nx_write(void *a_arg, cw_nxo_t *a_file, const cw_uint8_t *a_str,
    cw_uint32_t a_len);
#endif

int
main(int argc, char **argv, char **envp)
{
	int			retval;

	libonyx_init();

	/*
	 * Run differently, depending on whether this is an interactive session.
	 */
	if (isatty(0) && argc == 1)
		retval = interactive_run(argc, argv, envp);
	else
		retval = batch_run(argc, argv, envp);

	libonyx_shutdown();
	return retval;
}

void
usage(const char *a_progname)
{
	printf("%s usage:\n"
	    "    %s\n"
	    "    %s -h\n"
	    "    %s -V\n"
	    "    %s -e <expr>\n"
#ifdef _CW_POSIX_FILE
	    "    %s <file> [<args>]\n"
#endif
	    "\n"
	    "    Option    | Description\n"
	    "    ----------+------------------------------------\n"
	    "    -h        | Print usage and exit.\n"
	    "    -V        | Print version information and exit.\n"
	    "    -e <expr> | Execute <expr> as Onyx code.\n",
	    a_progname, a_progname, a_progname, a_progname, a_progname
#ifdef _CW_POSIX_FILE
	    , a_progname
#endif
	    );
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

#if (defined(_CW_USE_LIBEDIT) && defined(_CW_THREADS))
struct nx_read_arg_s *
stdin_init(cw_nx_t *a_nx, cw_nxo_t *a_thread, cw_bool_t a_interactive)
{
	cw_nxo_t			*nxo;
	static struct nx_read_arg_s	stdin_arg;

	/*
	 * Initialize the structure that is used for communication between this
	 * thread and the interpreter thread.
	 */
	stdin_arg.quit = FALSE;
	stdin_arg.want_data = FALSE;
	stdin_arg.have_data = FALSE;
	mtx_new(&stdin_arg.mtx);
	cnd_new(&stdin_arg.cl_cnd);
	cnd_new(&stdin_arg.nx_cnd);
	stdin_arg.buffer = NULL;
	stdin_arg.buffer_size = 0;
	stdin_arg.buffer_count = 0;

	/* Set up the interactive wrapper for stdin. */
	nxo = nx_stdin_get(a_nx);
	nxo_file_new(nxo, a_nx, TRUE);
	nxo_file_synthetic(nxo, nx_read, NULL, NULL, stdin_shutdown, (void
	    *)&stdin_arg);

	return &stdin_arg;
}
#else
struct nx_read_arg_s *
stdin_init(cw_nx_t *a_nx, cw_nxo_t *a_thread, cw_bool_t a_interactive)
{
	cw_nxo_t			*nxo;
	static struct nx_read_arg_s	stdin_arg;

	/* Initialize the stdin argument structure. */
	stdin_arg.interactive = a_interactive;
	stdin_arg.thread = a_thread;
	stdin_arg.buffer = NULL;
	stdin_arg.buffer_size = 0;
	stdin_arg.buffer_count = 0;

	nxo = nx_stdin_get(a_nx);
	nxo_file_new(nxo, a_nx, TRUE);
	nxo_file_synthetic(nxo, nx_read, NULL, NULL, stdin_shutdown, (void
	    *)&stdin_arg);

	return &stdin_arg;
}
#endif

void
stdin_shutdown(void *a_arg, cw_nx_t *a_nx)
{
	struct nx_read_arg_s *arg = (struct nx_read_arg_s *)a_arg;

	if (arg->buffer != NULL)
		_cw_free(arg->buffer);
}

#if (defined(_CW_USE_LIBEDIT) && defined(_CW_THREADS))
cw_sint32_t
nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	struct nx_read_arg_s	*arg = (struct nx_read_arg_s *)a_arg;

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
#elif (defined(_CW_USE_LIBEDIT))
cw_sint32_t
nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	struct nx_read_arg_s	*arg = (struct nx_read_arg_s *)a_arg;
	const char		*str;
	int			count = 0;
	static cw_bool_t	continuation = FALSE;

	_cw_assert(a_len > 0);

	if (arg->buffer_count == 0) {
		/*
		 * Read more data.
		 */
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

	return retval;
}
#else
cw_sint32_t
nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	struct nx_read_arg_s	*arg = (struct nx_read_arg_s *)a_arg;

	_cw_assert(a_len > 0);

	if (arg->buffer_count == 0) {
		/*
		 * Print the prompt if interactive and not in deferred execution
		 * mode.
		 */
		if ((arg->interactive) && (nxo_thread_deferred(arg->thread) ==
		    FALSE) && (nxo_thread_state(arg->thread) ==
		    THREADTS_START))
			_cw_onyx_code(arg->thread, "promptstring print flush");

		/*
		 * Read data until there are no more.
		 */
		while ((retval = read(0, r_str, a_len)) == -1 && errno ==
		    EINTR) {
			/*
			 * Interrupted system call, probably due to garbage
			 * collection.
			 */
		}
		if (retval == -1) {
			/* EOF. */
			retval = 0;
		} else {
			if (retval == a_len) {
				cw_sint32_t	count;

				/*
				 * There may be more data available.  Store any
				 * available data in a buffer.
				 */
				if (arg->buffer == NULL) {
					/* Initialize the buffer. */
					arg->buffer = (cw_uint8_t
					    *)_cw_malloc(_BUFFER_SIZE);
					arg->buffer_size = _BUFFER_SIZE;
					arg->buffer_count = 0;
				}

				for (;;) {
					while ((count = read(0,
					    &arg->buffer[arg->buffer_count],
					    arg->buffer_size -
					    arg->buffer_count)) == -1 && errno
					    == EINTR) {
						/*
						 * Interrupted system call,
						 * probably due to garbage
						 * collection.
						 */
					}
					if (count <= 0) {
						/*
						 * EOF or no more data buffered
						 * by stdin.
						 */
						break;
					}

					arg->buffer_count += count;
					if (arg->buffer_count ==
					    arg->buffer_size) {
						/* Expand the buffer. */
						arg->buffer = (cw_uint8_t
						    *)_cw_realloc(arg->buffer,
						    arg->buffer_size * 2);
						arg->buffer_size *= 2;
					}
				}
			}
		}
	} else {
		/*
		 * Return as much of the buffered data as possible.
		 */
		if (arg->buffer_count > a_len) {
			/* There are more data than we can return. */
			retval = a_len;
			memcpy(r_str, arg->buffer, a_len);
			arg->buffer_count -= a_len;
			memmove(arg->buffer, &arg->buffer[a_len],
			    arg->buffer_count);
		} else {
			/* Return all the data. */
			retval = arg->buffer_count;
			memcpy(r_str, arg->buffer, arg->buffer_count);
			arg->buffer_count = 0;
		}
	}

	return retval;
}
#endif

int
interactive_run(int argc, char **argv, char **envp)
{
	cw_nx_t			nx;
	struct nx_read_arg_s	*stdin_arg;
#ifdef _CW_USE_LIBEDIT
	struct sigaction	action;
	char			*editor;
#endif
#ifdef _CW_THREADS
	sigset_t		set, oset;
	cw_thd_t		*nx_thd;
#endif

#ifdef _CW_USE_LIBEDIT
	/*
	 * Set up a signal handler for various signals that are important to an
	 * interactive program.
	 */
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = signal_handle;
	sigemptyset(&action.sa_mask);
#define	_HANDLER_INSTALL(a_signal)					\
	if (sigaction((a_signal), &action, NULL) == -1) {		\
		fprintf(stderr, "Error in sigaction(%s, ...): %s\n",	\
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
#endif

	/* Initialize the interpreter. */
	nx_new(&nx, NULL, argc, argv, envp);

	stdin_arg = stdin_init(&nx, &thread, TRUE);
#ifndef _CW_POSIX_FILE
	stdout_init(&nx);
	stderr_init(&nx);
#endif

	/* Now that the files have been wrapped, create the initial thread. */
	nxo_thread_new(&thread, &nx);
	nxo_threadp_new(&threadp);

	/* Install custom operators. */
#ifdef _CW_POSIX
	onyx_ops_init(&thread);
#endif

	/* Run embedded initialization code. */
#ifdef _CW_POSIX
	onyx_nxcode(&thread);
#endif
	interactive_nxcode(&thread);

#if (defined(_CW_USE_LIBEDIT) && defined(_CW_THREADS))
	/*
	 * Acquire the interlock mtx before creating the interpreter thread, so
	 * that we won't miss any cnd_signal()s from it.
	 */
	mtx_lock(&stdin_arg->mtx);

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
	nx_thd = thd_new(nx_entry, stdin_arg, TRUE);
	thd_sigmask(SIG_SETMASK, &oset, NULL);

	/* Handle read requests from the interpreter thread. */
	cl_read(stdin_arg);

	/*
	 * Release the interlock mtx, join the interpreter thd, then clean up.
	 */
	mtx_unlock(&stdin_arg->mtx);
	thd_join(nx_thd);

	mtx_delete(&stdin_arg->mtx);
	cnd_delete(&stdin_arg->cl_cnd);
	cnd_delete(&stdin_arg->nx_cnd);
#else
	/* Run the interpreter such that it will not exit on errors. */
	nxo_thread_start(&thread);
#endif

#ifdef _CW_USE_LIBEDIT
	/* Clean up the command editor. */
	el_end(el);
	history_end(hist);
#endif

	nxo_threadp_delete(&threadp, &thread);
	nx_delete(&nx);
	return 0;
}

int
batch_run(int argc, char **argv, char **envp)
{
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
			fprintf(stderr, "%s: Incorrect number of arguments\n",
			    basename(argv[0]));
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
		fprintf(stderr,  "%s: Unrecognized option\n",
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
		fprintf(stderr, "%s: Incorrect number of arguments\n",
		    basename(argv[0]));
		usage(basename(argv[0]));
		retval = 1;
		goto CLERROR;
	}

	/*
	 * Since this is a non-interactive invocation, don't include all
	 * elements of argv.
	 */
	nx_new(&nx, NULL, argc - optind, &argv[optind], envp);
#ifndef _CW_POSIX_FILE
	stdin_init(&nx, &thread, FALSE);
	stdout_init(&nx);
	stderr_init(&nx);
#endif
	nxo_thread_new(&thread, &nx);
	nxo_threadp_new(&threadp);

	/*
	 * Install custom operators and run embedded initialization code.
	 */
#ifdef _CW_POSIX
	onyx_ops_init(&thread);
	onyx_nxcode(&thread);
#endif

	/*
	 * Run embedded initialization code specific to non-interactive
	 * execution.
	 */
	batch_nxcode(&thread);

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
	}
#ifdef _CW_POSIX_FILE
	else if (optind < argc) {
		int	src_fd;

		/*
		 * Remaining command line arguments should be the name of a
		 * source file, followed by optional arguments.  Open the source
		 * file, wrap it in an onyx file object, and push it onto the
		 * execution stack.
		 */
		src_fd = open(argv[optind], O_RDONLY);
		if (src_fd == -1) {
			fprintf(stderr, "%s: Error in open(\"%s\","
			    " O_RDONLY): %s\n", basename(argv[0]),
			    argv[optind], strerror(errno));
			retval = 1;
			goto RETURN;
		}
		file = nxo_stack_push(nxo_thread_ostack_get(&thread));
		nxo_file_new(file, &nx, FALSE);
		nxo_attr_set(file, NXOA_EXECUTABLE);

		nxo_file_fd_wrap(file, src_fd);
	}
#endif
	else {
		/*
		 * No source file specified (or not supported), and there there
		 * was no -e expression specified either, so treat stdin as the
		 * source.
		 */
		file = nxo_stack_push(nxo_thread_ostack_get(&thread));
		nxo_dup(file, nx_stdin_get(&nx));
		nxo_attr_set(file, NXOA_EXECUTABLE);
	}

	/* Run the interpreter non-interactively. */
	nxo_thread_start(&thread);

	retval = 0;
	RETURN:
	nxo_threadp_delete(&threadp, &thread);
	nx_delete(&nx);
	CLERROR:
	return retval;
}

#ifdef _CW_USE_LIBEDIT
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
			nxo_thread_nerror(&thread, NXN_stackunderflow);
			maxlen = 0;
		} else if (nxo_type_get(nxo) != NXOT_STRING) {
			nxo_thread_nerror(&thread, NXN_typecheck);
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
signal_handle(int a_signal)
{
	switch (a_signal) {
	case SIGWINCH:
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
		fprintf(stderr, "Unexpected signal %d\n", a_signal);
		abort();
	}
}
#endif

#if (defined(_CW_USE_LIBEDIT) && defined(_CW_THREADS))
void
cl_read(struct nx_read_arg_s *a_arg)
{
	const char		*str;
	int			count = 0;
	static cw_bool_t	continuation = FALSE;
	struct nx_read_arg_s	*arg = (struct nx_read_arg_s *)a_arg;

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
	struct nx_read_arg_s	*arg = (struct nx_read_arg_s *)a_arg;

	/* Run the interpreter such that it will not exit on errors. */
	nxo_thread_start(&thread);

	/* Tell the main thread to start shutting down. */
	mtx_lock(&arg->mtx);
	arg->quit = TRUE;
	cnd_signal(&arg->cl_cnd);
	mtx_unlock(&arg->mtx);

	return NULL;
}
#endif

#ifndef _CW_POSIX_FILE
void
stdout_init(cw_nx_t *a_nx)
{
	cw_nxo_t			*nxo;
	static struct nx_write_arg_s	stdout_arg;

	/* Initialize the stdout argument structure. */
	stdout_arg.fd = 1;

	nxo = nx_stdout_get(a_nx);
	nxo_file_new(nxo, a_nx, TRUE);
	nxo_file_synthetic(nxo, NULL, nx_write, NULL, NULL, (void
	    *)&stdout_arg);
}

void
stderr_init(cw_nx_t *a_nx)
{
	cw_nxo_t			*nxo;
	static struct nx_write_arg_s	stderr_arg;

	/* Initialize the stderr argument structure. */
	stderr_arg.fd = 2;

	nxo = nx_stderr_get(a_nx);
	nxo_file_new(nxo, a_nx, TRUE);
	nxo_file_synthetic(nxo, NULL, nx_write, NULL, NULL, (void
	    *)&stderr_arg);
}

cw_bool_t
nx_write(void *a_arg, cw_nxo_t *a_file, const cw_uint8_t *a_str, cw_uint32_t
    a_len)
{
	cw_bool_t		retval;
	struct nx_write_arg_s	*arg = (struct nx_write_arg_s *)a_arg;
	ssize_t			nwritten;

	while (((nwritten = write(arg->fd, a_str, a_len)) == -1) && errno ==
	    EINTR) {
		/*
		 * Interrupted system call, likely due to garbage collection.
		 */
	}
	if (nwritten != a_len) {
		_cw_assert(nwritten == -1);
		retval = TRUE;
		goto RETURN;
	}

	retval = FALSE;
	RETURN:
	return retval;
}
#endif
