/* -*-mode:c-*-
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
 * Implementation of bhp (binomial heap).
 *
 ****************************************************************************/

#include <string.h>

#define _INC_BHP_H_
#include <libstash.h>
#include <bhp_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 * bhp constructor.
 *
 ****************************************************************************/
cw_bhp_t *
bhp_new(cw_bhp_t * a_bhp_o, cw_bool_t a_is_thread_safe)
{
  cw_bhp_t * retval;

  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_new()");
  }
  
  if (a_bhp_o == NULL)
  {
    retval = (cw_bhp_t *) _cw_malloc(sizeof(cw_bhp_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_bhp_o;
    retval->is_malloced = FALSE;
  }

  if (a_is_thread_safe == TRUE)
  {
    retval->is_thread_safe = TRUE;
    rwl_new(&retval->rw_lock);
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
  
  retval->head = NULL;
  retval->num_nodes = 0;
  retval->priority_compare = bhp_p_priority_compare;
  
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_new()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * bhp destructor.
 *
 ****************************************************************************/
void
bhp_delete(cw_bhp_t * a_bhp_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_delete()");
  }
  _cw_check_ptr(a_bhp_o);

  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_delete(&a_bhp_o->rw_lock);
  }

  if (a_bhp_o->is_malloced == TRUE)
  {
    _cw_free(a_bhp_o);
  }
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_delete()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Print out the internal state in pseudo tree format.
 *
 ****************************************************************************/
void
bhp_dump(cw_bhp_t * a_bhp_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_dump()");
  }
  _cw_check_ptr(a_bhp_o);
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_rlock(&a_bhp_o->rw_lock);
  }

  _cw_error("Not implemented");
  
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_runlock(&a_bhp_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_dump()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Insert an item.
 *
 ****************************************************************************/
void
bhp_insert(cw_bhp_t * a_bhp_o, void * a_priority, void * a_data)
{
  cw_bhp_t temp_heap;
  cw_bhpi_t * new_item;
  
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_insert()");
  }
  _cw_check_ptr(a_bhp_o);
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_wlock(&a_bhp_o->rw_lock);
  }
  
  /* Create and initialize new_item. */
  new_item = (cw_bhpi_t *) _cw_malloc(sizeof(cw_bhpi_t));
  bzero(new_item, sizeof(cw_bhpi_t));
  new_item->priority = a_priority;
  new_item->data = a_data;

  /* Create and initialize temp_heap. */
  bhp_new(&temp_heap, FALSE);
  temp_heap.head = new_item;
  temp_heap.num_nodes = 1;

  /* Combine this heap and temp_heap. */
  bhp_union(a_bhp_o, &temp_heap);

  /* Clean up temp_heap. */
  /* XXX This should be done by bhp_merge(). */
/*   bhp_delete(&temp_heap); */
  
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_wunlock(&a_bhp_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_insert()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Find an item on the heap with minimum priority and set *a_priority and
 * *a_data to point to it.
 *
 ****************************************************************************/
cw_bool_t
bhp_find_min(cw_bhp_t * a_bhp_o, void ** a_priority, void ** a_data)
{
  cw_bool_t retval;
  cw_bhpi_t * curr_min, * curr_pos;
  
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_find_min()");
  }
  _cw_check_ptr(a_bhp_o);
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_rlock(&a_bhp_o->rw_lock);
  }

  if (a_bhp_o->head != NULL)
  {
    retval = FALSE;

    curr_min = a_bhp_o->head;
    curr_pos = a_bhp_o->head->sibling;
    
    while (curr_pos != NULL)
    {
      /* Check if curr_pos is less than curr_min
       * For priority_compare(a, b), -1 means a < b,
       *                              0 means a == b,
       *                              1 means a > b. */
      if (a_bhp_o->priority_compare(curr_pos, curr_min) == -1)
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
  
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_runlock(&a_bhp_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_find_min()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Find an item on the heap with minimum priority, set *a_priority and
 * *a_data to point to it, and delete the item from the heap.
 *
 ****************************************************************************/
cw_bool_t
bhp_del_min(cw_bhp_t * a_bhp_o, void ** a_priority, void ** a_data)
{
  cw_bool_t retval;
  cw_bhpi_t * prev_pos, * curr_pos, * next_pos, * before_min, * curr_min;
  cw_bhp_t temp_heap;
  
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_del_min()");
  }
  _cw_check_ptr(a_bhp_o);
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_wlock(&a_bhp_o->rw_lock);
  }

  if (a_bhp_o->num_nodes == 0)
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    
    /* Find a root with minimum priority. */
    before_min = NULL;
    prev_pos = NULL;
    curr_pos = a_bhp_o->head;
    curr_min = curr_pos;
    while (curr_pos != NULL)
    {
      if (a_bhp_o->priority_compare(curr_pos, curr_min) == -1)
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
      a_bhp_o->head = curr_min->sibling;
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
    bhp_new(&temp_heap, FALSE);
    temp_heap.head = prev_pos;
    bhp_union(a_bhp_o, &temp_heap);
    a_bhp_o->num_nodes--;

    /* Now point *a_priority and *a_data to the item and free the space taken 
     * up by the item structure. */
    *a_priority = curr_min->priority;
    *a_data = curr_min->data;
    _cw_free(curr_min);
  }
  
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_wunlock(&a_bhp_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_del_min()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Return the size of the heap.
 *
 ****************************************************************************/
cw_uint64_t
bhp_get_size(cw_bhp_t * a_bhp_o)
{
  cw_uint64_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_get_size()");
  }
  _cw_check_ptr(a_bhp_o);
  /* Don't need to lock. */

  retval = a_bhp_o->num_nodes;
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_get_size()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Creates a new heap that is the union of the two arguments.
 *
 ****************************************************************************/
void
bhp_union(cw_bhp_t * a_bhp_o, cw_bhp_t * a_other)
{
  cw_bhpi_t * prev_node, * curr_node, * next_node;
  
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_union()");
  }
  _cw_check_ptr(a_bhp_o);
  _cw_check_ptr(a_other);

  bhp_p_merge(a_bhp_o, a_other);
  if (a_bhp_o->head == NULL)
  {
    /* Empty heap.  We're done. */
    goto RETURN;
  }

  prev_node = NULL;
  curr_node = a_bhp_o->head;
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
    else if (a_bhp_o->priority_compare(curr_node, next_node) != 1) /* <= */
    {
      curr_node->sibling = next_node->sibling;
      bhp_p_bin_link(curr_node, next_node);
    }
    else
    {
      if (prev_node == NULL)
      {
	a_bhp_o->head = next_node;
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
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_union()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
bhp_set_priority_compare(cw_bhp_t * a_bhp_o,
			 bhp_prio_comp_t * a_new_prio_comp)
{
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_set_priority_compare()");
  }
  _cw_check_ptr(a_bhp_o);
  _cw_check_ptr(a_new_prio_comp);
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_wlock(&a_bhp_o->rw_lock);
  }

  /* To change this while there are items in the heap will wreak havoc. */
  _cw_assert(a_bhp_o->num_nodes == 0);
  
  a_bhp_o->priority_compare = a_new_prio_comp;
  
  if (a_bhp_o->is_thread_safe == TRUE)
  {
    rwl_wunlock(&a_bhp_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_set_priority_compare()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Links two binomial heaps of the same degree (n) together into one heap
 * of degree (n + 1).  a_root points to the root of the resulting heap.
 *
 ****************************************************************************/
void
bhp_p_bin_link(cw_bhpi_t * a_root, cw_bhpi_t * a_non_root)
{
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_p_bin_link()");
  }
/*   _cw_check_ptr(a_root); */
/*   _cw_check_ptr(a_non_root); */
  
  a_non_root->parent = a_root;
  a_non_root->sibling = a_root->child;
  a_root->child = a_non_root;
  a_root->degree++;
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_p_bin_link()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Merges the root lists of the two heaps specified by the arguments, in
 * monotonically increasing order.  The result is stored in a_bhp_o.
 *
 ****************************************************************************/
void
bhp_p_merge(cw_bhp_t * a_bhp_o, cw_bhp_t * a_other)
{
  cw_bhpi_t * curr_this, * curr_other, * this_marker = NULL, * other_marker;
  
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_p_merge()");
  }
/*   _cw_check_ptr(a_bhp_o); */
/*   _cw_check_ptr(a_other); */

  if (a_bhp_o->head != NULL)
  {
    curr_this = a_bhp_o->head;
  }
  else
  {
    a_bhp_o->head = a_other->head;
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
      if (curr_this == a_bhp_o->head)
      {
	a_bhp_o->head = a_other->head;
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
      if (curr_this == a_bhp_o->head)
      {
	/* This is an empty list! */
	a_bhp_o->head = a_other->head;
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
  a_bhp_o->num_nodes += a_other->num_nodes;

  /* Destroy the old other heap. */
  bhp_delete(a_other);
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_p_merge()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_sint32_t
bhp_p_priority_compare(cw_bhpi_t * a_a, cw_bhpi_t * a_b)
{
  cw_sint32_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Enter bhp_p_priority_compare()");
  }
  /* No need to bullet-proof this, since it's a private method. */
/*   _cw_check_ptr(a_a); */
/*   _cw_check_ptr(a_b); */

  if (a_a->priority < a_b->priority)
  {
    retval = -1;
  }
  else if (a_a->priority > a_b->priority)
  {
    retval = 1;
  }
  else
  {
    retval = 0;
  }
  
  if (_cw_pmatch(_STASH_DBG_R_BHP_FUNC))
  {
    _cw_marker("Exit bhp_p_priority_compare()");
  }
  return retval;
}
