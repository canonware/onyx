/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstil/libstil.h"

cw_stiloe_t *
stiloe_l_string_new(cw_stilt_t *a_stilt)
{
	cw_stiloe_string_t	*retval;

	retval = (cw_stiloe_string_t *)_cw_stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_string_t));

	retval->iterations = 0;
	retval->e.s.str = NULL;
	retval->e.s.len = -1;

	return (cw_stiloe_t *)retval;
}

void
stiloe_l_string_delete(cw_stiloe_t *a_stiloe)
{
	cw_stiloe_string_t	*stiloe = (cw_stiloe_string_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);
	_cw_assert(a_stiloe->composite == FALSE);

	if (stiloe->e.s.len != -1)
		_cw_stilt_free(a_stiloe->stilt, stiloe->e.s.str);
}

cw_stiloe_t *
stiloe_l_string_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_string_t	*stiloe = (cw_stiloe_string_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	if (a_reset)
		stiloe->iterations = 0;

	if (a_stiloe->composite == FALSE)
		retval = NULL;
	else if (stiloe->iterations == 0) {
		retval = stiloe->e.stiloec.stiloe;
		stiloe->iterations++;
	} else {
		retval = NULL;
		stiloe->iterations = 0;
	}

	return retval;
}

cw_sint32_t
stiloe_string_len_get(cw_stiloe_t *a_stiloe)
{
	cw_sint32_t		retval;
	cw_stiloe_string_t	*stiloe = (cw_stiloe_string_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	if (a_stiloe->composite == FALSE)
		retval = stiloe->e.s.len;
	else
		retval = stiloe->e.stiloec.len;

	return retval;
}

void
stiloe_string_len_set(cw_stiloe_t *a_stiloe, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*stiloe = (cw_stiloe_string_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);
	_cw_assert(a_stiloe->composite == FALSE);
	_cw_assert(stiloe->e.s.len == -1);

	if (a_len > 0) {
		stiloe->e.s.str = (cw_uint8_t
		    *)_cw_stilt_malloc(a_stiloe->stilt, a_len);
	}
	stiloe->e.s.len = a_len;
}

cw_uint8_t *
stiloe_string_el_get(cw_stiloe_t *a_stiloe, cw_uint32_t a_offset)
{
	cw_uint8_t		*retval;
	cw_stiloe_string_t	*stiloe = (cw_stiloe_string_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	if (a_stiloe->composite == FALSE) {
		_cw_assert(stiloe->e.s.len > a_offset);
		retval = &stiloe->e.s.str[a_offset];
	} else {
		retval = &stiloe_string_el_get(stiloe->e.stiloec.stiloe,
		    a_offset)[stiloe->e.stiloec.beg_offset];
	}

	return retval;
}

cw_uint8_t *
stiloe_string_get(cw_stiloe_t *a_stiloe)
{
	cw_uint8_t		*retval;
	cw_stiloe_string_t	*stiloe = (cw_stiloe_string_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	if (a_stiloe->composite == FALSE) {
		_cw_assert(stiloe->e.s.len != -1);
		retval = stiloe->e.s.str;
	} else {
		retval = &stiloe_string_get(stiloe->e.stiloec.stiloe)
		    [stiloe->e.stiloec.beg_offset];
	}

	return retval;
}

void
stiloe_string_set(cw_stiloe_t *a_stiloe, cw_uint32_t a_offset, const cw_uint8_t
    *a_str, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*stiloe = (cw_stiloe_string_t *)a_stiloe;
	cw_uint8_t		*str;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	/* Get the string pointer. */
	if (a_stiloe->composite == FALSE) {
		_cw_assert(stiloe->e.s.len >= a_len);
		str = stiloe->e.s.str;
	} else {
		_cw_assert(stiloe->e.stiloec.len >= a_len);
		str = &stiloe_string_get(stiloe->e.stiloec.stiloe)
		    [stiloe->e.stiloec.beg_offset];
	}

	/* Set the string. */
	memcpy(str, a_str, a_len);
}
