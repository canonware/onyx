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

#include <stdarg.h>
#include <ctype.h>

#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_string_l.h"

#define		stiloe_p_string_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_string_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

void
stilo_string_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking,
    cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)_cw_malloc(sizeof(cw_stiloe_string_t));

	stiloe_l_new(&string->stiloe, STILOT_STRING, a_locking);
	if (a_locking)
		mtx_new(&string->lock);
	string->e.s.len = a_len;
	if (string->e.s.len > 0) {
		string->e.s.str = (cw_uint8_t *)_cw_malloc(string->e.s.len);
		memset(string->e.s.str, 0, string->e.s.len);
	} else
		string->e.s.str = NULL;

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)string;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_STRING;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)string);
}

void
stilo_string_substring_new(cw_stilo_t *a_stilo, cw_stilo_t *a_string, cw_stil_t
    *a_stil, cw_uint32_t a_offset, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string, *orig;

	_cw_check_ptr(a_stilo);

	orig = (cw_stiloe_string_t *)a_string->o.stiloe;
	_cw_check_ptr(orig);
	_cw_assert(orig->stiloe.magic == _CW_STILOE_MAGIC);

	if (orig->stiloe.indirect) {
		stilo_string_substring_new(a_stilo, &orig->e.i.stilo, a_stil,
		    a_offset + orig->e.i.beg_offset, a_len);
	} else {
		_cw_assert(a_offset + a_len <= orig->e.s.len);

		string = (cw_stiloe_string_t
		    *)_cw_malloc(sizeof(cw_stiloe_string_t));

		stiloe_l_new(&string->stiloe, STILOT_STRING, FALSE);
		string->stiloe.indirect = TRUE;
		memcpy(&string->e.i.stilo, a_string, sizeof(cw_stilo_t));
		string->e.i.beg_offset = a_offset;
		string->e.i.len = a_len;

		memset(a_stilo, 0, sizeof(cw_stilo_t));
		a_stilo->o.stiloe = (cw_stiloe_t *)string;
#ifdef _LIBSTIL_DBG
		a_stilo->magic = _CW_STILO_MAGIC;
#endif
		a_stilo->type = STILOT_STRING;

		stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t
		    *)string);
	}
}

void
stiloe_l_string_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)a_stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE && string->e.s.len > 0)
		_CW_FREE(string->e.s.str);

	if (string->stiloe.locking && string->stiloe.indirect == FALSE)
		mtx_delete(&string->lock);

	_CW_STILOE_FREE(string);
}

cw_stiloe_t *
stiloe_l_string_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)a_stiloe;

	if (a_reset)
		string->ref_iter = 0;

	if (a_stiloe->indirect == FALSE)
		retval = NULL;
	else if (string->ref_iter == 0) {
		retval = string->e.i.stilo.o.stiloe;
		string->ref_iter++;
	} else
		retval = NULL;

	return retval;
}

void
stilo_l_string_print(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *depth, *string, *stdout_stilo;
	cw_stilo_threade_t	error;
	cw_uint8_t		*str;
	cw_sint32_t		len;
	cw_uint32_t		i;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(depth, ostack, a_thread);
	STILO_STACK_DOWN_GET(string, ostack, a_thread, depth);
	if (stilo_type_get(depth) != STILOT_INTEGER || stilo_type_get(string)
	    != STILOT_STRING) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stdout_stilo = stil_stdout_get(stilo_thread_stil_get(a_thread));

	str = stilo_string_get(string);
	len = stilo_string_len_get(string);

	stilo_file_output(stdout_stilo, "`");
	for (i = 0; i < len; i++) {
		switch (str[i]) {
		case '\n':
			error = stilo_file_output(stdout_stilo, "\\n");
			break;
		case '\r':
			error = stilo_file_output(stdout_stilo, "\\r");
			break;
		case '\t':
			error = stilo_file_output(stdout_stilo, "\\t");
			break;
		case '\b':
			error = stilo_file_output(stdout_stilo, "\\b");
			break;
		case '\f':
			error = stilo_file_output(stdout_stilo, "\\f");
			break;
		case '\\':
			error = stilo_file_output(stdout_stilo, "\\\\");
			break;
		case '`':
			error = stilo_file_output(stdout_stilo, "\\`");
			break;
		case '\'':
			error = stilo_file_output(stdout_stilo, "\\'");
			break;
		default:
			if (isprint(str[i]))
				error = stilo_file_output(stdout_stilo, "[c]",
				    str[i]);
			else {
				error = stilo_file_output(stdout_stilo,
				    "\\x[i|b:16|w:2|p:0]", str[i]);
			}
			break;
		}
		if (error) {
			stilo_thread_error(a_thread, error);
			return;
		}
	}
	error = stilo_file_output(stdout_stilo, "'");
	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_npop(ostack, 2);
}

void
stilo_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	cw_stiloe_string_t	*string_fr, *string_fr_i = NULL, *string_fr_l;
	cw_stiloe_string_t	*string_to, *string_to_i = NULL, *string_to_l;
	cw_uint8_t		*str_fr, *str_to;
	cw_uint32_t		len_fr, len_to;

	/*
	 * Set string pointers.
	 */
	string_fr = (cw_stiloe_string_t *)a_from->o.stiloe;
	if (string_fr->stiloe.indirect) {
		string_fr_i = (cw_stiloe_string_t
		    *)string_fr->e.i.stilo.o.stiloe;
	}
	string_to = (cw_stiloe_string_t *)a_to->o.stiloe;
	if (string_to->stiloe.indirect) {
		string_to_i = (cw_stiloe_string_t
		    *)string_to->e.i.stilo.o.stiloe;
	}

	/*
	 * Set str_fr and len_fr according to whether string_fr is an indirect
	 * object.
	 */
	if (string_fr_i != NULL) {
		string_fr_l = string_fr_i;
		str_fr = &string_fr_i->e.s.str[string_fr->e.i.beg_offset];
		len_fr = string_fr->e.i.len;
		_cw_assert(len_fr + string_fr->e.i.beg_offset <=
		    string_fr_i->e.s.len);
	} else {
		string_fr_l = string_fr;
		str_fr = string_fr->e.s.str;
		len_fr = string_fr->e.s.len;
	}

	/*
	 * Set str_to and len_to according to whether string_to is an indirect
	 * object.
	 */
	if (string_to_i != NULL) {
		string_to_l = string_to_i;
		str_to = &string_to_i->e.s.str[string_to->e.i.beg_offset];
		len_to = string_to->e.i.len;
	} else {
		string_to_l = string_to;
		str_to = string_to->e.s.str;
		len_to = string_to->e.s.len;
	}

	/* Make sure destination is large enough. */
	_cw_assert(len_fr <= len_to);

	/*
	 * Iteratively copy elements.  Only copy one level deep (not
	 * recursively), by using dup.
	 */
	stiloe_p_string_lock(string_fr_l);
	stiloe_p_string_lock(string_to_l);
	memcpy(str_to, str_fr, len_fr);
	stiloe_p_string_unlock(string_fr_l);

	/*
	 * Truncate the destination string if it is shorter than the source
	 * string.
	 */
	if (len_to > len_fr) {
		if (string_to_i != NULL)
			string_to->e.i.len = len_fr;
		else
			string_to->e.s.len = len_fr;
	}
	stiloe_p_string_unlock(string_to_l);
}

cw_uint32_t
stilo_string_len_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE)
		retval = string->e.s.len;
	else
		retval = string->e.i.len;

	return retval;
}

void
stilo_string_el_get(cw_stilo_t *a_stilo, cw_stiloi_t a_offset, cw_uint8_t *r_el)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE) {
		_cw_assert(a_offset >= 0 && a_offset < string->e.s.len);
		*r_el = string->e.s.str[a_offset];
	} else {
		stilo_string_el_get(&string->e.i.stilo, a_offset +
		    string->e.i.beg_offset, r_el);
	}
}

void
stilo_string_el_set(cw_stilo_t *a_stilo, cw_uint8_t a_el, cw_stiloi_t a_offset)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE) {
		_cw_assert(a_offset >= 0 && a_offset < string->e.s.len);
		string->e.s.str[a_offset] = a_el;
	} else {
		stilo_string_el_set(&string->e.i.stilo, a_el, a_offset +
		    string->e.i.beg_offset);
	}
}

void
stilo_string_lock(cw_stilo_t *a_stilo)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE)
		stiloe_p_string_lock(string);
	else
		stilo_string_lock(&string->e.i.stilo);
}

void
stilo_string_unlock(cw_stilo_t *a_stilo)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE)
		stiloe_p_string_unlock(string);
	else
		stilo_string_unlock(&string->e.i.stilo);
}

cw_uint8_t *
stilo_string_get(cw_stilo_t *a_stilo)
{
	cw_uint8_t		*retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE)
		retval = string->e.s.str;
	else {
		retval = &stilo_string_get(&string->e.i.stilo)
		    [string->e.i.beg_offset];
	}

	return retval;
}

void
stilo_string_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset, const cw_uint8_t
    *a_str, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string;
	cw_uint8_t		*str;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	/* Get the string pointer. */
	if (string->stiloe.indirect == FALSE) {
		_cw_assert(a_offset + a_len <= string->e.s.len);
		str = string->e.s.str;

		memcpy(&str[a_offset], a_str, a_len);
	} else
		stilo_string_set(&string->e.i.stilo, a_offset, a_str, a_len);
}
