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

	/* XXX Set up oom handler. */

	out_new(&out, cw_g_mem);
	out_set_default_fd(&out, 1);

	if (stil_new(&stil) == NULL) {
		_cw_out_put_e("Error in stil_new()\n");
		exit(1);
	}
	if (stilt_new(&stilt, &stil) == NULL) {
		_cw_out_put_e("Error in stilt_new()\n");
		exit(1);
	}
	if (stilts_new(&stilts, &stilt) == NULL) {
		_cw_out_put_e("Error in stilts_new()\n");
		exit(1);
	}
	if (stilts_new(&estilts, &stilt) == NULL) {
		_cw_out_put_e("Error in stilts_new()\n");
		exit(1);
	}

	if (isatty(0)) {
		cw_uint8_t	code[] =
		    "product print `, version ' print version print \".\n\""
		    " print flush\n";
		EditLine	*el;
		History		*hist;
		const HistEvent	*hevent;
		const char	*str;
		int		count;

		/* Print product and version info. */
		stilt_interp_str(&stilt, &estilts, code, sizeof(code) - 1);

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
			/*
			 * If not a duplicate, store the command to the
			 * history.
			 */
			hevent = history(hist, H_FIRST);
			if (hevent == NULL || strcmp(str, hevent->str))
				history(hist, H_ENTER, str);

			stilt_interp_str(&stilt, &stilts, str,
			    (cw_uint32_t)count);
		}

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
			stilt_interp_str(&stilt, &stilts, input,
			    (cw_uint32_t)bytes_read);
		}
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
	char	*retval;

	if (stilt_deferred(&stilt) == FALSE) {
		if (stilt_state(&stilt) == STATE_START) {
			cw_uint8_t	code[] = "prompt\n";
			cw_uint8_t	*pstr;
			cw_uint32_t	plen, maxlen;
			cw_stilo_t	*stilo;
			cw_stils_t	*stack = stilt_data_stack_get(&stilt);

			/* Push the prompt onto the data stack. */
			stilt_interp_str(&stilt, &estilts, code, sizeof(code) -
			    1);

			/* Get the actual prompt string. */
			stilo = stils_get(stack, 0);
			pstr = stilo_string_get(stilo);
			plen = stilo_string_len_get(stilo);

			/* Copy the prompt string to a global buffer. */
			maxlen = (plen > _PROMPT_STRLEN - 1) ? _PROMPT_STRLEN -
			    1 : plen;
			strncpy(prompt_str, pstr, _PROMPT_STRLEN - 1);
			prompt_str[maxlen] = '\0';

			/* Pop the prompt string off the data stack. */
			stils_pop(stack, &stilt, 1);

			retval = prompt_str;
		} else {
			/*
			 * Continuation of a string or similarly parsed token.
			 */
			retval = "";
		}
	} else {
		/*
		 * The scanner is in a deferred state right now which means that
		 * we cannot call the interpreter to get the prompt string.  Use
		 * the previous value this time around.  This will only result
		 * in an incorrect prompt if the user types something like:
		 *
		 * s> /prompt {"prompt> "} def /foo {
		 * s> "bar"
		 * s> } def
		 * prompt>
		 *
		 * Oh well.
		 */
		retval = prompt_str;
	}

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
