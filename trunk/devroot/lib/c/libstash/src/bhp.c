/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Implementation of binomial heaps.  This class is pretty straight-forward,
 * except that bhp_set_priority_compare() should be called before inserting any
 * data into the heap.  bhp does not re-heapify if the priority comparison
 * function is changed while there is data in the tree, so "don't do that".
 *
 ****************************************************************************/

#define _STASH_USE_BHP
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include "libstash/bhp_priv.h"

/****************************************************************************
 *
 * bhp constructor.
 *
 ****************************************************************************/
cw_bhp_t *
#ifdef _CW_REENTRANT
bhp_new(cw_bhp_t * a_bhp, cw_bool_t a_is_thread_safe)
#else
bhp_new(cw_bhp_t * a_bhp)
#endif
{
  cw_bhp_t * retval;

  if (a_bhp == NULL)
  {
    retval = (cw_bhp_t *) _cw_malloc(sizeof(cw_bhp_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_bhp;
    retval->is_malloced = FALSE;
  }

#ifdef _CW_REENTRANT
  if (a_is_thread_safe == TRUE)
  {
    retval->is_thread_safe = TRUE;
    rwl_new(&retval->rw_lock);
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
#endif
  
  retval->head = NULL;
  retval->num_nodes = 0;
  retval->priority_compare = bhp_p_priority_compare;
  
  return retval;
}

/****************************************************************************
 *
 * bhp destructor.
 *
 ****************************************************************************/
void
bhp_delete(cw_bhp_t * a_bhp)
{
  _cw_check_ptr(a_bhp);

#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_delete(&a_bhp->rw_lock);
  }
#endif

  if (a_bhp->is_malloced == TRUE)
  {
    _cw_free(a_bhp);
  }
}

/****************************************************************************
 *
 * Print out the internal state of the heap.
 *
 ****************************************************************************/
void
bhp_dump(cw_bhp_t * a_bhp)
{

  _cw_check_ptr(a_bhp);
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_wlock(&a_bhp->rw_lock);
  }
#endif

  bhp_p_dump(a_bhp->head, 0, NULL);
  
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_wunlock(&a_bhp->rw_lock);
  }
#endif
}

/****************************************************************************
 *
 * Insert an item.
 *
 ****************************************************************************/
void
bhp_insert(cw_bhp_t * a_bhp, void * a_priority, void * a_data)
{
  cw_bhp_t temp_heap;
  cw_bhpi_t * new_item;
  
  _cw_check_ptr(a_bhp);
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_wlock(&a_bhp->rw_lock);
  }
#endif
  
  /* Create and initialize new_item. */
  new_item = (cw_bhpi_t *) _cw_malloc(sizeof(cw_bhpi_t));
  bzero(new_item, sizeof(cw_bhpi_t));
  new_item->priority = a_priority;
  new_item->data = a_data;

  /* Create and initialize temp_heap. */
#ifdef _CW_REENTRANT
  bhp_new(&temp_heap, FALSE);
#else
  bhp_new(&temp_heap);
#endif
  temp_heap.head = new_item;
  temp_heap.num_nodes = 1;

  /* Combine this heap and temp_heap. */
  bhp_union(a_bhp, &temp_heap);

  /* Clean up temp_heap. */
  /* XXX This should be done by bhp_merge(). */
/*   bhp_delete(&temp_heap); */
  
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_wunlock(&a_bhp->rw_lock);
  }
#endif
}

/****************************************************************************
 *
 * Find an item on the heap with minimum priority and set *a_priority and
 * *a_data to point to it.
 *
 ****************************************************************************/
cw_bool_t
bhp_find_min(cw_bhp_t * a_bhp, void ** a_priority, void ** a_data)
{
  cw_bool_t retval;
  cw_bhpi_t * curr_min, * curr_pos;
  
  _cw_check_ptr(a_bhp);
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_rlock(&a_bhp->rw_lock);
  }
#endif

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

    /* We've found a minimum priority item now, so point *a_priority and
     * *a_data to it. */
    *a_priority = curr_min->priority;
    *a_data = curr_min->data;
  }
  else
  {
    retval = TRUE;
  }
  
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_runlock(&a_bhp->rw_lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Find an item on the heap with minimum priority, set *a_priority and
 * *a_data to point to it, and delete the item from the heap.
 *
 ****************************************************************************/
cw_bool_t
bhp_del_min(cw_bhp_t * a_bhp, void ** a_priority, void ** a_data)
{
  cw_bool_t retval;
  cw_bhpi_t * prev_pos, * curr_pos, * next_pos, * before_min, * curr_min;
  cw_bhp_t temp_heap;
  
  _cw_check_ptr(a_bhp);
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_wlock(&a_bhp->rw_lock);
  }
#endif

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
#ifdef _CW_REENTRANT
    bhp_new(&temp_heap, FALSE);
#else
    bhp_new(&temp_heap);
#endif
    temp_heap.head = prev_pos;
    bhp_union(a_bhp, &temp_heap);
    a_bhp->num_nodes--;

    /* Now point *a_priority and *a_data to the item and free the space taken 
     * up by the item structure. */
    *a_priority = curr_min->priority;
    *a_data = curr_min->data;
    _cw_free(curr_min);
  }
  
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_wunlock(&a_bhp->rw_lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Return the size of the heap.
 *
 ****************************************************************************/
cw_uint64_t
bhp_get_size(cw_bhp_t * a_bhp)
{
  cw_uint64_t retval;

  _cw_check_ptr(a_bhp);
  /* Don't need to lock. */

  retval = a_bhp->num_nodes;
  return retval;
}

/****************************************************************************
 *
 * Creates a new heap that is the union of the two arguments.
 *
 ****************************************************************************/
void
bhp_union(cw_bhp_t * a_bhp, cw_bhp_t * a_other)
{
  cw_bhpi_t * prev_node, * curr_node, * next_node;
  
  _cw_check_ptr(a_bhp);
  _cw_check_ptr(a_other);
  /* XXX Should this be locked? */

  bhp_p_merge(a_bhp, a_other);
  if (a_bhp->head == NULL)
  {
    /* Empty heap.  We're done. */
    goto RETURN;
  }

  prev_node = NULL;
  curr_node = a_bhp->head;
  next_node = curr_node->sibling;
  while(next_node != NULL)
  {
    if ((curr_node->degree != 0) /* XXX Is this correct? */
	|| ((next_node->sibling != NULL)
	    && (next_node->sibling->degree == curr_node->degree)))
    {
      prev_node = curr_node;
      curr_node = next_node;
    }
    else if (1 != a_bhp->priority_compare(curr_node->priority,
					    next_node->priority)) /* <= */
    {
      curr_node->sibling = next_node->sibling;
      bhp_p_bin_link(curr_node, next_node);
    }
    else
    {
      if (prev_node == NULL)
      {
	a_bhp->head = next_node;
      }
      else
      {
	prev_node->sibling = next_node;
      }
      bhp_p_bin_link(next_node, curr_node);
      curr_node = curr_node->parent;
    }
    next_node = curr_node->parent;
  }
 RETURN:
}

/****************************************************************************
 *
 * Set the priority comparison function.  If the heap has any data in it,
 * behavior is undefined.  The comparison function's return value should be:
 *
 * A < B  --> -1
 * A == B -->  0
 * A > B  -->  1
 *
 ****************************************************************************/
void
bhp_set_priority_compare(cw_bhp_t * a_bhp,
			 bhp_prio_comp_t * a_new_prio_comp)
{
  _cw_check_ptr(a_bhp);
  _cw_check_ptr(a_new_prio_comp);
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_wlock(&a_bhp->rw_lock);
  }
#endif

  /* To change this while there are items in the heap will wreak havoc. */
  _cw_assert(a_bhp->num_nodes == 0);
  
  a_bhp->priority_compare = a_new_prio_comp;
  
#ifdef _CW_REENTRANT
  if (a_bhp->is_thread_safe == TRUE)
  {
    rwl_wunlock(&a_bhp->rw_lock);
  }
#endif
}

/****************************************************************************
 *
 * Recursively print out the internal state of the heap (actually do the work).
 *
 ****************************************************************************/
cw_bhpi_t *
bhp_p_dump(cw_bhpi_t * a_bhpi, cw_uint32_t a_depth, cw_bhpi_t * a_last_printed)
{
/*    cw_bhpi_t * bhpi_p; */
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
    for (i = 0; i < (a_depth * 50); i++)
    {
      log_printf(g_log_o, " ");
    }
  }
  log_printf(g_log_o, "[depth: %d, degree: %d, priority: %d, data: %d]",
	     a_depth, a_bhpi->degree, *(cw_sint32_t *) a_bhpi->priority,
	     *(cw_sint32_t *) a_bhpi->data);
  
  /* Child. */
  if (NULL != a_bhpi->child)
  {
    log_printf(g_log_o, " ---> ");
    a_last_printed = bhp_p_dump(a_bhpi->child, a_depth + 1, a_bhpi);
  }
  else
  {
    log_printf(g_log_o, "\n");
  }

  return a_last_printed;
}

/****************************************************************************
 *
 * Links two binomial heaps of the same degree (n) together into one heap
 * of degree (n + 1).  a_root points to the root of the resulting heap.
 *
 ****************************************************************************/
void
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
void
bhp_p_merge(cw_bhp_t * a_bhp, cw_bhp_t * a_other)
{
  cw_bhpi_t * curr_this, * curr_other, * this_marker = NULL, * other_marker;
  
  if (a_bhp->head != NULL)
  {
    curr_this = a_bhp->head;
  }
  else
  {
    a_bhp->head = a_other->head;
    goto RETURN;
  }

  curr_other = a_other->head;
  while ((curr_this != NULL) && (curr_other != NULL))
  {
    if (curr_this->degree <= curr_other->degree)
    {
      /* Fast forward to where we need to insert from other */
      while ((curr_this->sibling != NULL)
	     && (curr_this->sibling->degree <= curr_other->degree))
      {
	curr_this = curr_this->sibling;
      }
      /* Move forward in other, but keep a marker. */
      other_marker = curr_other;
      curr_other = curr_other->sibling;

      /* Move forward in this, but keep a marker. */
      this_marker = curr_this;
      curr_this = curr_this->sibling;

      /* Now link things together. */
      this_marker->sibling = other_marker;
      other_marker->sibling = curr_this;
    }
    /* I think this can only happen once. */
    else if (curr_this->degree > curr_other->degree)
    {
      /* If first item in the list comes from other, then the following if
       * statement probably always evaluates to true. */
      if (curr_this == a_bhp->head)
      {
	a_bhp->head = a_other->head;
      }

      /* Fast forward to where we need to insert. */
      while ((curr_other->sibling != NULL)
	     && (curr_other->sibling->degree < curr_this->degree))
      {
	curr_other = curr_other->sibling;
      }

      /* Move forward in other, but keep a marker. */
      other_marker = curr_other;
      curr_other = curr_other->sibling;

      /* Link (prepend) to this. */
      other_marker->sibling = curr_this;
    }
  }
  if (curr_this == NULL)
  {
    if (curr_other == NULL)
    {
      /* Bother curr_this and curr_other are NULL.  We're done. */
    }
    else
    {
      if (curr_this == a_bhp->head)
      {
	/* This is an empty list! */
	a_bhp->head = a_other->head;
      }
      else
      {
	/* Append remainder of other to this. */
	_cw_assert(this_marker != NULL);
	this_marker->sibling = curr_other;
      }
    }
  }
  
 RETURN:
  a_bhp->num_nodes += a_other->num_nodes;

  /* Destroy the old other heap. */
  bhp_delete(a_other);
}

/****************************************************************************
 *
 * Default priority comparison function, compares the values of the pointers
 * themselves.  This probably isn't very useful.
 *
 ****************************************************************************/
cw_sint32_t
bhp_p_priority_compare(void * a_a, void * a_b)
{
  cw_sint32_t retval;

  if (a_a < a_b)
  {
    retval = -1;
  }
  else if (a_a > a_b)
  {
    retval = 1;
  }
  else
  {
    retval = 0;
  }
  
  return retval;
}
