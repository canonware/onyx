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
 * Dynamic open hashing class.  This is a somewhat sophisticated
 * implementation of hashing.  All internal consistency, growth, shrinkage,
 * etc. issues are taken care of internally.  The hash functions to use, as
 * well as just about any other useful parameter, can be modified on the
 * fly with no worries of inconsistency.  This class is thread safe, thanks
 * to read/write locks.  That is, multiple readers can be in the code
 * simultaneously, but only one locker (with no readers) can be in the code
 * at any given time.
 *
 * This implementation uses a secondary hash function instead of bucket
 * chaining.  As a result, the table needs to be kept a little emptier than
 * some implementations in order to avoid excessive secondary hashing.
 * This seems like a good tradeoff though, since it avoids list management,
 * pointer chasing, and calls to malloc().
 *
 * This code never rehashes during normal operation, because it is careful
 * to shuffle slot contents (items) whenever items are deleted.  This code
 * also keeps an internal list of all items to allow fast table rebuilding
 * when growing, shrinking, and rehashing (done only when hashing functions
 * are changed).  This also makes it possible to use realloc() instead of
 * malloc(), since the table can be bzero()ed, then rebuilt from the list.
 *
 * A useful side effect of the internal list is that calling
 * oh_item_delete_iterate() is guaranteed to remove the oldest item in the 
 * hash table, which means that the hash code has an integrated FIFO
 * queue.
 *
 * A list of spare item containers is kept around to avoid excessive calls
 * to malloc during insertion/deletion.  All internal lists and buffers are
 * kept tidy and at reasonable sizes.
 *
 ****************************************************************************/

#include <string.h>

#define _OH_PERF_

#define _INC_OH_H_
#include <libstash.h>
#include <oh_priv.h>

cw_oh_t *
oh_new(cw_oh_t * a_oh_o, cw_bool_t a_is_thread_safe)
{
  cw_oh_t * retval;

  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_new()");
  }
  if (a_oh_o == NULL)
  {
    retval = (cw_oh_t *) _cw_malloc(sizeof(cw_oh_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_oh_o;
    retval->is_malloced = FALSE;
  }

  if (a_is_thread_safe)
  {
    retval->is_thread_safe = TRUE;
    rwl_new(&retval->rw_lock);
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }

  /* Create the list for items. */
  list_new(&retval->items_list, FALSE);

  retval->size = 1 << _OH_BASE_POWER;

  /* Create the items pointer array. */
  retval->items = (cw_oh_item_t **) _cw_malloc(retval->size
					       * sizeof(cw_oh_item_t *));
  bzero(retval->items, retval->size * sizeof(cw_oh_item_t *));

  /* Create the spare items list. */
  list_new(&retval->spares_list, FALSE);
  
  retval->curr_h1 = oh_p_h1;
  retval->key_compare = oh_p_key_compare;

  retval->curr_power
    = retval->base_power
    = _OH_BASE_POWER;
  retval->curr_h2
    = retval->base_h2
    = _OH_BASE_H2;
  retval->curr_shrink_point
    = retval->base_shrink_point
    = _OH_BASE_SHRINK_POINT;
  retval->curr_grow_point
    = retval->base_grow_point
    = _OH_BASE_GROW_POINT;

#ifdef _OH_PERF_
  retval->num_collisions
    = retval->num_inserts
    = retval->num_deletes
    = retval->num_grows
    = retval->num_shrinks
    = 0;
#endif

  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_new()");
  }
  return retval;
}

void
oh_delete(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_delete()");
  }
  _cw_check_ptr(a_oh_o);

  if (a_oh_o->is_thread_safe)
  {
    rwl_delete(&a_oh_o->rw_lock);
  }

  /* Delete the items in the table, as well as items_list. */
  {
    cw_sint64_t i;
    cw_oh_item_t * item;

    for (i = list_count(&a_oh_o->items_list); i > 0; i--)
    {
      item = (cw_oh_item_t *) list_hpop(&a_oh_o->items_list);
      _cw_free(item);
    }
    list_delete(&a_oh_o->items_list);
  }
  
  /* Delete the spares list. */
  {
    cw_sint64_t i;
    cw_oh_item_t * item;

    for (i = list_count(&a_oh_o->spares_list); i > 0; i--)
    {
      item = (cw_oh_item_t *) list_hpop(&a_oh_o->spares_list);
      _cw_free(item);
    }
    list_delete(&a_oh_o->spares_list);
  }
  
  _cw_free(a_oh_o->items);
  if (a_oh_o->is_malloced == TRUE)
  {
    _cw_free(a_oh_o);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_delete()");
  }
}

cw_uint64_t
oh_get_size(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_size()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_size()");
  }
  return a_oh_o->size;
}

cw_uint64_t
oh_get_num_items(cw_oh_t * a_oh_o)
{
  cw_uint64_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_items()");
  }
  _cw_check_ptr(a_oh_o);
  if (a_oh_o->is_thread_safe)
  {
    rwl_rlock(&a_oh_o->rw_lock);
  }
  
  retval = list_count(&a_oh_o->items_list);
  
  if (a_oh_o->is_thread_safe)
  {
    rwl_runlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_items()");
  }
  return retval;
}

cw_uint64_t
oh_get_base_size(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_base_size()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_base_size()");
  }
  return (1 << a_oh_o->base_power);
}

cw_uint32_t
oh_get_base_h2(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_base_h2()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_base_h2()");
  }
  return a_oh_o->base_h2;
}

cw_uint32_t
oh_get_base_shrink_point(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_base_shrink_point()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_base_shrink_point()");
  }
  return a_oh_o->base_shrink_point;
}

cw_uint32_t
oh_get_base_grow_point(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_base_grow_point()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_base_grow_point()");
  }
  return a_oh_o->base_grow_point;
}

oh_h1_t *
oh_set_h1(cw_oh_t * a_oh_o,
	  oh_h1_t * a_new_h1)
{
  oh_h1_t * retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_h1()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_new_h1);
  if (a_oh_o->is_thread_safe)
  {
    rwl_wlock(&a_oh_o->rw_lock);
  }

  retval = a_oh_o->curr_h1;
  
  if (a_oh_o->curr_h1 != a_new_h1)
  {
    a_oh_o->curr_h1 = a_new_h1;
    oh_p_rehash(a_oh_o);
  }

  if (a_oh_o->is_thread_safe)
  {
    rwl_wunlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_h1()");
  }
  return retval;
}

oh_key_comp_t *
oh_set_key_compare(cw_oh_t * a_oh_o,
		   oh_key_comp_t * a_new_key_compare)
{
  oh_key_comp_t * retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_key_compare()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_new_key_compare);
  if (a_oh_o->is_thread_safe)
  {
    rwl_wlock(&a_oh_o->rw_lock);
  }

  retval = a_oh_o->key_compare;
  
  if (a_oh_o->key_compare != a_new_key_compare)
  {
    a_oh_o->key_compare = a_new_key_compare;
    oh_p_rehash(a_oh_o);
  }
  
  if (a_oh_o->is_thread_safe)
  {
    rwl_wunlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_key_compare()");
  }
  return retval;
}

cw_bool_t
oh_set_base_h2(cw_oh_t * a_oh_o,
	       cw_uint32_t a_h2)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_base_h2()");
  }
  _cw_check_ptr(a_oh_o);

  if (a_oh_o->is_thread_safe)
  {
    rwl_wlock(&a_oh_o->rw_lock);
  }
  
  if (((a_h2 % 2) == 0)
      || (a_h2 > (1 << a_oh_o->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    
    a_oh_o->base_h2 = a_h2;
    a_oh_o->curr_h2 = (((a_oh_o->base_h2 + 1)
			<< (a_oh_o->curr_power
			    - a_oh_o->base_power))
		       - 1);
    oh_p_rehash(a_oh_o);
  }

  if (a_oh_o->is_thread_safe)
  {
    rwl_wunlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_base_h2()");
  }
  return retval;
}

cw_bool_t
oh_set_base_shrink_point(cw_oh_t * a_oh_o,
			 cw_uint32_t a_shrink_point)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_base_shrink_point()");
  }
  _cw_check_ptr(a_oh_o);
  if (a_oh_o->is_thread_safe)
  {
    rwl_wlock(&a_oh_o->rw_lock);
  }

  if ((a_shrink_point >= a_oh_o->base_grow_point)
      || (a_shrink_point >= (1 << a_oh_o->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    
    a_oh_o->base_shrink_point = a_shrink_point;
    a_oh_o->curr_shrink_point
      = (a_oh_o->base_shrink_point
	 << (a_oh_o->curr_power - a_oh_o->base_power));
    oh_p_shrink(a_oh_o);
  }

  if (a_oh_o->is_thread_safe)
  {
    rwl_wunlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_base_shrink_point()");
  }
  return retval;
}

cw_bool_t
oh_set_base_grow_point(cw_oh_t * a_oh_o,
		       cw_uint32_t a_grow_point)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_base_grow_point()");
  }
  _cw_check_ptr(a_oh_o);
  if (a_oh_o->is_thread_safe)
  {
    rwl_wlock(&a_oh_o->rw_lock);
  }

  if ((a_grow_point <= a_oh_o->base_shrink_point)
      || (a_grow_point >= (1 << a_oh_o->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    
    a_oh_o->base_grow_point = a_grow_point;
    a_oh_o->curr_grow_point
      = (a_oh_o->base_grow_point
	 << (a_oh_o->curr_power - a_oh_o->base_power));
    oh_p_grow(a_oh_o);
  }

  if (a_oh_o->is_thread_safe)
  {
    rwl_wunlock(&a_oh_o->rw_lock);
  }

  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_base_grow_point()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Insert an item, unless an item with the same key already exists.
 *
 ****************************************************************************/
cw_bool_t
oh_item_insert(cw_oh_t * a_oh_o, void * a_key,
	       void * a_data)
{
  cw_oh_item_t * item;
  cw_bool_t retval;
  cw_uint64_t junk;

  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_insert()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_key);
  if (a_oh_o->is_thread_safe)
  {
    rwl_wlock(&a_oh_o->rw_lock);
  }

  /* Quiesce. */
  oh_p_shrink(a_oh_o);
  oh_p_grow(a_oh_o);

  if (oh_p_item_search(a_oh_o, a_key, &junk) == TRUE)
  {
    /* Item isn't a duplicate key.  Go ahead and insert it. */
    retval = FALSE;
    
    /* Grab an item off the spares list, if there are any. */
    if (list_count(&a_oh_o->spares_list) > 0)
    {
      item = (cw_oh_item_t *) list_hpop(&a_oh_o->spares_list);
    }
    else
    {
      item = (cw_oh_item_t *) _cw_malloc(sizeof(cw_oh_item_t));
    }

    item->key = a_key;
    item->data = a_data;

    oh_p_item_insert(a_oh_o, item);
  }
  else
  {
    /* An item with this key already exists. */
    retval = TRUE;
  }
  
  if (a_oh_o->is_thread_safe)
  {
    rwl_wunlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_item_insert()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Delete an item with key a_search_key.  If successful, set *a_key and
 * *a_data to point to the key and data, respectively.
 *
 ****************************************************************************/
cw_bool_t
oh_item_delete(cw_oh_t * a_oh_o,
	       void * a_search_key,
	       void ** a_key,
	       void ** a_data)
{
  cw_uint64_t slot;
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_delete()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_search_key);
  if (a_oh_o->is_thread_safe)
  {
    rwl_wlock(&a_oh_o->rw_lock);
  }

  /* Get the slot number for what we want to delete (if it exists). */
  if (oh_p_item_search(a_oh_o, a_search_key, &slot) == FALSE)
  {
    /* Found the item. */
    retval = FALSE;
    
#ifdef _OH_PERF_
    a_oh_o->num_deletes++;
#endif

    /* Set the return variables, decrement the item count, and delete the
       item. */
    *a_key = a_oh_o->items[slot]->key;
    *a_data = a_oh_o->items[slot]->data;
    list_remove(&a_oh_o->items_list, a_oh_o->items[slot]->list_item);
    /* XXX Potentially a good place for an assertion. */

    /* Put the item on the spares list. */
    list_hpush(&a_oh_o->spares_list, (void *) a_oh_o->items[slot]);

    a_oh_o->items[slot] = NULL;

    if (_cw_fmatch(_STASH_DBG_R_OH_SLOT))
    {
      log_eprintf(g_log_o, NULL, 0, "oh_item_delete",
		  "Marking invalid in slot %d\n", slot);
    }

    oh_p_slot_shuffle(a_oh_o, slot);
  }
  else
  {
    /* The item doesn't exist.  Return an error. */
    retval = TRUE;
  }

  if (a_oh_o->is_thread_safe)
  {
    rwl_wunlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_item_delete()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Search for an item with key a_key.  If found, set *a_data to point to
 * the associated data.
 *
 ****************************************************************************/
cw_bool_t
oh_item_search(cw_oh_t * a_oh_o,
	       void * a_key,
	       void ** a_data)
{
  cw_uint64_t slot;
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_search()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_key);
  if (a_oh_o->is_thread_safe)
  {
    rwl_rlock(&a_oh_o->rw_lock);
  }

  if (oh_p_item_search(a_oh_o, a_key, &slot) == FALSE)
  {
    /* Item found. */
    retval = FALSE;
    
    *a_data = a_oh_o->items[slot]->data;
    if (_cw_fmatch(_STASH_DBG_R_OH_SLOT))
    {
      log_eprintf(g_log_o, NULL, 0, "oh_item_search",
		  "Found in slot %d\n", slot);
    }
  }
  else
  {
    /* Item doesn't exist. */
    retval = TRUE;
  }

  if (a_oh_o->is_thread_safe)
  {
    rwl_runlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_item_search()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Searches linearly through the hash table and deletes the first valid
 * item found.
 *
 ****************************************************************************/
cw_bool_t
oh_item_delete_iterate(cw_oh_t * a_oh_o, void ** a_key, void ** a_data)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_delete_iterate()");
  }
  _cw_check_ptr(a_oh_o);
  if (a_oh_o->is_thread_safe)
  {
    rwl_wlock(&a_oh_o->rw_lock);
  }

  if (list_count(&a_oh_o->items_list) > 0)
  {
    cw_oh_item_t * item;

    retval = FALSE;

    item = (cw_oh_item_t *) list_hpop(&a_oh_o->items_list);

    *a_key = item->key;
    *a_data = item->data;

    /* Do slot shuffling. */
    a_oh_o->items[item->slot_num] = NULL;
    oh_p_slot_shuffle(a_oh_o, item->slot_num);

    /* Add item to spares list. */
    list_hpush(&a_oh_o->spares_list, item);
  }
  else
  {
    retval = TRUE;
  }
  
  if (a_oh_o->is_thread_safe)
  {
    rwl_wunlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_item_delete_iterate()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Print the internal state of the hash table.
 *
 ****************************************************************************/
void
oh_dump(cw_oh_t * a_oh_o, cw_bool_t a_all)
{
  cw_uint64_t i;
  char buf_a[21], buf_b[21], buf_c[21];

  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_dump()");
  }
  _cw_check_ptr(a_oh_o);
  
  if (a_oh_o->is_thread_safe)
  {
    rwl_rlock(&a_oh_o->rw_lock);
  }

  log_printf(g_log_o,
	     "============================================================\n");
  log_printf(g_log_o,
	     "Size: [%s]  Slots filled: [%d]\n\n",
	     log_print_uint64(a_oh_o->size, 10, buf_a),
	     list_count(&a_oh_o->items_list));
  log_printf(g_log_o, "      pow h1         h2    shrink grow \n");
  log_printf(g_log_o, "      --- ---------- ----- ------ -----\n");
  log_printf(g_log_o, "Base: %2d             %5d %5d  %5d\n",
	     a_oh_o->base_power,
	     a_oh_o->base_h2,
	     a_oh_o->base_shrink_point,
	     a_oh_o->base_grow_point);
  log_printf(g_log_o, "Curr: %2d  %10p %5s %5s  %5s\n\n",
	     a_oh_o->curr_power,
	     a_oh_o->curr_h1,
	     log_print_uint64(a_oh_o->curr_h2, 10, buf_a),
	     log_print_uint64(a_oh_o->curr_shrink_point, 10, buf_b),
	     log_print_uint64(a_oh_o->curr_grow_point, 10, buf_c));

#ifdef _OH_PERF_
  log_printf(g_log_o, "Counters: collisions[%s] inserts[%s] deletes[%s]\n",
	     log_print_uint64(a_oh_o->num_collisions, 10, buf_a),
	     log_print_uint64(a_oh_o->num_inserts, 10, buf_b),
	     log_print_uint64(a_oh_o->num_deletes, 10, buf_c));
  log_printf(g_log_o, "          grows[%s] shrinks[%s]\n\n",
	     log_print_uint64(a_oh_o->num_grows, 10, buf_a),
	     log_print_uint64(a_oh_o->num_shrinks, 10, buf_b));
#endif

  if (a_all)
  {
    log_printf(g_log_o, "slot key        value\n");
    log_printf(g_log_o, "---- ---------- ----------\n");
  
    for (i = 0; i < a_oh_o->size; i++)
    {
      log_printf(g_log_o, "%4d ", i);
      if (a_oh_o->items[i] != NULL)
      {
	log_printf(g_log_o, "0x%08x %10p\n",
		   a_oh_o->items[i]->key,
		   a_oh_o->items[i]->data);
      }
      else
      {
	log_printf(g_log_o, "\n");
      }
    }
  }
  log_printf(g_log_o,
	     "============================================================\n");
  if (a_oh_o->is_thread_safe)
  {
    rwl_runlock(&a_oh_o->rw_lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_dump()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Default primary hash function.  This is a string hash, so if the keys
 * being used for an oh instance aren't strings, don't use this.
 *
 ****************************************************************************/
cw_uint64_t
oh_p_h1(cw_oh_t * a_oh_o, void * a_key)
{
  cw_uint64_t retval;
  char * str;

  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_p_h1()");
  }
  for (str = (char *) a_key, retval = 0; *str != 0; str++)
  {
    retval = retval * 33 + *str;
  }

  retval = retval % (1 << a_oh_o->curr_power);
  
  if (_cw_fmatch(_STASH_DBG_R_OH_SLOT))
  {
    log_eprintf(g_log_o, NULL, 0, "oh_p_h1",
		"\"%s\" --> %d\n", a_key, retval);
    if (a_oh_o->items[retval] != NULL)
    {
      log_eprintf(g_log_o, NULL, 0, "oh_p_h1",
		  "(collision) items[%d]->key == :%s:\n",
		  retval, a_oh_o->items[retval]->key);
    }
    else
    {
      log_eprintf(g_log_o, NULL, 0, "oh_p_h1",
		  "Slot %d is empty.\n", retval);
    }
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_p_h1()");
  }
  return retval;
}
#if (0)
cw_uint64_t
oh_p_h1(cw_oh_t * a_oh_o, void * a_key)
{
  cw_uint64_t retval;

  _cw_check_ptr(a_oh_o);

  retval = (a_key >> 4) % (1 << a_oh_o->curr_power);

  return retval;
}
#endif

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == Keys are equal.
 *
 * <<< Description >>>
 *
 * Compares two keys for equality.
 *
 ****************************************************************************/
cw_bool_t
oh_p_key_compare(void * a_k1, void * a_k2)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_p_key_compare()");
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_p_key_compare()");
  }
  return strcmp((char *) a_k1, (char *) a_k2) ? FALSE : TRUE;
}

/****************************************************************************
 * <<< Description >>>
 *
 * If the table is too full, double in size and insert into the new table.
 *
 ****************************************************************************/
void
oh_p_grow(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_p_grow()");
  }
  /* Should we grow? */
  if (list_count(&a_oh_o->items_list) >= a_oh_o->curr_grow_point)
  {
#ifdef _OH_PERF_
    a_oh_o->num_grows++;
#endif

    a_oh_o->size <<= 1;
  
    /* Allocate new table */
    a_oh_o->items
      = (cw_oh_item_t **) _cw_realloc(a_oh_o->items,
				      a_oh_o->size * sizeof(cw_oh_item_t *));
    bzero(a_oh_o->items, (a_oh_o->size * sizeof(cw_oh_item_t *)));

    /* Re-calculate curr_* fields. */
    a_oh_o->curr_power += 1;
    a_oh_o->curr_h2 = (((a_oh_o->base_h2 + 1)
			<< (a_oh_o->curr_power
			    - a_oh_o->base_power))
		       - 1);
    a_oh_o->curr_shrink_point
      = a_oh_o->curr_shrink_point << 1;
    a_oh_o->curr_grow_point
      = a_oh_o->curr_grow_point << 1;

  /* Iterate through old table and insert items into new table. */
    {
      cw_oh_item_t * item;
      cw_uint64_t i;
    
      for (i = list_count(&a_oh_o->items_list); i > 0; i--)
      {
	item = (cw_oh_item_t *) list_hpop(&a_oh_o->items_list);
	oh_p_item_insert(a_oh_o, item);
      }
    }
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_p_grow()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * If the table is too empty, shrink it as small as possible, without
 * making it so small that the table would need to immediately grow again.
 *
 ****************************************************************************/
void
oh_p_shrink(cw_oh_t * a_oh_o)
{
  cw_uint32_t j;
  cw_uint32_t num_halvings;

  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_p_shrink()");
  }
  /* Should we shrink? */
  if ((list_count(&a_oh_o->items_list) < a_oh_o->curr_shrink_point)
      && (a_oh_o->curr_power > a_oh_o->base_power))
  {

    for (j = a_oh_o->curr_power - a_oh_o->base_power;
	 (((a_oh_o->curr_grow_point >> j) < list_count(&a_oh_o->items_list))
	  && (j > 0));
	 j--);
    num_halvings = j;

    /* We're not shrinking below the base table size, are we? */
    _cw_assert((a_oh_o->curr_power - num_halvings) >= a_oh_o->base_power);
  
#ifdef _OH_PERF_
    a_oh_o->num_shrinks++;
#endif

    a_oh_o->size >>= num_halvings;
  
    /* Allocate new table */
    a_oh_o->items
      = (cw_oh_item_t **) _cw_realloc(a_oh_o->items,
				      a_oh_o->size * sizeof(cw_oh_item_t *));
    bzero(a_oh_o->items, (a_oh_o->size * sizeof(cw_oh_item_t *)));
  
    /* Re-calculate curr_* fields. */
    a_oh_o->curr_power -= num_halvings;
    a_oh_o->curr_h2 = (((a_oh_o->base_h2 + 1)
			<< (a_oh_o->curr_power
			    - a_oh_o->base_power))
		       - 1);
    a_oh_o->curr_shrink_point
      = a_oh_o->curr_shrink_point >> num_halvings;
    a_oh_o->curr_grow_point
      = a_oh_o->curr_grow_point >> num_halvings;

    /* Iterate through old table and insert items into new table. */
    {
      cw_uint64_t i;
      cw_oh_item_t * item;
    
      for (i = list_count(&a_oh_o->items_list); i > 0; i--)
      {
	item = list_hpop(&a_oh_o->items_list);
	oh_p_item_insert(a_oh_o, item);
      }
    }

    /* Purge the spares in the items_list. */
    list_purge_spares(&a_oh_o->items_list);
  
    /* Shrink the spares list down to a reasonable size. */
    {
      cw_sint64_t i;
      cw_oh_item_t * item;

      for (i = list_count(&a_oh_o->spares_list);
	   i > a_oh_o->curr_grow_point;
	   i--)
      {
	item = (cw_oh_item_t *) list_hpop(&a_oh_o->spares_list);
	_cw_free(item);
      }
      list_purge_spares(&a_oh_o->spares_list);
    }
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_p_shrink()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Find the slot that we should insert into, given a_item, and insert.
 *
 ****************************************************************************/
void
oh_p_item_insert(cw_oh_t * a_oh_o,
		 cw_oh_item_t * a_item)
{
  cw_uint64_t slot, i, j;
  cw_bool_t retval = TRUE;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_p_item_insert()");
  }
  /* Primary hash to first possible location to insert. */
  slot = a_oh_o->curr_h1(a_oh_o, a_item->key);

  for (i = 0, j = slot;
       i < a_oh_o->size;
       i++, j = (j + a_oh_o->curr_h2) % a_oh_o->size)
  {
    if (a_oh_o->items[j] == NULL)
    {
#ifdef _OH_PERF_
      a_oh_o->num_inserts++;
#endif
      /* This slot is unused, so insert. */
      a_item->slot_num = j;
      a_item->jumps = i; /* For deletion shuffling. */
      a_oh_o->items[j] = a_item;

      /* Wow, this looks knarly.  What we're doing here is adding the item
       * to the items_list, then setting the list_item pointer inside the
       * item, so that we can rip the item out of the list when
       * deleting. */
      a_item->list_item = list_tpush(&a_oh_o->items_list, a_item);
      retval = FALSE;
      if (_cw_fmatch(_STASH_DBG_R_OH_SLOT))
      {
	log_eprintf(g_log_o, NULL, 0, "oh_p_item_insert",
		    "Inserting in slot %d\n", slot);
      }
      break;
    }
#ifdef _OH_PERF_
    else
    {
      a_oh_o->num_collisions++;
    }
#endif
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_p_item_insert()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Uses the primary and secondary hash function to search for an item with
 * key == a_key.
 *
 ****************************************************************************/
cw_bool_t
oh_p_item_search(cw_oh_t * a_oh_o,
		 void * a_key,
		 cw_uint64_t * a_slot)
{
  cw_uint64_t slot, i, j;
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_p_item_search()");
  }
  /* Primary hash to the first location to look. */
  slot = a_oh_o->curr_h1(a_oh_o, a_key);

  /* Jump by the secondary hash value until we either find what we're
   * looking for, or hit an empty slot. */
  for (i = 0, j = slot;
       ;
       i++, j = (j + a_oh_o->curr_h2) % a_oh_o->size)
  {
    if (a_oh_o->items[j] == NULL)
    {
      /* Hit an empty slot.  What we're looking for isn't here. */
      retval = TRUE;
      break;
    }
    else if (a_oh_o->key_compare(a_oh_o->items[j]->key, a_key) == TRUE)
    {
      /* Found it. */
      *a_slot = j;
      retval = FALSE;
      break;
    }
  }
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_p_item_search()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Rehash.
 *
 ****************************************************************************/
void
oh_p_rehash(cw_oh_t * a_oh_o)
{
  cw_uint64_t i;
  cw_oh_item_t * item;

  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_p_rehash()");
  }
  /* Clear the table. */
  bzero(a_oh_o->items, a_oh_o->size * sizeof(cw_oh_item_t *));
  
  /* Iterate through old table and rehash them. */
  for (i = list_count(&a_oh_o->items_list); i > 0; i--)
  {
    item = (cw_oh_item_t *) list_hpop(&a_oh_o->items_list);
    oh_p_item_insert(a_oh_o, item);
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_p_rehash()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 *  Figure out whether there are any items that bounced past this slot
 *  using the secondary hash.  If so, shuffle things backward to fill this
 *  slot in.  We know we've looked far enough forward when we hit an empty
 *  slot.
 *
 ****************************************************************************/
void
oh_p_slot_shuffle(cw_oh_t * a_oh_o, cw_uint64_t a_slot)
{
  cw_uint64_t i, curr_empty, curr_look, curr_distance;
  
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_p_slot_shuffle()");
  }
  for(i = 0,
	curr_distance = 1,
	curr_empty = a_slot,
	curr_look = ((curr_empty + a_oh_o->curr_h2) % a_oh_o->size);
      ((a_oh_o->items[curr_look] != NULL) && (i < a_oh_o->size));
      i++,
	curr_distance++,
	curr_look = (curr_look + a_oh_o->curr_h2) % a_oh_o->size)
  {
    /* See if this item had to jump at least the current distance to
     * the last empty slot in this secondary hash chain. */
    if (a_oh_o->items[curr_look]->jumps >= curr_distance)
    {
      /* This item should be shuffled back to the previous empty slot.
       * Do so, and reset curr_distance and curr_empty.  Also, update
       * the jumps field of the item we just shuffled, as well as its
       * record of the slot that it's now in. */
      if (_cw_fmatch(_STASH_DBG_R_OH_SLOT))
      {
	log_eprintf(g_log_o, NULL, 0, "oh_p_slot_shuffle",
		    "Shuffling slot %d to %d (%d jumps)\n",
		    curr_look, curr_empty, curr_distance);
      }
	  
      a_oh_o->items[curr_empty] = a_oh_o->items[curr_look];
      a_oh_o->items[curr_empty]->jumps -= curr_distance;
      a_oh_o->items[curr_empty]->slot_num = curr_empty;
      a_oh_o->items[curr_look] = NULL;

      curr_empty = curr_look;
      curr_distance = 0;
    }
  }
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_p_slot_shuffle()");
  }
}

#ifdef _OH_PERF_
cw_uint64_t
oh_get_num_collisions(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_collisions()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_collisions()");
  }
  return a_oh_o->num_collisions;
}

cw_uint64_t
oh_get_num_inserts(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_inserts()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_inserts()");
  }
  return a_oh_o->num_inserts;
}

cw_uint64_t
oh_get_num_deletes(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_deletes()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_deletes()");
  }
  return a_oh_o->num_deletes;
}

cw_uint64_t
oh_get_num_grows(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_grows()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_grows()");
  }
  return a_oh_o->num_grows;
}

cw_uint64_t
oh_get_num_shrinks(cw_oh_t * a_oh_o)
{
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_shrinks()");
  }
  _cw_check_ptr(a_oh_o);
  if (_cw_pmatch(_STASH_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_shrinks()");
  }
  return a_oh_o->num_shrinks;
}

#endif
