/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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

#define _LIBSTASH_USE_RING
#include "libstash/libstash.h"

#ifdef _LIBSTASH_DBG
#  define _CW_RING_MAGIC 0x410ff014
#endif

cw_ring_t *
ring_new(cw_ring_t * a_ring,
	 void (*a_dealloc_func)(void * dealloc_arg, void * ring),
	 void * a_dealloc_arg)
{
  cw_ring_t * retval;
  
  if (NULL == a_ring)
  {
    retval = (cw_ring_t *) _cw_malloc(sizeof(cw_ring_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->dealloc_func = mem_dealloc;
    retval->dealloc_arg = cw_g_mem;
  }
  else
  {
    retval = a_ring;
    retval->dealloc_func = a_dealloc_func;
    retval->dealloc_arg = a_dealloc_arg;
  }
  retval->prev = retval;
  retval->next = retval;
#ifdef _LIBSTASH_DBG
  retval->magic = _CW_RING_MAGIC;
#endif

  RETURN:
  return retval;
}

void
ring_delete(cw_ring_t * a_ring)
{
  cw_ring_t * t_ring;
  
  _cw_check_ptr(a_ring);
  _cw_assert(a_ring->magic == _CW_RING_MAGIC);

  while (a_ring->next != a_ring)
  {
    t_ring = ring_cut(a_ring);
    ring_delete(a_ring);
    a_ring = t_ring;
  }
  if (NULL != a_ring->dealloc_func)
  {
    a_ring->dealloc_func(a_ring->dealloc_arg, a_ring);
  }
#ifdef _LIBSTASH_DBG
  else
  {
    memset(a_ring, 0x5a, sizeof(cw_ring_t));
  }
#endif
}

void
ring_dump(cw_ring_t * a_ring, const char * a_prefix)
{
  cw_ring_t * t_ring;

  _cw_check_ptr(a_prefix);

  log_printf(cw_g_log, "%sbegin ====================================\n",
	     a_prefix);
  
  t_ring = a_ring;
  do
  {
    _cw_check_ptr(t_ring);
    _cw_assert(t_ring->magic == _CW_RING_MAGIC);

    log_printf(cw_g_log,
	       "%sprev: %p, this: %p, next: %p, data: %p, "
	       "dealloc_func: %p, dealloc_arg: %p\n",
	       a_prefix,
	       t_ring->prev,
	       t_ring,
	       t_ring->next,
	       t_ring->data,
	       t_ring->dealloc_func,
	       t_ring->dealloc_arg);
    
    t_ring = t_ring->next;
  } while (t_ring != a_ring);
  log_printf(cw_g_log, "%send ======================================\n",
	     a_prefix);
}

void *
ring_get_data(cw_ring_t * a_ring)
{
  _cw_check_ptr(a_ring);
  _cw_assert(a_ring->magic == _CW_RING_MAGIC);

  return a_ring->data;
}

void
ring_set_data(cw_ring_t * a_ring, void * a_data)
{
  _cw_check_ptr(a_ring);
  _cw_assert(a_ring->magic == _CW_RING_MAGIC);

  a_ring->data = a_data;
}

cw_ring_t *
ring_next(cw_ring_t * a_ring)
{
  _cw_check_ptr(a_ring);
  _cw_assert(a_ring->magic == _CW_RING_MAGIC);

  return a_ring->next;
}

cw_ring_t *
ring_prev(cw_ring_t * a_ring)
{
  _cw_check_ptr(a_ring);
  _cw_assert(a_ring->magic == _CW_RING_MAGIC);

  return a_ring->prev;
}

void
ring_meld(cw_ring_t * a_a, cw_ring_t * a_b)
{
  cw_ring_t * t_ring;
  
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
ring_cut(cw_ring_t * a_ring)
{
  cw_ring_t * retval;
  
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
ring_split(cw_ring_t * a_a, cw_ring_t * a_b)
{
  cw_ring_t * t_ring;
  
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
