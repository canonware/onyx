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
#include <errno.h>
#include <limits.h>
#include <math.h>	/* For HUGE_VAL, though this is probably an OS bug. */

/*  #define	_CW_STILT_SCANNER_DEBUG */

/* Initial size of userdict. */
#define	_CW_STILT_USERDICT_SIZE	 64

#define _CW_STILT_GETC(a_i)						\
	a_stilt->tok_str[(a_i)]

#define _CW_STILT_PUTC(a_c)						\
	do {								\
		if (a_stilt->index >= _CW_STILT_BUFFER_SIZE)		\
			stilt_p_tok_str_expand(a_stilt);		\
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
		a_stilts->line++;					\
		a_stilts->column = -1;				\
	} while (0)

struct cw_stilt_entry_s {
	cw_thd_t	thd;
	cw_stilt_t	*stilt;
	cw_stilts_t	*stilts;
	const cw_uint8_t *str;
	cw_uint32_t	len;
};

/*
 * Lookup table for base64 decoding.
 */
static cw_sint32_t cw_g_b64_codes[] = {
		    62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, -1, -1, -1, -1, -1, -1,

	-1,  0,  1,  2,  3,  4 , 5,  6,
	 7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51
};
#define	stilt_p_b64b(a) cw_g_b64_codes[(a) - 43]
	
static void		stilt_p_feed(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
static void		stilt_p_tok_str_expand(cw_stilt_t *a_stilt);
static void		stilt_p_reset(cw_stilt_t *a_stilt);
#ifdef _CW_STILT_SCANNER_DEBUG
static void		stilt_p_token_print(cw_stilt_t *a_stilt, cw_stilts_t
    *a_stilts, cw_uint32_t a_length, const cw_uint8_t *a_note);
#else
#define	stilt_p_token_print(a, b, c, d)
#endif
static void		stilt_p_syntax_error_print(cw_stilt_t *a_stilt,
    cw_uint8_t a_c);
static void		*stilt_p_entry(void *a_arg);
static void		stilt_p_special_accept(cw_stilt_t *a_stilt, const
    cw_uint8_t *a_token, cw_uint32_t a_len);
static void		stilt_p_procedure_accept(cw_stilt_t *a_stilt);
static void		stilt_p_name_accept(cw_stilt_t *a_stilt, cw_stilts_t
    *a_stilts);

#ifdef _LIBSTIL_DBG
#define _CW_STILT_MAGIC 0x978fdbe0
#define _CW_STILTS_MAGIC 0xdfe76a68
#endif

/*
 * Size and fullness control of initial thread-specific name cache hash table.
 * This starts out empty, but should be expected to grow rather quickly to start
 * with.
 */
#define _CW_STILT_NAME_BASE_TABLE	512
#define _CW_STILT_NAME_BASE_GROW	400
#define _CW_STILT_NAME_BASE_SHRINK	128

/*
 * Size and fullness control of initial root set for local VM.  Local VM is
 * empty to begin with.
 */
#define _CW_STILT_ROOTS_BASE_TABLE	 32
#define _CW_STILT_ROOTS_BASE_GROW	 24
#define _CW_STILT_ROOTS_BASE_SHRINK	  8

/*
 * Size of buffer to use when executing file objects.  This generally doesn't
 * need to be huge, because there is usually additional buffering going on
 * upstream.
 */
#define	_CW_STILT_FILE_READ_SIZE	128

/*
 * stilts.
 */
cw_stilts_t *
stilts_new(cw_stilts_t *a_stilts, cw_stilt_t *a_stilt)
{
	cw_stilts_t	*retval;

	if (a_stilts != NULL) {
		retval = a_stilts;
/*  		memset(a_stilts, 0, sizeof(cw_stilts_t)); */
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_stilts_t *)stilt_malloc(a_stilt,
		    sizeof(cw_stilts_t));
/*  		memset(a_stilts, 0, sizeof(cw_stilts_t)); */
		retval->is_malloced = TRUE;
	}

	retval->line = 1;
	retval->column = 0;

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILTS_MAGIC;
#endif
	return retval;
}

void
stilts_delete(cw_stilts_t *a_stilts, cw_stilt_t *a_stilt)
{
	if (a_stilt->state != STILTTS_START) {
		/*
		 * It's possible that the last token seen hasn't been accepted
		 * yet.  Reset the internal state so that this won't screw
		 * things up later.
		 */
		stilt_p_reset(a_stilt);
	}

	if (a_stilts->is_malloced)
		stilt_free(a_stilt, a_stilts);
#ifdef _LIBSTIL_DBG
	else
		memset(a_stilts, 0x5a, sizeof(cw_stilts_t));
#endif
}

void
stilts_position_get(cw_stilts_t *a_stilts, cw_uint32_t *r_line, cw_uint32_t
    *r_column)
{
	_cw_check_ptr(a_stilts);
	_cw_assert(a_stilts->magic == _CW_STILTS_MAGIC);

	*r_line = a_stilts->line;
	*r_column = a_stilts->column;
}

void
stilts_position_set(cw_stilts_t *a_stilts, cw_uint32_t a_line, cw_uint32_t
    a_column)
{
	_cw_check_ptr(a_stilts);
	_cw_assert(a_stilts->magic == _CW_STILTS_MAGIC);

	a_stilts->line = a_line;
	a_stilts->column = a_column;
}

/*
 * stilt.
 */
cw_stilt_t *
stilt_new(cw_stilt_t *a_stilt, cw_stil_t *a_stil)
{
	cw_stilt_t		*retval;
	cw_stilo_t		*stilo;
	volatile cw_uint32_t	try_stage = 0;

	xep_begin();
	volatile cw_stilt_t	*v_retval;
	xep_try {
		if (a_stilt != NULL) {
			v_retval = retval = a_stilt;
			memset(a_stilt, 0, sizeof(cw_stilt_t));
			retval->is_malloced = FALSE;
		} else {
			v_retval = retval = (cw_stilt_t
			    *)_cw_malloc(sizeof(cw_stilt_t));
			memset(a_stilt, 0, sizeof(cw_stilt_t));
			retval->is_malloced = TRUE;
		}
		retval->stil = a_stil;
		retval->tok_str = retval->buffer;
		try_stage = 1;

		stilat_new(&retval->stilat, a_stilt, stil_stilag_get(a_stil));
		try_stage = 2;

		dch_new(&retval->name_hash, stilat_mem_get(&retval->stilat),
		    _CW_STILT_NAME_BASE_TABLE, _CW_STILT_NAME_BASE_GROW,
		    _CW_STILT_NAME_BASE_SHRINK, stilo_name_hash,
		    stilo_name_key_comp);
		try_stage = 3;

		stils_new(&retval->estack,
		    stilat_stilsc_pool_get(&a_stilt->stilat));
		try_stage = 4;

		stils_new(&retval->ostack,
		    stilat_stilsc_pool_get(&a_stilt->stilat));
		try_stage = 5;

		stils_new(&retval->dstack,
		    stilat_stilsc_pool_get(&a_stilt->stilat));
		try_stage = 6;

		stils_new(&retval->tstack,
		    stilat_stilsc_pool_get(&a_stilt->stilat));
		try_stage = 7;

		/* Push systemdict onto the dictionary stack. */
		stilo = stils_push(&retval->dstack, retval);
		stilo_dup(stilo, stil_systemdict_get(a_stil));

		/* Push globaldict onto the dictionary stack. */
		stilo = stils_push(&retval->dstack, retval);
		stilo_dup(stilo, stil_globaldict_get(a_stil));

		/* Create and push userdict onto the dictionary stack. */
		stilo_dict_new(&retval->userdict, retval,
		    _CW_STILT_USERDICT_SIZE);
		stilo = stils_push(&retval->dstack, retval);
		stilo_dup(stilo, &retval->userdict);

		/* Create errordict. */
		errordict_populate(&retval->errordict, retval);
	}
	xep_catch (_CW_XEPV_OOM) {
		retval = (cw_stilt_t *)v_retval;
		switch (try_stage) {
		case 7:
			stils_delete(&retval->tstack, retval);
		case 6:
			stils_delete(&retval->dstack, retval);
		case 5:
			stils_delete(&retval->ostack, retval);
		case 4:
			stils_delete(&retval->estack, retval);
		case 3:
			dch_delete(&retval->name_hash);
		case 2:
			stilat_delete(&retval->stilat);
		case 1:
			if (retval->is_malloced)
				_cw_free(retval);
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILT_MAGIC;
#endif

	return retval;
}

void
stilt_delete(cw_stilt_t *a_stilt)
{
	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	if (a_stilt->tok_str != a_stilt->buffer) {
		/*
		 * This shouldn't happen, since it indicates that there is an
		 * unaccepted token.  However, it's really the caller's fault,
		 * so just clean up and return an error.
		 */
		stilt_free(a_stilt, a_stilt->tok_str);
	}

	stils_delete(&a_stilt->tstack, a_stilt);
	stils_delete(&a_stilt->dstack, a_stilt);
	stils_delete(&a_stilt->ostack, a_stilt);
	stils_delete(&a_stilt->estack, a_stilt);
	dch_delete(&a_stilt->name_hash);
	stilat_delete(&a_stilt->stilat);
	if (a_stilt->is_malloced)
		_cw_free(a_stilt);
#ifdef _LIBSTIL_DBG
	else
		memset(a_stilt, 0x5a, sizeof(cw_stilt_t));
#endif
}

void
stilt_loop(cw_stilt_t *a_stilt)
{
	cw_stilo_t	*stilo, *tstilo;
	cw_uint32_t	sdepth;

	for (sdepth = stils_count(&a_stilt->estack);
	     stils_count(&a_stilt->estack) >= sdepth;) {
		stilo = stils_get(&a_stilt->estack, a_stilt);
		if (stilo_attrs_get(stilo) == STILOA_LITERAL) {
			/* Always push literal objects onto the data stack. */
			tstilo = stils_push(&a_stilt->ostack, a_stilt);
			stilo_dup(tstilo, stilo);
			stils_pop(&a_stilt->estack, a_stilt);
			continue;
		}

		switch (stilo_type_get(stilo)) {
		case STILOT_BOOLEAN:
		case STILOT_CONDITION:
		case STILOT_DICT:
		case STILOT_INTEGER:
		case STILOT_MARK:
			/*
			 * Always push the object onto the data stack, even
			 * though it isn't literal.
			 */
			tstilo = stils_push(&a_stilt->ostack, a_stilt);
			stilo_dup(tstilo, stilo);
			stils_pop(&a_stilt->estack, a_stilt);
			break;
		case STILOT_NULL:
			/* Do nothing. */
			stils_pop(&a_stilt->estack, a_stilt);
			break;
		case STILOT_ARRAY: {
			cw_uint32_t	i, len;
			cw_stilo_t	*array;

			len = stilo_array_len_get(stilo);
			if (len == 0) {
				stils_pop(&a_stilt->estack, a_stilt);
				break;
			}

			/*
			 * Iterate through the array and execute each element in
			 * turn.  The generic algorithm is encapsulated in the
			 * last part of the if..else if..else statement, but the
			 * overhead of the pushing, recursion, and popping is
			 * excessive for the common cases of a simple object or
			 * operator.  Therefore, check for the most common
			 * simple cases and handle them specially.
			 */
			array = stilo_array_get(stilo);
			for (i = 0; i < len - 1; i++) {
				if (stilo_attrs_get(&array[i]) ==
				    STILOA_LITERAL) {
					/*
					 * Always push literal objects onto the
                                         * data stack.
					 */
					tstilo =
					    stils_push(&a_stilt->ostack,
					    a_stilt);
					stilo_dup(tstilo, &array[i]);
				} else if (stilo_type_get(&array[i]) ==
				    STILOT_OPERATOR) {
					/* Operator.  Execute it directly. */
					array[i].o.operator.f(a_stilt);
				} else {
					/*
					 * Not a simple common case, so use the
					 * generic algorithm.
					 */
					tstilo =
					    stils_push(&a_stilt->estack,
					    a_stilt);
					stilo_dup(tstilo, &array[i]);
					stilt_loop(a_stilt);
				}
			}

			/*
			 * If recursion is possible and likely, make tail
			 * recursion safe by replacing the array with its last
			 * element before executing the last element.
			 */
			if (stilo_attrs_get(&array[i]) == STILOA_LITERAL) {
				/*
				 * Always push literal objects onto the
				 * data stack.
				 */
				tstilo = stils_push(&a_stilt->ostack, a_stilt);
				stilo_dup(tstilo, &array[i]);
				stils_pop(&a_stilt->estack, a_stilt);
			} else {
				/*
				 * Possible recursion, so use the generic
				 * algorithm.
				 */
				/* XXX GC-unsafe. */
				stils_pop(&a_stilt->estack, a_stilt);
				tstilo = stils_push(&a_stilt->estack, a_stilt);
				stilo_dup(tstilo, &array[i]);
			}
			break;
		}
		case STILOT_STRING: {
			cw_stilts_t	stilts;

			/*
			 * Use the string as a source of code.
			 */
			stilts_new(&stilts, a_stilt);
			stilt_interpret(a_stilt, &stilts,
			    stilo_string_get(stilo),
			    stilo_string_len_get(stilo));
			stilt_flush(a_stilt, &stilts);
			stilts_delete(&stilts, a_stilt);
			stils_pop(&a_stilt->estack, a_stilt);

			break;
		}
		case STILOT_NAME: {
			cw_stilo_t	val;

			/*
			 * Search for a value associated with the name
			 * in the dictionary stack, push it onto the
			 * execution stack, and execute it.
			 */
			stilo = stils_get(&a_stilt->estack, a_stilt);
			stilo_no_new(&val);
			if (stilt_dict_stack_search(a_stilt, stilo, &val))
				stilt_error(a_stilt, STILTE_UNDEFINED);
			stilo_move(stilo, &val);
			break;
		}
		case STILOT_OPERATOR:
			stilo->o.operator.f(a_stilt);
			stils_pop(&a_stilt->estack, a_stilt);
			break;
		case STILOT_FILE: {
			cw_stilts_t	stilts;
			cw_sint32_t	nread;
			cw_uint8_t	buffer[_CW_STILT_FILE_READ_SIZE];

			stilts_new(&stilts, a_stilt);
			/*
			 * Read data from the file and interpret it until an EOF
			 * (0 byte read).
			 */
			for (nread = stilo_file_read(stilo, a_stilt,
			    _CW_STILT_FILE_READ_SIZE, buffer); nread > 0;
			    nread = stilo_file_read(stilo, a_stilt,
			    _CW_STILT_FILE_READ_SIZE, buffer)) {
				stilt_interpret(a_stilt, &stilts, buffer,
				    nread);
			}
			stilt_flush(a_stilt, &stilts);
			stilts_delete(&stilts, a_stilt);

			stils_pop(&a_stilt->estack, a_stilt);
			break;
		}
		case STILOT_HOOK:
		case STILOT_LOCK:
			_cw_not_reached();	/* XXX */
		default:
			_cw_not_reached();
		}
	}
}

void
stilt_interpret(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts, const cw_uint8_t
    *a_str, cw_uint32_t a_len)
{
	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	stilt_p_feed(a_stilt, a_stilts, a_str, a_len);
}

void
stilt_flush(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts)
{
	cw_uint8_t	str[2] = "\n";

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	stilt_p_feed(a_stilt, a_stilts, str, sizeof(str) - 1);
}

void
stilt_detach(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts, const cw_uint8_t
    *a_str, cw_uint32_t a_len)
{
	struct cw_stilt_entry_s	*entry_arg;

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);
	_cw_check_ptr(a_str);

	entry_arg = (struct cw_stilt_entry_s *)_cw_malloc(sizeof(struct
	    cw_stilt_entry_s));
	entry_arg->stilt = a_stilt;
	entry_arg->stilts = a_stilts;
	entry_arg->str = a_str;
	entry_arg->len = a_len;

	stilt_p_entry((void *)entry_arg);
}

void
stilt_error(cw_stilt_t *a_stilt, cw_stilte_t a_error)
{
	cw_stilo_t	*stilo, *errordict, *key, *handler;
	cw_bool_t	ostack_push = TRUE;

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	/*
	 * Get errordict.  We can't throw an undefined error here because it
	 * would go infinitely recursive.
	 */
	key = stils_push(&a_stilt->tstack, a_stilt);
	{
		cw_uint8_t	keystr[] = "errordict";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
	}
	if (stilt_dict_stack_search(a_stilt, key, errordict)) {
		stils_pop(&a_stilt->tstack, a_stilt);
		xep_throw(_CW_STILX_ERRORDICT);
	}

	/*
	 * Find handler corresponding to error.
	 */
	switch (a_error) {
	case STILTE_DICTSTACKOVERFLOW: {
		cw_uint8_t	keystr[] = "dictstackoverflow";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_DICTSTACKUNDERFLOW: {
		cw_uint8_t	keystr[] = "dictstackunderflow";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_EXECSTACKOVERFLOW: {
		cw_uint8_t	keystr[] = "execstackoverflow";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_INTERRUPT: {
		cw_uint8_t	keystr[] = "interrupt";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		ostack_push = FALSE;
		break;
	}
	case STILTE_INVALIDACCESS: {
		cw_uint8_t	keystr[] = "invalidaccess";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_INVALIDCONTEXT: {
		cw_uint8_t	keystr[] = "invalidcontext";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_INVALIDEXIT: {
		cw_uint8_t	keystr[] = "invalidexit";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_INVALIDFILEACCESS: {
		cw_uint8_t	keystr[] = "invalidfileaccess";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_IOERROR: {
		cw_uint8_t	keystr[] = "ioerror";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_LIMITCHECK: {
		cw_uint8_t	keystr[] = "limitcheck";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_RANGECHECK: {
		cw_uint8_t	keystr[] = "rangecheck";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_STACKOVERFLOW: {
		cw_uint8_t	keystr[] = "stackoverflow";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_STACKUNDERFLOW: {
		cw_uint8_t	keystr[] = "stackunderflow";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_SYNTAXERROR: {
		cw_uint8_t	keystr[] = "syntaxerror";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_TIMEOUT: {
		cw_uint8_t	keystr[] = "timeout";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		ostack_push = FALSE;
		break;
	}
	case STILTE_TYPECHECK: {
		cw_uint8_t	keystr[] = "typecheck";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_UNDEFINED: {
		cw_uint8_t	keystr[] = "undefined";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_UNDEFINEDFILENAME: {
		cw_uint8_t	keystr[] = "undefinedfilename";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_UNDEFINEDRESOURCE: {
		cw_uint8_t	keystr[] = "undefinedresource";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_UNDEFINEDRESULT: {
		cw_uint8_t	keystr[] = "undefinedresult";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_UNMATCHEDMARK: {
		cw_uint8_t	keystr[] = "unmatchedmark";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_UNREGISTERED: {
		cw_uint8_t	keystr[] = "unregistered";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	case STILTE_VMERROR: {
		cw_uint8_t	keystr[] = "vmerror";

		stilo_name_new(key, a_stilt, keystr, sizeof(keystr) - 1, TRUE);
		break;
	}
	default:
		_cw_not_reached();
	}

	/*
	 * Push the object being executed onto ostack unless this is an
	 * interrupt or timeout.
	 */
	if (ostack_push) {
		stilo = stils_push(&a_stilt->ostack, a_stilt);
		stilo_dup(stilo, stils_get(&a_stilt->estack, a_stilt));
	}

	/*
	 * Get the handler for this particular error.  We could potentially
	 * throw another error here without going infinitely recursive, but it's
	 * not worth the risk.  After all, the user has done some really hokey
	 * management of errordict if this happens.
	 */

	/* Execute errordict in order to make sure we have the value. */
	stilo = stils_push(&a_stilt->estack, a_stilt);
	stilo_dup(stilo, errordict);
	stilt_loop(a_stilt);

	/* Move errordict from ostack to tstack. */
	errordict = stils_push(&a_stilt->tstack, a_stilt);
	stilo = stils_get(&a_stilt->ostack, a_stilt);
	stilo_dup(errordict, stilo);

	/* Get the handler and push it onto estack. */
	handler = stils_push(&a_stilt->estack, a_stilt);
	if (stilo_dict_lookup(errordict, a_stilt, key, handler)) {
		stils_npop(&a_stilt->tstack, a_stilt, 2);
		stils_pop(&a_stilt->estack, a_stilt);
		xep_throw(_CW_STILX_ERRORDICT);
	}
	stils_npop(&a_stilt->tstack, a_stilt, 2);

	/* Execute the handler. */
	stilt_loop(a_stilt);
}

cw_bool_t
stilt_dict_stack_search(cw_stilt_t *a_stilt, cw_stilo_t *a_key, cw_stilo_t
    *r_value)
{
	cw_bool_t	retval;
	cw_stilo_t	*dict;
	cw_uint32_t	i, depth;

	/*
	 * Iteratively search the dictionaries on the dictionary stack for
	 * a_key.
	 */
	/* XXX We're guaranteed that dstack is >= 3 deep. */
	i = 0;
	depth = stils_count(&a_stilt->dstack);
	if (depth > 0) {
		dict = stils_get(&a_stilt->dstack, a_stilt);
		if (stilo_dict_lookup(dict, a_stilt, a_key, r_value) == FALSE) {
			/* Found. */
			retval = FALSE;
			goto RETURN;
		}

		for (i = 1; i < depth; i++) {
			dict = stils_down_get(&a_stilt->dstack, a_stilt, dict);
			if (stilo_dict_lookup(dict, a_stilt, a_key, r_value) ==
			    FALSE) {
				/* Found. */
				retval = FALSE;
				goto RETURN;
			}
		}
	}

	retval = TRUE;
	RETURN:
	return retval;
}

static void
stilt_p_feed(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts, const cw_uint8_t
    *a_str, cw_uint32_t a_len)
{
	cw_uint32_t	i;
	cw_uint8_t	c;
	cw_stilo_t	*stilo;

	for (i = 0; i < a_len; i++, a_stilts->column++) {
		c = a_str[i];

#ifdef _CW_STILT_SCANNER_DEBUG
#define _CW_STILT_PSTATE(a)						\
	do {								\
		if (a_stilt->state == (a))				\
			_cw_out_put("[s]\n", #a);			\
	} while (0)

		_cw_out_put("c: '[c]' ([i]), index: [i] ", c, c,
		    a_stilt->index);
		_CW_STILT_PSTATE(STILTTS_START);
		_CW_STILT_PSTATE(STILTTS_LT_CONT);
		_CW_STILT_PSTATE(STILTTS_GT_CONT);
		_CW_STILT_PSTATE(STILTTS_SLASH_CONT);
		_CW_STILT_PSTATE(STILTTS_COMMENT);
		_CW_STILT_PSTATE(STILTTS_INTEGER);
		_CW_STILT_PSTATE(STILTTS_INTEGER_RADIX);
		_CW_STILT_PSTATE(STILTTS_ASCII_STRING);
		_CW_STILT_PSTATE(STILTTS_ASCII_STRING_NEWLINE_CONT);
		_CW_STILT_PSTATE(STILTTS_ASCII_STRING_PROT_CONT);
		_CW_STILT_PSTATE(STILTTS_ASCII_STRING_CRLF_CONT);
		_CW_STILT_PSTATE(STILTTS_ASCII_STRING_HEX_CONT);
		_CW_STILT_PSTATE(STILTTS_ASCII_STRING_HEX_FINISH);
		_CW_STILT_PSTATE(STILTTS_LIT_STRING);
		_CW_STILT_PSTATE(STILTTS_LIT_STRING_NEWLINE_CONT);
		_CW_STILT_PSTATE(STILTTS_LIT_STRING_PROT_CONT);
		_CW_STILT_PSTATE(STILTTS_HEX_STRING);
		_CW_STILT_PSTATE(STILTTS_BASE64_STRING);
		_CW_STILT_PSTATE(STILTTS_BASE64_STRING_PAD);
		_CW_STILT_PSTATE(STILTTS_BASE64_STRING_TILDE);
		_CW_STILT_PSTATE(STILTTS_BASE64_STRING_FINISH);
		_CW_STILT_PSTATE(STILTTS_NAME);
#undef _CW_STILT_PSTATE
#endif

		/*
		 * If a special character causes the end of the previous token,
		 * the state machine builds the object, then restarts the state
		 * machine without incrementing the input character index.  This
		 * is done in order to avoid having to duplicate the
		 * STILTTS_START code.
		 */
		RESTART:

		switch (a_stilt->state) {
		case STILTTS_START:
			/*
			 * A literal string cannot be accepted until one
			 * character past the ' at the end has been seen, at
			 * which point the scanner jumps here.
			 */
			START_CONTINUE:
			_cw_assert(a_stilt->index == 0);

			/* Record where this token starts. */
			a_stilt->tok_line = a_stilts->line;
			a_stilt->tok_column = a_stilts->column;

			switch (c) {
			case '"':
				a_stilt->state = STILTTS_ASCII_STRING;
				break;
			case '`':
				a_stilt->state = STILTTS_LIT_STRING;
				break;
			case '<':
				a_stilt->state = STILTTS_LT_CONT;
				break;
			case '>':
				a_stilt->state = STILTTS_GT_CONT;
				break;
			case '[':
				/* An operator, not the same as '{'. */
				stilt_p_token_print(a_stilt, a_stilts, 0, "[");
				stilt_p_special_accept(a_stilt, "[", 1);
				break;
			case ']':
				/* An operator, not the same as '}'. */
				stilt_p_token_print(a_stilt, a_stilts, 0, "]");
				stilt_p_special_accept(a_stilt, "]", 1);
				break;
			case '{':
				stilt_p_token_print(a_stilt, a_stilts, 0, "{");
				a_stilt->defer_count++;
				stilo = stils_push(&a_stilt->ostack, a_stilt);
				stilo_no_new(stilo);
				/*
				 * Leave the stilo as notype in order to
				 * differentiate from normal marks.
				 */
				break;
			case '}':
				stilt_p_token_print(a_stilt, a_stilts, 0, "}");
				if (a_stilt->defer_count > 0)
					a_stilt->defer_count--;
				else {
					/* Missing '{'. */
					stilt_error(a_stilt,
					    STILTE_SYNTAXERROR);
				}
				stilt_p_procedure_accept(a_stilt);
				break;
			case '/':
				a_stilt->state = STILTTS_SLASH_CONT;
				break;
			case '%':
				a_stilt->state = STILTTS_COMMENT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* Swallow. */
				break;
			case '+':
				a_stilt->state = STILTTS_INTEGER;
				a_stilt->m.n.b_off = 1;
				_CW_STILT_PUTC(c);
				break;
			case '-':
				a_stilt->state = STILTTS_INTEGER;
				a_stilt->m.n.b_off = 1;
				_CW_STILT_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				a_stilt->state = STILTTS_INTEGER;
				a_stilt->m.n.b_off = 0;
				_CW_STILT_PUTC(c);
				break;
			default:
				a_stilt->state = STILTTS_NAME;
				a_stilt->m.m.action = ACTION_EXECUTE;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STILTTS_LT_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '<':
				a_stilt->state = STILTTS_START;
				stilt_p_token_print(a_stilt, a_stilts, 0, "<<");
				stilt_p_special_accept(a_stilt, "<<", 2);
				break;
			case '>':
				a_stilt->state = STILTTS_START;
				stilt_p_token_print(a_stilt, a_stilts,
				    a_stilt->index, "empty hex string");
				stilo = stils_push(&a_stilt->ostack, a_stilt);
				stilo_string_new(stilo, a_stilt, 0);
				break;
			case '~':
				a_stilt->state = STILTTS_BASE64_STRING;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
				a_stilt->state = STILTTS_HEX_STRING;
				_CW_STILT_PUTC(c);
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* Whitespace within a hex string. */
				a_stilt->state = STILTTS_HEX_STRING;
				break;
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_GT_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '>':
				a_stilt->state = STILTTS_START;
				stilt_p_token_print(a_stilt, a_stilts, 0, ">>");
				stilt_p_special_accept(a_stilt, ">>", 2);
				break;
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_SLASH_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '/':
				a_stilt->state = STILTTS_NAME;
				a_stilt->m.m.action = ACTION_EVALUATE;
				break;
			case '\n':
				stilt_p_syntax_error_print(a_stilt, c);

				_CW_STILT_NEWLINE();
				break;
			case '\0': case '\t': case '\f': case '\r': case ' ':
			case '"': case '`': case '\'': case '<': case '>':
			case '[': case ']': case '{': case '}': case '%':
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			default:
				a_stilt->state = STILTTS_NAME;
				a_stilt->m.m.action = ACTION_LITERAL;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STILTTS_COMMENT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\r':
				a_stilt->state = STILTTS_START;
				break;
			default:
				break;
			}
			break;
		case STILTTS_INTEGER: {
			cw_bool_t	restart = FALSE;

			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				_CW_STILT_PUTC(c);
				break;
			case '#': {
				cw_uint32_t	i, digit;

				/*
				 * Convert the string to a base (interpreted as
				 * base 10).
				 */
				a_stilt->m.n.base = 0;

				for (i = 0; i < a_stilt->index; i++) {
					digit =
					    _CW_STILT_GETC(a_stilt->m.n.b_off +
					    i) - '0';

					if (a_stilt->index - a_stilt->m.n.b_off
					    - i == 2)
						digit *= 10;
					a_stilt->m.n.base += digit;

					if (((digit != 0) && ((a_stilt->index -
					    a_stilt->m.n.b_off - i) > 2)) ||
					    (a_stilt->m.n.base > 36)) {
						/*
						 * Base too large. Set base to 0
						 * so that the check for too
						 * small a base catches this.
						 */
						a_stilt->m.n.base = 0;
						break;
					}
				}

				if (a_stilt->m.n.base < 2) {
					/*
					 * Base too small (or too large, as
					 * detected in the for loop above).
					 */
					a_stilt->state = STILTTS_NAME;
					a_stilt->m.m.action = ACTION_EXECUTE;
				} else {
					a_stilt->m.n.b_off = a_stilt->index + 1;
					a_stilt->state = STILTTS_INTEGER_RADIX;
				}
				_CW_STILT_PUTC(c);
				break;
			}
			case '\n':
				restart = TRUE; /* Inverted below. */
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '"': case '`': case '<': case '>': case '[':
			case ']': case '{': case '}': case '/': case '%':
				/* New token. */
				/*
				 * Invert, in case we fell through from
				 * above.
				 */
				restart = !restart;
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				if (a_stilt->index > a_stilt->m.n.b_off) {
					cw_sint64_t	val;

					/* Integer. */
					stilt_p_token_print(a_stilt, a_stilts,
					    a_stilt->index, "integer");

					/*
					 * Convert string to integer.  Do the
					 * conversion before mucking with the
					 * stack in case there is an exception.
					 */
					a_stilt->tok_str[a_stilt->index] = '\0';
					errno = 0;
					val = strtoq(a_stilt->tok_str, NULL,
					    10);
					if ((errno == ERANGE) && ((val ==
					    QUAD_MIN) || (val == QUAD_MAX)))
						stilt_error(a_stilt,
						    STILTE_RANGECHECK);

					stilo =
					    stils_push(&a_stilt->ostack,
					    a_stilt);
					stilo_integer_new(stilo, val);

					stilt_p_reset(a_stilt);
				} else {
					/* No number specified, so a name. */
					stilt_p_token_print(a_stilt, a_stilts,
					    a_stilt->index, "name 1");
					a_stilt->m.m.action = ACTION_EXECUTE;
					stilt_p_name_accept(a_stilt, a_stilts);
				}
				if (restart)
					goto RESTART;
				break;
			default:
				/* Not a number character. */
				a_stilt->m.m.action = ACTION_EXECUTE;
				a_stilt->state = STILTTS_NAME;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		}
		case STILTTS_INTEGER_RADIX: {
			cw_bool_t	restart = FALSE;

			switch (c) {
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y':
			case 'z':
				if (a_stilt->m.n.base <= (10 +
				    ((cw_uint32_t)(c - 'a')))) {
					/* Too big for this base. */
					a_stilt->state = STILTTS_NAME;
					a_stilt->m.m.action = ACTION_EXECUTE;
				}
				_CW_STILT_PUTC(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (a_stilt->m.n.base <=
				    ((cw_uint32_t)(c - '0'))) {
					/* Too big for this base. */
					a_stilt->state = STILTTS_NAME;
					a_stilt->m.m.action = ACTION_EXECUTE;
				}
				_CW_STILT_PUTC(c);
				break;
			case '\n':
				restart = TRUE; /* Inverted below. */
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '"': case '`': case '<': case '>': case '[':
			case ']': case '{': case '}': case '/': case '%':
				/* New token. */
				/*
				 * Invert, in case we fell through from
				 * above.
				 */
				restart = !restart;
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				if (a_stilt->index > a_stilt->m.n.b_off) {
					cw_sint64_t	val;

					/* Integer. */
					stilt_p_token_print(a_stilt, a_stilts,
					    a_stilt->index, "integer (radix)");

					/*
					 * Convert string to integer.  Do the
					 * conversion before mucking with the
					 * stack in case there is an exception.
					 */
					a_stilt->tok_str[a_stilt->index] = '\0';
					errno = 0;
					val =
					    strtoq(&a_stilt->tok_str[a_stilt->m.n.b_off],
					    NULL,
					    a_stilt->m.n.base);
					if ((errno == ERANGE) && ((val ==
					    QUAD_MIN) || (val == QUAD_MAX)))
						stilt_error(a_stilt,
						    STILTE_RANGECHECK);

					stilo =
					    stils_push(&a_stilt->ostack,
					    a_stilt);
					stilo_integer_new(stilo, val);

					stilt_p_reset(a_stilt);
				} else {
					/* No number specified, so a name. */
					stilt_p_token_print(a_stilt, a_stilts,
					    a_stilt->index, "name 2");
					a_stilt->m.m.action = ACTION_EXECUTE;
					stilt_p_name_accept(a_stilt, a_stilts);
				}
				if (restart)
					goto RESTART;
				break;
			default:
				/* Not a number character. */
				a_stilt->m.m.action = ACTION_EXECUTE;
				a_stilt->state = STILTTS_NAME;
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		}
		case STILTTS_ASCII_STRING:
			/* The CRLF code jumps here if there was no LF. */
			ASCII_STRING_CONTINUE:

			switch (c) {
			case '\\':
				a_stilt->state = STILTTS_ASCII_STRING_PROT_CONT;
				break;
			case '"':
				stilt_p_token_print(a_stilt, a_stilts,
				    a_stilt->index, "string");
				stilo = stils_push(&a_stilt->ostack, a_stilt);
				stilo_string_new(stilo, a_stilt,
				    a_stilt->index);
				stilo_string_set(stilo, a_stilt, 0,
				    a_stilt->tok_str, a_stilt->index);

				stilt_p_reset(a_stilt);
				break;
			case '\r':
				a_stilt->state =
				    STILTTS_ASCII_STRING_NEWLINE_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			default:
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STILTTS_ASCII_STRING_NEWLINE_CONT:
			/* All cases in the switch statement do this. */
			_CW_STILT_PUTC('\n');
			a_stilt->state = STILTTS_ASCII_STRING;
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
		case STILTTS_ASCII_STRING_PROT_CONT:
			switch (c) {
			case 'n':
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC('\n');
				break;
			case 'r':
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC('\r');
				break;
			case 't':
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC('\t');
				break;
			case 'b':
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC('\b');
				break;
			case 'f':
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC('\f');
				break;
			case '\\':
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC('\\');
				break;
			case '"':
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC('"');
				break;
			case 'x':
				a_stilt->state = STILTTS_ASCII_STRING_HEX_CONT;
				break;
			case '\r':
				a_stilt->state = STILTTS_ASCII_STRING_CRLF_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();

				/* Ignore. */
				a_stilt->state = STILTTS_ASCII_STRING;
				break;
			default:
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC('\\');
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STILTTS_ASCII_STRING_CRLF_CONT:
			switch (c) {
			case '\n':
				_CW_STILT_NEWLINE();

				/* Ignore. */
				a_stilt->state = STILTTS_ASCII_STRING;
				break;
			default:
				goto ASCII_STRING_CONTINUE;
			}
			break;
		case STILTTS_ASCII_STRING_HEX_CONT:
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':
				a_stilt->state =
				    STILTTS_ASCII_STRING_HEX_FINISH;
				a_stilt->m.s.hex_val = c;
				break;
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_ASCII_STRING_HEX_FINISH:
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f':{
				cw_uint8_t	val;

				a_stilt->state = STILTTS_ASCII_STRING;
				switch (a_stilt->m.s.hex_val) {
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					val =
					    (a_stilt->m.s.hex_val
					    - '0') << 4;
					break;
				case 'a': case 'b': case 'c': case 'd':
				case 'e': case 'f':
					val = ((a_stilt->m.s.hex_val
					    - 'a') + 10) << 4;
					break;
				default:
					_cw_not_reached();
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
					_cw_not_reached();
				}
				_CW_STILT_PUTC(val);
				break;
			}
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_LIT_STRING:
			/* The CRLF code jumps here if there was no LF. */
			LIT_STRING_CONTINUE:

			switch (c) {
			case '\'':
				a_stilt->state = STILTTS_LIT_STRING_PROT_CONT;
				break;
			case '\r':
				a_stilt->state =
				    STILTTS_LIT_STRING_NEWLINE_CONT;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			default:
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		case STILTTS_LIT_STRING_NEWLINE_CONT:
			/* All cases in the switch statement do this. */
			_CW_STILT_PUTC('\n');
			a_stilt->state = STILTTS_LIT_STRING;
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
		case STILTTS_LIT_STRING_PROT_CONT:
			switch (c) {
			case '\'':
				a_stilt->state = STILTTS_LIT_STRING;
				_CW_STILT_PUTC('\'');
				break;
			default:
				/* Accept literal string. */
				stilt_p_token_print(a_stilt, a_stilts,
				    a_stilt->index, "literal string");
				stilo = stils_push(&a_stilt->ostack, a_stilt);
				stilo_string_new(stilo, a_stilt,
				    a_stilt->index);
				stilo_string_set(stilo, a_stilt, 0,
				    a_stilt->tok_str, a_stilt->index);

				stilt_p_reset(a_stilt);

				/*
				 * We're currently looking at the first
				 * character of the next token, so re-scan it.
				 */
				goto START_CONTINUE;
			}
			break;
		case STILTTS_HEX_STRING:
			switch (c) {
			case '>': {
				cw_uint8_t	*str;
				cw_uint32_t	j;

				stilt_p_token_print(a_stilt, a_stilts,
				    a_stilt->index, "hex string");
				stilo = stils_push(&a_stilt->ostack, a_stilt);
				stilo_string_new(stilo, a_stilt,
				    (a_stilt->index + 1) >> 1);
				/*
				 * Set the character following the string in
				 * case the final hex character is missing.
				 */
				a_stilt->tok_str[a_stilt->index] = '0';

				str = stilo_string_get(stilo);
				for (j = 0; j < (a_stilt->index + 1) >> 1;
				     j++) {
					switch (a_stilt->tok_str[2 * j]) {
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9':
						str[j] = (a_stilt->tok_str[2 *
						    j] - '0') << 4;
						break;
					case 'a': case 'b': case 'c': case 'd':
					case 'e': case 'f':
						str[j] = ((a_stilt->tok_str[2 *
						    j] - 'a') + 10) << 4;
						break;
					default:
						_cw_not_reached();
					}
					
					switch (a_stilt->tok_str[2 * j + 1]) {
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9':
						str[j] |= (a_stilt->tok_str[2 *
						    j + 1] - '0');
						break;
					case 'a': case 'b': case 'c': case 'd':
					case 'e': case 'f':
						str[j] |= (a_stilt->tok_str[2 *
						    j + 1] - 'a' + 10);
						break;
					default:
						_cw_not_reached();
					}
				}
				stilt_p_reset(a_stilt);
				break;
			}
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
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_BASE64_STRING:
			switch (c) {
			case '~':
				a_stilt->m.b.nodd = 0;
				a_stilt->state = STILTTS_BASE64_STRING_FINISH;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r':
			case ' ':
				/* Ignore. */
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O':
			case 'P': case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X': case 'Y':
			case 'Z': case 'a': case 'b': case 'c': case 'd':
			case 'e': case 'f': case 'g': case 'h': case 'i':
			case 'j': case 'k': case 'l': case 'm': case 'n':
			case 'o': case 'p': case 'q': case 'r': case 's':
			case 't': case 'u': case 'v': case 'w': case 'x':
			case 'y': case 'z': case '0': case '1': case '2':
			case '3': case '4': case '5': case '6': case '7':
			case '8': case '9': case '+': case '/':
				_CW_STILT_PUTC(c);
				break;
			case '=':
				/*
				 * Calculate how many padding characters to
				 * expect.
				 */
				switch (a_stilt->index % 4) {
				case 0: case 1:
					/*
					 * We shouldn't have even seen this
					 * padding character.
					 */
					stilt_p_syntax_error_print(a_stilt, c);
					break;
				case 2:
					a_stilt->m.b.nodd = 1;
					a_stilt->state =
					    STILTTS_BASE64_STRING_PAD;
					break;
				case 3:
					a_stilt->m.b.nodd = 2;
					a_stilt->state =
					    STILTTS_BASE64_STRING_TILDE;
					break;
				default:
					_cw_not_reached();
				}
				break;
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_BASE64_STRING_PAD:
			switch (c) {
			case '=':
				a_stilt->state = STILTTS_BASE64_STRING_TILDE;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r':
			case ' ':
				/* Ignore. */
				break;
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_BASE64_STRING_TILDE:
			switch (c) {
			case '~':
				a_stilt->state = STILTTS_BASE64_STRING_FINISH;
				break;
			case '\n':
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r':
			case ' ':
				/* Ignore. */
				break;
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_BASE64_STRING_FINISH:
			switch (c) {
			case '>': {
				cw_uint8_t	*str;
				cw_uint32_t	j, ngroups;
				cw_uint32_t	bits;

				stilt_p_token_print(a_stilt, a_stilts,
				    a_stilt->index, "base 64 string");

				ngroups = a_stilt->index >> 2;
				stilo = stils_push(&a_stilt->ostack, a_stilt);
				stilo_string_new(stilo, a_stilt,
				    ngroups * 3 + a_stilt->m.b.nodd);

				str = stilo_string_get(stilo);
				for (j = 0; j < ngroups; j++) {
					/* Accumulate the bits. */
					bits = stilt_p_b64b(a_stilt->tok_str[j *
					    4]) << 18;
					bits |= stilt_p_b64b(a_stilt->tok_str[j
					    * 4 + 1]) << 12;
					bits |= stilt_p_b64b(a_stilt->tok_str[j
					    * 4 + 2]) << 6;
					bits |= stilt_p_b64b(a_stilt->tok_str[j
					    * 4 + 3]);

					/* Pull out the bytes. */
					str[j * 3] = bits >> 16;
					str[j * 3 + 1] = (bits >> 8) & 0xff;
					str[j * 3 + 2] = bits & 0xff;
				}

				switch (a_stilt->m.b.nodd) {
				case 0:
					/* abcd. Do nothing. */
					break;
				case 1:
					/* ab==. */
					/* Accumulate the bits. */
					bits = stilt_p_b64b(a_stilt->tok_str[j *
					    4]) << 18;
					bits |= stilt_p_b64b(a_stilt->tok_str[j
					    * 4 + 1]) << 12;

					/* Pull out the bytes. */
					str[j * 3] = bits >> 16;
					break;
				case 2:
					/* abc=. */
					/* Accumulate the bits. */
					bits = stilt_p_b64b(a_stilt->tok_str[j *
					    4]) << 18;
					bits |= stilt_p_b64b(a_stilt->tok_str[j
					    * 4 + 1]) << 12;
					bits |= stilt_p_b64b(a_stilt->tok_str[j
					    * 4 + 2]) << 6;

					/* Pull out the bytes. */
					str[j * 3] = bits >> 16;
					str[j * 3 + 1] = (bits >> 8) & 0xff;
					break;
				default:
					_cw_not_reached();
				}

				stilt_p_reset(a_stilt);
				break;
			}
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				break;
			}
			break;
		case STILTTS_NAME: {
			cw_bool_t	restart = FALSE;

			switch (c) {
			case '\n':
				restart = TRUE;	/* Inverted below. */
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '"': case '`': case '<': case '>': case '[':
			case ']': case '{': case '}': case '/': case '%':
				/* New token. */
				/*
				 * Invert, in case we fell through from
				 * above.
				 */
				restart = !restart;
				/* Fall through. */
			case '\0': case '\t': case '\f': case '\r': case ' ':
				/* End of name. */
				if (a_stilt->index > 0) {
					stilt_p_token_print(a_stilt, a_stilts,
					    a_stilt->index, "name 5");
					stilt_p_name_accept(a_stilt, a_stilts);
				} else {
					stilt_p_syntax_error_print(a_stilt, c);
					stilt_p_reset(a_stilt);
				}
				if (restart)
					goto RESTART;
				break;
			default:
				_CW_STILT_PUTC(c);
				break;
			}
			break;
		}
		default:
			_cw_not_reached();
			break;
		}
	}
}

#ifdef _CW_STILT_SCANNER_DEBUG
static void
stilt_p_token_print(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts, cw_uint32_t
    a_length, const cw_uint8_t *a_note)
{
	cw_uint32_t	line, col;

	stilts_position_get(a_stilts, &line, &col);
	_cw_out_put("-->");
	_cw_out_put_n(a_length, "[s]", a_stilt->tok_str);
	_cw_out_put("<-- [s] ([i]:[i] [[--> [i]:[i])\n", a_note,
	    a_stilt->tok_line, a_stilt->tok_column, a_stilts->line,
	    (a_stilts->column != -1) ? a_stilts->column : 0);
}
#endif

static void
stilt_p_syntax_error_print(cw_stilt_t *a_stilt, cw_uint8_t a_c)
{
	_cw_out_put("Syntax error for '[c]' (0x[i|b:16]), following -->", a_c,
	    a_c);
	_cw_out_put_n(a_stilt->index, "[s]", a_stilt->tok_str);
	_cw_out_put("<-- (starts at line [i], column [i])\n",
	    a_stilt->tok_line, a_stilt->tok_column);
	stilt_p_reset(a_stilt);
}

static void
stilt_p_tok_str_expand(cw_stilt_t *a_stilt)
{
	if (a_stilt->index == _CW_STILT_BUFFER_SIZE) {
		/*
		 * First overflow, initial expansion needed.
		 */
		a_stilt->tok_str = (cw_uint8_t *)stilt_malloc(a_stilt,
		    a_stilt->index * 2);
		a_stilt->buffer_len = a_stilt->index * 2;
		memcpy(a_stilt->tok_str, a_stilt->buffer,
		    a_stilt->index);
	} else if (a_stilt->index == a_stilt->buffer_len) {
		cw_uint8_t *t_str;

		/*
		 * Overflowed, and additional expansion needed.
		 */
		t_str = (cw_uint8_t *)stilt_malloc(a_stilt,
		    a_stilt->index * 2);
		a_stilt->buffer_len = a_stilt->index * 2;
		memcpy(t_str, a_stilt->tok_str, a_stilt->index);
		stilt_free(a_stilt, a_stilt->tok_str);
		a_stilt->tok_str = t_str;
	}
}

static void
stilt_p_reset(cw_stilt_t *a_stilt)
{
	a_stilt->state = STILTTS_START;
	if (a_stilt->index > _CW_STILT_BUFFER_SIZE) {
		stilt_free(a_stilt, a_stilt->tok_str);
		a_stilt->tok_str = a_stilt->buffer;
	}
	a_stilt->index = 0;
}

static void *
stilt_p_entry(void *a_arg)
{
	struct cw_stilt_entry_s	*arg = (struct cw_stilt_entry_s *)a_arg;

	stilt_interpret(arg->stilt, arg->stilts, arg->str, arg->len);
	stilt_flush(arg->stilt, arg->stilts);

	stilts_delete(arg->stilts, arg->stilt);
	stilt_delete(arg->stilt);
	/* XXX Deal with detach/join inside interpreter. */
	thd_delete(&arg->thd);
	_cw_free(arg);
	return NULL;
}

static void
stilt_p_special_accept(cw_stilt_t *a_stilt, const cw_uint8_t *a_token,
    cw_uint32_t a_len)
{
	cw_stilo_t	*stilo, key;	/*
					 * XXX In theory, GC-unsafe.  In
					 * practice, just bad practice.
					 */

	stilo_name_new(&key, a_stilt, a_token, a_len, TRUE);

	stilo = stils_push(&a_stilt->estack, a_stilt);
	if (stilt_dict_stack_search(a_stilt, &key, stilo)) {
		stils_pop(&a_stilt->estack, a_stilt);
		stilt_error(a_stilt, STILTE_UNDEFINED);
	}

	stilt_loop(a_stilt);
}

static void
stilt_p_procedure_accept(cw_stilt_t *a_stilt)
{
	cw_stilo_t	t_stilo, *stilo, *arr;	/* XXX GC-unsafe. */
	cw_uint32_t	nelements, i, depth;

	/* Find the "mark". */
	i = 0;
	depth = stils_count(&a_stilt->ostack);
	if (depth > 0) {
		stilo = stils_get(&a_stilt->ostack, a_stilt);
		if (stilo_type_get(stilo) == STILOT_NO)
			goto OUT;

		for (i = 1; i < depth; i++) {
			stilo = stils_down_get(&a_stilt->ostack, a_stilt,
			    stilo);
			if (stilo_type_get(stilo) == STILOT_NO)
				break;
		}
	}
	OUT:
	_cw_assert(i < depth);

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	stilo_array_new(&t_stilo, a_stilt, nelements);
	stilo_attrs_set(&t_stilo, STILOA_EXECUTABLE);
	arr = stilo_array_get(&t_stilo);

	/*
	 * Traverse up the stack, moving stilo's to the array.
	 */
	i = nelements;
	if (i > 0) {
		stilo = stils_get(&a_stilt->ostack, a_stilt);
		stilo_move(&arr[i - 1], stilo);

		for (i--; i > 0; i--) {
			stilo = stils_down_get(&a_stilt->ostack, a_stilt,
			    stilo);
			stilo_move(&arr[i - 1], stilo);
		}
	}

	/* Pop the stilo's off the stack now. */
	stils_npop(&a_stilt->ostack, a_stilt, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(&a_stilt->ostack, a_stilt);
	stilo_move(stilo, &t_stilo);
}

static void
stilt_p_name_accept(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts)
{
	cw_stilo_t	*stilo;

	switch (a_stilt->m.m.action) {
	case ACTION_EXECUTE:
		if (a_stilt->defer_count == 0) {
			/*
			 * Find the the value associated with the name in the
			 * dictionary stack, push it onto the execution stack,
			 * and run the execution loop.
			 */
			stilo = stils_push(&a_stilt->estack, a_stilt);
			stilo_name_new(stilo, a_stilt, a_stilt->tok_str,
			    a_stilt->index, FALSE);
			stilo_attrs_set(stilo, STILOA_EXECUTABLE);

			stilt_p_reset(a_stilt);
			stilt_loop(a_stilt);
		} else {
			/* Push the name object onto the data stack. */
			stilo = stils_push(&a_stilt->ostack, a_stilt);
			stilo_name_new(stilo, a_stilt, a_stilt->tok_str,
			    a_stilt->index, FALSE);
			stilo_attrs_set(stilo, STILOA_EXECUTABLE);
			stilt_p_reset(a_stilt);
		}
		break;
	case ACTION_LITERAL:
		/* Push the name object onto the data stack. */
		stilo = stils_push(&a_stilt->ostack, a_stilt);
		stilo_name_new(stilo, a_stilt, a_stilt->tok_str,
		    a_stilt->index, FALSE);
		stilt_p_reset(a_stilt);
		break;
	case ACTION_EVALUATE: {
		cw_stilo_t	key;	/* XXX GC-unsafe. */

		/*
		 * Find the value associated with the name in the dictionary
		 * stack and push the value onto the data stack.
		 */
		stilo_name_new(&key, a_stilt, a_stilt->tok_str,
		    a_stilt->index, FALSE);
		
		stilo = stils_push(&a_stilt->ostack, a_stilt);
		if (stilt_dict_stack_search(a_stilt, &key, stilo))
			stilt_error(a_stilt, STILTE_UNDEFINED);

		stilt_p_reset(a_stilt);
		break;
	}
	default:
		_cw_not_reached();
	}
}
