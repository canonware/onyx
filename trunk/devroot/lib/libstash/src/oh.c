/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1996-1998
 * Jason Evans <jasone@canonware.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JASON EVANS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL JASON EVANS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 57 $
 * $Date: 1998-05-01 03:17:44 -0700 (Fri, 01 May 1998) $
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
 * XXX Perhaps we should convert items to be within the items array, rather
 * than storing pointers to them.  With the current implementation, this
 * would introduce a copying overhead, but since this will eventually be
 * converted to shuffle slots to alleviate invalid slots, the copying
 * overhead will go away.
 *
 ****************************************************************************/

#define _OH_PERF_

#define _INC_THREAD_H_
#define _INC_OH_H_
#define _INC_STRING_H_
#include <config.h>
#include <oh_priv.h>

cw_oh_t *
oh_new(cw_oh_t * a_oh_o)
{
  cw_oh_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
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

  rwl_new(&retval->rw_lock);

  retval->size = 1 << _OH_BASE_POWER;
  
  retval->items = (cw_oh_item_t **) _cw_malloc(retval->size
					       * sizeof(cw_oh_item_t *));
  bzero(retval->items, retval->size * sizeof(cw_oh_item_t *));

  retval->base_h1 = oh_h1_priv;
  retval->curr_h1 = oh_h1_priv;
  retval->key_compare = oh_key_compare_priv;

  retval->num_items
    = retval->num_invalid
    = 0;
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
  retval->curr_rehash_point
    = retval->base_rehash_point
    = _OH_BASE_REHASH_POINT;

#ifdef _OH_PERF_
  retval->num_collisions
    = retval->num_inserts
    = retval->num_deletes
    = retval->num_grows
    = retval->num_shrinks
    = retval->num_rehashes
    = 0;
#endif

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_new()");
  }
  return retval;
}

void
oh_delete(cw_oh_t * a_oh_o)
{
  cw_uint32_t i;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_delete()");
  }
  _cw_check_ptr(a_oh_o);

  rwl_delete(&a_oh_o->rw_lock);

  /* Iteratively delete the items in the table. */
  for (i = 0; i < a_oh_o->size; i++)
  {
    if (a_oh_o->items[i] != NULL)
    {
      _cw_free(a_oh_o->items[i]);
    }
  }
  
  _cw_free(a_oh_o->items);
  if (a_oh_o->is_malloced == TRUE)
  {
    _cw_free(a_oh_o);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_delete()");
  }
}

cw_bool_t
oh_rehash(cw_oh_t * a_oh_o)
{
  cw_bool_t retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_rehash()");
  }
  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);

  retval = oh_rehash_priv(a_oh_o, TRUE);

  rwl_wunlock(&a_oh_o->rw_lock);
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_rehash()");
  }
  return retval;
}

cw_uint32_t
oh_get_size(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_size()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_size()");
  }
  return a_oh_o->size;
}

cw_uint32_t
oh_get_num_invalid(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_invalid()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_invalid()");
  }
  return a_oh_o->num_invalid;
}

cw_uint32_t
oh_get_num_items(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_items()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_items()");
  }
  return a_oh_o->num_items;
}

oh_h1_t *
oh_get_h1(cw_oh_t * a_oh_o)
{
  oh_h1_t * retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_h1()");
  }
  _cw_check_ptr(a_oh_o);

  retval = a_oh_o->curr_h1;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_h1()");
  }
  return retval;
}

oh_key_comp_t *
oh_get_key_compare(cw_oh_t * a_oh_o)
{
  oh_key_comp_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_key_compare()");
  }
  _cw_check_ptr(a_oh_o);

  retval = a_oh_o->key_compare;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_key_compare()");
  }
  return retval;
}

cw_sint32_t
oh_get_base_h2(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_base_h2()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_base_h2()");
  }
  return a_oh_o->base_h2;
}

cw_sint32_t
oh_get_base_shrink_point(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_base_shrink_point()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_base_shrink_point()");
  }
  return a_oh_o->base_shrink_point;
}

cw_sint32_t
oh_get_base_grow_point(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_base_grow_point()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_base_grow_point()");
  }
  return a_oh_o->base_grow_point;
}

cw_sint32_t
oh_get_base_rehash_point(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_base_rehash_point()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_base_rehash_point()");
  }
  return a_oh_o->base_rehash_point;
}

cw_bool_t
oh_set_h1(cw_oh_t * a_oh_o,
	  oh_h1_t * a_new_h1)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_h1()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_new_h1);

  rwl_wlock(&a_oh_o->rw_lock);

  if (a_oh_o->curr_h1 != a_new_h1)
  {
    a_oh_o->curr_h1 = a_new_h1;

    retval = oh_rehash_priv(a_oh_o, TRUE);
    if (retval == FALSE)
    {
      retval = oh_coalesce_priv(a_oh_o);
    }
  }
  else
  {
    retval = TRUE;
  }

  rwl_wunlock(&a_oh_o->rw_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_h1()");
  }
  return retval;
}

void
oh_set_key_compare(cw_oh_t * a_oh_o,
		   oh_key_comp_t * a_new_key_compare)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_key_compare()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_new_key_compare);
  rwl_wlock(&a_oh_o->rw_lock);

  a_oh_o->key_compare = a_new_key_compare;
  
  rwl_wunlock(&a_oh_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_key_compare()");
  }
}

cw_bool_t
oh_set_base_h2(cw_oh_t * a_oh_o,
	       cw_uint32_t a_h2)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_base_h2()");
  }
  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);
  
  if (((a_h2 % 2) != 0)
      || (a_h2 > (1 << a_oh_o->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    a_oh_o->base_h2 = a_h2;
    a_oh_o->curr_h2 = (((a_oh_o->base_h2 + 1)
			<< (a_oh_o->curr_power
			    - a_oh_o->base_power))
		       - 1);
    retval = oh_rehash_priv(a_oh_o, TRUE);
  }

  rwl_wunlock(&a_oh_o->rw_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_base_h2()");
  }
  return retval;
}

cw_bool_t
oh_set_base_shrink_point(cw_oh_t * a_oh_o,
			 cw_sint32_t a_shrink_point)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_base_shrink_point()");
  }
  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);

  if ((a_shrink_point < 0)
      || (a_shrink_point >= (1 << a_oh_o->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    a_oh_o->base_shrink_point = a_shrink_point;
    a_oh_o->curr_shrink_point
      = (a_oh_o->base_shrink_point
	 << (a_oh_o->curr_power - a_oh_o->base_power));
    retval = oh_shrink_priv(a_oh_o, 0);
  }

  rwl_wunlock(&a_oh_o->rw_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_base_shrink_point()");
  }
  return retval;
}

cw_bool_t
oh_set_base_grow_point(cw_oh_t * a_oh_o,
		       cw_sint32_t a_grow_point)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_base_grow_point()");
  }
  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);

  if ((a_grow_point <= 0)
      || (a_grow_point >= (1 << a_oh_o->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    a_oh_o->base_grow_point = a_grow_point;
    a_oh_o->curr_grow_point
      = (a_oh_o->base_grow_point
	 << (a_oh_o->curr_power - a_oh_o->base_power));
    retval = oh_grow_priv(a_oh_o, 0);
  }

  rwl_wunlock(&a_oh_o->rw_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_base_grow_point()");
  }
  return retval;
}

cw_bool_t
oh_set_base_rehash_point(cw_oh_t * a_oh_o,
			 cw_sint32_t a_rehash_point)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_set_base_rehash_point()");
  }
  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);

  if ((a_rehash_point <= 0)
      || (a_rehash_point >= (1 << a_oh_o->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    a_oh_o->base_rehash_point = a_rehash_point;
    a_oh_o->curr_rehash_point
      = (a_oh_o->base_rehash_point
	 << (a_oh_o->curr_power - a_oh_o->base_power));
    retval = oh_rehash_priv(a_oh_o, FALSE);
  }

  rwl_wunlock(&a_oh_o->rw_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_set_base_rehash_point()");
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

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_insert()");
  }
  _cw_check_ptr(a_oh_o);
  rwl_wlock(&a_oh_o->rw_lock);

  retval = oh_coalesce_priv(a_oh_o);
  if (retval == TRUE)
  {
    goto RETURN;
  }
  
  item = (cw_oh_item_t *) _cw_malloc(sizeof(cw_oh_item_t));
  _cw_check_ptr(item);

  item->is_valid = TRUE;
  item->key = a_key;
  item->data = a_data;

  retval = oh_item_insert_priv(a_oh_o, item);
  if (retval == TRUE)
  {
    oh_coalesce_priv(a_oh_o);
  }

 RETURN:
  rwl_wunlock(&a_oh_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
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
  cw_uint32_t slot;
  cw_bool_t error, retval = FALSE;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_delete()");
  }
  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);

  error = oh_item_search_priv(a_oh_o, a_search_key, &slot);

  if (error == FALSE)
  {
#ifdef _OH_PERF_
    a_oh_o->num_deletes++;
#endif
    a_oh_o->items[slot]->is_valid = FALSE;
    a_oh_o->num_invalid++;
    a_oh_o->num_items--;
    *a_key = a_oh_o->items[slot]->key;
    *a_data = a_oh_o->items[slot]->data;
    if (dbg_fmatch(g_dbg_o, _CW_DBG_R_OH_SLOT))
    {
      log_printf(g_log_o, "oh_item_delete(): Deleting in slot %d\n", slot);
    }
  }
  else
  {
    *a_key = NULL;
    *a_data = NULL;
    retval = TRUE;
  }

  rwl_wunlock(&a_oh_o->rw_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
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
  cw_uint32_t slot;
  cw_bool_t error, retval = FALSE;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_search()");
  }
  _cw_check_ptr(a_oh_o);

  rwl_rlock(&a_oh_o->rw_lock);

  error = oh_item_search_priv(a_oh_o, a_key, &slot);
  if (error == FALSE)
  {
    *a_data = a_oh_o->items[slot]->data;
    if (dbg_fmatch(g_dbg_o, _CW_DBG_R_OH_SLOT))
    {
      log_printf(g_log_o, "oh_item_search(): Found in slot %d\n", slot);
    }
  }
  else
  {
    _cw_marker("Wish we hadn't gotten here.");
    *a_data = NULL;
    retval = TRUE;
  }

  rwl_runlock(&a_oh_o->rw_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
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
  cw_uint32_t i;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_delete_iterate()");
  }
  _cw_check_ptr(a_oh_o);
  rwl_wlock(&a_oh_o->rw_lock);

  if (a_oh_o->num_items > 0)
  {
    for (i = 0; *a_key == NULL; i++)
    {
      if (a_oh_o->items[i]->is_valid == TRUE)
      {
	*a_key = a_oh_o->items[i]->key;
	*a_data = a_oh_o->items[i]->data;
	a_oh_o->items[i]->is_valid = FALSE;
      }
    }
    retval = FALSE;
  }
  else
  {
    retval = TRUE;
  }
  
  rwl_wunlock(&a_oh_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
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
  cw_uint32_t i;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_dump()");
  }
  _cw_check_ptr(a_oh_o);
  
  rwl_rlock(&a_oh_o->rw_lock);

  log_printf(g_log_o,
	     "============================================================\n");
  log_printf(g_log_o,
	     "Size: [%d]  Slots filled: [%d]  Invalid slots: [%d]\n\n",
	     a_oh_o->size,
	     a_oh_o->num_items,
	     a_oh_o->num_invalid);
  log_printf(g_log_o, "      pow h1         h2    shrink grow  rehash\n");
  log_printf(g_log_o, "      --- ---------- ----- ------ ----- ------\n");
#ifdef _PEDANTIC
  log_printf(g_log_o, "Base: %2d  %10p %5d %5d  %5d %5d\n",
	     a_oh_o->base_power,
	     a_oh_o->base_h1,
	     a_oh_o->base_h2,
	     a_oh_o->base_shrink_point,
	     a_oh_o->base_grow_point,
	     a_oh_o->base_rehash_point);
  log_printf(g_log_o, "Curr: %2d  %10p %5d %5d  %5d %5d\n\n",
	     a_oh_o->curr_power,
	     a_oh_o->curr_h1,
	     a_oh_o->curr_h2,
	     a_oh_o->curr_shrink_point,
	     a_oh_o->curr_grow_point,
	     a_oh_o->curr_rehash_point);
#else
  log_printf(g_log_o, "Base: %2d  %010p %5d %5d  %5d %5d\n",
	     a_oh_o->base_power,
	     a_oh_o->base_h1,
	     a_oh_o->base_h2,
	     a_oh_o->base_shrink_point,
	     a_oh_o->base_grow_point,
	     a_oh_o->base_rehash_point);
  log_printf(g_log_o, "Curr: %2d  %010p %5d %5d  %5d %5d\n\n",
	     a_oh_o->curr_power,
	     a_oh_o->curr_h1,
	     a_oh_o->curr_h2,
	     a_oh_o->curr_shrink_point,
	     a_oh_o->curr_grow_arg,
	     point_oh_o->curr_rehash_point);
#endif
  
#ifdef _OH_PERF_
  log_printf(g_log_o, "Counters: collisions[%d] inserts[%d] deletes[%d]\n",
	     a_oh_o->num_collisions,
	     a_oh_o->num_inserts,
	     a_oh_o->num_deletes);
  log_printf(g_log_o, "          grows[%d] shrinks[%d] rehashes[%d]\n\n",
	     a_oh_o->num_grows,
	     a_oh_o->num_shrinks,
	     a_oh_o->num_rehashes);
#endif

  if (a_all)
  {
    log_printf(g_log_o, "slot is_valid key        value\n");
    log_printf(g_log_o, "---- -------- ---------- ----------\n");
  
    for (i = 0; i < a_oh_o->size; i++)
    {
      log_printf(g_log_o, "%4d ", i);
      if (a_oh_o->items[i] != NULL)
      {
	if (a_oh_o->items[i]->is_valid == FALSE)
	{
	  log_printf(g_log_o, "FALSE    ");
	}
	else
	{
	  log_printf(g_log_o, "TRUE     ");
	}
#ifdef _PEDANTIC
	log_printf(g_log_o, "0x%08x %10p\n",
		   a_oh_o->items[i]->key,
		   a_oh_o->items[i]->data);
#else
	log_printf(g_log_o, "0x%08x %010p\n",
		   a_oh_o->items[i]->key,
		   a_oh_o->items[i]->data);
#endif
      }
      else
      {
	log_printf(g_log_o, "\n");
      }
    }
  }
  log_printf(g_log_o,
	     "============================================================\n");
  rwl_runlock(&a_oh_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
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
cw_uint32_t
oh_h1_priv(cw_oh_t * a_oh_o, void * a_key)
{
  cw_uint32_t retval;
  char * str;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_h1_priv()");
  }
  
  for (str = (char *) a_key, retval = 0; *str != 0; str++)
  {
    retval = retval * 33 + *str;
  }

  retval = retval % (1 << a_oh_o->curr_power);
  
  if (dbg_fmatch(g_dbg_o, _CW_DBG_R_OH_SLOT))
  {
    log_printf(g_log_o, "oh_h1_priv(): :%s: --> %d\n", a_key, retval);
    if (a_oh_o->items[retval] != NULL)
    {
      log_printf(g_log_o, "oh_h1_priv(): items[%d]->is_valid == %d\n",
		 retval, a_oh_o->items[retval]->is_valid);
      log_printf(g_log_o, "oh_h1_priv(): items[%d]->key == :%s:\n",
		 retval, a_oh_o->items[retval]->key);
    }
    else
    {
      log_printf(g_log_o, "oh_h1_priv(): Slot %d is empty.\n", retval);
    }
  }
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_h1_priv()");
  }
  return retval;
}
#if (0)
cw_uint32_t
oh_h1_priv(cw_oh_t * a_oh_o, void * a_key)
{
  cw_uint32_t retval;

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
oh_key_compare_priv(void * a_k1, void * a_k2)
{
  cw_bool_t retval;

  _cw_check_ptr(a_k1);
  _cw_check_ptr(a_k2);

  if (strcmp((char *) a_k1, (char *) a_k2))
  {
    retval = FALSE;
  }
  else
  {
    retval = TRUE;
  }

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Make sure that the hash table is in an acceptable state.  If not, fix it.
 *
 ****************************************************************************/
cw_bool_t
oh_coalesce_priv(cw_oh_t * a_oh_o)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_coalesce_priv()");
  }
  retval = oh_shrink_priv(a_oh_o, 0);
  if (retval == TRUE)
  {
    goto RETURN;
  }

  retval = oh_rehash_priv(a_oh_o, FALSE);
  if (retval == TRUE)
  {
    goto RETURN;
  }

  retval = oh_grow_priv(a_oh_o, 0);
  if (retval == TRUE)
  {
    goto RETURN;
  }

 RETURN:
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_coalesce_priv()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * If the table is too full, double in size and insert into the new table.
 *
 ****************************************************************************/
cw_bool_t
oh_grow_priv(cw_oh_t * a_oh_o,
	     cw_sint32_t a_num_doublings)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t old_size, i;
  cw_bool_t retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_grow_priv()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_assert
    (a_num_doublings + a_oh_o->curr_power < sizeof(cw_uint32_t) * 8);
  _cw_assert(a_num_doublings >= 0);

  if (a_num_doublings == 0)
  {
    /* Should we grow? */
    if (a_oh_o->num_items >= a_oh_o->curr_grow_point)
    {
      _cw_assert(a_oh_o->curr_grow_point != 0);
      retval = oh_grow_priv(a_oh_o,
			    (a_oh_o->num_items
			     / a_oh_o->curr_grow_point));
    }
    else
    {
      retval = FALSE;
    }
    goto RETURN;
  }
  
#ifdef _OH_PERF_
  a_oh_o->num_grows++;
#endif

  old_items = a_oh_o->items;

  old_size = a_oh_o->size;
  a_oh_o->size <<= a_num_doublings;
  
  /* Allocate new table */
  a_oh_o->items
    = (cw_oh_item_t **) _cw_malloc(a_oh_o->size
				   * sizeof(cw_oh_item_t *));
  _cw_check_ptr(a_oh_o->items);
  bzero(a_oh_o->items, (a_oh_o->size * sizeof(cw_oh_item_t *)));

  /* Re-calculate curr_* fields. */
  a_oh_o->curr_power += a_num_doublings;
  a_oh_o->curr_h2 = (((a_oh_o->base_h2 + 1)
		      << (a_oh_o->curr_power
			  - a_oh_o->base_power))
		     - 1);
  a_oh_o->curr_shrink_point
    = a_oh_o->curr_shrink_point << a_num_doublings;
  a_oh_o->curr_grow_point
    = a_oh_o->curr_grow_point << a_num_doublings;
  a_oh_o->curr_rehash_point
    = a_oh_o->curr_rehash_point << a_num_doublings;

  /* Reset other fields. */
  a_oh_o->num_items = 0;
  a_oh_o->num_invalid = 0;
  
  /* Iterate through old table and insert items into new table. */
  for (i = 0; i < old_size; i++)
  {
    if (old_items[i] != NULL)
    {
      if (old_items[i]->is_valid == TRUE)
      {
	retval = oh_item_insert_priv(a_oh_o, old_items[i]);
	if (retval == TRUE)
	{
	  goto RETURN;
	}
      }
      else
      {
	_cw_free(old_items[i]);
      }
    }
  }
  _cw_free(old_items);
  
  retval = FALSE;
 RETURN:
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_grow_priv()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * If the table is too empty, shrink it as small as possible, without
 * making it so small that the table would need to immediately grow again.
 *
 ****************************************************************************/
cw_bool_t
oh_shrink_priv(cw_oh_t * a_oh_o,
	       cw_sint32_t a_num_halvings)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t old_size, i;
  cw_bool_t retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_shrink_priv()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_assert(a_num_halvings >= 0);

  if (a_num_halvings == 0)
  {
    /* Should we shrink? */
    if ((a_oh_o->num_items < a_oh_o->curr_shrink_point)
	&& (a_oh_o->curr_power > a_oh_o->base_power))
    {
      if (a_oh_o->num_items == 0)
      {
	retval = oh_shrink_priv(a_oh_o,
				(a_oh_o->curr_power
				 - a_oh_o->base_power));
      }
      else
      {
	retval = oh_shrink_priv(a_oh_o,
				(a_oh_o->curr_shrink_point
				 / a_oh_o->num_items));
      }
    }
    else
    {
      retval = FALSE;
    }
    goto RETURN;
  }

#ifdef _OH_PERF_
  a_oh_o->num_shrinks++;
#endif

  if (a_num_halvings - a_oh_o->curr_power
      < a_oh_o->base_power)
  {
    a_num_halvings = a_oh_o->curr_power - a_oh_o->base_power;
  }

  old_items = a_oh_o->items;
  old_size = a_oh_o->size;
  a_oh_o->size >>= a_num_halvings;
  
  /* Allocate new table */
  a_oh_o->items
    = (cw_oh_item_t **) _cw_malloc(a_oh_o->size
				   * sizeof(cw_oh_item_t *));
  _cw_check_ptr(a_oh_o->items);
  bzero(a_oh_o->items, (a_oh_o->size * sizeof(cw_oh_item_t *)));
  
  /* Re-calculate curr_* fields. */
  a_oh_o->curr_power -= a_num_halvings;
  a_oh_o->curr_h2 = (((a_oh_o->base_h2 + 1)
		      << (a_oh_o->curr_power
			  - a_oh_o->base_power))
		     - 1);
  a_oh_o->curr_shrink_point
    = a_oh_o->curr_shrink_point >> a_num_halvings;
  a_oh_o->curr_grow_point
    = a_oh_o->curr_grow_point >> a_num_halvings;
  a_oh_o->curr_rehash_point
    = a_oh_o->curr_rehash_point >> a_num_halvings;

  /* Reset other fields. */
  a_oh_o->num_items = 0;
  a_oh_o->num_invalid = 0;
  
  /* Iterate through old table and insert items into new table. */
  for (i = 0; i < old_size; i++)
  {
    if (old_items[i] != NULL)
    {
      if (old_items[i]->is_valid == TRUE)
      {
	retval = oh_item_insert_priv(a_oh_o, old_items[i]);
	if (retval == TRUE)
	{
	  goto RETURN;
	}
      }
      else
      {
	_cw_free(old_items[i]);
      }
    }
  }
  _cw_free(old_items);
  
  retval = FALSE;
 RETURN:
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_shrink_priv()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Find the slot that we should insert into, given a_item, and insert.
 *
 ****************************************************************************/
cw_bool_t
oh_item_insert_priv(cw_oh_t * a_oh_o,
		    cw_oh_item_t * a_item)
{
  cw_uint32_t slot, i, j, junk;
  cw_bool_t retval = TRUE;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_insert_priv()");
  }
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_item);

#ifdef _OH_PERF_
  a_oh_o->num_inserts++;
#endif

  /* Primary hash to first possible location to insert. */
  slot = a_oh_o->curr_h1(a_oh_o, a_item->key);

  for (i = 0, j = slot;
       i < a_oh_o->size;
       i++, j = (j + a_oh_o->curr_h2) % a_oh_o->size)
  {
    if (a_oh_o->items[j] == NULL)
    {
      /* This slot is unused, so insert. */
      a_oh_o->items[j] = a_item;
      a_oh_o->num_items++;
      retval = FALSE;
      if (dbg_fmatch(g_dbg_o, _CW_DBG_R_OH_SLOT))
      {
	log_printf(g_log_o, "oh_item_insert_priv(): Inserting in slot %d\n",
		   slot);
      }
      break;
    }
    else if (a_oh_o->items[j]->is_valid == FALSE)
    {
      /* Slot is invalid.  Before inserting, make sure that an entry with a
       * duplicate key doesn't already exist in the table. */
      if (TRUE == oh_item_search_priv(a_oh_o, a_item->key,
				      &junk))
      {
	/* No duplicate in the table.  Go ahead and use this slot. */
	free(a_oh_o->items[j]);
	a_oh_o->items[j] = a_item;
	a_oh_o->num_items++;
	a_oh_o->num_invalid--;
	retval = FALSE;
	if (dbg_fmatch(g_dbg_o, _CW_DBG_R_OH_SLOT))
	{
	  log_printf(g_log_o, "oh_item_insert_priv(): Inserting in slot %d\n",
		     slot);
	}
	break;
      }
      else
      {
	/* Detected attempt to insert a duplicate. */
/* 	retval = TRUE; */
	break;
      }
    }
#ifdef _OH_PERF_
    else
    {
      a_oh_o->num_collisions++;
    }
#endif
  }
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_item_insert_priv()");
  }
  _cw_assert(retval != TRUE);
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Uses the primary and secondary hash function to search for an item with
 * key == a_key.
 *
 ****************************************************************************/
cw_bool_t
oh_item_search_priv(cw_oh_t * a_oh_o,
		    void * a_key,
		    cw_uint32_t * a_slot)
{
  cw_uint32_t slot, i, j;
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_item_search_priv()");
  }
  _cw_check_ptr(a_oh_o);

  /* Primary hash to the first location to look. */
  slot = a_oh_o->curr_h1(a_oh_o, a_key);

  /* Jump by the secondary hash value until we either find what we're
   * looking for, or hit an empty slot. */
  for (i = 0, j = slot;
       i < a_oh_o->size;
       i++, j = (j + a_oh_o->curr_h2) % a_oh_o->size)
  {
    if (a_oh_o->items[j] == NULL)
    {
      /* Hit an empty slot.  What we're looking for isn't here. */
      retval = TRUE;
      break;
    }
    else if ((a_oh_o->items[j]->is_valid == TRUE) &&
	     (a_oh_o->key_compare(a_oh_o->items[j]->key, a_key) == TRUE))
    {
      /* Found it. */
      *a_slot = j;
      retval = FALSE;
      break;
    }
  }
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_item_search_priv()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * If necessary, rehash.  The decision is made based on the fullness and
 * dirtiness of the table, in conjunction with curr_rehash_point.
 *
 ****************************************************************************/
cw_bool_t
oh_rehash_priv(cw_oh_t * a_oh_o, cw_bool_t a_force)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t i;
  cw_bool_t retval = FALSE;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_rehash_priv()");
  }
  _cw_check_ptr(a_oh_o);

  /* Should we rehash? */
  if ((a_force == TRUE)
      || ((a_oh_o->num_invalid > 0)
	  && (a_oh_o->num_items <= a_oh_o->curr_grow_point)
	  && (((a_oh_o->num_items + a_oh_o->num_invalid)
	       >= a_oh_o->curr_rehash_point)
	      || (0)
	      )
	  )
      )
  {
#ifdef _OH_PERF_
    a_oh_o->num_rehashes++;
#endif
    old_items = a_oh_o->items;

    /* Allocate new table */
    a_oh_o->items
      = (cw_oh_item_t **) _cw_malloc(a_oh_o->size
				     * sizeof(cw_oh_item_t *));
    _cw_check_ptr(a_oh_o->items);
    bzero(a_oh_o->items,
	  (a_oh_o->size * sizeof(cw_oh_item_t *)));

    a_oh_o->num_items = 0;
    
    /* Iterate through old table and insert items into new table. */
    for (i = 0; i < a_oh_o->size; i++)
    {
      if (old_items[i] != NULL)
      {
	if (old_items[i]->is_valid == TRUE)
	{
	  retval = oh_item_insert_priv(a_oh_o, old_items[i]);
	  if (retval == TRUE)
	  {
	    /* XXX If this happens, we're in deep trouble, because it means
	     * we had a corrupted hash table before the rehash.  Maybe we
	     * should crash instead. */
	    break;
	  }
	}
	else
	{
	  _cw_free(old_items[i]);
	}
      }
    }

    a_oh_o->num_invalid = 0;

    _cw_free(old_items);
  }

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_rehash_priv()");
  }
  return retval;
}

#ifdef _OH_PERF_
cw_uint32_t
oh_get_num_collisions(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_collisions()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_collisions()");
  }
  return a_oh_o->num_collisions;
}

cw_uint32_t
oh_get_num_inserts(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_inserts()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_inserts()");
  }
  return a_oh_o->num_inserts;
}

cw_uint32_t
oh_get_num_deletes(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_deletes()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_deletes()");
  }
  return a_oh_o->num_deletes;
}

cw_uint32_t
oh_get_num_grows(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_grows()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_grows()");
  }
  return a_oh_o->num_grows;
}

cw_uint32_t
oh_get_num_shrinks(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_shrinks()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_shrinks()");
  }
  return a_oh_o->num_shrinks;
}

cw_uint32_t
oh_get_num_rehashes(cw_oh_t * a_oh_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Enter oh_get_num_rehashes()");
  }
  _cw_check_ptr(a_oh_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_OH_FUNC))
  {
    _cw_marker("Exit oh_get_num_rehashes()");
  }
  return a_oh_o->num_rehashes;
}

#endif
