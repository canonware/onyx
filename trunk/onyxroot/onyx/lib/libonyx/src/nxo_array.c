/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

/* Compile non-inlined functions if not using inlines. */
#define	CW_NXO_ARRAY_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_array_l.h"
#include "../include/libonyx/nxa_l.h"

void
nxo_array_new(cw_nxo_t *a_nxo, bool a_locking, uint32_t a_len)
{
    cw_nxoe_array_t *array;
    uint32_t i;

    array = (cw_nxoe_array_t *) nxa_malloc(sizeof(cw_nxoe_array_t));

    nxoe_l_new(&array->nxoe, NXOT_ARRAY, a_locking);
#ifdef CW_THREADS
    if (a_locking)
    {
	mtx_new(&array->lock);
    }
#endif
    array->e.a.len = a_len;
    array->e.a.alloc_len = a_len;
    if (array->e.a.len > 0)
    {
	array->e.a.arr = (cw_nxo_t *) nxa_malloc(sizeof(cw_nxo_t) *
						 array->e.a.alloc_len);
	for (i = 0; i < array->e.a.len; i++)
	{
	    nxo_null_new(&array->e.a.arr[i]);
	}
    }

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) array;
    nxo_p_type_set(a_nxo, NXOT_ARRAY);

    nxa_l_gc_register((cw_nxoe_t *) array);
}

void
nxo_array_subarray_new(cw_nxo_t *a_nxo, cw_nxo_t *a_array, uint32_t a_offset,
		       uint32_t a_len)
{
    cw_nxoe_array_t *array, *orig;

    cw_check_ptr(a_nxo);

    orig = (cw_nxoe_array_t *) a_array->o.nxoe;
    cw_check_ptr(orig);
    cw_dassert(orig->nxoe.magic == CW_NXOE_MAGIC);

    array = (cw_nxoe_array_t *) nxa_malloc(sizeof(cw_nxoe_array_t));

    nxoe_l_new(&array->nxoe, NXOT_ARRAY, false);
    array->nxoe.indirect = true;

    if (orig->nxoe.indirect)
    {
	cw_assert(a_offset + a_len + orig->e.i.beg_offset
		  <= orig->e.i.array->e.a.len);
	array->e.i.array = orig->e.i.array;
	array->e.i.beg_offset = a_offset + orig->e.i.beg_offset;
    }
    else
    {
	cw_assert(a_offset + a_len <= orig->e.a.len);
	array->e.i.array = orig;
	array->e.i.beg_offset = a_offset;
    }
    array->e.i.len = a_len;

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) array;
    nxo_p_type_set(a_nxo, NXOT_ARRAY);

    nxa_l_gc_register((cw_nxoe_t *) array);
}

void
nxo_array_copy(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
    cw_nxoe_array_t *array_fr, *array_fr_i = NULL, *array_fr_l;
    cw_nxoe_array_t *array_to, *array_to_i = NULL, *array_to_l;
    cw_nxo_t *arr_fr, *arr_to;
    uint32_t i, len_fr, len_to;
#ifdef CW_THREADS
    bool locking_fr, locking_to;
#endif

    /* Set array pointers. */
    array_fr = (cw_nxoe_array_t *) a_from->o.nxoe;
    if (array_fr->nxoe.indirect)
    {
	array_fr_i = array_fr->e.i.array;
    }
    array_to = (cw_nxoe_array_t *) a_to->o.nxoe;
    if (array_to->nxoe.indirect)
    {
	array_to_i = array_to->e.i.array;
    }

    /* Set arr_fr and len_fr according to whether array_fr is an indirect
     * object. */
    if (array_fr_i != NULL)
    {
	array_fr_l = array_fr_i;
	arr_fr = &array_fr_i->e.a.arr[array_fr->e.i.beg_offset];
	len_fr = array_fr->e.i.len;
	cw_assert(len_fr + array_fr->e.i.beg_offset <= array_fr_i->e.a.len);
    }
    else
    {
	array_fr_l = array_fr;
	arr_fr = array_fr->e.a.arr;
	len_fr = array_fr->e.a.len;
    }

    /* Set arr_to and len_to according to whether array_to is an indirect
     * object. */
    if (array_to_i != NULL)
    {
	array_to_l = array_to_i;
	arr_to = &array_to_i->e.a.arr[array_to->e.i.beg_offset];
	len_to = array_to->e.i.len;
    }
    else
    {
	array_to_l = array_to;
	arr_to = array_to->e.a.arr;
	len_to = array_to->e.a.len;
    }

    /* Make sure destination is large enough. */
    cw_assert(len_fr <= len_to);

    /* Iteratively copy elements.  Only copy one level deep (not recursively),
     * by using dup. */
#ifdef CW_THREADS
    if (array_fr_l->nxoe.locking && array_fr_l->nxoe.indirect == false)
    {
	locking_fr = true;
	mtx_lock(&array_fr_l->lock);
    }
    else
    {
	locking_fr = false;
    }

    if (array_to_l->nxoe.locking && array_to_l->nxoe.indirect == false)
    {
	locking_to = true;
	mtx_lock(&array_to_l->lock);
    }
    else
    {
	locking_to = false;
    }
#endif
    for (i = 0; i < len_fr; i++)
    {
	nxo_dup(&arr_to[i], &arr_fr[i]);
    }
#ifdef CW_THREADS
    if (locking_fr)
    {
	mtx_unlock(&array_fr_l->lock);
    }
#endif

    /* Truncate the destination array if it is shorter than the source array. */
    if (len_to > len_fr)
    {
	if (array_to_i != NULL)
	{
	    array_to->e.i.len = len_fr;
	}
	else
	{
	    array_to->e.a.len = len_fr;
	}
    }
#ifdef CW_THREADS
    if (locking_to)
    {
	mtx_unlock(&array_to_l->lock);
    }
#endif
}

uint32_t
nxo_array_len_get(const cw_nxo_t *a_nxo)
{
    uint32_t retval;
    cw_nxoe_array_t *array;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    array = (cw_nxoe_array_t *) a_nxo->o.nxoe;
    cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(array->nxoe.type == NXOT_ARRAY);

    if (array->nxoe.indirect == false)
    {
	retval = array->e.a.len;
    }
    else
    {
	retval = array->e.i.len;
    }

    return retval;
}

void
nxo_array_el_get(const cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el)
{
    cw_nxoe_array_t *array;
#ifdef CW_THREADS
    bool locking;
#endif

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);
    cw_check_ptr(r_el);

    array = (cw_nxoe_array_t *) a_nxo->o.nxoe;

    cw_check_ptr(array);
    cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(array->nxoe.type == NXOT_ARRAY);

    if (array->nxoe.indirect)
    {
	a_offset += array->e.i.beg_offset;
	array = array->e.i.array;
#ifdef CW_THREADS
	locking = false;
#endif
    }
#ifdef CW_THREADS
    else
    {
	if (array->nxoe.locking)
	{
	    locking = true;
	}
	else
	{
	    locking = false;
	}
    }

    if (locking)
    {
	mtx_lock(&array->lock);
    }
#endif
    cw_assert(array->nxoe.indirect == false);

    cw_assert(a_offset >= 0 && a_offset < array->e.a.len);
    nxo_dup(r_el, &array->e.a.arr[a_offset]);
#ifdef CW_THREADS
    if (locking)
    {
	mtx_unlock(&array->lock);
    }
#endif
}

void
nxo_array_el_set(cw_nxo_t *a_nxo, cw_nxo_t *a_el, cw_nxoi_t a_offset)
{
    cw_nxoe_array_t *array;
#ifdef CW_THREADS
    bool locking;
#endif

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);

    array = (cw_nxoe_array_t *) a_nxo->o.nxoe;

    cw_check_ptr(array);
    cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(array->nxoe.type == NXOT_ARRAY);

    if (array->nxoe.indirect)
    {
	a_offset += array->e.i.beg_offset;
	array = array->e.i.array;
#ifdef CW_THREADS
	locking = false;
#endif
    }
#ifdef CW_THREADS
    else
    {
	if (array->nxoe.locking)
	{
	    locking = true;
	}
	else
	{
	    locking = false;
	}
    }

    if (locking)
    {
	mtx_lock(&array->lock);
    }
#endif
    cw_assert(array->nxoe.indirect == false);

    cw_assert(a_offset >= 0 && a_offset < array->e.a.len);
    nxo_no_new(&array->e.a.arr[a_offset]);
    nxo_dup(&array->e.a.arr[a_offset], a_el);
#ifdef CW_THREADS
    if (locking)
    {
	mtx_unlock(&array->lock);
    }
#endif
}

bool
nxo_array_origin_get(cw_nxo_t *a_nxo, const uint8_t **r_origin,
		     uint32_t *r_olen, uint32_t *r_line_num)
{
    bool retval;
    cw_nxoe_array_t *array;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);

    array = (cw_nxoe_array_t *) a_nxo->o.nxoe;

    cw_check_ptr(array);
    cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(array->nxoe.type == NXOT_ARRAY);

    if (array->nxoe.origin)
    {
	retval = origin_l_lookup(array, r_origin, r_olen, r_line_num);
    }
    else
    {
	retval = true;
    }

    return retval;
}

void
nxo_array_origin_set(cw_nxo_t *a_nxo, const uint8_t *a_origin,
		     uint32_t a_olen, uint32_t a_line_num)
{
    cw_nxoe_array_t *array;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);

    array = (cw_nxoe_array_t *) a_nxo->o.nxoe;

    cw_check_ptr(array);
    cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(array->nxoe.type == NXOT_ARRAY);

    cw_assert(array->nxoe.origin == false);

    origin_l_insert(array, a_origin, a_olen, a_line_num);
    array->nxoe.origin = true;
}
