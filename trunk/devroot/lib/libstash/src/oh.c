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

#define _LIBSTASH_USE_OH
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include "libstash/mem_l.h"
#include "libstash/oh_p.h"

cw_oh_t *
#ifdef _CW_REENTRANT
oh_new(cw_oh_t * a_oh, cw_bool_t a_is_thread_safe)
#else
     oh_new(cw_oh_t * a_oh)
#endif
{
  cw_oh_t * retval;

  if (a_oh == NULL)
  {
    retval = (cw_oh_t *) _cw_malloc(sizeof(cw_oh_t));
    _cw_check_ptr(retval);
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_oh;
    retval->is_malloced = FALSE;
  }

#ifdef _CW_REENTRANT
  if (a_is_thread_safe)
  {
    retval->is_thread_safe = TRUE;
    rwl_new(&retval->rw_lock);
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
#endif

  /* Create the list for items. */
#ifdef _CW_REENTRANT
  list_new(&retval->items_list, FALSE);
#else
  list_new(&retval->items_list);
#endif

  retval->size = 1 << _OH_BASE_POWER;

  /* Create the items pointer array. */
  retval->items = (cw_oh_item_t **) _cw_malloc(retval->size
					       * sizeof(cw_oh_item_t *));
  _cw_check_ptr(retval->items);
  bzero(retval->items, retval->size * sizeof(cw_oh_item_t *));

  /* Create the spare items list. */
#ifdef _CW_REENTRANT
  list_new(&retval->spares_list, FALSE);
#else
  list_new(&retval->spares_list);
#endif
  
  retval->curr_h1 = oh_h1_string;
  retval->key_compare = oh_key_compare_string;

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

  retval->num_collisions
    = retval->num_inserts
    = retval->num_deletes
    = retval->num_grows
    = retval->num_shrinks
    = 0;

  return retval;
}

void
oh_delete(cw_oh_t * a_oh)
{
  _cw_check_ptr(a_oh);

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_delete(&a_oh->rw_lock);
  }
#endif

  /* Delete the items in the table, as well as items_list. */
  {
    cw_sint64_t i;
    cw_oh_item_t * item;

    for (i = list_count(&a_oh->items_list); i > 0; i--)
    {
      item = (cw_oh_item_t *) list_hpop(&a_oh->items_list);
      _cw_free(item);
    }
    list_delete(&a_oh->items_list);
  }
  
  /* Delete the spares list. */
  {
    cw_sint64_t i;
    cw_oh_item_t * item;

    for (i = list_count(&a_oh->spares_list); i > 0; i--)
    {
      item = (cw_oh_item_t *) list_hpop(&a_oh->spares_list);
      _cw_free(item);
    }
    list_delete(&a_oh->spares_list);
  }
  
  _cw_free(a_oh->items);
  if (a_oh->is_malloced == TRUE)
  {
    _cw_free(a_oh);
  }
}

cw_uint64_t
oh_get_size(cw_oh_t * a_oh)
{
  cw_uint64_t retval;

  _cw_check_ptr(a_oh);

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->size;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint64_t
oh_get_num_items(cw_oh_t * a_oh)
{
  cw_uint64_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif
  
  retval = list_count(&a_oh->items_list);
  
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint64_t
oh_get_base_size(cw_oh_t * a_oh)
{
  cw_uint64_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = (1 << a_oh->base_power);

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint32_t
oh_get_base_h2(cw_oh_t * a_oh)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->base_h2;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint32_t
oh_get_base_shrink_point(cw_oh_t * a_oh)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->base_shrink_point;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint32_t
oh_get_base_grow_point(cw_oh_t * a_oh)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->base_grow_point;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

oh_h1_t *
oh_set_h1(cw_oh_t * a_oh,
	  oh_h1_t * a_new_h1)
{
  oh_h1_t * retval;
  
  _cw_check_ptr(a_oh);
  _cw_check_ptr(a_new_h1);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->curr_h1;
  
  if (a_oh->curr_h1 != a_new_h1)
  {
    a_oh->curr_h1 = a_new_h1;
    oh_p_rehash(a_oh);
  }

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

oh_key_comp_t *
oh_set_key_compare(cw_oh_t * a_oh,
		   oh_key_comp_t * a_new_key_compare)
{
  oh_key_comp_t * retval;
  
  _cw_check_ptr(a_oh);
  _cw_check_ptr(a_new_key_compare);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->key_compare;
  
  if (a_oh->key_compare != a_new_key_compare)
  {
    a_oh->key_compare = a_new_key_compare;
    oh_p_rehash(a_oh);
  }
  
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_bool_t
oh_set_base_h2(cw_oh_t * a_oh,
	       cw_uint32_t a_h2)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif
  
  if (((a_h2 % 2) == 0)
      || (a_h2 > (1 << a_oh->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    
    a_oh->base_h2 = a_h2;
    a_oh->curr_h2 = (((a_oh->base_h2 + 1)
			<< (a_oh->curr_power
			    - a_oh->base_power))
		       - 1);
    oh_p_rehash(a_oh);
  }

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_bool_t
oh_set_base_shrink_point(cw_oh_t * a_oh,
			 cw_uint32_t a_shrink_point)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif

  if ((a_shrink_point >= a_oh->base_grow_point)
      || (a_shrink_point >= (1 << a_oh->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    
    a_oh->base_shrink_point = a_shrink_point;
    a_oh->curr_shrink_point
      = (a_oh->base_shrink_point
	 << (a_oh->curr_power - a_oh->base_power));
    oh_p_shrink(a_oh);
  }

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_bool_t
oh_set_base_grow_point(cw_oh_t * a_oh,
		       cw_uint32_t a_grow_point)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif

  if ((a_grow_point <= a_oh->base_shrink_point)
      || (a_grow_point >= (1 << a_oh->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    
    a_oh->base_grow_point = a_grow_point;
    a_oh->curr_grow_point
      = (a_oh->base_grow_point
	 << (a_oh->curr_power - a_oh->base_power));
    oh_p_grow(a_oh);
  }

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_bool_t
oh_item_insert(cw_oh_t * a_oh, const void * a_key, const void * a_data)
{
  cw_oh_item_t * item;
  cw_bool_t retval;
  cw_uint64_t junk;

  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif

  /* Quiesce. */
  oh_p_shrink(a_oh);
  oh_p_grow(a_oh);

  if (oh_p_item_search(a_oh, a_key, &junk) == TRUE)
  {
    /* Item isn't a duplicate key.  Go ahead and insert it. */
    retval = FALSE;
    
    /* Grab an item off the spares list, if there are any. */
    if (list_count(&a_oh->spares_list) > 0)
    {
      item = (cw_oh_item_t *) list_hpop(&a_oh->spares_list);
    }
    else
    {
      item = (cw_oh_item_t *) _cw_malloc(sizeof(cw_oh_item_t));
      _cw_check_ptr(item);
    }

    item->key = a_key;
    item->data = a_data;

    oh_p_item_insert(a_oh, item);
  }
  else
  {
    /* An item with this key already exists. */
    retval = TRUE;
  }
  
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_bool_t
oh_item_delete(cw_oh_t * a_oh,
	       const void * a_search_key,
	       void ** a_key,
	       void ** a_data)
{
  cw_uint64_t slot;
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh);

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif

  /* Get the slot number for what we want to delete (if it exists). */
  if (oh_p_item_search(a_oh, a_search_key, &slot) == FALSE)
  {
    /* Found the item. */
    retval = FALSE;
    
    a_oh->num_deletes++;

    /* Set the return variables, decrement the item count, and delete the
       item. */
    *a_key = (void *) a_oh->items[slot]->key;
    *a_data = (void *) a_oh->items[slot]->data;
    list_remove_container(&a_oh->items_list, a_oh->items[slot]->list_item);

    /* Put the item on the spares list. */
    list_hpush(&a_oh->spares_list, (void *) a_oh->items[slot]);

    a_oh->items[slot] = NULL;

    oh_p_slot_shuffle(a_oh, slot);
  }
  else
  {
    /* The item doesn't exist.  Return an error. */
    retval = TRUE;
  }

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_bool_t
oh_item_search(cw_oh_t * a_oh,
	       const void * a_key,
	       void ** a_data)
{
  cw_uint64_t slot;
  cw_bool_t retval;

  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  if (oh_p_item_search(a_oh, a_key, &slot) == FALSE)
  {
    /* Item found. */
    retval = FALSE;

    *a_data = (void *) a_oh->items[slot]->data;
  }
  else
  {
    /* Item doesn't exist. */
    retval = TRUE;
  }

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_bool_t
oh_item_get_iterate(cw_oh_t * a_oh, void ** a_key, void ** a_data)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif

  if (list_count(&a_oh->items_list) > 0)
  {
    cw_oh_item_t * item;

    retval = FALSE;

    item = (cw_oh_item_t *) list_hpop(&a_oh->items_list);

    *a_key = (void *) item->key;
    *a_data = (void *) item->data;

    /* Put the item back on the tail of the list. */
    list_tpush(&a_oh->items_list, item);
  }
  else
  {
    retval = TRUE;
  }
  
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_bool_t
oh_item_delete_iterate(cw_oh_t * a_oh, void ** a_key, void ** a_data)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wlock(&a_oh->rw_lock);
  }
#endif

  if (list_count(&a_oh->items_list) > 0)
  {
    cw_oh_item_t * item;

    retval = FALSE;

    item = (cw_oh_item_t *) list_hpop(&a_oh->items_list);

    *a_key = (void *) item->key;
    *a_data = (void *) item->data;

    /* Do slot shuffling. */
    a_oh->items[item->slot_num] = NULL;
    oh_p_slot_shuffle(a_oh, item->slot_num);

    /* Add item to spares list. */
    list_hpush(&a_oh->spares_list, item);
  }
  else
  {
    retval = TRUE;
  }
  
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_wunlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

void
oh_dump(cw_oh_t * a_oh, cw_bool_t a_all)
{
  cw_uint64_t i;
  char buf_a[21], buf_b[21], buf_c[21];

  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  log_printf(cw_g_log,
	     "============================================================\n");
  log_printf(cw_g_log,
	     "Size: [%s]  Slots filled: [%d]\n\n",
	     log_print_uint64(a_oh->size, 10, buf_a),
	     list_count(&a_oh->items_list));
  log_printf(cw_g_log, "      pow h1         h2    shrink grow \n");
  log_printf(cw_g_log, "      --- ---------- ----- ------ -----\n");
  log_printf(cw_g_log, "Base: %2d             %5d %5d  %5d\n",
	     a_oh->base_power,
	     a_oh->base_h2,
	     a_oh->base_shrink_point,
	     a_oh->base_grow_point);
  log_printf(cw_g_log, "Curr: %2d  %10p %5s %5s  %5s\n\n",
	     a_oh->curr_power,
	     a_oh->curr_h1,
	     log_print_uint64(a_oh->curr_h2, 10, buf_a),
	     log_print_uint64(a_oh->curr_shrink_point, 10, buf_b),
	     log_print_uint64(a_oh->curr_grow_point, 10, buf_c));

  log_printf(cw_g_log, "Counters: collisions[%s] inserts[%s] deletes[%s]\n",
	     log_print_uint64(a_oh->num_collisions, 10, buf_a),
	     log_print_uint64(a_oh->num_inserts, 10, buf_b),
	     log_print_uint64(a_oh->num_deletes, 10, buf_c));
  log_printf(cw_g_log, "          grows[%s] shrinks[%s]\n\n",
	     log_print_uint64(a_oh->num_grows, 10, buf_a),
	     log_print_uint64(a_oh->num_shrinks, 10, buf_b));

  if (a_all)
  {
    log_printf(cw_g_log, "slot key        value\n");
    log_printf(cw_g_log, "---- ---------- ----------\n");
  
    for (i = 0; i < a_oh->size; i++)
    {
      log_printf(cw_g_log, "%4d ", i);
      if (a_oh->items[i] != NULL)
      {
	log_printf(cw_g_log, "0x%08x %10p\n",
		   a_oh->items[i]->key,
		   a_oh->items[i]->data);
      }
      else
      {
	log_printf(cw_g_log, "\n");
      }
    }
  }
  log_printf(cw_g_log,
	     "============================================================\n");

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
}

cw_uint64_t
oh_get_num_collisions(cw_oh_t * a_oh)
{
  cw_uint64_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->num_collisions;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint64_t
oh_get_num_inserts(cw_oh_t * a_oh)
{
  cw_uint64_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->num_inserts;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint64_t
oh_get_num_deletes(cw_oh_t * a_oh)
{
  cw_uint64_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->num_deletes;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint64_t
oh_get_num_grows(cw_oh_t * a_oh)
{
  cw_uint64_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->num_grows;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint64_t
oh_get_num_shrinks(cw_oh_t * a_oh)
{
  cw_uint64_t retval;
  
  _cw_check_ptr(a_oh);
#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_rlock(&a_oh->rw_lock);
  }
#endif

  retval = a_oh->num_shrinks;

#ifdef _CW_REENTRANT
  if (a_oh->is_thread_safe)
  {
    rwl_runlock(&a_oh->rw_lock);
  }
#endif
  return retval;
}

cw_uint64_t
oh_h1_string(cw_oh_t * a_oh, const void * a_key)
{
  cw_uint64_t retval;
  char * str;

  _cw_check_ptr(a_key);

  for (str = (char *) a_key, retval = 0; *str != 0; str++)
  {
    retval = retval * 33 + *str;
  }

/*    retval = retval % (1 << a_oh->curr_power); */
  
  return retval;
}

cw_uint64_t
oh_h1_direct(cw_oh_t * a_oh, const void * a_key)
{
  cw_uint64_t retval;

  _cw_check_ptr(a_oh);

/*    retval = (a_key >> 4) % (1 << a_oh->curr_power); */
  retval = (cw_uint64_t) (cw_uint32_t) a_key;

  return retval;
}

cw_bool_t
oh_key_compare_string(const void * a_k1, const void * a_k2)
{
  return strcmp((char *) a_k1, (char *) a_k2) ? FALSE : TRUE;
}

cw_bool_t
oh_key_compare_direct(const void * a_k1, const void * a_k2)
{
  return (a_k1 == a_k2) ? TRUE : FALSE;
}

static void
oh_p_grow(cw_oh_t * a_oh)
{
  /* Should we grow? */
  if (list_count(&a_oh->items_list) >= a_oh->curr_grow_point)
  {
    a_oh->num_grows++;

    a_oh->size <<= 1;
  
    /* Allocate new table */
    a_oh->items
      = (cw_oh_item_t **) _cw_realloc(a_oh->items,
				      a_oh->size * sizeof(cw_oh_item_t *));
    _cw_check_ptr(a_oh->items);
    bzero(a_oh->items, (a_oh->size * sizeof(cw_oh_item_t *)));

    /* Re-calculate curr_* fields. */
    a_oh->curr_power += 1;
    a_oh->curr_h2 = (((a_oh->base_h2 + 1)
			<< (a_oh->curr_power
			    - a_oh->base_power))
		       - 1);
    a_oh->curr_shrink_point
      = a_oh->curr_shrink_point << 1;
    a_oh->curr_grow_point
      = a_oh->curr_grow_point << 1;

    /* Iterate through old table and insert items into new table. */
    {
      cw_oh_item_t * item;
      cw_uint64_t i;
    
      for (i = list_count(&a_oh->items_list); i > 0; i--)
      {
	item = (cw_oh_item_t *) list_hpop(&a_oh->items_list);
	oh_p_item_insert(a_oh, item);
      }
    }
  }
}

static void
oh_p_shrink(cw_oh_t * a_oh)
{
  cw_uint32_t j;
  cw_uint32_t num_halvings;

  /* Should we shrink? */
  if ((list_count(&a_oh->items_list) < a_oh->curr_shrink_point)
      && (a_oh->curr_power > a_oh->base_power))
  {

    for (j = a_oh->curr_power - a_oh->base_power;
	 (((a_oh->curr_grow_point >> j) < list_count(&a_oh->items_list))
	  && (j > 0));
	 j--);
    num_halvings = j;

    /* We're not shrinking below the base table size, are we? */
    _cw_assert((a_oh->curr_power - num_halvings) >= a_oh->base_power);
  
    a_oh->num_shrinks++;

    a_oh->size >>= num_halvings;
  
    /* Allocate new table */
    a_oh->items
      = (cw_oh_item_t **) _cw_realloc(a_oh->items,
				      a_oh->size * sizeof(cw_oh_item_t *));
    _cw_check_ptr(a_oh->items);
    bzero(a_oh->items, (a_oh->size * sizeof(cw_oh_item_t *)));
  
    /* Re-calculate curr_* fields. */
    a_oh->curr_power -= num_halvings;
    a_oh->curr_h2 = (((a_oh->base_h2 + 1)
			<< (a_oh->curr_power
			    - a_oh->base_power))
		       - 1);
    a_oh->curr_shrink_point
      = a_oh->curr_shrink_point >> num_halvings;
    a_oh->curr_grow_point
      = a_oh->curr_grow_point >> num_halvings;

    /* Iterate through old table and insert items into new table. */
    {
      cw_uint64_t i;
      cw_oh_item_t * item;
    
      for (i = list_count(&a_oh->items_list); i > 0; i--)
      {
	item = list_hpop(&a_oh->items_list);
	oh_p_item_insert(a_oh, item);
      }
    }

    /* Purge the spares in the items_list. */
    list_purge_spares(&a_oh->items_list);
  
    /* Shrink the spares list down to a reasonable size. */
    {
      cw_sint64_t i;
      cw_oh_item_t * item;

      for (i = list_count(&a_oh->spares_list);
	   i > a_oh->curr_grow_point;
	   i--)
      {
	item = (cw_oh_item_t *) list_hpop(&a_oh->spares_list);
	_cw_free(item);
      }
      list_purge_spares(&a_oh->spares_list);
    }
  }
}

static void
oh_p_item_insert(cw_oh_t * a_oh,
		 cw_oh_item_t * a_item)
{
  cw_uint64_t slot, i, j;
  cw_bool_t retval = TRUE;
  
  /* Primary hash to first possible location to insert. */
  slot = a_oh->curr_h1(a_oh, a_item->key) % a_oh->size;

  for (i = 0, j = slot;
       i < a_oh->size;
       i++, j = (j + a_oh->curr_h2) % a_oh->size)
  {
    if (a_oh->items[j] == NULL)
    {
      a_oh->num_inserts++;

      /* This slot is unused, so insert. */
      a_item->slot_num = j;
      a_item->jumps = i; /* For deletion shuffling. */
      a_oh->items[j] = a_item;

      /* Wow, this looks knarly.  What we're doing here is adding the item
       * to the items_list, then setting the list_item pointer inside the
       * item, so that we can rip the item out of the list when
       * deleting. */
      a_item->list_item = list_tpush(&a_oh->items_list, a_item);
      retval = FALSE;
      break;
    }
    else
    {
      a_oh->num_collisions++;
    }
  }
}

static cw_bool_t
oh_p_item_search(cw_oh_t * a_oh,
		 const void * a_key,
		 cw_uint64_t * a_slot)
{
  cw_uint64_t slot, i, j;
  cw_bool_t retval;
  
  /* Primary hash to the first location to look. */
  slot = a_oh->curr_h1(a_oh, a_key) % a_oh->size;

  /* Jump by the secondary hash value until we either find what we're
   * looking for, or hit an empty slot. */
  for (i = 0, j = slot;
       ;
       i++, j = (j + a_oh->curr_h2) % a_oh->size)
  {
    if (a_oh->items[j] == NULL)
    {
      /* Hit an empty slot.  What we're looking for isn't here. */
      retval = TRUE;
      break;
    }
    else if (a_oh->key_compare(a_oh->items[j]->key, a_key) == TRUE)
    {
      /* Found it. */
      *a_slot = j;
      retval = FALSE;
      break;
    }
  }
  
  return retval;
}

static void
oh_p_rehash(cw_oh_t * a_oh)
{
  cw_uint64_t i;
  cw_oh_item_t * item;

  /* Clear the table. */
  bzero(a_oh->items, a_oh->size * sizeof(cw_oh_item_t *));
  
  /* Iterate through old table and rehash them. */
  for (i = list_count(&a_oh->items_list); i > 0; i--)
  {
    item = (cw_oh_item_t *) list_hpop(&a_oh->items_list);
    oh_p_item_insert(a_oh, item);
  }
}

static void
oh_p_slot_shuffle(cw_oh_t * a_oh, cw_uint64_t a_slot)
{
  cw_uint64_t i, curr_empty, curr_look, curr_distance;
  
  for(i = 0,
	curr_distance = 1,
	curr_empty = a_slot,
	curr_look = ((curr_empty + a_oh->curr_h2) % a_oh->size);
      ((a_oh->items[curr_look] != NULL) && (i < a_oh->size));
      i++,
	curr_distance++,
	curr_look = (curr_look + a_oh->curr_h2) % a_oh->size)
  {
    /* See if this item had to jump at least the current distance to
     * the last empty slot in this secondary hash chain. */
    if (a_oh->items[curr_look]->jumps >= curr_distance)
    {
      /* This item should be shuffled back to the previous empty slot.
       * Do so, and reset curr_distance and curr_empty.  Also, update
       * the jumps field of the item we just shuffled, as well as its
       * record of the slot that it's now in. */
	  
      a_oh->items[curr_empty] = a_oh->items[curr_look];
      a_oh->items[curr_empty]->jumps -= curr_distance;
      a_oh->items[curr_empty]->slot_num = curr_empty;
      a_oh->items[curr_look] = NULL;

      curr_empty = curr_look;
      curr_distance = 0;
    }
  }
}
