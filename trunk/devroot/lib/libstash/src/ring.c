/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Implementation of the ring class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#ifdef _LIBSTASH_DBG
#define _CW_RING_MAGIC 0x410ff014
#endif

void
ring_new(cw_ring_t *a_ring)
{
	_cw_check_ptr(a_ring);

	a_ring->prev = a_ring;
	a_ring->next = a_ring;
#ifdef _LIBSTASH_DBG
	a_ring->magic = _CW_RING_MAGIC;
#endif
}

void
ring_delete(cw_ring_t *a_ring)
{
	_cw_check_ptr(a_ring);
	_cw_assert(a_ring->magic == _CW_RING_MAGIC);

	ring_cut(a_ring);

#ifdef _LIBSTASH_DBG
	memset(a_ring, 0x5a, sizeof(cw_ring_t));
#endif
}

void
ring_dump(cw_ring_t *a_ring, const char *a_prefix)
{
	cw_ring_t *t_ring;

	_cw_check_ptr(a_prefix);

	_cw_out_put("[s]begin ====================================\n",
	    a_prefix);

	t_ring = a_ring;
	do {
		_cw_check_ptr(t_ring);
		_cw_assert(t_ring->magic == _CW_RING_MAGIC);

		_cw_out_put("[s]prev: 0x[p], this: 0x[p], next: 0x[p], data: 0x[p]\n",
		    a_prefix,
		    t_ring->prev,
		    t_ring,
		    t_ring->next,
		    t_ring->data);

		t_ring = t_ring->next;
	} while (t_ring != a_ring);
	_cw_out_put("[s]end ======================================\n",
	    a_prefix);
}

void   *
ring_get_data(cw_ring_t *a_ring)
{
	_cw_check_ptr(a_ring);
	_cw_assert(a_ring->magic == _CW_RING_MAGIC);

	return a_ring->data;
}

void
ring_set_data(cw_ring_t *a_ring, void *a_data)
{
	_cw_check_ptr(a_ring);
	_cw_assert(a_ring->magic == _CW_RING_MAGIC);

	a_ring->data = a_data;
}

cw_ring_t *
ring_next(cw_ring_t *a_ring)
{
	_cw_check_ptr(a_ring);
	_cw_assert(a_ring->magic == _CW_RING_MAGIC);

	return a_ring->next;
}

cw_ring_t *
ring_prev(cw_ring_t *a_ring)
{
	_cw_check_ptr(a_ring);
	_cw_assert(a_ring->magic == _CW_RING_MAGIC);

	return a_ring->prev;
}

void
ring_meld(cw_ring_t *a_a, cw_ring_t *a_b)
{
	cw_ring_t *t_ring;

	_cw_check_ptr(a_a);
	_cw_assert(a_a->magic == _CW_RING_MAGIC);
	_cw_check_ptr(a_b);
	_cw_assert(a_b->magic == _CW_RING_MAGIC);

	a_a->prev->next = a_b;
	a_b->prev->next = a_a;

	t_ring = a_a->prev;
	a_a->prev = a_b->prev;
	a_b->prev = t_ring;
}

cw_ring_t *
ring_cut(cw_ring_t *a_ring)
{
	cw_ring_t *retval;

	_cw_check_ptr(a_ring);
	_cw_assert(a_ring->magic == _CW_RING_MAGIC);

	retval = a_ring->next;
	retval->prev = a_ring->prev;
	retval->prev->next = retval;

	a_ring->prev = a_ring;
	a_ring->next = a_ring;

	return retval;
}

void
ring_split(cw_ring_t *a_a, cw_ring_t *a_b)
{
	cw_ring_t *t_ring;

	_cw_check_ptr(a_a);
	_cw_assert(a_a->magic == _CW_RING_MAGIC);
	_cw_check_ptr(a_b);
	_cw_assert(a_b->magic == _CW_RING_MAGIC);

	t_ring = a_a->prev;
	a_a->prev = a_b->prev;
	a_b->prev = t_ring;

	a_a->prev->next = a_a;
	a_b->prev->next = a_b;
}
