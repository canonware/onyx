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

#include <libstil/libstil.h>
#include <libedit/libedit.h>
#include <libstash/libstash.h>

#include "stil_defs.h"

#define	_BUF_SIZE	4096
#define	_PROMPT_STRLEN	  80

/*  #define	_STIL_SIGHANDLER */
#ifdef _STIL_SIGHANDLER
struct handler_s {
	cw_bool_t	quit;
	sigset_t	hupset;
	cw_thd_t	*sig_thd;
};
#endif

struct stil_arg_s {
	cw_uint8_t	*buffer;
	cw_uint32_t	buffer_len;
	cw_uint32_t	buffer_offset;
};

/*
 * Globals.  These are global due to the libedit API not providing a way to pass
 * them to the prompt function.
 */
cw_stilo_t	thread;
cw_stilo_threadp_t threadp;
EditLine	*el;
History		*hist;
cw_uint8_t	prompt_str[_PROMPT_STRLEN];

char		*prompt(EditLine *a_el);
cw_sint32_t	cl_read(void *a_arg, cw_stilo_t *a_file, cw_uint32_t a_len,
    cw_uint8_t *r_str);
#ifdef _STIL_SIGHANDLER
void		*sig_handler(void *a_arg);
#endif
void		usage(const char *a_progname);
const char	*basename(const char *a_str);

int
main(int argc, char **argv, char **envp)
{
	int		retval;
#ifdef _STIL_SIGHANDLER
	struct handler_s handler_arg;
#endif
	cw_stil_t	stil;
	static const cw_uint8_t	version[] =
	    "product print (, version ) print version print (.\n) print flush";

	libstash_init();
#ifdef _STIL_SIGHANDLER
	/*
	 * Set the per-thread signal masks such that only one thread will catch
	 * SIGHUP and SIGINT.
	 */
	handler_arg.quit = FALSE;
	sigemptyset(&handler_arg.hupset);
	sigaddset(&handler_arg.hupset, SIGHUP);
	sigaddset(&handler_arg.hupset, SIGINT);
	sigaddset(&handler_arg.hupset, SIGWINCH);
	thd_sigmask(SIG_BLOCK, &handler_arg.hupset, NULL);
	handler_arg.sig_thd = thd_new(sig_handler, (void *)&handler_arg, FALSE);
#endif

	/*
	 * Do a bunch of extra setup work to hook in command editing
	 * functionality if this is an interactive session.  Otherwise, parse
	 * command line arguments to get the source file and run the
	 * interpreter.
	 */
	if (isatty(0) && argc == 1) {
		/*
		 * Do not stop on error.
		 *
		 * Quit on estackoverflow in order to avoid an infinite loop.
		 */
		static const cw_uint8_t	code[] = "
currenterror begin
	/stop {} def
end
errordict begin
	/estackoverflow {
		//estackoverflow quit
	} def
end
/promptstring {
	count cvs dup length 4 add string
	dup 0 (s:) putinterval dup dup length 2 sub (> )
	putinterval dup 3 2 roll 2 exch putinterval
} bind def
";
		struct stil_arg_s	arg = {NULL, 0, 0};
		char			*editor;

		/*
		 * Initialize the command editor.  Do this before initializing
		 * the stil interpreter, since libstil munges the environment.
		 */
		hist = history_init();
		history(hist, H_EVENT, 512);

		el = el_init(basename(argv[0]), stdin, stdout);
		el_set(el, EL_HIST, history, hist);
		el_set(el, EL_PROMPT, prompt);

		editor = getenv("STIL_EDITOR");
		if (editor == NULL || (strcmp(editor, "emacs") && strcmp(editor,
		    "vi"))) {
			/*
			 * Default to emacs key bindings, since they're more
			 * intuitive to the uninitiated.
			 */
			editor = "emacs";
		}
		el_set(el, EL_EDITOR, editor);
/*  		el_set(el, EL_SIGNAL, 1); */

		stil_new(&stil, argc, argv, envp, cl_read, NULL, NULL, (void
		    *)&arg, NULL);
		stilo_thread_new(&thread, &stil);
		stilo_threadp_new(&threadp);

		/*
		 * Print product and version info.  Redefine stop so that the
		 * interpreter won't exit on error.
		 */
		stilo_thread_interpret(&thread, &threadp, version,
		    sizeof(version) - 1);
		stilo_thread_interpret(&thread, &threadp, code, sizeof(code) -
		    1);
		stilo_thread_flush(&thread, &threadp);
		/* Run the interpreter such that it will not exit on errors. */
		stilo_thread_start(&thread);

		/* Clean up the command editor. */
		el_end(el);
		history_end(hist);

		if (arg.buffer != NULL)
			_cw_free(arg.buffer);
	} else {
		cw_stilo_t		*file;
		static const cw_uint8_t	magic[] =
		    "/#! {mark} def /!# {cleartomark} def";
		int			c;
		cw_bool_t		opt_version = FALSE;
		cw_uint8_t		*opt_expression = NULL;

		/*
		 * Parse command line arguments, but postpone taking any actions
		 * that require the stil interpreter to be up and running.  This
		 * is necessary because we need to know how much of argv to pass
		 * to stil_new().
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
		stil_new(&stil, argc - optind, &argv[optind], envp, NULL, NULL,
		    NULL, NULL, NULL);
		stilo_thread_new(&thread, &stil);
		stilo_threadp_new(&threadp);

		/*
		 * Act on the command line arguments, now that the interpreter
		 * is initialized.
		 */
		if (opt_version) {
			/*
			 * Print the version and exit.
			 */
			stilo_thread_interpret(&thread, &threadp, version,
			    sizeof(version) - 1);
			stilo_thread_flush(&thread, &threadp);
			retval = 0;
			goto RETURN;
		} else if (opt_expression != NULL) {
			cw_stilo_t	*string;
			cw_uint8_t	*str;

			/*
			 * Push the string onto the execution stack.
			 */
			string =
			    stilo_stack_push(stilo_thread_ostack_get(&thread));
			stilo_string_new(string, &stil, FALSE,
			    strlen(opt_expression));
			stilo_attrs_set(string, STILOA_EXECUTABLE);
			str = stilo_string_get(string);
			memcpy(str, opt_expression,
			    stilo_string_len_get(string));
		} else if (optind < argc) {
			int	src_fd;

			/*
			 * Remaining command line arguments should be the name
			 * of a source file, followed by optional arguments.
			 * Open the source file, wrap it in a stil file object,
			 * and push it onto the execution stack.
			 */
			src_fd = open(argv[optind], O_RDONLY);
			if (src_fd == -1) {
				out_put(out_err, "[s]: Error in open(\"[s]\","
				    " O_RDONLY): [s]\n", basename(argv[0]),
				    argv[optind], strerror(errno));
				retval = 1;
				goto RETURN;
			}
			file =
			    stilo_stack_push(stilo_thread_ostack_get(&thread));
			stilo_file_new(file, &stil, FALSE);
			stilo_attrs_set(file, STILOA_EXECUTABLE);
			stilo_file_fd_wrap(file, src_fd);
		} else if (argc == 1) {
			/*
			 * No source file specified, and there there was no -e
			 * expression specified either, so treat stdin as the
			 * source.
			 *
			 * In other words, there were no arguments specified,
			 * and this isn't a tty.
			 */
			file =
			    stilo_stack_push(stilo_thread_ostack_get(&thread));
			stilo_dup(file, stil_stdin_get(&stil));
			stilo_attrs_set(file, STILOA_EXECUTABLE);
		} else
			_cw_not_reached();

		/* Create procedures to handle #! magic. */
		stilo_thread_interpret(&thread, &threadp, magic, sizeof(magic) -
		    1);
		stilo_thread_flush(&thread, &threadp);

		/* Run the interpreter non-interactively. */
		systemdict_start(&thread);
	}

	retval = 0;
	RETURN:
	stilo_threadp_delete(&threadp, &thread);
	stil_delete(&stil);
	CLERROR:
#ifdef _STIL_SIGHANDLER
	/*
	 * Tell the signal handler thread to quit, then join on it.
	 */
	handler_arg.quit = TRUE;
	raise(SIGHUP);
	thd_join(handler_arg.sig_thd);
#endif
	libstash_shutdown();
	return retval;
}

char *
prompt(EditLine *a_el)
{
	if ((stilo_thread_deferred(&thread) == FALSE) &&
	    (stilo_thread_state(&thread) == THREADTS_START)) {
		static const cw_uint8_t	code[] = "promptstring";
		cw_uint8_t		*pstr;
		cw_uint32_t		plen, maxlen;
		cw_stilo_t		*stilo;
		cw_stilo_t		*stack;

		stack = stilo_thread_ostack_get(&thread);

		/* Push the prompt onto the data stack. */
		stilo_thread_interpret(&thread, &threadp, code, sizeof(code) -
		    1);
		stilo_thread_flush(&thread, &threadp);

		/* Get the actual prompt string. */
		stilo = stilo_stack_get(stack);
		if (stilo == NULL) {
			stilo_thread_error(&thread,
			    STILO_THREADE_STACKUNDERFLOW);
			maxlen = 0;
		} else if (stilo_type_get(stilo) != STILOT_STRING) {
			stilo_thread_error(&thread, STILO_THREADE_TYPECHECK);
			maxlen = 0;
		} else {
			pstr = stilo_string_get(stilo);
			plen = stilo_string_len_get(stilo);

			/* Copy the prompt string to a global buffer. */
			maxlen = (plen > _PROMPT_STRLEN - 1) ? _PROMPT_STRLEN -
			    1 : plen;
			strncpy(prompt_str, pstr, _PROMPT_STRLEN - 1);
		}

		prompt_str[maxlen] = '\0';

		/* Pop the prompt string off the data stack. */
		stilo_stack_pop(stack);
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

cw_sint32_t
cl_read(void *a_arg, cw_stilo_t *a_file, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	const char		*str;
	int			count = 0;
	static cw_bool_t	continuation = FALSE;
	struct stil_arg_s	*arg = (struct stil_arg_s *)a_arg;

	_cw_assert(a_len > 0);

	if (arg->buffer_offset == 0) {
		if ((str = el_gets(el, &count)) == NULL) {
			retval = 0;
			goto RETURN;
		}
		_cw_assert(count > 0);

		/*
		 * Update the command line history.
		 */
		if ((stilo_thread_deferred(&thread) == FALSE) &&
		    (stilo_thread_state(&thread) == THREADTS_START)) {
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
		 * Return as much data as possible.  If necessary, store the
		 * rest in buffer.
		 */
		if (count > a_len) {
			/* It won't fit. */
			memcpy(r_str, str, a_len);
			count -= a_len;
			str += a_len;
			if (count > arg->buffer_len) {
				/*
				 * The buffer isn't big enough.  Expand it so
				 * that it's just large enough.
				 */
				if (arg->buffer == NULL)
					arg->buffer = (cw_uint8_t
					    *)_cw_malloc(count);
				else
					arg->buffer = (cw_uint8_t
					    *)_cw_realloc(arg->buffer, count);
				arg->buffer_len = count;
			}
			memcpy(arg->buffer, str, count);
			arg->buffer_offset = count;

			retval = a_len;
		} else {
			/* It will fit. */
			memcpy(r_str, str, count);
			retval = count;
		}
	} else {
		/*
		 * We still have buffered data from the last time we were
		 * called.  Return as much of it as possible.
		 */
		if (arg->buffer_offset > a_len) {
			/* There are more data than we can return. */
			memcpy(r_str, arg->buffer, a_len);
			memmove(r_str, &r_str[a_len], arg->buffer_offset -
			    a_len);
			arg->buffer_offset -= a_len;
			retval = a_len;
		} else {
			/* Return all the data. */
			memcpy(r_str, arg->buffer, arg->buffer_offset);
			retval = arg->buffer_offset;
			arg->buffer_offset = 0;
		}
	}

	RETURN:
	return retval;
}

#ifdef _STIL_SIGHANDLER
void *
sig_handler(void *a_arg)
{
	struct handler_s	*arg = (struct handler_s *)a_arg;
	int			sig;

	for (;;) {
		sigwait(&arg->hupset, &sig);

		switch (sig) {
		case SIGHUP:
			if (arg->quit == FALSE) {
				/*
				 * We've received a signal from somewhere
				 * outside this program.
				 */
				exit(0);
			} else
				goto RETURN;
		case SIGINT:
			exit(0);
		case SIGWINCH:
			/* XXX Doesn't return until cl_read() returns. */
			el_resize(el);
			break;
		default:
			_cw_not_reached();
		}
	}

	RETURN:
	return NULL;
}
#endif

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
	    "    -e <expr> | Execute <expr> as stil code.\n",
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
