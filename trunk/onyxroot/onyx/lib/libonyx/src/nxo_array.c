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
nxo_array_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking,
	      cw_uint32_t a_len)
{
    cw_nxoe_array_t *array;
    cw_nxa_t *nxa;
    cw_uint32_t i;

    nxa = nx_nxa_get(a_nx);

    array = (cw_nxoe_array_t *) nxa_malloc(nxa, sizeof(cw_nxoe_array_t));

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
	array->e.a.arr = (cw_nxo_t *) nxa_malloc(nxa, sizeof(cw_nxo_t) *
						 array->e.a.alloc_len);
	for (i = 0; i < array->e.a.len; i++)
	{
	    nxo_null_new(&array->e.a.arr[i]);
	}
    }

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) array;
    nxo_p_type_set(a_nxo, NXOT_ARRAY);

    nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *) array);
}

void
nxo_array_subarray_new(cw_nxo_t *a_nxo, cw_nxo_t *a_array, cw_nx_t *a_nx,
		       cw_uint32_t a_offset, cw_uint32_t a_len)
{
    cw_nxoe_array_t *array, *orig;

    cw_check_ptr(a_nxo);

    orig = (cw_nxoe_array_t *) a_array->o.nxoe;
    cw_check_ptr(orig);
    cw_dassert(orig->nxoe.magic == CW_NXOE_MAGIC);

    array = (cw_nxoe_array_t *) nxa_malloc(nx_nxa_get(a_nx),
					   sizeof(cw_nxoe_array_t));

    nxoe_l_new(&array->nxoe, NXOT_ARRAY, FALSE);
    array->nxoe.indirect = TRUE;

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

    nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *) array);
}

void
nxo_array_copy(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
    cw_nxoe_array_t *array_fr, *array_fr_i = NULL, *array_fr_l;
    cw_nxoe_array_t *array_to, *array_to_i = NULL, *array_to_l;
    cw_nxo_t *arr_fr, *arr_to;
    cw_uint32_t i, len_fr, len_to;

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
    nxoe_p_array_lock(array_fr_l);
    nxoe_p_array_lock(array_to_l);
#endif
    for (i = 0; i < len_fr; i++)
    {
	nxo_dup(&arr_to[i], &arr_fr[i]);
    }
#ifdef CW_THREADS
    nxoe_p_array_unlock(array_fr_l);
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
    nxoe_p_array_unlock(array_to_l);
#endif
}

cw_uint32_t
nxo_array_len_get(const cw_nxo_t *a_nxo)
{
    cw_uint32_t retval;
    cw_nxoe_array_t *array;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    array = (cw_nxoe_array_t *) a_nxo->o.nxoe;
    cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(array->nxoe.type == NXOT_ARRAY);

    if (array->nxoe.indirect == FALSE)
    {
	retval = array->e.a.len;
    }
    else
    {
	retval = array->e.i.len;
    }

    return retval;
}

#ifdef CW_USE_INLINES
void
nxo_array_el_get(const cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el)
{
    nxo_l_array_el_get(a_nxo, a_offset, r_el);
}
#endif

void
nxo_array_el_set(cw_nxo_t *a_nxo, cw_nxo_t *a_el, cw_nxoi_t a_offset)
{
    cw_nxoe_array_t *array;

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
    }
#ifdef CW_THREADS
    nxoe_p_array_lock(array);
#endif
    cw_assert(array->nxoe.indirect == FALSE);

    cw_assert(a_offset >= 0 && a_offset < array->e.a.len);
    nxo_no_new(&array->e.a.arr[a_offset]);
    nxo_dup(&array->e.a.arr[a_offset], a_el);
#ifdef CW_THREADS
    nxoe_p_array_unlock(array);
#endif
}
