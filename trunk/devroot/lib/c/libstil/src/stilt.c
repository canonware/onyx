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

#include "../include/libstil/libstil.h"

#include <ctype.h>
#include <limits.h>

cw_bool_t	stilnt_l_new(cw_stilnt_t *a_stilnt, cw_mem_t *a_mem,
    cw_stilng_t *a_stilng);
void		stilnt_l_delete(cw_stilnt_t *a_stilnt);

#define _CW_STILT_GETC(a_i)						\
	a_stilt->tok_str[(a_i)]

#define _CW_STILT_PUTC(a_c)						\
	do {								\
		if ((a_stilt->index >= _CW_STILT_BUFFER_SIZE) &&	\
		    (stilt_p_tok_str_expand(a_stilt) == -1)) {		\
			retval = -1;					\
			goto RETURN;					\
		}							\
		a_stilt->tok_str[a_stilt->index] = (a_c);		\
		a_stilt->index++;					\
	} while (0)

/*
 * Update the line and column counters to reflect that a '\n' was just seen.
 * The column number is set to -1 because the loop will increment it before the
 * next character is seen, which should be at column 0.
 */
#define _CW_STILT_NEWLINE()						\
	do {								\
		a_stilt->line++;					\
		a_stilt->column = -1;					\
	} while (0)

struct cw_stilt_entry_s {
	cw_thd_t	thd;
	cw_stilt_t	*stilt;
	cw_buf_t	buf;
};

static cw_sint32_t	stilt_p_feed(cw_stilt_t *a_stilt, const char *a_str,
    cw_uint32_t a_len);
static cw_sint32_t	stilt_p_exec(cw_stilt_t *a_stilt);
static void		stilt_p_tok_str_reset(cw_stilt_t *a_stilt);

static cw_sint32_t	stilt_p_tok_str_expand(cw_stilt_t *a_stilt);
static void		stilt_p_print_token(cw_stilt_t *a_stilt, cw_uint32_t
    a_length, const char *a_note);
static void		stilt_p_print_syntax_error(cw_stilt_t *a_stilt,
    cw_uint8_t a_c);
static void		*stilt_p_entry(void *a_arg);
static void		stilt_p_procedure_accept(cw_stilt_t *a_stilt);
static void		stilt_p_name_accept(cw_stilt_t *a_stilt);

#ifdef _LIBSTIL_DBG
#define _CW_STILT_MAGIC 0x978fdbe0
#endif

/*
 * Size and fullness control of initial name cache hash table.  This starts out
 * empty, but should be expected to grow rather quickly to start with.
 */
#define _CW_STILT_STILN_BASE_TABLE	256
#define _CW_STILT_STILN_BASE_GROW	200
#define _CW_STILT_STILN_BASE_SHRINK	 64

/*
 * Size and fullness control of initial root set for local VM.  Local VM is
 * empty to begin with.
 */
#define _CW_STILT_ROOTS_BASE_TABLE	 32
#define _CW_STILT_ROOTS_BASE_GROW	 24
#define _CW_STILT_ROOTS_BASE_SHRINK	  8

cw_stilt_t *
stilt_new(cw_stilt_t *a_stilt, cw_stil_t *a_stil)
{
	cw_stilt_t	*retval;
	cw_stilo_t	*stilo;

	if (a_stilt != NULL) {
		retval = a_stilt;
		memset(a_stilt, 0, sizeof(cw_stilt_t));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_stilt_t *)_cw_malloc(sizeof(cw_stilt_t));
		if (retval == NULL)
			goto OOM_1;
		memset(a_stilt, 0, sizeof(cw_stilt_t));
		retval->is_malloced = TRUE;
	}

	if (stilat_new(&retval->stilat, a_stilt, stil_stilag_get(a_stil)))
		goto OOM_2;

	if (stilnt_l_new(&retval->stilnt, stilat_mem_get(&retval->stilat),
	    stil_stilng_get(a_stil)))
		goto OOM_3;

	if (stils_new(&retval->exec_stils,
	    stilat_stilsc_pool_get(&a_stilt->stilat)) == NULL)
		goto OOM_4;

	if (stils_new(&retval->data_stils,
	    stilat_stilsc_pool_get(&a_stilt->stilat)) == NULL)
		goto OOM_5;

	if (stils_new(&retval->dict_stils,
	    stilat_stilsc_pool_get(&a_stilt->stilat)) == NULL)
		goto OOM_6;

	/* Push systemdict onto the dictionary stack. */
	stilo = stils_push(&retval->dict_stils);
	stilo_dup(stilo, stil_systemdict_get(a_stil));

	retval->stdout_fd = 1;
	retval->stil = a_stil;
	retval->state = STATE_START;
	retval->line = 1;
	retval->tok_str = retval->buffer;
#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILT_MAGIC;
#endif

	return retval;
	OOM_6:
	stils_delete(&retval->data_stils, retval);
	OOM_5:
	stils_delete(&retval->exec_stils, retval);
	OOM_4:
	stilnt_l_delete(&retval->stilnt);
	OOM_3:
	stilat_delete(&retval->stilat);
	OOM_2:
	if (retval->is_malloced)
		_cw_free(retval);
	OOM_1:
	return NULL;
}

void
stilt_delete(cw_stilt_t *a_stilt)
{
	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	stils_delete(&a_stilt->dict_stils, a_stilt);
	stils_delete(&a_stilt->data_stils, a_stilt);
	stils_delete(&a_stilt->exec_stils, a_stilt);
	stilnt_l_delete(&a_stilt->stilnt);
	stilat_delete(&a_stilt->stilat);
	if (a_stilt->is_malloced)
		_cw_free(a_stilt);
}

void
stilt_get_position(cw_stilt_t *a_stilt, cw_uint32_t *r_line, cw_uint32_t
    *r_column)
{
	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	*r_line = a_stilt->line;
	*r_column = a_stilt->column;
}

void
stilt_set_position(cw_stilt_t *a_stilt, cw_uint32_t a_line, cw_uint32_t
    a_column)
{
	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	a_stilt->line = a_line;
	a_stilt->column = a_column;
}

cw_bool_t
stilt_interp_str(cw_stilt_t *a_stilt, const char *a_str, cw_uint32_t a_len)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	retval = stilt_p_feed(a_stilt, a_str, a_len);

	return retval;
}

cw_bool_t
stilt_interp_buf(cw_stilt_t *a_stilt, cw_buf_t *a_buf)
{
	cw_bool_t	retval;
	int		iov_cnt, i;
	const struct iovec *iov;

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);
	_cw_check_ptr(a_buf);

	iov = buf_iovec_get(a_buf, UINT_MAX, FALSE, &iov_cnt);

	for (i = 0; i < iov_cnt; i++) {
		if (stilt_p_feed(a_stilt, iov[i].iov_base,
		    (cw_uint32_t)iov[i].iov_len)) {
			retval = TRUE;
			goto RETURN;
		}
	}

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
stilt_detach_str(cw_stilt_t *a_stilt, const char *a_str, cw_uint32_t a_len)
{
	cw_bool_t	retval;
	cw_buf_t	buf;

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);
	_cw_check_ptr(a_str);

	buf_new(&buf, stilt_mem_get(a_stilt));
	if (buf_range_set(&buf, 0, a_len, (cw_uint8_t *)a_str, FALSE)) {
		retval = TRUE;
		goto RETURN;
	}
	retval = stilt_detach_buf(a_stilt, &buf);

	RETURN:
	buf_delete(&buf);
	return retval;
}

cw_bool_t
stilt_detach_buf(cw_stilt_t *a_stilt, cw_buf_t *a_buf)
{
	struct cw_stilt_entry_s	*entry_arg;

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);
	_cw_check_ptr(a_buf);

	entry_arg = (struct cw_stilt_entry_s *)_cw_malloc(sizeof(struct
	    cw_stilt_entry_s));

	if (entry_arg == NULL)
		goto OOM_1;
	buf_new(&entry_arg->buf, stilt_mem_get(a_stilt));
	if (buf_buf_catenate(&entry_arg->buf, a_buf, TRUE))
		goto OOM_2;
	stilt_p_entry((void *)entry_arg);

	return FALSE;

	OOM_2:
	buf_delete(&entry_arg->buf);
	_cw_free(entry_arg);
	OOM_1:
	return TRUE;
}

static cw_sint32_t
stilt_p_feed(cw_stilt_t *a_stilt, const char *a_str, cw_uint32_t a_len)
{
	cw_sint32_t	retval;
	cw_uint32_t	i;
	cw_uint8_t	c;
	cw_stilo_t	*stilo;

	for (i = 0; i < a_len; i++, a_stilt->column++) {
		c = a_str[i];

#if (0)
#define _CW_STILS_PSTATE(a)						\
	do {								\
		if (a_stilt->state == (a))				\
			_cw_out_put("[s]\n", #a);			\
	} while (0)

		_cw_out_put("c: '[c]' ([i]), index: [i] ", c, c,
		    a_stilt->index);
		_CW_STILS_PSTATE(STATE_START);
		_CW_STILS_PSTATE(STATE_LT_CONT);
		_CW_STILS_PSTATE(STATE_GT_CONT);
		_CW_STILS_PSTATE(STATE_SLASH_CONT);
		_CW_STILS_PSTATE(STATE_COMMENT);
		_CW_STILS_PSTATE(STATE_NUMBER);
		_CW_STILS_PSTATE(STATE_ASCII_STRING);
		_CW_STILS_PSTATE(STATE_ASCII_STRING_NEWLINE_CONT);
		_CW_STILS_PSTATE(STATE_ASCII_STRING_CRLF_CONT);
		_CW_STILS_PSTATE(STATE_ASCII_STRING_PROT_CONT);
		_CW_STILS_PSTATE(STATE_ASCII_STRING_HEX_CONT);
		_CW_STILS_PSTATE(STATE_ASCII_STRING_HEX_FINISH);
		_CW_STILS_PSTATE(STATE_HEX_STRING);
		_CW_STILS_PSTATE(STATE_BASE85_STRING);
		_CW_STILS_PSTATE(STATE_BASE85_STRING_CONT);
		_CW_STILS_PSTATE(STATE_NAME);
#undef _CW_STILS_PSTATE
#endif

		/*
		 * If a special character causes the end of the previous token,
		 * the state machine builds the object, then restarts the state
		 * machine without incrementing the input character index.  This
		 * is done in order to avoid having to duplicate the
		 * STATE_START code.
		 */
		RESTART:

		switch (a_stilt->state) {
		case STATE_START:
			/*
			 * A literal string cannot be accepted until one
			 * character past the ' at the end has been seen, at
			 * which point the scanner jumps here.
			 */
			START_CONTINUE:
			_cw_assert(a_stilt->index == 0);

			/* Record where this token starts. */
			a_stilt->tok_line = a_stilt->line;
			a_stilt->tok_column = a_stilt->column;

			switch (c) {
			case '"':
				a_stilt->state = STATE_ASCII_STRING;
				break;
			case '`':
				a_stilt->state = STATE_LIT_STRING;
				break;
			case '<':
				a_stilt->state = STATE_LT_CONT;
				break;
			case '>':
				a_stilt->state = STATE_GT_CONT;
				break;
			case '[':
				stilt_p_print_token(a_stilt, 0, "[");
				/* An operator, not the same as '{'. */
				break;
			case ']':
				stilt_p_print_token(a_stilt, 0, "]");
				/* An operator, not the same as '}'. */
				break;
			case '{':
				stilt_p_print_token(a_stilt, 0, "{");
				a_stilt->defer_count++;
				stilo = stils_push(&a_stilt->data_stils);
				stilo_no_new(stilo);
				/*
				 * Leave the stilo as notype in order to
				 * differentiate from normal marks.
				 */
				break;
			case '}':
				stilt_p_print_token(a_stilt, 0, "}");
				if (a_stilt->defer_count > 0)
					a_stilt->defer_count--;
				else {
					/* XXX Missing '{'. */
					_cw_error("XXX Missing '}'\n");
				}
				stilt_p_procedure_accept(a_stilt);
				break;
			case '/':
				a_stilt->state = STATE_SLASH_CONT;
				break;
			case '%':
				a_stilt->state = STATE_COMMENT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* Swallow. */
				break;
			case '+':
				a_stilt->state = STATE_NUMBER;
				a_stilt->meta.number.sign = POS;
				a_stilt->meta.number.base = 10;
				a_stilt->meta.number.point_offset = -1;
				a_stilt->meta.number.begin_offset = 1;
				_CW_STILT_PUTC(c);
				break;
			case '-':
				a_stilt->state = STATE_NUMBER;
				a_stilt->meta.number.sign = NEG;
				a_stilt->meta.number.base = 10;
				a_stilt->meta.number.point_offset = -1;
				a_stilt->meta.number.begin_offset = 1;
				_CW_STILT_PUTC(c);
				break;
			case '.':
				a_stilt->state = STATE_NUMBER;
				a_stilt->meta.number.sign =
				    POS;
				a_stilt->meta.number.base = 10;
				a_stilt->meta.number.point_offset = 0;
				a_stilt->meta.number.begin_offset = 0;
				_CW_STILT_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				a_stilt->state = STATE_NUMBER;
				a_stilt->meta.number.sign = POS;
				a_stilt->meta.number.base = 10;
				a_stilt->meta.number.point_offset = -1;
				a_stilt->meta.number.begin_offset = 0;
				_CW_STILT_PUTC(c);
				break;
			default:
				a_stilt->state = STATE_NAME;
				a_stilt->meta.name.action = EXEC;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STATE_LT_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '<':
				a_stilt->state = STATE_START;
				stilt_p_print_token(a_stilt, 0, "<<");
				break;
			case '>':
				a_stilt->state = STATE_START;
				stilt_p_print_token(a_stilt, a_stilt->index,
				    "empty hex string");
				stilt_p_tok_str_reset(a_stilt);
				break;
			case '~':
				a_stilt->state = STATE_BASE85_STRING;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
				a_stilt->state = STATE_HEX_STRING;
				_CW_STILT_PUTC(c);
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* Whitespace within a hex string. */
				a_stilt->state = STATE_HEX_STRING;
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case STATE_GT_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '>':
				a_stilt->state = STATE_START;
				stilt_p_print_token(a_stilt, 0, ">>");
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case STATE_SLASH_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '/':
				a_stilt->state = STATE_NAME;
				a_stilt->meta.name.action = EVAL;
				break;
			case '\n':
				stilt_p_print_syntax_error(a_stilt, c);

				_CW_STILT_NEWLINE();
				break;
			case '\0': case '\t': case '\f': case '\r': case ' ':
			case '"': case '`': case '\'': case '<': case '>':
			case '[': case ']': case '{': case '}': case '%':
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			default:
				a_stilt->state = STATE_NAME;
				a_stilt->meta.name.action = LITERAL;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STATE_COMMENT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\r':
				a_stilt->state = STATE_START;
				break;
			default:
				break;
			}
			break;
		case STATE_NUMBER:
			switch (c) {
			case '.':
				if (a_stilt->meta.number.point_offset == -1) {
					a_stilt->meta.number.point_offset =
					    a_stilt->index;
				} else {
					a_stilt->state = STATE_NAME;
					a_stilt->meta.name.action = EXEC;
				}
				_CW_STILT_PUTC(c);
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y':
			case 'z':
				if (a_stilt->meta.number.base <= (10 +
				    ((cw_uint32_t)(c - 'a')))) {
					/* Too big for this base. */
					a_stilt->state = STATE_NAME;
					a_stilt->meta.name.action = EXEC;
				}
				_CW_STILT_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (a_stilt->meta.number.base <=
				    ((cw_uint32_t)(c - '0'))) {
					/* Too big for this base. */
					a_stilt->state = STATE_NAME;
					a_stilt->meta.name.action = EXEC;
				}
				_CW_STILT_PUTC(c);
				break;
			case '#':{
				cw_uint32_t	ndigits;

				ndigits = a_stilt->index -
				    a_stilt->meta.number.begin_offset;

				if ((a_stilt->meta.number.point_offset != -1) ||
				    (a_stilt->meta.number.begin_offset ==
				    a_stilt->index)) {
					/*
					 * Decimal point already seen, or no
					 * base specified.
					 */
					a_stilt->state = STATE_NAME;
					a_stilt->meta.name.action = EXEC;
				} else {
					cw_uint32_t	i, digit;

					/*
					 * Convert the string to a base
					 * (interpreted as base 10).
					 */
					a_stilt->meta.number.base = 0;

					for (i = 0; i < ndigits; i++) {
						digit =
						    _CW_STILT_GETC(a_stilt->meta.number.begin_offset
						    + i) - '0';

						if (a_stilt->index -
						    a_stilt->meta.number.begin_offset
						    - i == 2)
							digit *= 10;
						a_stilt->meta.number.base +=
						    digit;

						if (((digit != 0) &&
						    ((a_stilt->index -
						    a_stilt->meta.number.begin_offset
						    - i) > 2)) ||
						    (a_stilt->meta.number.base >
						    36)) {
							/*
							 * Base too large. Set
							 * base to 0 so that the
							 * check for too small a
							 * base catches this.
							 */
							a_stilt->meta.number.base
							    = 0;
							break;
						}
					}

					if (a_stilt->meta.number.base < 2) {
						/*
						 * Base too small (or too large,
						 * as detected in the for loop
						 * above).
						 */
						a_stilt->state =
						    STATE_NAME;
						a_stilt->meta.name.action =
						    EXEC;
					} else {
						a_stilt->meta.number.begin_offset
						    = a_stilt->index + 1;
					}
				}

				_CW_STILT_PUTC(c);
				break;
			}
			case '"': case '`': case '<': case '>': case '[':
			case ']': case '{': case '}': case '/': case '%':
				/* New token. */
				a_stilt->state = STATE_START;
				if ((a_stilt->index -
				    a_stilt->meta.number.begin_offset > 1) ||
				    ((a_stilt->index -
				    a_stilt->meta.number.begin_offset > 0) &&
				    (a_stilt->meta.number.point_offset ==
				    -1))) {
					stilt_p_print_token(a_stilt,
					    a_stilt->index, "number");
				} else {
					/* No number specified, so a name. */
					stilt_p_print_token(a_stilt,
					    a_stilt->index, "name 1");
					stilt_p_name_accept(a_stilt);
				}
				stilt_p_tok_str_reset(a_stilt);
				goto RESTART;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				a_stilt->state = STATE_START;
				if ((a_stilt->index -
				    a_stilt->meta.number.begin_offset > 1) ||
				    ((a_stilt->index -
				    a_stilt->meta.number.begin_offset > 0) &&
				    (a_stilt->meta.number.point_offset ==
				    -1))) {
					stilt_p_print_token(a_stilt,
					    a_stilt->index, "number");
				} else {
					/* No number specified, so a name. */
					stilt_p_print_token(a_stilt,
					    a_stilt->index, "name 2");
					stilt_p_name_accept(a_stilt);
				}
				stilt_p_tok_str_reset(a_stilt);
				break;
			default:
				/* Not a number character. */
				a_stilt->state = STATE_NAME;
				a_stilt->meta.name.action = EXEC;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STATE_ASCII_STRING:
			/* The CRLF code jumps here if there was no LF. */
			ASCII_STRING_CONTINUE:

			switch (c) {
			case '\\':
				a_stilt->state =
				    STATE_ASCII_STRING_PROT_CONT;
				break;
			case '"':
				a_stilt->state = STATE_START;

				stilt_p_print_token(a_stilt,
				    a_stilt->index, "string");
				stilo = stils_push(&a_stilt->data_stils);
				stilo_string_new(stilo, a_stilt,
				    a_stilt->index);
				stilo_string_set(stilo, 0, a_stilt->tok_str,
				    a_stilt->index);

				stilt_p_tok_str_reset(a_stilt);
				break;
			case '\r':
				a_stilt->state =
				    STATE_ASCII_STRING_NEWLINE_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			default:
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STATE_ASCII_STRING_NEWLINE_CONT:
			/* All cases in the switch statement do this. */
			_CW_STILT_PUTC('\n');
			a_stilt->state = STATE_ASCII_STRING;
			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();
				break;
			default:
				/*
				 * '\r' was not followed by a '\n'.  Translate
				 * the '\r' to a '\n' and jump back up to the
				 * string scanning state to scan c again.
				 */
				goto ASCII_STRING_CONTINUE;
			}
			break;
		case STATE_ASCII_STRING_PROT_CONT:
			switch (c) {
			case 'n':
				a_stilt->state = STATE_ASCII_STRING;
				_CW_STILT_PUTC('\n');
				break;
			case 'r':
				a_stilt->state = STATE_ASCII_STRING;
				_CW_STILT_PUTC('\r');
				break;
			case 't':
				a_stilt->state = STATE_ASCII_STRING;
				_CW_STILT_PUTC('\t');
				break;
			case 'b':
				a_stilt->state = STATE_ASCII_STRING;
				_CW_STILT_PUTC('\b');
				break;
			case 'f':
				a_stilt->state = STATE_ASCII_STRING;
				_CW_STILT_PUTC('\f');
				break;
			case '\\':
				a_stilt->state = STATE_ASCII_STRING;
				_CW_STILT_PUTC('\\');
				break;
			case '"':
				a_stilt->state = STATE_ASCII_STRING;
				_CW_STILT_PUTC('"');
				break;
			case 'x':
				a_stilt->state =
				    STATE_ASCII_STRING_HEX_CONT;
				break;
			case '\r':
				a_stilt->state =
				    STATE_ASCII_STRING_CRLF_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();

				/* Ignore. */
				a_stilt->state = STATE_ASCII_STRING;
				break;
			default:
				a_stilt->state = STATE_ASCII_STRING;
				_CW_STILT_PUTC('\\');
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STATE_ASCII_STRING_CRLF_CONT:
			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();

				/* Ignore. */
				a_stilt->state = STATE_ASCII_STRING;
				break;
			default:
				goto ASCII_STRING_CONTINUE;
			}
			break;
		case STATE_ASCII_STRING_HEX_CONT:
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
				a_stilt->state =
				    STATE_ASCII_STRING_HEX_FINISH;
				a_stilt->meta.string.hex_val = c;
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case STATE_ASCII_STRING_HEX_FINISH:
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':{
				cw_uint8_t	val;

				a_stilt->state =
				    STATE_ASCII_STRING;
				switch (a_stilt->meta.string.hex_val) {
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					val =
					    (a_stilt->meta.string.hex_val
					    - '0') << 4;
					break;
				case 'a': case 'b': case 'c': case 'd':
				case 'e': case 'f':
					val = ((a_stilt->meta.string.hex_val -
					    'a') + 10) << 4;
					break;
				default:
					_cw_error("Programming error");
				}
				switch (c) {
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					val |= (c - '0');
					break;
				case 'a': case 'b': case 'c': case 'd':
				case 'e': case 'f':
					val |= ((c - 'a') + 10);
					break;
				default:
					_cw_error("Programming error");
				}
				_CW_STILT_PUTC(val);
				break;
			}
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case STATE_LIT_STRING:
			/* The CRLF code jumps here if there was no LF. */
			LIT_STRING_CONTINUE:

			switch (c) {
			case '\'':
				a_stilt->state =
				    STATE_LIT_STRING_PROT_CONT;
				break;
			case '\r':
				a_stilt->state =
				    STATE_LIT_STRING_NEWLINE_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			default:
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STATE_LIT_STRING_NEWLINE_CONT:
			/* All cases in the switch statement do this. */
			_CW_STILT_PUTC('\n');
			a_stilt->state = STATE_LIT_STRING;
			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();
				break;
			default:
				/*
				 * '\r' was not followed by a '\n'.  Translate
				 * the '\r' to a '\n' and jump back up to the
				 * string scanning state to scan c again.
				 */
				goto LIT_STRING_CONTINUE;
			}
			break;
		case STATE_LIT_STRING_PROT_CONT:
			switch (c) {
			case '\'':
				a_stilt->state = STATE_LIT_STRING;
				_CW_STILT_PUTC('\'');
				break;
			default:
				/* Accept literal string. */
				a_stilt->state = STATE_START;

				stilt_p_print_token(a_stilt,
				    a_stilt->index, "literal string");
				stilo = stils_push(&a_stilt->data_stils);
				stilo_string_new(stilo, a_stilt,
				    a_stilt->index);
				stilo_string_set(stilo, 0, a_stilt->tok_str,
				    a_stilt->index);

				stilt_p_tok_str_reset(a_stilt);

				/*
				 * We're currently looking at the first
				 * character of the next token, so re-scan it.
				 */
				goto START_CONTINUE;
			}
			break;
		case STATE_HEX_STRING:
			switch (c) {
			case '>':
				a_stilt->state = STATE_START;
				stilt_p_print_token(a_stilt, a_stilt->index,
				    "hex string");
				stilt_p_tok_str_reset(a_stilt);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
				_CW_STILT_PUTC(c);
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case STATE_BASE85_STRING:
			switch (c) {
			case '~':
				a_stilt->state =
				    STATE_BASE85_STRING_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r':
			case ' ':
				/* Ignore. */
				break;
			default:
				if (((c >= '!') && (c <= 'u')) || (c == 'z'))
					_CW_STILT_PUTC(c);
				else
					stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case STATE_BASE85_STRING_CONT:
			switch (c) {
			case '>':
				a_stilt->state = STATE_START;
				stilt_p_print_token(a_stilt, a_stilt->index,
				    "base 85 string");
				stilt_p_tok_str_reset(a_stilt);
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case STATE_NAME:
			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* End of name. */
				a_stilt->state = STATE_START;
				if (a_stilt->index > 0) {
					stilt_p_print_token(a_stilt,
					    a_stilt->index, "name 3");
					stilt_p_name_accept(a_stilt);
				} else
					stilt_p_print_syntax_error(a_stilt, c);
				stilt_p_tok_str_reset(a_stilt);
				break;
			case '"': case '`': case '<': case '>': case '[':
			case ']': case '{': case '}': case '/': case '%':
				/* New token. */
				a_stilt->state = STATE_START;
				if (a_stilt->index > 0) {
					stilt_p_print_token(a_stilt,
					    a_stilt->index, "name 4");
					stilt_p_name_accept(a_stilt);
				} else
					stilt_p_print_syntax_error(a_stilt, c);
				stilt_p_tok_str_reset(a_stilt);
				goto RESTART;
			default:
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		default:
			_cw_error("Programming error");
			break;
		}
	}

	retval = 0;
	RETURN:
	return retval;
}

static cw_sint32_t
stilt_p_exec(cw_stilt_t *a_stilt)
{
	cw_stilo_t	*stilo;

	/* XXX It's harder than this. */
	stilo = stils_get(&a_stilt->exec_stils, 0);
	stilo->o.operator.f(a_stilt);
	stils_pop(&a_stilt->exec_stils, a_stilt, 1);

	return 0;	/* XXX */
}

static void
stilt_p_tok_str_reset(cw_stilt_t *a_stilt)
{
	if (a_stilt->index > _CW_STILT_BUFFER_SIZE) {
		_cw_stilt_free(a_stilt, a_stilt->tok_str);
		a_stilt->tok_str = a_stilt->buffer;
	}
	a_stilt->index = 0;
}

static cw_sint32_t
stilt_p_tok_str_expand(cw_stilt_t *a_stilt)
{
	cw_sint32_t	retval;

	if (a_stilt->index == _CW_STILT_BUFFER_SIZE) {
		/*
		 * First overflow, initial expansion needed.
		 */
		a_stilt->tok_str = (cw_uint8_t *)_cw_stilt_malloc(a_stilt,
		    a_stilt->index * 2);
		if (a_stilt->tok_str == NULL) {
			retval = -1;
			goto RETURN;
		}
		a_stilt->buffer_len = a_stilt->index * 2;
		memcpy(a_stilt->tok_str, a_stilt->buffer, a_stilt->index);
	} else if (a_stilt->index == a_stilt->buffer_len) {
		cw_uint8_t *t_str;

		/*
		 * Overflowed, and additional expansion needed.
		 */
		t_str = (cw_uint8_t *)_cw_stilt_malloc(a_stilt, a_stilt->index *
		    2);
		if (t_str == NULL) {
			retval = -1;
			goto RETURN;
		}
		a_stilt->buffer_len = a_stilt->index * 2;
		memcpy(t_str, a_stilt->tok_str, a_stilt->index);
		_cw_stilt_free(a_stilt, a_stilt->tok_str);
		a_stilt->tok_str = t_str;
	}

	retval = 0;
	RETURN:
	return retval;
}

static void
stilt_p_print_token(cw_stilt_t *a_stilt, cw_uint32_t a_length, const char
    *a_note)
{
#ifdef _LIBSTIL_DBG
	cw_uint32_t	line, col;

	stilt_get_position(a_stilt, &line, &col);
	_cw_out_put("-->");
	_cw_out_put_n(a_length, "[s]", a_stilt->tok_str);
	_cw_out_put("<-- [s] ([i]:[i] [[--> [i]:[i])\n", a_note,
	    a_stilt->tok_line, a_stilt->tok_column, a_stilt->line,
	    (a_stilt->column != -1) ? a_stilt->column : 0);
#endif
}

static void
stilt_p_print_syntax_error(cw_stilt_t *a_stilt, cw_uint8_t a_c)
{
	_cw_out_put("Syntax error for '[c]' (0x[i|b:16]), following -->", a_c,
	    a_c);
	_cw_out_put_n(a_stilt->index, "[s]", a_stilt->tok_str);
	_cw_out_put("<-- (starts at line [i], column [i])\n",
	    a_stilt->tok_line, a_stilt->tok_column);
	a_stilt->state = STATE_START;
	stilt_p_tok_str_reset(a_stilt);
}

static void *
stilt_p_entry(void *a_arg)
{
	struct cw_stilt_entry_s	*arg = (struct cw_stilt_entry_s *)a_arg;

	if (stilt_interp_buf(arg->stilt, &arg->buf)) {
		/* XXX OOM error needs delivered in interpreter. */
	}
	buf_delete(&arg->buf);
	stilt_delete(arg->stilt);
	/* XXX Deal with detach/join inside interpreter. */
	thd_delete(&arg->thd);
	_cw_free(arg);
	return NULL;
}

static void
stilt_p_procedure_accept(cw_stilt_t *a_stilt)
{
	cw_stilo_t	t_stilo, *stilo, *arr;
	cw_uint32_t	nelements, i;

	/* Find the "mark". */
	for (i = 0, stilo = stils_get(&a_stilt->data_stils, 0);
	     stilo != NULL && stilo_type_get(stilo) != _CW_STILOT_NOTYPE;
	     i++, stilo = stils_get_down(&a_stilt->data_stils, stilo));

	_cw_assert(stilo != NULL);

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	stilo_array_new(&t_stilo, a_stilt, nelements);
	stilo_executable_set(&t_stilo, TRUE);
	arr = stilo_array_get(&t_stilo);

	/*
	 * Traverse up the stack, moving stilo's to the array.
	 */
	for (i = 0, stilo = stils_get_up(&a_stilt->data_stils, stilo); i <
	    nelements; i++, stilo = stils_get_up(&a_stilt->data_stils,
	    stilo))
		stilo_move(&arr[i], stilo);

	/* Pop the stilo's off the stack now. */
	stils_pop(&a_stilt->data_stils, a_stilt, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(&a_stilt->data_stils);
	stilo_move(stilo, &t_stilo);

	/* Clean up. */
	stilo_delete(&t_stilo, a_stilt);
}

static void
stilt_p_name_accept(cw_stilt_t *a_stilt)
{
	cw_stilo_t	*stilo;

	switch (a_stilt->meta.name.action) {
	case EXEC:
		if (a_stilt->defer_count == 0) {
			/*
			 * Execute the value associated with the name in the
			 * dictionary stack.
			 */
			/* XXX Actually search dictionary stack. */
			cw_stilo_t	key, *systemdict;

			stilo_name_new(&key, a_stilt, a_stilt->tok_str,
			    a_stilt->index, FALSE);
			systemdict = stil_systemdict_get(a_stilt->stil);

			stilo = stils_push(&a_stilt->exec_stils);
			if (stilo_dict_lookup(systemdict, a_stilt, &key,
			    stilo)) {
				_cw_error("XXX Undefined name");
			}

			stilt_p_exec(a_stilt);
			break;
		}
		/* Fall through. */
	case LITERAL:
		/* Push the name object onto the data stack. */
		stilo = stils_push(&a_stilt->data_stils);
		stilo_name_new(stilo, a_stilt, a_stilt->tok_str, a_stilt->index,
		    FALSE);
		break;
	case EVAL:
		/*
		 * Find the value associated with the name in the dictionary
		 * stack and push the value onto the data stack.
		 */
		break;
	default:
		_cw_error("Programming error");
	}
}
