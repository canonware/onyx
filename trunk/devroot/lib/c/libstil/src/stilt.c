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

#include "../include/libstil/libstil.h"
#include "../include/libstil/stil_l.h"
#include "../include/libstil/stilo_l.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>

cw_stiln_t
stilte_stiln(cw_stilte_t a_stilte)
{
	static const cw_stiln_t stilte_stiln[] = {
		0,
		STILN_dictstackoverflow,
		STILN_dictstackunderflow,
		STILN_execstackoverflow,
		STILN_interrupt,
		STILN_invalidaccess,
		STILN_invalidcontext,
		STILN_invalidexit,
		STILN_invalidfileaccess,
		STILN_ioerror,
		STILN_limitcheck,
		STILN_rangecheck,
		STILN_stackoverflow,
		STILN_stackunderflow,
		STILN_syntaxerror,
		STILN_timeout,
		STILN_typecheck,
		STILN_undefined,
		STILN_undefinedfilename,
		STILN_undefinedresult,
		STILN_unmatchedmark,
		STILN_unregistered,
		STILN_vmerror
	};

	_cw_assert(a_stilte > 0 && a_stilte <= STILTE_LAST);
	return stilte_stiln[a_stilte];
}

/*  #define	_CW_STILT_SCANNER_DBG */

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
		a_stilts->column = -1;					\
	} while (0)

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
	
static cw_uint32_t	stilt_p_feed(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts,
    cw_uint32_t a_token, const cw_uint8_t *a_str, cw_uint32_t a_len);
static void		stilt_p_tok_str_expand(cw_stilt_t *a_stilt);
#ifdef _CW_STILT_SCANNER_DBG
static void		stilt_p_token_print(cw_stilt_t *a_stilt, cw_stilts_t
    *a_stilts, cw_uint32_t a_length, const cw_uint8_t *a_note);
static void		stilt_p_syntax_error_print(cw_stilt_t *a_stilt,
    cw_uint8_t a_c);
#else
#define			stilt_p_token_print(a, b, c, d)
#define			stilt_p_syntax_error_print(a, b)
#endif
static void		stilt_p_special_accept(cw_stilt_t *a_stilt, const
    cw_uint8_t *a_token, cw_uint32_t a_len);
static void		stilt_p_reset(cw_stilt_t *a_stilt);
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
stilts_new(cw_stilts_t *a_stilts)
{
	cw_stilts_t	*retval;

	if (a_stilts != NULL) {
		retval = a_stilts;
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_stilts_t *)_cw_malloc(sizeof(cw_stilts_t));
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
		_cw_free(a_stilts);
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
			memset(retval, 0, sizeof(cw_stilt_t));
			retval->is_malloced = TRUE;
		}
		retval->stil = a_stil;
		ql_elm_new(retval, link);
		retval->tok_str = retval->buffer;
		try_stage = 1;

		dch_new(&retval->name_hash, NULL, _CW_STILT_NAME_BASE_TABLE,
		    _CW_STILT_NAME_BASE_GROW, _CW_STILT_NAME_BASE_SHRINK,
		    stilo_l_name_hash, stilo_l_name_key_comp);
		try_stage = 2;

		stils_new(&retval->estack,
		    stila_stilsc_pool_get(stil_stila_get(a_stil)));
		try_stage = 3;

		stils_new(&retval->ostack,
		    stila_stilsc_pool_get(stil_stila_get(a_stil)));
		try_stage = 4;

		stils_new(&retval->dstack,
		    stila_stilsc_pool_get(stil_stila_get(a_stil)));
		try_stage = 5;

		stils_new(&retval->tstack,
		    stila_stilsc_pool_get(stil_stila_get(a_stil)));
		try_stage = 6;

		/*
		 * Create derror, errordict, and userdict.  threaddict
		 * initialization needs these to already be initialized.
		 */
		derror_populate(&retval->derror, retval);
		errordict_populate(&retval->errordict, retval);
		stilo_dict_new(&retval->userdict, stilt_stil_get(retval),
		    _CW_STILT_USERDICT_SIZE);

		/* Create threaddict. */
		threaddict_populate(&retval->threaddict, retval);

		/*
		 * Push threaddict, systemdict, globaldict, and userdict onto
		 * the dictionary stack.
		 */
		stilo = stils_push(&retval->dstack);
		stilo_dup(stilo, &retval->threaddict);

		stilo = stils_push(&retval->dstack);
		stilo_dup(stilo, stil_systemdict_get(a_stil));

		stilo = stils_push(&retval->dstack);
		stilo_dup(stilo, stil_globaldict_get(a_stil));

		stilo = stils_push(&retval->dstack);
		stilo_dup(stilo, &retval->userdict);
	}
	xep_catch(_CW_XEPV_OOM) {
		retval = (cw_stilt_t *)v_retval;
		switch (try_stage) {
		case 6:
			stils_delete(&retval->tstack);
		case 5:
			stils_delete(&retval->dstack);
		case 4:
			stils_delete(&retval->ostack);
		case 3:
			stils_delete(&retval->estack);
		case 2:
			dch_delete(&retval->name_hash);
		case 1:
			if (retval->is_malloced)
				_cw_free(retval);
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();

	stil_l_stilt_insert(a_stil, retval);
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
		 * so just clean up.
		 */
		_cw_free(a_stilt->tok_str);
	}

	stils_delete(&a_stilt->tstack);
	stils_delete(&a_stilt->dstack);
	stils_delete(&a_stilt->ostack);
	stils_delete(&a_stilt->estack);
	dch_delete(&a_stilt->name_hash);
	stil_l_stilt_remove(a_stilt->stil, a_stilt);
	if (a_stilt->is_malloced)
		_cw_free(a_stilt);
#ifdef _LIBSTIL_DBG
	else
		memset(a_stilt, 0x5a, sizeof(cw_stilt_t));
#endif
}

void *
stilt_p_thread_entry(void *a_arg)
{
	cw_stilt_thread_entry_t	*arg = (cw_stilt_thread_entry_t *)a_arg;

	/* Run. */
	stilt_loop(arg->stilt);

	/* Wait to be joined or detated, if not already so. */
	mtx_lock(&arg->lock);
	arg->done = TRUE;
	while (arg->detached == FALSE && arg->joined == FALSE) {
		cnd_wait(&arg->done_cnd, &arg->lock);
	}
	if (arg->detached) {
		mtx_unlock(&arg->lock);

		/* Clean up. */
		cnd_delete(&arg->join_cnd);
		cnd_delete(&arg->done_cnd);
		mtx_delete(&arg->lock);
		stilt_delete(arg->stilt);
		thd_delete(arg->thd);
		_cw_free(arg);
	} else if (arg->joined) {
		/* Wake the joiner back up. */
		cnd_signal(&arg->join_cnd);
		/* We're done.  The joiner will clean up. */
		arg->gone = TRUE;
		mtx_unlock(&arg->lock);
	} else
		_cw_not_reached();

	return NULL;
}

void
stilt_thread(cw_stilt_t *a_stilt)
{
	a_stilt->entry = (cw_stilt_thread_entry_t
	    *)_cw_malloc(sizeof(cw_stilt_thread_entry_t));

	a_stilt->entry->stilt = a_stilt;
	mtx_new(&a_stilt->entry->lock);
	cnd_new(&a_stilt->entry->done_cnd);
	cnd_new(&a_stilt->entry->join_cnd);
	a_stilt->entry->done = FALSE;
	a_stilt->entry->gone = FALSE;
	a_stilt->entry->detached = FALSE;
	a_stilt->entry->joined = FALSE;

	a_stilt->entry->thd = thd_new(stilt_p_thread_entry, (void
	    *)a_stilt->entry);
}

void
stilt_detach(cw_stilt_t *a_stilt)
{
	mtx_lock(&a_stilt->entry->lock);
	a_stilt->entry->detached = TRUE;
	if (a_stilt->entry->done) {
		/* The thread is already done, so wake it back up. */
		cnd_signal(&a_stilt->entry->done_cnd);
	}
	mtx_unlock(&a_stilt->entry->lock);
}

void
stilt_join(cw_stilt_t *a_stilt)
{
	cw_stilt_thread_entry_t	*entry;

	mtx_lock(&a_stilt->entry->lock);
	a_stilt->entry->joined = TRUE;
	if (a_stilt->entry->done) {
		/* The thread is already done, so wake it back up. */
		cnd_signal(&a_stilt->entry->done_cnd);
	}
	/* Wait for the thread to totally go away. */
	while (a_stilt->entry->gone == FALSE)
		cnd_wait(&a_stilt->entry->join_cnd, &a_stilt->entry->lock);
	mtx_unlock(&a_stilt->entry->lock);

	/* Clean up. */
	cnd_delete(&a_stilt->entry->join_cnd);
	cnd_delete(&a_stilt->entry->done_cnd);
	mtx_delete(&a_stilt->entry->lock);
	thd_join(a_stilt->entry->thd);
	entry = a_stilt->entry;
	stilt_delete(a_stilt->entry->stilt);
	_cw_free(entry);
}

void
stilt_reset(cw_stilt_t *a_stilt)
{
	a_stilt->defer_count = 0;
	stilt_p_reset(a_stilt);
}

void
stilt_loop(cw_stilt_t *a_stilt)
{
	cw_stilo_t	*stilo, *tstilo;
	cw_uint32_t	sdepth;

	for (sdepth = stils_count(&a_stilt->estack);
	     stils_count(&a_stilt->estack) >= sdepth;) {
		stilo = stils_get(&a_stilt->estack);
		if (stilo_attrs_get(stilo) == STILOA_LITERAL) {
			/* Always push literal objects onto the data stack. */
			tstilo = stils_push(&a_stilt->ostack);
			stilo_dup(tstilo, stilo);
			stils_pop(&a_stilt->estack);
			continue;
		}

		switch (stilo_type_get(stilo)) {
		case STILOT_BOOLEAN:
		case STILOT_CONDITION:
		case STILOT_DICT:
		case STILOT_INTEGER:
		case STILOT_MARK:
		case STILOT_MUTEX:
			/*
			 * Always push the object onto the data stack, even
			 * though it isn't literal.
			 */
			tstilo = stils_push(&a_stilt->ostack);
			stilo_dup(tstilo, stilo);
			stils_pop(&a_stilt->estack);
			break;
		case STILOT_NULL:
			/* Do nothing. */
			stils_pop(&a_stilt->estack);
			break;
		case STILOT_ARRAY: {
			cw_uint32_t	i, len;
			cw_stilo_t	*array;

			len = stilo_array_len_get(stilo);
			if (len == 0) {
				stils_pop(&a_stilt->estack);
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
					    stils_push(&a_stilt->ostack);
					stilo_dup(tstilo, &array[i]);
					continue;
				}

				switch (stilo_type_get(&array[i])) {
				case STILOT_ARRAY:
					/*
					 * Don't execute nested arrays.
					 */
					tstilo = stils_push(&a_stilt->ostack);
					stilo_dup(tstilo, &array[i]);
					break;
				case STILOT_OPERATOR:
					if (stilo_operator_fast(&array[i]) ==
					    FALSE) {
						stilo_operator_f(&array[i])
						    (a_stilt);
						break;
					}

					/* Fast operator. */
					switch (stilo_operator_stiln(&array[i])) {
					case STILN_add: {
						cw_stilo_t	*a, *b;

						b = stils_get(&a_stilt->ostack);
						if (b == NULL) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							goto NEXT;
						}
						a =
						    stils_down_get(&a_stilt->ostack,
						    b);
						if (a == NULL) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							goto NEXT;
						}

						if (stilo_type_get(a) !=
						    STILOT_INTEGER ||
						    stilo_type_get(b) !=
						    STILOT_INTEGER) {
							stilt_error(a_stilt,
							    STILTE_TYPECHECK);
							goto NEXT;
						}

						stilo_integer_set(a,
						    stilo_integer_get(a) +
						    stilo_integer_get(b));
						stils_pop(&a_stilt->ostack);
						break;
					}
					case STILN_dup: {
						cw_stilo_t	*orig, *dup;

						orig =
						    stils_get(&a_stilt->ostack);
						if (orig == NULL) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							goto NEXT;
						}
						dup =
						    stils_push(&a_stilt->ostack);
						stilo_dup(dup, orig);
						break;
					}
					case STILN_exch:
						if (stils_count(&a_stilt->ostack)
						    < 2) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							goto NEXT;
						}
						stils_roll(&a_stilt->ostack, 2,
						    1);
						break;
					case STILN_index: {
						cw_stilo_t	*stilo, *orig;
						cw_sint64_t	index;

						stilo =
						    stils_get(&a_stilt->ostack);
						if (stilo == NULL) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							    goto NEXT;
						}
						if (stilo_type_get(stilo) !=
						    STILOT_INTEGER) {
							stilt_error(a_stilt,
							    STILTE_TYPECHECK);
							goto NEXT;
						}
						index =
						    stilo_integer_get(stilo);
						if (index < 0) {
							stilt_error(a_stilt,
							    STILTE_RANGECHECK);
							goto NEXT;
						}
						orig =
						    stils_nget(&a_stilt->ostack,
						    index + 1);
						if (orig == NULL) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							goto NEXT;
						}

						stilo_dup(stilo, orig);
						break;
					}
					case STILN_pop:
						if (stils_pop(&a_stilt->ostack)) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							    goto NEXT;
						}
						break;
					case STILN_roll: {
						cw_stilo_t	*stilo;
						cw_sint64_t	count, amount;

						stilo =
						    stils_get(&a_stilt->ostack);
						if (stilo == NULL) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							    goto NEXT;
						}
						if (stilo_type_get(stilo) !=
						    STILOT_INTEGER) {
							stilt_error(a_stilt,
							    STILTE_TYPECHECK);
							goto NEXT;
						}
						amount =
						    stilo_integer_get(stilo);

						stilo =
						    stils_down_get(&a_stilt->ostack, stilo);
						if (stilo == NULL) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							    goto NEXT;
						}
						if (stilo_type_get(stilo) !=
						    STILOT_INTEGER) {
							stilt_error(a_stilt,
							    STILTE_TYPECHECK);
							goto NEXT;
						}
						count =
						    stilo_integer_get(stilo);
						if (count < 1) {
							stilt_error(a_stilt,
							    STILTE_RANGECHECK);
							goto NEXT;
						}
						if (count >
						    stils_count(&a_stilt->ostack)
						    - 2) {
							stilt_error(a_stilt,
							    STILTE_STACKUNDERFLOW);
							goto NEXT;
						}

						stils_npop(&a_stilt->ostack, 2);
						stils_roll(&a_stilt->ostack,
						    count, amount);

						break;
					}
					default:
						   _cw_not_reached();
					}
					break;
				default:
					/*
					 * Not a simple common case, so use the
					 * generic algorithm.
					 */
					tstilo = stils_push(&a_stilt->estack);
					stilo_dup(tstilo, &array[i]);
					stilt_loop(a_stilt);
				}
				NEXT:
			}

			/*
			 * If recursion is possible and likely, make tail
			 * recursion safe by replacing the array with its last
			 * element before executing the last element.
			 */
			if ((stilo_attrs_get(&array[i]) == STILOA_LITERAL) ||
			    (stilo_type_get(&array[i]) == STILOT_ARRAY)) {
				/*
				 * Always push literal objects and nested arrays
				 * onto the data stack.
				 */
				tstilo = stils_push(&a_stilt->ostack);
				stilo_dup(tstilo, &array[i]);
				stils_pop(&a_stilt->estack);
			} else {
				/* Possible recursion. */
				tstilo = stils_push(&a_stilt->estack);
				stilo_dup(tstilo, &array[i]);
				stils_roll(&a_stilt->estack, 2, 1);
				stils_pop(&a_stilt->estack);
			}
			break;
		}
		case STILOT_STRING: {
			cw_stilts_t	stilts;

			/*
			 * Use the string as a source of code.
			 */
			stilts_new(&stilts);
			stilt_interpret(a_stilt, &stilts,
			    stilo_string_get(stilo),
			    stilo_string_len_get(stilo));
			stilt_flush(a_stilt, &stilts);
			stilts_delete(&stilts, a_stilt);
			stils_pop(&a_stilt->estack);

			break;
		}
		case STILOT_NAME: {
			cw_stilo_t	*val;

			/*
			 * Search for a value associated with the name
			 * in the dictionary stack, push it onto the
			 * execution stack, and execute it.
			 */
			val = stils_push(&a_stilt->tstack);
			if (stilt_dict_stack_search(a_stilt, stilo, val)) {
				stilt_error(a_stilt, STILTE_UNDEFINED);
				stils_pop(&a_stilt->estack);
			} else {
				stilo_dup(stilo, val);
				stils_pop(&a_stilt->tstack);
			}
			break;
		}
		case STILOT_OPERATOR:
			stilo_operator_f(stilo)(a_stilt);
			stils_pop(&a_stilt->estack);
			break;
		case STILOT_FILE: {
			cw_stilts_t	stilts;
			cw_sint32_t	nread;
			cw_uint8_t	buffer[_CW_STILT_FILE_READ_SIZE];

			stilts_new(&stilts);
			/*
			 * Read data from the file and interpret it until an EOF
			 * (0 byte read).
			 */
			for (nread = stilo_file_read(stilo,
			    _CW_STILT_FILE_READ_SIZE, buffer); nread > 0;
			    nread = stilo_file_read(stilo,
			    _CW_STILT_FILE_READ_SIZE, buffer)) {
				stilt_interpret(a_stilt, &stilts, buffer,
				    nread);
			}
			stilt_flush(a_stilt, &stilts);
			stilts_delete(&stilts, a_stilt);

			stils_pop(&a_stilt->estack);
			break;
		}
		case STILOT_HOOK: {
			cw_stilte_t	error;
			
			error = stilo_hook_exec(stilo, a_stilt);
			if (error)
				stilt_error(a_stilt, error);

			stils_pop(&a_stilt->estack);
			break;
		}
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

	stilt_p_feed(a_stilt, a_stilts, 0, a_str, a_len);
}

void
stilt_flush(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts)
{
	static const cw_uint8_t	str[] = "\n";

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	stilt_p_feed(a_stilt, a_stilts, 0, str, sizeof(str) - 1);
}

cw_uint32_t
stilt_token(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts, const cw_uint8_t *a_str,
    cw_uint32_t a_len)
{
	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	return stilt_p_feed(a_stilt, a_stilts, 1, a_str, a_len);
}

void
stilt_error(cw_stilt_t *a_stilt, cw_stilte_t a_error)
{
	cw_stilo_t	*stilo, *errordict, *key, *handler;
	cw_stiln_t	stiln;
	cw_bool_t	ostack_push = TRUE;
	cw_uint32_t	defer_count;

	_cw_check_ptr(a_stilt);
	_cw_assert(a_stilt->magic == _CW_STILT_MAGIC);

	/* Shut off deferral temporarily. */
	defer_count = a_stilt->defer_count;
	a_stilt->defer_count = 0;

	/*
	 * Get errordict.  We can't throw an undefined error here because it
	 * would go infinitely recursive.
	 */
	errordict = stils_push(&a_stilt->tstack);
	key = stils_push(&a_stilt->tstack);
	stilo_name_new(key, stilt_stil_get(a_stilt), stiln_str(STILN_errordict),
	    stiln_len(STILN_errordict), TRUE);
	if (stilt_dict_stack_search(a_stilt, key, errordict)) {
		stils_npop(&a_stilt->tstack, 2);
		xep_throw(_CW_STILX_ERRORDICT);
	}

	/*
	 * Set the error in case our generic error handler gets called (not to
	 * mention for general use).
	 */
	a_stilt->error = a_error;

	/*
	 * Find handler corresponding to error.
	 */
	stiln = stilte_stiln(a_error);
	stilo_name_new(key, stilt_stil_get(a_stilt), stiln_str(stiln),
	    stiln_len(stiln), TRUE);

	/*
	 * Push the object being executed onto ostack unless this is an
	 * interrupt or timeout.
	 */
	if (ostack_push) {
		stilo = stils_push(&a_stilt->ostack);
		stilo_dup(stilo, stils_get(&a_stilt->estack));
	}

	/*
	 * Get the handler for this particular error and push it onto estack.
	 * We could potentially throw another error here without going
	 * infinitely recursive, but it's not worth the risk.  After all, the
	 * user has done some really hokey management of errordict if this
	 * happens.
	 */
	handler = stils_push(&a_stilt->estack);
	if (stilo_dict_lookup(errordict, a_stilt, key, handler)) {
		stils_npop(&a_stilt->tstack, 2);
		stils_pop(&a_stilt->estack);
		xep_throw(_CW_STILX_ERRORDICT);
	}
	stils_npop(&a_stilt->tstack, 2);

	/* Execute the handler. */
	stilt_loop(a_stilt);

	/* Turn deferral back on. */
	a_stilt->defer_count = defer_count;
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
	for (i = 0, depth = stils_count(&a_stilt->dstack), dict = NULL; i <
	    depth; i++) {
		dict = stils_down_get(&a_stilt->dstack, dict);
		if (stilo_dict_lookup(dict, a_stilt, a_key, r_value) ==
		    FALSE) {
			/* Found. */
			retval = FALSE;
			goto RETURN;
		}
	}

	retval = TRUE;
	RETURN:
	return retval;
}

cw_stils_t *
stilt_l_ref_iter(cw_stilt_t *a_stilt, cw_bool_t a_reset)
{
	cw_stils_t	*retval;

	if (a_reset)
		a_stilt->ref_iter = 0;

	switch (a_stilt->ref_iter) {
	case 0:
		retval = &a_stilt->estack;
		break;
	case 1:
		retval = &a_stilt->ostack;
		break;
	case 2:
		retval = &a_stilt->dstack;
		break;
	case 3:
		retval = &a_stilt->tstack;
		break;
	default:
		retval = NULL;
	}
	a_stilt->ref_iter++;

	return retval;
}

static cw_uint32_t
stilt_p_feed(cw_stilt_t *a_stilt, cw_stilts_t *a_stilts, cw_uint32_t a_token,
    const cw_uint8_t *a_str, cw_uint32_t a_len)
{
	cw_uint32_t	retval, i;
	cw_uint8_t	c;
	cw_stilo_t	*stilo;
	cw_bool_t	token;

	if (a_token) {
		token = FALSE;
		a_stilt->defer_count++;
	}

	for (i = 0; i < a_len; i++, a_stilts->column++) {
		c = a_str[i];

#ifdef _CW_STILT_SCANNER_DBG
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

			if (a_token) {
				/*
				 * token is TRUE if a token has been accepted.
				 * We look for the situation where token is TRUE
				 * and a_stilt->defer_count is only 1
				 * (artificially raised).  If these conditions
				 * are met, then we've managed to scan an entire
				 * token, as defined by the token operator.
				 */
				if (token && a_stilt->defer_count == 1) {
					/*
					 * Return the offset of the next
					 * character.
					 */
					retval = i;
					goto RETURN;
				}
			}

			/* Record where this token starts. */
			a_stilt->tok_line = a_stilts->line;
			a_stilt->tok_column = a_stilts->column;

			switch (c) {
			case '(':
				a_stilt->state = STILTTS_ASCII_STRING;
				a_stilt->m.s.p_depth = 1;
				break;
			case ')':
				stilt_p_syntax_error_print(a_stilt, c);
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
				token = TRUE;
				stilt_p_special_accept(a_stilt, "[", 1);
				break;
			case ']':
				/* An operator, not the same as '}'. */
				stilt_p_token_print(a_stilt, a_stilts, 0, "]");
				token = TRUE;
				stilt_p_special_accept(a_stilt, "]", 1);
				break;
			case '{':
				stilt_p_token_print(a_stilt, a_stilts, 0, "{");
				a_stilt->defer_count++;
				stilo = stils_push(&a_stilt->ostack);
				stilo_no_new(stilo);
				/*
				 * Leave the stilo as notype in order to
				 * differentiate from normal marks.
				 */
				break;
			case '}':
				stilt_p_token_print(a_stilt, a_stilts, 0, "}");
				if (a_stilt->defer_count > a_token) {
					token = TRUE;
					a_stilt->defer_count--;
					stilt_p_procedure_accept(a_stilt);
				} else {
					/* Missing '{'. */
					stilt_p_syntax_error_print(a_stilt, c);
					stilt_p_reset(a_stilt);
					stilt_error(a_stilt,
					    STILTE_SYNTAXERROR);
					if (a_token)
						goto RETURN;
				}
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
				token = TRUE;
				stilt_p_special_accept(a_stilt, "<<", 2);
				break;
			case '>':
				a_stilt->state = STILTTS_START;
				stilt_p_token_print(a_stilt, a_stilts,
				    a_stilt->index, "empty hex string");
				token = TRUE;
				stilo = stils_push(&a_stilt->ostack);
				stilo_string_new(stilo, stilt_stil_get(a_stilt),
				    0);
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
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
			}
			break;
		case STILTTS_GT_CONT:
			_cw_assert(a_stilt->index == 0);

			switch (c) {
			case '>':
				a_stilt->state = STILTTS_START;
				stilt_p_token_print(a_stilt, a_stilts, 0, ">>");
				token = TRUE;
				stilt_p_special_accept(a_stilt, ">>", 2);
				break;
			default:
				stilt_p_syntax_error_print(a_stilt, c);
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
				_CW_STILT_NEWLINE();

				stilt_p_syntax_error_print(a_stilt, c);
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
				break;
			case '\0': case '\t': case '\f': case '\r': case ' ':
			case '(': case ')': case '`': case '\'': case '<':
			case '>': case '[': case ']': case '{': case '}':
			case '%':
				stilt_p_syntax_error_print(a_stilt, c);
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
			case '(': case ')': case '`': case '\'': case '<':
			case '>': case '[': case ']': case '{': case '}':
			case '/': case '%':
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
					    QUAD_MIN) || (val == QUAD_MAX))) {
						stilt_p_reset(a_stilt);
						stilt_error(a_stilt,
						    STILTE_RANGECHECK);
					} else {
						token = TRUE;
						stilo =
						    stils_push(&a_stilt->ostack);
						stilo_integer_new(stilo, val);
						stilt_p_reset(a_stilt);
					}
				} else {
					/* No number specified, so a name. */
					stilt_p_token_print(a_stilt, a_stilts,
					    a_stilt->index, "name 1");
					token = TRUE;
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
			case '(': case ')': case '`': case '\'': case '<':
			case '>': case '[': case ']': case '{': case '}':
			case '/': case '%':
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
					    QUAD_MIN) || (val == QUAD_MAX))) {
						stilt_p_reset(a_stilt);
						stilt_error(a_stilt,
						    STILTE_RANGECHECK);
					} else {
						token = TRUE;
						stilo =
						    stils_push(&a_stilt->ostack);
						stilo_integer_new(stilo, val);
						stilt_p_reset(a_stilt);
					}
				} else {
					/* No number specified, so a name. */
					stilt_p_token_print(a_stilt, a_stilts,
					    a_stilt->index, "name 2");
					token = TRUE;
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
			case '(':
				a_stilt->m.s.p_depth++;
				_CW_STILT_PUTC(c);
				break;
			case ')':
				a_stilt->m.s.p_depth--;
				if (a_stilt->m.s.p_depth == 0) {
					stilt_p_token_print(a_stilt, a_stilts,
					    a_stilt->index, "string");
					token = TRUE;
					stilo = stils_push(&a_stilt->ostack);
					stilo_string_new(stilo,
					    stilt_stil_get(a_stilt),
					    a_stilt->index);
					stilo_string_set(stilo, 0,
					    a_stilt->tok_str, a_stilt->index);

					stilt_p_reset(a_stilt);
				} else
					_CW_STILT_PUTC(c);
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
			case '(': case ')': case '\\':
				a_stilt->state = STILTTS_ASCII_STRING;
				_CW_STILT_PUTC(c);
				break;
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
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
				token = TRUE;
				stilo = stils_push(&a_stilt->ostack);
				stilo_string_new(stilo, stilt_stil_get(a_stilt),
				    a_stilt->index);
				stilo_string_set(stilo, 0, a_stilt->tok_str,
				    a_stilt->index);

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
				token = TRUE;
				stilo = stils_push(&a_stilt->ostack);
				stilo_string_new(stilo, stilt_stil_get(a_stilt),
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
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
					stilt_p_reset(a_stilt);
					stilt_error(a_stilt,
					    STILTE_SYNTAXERROR);
					if (a_token)
						goto RETURN;
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
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
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
				token = TRUE;

				ngroups = a_stilt->index >> 2;
				stilo = stils_push(&a_stilt->ostack);
				stilo_string_new(stilo, stilt_stil_get(a_stilt),
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
				stilt_p_reset(a_stilt);
				stilt_error(a_stilt, STILTE_SYNTAXERROR);
				if (a_token)
					goto RETURN;
			}
			break;
		case STILTTS_NAME: {
			cw_bool_t	restart = FALSE;

			switch (c) {
			case '\n':
				restart = TRUE;	/* Inverted below. */
				_CW_STILT_NEWLINE();
				/* Fall through. */
			case '(': case ')': case '`': case '\'': case '<':
			case '>': case '[': case ']': case '{': case '}':
			case '/': case '%':
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
					token = TRUE;
					stilt_p_name_accept(a_stilt, a_stilts);
				} else {
					stilt_p_syntax_error_print(a_stilt, c);
					stilt_p_reset(a_stilt);
					stilt_error(a_stilt,
					    STILTE_SYNTAXERROR);
					if (a_token)
						goto RETURN;
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
		}
	}

	retval = i;
	RETURN:
	if (a_token)
		a_stilt->defer_count--;
	return retval;
}

#ifdef _CW_STILT_SCANNER_DBG
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

static void
stilt_p_syntax_error_print(cw_stilt_t *a_stilt, cw_uint8_t a_c)
{
	_cw_out_put("Syntax error for '[c]' (0x[i|b:16]), following -->", a_c,
	    a_c);
	_cw_out_put_n(a_stilt->index, "[s]", a_stilt->tok_str);
	_cw_out_put("<-- (starts at line [i], column [i])\n",
	    a_stilt->tok_line, a_stilt->tok_column);
}
#endif

static void
stilt_p_tok_str_expand(cw_stilt_t *a_stilt)
{
	if (a_stilt->index == _CW_STILT_BUFFER_SIZE) {
		/*
		 * First overflow, initial expansion needed.
		 */
		a_stilt->tok_str = (cw_uint8_t *)_cw_malloc(a_stilt->index * 2);
		a_stilt->buffer_len = a_stilt->index * 2;
		memcpy(a_stilt->tok_str, a_stilt->buffer,
		    a_stilt->index);
	} else if (a_stilt->index == a_stilt->buffer_len) {
		cw_uint8_t *t_str;

		/*
		 * Overflowed, and additional expansion needed.
		 */
		t_str = (cw_uint8_t *)_cw_malloc(a_stilt->index * 2);
		a_stilt->buffer_len = a_stilt->index * 2;
		memcpy(t_str, a_stilt->tok_str, a_stilt->index);
		_cw_free(a_stilt->tok_str);
		a_stilt->tok_str = t_str;
	}
}

static void
stilt_p_reset(cw_stilt_t *a_stilt)
{
	a_stilt->state = STILTTS_START;
	if (a_stilt->index > _CW_STILT_BUFFER_SIZE) {
		_cw_free(a_stilt->tok_str);
		a_stilt->tok_str = a_stilt->buffer;
	}
	a_stilt->index = 0;
}

static void
stilt_p_special_accept(cw_stilt_t *a_stilt, const cw_uint8_t *a_token,
    cw_uint32_t a_len)
{
	cw_stilo_t	*stilo, key;	/*
					 * XXX In theory, GC-unsafe.  In
					 * practice, just bad practice.
					 */

	stilo_name_new(&key, stilt_stil_get(a_stilt), a_token, a_len, TRUE);

	stilo = stils_push(&a_stilt->estack);
	if (stilt_dict_stack_search(a_stilt, &key, stilo)) {
		stilt_error(a_stilt, STILTE_UNDEFINED);
		stils_pop(&a_stilt->estack);
	} else
		stilt_loop(a_stilt);
}

static void
stilt_p_procedure_accept(cw_stilt_t *a_stilt)
{
	cw_stilo_t	*tstilo, *stilo, *arr;
	cw_uint32_t	nelements, i, depth;

	/* Find the no "mark". */
	for (i = 0, depth = stils_count(&a_stilt->ostack), stilo = NULL; i <
	    depth; i++) {
		stilo = stils_down_get(&a_stilt->ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_NO)
			break;
	}
	_cw_assert(i < depth);

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	tstilo = stils_push(&a_stilt->tstack);
	stilo_array_new(tstilo, stilt_stil_get(a_stilt), nelements);
	stilo_attrs_set(tstilo, STILOA_EXECUTABLE);
	arr = stilo_array_get(tstilo);

	/*
	 * Traverse down the stack, moving stilo's to the array.
	 */
	for (i = nelements, stilo = NULL; i > 0; i--) {
		stilo = stils_down_get(&a_stilt->ostack, stilo);
		stilo_dup(&arr[i - 1], stilo);
	}

	/* Pop the stilo's off the stack now. */
	stils_npop(&a_stilt->ostack, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(&a_stilt->ostack);
	stilo_dup(stilo, tstilo);
	stils_pop(&a_stilt->tstack);
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
			stilo = stils_push(&a_stilt->estack);
			stilo_name_new(stilo, stilt_stil_get(a_stilt),
			    a_stilt->tok_str, a_stilt->index, FALSE);
			stilo_attrs_set(stilo, STILOA_EXECUTABLE);

			stilt_p_reset(a_stilt);
			stilt_loop(a_stilt);
		} else {
			/* Push the name object onto the data stack. */
			stilo = stils_push(&a_stilt->ostack);
			stilo_name_new(stilo, stilt_stil_get(a_stilt),
			    a_stilt->tok_str, a_stilt->index, FALSE);
			stilo_attrs_set(stilo, STILOA_EXECUTABLE);
			stilt_p_reset(a_stilt);
		}
		break;
	case ACTION_LITERAL:
		/* Push the name object onto the data stack. */
		stilo = stils_push(&a_stilt->ostack);
		stilo_name_new(stilo, stilt_stil_get(a_stilt), a_stilt->tok_str,
		    a_stilt->index, FALSE);
		stilt_p_reset(a_stilt);
		break;
	case ACTION_EVALUATE: {
		cw_stilo_t	*key;

		/*
		 * Find the value associated with the name in the dictionary
		 * stack and push the value onto the data stack.
		 */
		key = stils_push(&a_stilt->estack);
		stilo_name_new(key, stilt_stil_get(a_stilt), a_stilt->tok_str,
		    a_stilt->index, FALSE);
		stilt_p_reset(a_stilt);

		stilo = stils_push(&a_stilt->ostack);
		if (stilt_dict_stack_search(a_stilt, key, stilo)) {
			stils_pop(&a_stilt->ostack);
			stilt_error(a_stilt, STILTE_UNDEFINED);
		}
		stils_pop(&a_stilt->estack);

		break;
	}
	default:
		_cw_not_reached();
	}
}
