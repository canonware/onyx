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
 ****************************************************************************/

#include "../include/libstash/libstash.h"
#include "../include/libstash/treen_p.h"

cw_treen_t *
treen_new(cw_treen_t * a_treen,
	  void (*a_dealloc_func)(void * dealloc_arg, void * move),
	  void * a_dealloc_arg)
{
  cw_treen_t * retval;

  if (NULL != a_treen)
  {
    retval = a_treen;
    bzero(retval, sizeof(cw_treen_t));
    retval->dealloc_func = a_dealloc_func;
    retval->dealloc_arg = a_dealloc_arg;
  }
  else
  {
    retval = (cw_treen_t *) _cw_malloc(sizeof(cw_treen_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_treen_t));
    retval->dealloc_func = mem_dealloc;
    retval->dealloc_arg = cw_g_mem;
  }
  
  ring_new(&retval->siblings, NULL, NULL);
  ring_set_data(&retval->siblings, (void *) retval);

#ifdef _LIBSTASH_DBG
  retval->magic_a = _CW_TREEN_MAGIC;
  retval->size_of = sizeof(cw_treen_t);
  retval->magic_b = _CW_TREEN_MAGIC;
#endif

  RETURN:
  return retval;
}

void
treen_delete(cw_treen_t * a_treen)
{
  cw_treen_t * child;

  _cw_check_ptr(a_treen);
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_a);
  _cw_assert(a_treen->size_of == sizeof(cw_treen_t));
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_b);
  
  /* Recursively delete all subtrees. */
  while (NULL != (child = treen_get_child(a_treen)))
  {
    treen_delete(child);
  }

  /* Delete self. */
  treen_link(a_treen, NULL);
  if (NULL != a_treen->dealloc_func)
  {
    a_treen->dealloc_func(a_treen->dealloc_arg, (void *) a_treen);
  }
#ifdef _LIBSTASH_DBG
  else
  {
    memset(a_treen, 0x5a, sizeof(cw_treen_t));
  }
#endif
}

void
treen_link(cw_treen_t * a_treen, cw_treen_t * a_parent)
{
  _cw_check_ptr(a_treen);
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_a);
  _cw_assert(a_treen->size_of == sizeof(cw_treen_t));
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_b);

  /* Extract ourselves from any current linkage before linking somewhere
   * else. */
  if (NULL != a_treen->parent)
  {
    if (a_treen == a_treen->parent->child)
    {
      if (treen_get_sibling(a_treen) != a_treen)
      {
	/* The parent's child pointer points to a_treen, and this isn't the only
	 * child, so parent's child pointer needs to be changed to another
	 * child. */
	a_treen->parent->child = treen_get_sibling(a_treen);
      }
      else
      {
	/* Last child. */
	a_treen->parent->child = NULL;
      }
    }
    
    a_treen->parent = NULL;

    ring_cut(&a_treen->siblings);
  }

  if (NULL != a_parent)
  {
    _cw_assert(_CW_TREEN_MAGIC == a_parent->magic_a);
    _cw_assert(a_parent->size_of == sizeof(cw_treen_t));
    _cw_assert(_CW_TREEN_MAGIC == a_parent->magic_b);

    a_treen->parent = a_parent;
  
    if (NULL == a_parent->child)
    {
      /* The parent has no children yet. */
      a_parent->child = a_treen;
    }
    else
    {
      cw_treen_t * sibling = treen_get_child(a_parent);

      ring_meld(&sibling->siblings, &a_treen->siblings);
    }
  }
}

cw_treen_t *
treen_get_parent(cw_treen_t * a_treen)
{
  _cw_check_ptr(a_treen);
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_a);
  _cw_assert(a_treen->size_of == sizeof(cw_treen_t));
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_b);

  return a_treen->parent;
}

cw_treen_t *
treen_get_child(cw_treen_t * a_treen)
{
  _cw_check_ptr(a_treen);
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_a);
  _cw_assert(a_treen->size_of == sizeof(cw_treen_t));
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_b);

  return a_treen->child;
}

cw_treen_t *
treen_get_sibling(cw_treen_t * a_treen)
{
  cw_treen_t * retval;
  cw_ring_t * t_ring;
  
  _cw_check_ptr(a_treen);
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_a);
  _cw_assert(a_treen->size_of == sizeof(cw_treen_t));
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_b);

  t_ring = ring_next(&a_treen->siblings);
  retval = (cw_treen_t *) ring_get_data(t_ring);

  return retval;
}

void *
treen_get_data_ptr(cw_treen_t * a_treen)
{
  _cw_check_ptr(a_treen);
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_a);
  _cw_assert(a_treen->size_of == sizeof(cw_treen_t));
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_b);

  return a_treen->data;
}

void
treen_set_data_ptr(cw_treen_t * a_treen, void * a_data)
{
  _cw_check_ptr(a_treen);
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_a);
  _cw_assert(a_treen->size_of == sizeof(cw_treen_t));
  _cw_assert(_CW_TREEN_MAGIC == a_treen->magic_b);

  a_treen->data = a_data;
}
