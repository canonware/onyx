/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 108 $
 * $Date: 1998-06-30 00:07:07 -0700 (Tue, 30 Jun 1998) $
 *
 * <<< Description >>>
 *
 * Doubly linked list implementation.
 *
 ****************************************************************************/

#define _INC_LIST_H_
#include <config.h>
#include <list_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 * list_item constructor.
 *
 ****************************************************************************/
cw_list_item_t *
list_item_new()
{
  cw_list_item_t * retval;

  retval = (cw_list_item_t *) _cw_malloc(sizeof(cw_list_item_t));

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * list_item_destructor.
 *
 ****************************************************************************/
void
list_item_delete(cw_list_item_t * a_list_item_o)
{
  _cw_check_ptr(a_list_item_o);

  _cw_free(a_list_item_o);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Get the value of the data pointer.
 *
 ****************************************************************************/
void *
list_item_get(cw_list_item_t * a_list_item_o)
{
  _cw_check_ptr(a_list_item_o);

  return a_list_item_o->item;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Set the value of the data pointer.
 *
 ****************************************************************************/
void
list_item_set(cw_list_item_t * a_list_item_o, void * a_data)
{
  _cw_check_ptr(a_list_item_o);
  _cw_check_ptr(a_data);

  a_list_item_o->item = a_data;
}

/****************************************************************************
 * <<< Description >>>
 *
 * list constructor.
 *
 ****************************************************************************/
cw_list_t *
list_new(cw_list_t * a_list_o, cw_bool_t a_is_thread_safe)
{
  cw_list_t * retval;

  if (a_list_o == NULL)
  {
    retval = (cw_list_t *) _cw_malloc(sizeof(cw_list_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_list_o;
    retval->is_malloced = FALSE;
  }

  if (a_is_thread_safe)
  {
    retval->is_thread_safe = TRUE;
    mtx_new(&retval->lock);
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
  
  retval->head = NULL;
  retval->tail = NULL;
  retval->count = 0;
  retval->spares_head = NULL;
  retval->spares_count = 0;

  return retval;
}
     
/****************************************************************************
 * <<< Description >>>
 *
 * list destructor.
 *
 ****************************************************************************/
void
list_delete(cw_list_t * a_list_o)
{
  _cw_check_ptr(a_list_o);

  /* Delete whatever items are still in the list.  This does *not* free
   * memory pointed to by the item pointers. */
  for (; a_list_o->count > 0; a_list_o->count--)
  {
    _cw_free(list_p_hpop(a_list_o));
  }
  
  /* Delete the spares list. */
  {
    cw_list_item_t * item;
    
    for (; a_list_o->spares_count > 0; a_list_o->spares_count--)
    {
      item = a_list_o->spares_head;
      a_list_o->spares_head = a_list_o->spares_head->next;
      _cw_free(item);
    }
  }

  if (a_list_o->is_thread_safe)
  {
    mtx_delete(&a_list_o->lock);
  }
  if (a_list_o->is_malloced)
  {
    _cw_free(a_list_o);
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the number of items in the list.
 *
 ****************************************************************************/
cw_uint64_t
list_count(cw_list_t * a_list_o)
{
  cw_uint64_t retval;

  _cw_check_ptr(a_list_o);

  retval = a_list_o->count;
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Pushes an item onto the head of the list.
 *
 ****************************************************************************/
cw_list_item_t *
list_hpush(cw_list_t * a_list_o, void * a_data)
{
  cw_list_item_t * retval;
  
  _cw_check_ptr(a_list_o);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  /* Find a list item somewhere. */
  if (a_list_o->spares_count > 0)
  {
    /* A spare item is available, so use it. */
    retval = a_list_o->spares_head;
    a_list_o->spares_head = a_list_o->spares_head->next;
    a_list_o->spares_count--;
  }
  else
  {
    /* No spares available.  Create a new item. */
    retval = (cw_list_item_t *) _cw_malloc(sizeof(cw_list_item_t));
  }
  retval->item = a_data;

  /* Link things together. */
  if (a_list_o->head != NULL)
  {
    /* The list isn't empty. */
    retval->prev = NULL;
    retval->next = a_list_o->head;
    a_list_o->head->prev = retval;
    a_list_o->head = retval;
  }
  else
  {
    /* The list is empty. */
    retval->prev = NULL;
    retval->next = NULL;
    a_list_o->head = retval;
    a_list_o->tail = retval;
  }
  a_list_o->count++;

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Pops an item off the head of the list.
 *
 ****************************************************************************/
void *
list_hpop(cw_list_t * a_list_o)
{
  void * retval;

  _cw_check_ptr(a_list_o);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  retval = list_p_hpop(a_list_o);

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Pushes an item onto the tail of the list.
 *
 ****************************************************************************/
cw_list_item_t *
list_tpush(cw_list_t * a_list_o, void * a_data)
{
  cw_list_item_t * retval;

  _cw_check_ptr(a_list_o);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  /* Find a list item somewhere. */
  if (a_list_o->spares_count > 0)
  {
    /* A spare item is available, so use it. */
    retval = a_list_o->spares_head;
    a_list_o->spares_head = a_list_o->spares_head->next;
    a_list_o->spares_count--;
  }
  else
  {
    /* No spares available.  Create a new item. */
    retval = (cw_list_item_t *) _cw_malloc(sizeof(cw_list_item_t));
  }
  retval->item = a_data;
  
  /* Link things together. */
  if (a_list_o->tail != NULL)
  {
    /* The list isn't empty. */
    retval->next = NULL;
    retval->prev = a_list_o->tail;
    a_list_o->tail->next = retval;
    a_list_o->tail = retval;
  }
  else
  {
    /* The list is empty. */
    retval->prev = NULL;
    retval->next = NULL;
    a_list_o->head = retval;
    a_list_o->tail = retval;
  }
  a_list_o->count++;

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Pops an item off the tail of the list.
 *
 ****************************************************************************/
void *
list_tpop(cw_list_t * a_list_o)
{
  void * retval;

  _cw_check_ptr(a_list_o);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  retval = list_p_tpop(a_list_o);

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Inserts an item after the list node pointed to by a_in_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_insert_after(cw_list_t * a_list_o,
		  cw_list_item_t * a_in_list,
		  void * a_data)
{
  cw_list_item_t * retval;
  
  _cw_check_ptr(a_list_o);
  _cw_check_ptr(a_in_list);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  /* Find a list item somewhere. */
  if (a_list_o->spares_count > 0)
  {
    /* A spare item is available, so use it. */
    retval = a_list_o->spares_head;
    a_list_o->spares_head = a_list_o->spares_head->next;
    a_list_o->spares_count--;
  }
  else
  {
    /* No spares available.  Create a new item. */
    retval = (cw_list_item_t *) _cw_malloc(sizeof(cw_list_item_t));
  }
  retval->item = a_data;
  
  if (a_in_list->next == NULL)
  {
    /* Inserting at the end of the list. */
    a_in_list->next = retval;
    retval->prev = a_in_list;
    retval->next = NULL;
  }
  else
  {
    /* Not at the end of the list. */
    retval->prev = a_in_list;
    retval->next = a_in_list->next;
    a_in_list->next = retval;
    retval->next->prev = retval;
  }
  a_list_o->count++;

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Given a pointer to an item, removes the item from the list and returns
 * the data pointer.
 *
 ****************************************************************************/
void *
list_remove(cw_list_t * a_list_o, cw_list_item_t * a_to_remove)
{
  void * retval;

  _cw_check_ptr(a_list_o);
  _cw_check_ptr(a_to_remove);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  if (a_to_remove->prev == NULL)
  {
    /* Removing from the beginning of the list. */
    retval = list_p_hpop(a_list_o);
  }
  else if (a_to_remove->next == NULL)
  {
    /* Removing from the end of the list. */
    retval = list_p_tpop(a_list_o);
  }
  else
  {
    /* Removing from the middle of the list. */
    retval = a_to_remove->item;
    
    a_to_remove->prev->next = a_to_remove->next;
    a_to_remove->next->prev = a_to_remove->prev;

    /* Put item on the spares list. */
    a_to_remove->next = a_list_o->spares_head;
    a_list_o->spares_head = a_to_remove;
    a_list_o->spares_count++;
    
    a_list_o->count--;
  }

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Free the space used by the free item list.
 *
 ****************************************************************************/
void
list_purge_spares(cw_list_t * a_list_o)
{
  cw_list_item_t * item;

  _cw_check_ptr(a_list_o);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }
    
  for (; a_list_o->spares_count > 0; a_list_o->spares_count--)
  {
    item = a_list_o->spares_head;
    a_list_o->spares_head = a_list_o->spares_head->next;
    _cw_free(item);
  }

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Print debugging spew.  Note that the 64 bit values don't print correctly 
 * when using long long for 64 bit variables.
 *
 ****************************************************************************/
void
list_dump(cw_list_t * a_list_o)
{
  _cw_check_ptr(a_list_o);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  log_printf(g_log_o,
	     "=== cw_list_t ==============================================\n");
  log_printf(g_log_o, "is_malloced: [%d], is_thread_safe: [%d]\n",
	     a_list_o->is_malloced, a_list_o->is_thread_safe);
  log_printf(g_log_o, "count: [%d]  spares: [%d]\n",
	     a_list_o->count, a_list_o->spares_count);
  log_printf(g_log_o, "head: [%010p]  tail: [%010p]  spares_head: [%010p]\n",
	     a_list_o->head, a_list_o->tail, a_list_o->spares_head);
  if (a_list_o->count > 0)
  {
    log_printf(g_log_o, "head->item: [%010p]  tail->item: [%010p]\n",
	       a_list_o->head->item, a_list_o->tail->item);
  }
  else
  {
    log_printf(g_log_o, "head->item: [N/A]  tail->item: [N/A]\n");
  }
  log_printf(g_log_o,
	     "============================================================\n");
  
  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Pop an item of the head of the list, without locking.
 *
 ****************************************************************************/
void *
list_p_hpop(cw_list_t * a_list_o)
{
  void * retval;

  if (a_list_o->head == NULL)
  {
    /* List is empty. */
    retval = NULL;
  }
  else if (a_list_o->head == a_list_o->tail)
  {
    /* Only one item in the list. */
    retval = a_list_o->head->item;

    /* Put the item on the spares list. */
    a_list_o->head->next = a_list_o->spares_head;
    a_list_o->spares_head = a_list_o->head;
    a_list_o->spares_count++;

    a_list_o->head = NULL;
    a_list_o->tail = NULL;

    a_list_o->count--;
  }
  else
  {
    cw_list_item_t * temp_ptr;
    
    /* More than one item in the list. */
    retval = a_list_o->head->item;

    temp_ptr = a_list_o->spares_head;
    a_list_o->spares_head = a_list_o->head;

    a_list_o->head = a_list_o->head->next;
    a_list_o->head->prev = NULL;
    /* Done with main list. */
    
    a_list_o->spares_head->next = temp_ptr;
    a_list_o->spares_count++;
    /* Done with spares list. */

    a_list_o->count--;
  }

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Pop an item off the tail of the list, without locking.
 *
 ****************************************************************************/
void *
list_p_tpop(cw_list_t * a_list_o)
{
  void * retval;

  if (a_list_o->tail == NULL)
  {
    /* List is empty. */
    retval = NULL;
  }
  else if (a_list_o->tail == a_list_o->head)
  {
    /* Only one item in the list. */
    retval = a_list_o->tail->item;

    /* Put the item on the spares list. */
    a_list_o->head->next = a_list_o->spares_head;
    a_list_o->spares_head = a_list_o->tail;
    a_list_o->spares_count++;
    
    a_list_o->head = NULL;
    a_list_o->tail = NULL;

    a_list_o->count--;
  }
  else
  {
    cw_list_item_t * temp_ptr;

    /* More than one item in the list. */
    retval = a_list_o->tail->item;

    temp_ptr = a_list_o->spares_head;
    a_list_o->spares_head = a_list_o->tail;

    a_list_o->tail = a_list_o->tail->prev;
    a_list_o->tail->next = NULL;
    /* Done with main list. */

    a_list_o->spares_head->next = temp_ptr;
    a_list_o->spares_count++;
    /* Done with spares list. */
      
    a_list_o->count--;
  }

  return retval;
}
