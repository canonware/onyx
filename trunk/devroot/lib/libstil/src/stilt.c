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

/*
 * If defined, inline oft-used functions for improved performance (and slightly
 * increased code size).
 */
#define _CW_STILT_INLINE

#ifdef _CW_STILT_INLINE
#define _CW_STILT_GETC(a_i)						\
	((a_stilt->index <= _CW_STIL_BUFC_SIZE)				\
	    ? a_stilt->tok_buffer.str[(a_i)]				\
	    : buf_uint8_get(&a_stilt->tok_buffer.buf, (a_i)))
#else
#define _CW_STILT_GETC(a_i)						\
	stilt_p_getc(a_stilt, a_i)
#endif

#ifdef _CW_STILT_INLINE
#define _CW_STILT_PUTC(a_c)						\
	do {								\
		if (a_stilt->index < _CW_STIL_BUFC_SIZE) {		\
			a_stilt->tok_buffer.str[a_stilt->index] = (a_c);\
			a_stilt->index++;				\
		} else {						\
			if (stilt_p_putc(a_stilt, a_c) == -1) {		\
				retval = -1;				\
				goto RETURN;				\
			}						\
		}							\
	} while (0)
#else
#define _CW_STILT_PUTC(a_c)						\
	do {								\
		if (stilt_p_putc(a_stilt, a_c) == -1) {			\
			retval = -1;					\
			goto RETURN;					\
		}							\
	} while (0)
#endif

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
static void		stilt_p_reset_tok_buffer(cw_stilt_t *a_stilt);

#ifndef _CW_STILT_INLINE
static cw_uint8_t	stilt_p_getc(cw_stilt_t *a_stilt, cw_uint32_t a_index);
#endif
static cw_sint32_t	stilt_p_putc(cw_stilt_t *a_stilt, cw_uint32_t a_c);
static void		stilt_p_print_token(cw_stilt_t *a_stilt, cw_uint32_t
    a_length, const char *a_note);
static void		stilt_p_print_syntax_error(cw_stilt_t *a_stilt,
    cw_uint8_t a_c);
static void		*stilt_p_entry(void *a_arg);
static void		stilt_p_procedure_accept(cw_stilt_t *a_stilt);

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

	if (stilnt_new(&retval->stilnt, stilat_mem_get(&retval->stilat),
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

	retval->stdout_fd = 1;
	retval->stil = a_stil;
	retval->state = _CW_STILT_STATE_START;
	retval->line = 1;
#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILT_MAGIC;
#endif

	return retval;
	OOM_6:
	stils_delete(&retval->data_stils);
	OOM_5:
	stils_delete(&retval->exec_stils);
	OOM_4:
	stilnt_delete(&retval->stilnt);
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

	stils_delete(&a_stilt->dict_stils);
	stils_delete(&a_stilt->data_stils);
	stils_delete(&a_stilt->exec_stils);
	stilnt_delete(&a_stilt->stilnt);
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

const cw_stiln_t *
stiltn_ref(cw_stilt_t *a_stilt, const cw_uint8_t *a_name, cw_uint32_t a_len,
    cw_bool_t a_force)
{
	return NULL;	/* XXX */
}

void
stiltn_unref(cw_stilt_t *a_stilt, const cw_stiln_t *a_stiln)
{
	/* XXX */
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
		_CW_STILS_PSTATE(_CW_STILT_STATE_START);
		_CW_STILS_PSTATE(_CW_STILT_STATE_LT_CONT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_GT_CONT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_SLASH_CONT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_COMMENT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_NUMBER);
		_CW_STILS_PSTATE(_CW_STILT_STATE_ASCII_STRING);
		_CW_STILS_PSTATE(_CW_STILT_STATE_ASCII_STRING_NEWLINE_CONT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_ASCII_STRING_CRLF_CONT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_ASCII_STRING_PROT_CONT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_ASCII_STRING_HEX_CONT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_ASCII_STRING_HEX_FINISH);
		_CW_STILS_PSTATE(_CW_STILT_STATE_HEX_STRING);
		_CW_STILS_PSTATE(_CW_STILT_STATE_BASE85_STRING);
		_CW_STILS_PSTATE(_CW_STILT_STATE_BASE85_STRING_CONT);
		_CW_STILS_PSTATE(_CW_STILT_STATE_NAME);
#undef _CW_STILS_PSTATE
#endif

		/*
		 * If a special character causes the end of the previous token,
		 * the state machine builds the object, then restarts the state
		 * machine without incrementing the input character index.  This
		 * is done in order to avoid having to duplicate the
		 * _CW_STILT_STATE_START code.
		 */
		RESTART:

		switch (a_stilt->state) {
		case _CW_STILT_STATE_START:
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
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				break;
			case '`':
				a_stilt->state = _CW_STILT_STATE_LIT_STRING;
				break;
			case '<':
				a_stilt->state = _CW_STILT_STATE_LT_CONT;
				break;
			case '>':
				a_stilt->state = _CW_STILT_STATE_GT_CONT;
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
				stilo = stils_push(&a_stilt->data_stils,
				    a_stilt, _CW_STILOT_NOTYPE);
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
				a_stilt->state = _CW_STILT_STATE_SLASH_CONT;
				break;
			case '%':
				a_stilt->state = _CW_STILT_STATE_COMMENT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* Swallow. */
				break;
			case '+':
				a_stilt->state = _CW_STILT_STATE_NUMBER;
				a_stilt->meta.number.sign =
				    _CW_STILS_NUMBER_POS;
				a_stilt->meta.number.base = 10;
				a_stilt->meta.number.point_offset = -1;
				a_stilt->meta.number.begin_offset = 1;
				_CW_STILT_PUTC(c);
				break;
			case '-':
				a_stilt->state = _CW_STILT_STATE_NUMBER;
				a_stilt->meta.number.sign =
				    _CW_STILS_NUMBER_NEG;
				a_stilt->meta.number.base = 10;
				a_stilt->meta.number.point_offset = -1;
				a_stilt->meta.number.begin_offset = 1;
				_CW_STILT_PUTC(c);
				break;
			case '.':
				a_stilt->state = _CW_STILT_STATE_NUMBER;
				a_stilt->meta.number.sign =
				    _CW_STILS_NUMBER_POS;
				a_stilt->meta.number.base = 10;
				a_stilt->meta.number.point_offset = 0;
				a_stilt->meta.number.begin_offset = 0;
				_CW_STILT_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				a_stilt->state = _CW_STILT_STATE_NUMBER;
				a_stilt->meta.number.sign =
				    _CW_STILS_NUMBER_POS;
				a_stilt->meta.number.base = 10;
				a_stilt->meta.number.point_offset = -1;
				a_stilt->meta.number.begin_offset = 0;
				_CW_STILT_PUTC(c);
				break;
			default:
				a_stilt->state = _CW_STILT_STATE_NAME;
				a_stilt->meta.name.is_literal = FALSE;
				a_stilt->meta.name.is_immediate = FALSE;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case _CW_STILT_STATE_LT_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '<':
				a_stilt->state = _CW_STILT_STATE_START;
				stilt_p_print_token(a_stilt, 0, "<<");
				break;
			case '>':
				a_stilt->state = _CW_STILT_STATE_START;
				stilt_p_print_token(a_stilt, a_stilt->index,
				    "empty hex string");
				stilt_p_reset_tok_buffer(a_stilt);
				break;
			case '~':
				a_stilt->state = _CW_STILT_STATE_BASE85_STRING;
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F':
				/* To lower case. */
				c += 32;
				/* Fall through. */
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
				a_stilt->state = _CW_STILT_STATE_HEX_STRING;
				_CW_STILT_PUTC(c);
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* Whitespace within a hex string. */
				a_stilt->state = _CW_STILT_STATE_HEX_STRING;
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case _CW_STILT_STATE_GT_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '>':
				a_stilt->state = _CW_STILT_STATE_START;
				stilt_p_print_token(a_stilt, 0, ">>");
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case _CW_STILT_STATE_SLASH_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '/':
				a_stilt->state = _CW_STILT_STATE_NAME;
				a_stilt->meta.name.is_literal = FALSE;
				a_stilt->meta.name.is_immediate = TRUE;
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
				a_stilt->state = _CW_STILT_STATE_NAME;
				a_stilt->meta.name.is_literal = TRUE;
				a_stilt->meta.name.is_immediate = FALSE;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case _CW_STILT_STATE_COMMENT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\r':
				a_stilt->state = _CW_STILT_STATE_START;
				break;
			default:
				break;
			}
			break;
		case _CW_STILT_STATE_NUMBER:
			switch (c) {
			case '.':
				if (a_stilt->meta.number.point_offset == -1) {
					a_stilt->meta.number.point_offset =
					    a_stilt->index;
				} else {
					a_stilt->state = _CW_STILT_STATE_NAME;
					a_stilt->meta.name.is_literal = FALSE;
					a_stilt->meta.name.is_immediate = FALSE;
				}
				_CW_STILT_PUTC(c);
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O':
			case 'P': case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X': case 'Y':
			case 'Z':
				/*
				 * We can't change the case of letters, since we
				 * may later discover that this token is
				 * actually a name.  So, the number acceptor
				 * needs to deal with changing the case of
				 * letters.
				 */
				if (a_stilt->meta.number.base <= (10 +
				    ((cw_uint32_t)(c - 'A')))) {
					/* Too big for this base. */
					a_stilt->state = _CW_STILT_STATE_NAME;
					a_stilt->meta.name.is_literal = FALSE;
					a_stilt->meta.name.is_immediate = FALSE;
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
					a_stilt->state = _CW_STILT_STATE_NAME;
					a_stilt->meta.name.is_literal = FALSE;
					a_stilt->meta.name.is_immediate = FALSE;
				}
				_CW_STILT_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (a_stilt->meta.number.base <=
				    ((cw_uint32_t)(c - '0'))) {
					/* Too big for this base. */
					a_stilt->state = _CW_STILT_STATE_NAME;
					a_stilt->meta.name.is_literal = FALSE;
					a_stilt->meta.name.is_immediate = FALSE;
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
					a_stilt->state = _CW_STILT_STATE_NAME;
					a_stilt->meta.name.is_literal = FALSE;
					a_stilt->meta.name.is_immediate = FALSE;
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
						    _CW_STILT_STATE_NAME;
						a_stilt->meta.name.is_literal =
						    FALSE;
						a_stilt->meta.name.is_immediate
						    = FALSE;
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
				a_stilt->state = _CW_STILT_STATE_START;
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
					    a_stilt->index, "name");
				}
				stilt_p_reset_tok_buffer(a_stilt);
				goto RESTART;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				a_stilt->state = _CW_STILT_STATE_START;
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
					    a_stilt->index, "name");
				}
				stilt_p_reset_tok_buffer(a_stilt);
				break;
			default:
				/* Not a number character. */
				a_stilt->state = _CW_STILT_STATE_NAME;
				a_stilt->meta.name.is_literal = FALSE;
				a_stilt->meta.name.is_immediate = FALSE;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case _CW_STILT_STATE_ASCII_STRING:
			/* The CRLF code jumps here if there was no LF. */
			ASCII_STRING_CONTINUE:

			switch (c) {
			case '\\':
				a_stilt->state =
				    _CW_STILT_STATE_ASCII_STRING_PROT_CONT;
				break;
			case '"':
				a_stilt->state = _CW_STILT_STATE_START;

				stilt_p_print_token(a_stilt,
				    a_stilt->index, "string");
				stilo = stils_push(&a_stilt->data_stils,
				    a_stilt, _CW_STILOT_STRINGTYPE,
				    a_stilt->index);
				if (a_stilt->index <=
				    _CW_STIL_BUFC_SIZE) {
					stilo_string_set(stilo, 0,
					    a_stilt->tok_buffer.str,
					    a_stilt->index);
				} else
					_cw_error("XXX Unimplemented");

				stilt_p_reset_tok_buffer(a_stilt);
				break;
			case '\r':
				a_stilt->state =
				    _CW_STILT_STATE_ASCII_STRING_NEWLINE_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			default:
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case _CW_STILT_STATE_ASCII_STRING_NEWLINE_CONT:
			/* All cases in the switch statement do this. */
			_CW_STILT_PUTC('\n');
			a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
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
		case _CW_STILT_STATE_ASCII_STRING_PROT_CONT:
			switch (c) {
			case 'n':
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				_CW_STILT_PUTC('\n');
				break;
			case 'r':
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				_CW_STILT_PUTC('\r');
				break;
			case 't':
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				_CW_STILT_PUTC('\t');
				break;
			case 'b':
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				_CW_STILT_PUTC('\b');
				break;
			case 'f':
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				_CW_STILT_PUTC('\f');
				break;
			case '\\':
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				_CW_STILT_PUTC('\\');
				break;
			case '"':
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				_CW_STILT_PUTC('"');
				break;
			case 'x':
				a_stilt->state =
				    _CW_STILT_STATE_ASCII_STRING_HEX_CONT;
				break;
			case '\r':
				a_stilt->state =
				    _CW_STILT_STATE_ASCII_STRING_CRLF_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();

				/* Ignore. */
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				break;
			default:
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				_CW_STILT_PUTC('\\');
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case _CW_STILT_STATE_ASCII_STRING_CRLF_CONT:
			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();

				/* Ignore. */
				a_stilt->state = _CW_STILT_STATE_ASCII_STRING;
				break;
			default:
				goto ASCII_STRING_CONTINUE;
			}
			break;
		case _CW_STILT_STATE_ASCII_STRING_HEX_CONT:
			switch (c) {
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F':
				/* To lower case. */
				c += 32;
				/* Fall through. */
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
				a_stilt->state =
				    _CW_STILT_STATE_ASCII_STRING_HEX_FINISH;
				a_stilt->meta.string.hex_val = c;
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case _CW_STILT_STATE_ASCII_STRING_HEX_FINISH:
			switch (c) {
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F':
				/* To lower case. */
				c += 32;
				/* Fall through. */
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':{
				cw_uint8_t	val;

				a_stilt->state =
				    _CW_STILT_STATE_ASCII_STRING;
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
		case _CW_STILT_STATE_LIT_STRING:
			/* The CRLF code jumps here if there was no LF. */
			LIT_STRING_CONTINUE:

			switch (c) {
			case '\'':
				a_stilt->state =
				    _CW_STILT_STATE_LIT_STRING_PROT_CONT;
				break;
			case '\r':
				a_stilt->state =
				    _CW_STILT_STATE_LIT_STRING_NEWLINE_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			default:
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case _CW_STILT_STATE_LIT_STRING_NEWLINE_CONT:
			/* All cases in the switch statement do this. */
			_CW_STILT_PUTC('\n');
			a_stilt->state = _CW_STILT_STATE_LIT_STRING;
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
		case _CW_STILT_STATE_LIT_STRING_PROT_CONT:
			switch (c) {
			case '\'':
				a_stilt->state = _CW_STILT_STATE_LIT_STRING;
				_CW_STILT_PUTC('\'');
				break;
			default:
				/* Accept literal string. */
				a_stilt->state = _CW_STILT_STATE_START;

				stilt_p_print_token(a_stilt,
				    a_stilt->index, "literal string");
				stilo = stils_push(&a_stilt->data_stils,
				    a_stilt, _CW_STILOT_STRINGTYPE,
				    a_stilt->index);
				if (a_stilt->index <=
				    _CW_STIL_BUFC_SIZE) {
					stilo_string_set(stilo, 0,
					    a_stilt->tok_buffer.str,
					    a_stilt->index);
				} else
					_cw_error("XXX Unimplemented");

				stilt_p_reset_tok_buffer(a_stilt);

				/*
				 * We're currently looking at the first
				 * character of the next token, so re-scan it.
				 */
				goto START_CONTINUE;
			}
			break;
		case _CW_STILT_STATE_HEX_STRING:
			switch (c) {
			case '>':
				a_stilt->state = _CW_STILT_STATE_START;
				stilt_p_print_token(a_stilt, a_stilt->index,
				    "hex string");
				stilt_p_reset_tok_buffer(a_stilt);
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F':
				/* To lower case. */
				c += 32;
				/* Fall through. */
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
		case _CW_STILT_STATE_BASE85_STRING:
			switch (c) {
			case '~':
				a_stilt->state =
				    _CW_STILT_STATE_BASE85_STRING_CONT;
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
		case _CW_STILT_STATE_BASE85_STRING_CONT:
			switch (c) {
			case '>':
				a_stilt->state = _CW_STILT_STATE_START;
				stilt_p_print_token(a_stilt, a_stilt->index,
				    "base 85 string");
				stilt_p_reset_tok_buffer(a_stilt);
				break;
			default:
				stilt_p_print_syntax_error(a_stilt, c);
				break;
			}
			break;
		case _CW_STILT_STATE_NAME:
			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* End of name. */
				a_stilt->state = _CW_STILT_STATE_START;
				if (a_stilt->index > 0) {
					stilt_p_print_token(a_stilt,
					    a_stilt->index, "name");
				} else
					stilt_p_print_syntax_error(a_stilt, c);
				stilt_p_reset_tok_buffer(a_stilt);
				break;
			case '"': case '`': case '<': case '>': case '[':
			case ']': case '{': case '}': case '/': case '%':
				/* New token. */
				a_stilt->state = _CW_STILT_STATE_START;
				if (a_stilt->index > 0) {
					stilt_p_print_token(a_stilt,
					    a_stilt->index, "name");
				} else
					stilt_p_print_syntax_error(a_stilt, c);
				stilt_p_reset_tok_buffer(a_stilt);
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

static void
stilt_p_reset_tok_buffer(cw_stilt_t *a_stilt)
{
	if (a_stilt->index > _CW_STIL_BUFC_SIZE)
		buf_delete(&a_stilt->tok_buffer.buf);
	a_stilt->index = 0;
}

#ifndef _CW_STILT_INLINE
static cw_uint8_t
stilt_p_getc(cw_stilt_t *a_stilt, cw_uint32_t a_index)
{
	cw_uint8_t	retval;

	if (a_stilt->index < _CW_STIL_BUFC_SIZE)
		retval = a_stilt->tok_buffer.str[a_index];
	else
		retval = buf_get_uint8(&a_stilt->tok_buffer.buf, a_index);

	return retval;
}
#endif

static cw_sint32_t
stilt_p_putc(cw_stilt_t *a_stilt, cw_uint32_t a_c)
{
	cw_sint32_t	retval;

#ifndef _CW_STILT_INLINE
	if (a_stilt->index < _CW_STIL_BUFC_SIZE)
		a_stilt->tok_buffer.str[a_stilt->index] = a_c;
	else
#endif
	{
		if (a_stilt->index == _CW_STIL_BUFC_SIZE) {
			cw_stil_bufc_t	*kbufc;

			kbufc = stil_stil_bufc_get(a_stilt->stil);
			memcpy(kbufc->buffer, a_stilt->tok_buffer.str,
			    _CW_STIL_BUFC_SIZE);
			buf_new(&a_stilt->tok_buffer.buf,
			    stilt_mem_get(a_stilt));
			if (buf_bufc_append(&a_stilt->tok_buffer.buf,
			    &kbufc->bufc, 0, _CW_STIL_BUFC_SIZE)) {
				bufc_delete(&kbufc->bufc);
				retval = -1;
				goto RETURN;
			}
			bufc_delete(&kbufc->bufc);
		}
		if (buf_size_get(&a_stilt->tok_buffer.buf) == a_stilt->index) {
			cw_stil_bufc_t	*kbufc;

			kbufc = stil_stil_bufc_get(a_stilt->stil);
			if (buf_bufc_append(&a_stilt->tok_buffer.buf,
			    &kbufc->bufc, 0, _CW_STIL_BUFC_SIZE)) {
				bufc_delete(&kbufc->bufc);
				retval = -1;
				goto RETURN;
			}
			bufc_delete(&kbufc->bufc);
		}
		buf_uint8_set(&a_stilt->tok_buffer.buf, a_stilt->index, a_c);
	}
	a_stilt->index++;

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
	if (a_stilt->index <= _CW_STIL_BUFC_SIZE)
		_cw_out_put_n(a_length, "[s]", a_stilt->tok_buffer.str);
	else
		_cw_out_put_n(a_length, "[b]", &a_stilt->tok_buffer.buf);
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
	if (a_stilt->index <= _CW_STIL_BUFC_SIZE)
		_cw_out_put_n(a_stilt->index, "[s]", a_stilt->tok_buffer.str);
	else
		_cw_out_put_n(a_stilt->index, "[b]", &a_stilt->tok_buffer.buf);
	_cw_out_put("<-- (starts at line [i], column [i])\n",
	    a_stilt->tok_line, a_stilt->tok_column);
	a_stilt->state = _CW_STILT_STATE_START;
	stilt_p_reset_tok_buffer(a_stilt);
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

	stilo_new(&t_stilo, a_stilt, _CW_STILOT_ARRAYTYPE, nelements);
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
	stils_pop(&a_stilt->data_stils, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(&a_stilt->data_stils, a_stilt, _CW_STILOT_NOTYPE);
	stilo_move(stilo, &t_stilo);

	/* Clean up. */
	stilo_delete(&t_stilo);
}
