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
#include "../include/libstash/bhp_p.h"

cw_bhpi_t *
bhpi_new(cw_bhpi_t * a_bhpi, const void * a_priority, const void * a_data,
	 void (*a_dealloc_func)(void * dealloc_arg, void * bhpi),
	 void * a_dealloc_arg)
{
  cw_bhpi_t * retval;

  if (NULL != a_bhpi)
  {
    retval = a_bhpi;
    bzero(retval, sizeof(cw_bhpi_t));
    retval->dealloc_func = a_dealloc_func;
    retval->dealloc_arg = a_dealloc_arg;
  }
  else
  {
    retval = (cw_bhpi_t *) _cw_malloc(sizeof(cw_bhpi_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_bhpi_t));
    retval->dealloc_func = mem_dealloc;
    retval->dealloc_arg = cw_g_mem;
  }

  retval->priority = a_priority;
  retval->data = a_data;

#ifdef _LIBSTASH_DBG
  retval->magic_a = _LIBSTASH_BHPI_MAGIC;
  retval->size_of = sizeof(cw_bhpi_t);
  retval->magic_b = _LIBSTASH_BHPI_MAGIC;
#endif

  RETURN:
  return retval;
}

void
bhpi_delete(cw_bhpi_t * a_bhpi)
{
  _cw_check_ptr(a_bhpi);
  _cw_assert(_LIBSTASH_BHPI_MAGIC == a_bhpi->magic_a);
  _cw_assert(sizeof(cw_bhpi_t) == a_bhpi->size_of);
  _cw_assert(_LIBSTASH_BHPI_MAGIC == a_bhpi->magic_b);

  if (NULL != a_bhpi->dealloc_func)
  {
    a_bhpi->dealloc_func(a_bhpi->dealloc_arg, a_bhpi);
  }
#ifdef _LIBSTASH_DBG
  else
  {
    memset(a_bhpi, 0x5a, sizeof(cw_bhpi_t));
  }
#endif
}

cw_bhp_t *
bhp_new(cw_bhp_t * a_bhp, bhp_prio_comp_t * a_prio_comp)
{
  return bhp_p_new(a_bhp, a_prio_comp, FALSE);
}

cw_bhp_t *
bhp_new_r(cw_bhp_t * a_bhp, bhp_prio_comp_t * a_prio_comp)
{
  return bhp_p_new(a_bhp, a_prio_comp, TRUE);
}

void
bhp_delete(cw_bhp_t * a_bhp)
{
  _cw_check_ptr(a_bhp);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_a);
  _cw_assert(sizeof(cw_bhp_t) == a_bhp->size_of);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_b);

  /* Empty the heap. */
  if (NULL != a_bhp->head)
  {
    while (0 < a_bhp->num_nodes)
    {
      bhp_del_min(a_bhp, NULL, NULL);
    }
  }
  
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_delete(&a_bhp->lock);
  }

  if (TRUE == a_bhp->is_malloced)
  {
    _cw_free(a_bhp);
  }
}

void
bhp_dump(cw_bhp_t * a_bhp)
{
  _cw_check_ptr(a_bhp);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_a);
  _cw_assert(sizeof(cw_bhp_t) == a_bhp->size_of);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_b);
  
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_lock(&a_bhp->lock);
  }

  out_put(cw_g_out, "=== bhp_dump() start ==============================\n");
  out_put(cw_g_out, "num_nodes: [q]\n",
	  a_bhp->num_nodes);
  if (NULL != a_bhp->head)
  {
    bhp_p_dump(a_bhp->head, 0, NULL);
  }
  out_put(cw_g_out, "=== bhp_dump() end ================================\n");
  
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_unlock(&a_bhp->lock);
  }
}

void
bhp_insert(cw_bhp_t * a_bhp, cw_bhpi_t * a_bhpi)
{
  cw_bhp_t temp_heap;
  
  _cw_check_ptr(a_bhp);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_a);
  _cw_assert(sizeof(cw_bhp_t) == a_bhp->size_of);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_b);
  _cw_check_ptr(a_bhpi);
  _cw_assert(_LIBSTASH_BHPI_MAGIC == a_bhpi->magic_a);
  _cw_assert(sizeof(cw_bhpi_t) == a_bhpi->size_of);
  _cw_assert(_LIBSTASH_BHPI_MAGIC == a_bhpi->magic_b);
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_lock(&a_bhp->lock);
  }
  
  /* Create and initialize temp_heap. */
  bhp_new(&temp_heap, a_bhp->priority_compare);
  temp_heap.head = a_bhpi;
  temp_heap.num_nodes = 1;

  /* Combine this heap and temp_heap. */
  bhp_p_union(a_bhp, &temp_heap);
  
  /* Destroy the old heap. */
  temp_heap.head = NULL;
  bhp_delete(&temp_heap);

  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_unlock(&a_bhp->lock);
  }
}

cw_bool_t
bhp_find_min(cw_bhp_t * a_bhp, void ** r_priority, void ** r_data)
{
  cw_bool_t retval;
  cw_bhpi_t * curr_min, * curr_pos;
  
  _cw_check_ptr(a_bhp);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_a);
  _cw_assert(sizeof(cw_bhp_t) == a_bhp->size_of);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_b);
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_lock(&a_bhp->lock);
  }

  if (a_bhp->head != NULL)
  {
    retval = FALSE;

    curr_min = a_bhp->head;
    curr_pos = a_bhp->head->sibling;
    
    while (curr_pos != NULL)
    {
      /* Check if curr_pos is less than curr_min
       * For priority_compare(a, b), -1 means a < b,
       *                              0 means a == b,
       *                              1 means a > b. */
      if (-1 == a_bhp->priority_compare(curr_pos->priority,
					curr_min->priority))
      {
	curr_min = curr_pos;
      }
      curr_pos = curr_pos->sibling;
    }

    /* We've found a minimum priority item now, so point *r_priority and
     * *r_data to it. */
    if (NULL != r_priority)
    {
      *r_priority = (void *) curr_min->priority;
    }
    if (NULL != r_data)
    {
      *r_data = (void *) curr_min->data;
    }
  }
  else
  {
    retval = TRUE;
  }
  
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_unlock(&a_bhp->lock);
  }
  return retval;
}

cw_bool_t
bhp_del_min(cw_bhp_t * a_bhp, void ** r_priority, void ** r_data)
{
  cw_bool_t retval;
  cw_bhpi_t * prev_pos, * curr_pos, * next_pos, * before_min, * curr_min;
  cw_bhp_t temp_heap;
  
  _cw_check_ptr(a_bhp);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_a);
  _cw_assert(sizeof(cw_bhp_t) == a_bhp->size_of);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_b);
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_lock(&a_bhp->lock);
  }

  if (a_bhp->num_nodes == 0)
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    
    /* Find a root with minimum priority. */
    before_min = NULL;
    prev_pos = NULL;
    curr_pos = a_bhp->head;
    curr_min = curr_pos;
    while (curr_pos != NULL)
    {
      if (-1 == a_bhp->priority_compare(curr_pos->priority,
					curr_min->priority))
      {
	/* Found a new minimum. */
	curr_min = curr_pos;
	before_min = prev_pos;
      }
      prev_pos = curr_pos;
      curr_pos = curr_pos->sibling;
    }

    /* Take the minimum root out of the list. */
    if (before_min == NULL)
    {
      /* Minimum root is the first in the list, so move the head pointer
       * forward. */
      a_bhp->head = curr_min->sibling;
    }
    else
    {
      /* Attach previous and next roots together. */
      before_min->sibling = curr_min->sibling;
    }

    /* Reverse order of curr_min's children. */
    prev_pos = NULL;
    curr_pos = curr_min->child;
    if (curr_pos != NULL)
    {
      next_pos = curr_pos->sibling;
    }
    else
    {
      next_pos = NULL; /* Make optimizing compilers shut up about using
			  next_pos uninitialized. */
    }
    
    while (curr_pos != NULL)
    {
      curr_pos->parent = NULL;
      curr_pos->sibling = prev_pos;

      prev_pos = curr_pos;
      curr_pos = next_pos;
      if (next_pos != NULL)
      {
	next_pos = next_pos->sibling;
      }
    }

    /* Create a temporary heap and initialize it. */
    bhp_new(&temp_heap, a_bhp->priority_compare);
    temp_heap.head = prev_pos;
    bhp_p_union(a_bhp, &temp_heap);

    /* Destroy the old heap. */
    temp_heap.head = NULL;
    bhp_delete(&temp_heap);

    a_bhp->num_nodes--;

    /* Now point *r_priority and *r_data to the item and free the space taken 
     * up by the item structure. */
    if (NULL != r_priority)
    {
      *r_priority = (void *) curr_min->priority;
    }
    if (NULL != r_data)
    {
      *r_data = (void *) curr_min->data;
    }
    bhpi_delete(curr_min);
  }
  
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_unlock(&a_bhp->lock);
  }
  return retval;
}

cw_uint64_t
bhp_get_size(cw_bhp_t * a_bhp)
{
  cw_uint64_t retval;

  _cw_check_ptr(a_bhp);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_a);
  _cw_assert(sizeof(cw_bhp_t) == a_bhp->size_of);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_bhp->magic_b);
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_lock(&a_bhp->lock);
  }

  retval = a_bhp->num_nodes;
  
  if (a_bhp->is_thread_safe == TRUE)
  {
    mtx_unlock(&a_bhp->lock);
  }
  return retval;
}

void
bhp_union(cw_bhp_t * a_a, cw_bhp_t * a_b)
{
  _cw_check_ptr(a_a);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_a->magic_a);
  _cw_assert(sizeof(cw_bhp_t) == a_a->size_of);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_a->magic_b);
  _cw_check_ptr(a_b);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_b->magic_a);
  _cw_assert(sizeof(cw_bhp_t) == a_b->size_of);
  _cw_assert(_LIBSTASH_BHP_MAGIC == a_b->magic_b);
  if (a_a->is_thread_safe == TRUE)
  {
    mtx_lock(&a_a->lock);
  }
  if (a_b->is_thread_safe == TRUE)
  {
    mtx_lock(&a_b->lock);
  }

  bhp_p_union(a_a, a_b);

  if (a_b->is_thread_safe == TRUE)
  {
    mtx_unlock(&a_b->lock);
  }
  
  /* Destroy the old heap. */
  a_b->head = NULL;
  bhp_delete(a_b);

  if (a_a->is_thread_safe == TRUE)
  {
    mtx_unlock(&a_a->lock);
  }
}

cw_sint32_t
bhp_priority_compare_uint32(const void * a_a, const void * a_b)
{
  cw_sint32_t retval;
  cw_uint32_t a = *(cw_uint32_t *) a_a;
  cw_uint32_t b = *(cw_uint32_t *) a_b;

  _cw_check_ptr(a_a);
  _cw_check_ptr(a_b);

  if (a < b)
  {
    retval = -1;
  }
  else if (a > b)
  {
    retval = 1;
  }
  else
  {
    retval = 0;
  }
  
  return retval;
}

cw_sint32_t
bhp_priority_compare_sint32(const void * a_a, const void * a_b)
{
  cw_sint32_t retval;
  cw_sint32_t a = *(cw_sint32_t *) a_a;
  cw_sint32_t b = *(cw_sint32_t *) a_b;

  _cw_check_ptr(a_a);
  _cw_check_ptr(a_b);

  if (a < b)
  {
    retval = -1;
  }
  else if (a > b)
  {
    retval = 1;
  }
  else
  {
    retval = 0;
  }
  
  return retval;
}

cw_sint32_t
bhp_priority_compare_uint64(const void * a_a, const void * a_b)
{
  cw_sint32_t retval;
  cw_uint64_t a = *(cw_uint64_t *) a_a;
  cw_uint64_t b = *(cw_uint64_t *) a_b;

  _cw_check_ptr(a_a);
  _cw_check_ptr(a_b);

  if (a < b)
  {
    retval = -1;
  }
  else if (a > b)
  {
    retval = 1;
  }
  else
  {
    retval = 0;
  }
  
  return retval;
}

static cw_bhp_t *
bhp_p_new(cw_bhp_t * a_bhp, bhp_prio_comp_t * a_prio_comp,
	  cw_bool_t a_is_thread_safe)
{
  cw_bhp_t * retval;

  _cw_check_ptr(a_prio_comp);

  if (a_bhp == NULL)
  {
    retval = (cw_bhp_t *) _cw_malloc(sizeof(cw_bhp_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_bhp;
    retval->is_malloced = FALSE;
  }

  if (a_is_thread_safe == TRUE)
  {
    retval->is_thread_safe = TRUE;
    mtx_new(&retval->lock);
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
  
  retval->head = NULL;
  retval->num_nodes = 0;
  retval->priority_compare = a_prio_comp;

#ifdef _LIBSTASH_DBG
  retval->magic_a = _LIBSTASH_BHP_MAGIC;
  retval->size_of = sizeof(cw_bhp_t);
  retval->magic_b = _LIBSTASH_BHP_MAGIC;
#endif

  RETURN:
  return retval;
}

static cw_bhpi_t *
bhp_p_dump(cw_bhpi_t * a_bhpi, cw_uint32_t a_depth, cw_bhpi_t * a_last_printed)
{
  cw_uint32_t i;
  
  /* Sibling. */
  if (NULL != a_bhpi->sibling)
  {
    a_last_printed = bhp_p_dump(a_bhpi->sibling, a_depth, a_last_printed);
  }

  /* Self. */
  if (a_bhpi->parent != a_last_printed)
  {
    /* Indent. */
    for (i = 0; i < (a_depth * 38); i++)
    {
      out_put(cw_g_out, " ");
    }
  }
  out_put(cw_g_out, "[[deg:[i] pri:0x[p|w:8|p:0] dat:0x[p|w:8|p:0]]",
	  a_bhpi->degree, a_bhpi->priority, a_bhpi->data);
  a_last_printed = a_bhpi;
  
  /* Child. */
  if (NULL != a_bhpi->child)
  {
    out_put(cw_g_out, "-");
    a_last_printed = bhp_p_dump(a_bhpi->child, a_depth + 1, a_bhpi);
  }
  else
  {
    out_put(cw_g_out, "\n");
  }

  return a_last_printed;
}

/****************************************************************************
 *
 * Links two binomial heaps of the same degree (n) together into one heap
 * of degree (n + 1).  a_root points to the root of the resulting heap.
 *
 ****************************************************************************/
static void
bhp_p_bin_link(cw_bhpi_t * a_root, cw_bhpi_t * a_non_root)
{
  a_non_root->parent = a_root;
  a_non_root->sibling = a_root->child;
  a_root->child = a_non_root;
  a_root->degree++;
}

/****************************************************************************
 *
 * Merges the root lists of the two heaps specified by the arguments, in
 * monotonically increasing order.  The result is stored in a_bhp.
 *
 ****************************************************************************/
static void
bhp_p_merge(cw_bhp_t * a_a, cw_bhp_t * a_b)
{
  if (NULL == a_a->head)
  {
    a_a->head = a_b->head;
  }
  else if (NULL != a_b->head)
  {
    cw_bhpi_t * mark_a, * curr_a, * mark_b, * curr_b;
    
    /* Both heaps have contents. */
    
    if (a_a->head->degree > a_b->head->degree)
    {
      cw_bhpi_t * t_bhpi;
      
      /* Swap the heads to simplify the following loop.  Now we know that
       * a_a->head is set correctly. */
      t_bhpi = a_a->head;
      a_a->head = a_b->head;
      a_b->head = t_bhpi;
    }

    mark_a = NULL; /* Avoid optimization warnings about uninitialized reads. */
    curr_a = a_a->head;
    curr_b = a_b->head;
    while ((NULL != curr_a->sibling) && (NULL != curr_b))
    {
      /* Fast forward to where we need to insert from b. */
      while ((NULL != curr_a->sibling)
	     && (curr_a->degree <= curr_b->degree))
      {
	mark_a = curr_a;
	curr_a = curr_a->sibling;
      }

      /* Move forward in b. */
      mark_b = curr_b;
      curr_b = curr_b->sibling;

      /* Link things together. */
      mark_a->sibling = mark_b;
      while ((NULL != curr_b)
	     && (curr_b->degree <= curr_a->degree))
      {
	mark_b = curr_b;
	curr_b = curr_b->sibling;
      }
      mark_a = mark_b;
      mark_b->sibling = curr_a;
    }

    /* If there are still nodes in b, append them. */
    if (NULL != curr_b) /* curr_a->sibling is implicitly NULL if this is true,
			 * due to the loop exit condition above. */
    {
      _cw_assert(NULL == curr_a->sibling);
      _cw_check_ptr(curr_b);
      curr_a->sibling = curr_b;
    }
  }

  /* Adjust the size. */
  a_a->num_nodes += a_b->num_nodes;
}

static void
bhp_p_union(cw_bhp_t * a_a, cw_bhp_t * a_b)
{
  cw_bhpi_t * prev_node, * curr_node, * next_node;
  
  _cw_assert(a_a->priority_compare == a_b->priority_compare);
  
  bhp_p_merge(a_a, a_b);

  if (a_a->head == NULL)
  {
    /* Empty heap.  We're done. */
    return;
  }

  prev_node = NULL;
  curr_node = a_a->head;
  next_node = curr_node->sibling;
  while(next_node != NULL)
  {
    if ((curr_node->degree != next_node->degree)
	|| ((next_node->sibling != NULL)
	    && (next_node->sibling->degree == curr_node->degree)))
    {
      /* Either these two roots are unequal, or we're looking at the first two
       * of three roots of equal degree (can happen because of merge (2) plus
       * ripple carry (1)). */
      prev_node = curr_node;
      curr_node = next_node;
    }
    else if (1 != a_a->priority_compare(curr_node->priority,
					next_node->priority)) /* <= */
    {
      /* The priority of the root of curr_node is <= the priority of the root of
       * next_node. */
      curr_node->sibling = next_node->sibling;
      bhp_p_bin_link(curr_node, next_node);
    }
    else
    {
      /* The priority of the root of curr_node is > the priority of the root of
       * next_node. */
      if (prev_node == NULL)
      {
	a_a->head = next_node;
      }
      else
      {
	prev_node->sibling = next_node;
      }
      bhp_p_bin_link(next_node, curr_node);
      curr_node = curr_node->parent;
    }
    next_node = curr_node->sibling;
  }
}
