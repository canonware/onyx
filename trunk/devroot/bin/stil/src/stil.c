/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include <libstil/libstil.h>
#include <libedit/libedit.h>
#include <libstash/libstash.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#define	_BUF_SIZE	4096
#define	_PROMPT_STRLEN	  80

/*
 * Globals.
 */
cw_stilt_t	stilt;
cw_stilts_t	estilts;
cw_uint8_t	prompt_str[_PROMPT_STRLEN];

char *prompt(EditLine *a_el);
const char *basename(const char *a_str);

int
main(int argc, char **argv)
{
	cw_stil_t	stil;
	cw_stilts_t	stilts;
	cw_out_t	out;

	libstash_init();

	out_new(&out, cw_g_mem);
	out_set_default_fd(&out, 1);

	stil_new(&stil);
	stilt_new(&stilt, &stil);
	stilts_new(&stilts, &stilt);
	stilts_new(&estilts, &stilt);

	if (isatty(0)) {
		cw_uint8_t	code[] =
		    "product print `, version ' print version print \".\n\""
		    " print flush";
		EditLine	*el;
		History		*hist;
		const char	*str;
		int		count;
		cw_bool_t	continuation = FALSE;

		/* Print product and version info. */
		stilt_interpret(&stilt, &estilts, code, sizeof(code) - 1);
		stilt_flush(&stilt, &estilts);

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

		/* Loop on input until the user quits. */
		for (;;) {
			if ((str = el_gets(el, &count)) == NULL)
				break;
			if ((stilt_deferred(&stilt) == FALSE) &&
			    (stilt_state(&stilt) == STATE_START)) {
				const HistEvent	*hevent;

				/*
				 * Completion of a history element.  Insert it,
				 * taking care to avoid simple (non-continued)
				 * duplicates.
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
				 * Continuation.  Append it to the current
				 * history element.
				 */
				history(hist, H_ADD, str);
				continuation = TRUE;
			}

			stilt_interpret(&stilt, &stilts, str,
			    (cw_uint32_t)count);
		}
		stilt_flush(&stilt, &stilts);

		/* Clean up the command editor. */
		el_end(el);
		history_end(hist);
	} else {
		char	input[_BUF_SIZE];
		ssize_t	bytes_read;

		/* Loop on input until EOF. */
		for (;;) {
			bytes_read = read(0, input, _BUF_SIZE - 1);
			if (bytes_read <= 0)
				break;
			stilt_interpret(&stilt, &stilts, input,
			    (cw_uint32_t)bytes_read);
		}
		stilt_flush(&stilt, &stilts);
	}

	stilts_delete(&estilts, &stilt);
	stilts_delete(&stilts, &stilt);
	stilt_delete(&stilt);
	stil_delete(&stil);
	libstash_shutdown();
	return 0;
}

char *
prompt(EditLine *a_el)
{
	if ((stilt_deferred(&stilt) == FALSE) && (stilt_state(&stilt) ==
	    STATE_START)) {
		cw_uint8_t	code[] = "prompt";
		cw_uint8_t	*pstr;
		cw_uint32_t	plen, maxlen;
		cw_stilo_t	*stilo;
		cw_stils_t	*stack = stilt_data_stack_get(&stilt);

		/* Push the prompt onto the data stack. */
		stilt_interpret(&stilt, &estilts, code, sizeof(code) - 1);
		stilt_flush(&stilt, &estilts);

		/* Get the actual prompt string. */
		stilo = stils_get(stack);
		pstr = stilo_string_get(stilo);
		plen = stilo_string_len_get(stilo);

		/* Copy the prompt string to a global buffer. */
		maxlen = (plen > _PROMPT_STRLEN - 1) ? _PROMPT_STRLEN - 1 :
		    plen;
		strncpy(prompt_str, pstr, _PROMPT_STRLEN - 1);
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
