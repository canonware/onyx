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

void
stiloe_l_array_init(cw_stiloe_t *a_stiloe)
{
	cw_stiloe_array_t	*stiloe = (cw_stiloe_array_t *)a_stiloe;

	stiloe->iterations = 0;
	stiloe->e.a.arr = NULL;
	stiloe->e.a.len = -1;
}

void
stiloe_l_array_delete(cw_stiloe_t *a_stiloe)
{
	cw_stiloe_array_t	*stiloe = (cw_stiloe_array_t *)a_stiloe;
	cw_uint32_t		i;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);
	_cw_assert(a_stiloe->composite == FALSE);

	if (stiloe->e.a.len != -1) {
		for (i = 0; i < stiloe->e.a.len; i++)
			stilo_delete(&stiloe->e.a.arr[i]);
	}
}

cw_stiloe_t *
stiloe_array_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_array_t	*stiloe = (cw_stiloe_array_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	if (a_reset)
		stiloe->iterations = 0;

	if (a_stiloe->composite == FALSE)
		retval = NULL;
	else if (stiloe->iterations < stiloe->e.a.len) {
		retval = stiloe->e.stiloec.stiloe;
		stiloe->iterations++;
	} else {
		retval = NULL;
		stiloe->iterations = 0;
	}

	return retval;
}

cw_sint32_t
stiloe_array_len_get(cw_stiloe_t *a_stiloe)
{
	cw_sint32_t		retval;
	cw_stiloe_array_t	*stiloe = (cw_stiloe_array_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	if (a_stiloe->composite == FALSE)
		retval = stiloe->e.a.len;
	else
		retval = stiloe->e.stiloec.len;

	return retval;
}

void
stiloe_array_len_set(cw_stiloe_t *a_stiloe, cw_uint32_t a_len)
{
	cw_stiloe_array_t	*stiloe = (cw_stiloe_array_t *)a_stiloe;
	cw_uint32_t		i;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);
	_cw_assert(a_stiloe->composite == FALSE);
	_cw_assert(stiloe->e.a.len == -1);

	if (a_len > 0) {
		stiloe->e.a.arr = (cw_stilo_t
		    *)_cw_stilt_malloc(a_stiloe->stilt, sizeof(cw_stilo_t) *
		    a_len);
		for (i = 0; i < a_len; i++)
			stilo_new(&stiloe->e.a.arr[i]);
	}
	stiloe->e.a.len = a_len;
}

cw_stilo_t *
stiloe_array_el_get(cw_stiloe_t *a_stiloe, cw_uint32_t a_offset)
{
	cw_stilo_t		*retval;
	cw_stiloe_array_t	*stiloe = (cw_stiloe_array_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	if (a_stiloe->composite == FALSE) {
		_cw_assert(stiloe->e.a.len > a_offset);
		retval = &stiloe->e.a.arr[a_offset];
	} else {
		retval = &stiloe_array_el_get(stiloe->e.stiloec.stiloe,
		    a_offset)[stiloe->e.stiloec.beg_offset];
	}

	return retval;
}

cw_stilo_t *
stiloe_array_get(cw_stiloe_t *a_stiloe)
{
	cw_stilo_t		*retval;
	cw_stiloe_array_t	*stiloe = (cw_stiloe_array_t *)a_stiloe;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	if (a_stiloe->composite == FALSE) {
		_cw_assert(stiloe->e.a.len != -1);
		retval = stiloe->e.a.arr;
	} else {
		retval = &stiloe_array_get(stiloe->e.stiloec.stiloe)
		    [stiloe->e.stiloec.beg_offset];
	}

	return retval;
}

void
stiloe_array_set(cw_stiloe_t *a_stiloe, cw_uint32_t a_offset, cw_stilo_t *a_arr,
    cw_uint32_t a_len)
{
	cw_stiloe_array_t	*stiloe = (cw_stiloe_array_t *)a_stiloe;
	cw_stilo_t		*arr;
	cw_uint32_t		i;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	/* Get the array pointer. */
	if (a_stiloe->composite == FALSE) {
		_cw_assert(stiloe->e.a.len >= a_len);
		arr = stiloe->e.a.arr;
	} else {
		_cw_assert(stiloe->e.stiloec.len > a_len);
		arr = &stiloe_array_get(stiloe->e.stiloec.stiloe)
		    [stiloe->e.stiloec.beg_offset];
	}

	/* Set the array. */
	for (i = 0; i < a_len; i++)
		stilo_copy(&arr[i], &a_arr[i]);
}
