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

/* Compile non-inlined functions if not using inlines. */
#define	_STILO_ARRAY_C_

#include "../include/libstil/libstil.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_array_l.h"
#include "../include/libstil/stila_l.h"

void
stilo_array_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking,
    cw_uint32_t a_len)
{
	cw_stiloe_array_t	*array;
	cw_uint32_t		i;

	array = (cw_stiloe_array_t *)_cw_malloc(sizeof(cw_stiloe_array_t));

	stiloe_l_new(&array->stiloe, STILOT_ARRAY, a_locking);
	if (a_locking)
		mtx_new(&array->lock);
	array->e.a.len = a_len;
	if (array->e.a.len > 0) {
		array->e.a.arr = (cw_stilo_t *)_cw_malloc(sizeof(cw_stilo_t) *
		    array->e.a.len);
		for (i = 0; i < array->e.a.len; i++)
			stilo_null_new(&array->e.a.arr[i]);
	}

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)array;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_ARRAY;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)array);
}

void
stilo_array_subarray_new(cw_stilo_t *a_stilo, cw_stilo_t *a_array, cw_stil_t
    *a_stil, cw_uint32_t a_offset, cw_uint32_t a_len)
{
	cw_stiloe_array_t	*array, *orig;

	_cw_check_ptr(a_stilo);

	orig = (cw_stiloe_array_t *)a_array->o.stiloe;
	_cw_check_ptr(orig);
	_cw_assert(orig->stiloe.magic == _CW_STILOE_MAGIC);

	if (orig->stiloe.indirect) {
		stilo_array_subarray_new(a_stilo, &orig->e.i.stilo, a_stil,
		    a_offset + orig->e.i.beg_offset, a_len);
	} else {
		_cw_assert(a_offset + a_len <= orig->e.a.len);

		array = (cw_stiloe_array_t
		    *)_cw_malloc(sizeof(cw_stiloe_array_t));

		stiloe_l_new(&array->stiloe, STILOT_ARRAY, FALSE);
		array->stiloe.indirect = TRUE;
		memcpy(&array->e.i.stilo, a_array, sizeof(cw_stilo_t));
		array->e.i.beg_offset = a_offset;
		array->e.i.len = a_len;

		memset(a_stilo, 0, sizeof(cw_stilo_t));
		a_stilo->o.stiloe = (cw_stiloe_t *)array;
#ifdef _LIBSTIL_DBG
		a_stilo->magic = _CW_STILO_MAGIC;
#endif
		a_stilo->type = STILOT_ARRAY;

		stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t
		    *)array);
	}
}

void
stiloe_l_array_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_array_t	*array;

	array = (cw_stiloe_array_t *)a_stiloe;

	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	if (array->stiloe.indirect == FALSE && array->e.a.len > 0)
		_CW_FREE(array->e.a.arr);

	if (array->stiloe.locking && array->stiloe.indirect == FALSE)
		mtx_delete(&array->lock);

	_CW_STILOE_FREE(array);
}

cw_stiloe_t *
stiloe_l_array_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_array_t	*array;

	array = (cw_stiloe_array_t *)a_stiloe;

	if (a_reset)
		array->ref_iter = 0;

	if (a_stiloe->indirect) {
		if (array->ref_iter == 0) {
			retval = array->e.i.stilo.o.stiloe;
			array->ref_iter++;
		} else
			retval = NULL;
	} else {
		retval = NULL;
		while (retval == NULL && array->ref_iter < array->e.a.len) {
			retval =
			    stilo_stiloe_get(&array->e.a.arr[array->ref_iter]);
			array->ref_iter++;
		}
	}

	return retval;
}

cw_stilte_t
stilo_l_array_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	if (a_depth > 0) {
		cw_stilo_t	el;
		cw_uint32_t	nelms, i;

		if (a_stilo->attrs == STILOA_EXECUTABLE) {
			retval = stilo_file_output(a_file, "{");
			if (retval)
				goto RETURN;
		} else {
			retval = stilo_file_output(a_file, "[[");
			if (retval)
				goto RETURN;
		}

		nelms = stilo_array_len_get(a_stilo);
		for (i = 0; i < nelms; i++) {
			stilo_array_el_get(a_stilo, i, &el);
			retval = stilo_print(&el, a_file, a_depth - 1, FALSE);
			if (retval)
				goto RETURN;

			if (i < nelms - 1) {
				retval = stilo_file_output(a_file, " ");
				if (retval)
					goto RETURN;
			}
		}
		if (a_stilo->attrs == STILOA_EXECUTABLE) {
			retval = stilo_file_output(a_file, "}");
			if (retval)
				goto RETURN;
		} else {
			retval = stilo_file_output(a_file, "]");
			if (retval)
				goto RETURN;
		}
	} else {
		retval = stilo_file_output(a_file, "-array-");
		if (retval)
			goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

void
stilo_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	cw_stiloe_array_t	*array_fr, *array_fr_i = NULL, *array_fr_l;
	cw_stiloe_array_t	*array_to, *array_to_i = NULL, *array_to_l;
	cw_stilo_t		*arr_fr, *arr_to;
	cw_uint32_t		i, len_fr, len_to;

	/*
	 * Set array pointers.
	 */
	array_fr = (cw_stiloe_array_t *)a_from->o.stiloe;
	if (array_fr->stiloe.indirect)
		array_fr_i = (cw_stiloe_array_t *)array_fr->e.i.stilo.o.stiloe;
	array_to = (cw_stiloe_array_t *)a_to->o.stiloe;
	if (array_to->stiloe.indirect)
		array_to_i = (cw_stiloe_array_t *)array_to->e.i.stilo.o.stiloe;

	/*
	 * Set arr_fr and len_fr according to whether array_fr is an indirect
	 * object.
	 */
	if (array_fr_i != NULL) {
		array_fr_l = array_fr_i;
		arr_fr = &array_fr_i->e.a.arr[array_fr->e.i.beg_offset];
		len_fr = array_fr->e.i.len;
		_cw_assert(len_fr + array_fr->e.i.beg_offset <=
		    array_fr_i->e.a.len);
	} else {
		array_fr_l = array_fr;
		arr_fr = array_fr->e.a.arr;
		len_fr = array_fr->e.a.len;
	}

	/*
	 * Set arr_to and len_to according to whether array_to is an indirect
	 * object.
	 */
	if (array_to_i != NULL) {
		array_to_l = array_to_i;
		arr_to = &array_to_i->e.a.arr[array_to->e.i.beg_offset];
		len_to = array_to->e.i.len;
	} else {
		array_to_l = array_to;
		arr_to = array_to->e.a.arr;
		len_to = array_to->e.a.len;
	}

	/* Make sure destination is large enough. */
	_cw_assert(len_fr <= len_to);

	/*
	 * Iteratively copy elements.  Only copy one level deep (not
	 * recursively), by using dup.
	 */
	stiloe_p_array_lock(array_fr_l);
	stiloe_p_array_lock(array_to_l);
	for (i = 0; i < len_fr; i++) {
		stilo_no_new(&arr_to[i]);
		stilo_dup(&arr_to[i], &arr_fr[i]);
	}
	stiloe_p_array_unlock(array_fr_l);

	/*
	 * Truncate the destination array if it is shorter than the source
	 * array.
	 */
	if (len_to > len_fr) {
		if (array_to_i != NULL)
			array_to->e.i.len = len_fr;
		else
			array_to->e.a.len = len_fr;
	}
	stiloe_p_array_unlock(array_to_l);
}

cw_uint32_t
stilo_array_len_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	if (array->stiloe.indirect == FALSE)
		retval = array->e.a.len;
	else
		retval = array->e.i.len;

	return retval;
}

void
stilo_array_el_get(cw_stilo_t *a_stilo, cw_stiloi_t a_offset, cw_stilo_t *r_el)
{
	stilo_l_array_el_get(a_stilo, a_offset, r_el);
}

void
stilo_array_el_set(cw_stilo_t *a_stilo, cw_stilo_t *a_el, cw_stiloi_t a_offset)
{
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;

	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	stiloe_p_array_lock(array);
	if (array->stiloe.indirect == FALSE) {
		_cw_assert(a_offset < array->e.a.len && a_offset >= 0);
		stilo_no_new(&array->e.a.arr[a_offset]);
		stilo_dup(&array->e.a.arr[a_offset], a_el);
	} else {
		stilo_array_el_set(&array->e.i.stilo, a_el, a_offset +
		    array->e.i.beg_offset);
	}
	stiloe_p_array_unlock(array);
}

/*
 * This function is unsafe to use for arrays with locking, so the public API
 * forces the use of stilo_array_el_get() instead.
 * However, the GC needs to cache pointers to the arrays in gcdict for
 * performance reasons, so it uses this function.
 */
cw_stilo_t *
stilo_l_array_get(cw_stilo_t *a_stilo)
{
	cw_stilo_t		*retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;

	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	if (array->stiloe.indirect == FALSE)
		retval = array->e.a.arr;
	else {
		retval = &stilo_l_array_get(&array->e.i.stilo)
		    [array->e.i.beg_offset];
	}

	return retval;
}
