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

struct stil_arg_s {
	cw_uint8_t	*buffer;
	cw_uint32_t	buffer_len;
	cw_uint32_t	buffer_offset;
};

/*
 * Globals.  These are global due to the libedit API not providing a way to pass
 * them to the prompt function.
 */
cw_stilt_t	stilt;
cw_stilts_t	stilts;
EditLine	*el;
History		*hist;
cw_uint8_t	prompt_str[_PROMPT_STRLEN];

void		argv_init(cw_stilt_t *a_stilt, int a_argc, char **a_argv);
void		envdict_init(cw_stilt_t *a_stilt, char **a_envp);
char		*prompt(EditLine *a_el);
cw_sint32_t	cl_read(void *a_arg, cw_stilo_t *a_file, cw_uint32_t a_len,
    cw_uint8_t *r_str);
const char	*basename(const char *a_str);

int
main(int argc, char **argv, char **envp)
{
	int		retval;
	cw_stil_t	stil;

	libstash_init();

	/*
	 * Do a bunch of extra setup work to hook in command editing
	 * functionality if this is an interactive session.  Otherwise, parse
	 * command line arguments to get the source file and run the
	 * interpreter.
	 */
	if (isatty(0) && argc == 1) {
		static const cw_uint8_t	code[] =
		    "product print (, version ) print version print (.\n)"
		    " print flush"
		    "/stop {} def"
		    "/promptstring {count cvs dup length 4 add string"
		    " dup 0 (s:) putinterval dup dup length 2 sub (> )"
		    " putinterval dup 3 2 roll 2 exch putinterval} bind def"
		    ;
		struct stil_arg_s	arg = {NULL, 0, 0};

		stil_new(&stil, cl_read, NULL, NULL, (void *)&arg);
		stilt_new(&stilt, &stil);
		stilts_new(&stilts);

		/* Set up argv and envdict. */
		argv_init(&stilt, argc, argv);
		envdict_init(&stilt, envp);

		/*
		 * Print product and version info.  Redefine stop so that the
		 * interpreter won't exit on error.
		 */
		stilt_interpret(&stilt, &stilts, code, sizeof(code) - 1);
		stilt_flush(&stilt, &stilts);

		/*
		 * Initialize the command editor.
		 */
		hist = history_init();
		history(hist, H_EVENT, 512);

		el = el_init(basename(argv[0]), stdin, stdout);
		el_set(el, EL_HIST, history, hist);
		el_set(el, EL_PROMPT, prompt);
		el_set(el, EL_EDITOR, "emacs");
		el_set(el, EL_SIGNAL, 1);

		/* Run the interpreter such that it will not exit on errors. */
		stilt_start(&stilt);

		/* Clean up the command editor. */
		el_end(el);
		history_end(hist);

		if (arg.buffer != NULL)
			_cw_free(arg.buffer);
	} else {
		int			src_fd;
		cw_stilo_t		*file;
		static const cw_uint8_t	magic[] =
		    "/#! {mark} def /!# {cleartomark} def";

		stil_new(&stil, NULL, NULL, NULL, NULL);
		stilt_new(&stilt, &stil);
		stilts_new(&stilts);

		/*
		 * Set up argv and envdict.  Since this is a non-interactive
		 * invocation, don't include the first element of argv.
		 */
		argv_init(&stilt, argc - 1, &argv[1]);
		envdict_init(&stilt, envp);

		/* Create procedures to handle #! magic. */
		stilt_interpret(&stilt, &stilts, magic, sizeof(magic) - 1);
		stilt_flush(&stilt, &stilts);

		/*
		 * Open the source file, wrap it in a stil file object, and push
		 * it onto the execution stack.
		 */
		src_fd = open(argv[1], O_RDONLY);
		if (src_fd == -1) {
			_cw_out_put("[s]: Error in open(): [s]\n",
			    basename(argv[0]), strerror(errno));
			retval = 1;
			goto RETURN;
		}
		file = stils_push(stilt_estack_get(&stilt));
		stilo_file_new(file, &stil);
		stilo_file_fd_wrap(file, src_fd);
		stilo_attrs_set(file, STILOA_EXECUTABLE);

		/* Run the interpreter non-interactively. */
		systemdict_start(&stilt);
	}

	retval = 0;
	RETURN:
	stilts_delete(&stilts, &stilt);
	stilt_delete(&stilt);
	stil_delete(&stil);
	libstash_shutdown();
	return retval;
}

/*
 * Define the argv array in globaldict.
 */
void
argv_init(cw_stilt_t *a_stilt, int a_argc, char **a_argv)
{
	int		i;
	cw_sint32_t	len;
	static const cw_uint8_t footer[] = " put setglobal";
	char		*code;
	cw_stilts_t	stilts;

	stilts_new(&stilts);

	len = out_put_sa(cw_g_out, &code,
	    "currentglobal true setglobal globaldict /argv [i] array",
	    a_argc);
	stilt_interpret(a_stilt, &stilts, code, len);
	_cw_free(code);

	for (i = 0; i < a_argc; i++) {
		len = out_put_sa(cw_g_out, &code, " dup [i] ([s]) put", i,
		    a_argv[i]);
		    stilt_interpret(a_stilt, &stilts, code, len);
		    _cw_free(code);
	}
	stilt_interpret(a_stilt, &stilts, footer, sizeof(footer) - 1);
	
	stilt_flush(a_stilt, &stilts);
	stilts_delete(&stilts, a_stilt);
}

/*
 * Define the envdict dictionary in globaldict.
 */
void
envdict_init(cw_stilt_t *a_stilt, char **a_envp)
{
	int		i, j;
	cw_sint32_t	len;
	static const cw_uint8_t header[] =
	    "currentglobal true setglobal globaldict /envdict <<";
	static const cw_uint8_t footer[] = ">> put setglobal";
	char		*code, *key, *val, *oldval;
	cw_stilts_t	stilts;

	stilts_new(&stilts);

	stilt_interpret(a_stilt, &stilts, header, sizeof(header) - 1);

	for (i = 0; a_envp[i] != NULL; i++) {
		/* Break the key and value apart. */
		oldval = a_envp[i];
		key = strsep(&oldval, "=");
		val = (char *)_cw_malloc(2 * strlen(oldval) + 1);

		/*
		 * Protect '\'' characters, which delimit literal strings in
		 * stil.
		 */
		for (j = 0; *oldval != '\0'; oldval++, j++) {
			val[j] = *oldval;
			if (val[j] == '\'') {
				j++;
				val[j] = '\'';
			}
		}
		val[j] = '\'';	/* stil string delimiter. */

		len = out_put_sa(cw_g_out, &code, " /[s] `", key);
		stilt_interpret(a_stilt, &stilts, code, len);
		_cw_free(code);

		/*
		 * Inject the value this way, so that the printing code doesn't
		 * stumble on binary data.
		 */
		stilt_interpret(a_stilt, &stilts, val, j + 1);

		_cw_free(val);
	}
	stilt_interpret(a_stilt, &stilts, footer, sizeof(footer) - 1);
	
	stilt_flush(a_stilt, &stilts);
	stilts_delete(&stilts, a_stilt);
}

char *
prompt(EditLine *a_el)
{
	if ((stilt_deferred(&stilt) == FALSE) && (stilt_state(&stilt) ==
	    STILTTS_START)) {
		static const cw_uint8_t	code[] = "promptstring";
		cw_uint8_t		*pstr;
		cw_uint32_t		plen, maxlen;
		cw_stilo_t		*stilo;
		cw_stils_t		*stack = stilt_ostack_get(&stilt);

		/* Push the prompt onto the data stack. */
		stilt_interpret(&stilt, &stilts, code, sizeof(code) - 1);
		stilt_flush(&stilt, &stilts);

		/* Get the actual prompt string. */
		stilo = stils_get(stack);
		if (stilo == NULL) {
			stilt_error(&stilt, STILTE_STACKUNDERFLOW);
			maxlen = 0;
		} else if (stilo_type_get(stilo) != STILOT_STRING) {
			stilt_error(&stilt, STILTE_TYPECHECK);
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
		stils_pop(stack);
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
		if ((stilt_deferred(&stilt) == FALSE) &&
		    (stilt_state(&stilt) == STILTTS_START)) {
			const HistEvent	*hevent;

			/*
			 * Completion of a history element.  Insert it, taking
			 * care to avoid simple (non-continued) duplicates.
			 */
			if (continuation) {
				history(hist, H_ENTER, str);
				continuation = FALSE;
			} else {
				hevent = history(hist, H_FIRST);
				if (hevent == NULL || strcmp(str,
				    hevent->str))
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
