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

#include "../include/libstash/libstash.h"

#include <stdarg.h>
#include <fcntl.h>

static cw_bool_t	res_p_res_parse(cw_res_t *a_res, cw_bool_t a_is_file);
static cw_uint32_t	res_p_char_type(char a_char);
static void		res_p_res_merge(cw_res_t *a_res, const char *a_name,
    const char *a_val);

/*
 * Parameters that control the initial size and growth/shrinkage characteristics
 * of the hash table.
 */
#define _CW_RES_BASE_TABLE			32
#define _CW_RES_BASE_GROW			24
#define _CW_RES_BASE_SHRINK			 8

/* Initial size of buffer to use for name/value parsing. */
#define _LIBSTASH_RES_BUFFSIZE 8192

/* Character types for state machine. */
#define _LIBSTASH_RES_CHAR_CAP			 0
#define _LIBSTASH_RES_CHAR_LOWER		 1
#define _LIBSTASH_RES_CHAR_NUMBER		 2
#define _LIBSTASH_RES_CHAR_UNDER		 3
#define _LIBSTASH_RES_CHAR_PERIOD		 4
#define _LIBSTASH_RES_CHAR_HASH			 5
#define _LIBSTASH_RES_CHAR_WHITESPACE		 6
#define _LIBSTASH_RES_CHAR_COLON		 7
#define _LIBSTASH_RES_CHAR_BACKSLASH		 8
#define _LIBSTASH_RES_CHAR_NEWLINE		 9
#define _LIBSTASH_RES_CHAR_NULL			10
#define _LIBSTASH_RES_CHAR_VALID_IN_VAL		11
#define _LIBSTASH_RES_CHAR_OTHER		12

/* State machine states. */
#define _LIBSTASH_RES_STATE_START		 0
#define _LIBSTASH_RES_STATE_BEGIN_WHITESPACE	 1
#define _LIBSTASH_RES_STATE_BEGIN_COMMENT	 2
#define _LIBSTASH_RES_STATE_NAME		 3
#define _LIBSTASH_RES_STATE_POST_NAME_WHITESPACE 4
#define _LIBSTASH_RES_STATE_POST_COLON_WHITESPACE 5
#define _LIBSTASH_RES_STATE_VALUE		 6
#define _LIBSTASH_RES_STATE_VALUE_BACKSLASH	 7
#define _LIBSTASH_RES_STATE_BACKSLASH_WHITESPACE 8
#define _LIBSTASH_RES_STATE_TRAILING_COMMENT	 9
#define _LIBSTASH_RES_STATE_FINISH		10

cw_res_t *
res_new(cw_res_t *a_res, cw_mem_t *a_mem)
{
	cw_res_t		*retval;
	volatile cw_uint32_t	try_stage = 0;

	xep_begin();
	volatile cw_res_t	*v_retval;
	xep_try {
		if (a_res != NULL) {
			v_retval = retval = a_res;
			retval->is_malloced = FALSE;
		} else {
			v_retval = retval = (cw_res_t *)mem_malloc(a_mem,
			    sizeof(cw_res_t));
			retval->is_malloced = TRUE;
		}
		try_stage = 1;

		/* Initialize internals. */
		retval->mem = a_mem;
		rwl_new(&retval->rw_lock);
		dch_new(&retval->hash, a_mem, _CW_RES_BASE_TABLE,
		    _CW_RES_BASE_GROW, _CW_RES_BASE_SHRINK, ch_string_hash,
		    ch_string_key_comp);
		try_stage = 2;
	}
	xep_catch(_CW_XEPV_OOM) {
		retval = (cw_res_t *)v_retval;
		switch (try_stage) {
		case 1:
			if (retval->is_malloced)
				mem_free(a_mem, retval);
		case 0:
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();

	return retval;
}

void
res_delete(cw_res_t *a_res)
{
	cw_uint64_t	i, num_resources;
	void		*name, *val;

	_cw_check_ptr(a_res);

	/* Clean up internals. */
	rwl_delete(&a_res->rw_lock);

	for (i = 0, num_resources = dch_count(&a_res->hash); i < num_resources;
	     i++) {
		dch_remove_iterate(&a_res->hash, &name, &val, NULL);
		mem_free(a_res->mem, name);
		mem_free(a_res->mem, val);
	}
	dch_delete(&a_res->hash);

	if (a_res->is_malloced)
		mem_free(a_res->mem, a_res);
}

void
res_clear(cw_res_t *a_res)
{
	char	*key, *val;

	_cw_check_ptr(a_res);
	rwl_wlock(&a_res->rw_lock);

	while (dch_remove_iterate(&a_res->hash, (void **)&key, (void **)&val,
	    NULL) == FALSE) {
		mem_free(a_res->mem, key);
		mem_free(a_res->mem, val);
	}

	rwl_wunlock(&a_res->rw_lock);
}

cw_bool_t
res_is_equal(cw_res_t *a_a, cw_res_t *a_b)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_a);
	_cw_check_ptr(a_b);
	rwl_wlock(&a_a->rw_lock);
	rwl_rlock(&a_b->rw_lock);

	if (a_a == a_b) {
		/* Two pointers to the same instance. */
		retval = TRUE;
	} else if (dch_count(&a_a->hash) != dch_count(&a_b->hash))
		retval = FALSE;
	else {
		cw_uint32_t	i, num_resources;
		char		*key, *val;

		num_resources = dch_count(&a_a->hash);

		for (i = 0, retval = FALSE; (i < num_resources) && (retval ==
		    FALSE); i++) {
			dch_remove_iterate(&a_a->hash, (void **)&key, (void
			    **)&val, NULL);

			if (res_res_val_get(a_b, key) == NULL)
				retval = TRUE;
			dch_insert(&a_a->hash, key, val, NULL);
		}
	}

	rwl_runlock(&a_b->rw_lock);
	rwl_wunlock(&a_a->rw_lock);
	return retval;
}

cw_bool_t
res_file_merge(cw_res_t *a_res, const char *a_filename)
{
	cw_bool_t	retval = FALSE, state_mach_error;
	int		error;

	_cw_check_ptr(a_res);
	rwl_wlock(&a_res->rw_lock);

	a_res->fd = fopen(a_filename, "r");
	if (a_res->fd == NULL)
		retval = TRUE;
	else {
		/* Run the state machine on the file. */
		state_mach_error = res_p_res_parse(a_res, TRUE);
		if (state_mach_error == TRUE)
			retval = TRUE;
		/* Close the file. */
		error = fclose(a_res->fd);
		if (error)
			retval = TRUE;
	}

	rwl_wunlock(&a_res->rw_lock);
	return retval;
}

cw_bool_t
res_list_merge(cw_res_t *a_res,...)
{
	va_list		ap;
	cw_bool_t	retval = FALSE, state_mach_error;

	_cw_check_ptr(a_res);
	rwl_wlock(&a_res->rw_lock);

	/* Run the strings through the insertion state machine. */
	va_start(ap, a_res);
	for (a_res->str = va_arg(ap, char *); ((a_res->str != NULL) && (retval
	    != TRUE)); a_res->str = va_arg(ap, char *)) {
		state_mach_error = res_p_res_parse(a_res, FALSE);
		if (state_mach_error == TRUE)
			retval = TRUE;
	}
	va_end(ap);

	rwl_wunlock(&a_res->rw_lock);
	return retval;
}

const char *
res_res_val_get(cw_res_t *a_res, const char *a_res_name)
{
	char		*retval;
	cw_bool_t	error;

	_cw_check_ptr(a_res);
	_cw_check_ptr(a_res_name);
	rwl_rlock(&a_res->rw_lock);

	error = dch_search(&a_res->hash, (void *)a_res_name, (void **)&retval);
	if (error == TRUE)
		retval = NULL;
	rwl_runlock(&a_res->rw_lock);
	return retval;
}

cw_bool_t
res_res_extract(cw_res_t *a_res, const char *a_res_key,
    char **r_res_name, char **r_res_val)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_res);
	rwl_wlock(&a_res->rw_lock);

	retval = dch_remove(&a_res->hash, a_res_key, (void **)r_res_name,
	    (void **)r_res_val, NULL);

	rwl_wunlock(&a_res->rw_lock);
	return retval;
}

cw_bool_t
res_dump(cw_res_t *a_res, const char *a_filename)
{
	cw_bool_t	retval;
	int		fd = -1;
	cw_out_t	*t_out = NULL;

	_cw_check_ptr(a_res);
	rwl_wlock(&a_res->rw_lock);

	if (a_filename != NULL) {
		t_out = out_new(NULL, a_res->mem);
		if (t_out == NULL) {
			retval = TRUE;
			goto RETURN;
		}
		fd = open(a_filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (fd == -1) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Error opening file \"[s]\"\n", a_filename);
			retval = TRUE;
			goto RETURN;
		}
		out_set_default_fd(t_out, fd);
	} else
		t_out = NULL;

	retval = FALSE;

	/* Now dump the resources to t_out. */
	{
		cw_uint32_t	num_items, i, j, curr_offset, val_len;
		char		*key, *val;

		num_items = dch_count(&a_res->hash);

		for (i = 0; i < num_items; i++) {
			dch_get_iterate(&a_res->hash, (void *)&key, (void
			    *)&val);

			if (out_put(t_out, "[s]:", key) < 0) {
				retval = TRUE;
				goto RETURN;
			}
			for (j = 0, curr_offset = 0, val_len = strlen(val); j <
			    val_len + 1; j++) {
				
				if (val[j] == '\n') {
					val[j] = '\0';
					if ((j < val_len) && (val[j + 1] !=
					    '\0')) {
						if (out_put(t_out, "[s]\\n\\\n",
						    (char *)(val + curr_offset))
						    < 0) {
							retval = TRUE;
							goto RETURN;
						}
					} else {
						if (out_put(t_out, "[s]\\n",
						    (char *)(val + curr_offset))
						    < 0) {
							retval = TRUE;
							goto RETURN;
						}
					}
					val[j] = '\n';
					curr_offset = j + 1;
				} else if (val[j] == '\0') {
					if (out_put(t_out, "[s]\n", (char *)(val
					    + curr_offset)) < 0) {
						retval = TRUE;
						goto RETURN;
					}
				}
			}
		}
	}

	RETURN:
	if (t_out != NULL)
		out_delete(t_out);
	if (fd != -1)
		close(fd);
	rwl_wunlock(&a_res->rw_lock);
	return retval;
}

/*
 * Parse the resources contained either in a string or in a file and insert the
 * results into the hash table.
 */
static cw_bool_t
res_p_res_parse(cw_res_t *a_res, cw_bool_t a_is_file)
{
	cw_bool_t	retval = FALSE;
	size_t		i, name_pos = 0, val_pos = 0;
	cw_uint32_t	state = _LIBSTASH_RES_STATE_START;
	cw_uint32_t	col_num, line_num = 1, name_bufsize, val_bufsize;
	char		c, *name = NULL, *val = NULL;

	name_bufsize = _LIBSTASH_RES_BUFFSIZE;
	val_bufsize = _LIBSTASH_RES_BUFFSIZE;

	name = (char *)mem_malloc(a_res->mem, name_bufsize);
	if (name == NULL)
		goto RETURN;
	val = (char *)mem_malloc(a_res->mem, val_bufsize);
	if (val == NULL)
		goto RETURN;
	for (i = 0, col_num = 1; ((state != _LIBSTASH_RES_STATE_FINISH) &&
	    (retval != TRUE)); i++, col_num++) {
		/*
		 * Check whether we overflowed the buffers, and expand them, if
		 * necessary.
		 */
		if (name_pos >= name_bufsize) {
			name_bufsize <<= 1;
			name = (char *)mem_realloc(a_res->mem, name,
			    name_bufsize);
			if (name == NULL)
				goto RETURN;
		}
		if (val_pos >= val_bufsize) {
			val_bufsize <<= 1;
			val = (char *)mem_realloc(a_res->mem, val,
			    val_bufsize);
			if (val == NULL)
				goto RETURN;
		}
		/* Read the next character in. */
		if (a_is_file) {
			c = (char)getc(a_res->fd);
			if (c == EOF) {
				/* Make sure it's an EOF, not an error. */
				if (ferror(a_res->fd)) {
					_cw_out_put("res_parse_res(): "
					    "Error reading from file\n");
					retval = TRUE;
					break;
				} else {
					/*
					 * Just in case EOF != NULL (is this
					 * possible?).
					 */
					c = '\0';
				}
			}
		} else
			c = a_res->str[i];

		if (dbg_is_registered(cw_g_dbg, "res_state")) {
			_cw_out_put("res_parse_res(): State == [i], "
			    "Input == \'[c]\'\n",
			    state, c);
		}
		switch (state) {
			/* Starting state. */
		case _LIBSTASH_RES_STATE_START:
			/* Initialize counters, buffers, etc. */
			name_pos = 0;
			val_pos = 0;
			col_num = 1;
			/*
			 * Truncate.  Not strictly necessary with static
			 * buffers.
			 */
			/* name = '\0'; */
			/* val = '\0'; */

			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
				/* First character of the name. */
				name[name_pos] = c;
				name_pos++;
				state = _LIBSTASH_RES_STATE_NAME;
				break;
			case _LIBSTASH_RES_CHAR_HASH:
				/*
				 * Beginning of comment.  Throw the character
				 * away.
				 */
				state = _LIBSTASH_RES_STATE_BEGIN_COMMENT;
				break;
			case _LIBSTASH_RES_CHAR_WHITESPACE:
				/* Leading whitespace.  Swallow it. */
				break;
			case _LIBSTASH_RES_CHAR_NEWLINE:
				/* Leading whitespace.  Swallow it. */
				line_num++;
				col_num = 1;
				break;
			case _LIBSTASH_RES_CHAR_NULL:
				/* Completely empty string or file. */
				state = _LIBSTASH_RES_STATE_FINISH;
				break;
			case _LIBSTASH_RES_CHAR_COLON:
			case _LIBSTASH_RES_CHAR_BACKSLASH:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_START,"
				    " line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		/* Whitespace that precedes the resource name. */
		case _LIBSTASH_RES_STATE_BEGIN_WHITESPACE:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
				/* First character of the name. */
				name[name_pos] = c;
				name_pos++;
				state = _LIBSTASH_RES_STATE_NAME;
				break;
			case _LIBSTASH_RES_CHAR_HASH:
				/* Beginning of a comment. */
				state = _LIBSTASH_RES_STATE_BEGIN_COMMENT;
				break;
			case _LIBSTASH_RES_CHAR_WHITESPACE:
				/* More whitespace.  Swallow it. */
				break;
			case _LIBSTASH_RES_CHAR_NEWLINE:
				/*
				 * Blank line.  Jump back to the start state to
				 * make sure all counters are correctly reset.
				 */
				line_num++;
				col_num = 1;
				state = _LIBSTASH_RES_STATE_START;
				break;
			case _LIBSTASH_RES_CHAR_NULL:
				/* String is legal, but contains no resource. */
				state = _LIBSTASH_RES_STATE_START;
				break;
			case _LIBSTASH_RES_CHAR_COLON:
			case _LIBSTASH_RES_CHAR_BACKSLASH:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_BEGIN_WHITESPACE,"
				    " line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		/* Comment that was preceded only by whitespace characters. */
		case _LIBSTASH_RES_STATE_BEGIN_COMMENT:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
			case _LIBSTASH_RES_CHAR_HASH:
			case _LIBSTASH_RES_CHAR_WHITESPACE:
			case _LIBSTASH_RES_CHAR_COLON:
			case _LIBSTASH_RES_CHAR_BACKSLASH:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
				/* Swallow the character. */
				break;
			case _LIBSTASH_RES_CHAR_NEWLINE:
				/* Go back to the initial state. */
				line_num++;
				col_num = 1;
				state = _LIBSTASH_RES_STATE_START;
				break;
			case _LIBSTASH_RES_CHAR_NULL:
				/* String is legal, but contains no resource. */
				state = _LIBSTASH_RES_STATE_FINISH;
				break;
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_BEGIN_COMMENT, "
				    "line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		/* Resource name state. */
		case _LIBSTASH_RES_STATE_NAME:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
				/* Character of the resource name. */
				name[name_pos] = c;
				name_pos++;
				break;
			case _LIBSTASH_RES_CHAR_WHITESPACE:
				/*
				 * End of name.  Jump to a state that can deal
				 * with additional whitespace.
				 */
				name[name_pos] = '\0';
				state =
				    _LIBSTASH_RES_STATE_POST_NAME_WHITESPACE;
				break;
			case _LIBSTASH_RES_CHAR_COLON:
				/*
				 * Okay, here's the colon.  Terminate the name
				 * and jump to a state that can deal with
				 * whitespace leading the value.
				 */
				name[name_pos] = '\0';
				state =
				    _LIBSTASH_RES_STATE_POST_COLON_WHITESPACE;
				break;
			case _LIBSTASH_RES_CHAR_HASH:
			case _LIBSTASH_RES_CHAR_BACKSLASH:
			case _LIBSTASH_RES_CHAR_NEWLINE:
			case _LIBSTASH_RES_CHAR_NULL:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_NAME, "
				    "line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		/* Swallow whitespace following the resource name. */
		case _LIBSTASH_RES_STATE_POST_NAME_WHITESPACE:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_COLON:
				/*
				 * Here's the colon.  Jump to a state that can
				 * deal with whitespace leading the value.
				 */
				state =
				    _LIBSTASH_RES_STATE_POST_COLON_WHITESPACE;
				break;
			case _LIBSTASH_RES_CHAR_WHITESPACE:
				/* Additional whitespace.  Swallow it. */
				break;
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
			case _LIBSTASH_RES_CHAR_HASH:
			case _LIBSTASH_RES_CHAR_BACKSLASH:
			case _LIBSTASH_RES_CHAR_NEWLINE:
			case _LIBSTASH_RES_CHAR_NULL:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_POST_NAME_WHITESPACE, "
				    "line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		/* Swallow whitespace following the colon. */
		case _LIBSTASH_RES_STATE_POST_COLON_WHITESPACE:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
			case _LIBSTASH_RES_CHAR_COLON:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
				/* Beginning of value. */
				val[val_pos] = c;
				val_pos++;
				state = _LIBSTASH_RES_STATE_VALUE;
				break;
			case _LIBSTASH_RES_CHAR_WHITESPACE:
				/* More whitespace.  Swallow it. */
				break;
			case _LIBSTASH_RES_CHAR_HASH:
				/*
				 * Empty value.  NULL-terminate the string and
				 * jump to the trailing comment state.
				 */
				val[val_pos] = '\0';
				res_p_res_merge(a_res, name, val);
				state = _LIBSTASH_RES_STATE_TRAILING_COMMENT;
				break;
			case _LIBSTASH_RES_CHAR_BACKSLASH:
				/*
				 * Beginning of value, but it's a backslash, so
				 * jump to the backslash handling state.
				 */
				state = _LIBSTASH_RES_STATE_VALUE_BACKSLASH;
				break;
			case _LIBSTASH_RES_CHAR_NEWLINE:
				/* Empty value.  Insert it though. */
				line_num++;
				col_num = 1;
				val[val_pos] = '\0';
				res_p_res_merge(a_res, name, val);
				state = _LIBSTASH_RES_STATE_START;
				break;
			case _LIBSTASH_RES_CHAR_NULL:
				/*
				 * Empty value, and end of input.  Insert the
				 * resource.
				 */
				val[val_pos] = '\0';
				res_p_res_merge(a_res, name, val);
				state = _LIBSTASH_RES_STATE_FINISH;
				break;
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_POST_COLON_WHITESPACE"
				    ", line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		/* Resource value. */
		case _LIBSTASH_RES_STATE_VALUE:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
			case _LIBSTASH_RES_CHAR_COLON:
			case _LIBSTASH_RES_CHAR_WHITESPACE:	/*
								 * Allow
								 * whitespace in
								 * value.
								 */
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
				/* More of the value. */
				val[val_pos] = c;
				val_pos++;
				state = _LIBSTASH_RES_STATE_VALUE;
				break;
			case _LIBSTASH_RES_CHAR_HASH:
				/*
				 * Beginning of comment, and therefore the end
				 * of the value. NULL-terminate the string,
				 * insert the resource, and jump to the trailing
				 * comment state.
				 */
				val[val_pos] = '\0';
				res_p_res_merge(a_res, name, val);
				state = _LIBSTASH_RES_STATE_TRAILING_COMMENT;
				break;
			case _LIBSTASH_RES_CHAR_BACKSLASH:
				/* Backslash.  Jump to the backslash state. */
				state = _LIBSTASH_RES_STATE_VALUE_BACKSLASH;
				break;
			case _LIBSTASH_RES_CHAR_NEWLINE:
				/*
				 * End of line, and therefore the end of the
				 * value. NULL-terminate the string, insert the
				 * resource in the hash table, and jump back to
				 * the starting state.
				 */
				line_num++;
				col_num = 1;
				val[val_pos] = '\0';
				res_p_res_merge(a_res, name, val);
				state = _LIBSTASH_RES_STATE_START;
				break;
			case _LIBSTASH_RES_CHAR_NULL:
				/*
				 * Do the same thing as for a newline, except
				 * that we want the state machine to exit.
				 */
				val[val_pos] = '\0';
				res_p_res_merge(a_res, name, val);
				state = _LIBSTASH_RES_STATE_FINISH;
				break;
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_VALUE, "
				    "line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		/* Backslash within the resource value. */
		case _LIBSTASH_RES_STATE_VALUE_BACKSLASH:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_BACKSLASH:
				/*
				 * Insert a backslash and jump back to the value
				 * state.
				 */
				val[val_pos] = '\\';
				val_pos++;
				state = _LIBSTASH_RES_STATE_VALUE;
				break;
			case _LIBSTASH_RES_CHAR_HASH:
				/*
				 * Insert a hash and jump back to the value
				 * state.
				 */
				val[val_pos] = '#';
				val_pos++;
				state = _LIBSTASH_RES_STATE_VALUE;
				break;
			case _LIBSTASH_RES_CHAR_WHITESPACE:
				/*
				 * We need to make sure that what follows is
				 * whitespace followed by a newline, but we
				 * can't do that if we stay in this state, since
				 * we woudn't notice '\\' and '#' later on in
				 * the stream.
				 */
				state = _LIBSTASH_RES_STATE_BACKSLASH_WHITESPACE;
				break;
			case _LIBSTASH_RES_CHAR_NEWLINE:
				/*
				 * \ continuation.  Swallow this and jump back
				 * to the value state.
				 */
				line_num++;
				col_num = 1;
				state = _LIBSTASH_RES_STATE_VALUE;
				break;
			case _LIBSTASH_RES_CHAR_LOWER:
				if (c == 'n') {
					/*
					 * Insert a carriage return and jump
					 * back to the value state.  Yes, this
					 * is a gross hack, and violation of the
					 * otherwise purity of the state machine
					 * and character classes, but to have
					 * made a special character class just
					 * for this would probably have made the
					 * code less understandable.
					 */
					val[val_pos] = '\n';
					val_pos++;
					state = _LIBSTASH_RES_STATE_VALUE;
					break;
				}
				/*
				 * Note that if it's not an 'n', we fall through
				 * to the error case.
				 */
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
			case _LIBSTASH_RES_CHAR_COLON:
			case _LIBSTASH_RES_CHAR_NULL:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_VALUE_BACKSLASH, "
				    "line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		case _LIBSTASH_RES_STATE_BACKSLASH_WHITESPACE:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_WHITESPACE:
				/* Swallow the character. */
				break;
			case _LIBSTASH_RES_CHAR_NEWLINE:
				/*
				 * \ continuation.  Swallow this and jump back
				 * to the value state.
				 */
				line_num++;
				col_num = 1;
				state = _LIBSTASH_RES_STATE_VALUE;
				break;
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
			case _LIBSTASH_RES_CHAR_HASH:
			case _LIBSTASH_RES_CHAR_COLON:
			case _LIBSTASH_RES_CHAR_BACKSLASH:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
			case _LIBSTASH_RES_CHAR_NULL:
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_BACKSLASH_WHITESPACE, "
				    "line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		/* Comment at end of resource. */
		case _LIBSTASH_RES_STATE_TRAILING_COMMENT:
			switch (res_p_char_type(c)) {
			case _LIBSTASH_RES_CHAR_CAP:
			case _LIBSTASH_RES_CHAR_LOWER:
			case _LIBSTASH_RES_CHAR_NUMBER:
			case _LIBSTASH_RES_CHAR_UNDER:
			case _LIBSTASH_RES_CHAR_PERIOD:
			case _LIBSTASH_RES_CHAR_HASH:
			case _LIBSTASH_RES_CHAR_WHITESPACE:
			case _LIBSTASH_RES_CHAR_COLON:
			case _LIBSTASH_RES_CHAR_BACKSLASH:
			case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
				/* Swallow the character. */
				break;
			case _LIBSTASH_RES_CHAR_NEWLINE:
				/*
				 * Okay, end of comment.  Jump back to the
				 * starting state.
				 */
				line_num++;
				col_num = 1;
				state = _LIBSTASH_RES_STATE_START;
				break;
			case _LIBSTASH_RES_CHAR_NULL:
				/* End of input.  Finish. */
				state = _LIBSTASH_RES_STATE_FINISH;
				break;
			case _LIBSTASH_RES_CHAR_OTHER:
			default:
				/* Error. */
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Illegal character while in "
				    "_LIBSTASH_RES_STATE_TRAILING_COMMENT, "
				    "line [i], column [i]\n",
				    line_num, col_num);
				retval = TRUE;
				break;
			}
			break;
		default:
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Jumped to non-existant state, line [i], "
			    "column [i]\n", line_num, col_num);
			retval = TRUE;
			break;
		}
	}

	RETURN:
	if (NULL != name)
		mem_free(a_res->mem, name);
	if (NULL != val)
		mem_free(a_res->mem, val);
	return retval;
}

/*
 * Returns the "type" of a_char.
 */
static cw_uint32_t
res_p_char_type(char a_char)
{
	cw_uint32_t retval;

	switch (a_char) {
		/* Capital letters. */
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	case 'V': case 'W': case 'X': case 'Y': case 'Z':
		retval = _LIBSTASH_RES_CHAR_CAP;
		break;

		/* Lower case letters. */
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	case 'v': case 'w': case 'x': case 'y': case 'z':
		retval = _LIBSTASH_RES_CHAR_LOWER;
		break;

		/* Numbers. */
	case '0': case '1': case '2': case '3': case '4': case '5': case '6':
	case '7': case '8': case '9':
		retval = _LIBSTASH_RES_CHAR_NUMBER;
		break;

		/* Underscore. */
	case '_':
		retval = _LIBSTASH_RES_CHAR_UNDER;
		break;

		/* Period. */
	case '.':
		retval = _LIBSTASH_RES_CHAR_PERIOD;
		break;

		/* Start comment. */
	case '#':
		retval = _LIBSTASH_RES_CHAR_HASH;
		break;

		/* Whitespace. */
	case '\t': case ' ':
		retval = _LIBSTASH_RES_CHAR_WHITESPACE;
		break;

		/* Colon. */
	case ':':
		retval = _LIBSTASH_RES_CHAR_COLON;
		break;

		/* Backslash. */
	case '\\':
		retval = _LIBSTASH_RES_CHAR_BACKSLASH;
		break;

		/* Carriage return. */
	case '\n':
		retval = _LIBSTASH_RES_CHAR_NEWLINE;
		break;

		/* Null terminator. */
	case '\0':
		retval = _LIBSTASH_RES_CHAR_NULL;
		break;

		/* Other valid characters within a resource values. */
	case '!': case '"': case '$': case '%': case '&': case '\'': case '(':
	case ')': case '*': case '+': case ',': case '-': case '/': case ';':
	case '<': case '=': case '>': case '?': case '@': case '[': case ']':
	case '^': case '`': case '{': case '|': case '}': case '~':
		retval = _LIBSTASH_RES_CHAR_VALID_IN_VAL;
		break;

	/* Something we're not expecting at all. */
	default:
		retval = _LIBSTASH_RES_CHAR_OTHER;
		break;
	}

	return retval;
}

/*
 * Merge a resource into the hash table, taking care to clean up any entry it
 * replaces.
 */
static void
res_p_res_merge(cw_res_t *a_res, const char *a_name, const char *a_val)
{
	char	*temp_name, *temp_val;

	/* Make copies to insert into the hash table. */
	temp_name = (char *)mem_malloc(a_res->mem, strlen(a_name) + 1);
	strcpy(temp_name, a_name);

	xep_begin();
	xep_try {
		temp_val = (char *)mem_malloc(a_res->mem, strlen(a_val) +
		    1);
	}
	xep_catch(_CW_XEPV_OOM) {
		mem_free(a_res->mem, temp_name);
	}
	xep_end();

	strcpy(temp_val, a_val);

	if (dbg_is_registered(cw_g_dbg, "res_state")) {
		_cw_out_put("res_res_merge(): Merging name == :[s]:, "
		    "value == :[s]:\n", a_name, a_val);
	}
	/* Insert the resource into the hash table. */
	if (dch_search(&a_res->hash, (void *)temp_name, NULL) == FALSE) {
		char	*old_name, *old_val;

		/*
		 * The resource already exists.  That means we need to
		 * delete the existing one, free the resources that are
		 * taken up by it, and redo the insertion.
		 */
		dch_remove(&a_res->hash, (void *)temp_name, (void **)&old_name,
		    (void **)&old_val, NULL);
		mem_free(a_res->mem, old_name);
		mem_free(a_res->mem, old_val);

		dch_insert(&a_res->hash, (void *)temp_name, (void *)temp_val,
		    NULL);
	}

	xep_begin();
	xep_try {
		dch_insert(&a_res->hash, (void *)temp_name, (void *)temp_val,
		    NULL);
	}
	xep_catch(_CW_XEPV_OOM) {
		/*
		 * We may have removed the old definition of the resource
		 * without inserting the new definition, which means that state
		 * could have changed even though this call was a failure.  Oh
		 * well.
		 */
		mem_free(a_res->mem, temp_name);
		mem_free(a_res->mem, temp_val);
	}
	xep_end();
}
