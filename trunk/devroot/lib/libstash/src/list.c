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
 ****************************************************************************/

#define _LIBSTASH_USE_LIST
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include "libstash/list_p.h"

cw_list_item_t *
list_item_new()
{
  cw_list_item_t * retval;

  retval = (cw_list_item_t *) _cw_malloc(sizeof(cw_list_item_t));

  return retval;
}

void
list_item_delete(cw_list_item_t * a_list_item)
{
  _cw_check_ptr(a_list_item);

  _cw_free(a_list_item);
}

void *
list_item_get(cw_list_item_t * a_list_item)
{
  _cw_check_ptr(a_list_item);

  return a_list_item->item;
}

void
list_item_set(cw_list_item_t * a_list_item, void * a_data)
{
  _cw_check_ptr(a_list_item);

  a_list_item->item = a_data;
}

cw_list_t *
#ifdef _CW_REENTRANT
list_new(cw_list_t * a_list, cw_bool_t a_is_thread_safe)
#else
     list_new(cw_list_t * a_list)
#endif
{
  cw_list_t * retval;

  if (a_list == NULL)
  {
    retval = (cw_list_t *) _cw_malloc(sizeof(cw_list_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_list;
    retval->is_malloced = FALSE;
  }

#ifdef _CW_REENTRANT
  if (a_is_thread_safe)
  {
    retval->is_thread_safe = TRUE;
    mtx_new(&retval->lock);
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
#endif
  
  retval->head = NULL;
  retval->tail = NULL;
  retval->count = 0;
  retval->spares_head = NULL;
  retval->spares_count = 0;

  return retval;
}
     
void
list_delete(cw_list_t * a_list)
{
  cw_list_item_t * item;
  cw_uint64_t i;
  _cw_check_ptr(a_list);

  /* Delete whatever items are still in the list.  This does *not* free
   * memory pointed to by the item pointers. */
  for(i = 0; i < a_list->count; i++)
  {
    item = a_list->head;
    a_list->head = list_item_p_get_next(a_list->head);
    list_item_delete(item);
  }
  
  /* Delete the spares list. */
  for (i = 0; i < a_list->spares_count; i++)
  {
    item = a_list->spares_head;
    a_list->spares_head = list_item_p_get_next(a_list->spares_head);
    list_item_delete(item);
  }

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_delete(&a_list->lock);
  }
#endif
  if (a_list->is_malloced)
  {
    _cw_free(a_list);
  }
}

cw_uint64_t
list_count(cw_list_t * a_list)
{
  cw_uint64_t retval;

  _cw_check_ptr(a_list);

  retval = a_list->count;
  
  return retval;
}

cw_list_item_t *
list_hpush(cw_list_t * a_list, void * a_data)
{
  cw_list_item_t * retval;
  
  _cw_check_ptr(a_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  /* Find a list item somewhere. */
  if (a_list->spares_count > 0)
  {
    /* A spare item is available, so use it. */
    retval = a_list->spares_head;
    a_list->spares_head = list_item_p_get_next(a_list->spares_head);
    a_list->spares_count--;
  }
  else
  {
    /* No spares available.  Create a new item. */
    retval = list_item_new();
  }
  list_item_set(retval, a_data);

  /* Link things together. */
  if (a_list->head != NULL)
  {
    /* The list isn't empty. */
    list_item_p_set_prev(retval, NULL);
    list_item_p_set_next(retval, a_list->head);
    list_item_p_set_prev(a_list->head, retval);
    a_list->head = retval;
  }
  else
  {
    /* The list is empty. */
    list_item_p_set_prev(retval, NULL);
    list_item_p_set_next(retval, NULL);
    a_list->head = retval;
    a_list->tail = retval;
  }
  a_list->count++;

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

void *
list_hpop(cw_list_t * a_list)
{
  void * retval;

  _cw_check_ptr(a_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  retval = list_p_hpop(a_list);

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

void *
list_hpeek(cw_list_t * a_list)
{
  void * retval;

  _cw_check_ptr(a_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  if (a_list->head == NULL)
  {
    /* List is empty. */
    retval = NULL;
  }
  else
  {
    retval = list_item_get(a_list->head);
  }

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

cw_list_item_t *
list_tpush(cw_list_t * a_list, void * a_data)
{
  cw_list_item_t * retval;

  _cw_check_ptr(a_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  /* Find a list item somewhere. */
  if (a_list->spares_count > 0)
  {
    /* A spare item is available, so use it. */
    retval = a_list->spares_head;
    a_list->spares_head = list_item_p_get_next(a_list->spares_head);
    a_list->spares_count--;
  }
  else
  {
    /* No spares available.  Create a new item. */
    retval = list_item_new();
  }
  list_item_set(retval, a_data);
  
  /* Link things together. */
  if (a_list->tail != NULL)
  {
    /* The list isn't empty. */
    list_item_p_set_next(retval, NULL);
    list_item_p_set_prev(retval, a_list->tail);
    list_item_p_set_next(a_list->tail, retval);
    a_list->tail = retval;
  }
  else
  {
    /* The list is empty. */
    list_item_p_set_prev(retval, NULL);
    list_item_p_set_next(retval, NULL);
    a_list->head = retval;
    a_list->tail = retval;
  }
  a_list->count++;

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

void *
list_tpop(cw_list_t * a_list)
{
  void * retval;

  _cw_check_ptr(a_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  retval = list_p_tpop(a_list);

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

void *
list_tpeek(cw_list_t * a_list)
{
  void * retval;

  _cw_check_ptr(a_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  if (a_list->head == NULL)
  {
    /* List is empty. */
    retval = NULL;
  }
  else
  {
    retval = list_item_get(a_list->tail);
  }

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

cw_list_item_t *
list_insert_before(cw_list_t * a_list,
		   cw_list_item_t * a_in_list,
		   void * a_data)
{
  cw_list_item_t * retval;
  
  _cw_check_ptr(a_list);
  _cw_check_ptr(a_in_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  /* Find a list item somewhere. */
  if (a_list->spares_count > 0)
  {
    /* A spare item is available, so use it. */
    retval = a_list->spares_head;
    a_list->spares_head = list_item_p_get_next(a_list->spares_head);
    a_list->spares_count--;
  }
  else
  {
    /* No spares available.  Create a new item. */
    retval = list_item_new();
  }
  list_item_set(retval, a_data);

  if (list_item_p_get_prev(a_in_list) == NULL)
  {
    /* Inserting at the beginning of the list. */
    list_item_p_set_prev(a_in_list, retval);
    list_item_p_set_next(retval, a_in_list);
    list_item_p_set_prev(retval, NULL);
    a_list->head = retval;
  }
  else
  {
    /* Not at the beginning of the list. */
    list_item_p_set_next(retval, a_in_list);
    list_item_p_set_prev(retval, list_item_p_get_prev(a_in_list));
    list_item_p_set_prev(a_in_list, retval);
    list_item_p_set_next(list_item_p_get_prev(retval), retval);
  }
  a_list->count++;

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

cw_list_item_t *
list_insert_after(cw_list_t * a_list,
		  cw_list_item_t * a_in_list,
		  void * a_data)
{
  cw_list_item_t * retval;
  
  _cw_check_ptr(a_list);
  _cw_check_ptr(a_in_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  /* Find a list item somewhere. */
  if (a_list->spares_count > 0)
  {
    /* A spare item is available, so use it. */
    retval = a_list->spares_head;
    a_list->spares_head = list_item_p_get_next(a_list->spares_head);
    a_list->spares_count--;
  }
  else
  {
    /* No spares available.  Create a new item. */
    retval = list_item_new();
  }
  list_item_set(retval, a_data);
  
  if (list_item_p_get_next(a_in_list) == NULL)
  {
    /* Inserting at the end of the list. */
    list_item_p_set_next(a_in_list, retval);
    list_item_p_set_prev(retval, a_in_list);
    list_item_p_set_next(retval, NULL);
    a_list->tail = retval;
  }
  else
  {
    /* Not at the end of the list. */
    list_item_p_set_prev(retval, a_in_list);
    list_item_p_set_next(retval, list_item_p_get_next(a_in_list));
    list_item_p_set_next(a_in_list, retval);
    list_item_p_set_prev(list_item_p_get_next(retval), retval);
  }
  a_list->count++;

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

void *
list_remove(cw_list_t * a_list, cw_list_item_t * a_to_remove)
{
  void * retval;

  _cw_check_ptr(a_list);
  _cw_check_ptr(a_to_remove);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  if (list_item_p_get_prev(a_to_remove) == NULL)
  {
    /* Removing from the beginning of the list. */
    retval = list_p_hpop(a_list);
  }
  else if (list_item_p_get_next(a_to_remove) == NULL)
  {
    /* Removing from the end of the list. */
    retval = list_p_tpop(a_list);
  }
  else
  {
    /* Removing from the middle of the list. */
    retval = list_item_get(a_to_remove);

    list_item_p_set_next(list_item_p_get_prev(a_to_remove),
			 list_item_p_get_next(a_to_remove));
    list_item_p_set_prev(list_item_p_get_next(a_to_remove),
			 list_item_p_get_prev(a_to_remove));

    /* Put item on the spares list. */
    list_item_p_set_next(a_to_remove, a_list->spares_head);
    a_list->spares_head = a_to_remove;
    a_list->spares_count++;
    
    a_list->count--;
  }

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
  return retval;
}

void
list_purge_spares(cw_list_t * a_list)
{
  cw_list_item_t * item;

  _cw_check_ptr(a_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif
    
  for (; a_list->spares_count > 0; a_list->spares_count--)
  {
    item = a_list->spares_head;
    a_list->spares_head = list_item_p_get_next(a_list->spares_head);
    list_item_delete(item);
  }

#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
}

void
list_dump(cw_list_t * a_list)
{
  _cw_check_ptr(a_list);
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_lock(&a_list->lock);
  }
#endif

  log_printf(cw_g_log,
	     "=== cw_list_t ==============================================\n");
#ifdef _CW_REENTRANT
  log_printf(cw_g_log, "is_malloced: [%d], is_thread_safe: [%d]\n",
	     a_list->is_malloced, a_list->is_thread_safe);
#else
  log_printf(cw_g_log, "is_malloced: [%d]\n", a_list->is_malloced);
#endif
  {
    char buf_a[21], buf_b[21];
    
    log_printf(cw_g_log, "count: [%s]  spares: [%s]\n",
	       log_print_uint64(a_list->count, 10, buf_a),
	       log_print_uint64(a_list->spares_count, 10, buf_b));
  }
  
  log_printf(cw_g_log, "head: [%010p]  tail: [%010p]  spares_head: [%010p]\n",
	     a_list->head, a_list->tail, a_list->spares_head);
  if (a_list->count > 0)
  {
    log_printf(cw_g_log, "head->item: [%010p]  tail->item: [%010p]\n",
	       list_item_get(a_list->head), list_item_get(a_list->tail));
  }
  else
  {
    log_printf(cw_g_log, "head->item: [N/A]  tail->item: [N/A]\n");
  }
  log_printf(cw_g_log,
	     "============================================================\n");
  
#ifdef _CW_REENTRANT
  if (a_list->is_thread_safe)
  {
    mtx_unlock(&a_list->lock);
  }
#endif
}

static void *
list_p_hpop(cw_list_t * a_list)
{
  void * retval;

  if (a_list->head == NULL)
  {
    /* List is empty. */
    retval = NULL;
  }
  else if (a_list->head == a_list->tail)
  {
    /* Only one item in the list. */
    retval = list_item_get(a_list->head);

    /* Put the item on the spares list. */
    list_item_p_set_next(a_list->head, a_list->spares_head);
    a_list->spares_head = a_list->head;
    a_list->spares_count++;

    a_list->head = NULL;
    a_list->tail = NULL;

    a_list->count--;
  }
  else
  {
    cw_list_item_t * temp_ptr;
    
    /* More than one item in the list. */
    retval = list_item_get(a_list->head);

    temp_ptr = a_list->spares_head;
    a_list->spares_head = a_list->head;

    a_list->head = list_item_p_get_next(a_list->head);
    list_item_p_set_prev(a_list->head, NULL);
    /* Done with main list. */
    
    list_item_p_set_next(a_list->spares_head, temp_ptr);
    a_list->spares_count++;
    /* Done with spares list. */

    a_list->count--;
  }

  return retval;
}

static void *
list_p_tpop(cw_list_t * a_list)
{
  void * retval;

  if (a_list->tail == NULL)
  {
    /* List is empty. */
    retval = NULL;
  }
  else if (a_list->tail == a_list->head)
  {
    /* Only one item in the list. */
    retval = list_item_get(a_list->tail);

    /* Put the item on the spares list. */
    list_item_p_set_next(a_list->head, a_list->spares_head);
    a_list->spares_head = a_list->tail;
    a_list->spares_count++;
    
    a_list->head = NULL;
    a_list->tail = NULL;

    a_list->count--;
  }
  else
  {
    cw_list_item_t * temp_ptr;

    /* More than one item in the list. */
    retval = list_item_get(a_list->tail);

    temp_ptr = a_list->spares_head;
    a_list->spares_head = a_list->tail;

    a_list->tail = list_item_p_get_prev(a_list->tail);
    list_item_p_set_next(a_list->tail, NULL);
    /* Done with main list. */

    list_item_p_set_next(a_list->spares_head, temp_ptr);
    a_list->spares_count++;
    /* Done with spares list. */
      
    a_list->count--;
  }

  return retval;
}
