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
 * $Revision: 36 $
 * $Date: 1998-04-19 21:27:40 -0700 (Sun, 19 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _OH_PERF_

#define _INC_THREAD_H_
#define _INC_OH_H_
#define _INC_OH_PRIV_H_

#define _INC_STRING_H_
#include <config.h>

oh_t *
oh_new()
{
  oh_t * retval;

  retval = (oh_t *) _cw_malloc(sizeof(oh_t));
  _cw_check_ptr(retval);

  rwl_new(&retval->rw_lock);

  retval->size = 1 << _OH_BASE_POWER;
  
  retval->items = (cw_oh_item_t **) _cw_malloc(retval->size
						* sizeof(cw_oh_item_t *));
  _cw_check_ptr(retval->items);
  bzero(retval->items, retval->size * sizeof(cw_oh_item_t *));

  retval->base_h1 = oh_h1_priv;
  retval->curr_h1 = oh_h1_priv;

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

  return retval;
}

void
oh_delete(oh_t * arg_oh_obj)
{
  cw_uint32_t i;

  _cw_check_ptr(arg_oh_obj);

  rwl_delete(&arg_oh_obj->rw_lock);

  /* Iteratively delete the items in the table. */
  for (i = 0; i < arg_oh_obj->size; i++)
  {
    if (arg_oh_obj->items[i] != NULL)
    {
      _cw_free(arg_oh_obj->items[i]);
    }
  }
  
  _cw_free(arg_oh_obj->items);
  _cw_free(arg_oh_obj);
}

cw_bool_t
oh_rehash(oh_t * arg_oh_obj)
{
  cw_bool_t retval;

  _cw_check_ptr(arg_oh_obj);

  rwl_wlock(&arg_oh_obj->rw_lock);

  retval = oh_rehash_priv(arg_oh_obj, TRUE);

  rwl_wunlock(&arg_oh_obj->rw_lock);
  
  return retval;
}

cw_uint32_t
oh_get_size(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->size;
}

cw_uint32_t
oh_get_num_invalid(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->num_invalid;
}

cw_uint32_t
oh_get_num_items(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->num_items;
}

oh_h1_t *
oh_get_h1(oh_t * arg_oh_obj)
{
  oh_h1_t * retval;
  
  _cw_check_ptr(arg_oh_obj);
  retval = arg_oh_obj->curr_h1;
  return retval;
}

cw_sint32_t
oh_get_base_h2(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->base_h2;
}

cw_sint32_t
oh_get_base_shrink_point(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->base_shrink_point;
}

cw_sint32_t
oh_get_base_grow_point(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->base_grow_point;
}

cw_sint32_t
oh_get_base_rehash_point(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->base_rehash_point;
}

cw_bool_t
oh_set_h1(oh_t * arg_oh_obj,
	  oh_h1_t * arg_new_h1)
{
  cw_bool_t retval;
  
  _cw_check_ptr(arg_oh_obj);
  _cw_check_ptr(arg_new_h1);

  rwl_wlock(&arg_oh_obj->rw_lock);

  if (arg_oh_obj->curr_h1 != arg_new_h1)
  {
    arg_oh_obj->curr_h1 = arg_new_h1;

    retval = oh_rehash_priv(arg_oh_obj, TRUE);
    if (retval == FALSE)
    {
      retval = oh_coalesce_priv(arg_oh_obj);
    }
  }
  else
  {
    retval = TRUE;
  }

  rwl_wunlock(&arg_oh_obj->rw_lock);

  return retval;
}

cw_bool_t
oh_set_base_h2(oh_t * arg_oh_obj,
	       cw_uint32_t arg_h2)
{
  cw_bool_t retval;
  
  _cw_check_ptr(arg_oh_obj);

  rwl_wlock(&arg_oh_obj->rw_lock);
  
  if (((arg_h2 % 2) != 0)
      || (arg_h2 > (1 << arg_oh_obj->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    arg_oh_obj->base_h2 = arg_h2;
    arg_oh_obj->curr_h2 = (((arg_oh_obj->base_h2 + 1)
			    << (arg_oh_obj->curr_power
				- arg_oh_obj->base_power))
			   - 1);
    retval = oh_rehash_priv(arg_oh_obj, TRUE);
  }

  rwl_wunlock(&arg_oh_obj->rw_lock);

  return retval;
}

cw_bool_t
oh_set_base_shrink_point(oh_t * arg_oh_obj,
			 cw_sint32_t arg_shrink_point)
{
  cw_bool_t retval;
  
  _cw_check_ptr(arg_oh_obj);

  rwl_wlock(&arg_oh_obj->rw_lock);

  if ((arg_shrink_point < 0)
      || (arg_shrink_point >= (1 << arg_oh_obj->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    arg_oh_obj->base_shrink_point = arg_shrink_point;
    arg_oh_obj->curr_shrink_point
      = (arg_oh_obj->base_shrink_point
	 << (arg_oh_obj->curr_power - arg_oh_obj->base_power));
    retval = oh_shrink_priv(arg_oh_obj, 0);
  }

  rwl_wunlock(&arg_oh_obj->rw_lock);

return retval;
}

cw_bool_t
oh_set_base_grow_point(oh_t * arg_oh_obj,
		       cw_sint32_t arg_grow_point)
{
  cw_bool_t retval;
  
  _cw_check_ptr(arg_oh_obj);

  rwl_wlock(&arg_oh_obj->rw_lock);

  if ((arg_grow_point <= 0)
      || (arg_grow_point >= (1 << arg_oh_obj->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    arg_oh_obj->base_grow_point = arg_grow_point;
    arg_oh_obj->curr_grow_point
      = (arg_oh_obj->base_grow_point
	 << (arg_oh_obj->curr_power - arg_oh_obj->base_power));
    retval = oh_grow_priv(arg_oh_obj, 0);
  }

  rwl_wunlock(&arg_oh_obj->rw_lock);

return retval;
}

cw_bool_t
oh_set_base_rehash_point(oh_t * arg_oh_obj,
			 cw_sint32_t arg_rehash_point)
{
  cw_bool_t retval;
  
  _cw_check_ptr(arg_oh_obj);

  rwl_wlock(&arg_oh_obj->rw_lock);

  if ((arg_rehash_point <= 0)
      || (arg_rehash_point >= (1 << arg_oh_obj->base_power)))
  {
    retval = TRUE;
  }
  else
  {
    arg_oh_obj->base_rehash_point = arg_rehash_point;
    arg_oh_obj->curr_rehash_point
      = (arg_oh_obj->base_rehash_point
	 << (arg_oh_obj->curr_power - arg_oh_obj->base_power));
    retval = oh_rehash_priv(arg_oh_obj, FALSE);
  }

  rwl_wunlock(&arg_oh_obj->rw_lock);

  return retval;
}

cw_bool_t
oh_item_insert(oh_t * arg_oh_obj, void * arg_key,
	       void * arg_data_addr)
{
  cw_oh_item_t * item;
  cw_bool_t retval;
  
  _cw_check_ptr(arg_oh_obj);

  rwl_wlock(&arg_oh_obj->rw_lock);

  retval = oh_coalesce_priv(arg_oh_obj);
  {
    if (retval == TRUE)
    {
      goto RETURN;
    }
  }
  
  item = (cw_oh_item_t *) _cw_malloc(sizeof(cw_oh_item_t));
  _cw_check_ptr(item);

  item->is_valid = TRUE;
  item->key = arg_key;
  item->data = arg_data_addr;

  retval = oh_item_insert_priv(arg_oh_obj, item);
  if (retval == TRUE)
  {
    oh_coalesce_priv(arg_oh_obj);
  }

 RETURN:
  rwl_wunlock(&arg_oh_obj->rw_lock);
  return retval;
}

cw_bool_t
oh_item_delete(oh_t * arg_oh_obj,
	       void * arg_key,
	       void ** arg_data)
{
  cw_uint32_t slot;
  cw_bool_t error, retval = FALSE;
  
  _cw_check_ptr(arg_oh_obj);

  rwl_wlock(&arg_oh_obj->rw_lock);

  error = oh_item_search_priv(arg_oh_obj, arg_key, &slot);

  if (error == FALSE)
  {
#ifdef _OH_PERF_
    arg_oh_obj->num_deletes++;
#endif
    arg_oh_obj->items[slot]->is_valid = FALSE;
    arg_oh_obj->num_invalid++;
    arg_oh_obj->num_items--;
    *arg_data = arg_oh_obj->items[slot]->data;
  }
  else
  {
    *arg_data = NULL;
    retval = TRUE;
  }

  rwl_wunlock(&arg_oh_obj->rw_lock);

  return retval;
}

cw_bool_t
oh_item_search(oh_t * arg_oh_obj,
	       void * arg_key,
	       void ** arg_data)
{
  cw_uint32_t slot;
  cw_bool_t error, retval = FALSE;
  
  _cw_check_ptr(arg_oh_obj);

  rwl_rlock(&arg_oh_obj->rw_lock);

  error = oh_item_search_priv(arg_oh_obj, arg_key, &slot);

  if (error == FALSE)
  {
    *arg_data = arg_oh_obj->items[slot]->data;
  }
  else
  {
    *arg_data = NULL;
    retval = TRUE;
  }

  rwl_runlock(&arg_oh_obj->rw_lock);

  return retval;
}

void
oh_dump(oh_t * arg_oh_obj, cw_bool_t arg_all)
{
  cw_uint32_t i;

  _cw_check_ptr(arg_oh_obj);
  
  rwl_rlock(&arg_oh_obj->rw_lock);

  log_printf(g_log_obj,
	     "============================================================\n");
  log_printf(g_log_obj,
	     "Size: [%d]  Slots filled: [%d]  Invalid slots: [%d]\n\n",
	     arg_oh_obj->size,
	     arg_oh_obj->num_items,
	     arg_oh_obj->num_invalid);
  log_printf(g_log_obj, "      pow h1         h2    shrink grow  rehash\n");
  log_printf(g_log_obj, "      --- ---------- ----- ------ ----- ------\n");
#ifdef _PEDANTIC
  log_printf(g_log_obj, "Base: %2d  %10p %5d %5d  %5d %5d\n",
	     arg_oh_obj->base_power,
	     arg_oh_obj->base_h1,
	     arg_oh_obj->base_h2,
	     arg_oh_obj->base_shrink_point,
	     arg_oh_obj->base_grow_point,
	     arg_oh_obj->base_rehash_point);
  log_printf(g_log_obj, "Curr: %2d  %10p %5d %5d  %5d %5d\n\n",
	     arg_oh_obj->curr_power,
	     arg_oh_obj->curr_h1,
	     arg_oh_obj->curr_h2,
	     arg_oh_obj->curr_shrink_point,
	     arg_oh_obj->curr_grow_point,
	     arg_oh_obj->curr_rehash_point);
#else
  log_printf(g_log_obj, "Base: %2d  %010p %5d %5d  %5d %5d\n",
	     arg_oh_obj->base_power,
	     arg_oh_obj->base_h1,
	     arg_oh_obj->base_h2,
	     arg_oh_obj->base_shrink_point,
	     arg_oh_obj->base_grow_point,
	     arg_oh_obj->base_rehash_point);
  log_printf(g_log_obj, "Curr: %2d  %010p %5d %5d  %5d %5d\n\n",
	     arg_oh_obj->curr_power,
	     arg_oh_obj->curr_h1,
	     arg_oh_obj->curr_h2,
	     arg_oh_obj->curr_shrink_point,
	     arg_oh_obj->curr_grow_arg,
	     point_oh_obj->curr_rehash_point);
#endif
  
#ifdef _OH_PERF_
  log_printf(g_log_obj, "Counters: collisions[%d] inserts[%d] deletes[%d]\n",
	     arg_oh_obj->num_collisions,
	     arg_oh_obj->num_inserts,
	     arg_oh_obj->num_deletes);
  log_printf(g_log_obj, "          grows[%d] shrinks[%d] rehashes[%d]\n\n",
	     arg_oh_obj->num_grows,
	     arg_oh_obj->num_shrinks,
	     arg_oh_obj->num_rehashes);
#endif

  if (arg_all)
  {
    log_printf(g_log_obj, "slot is_valid key        value\n");
    log_printf(g_log_obj, "---- -------- ---------- ----------\n");
  
    for (i = 0; i < arg_oh_obj->size; i++)
    {
      log_printf(g_log_obj, "%4d ", i);
      if (arg_oh_obj->items[i] != NULL)
      {
	if (arg_oh_obj->items[i]->is_valid == FALSE)
	{
	  log_printf(g_log_obj, "FALSE    ");
	}
	else
	{
	  log_printf(g_log_obj, "TRUE     ");
	}
#ifdef _PEDANTIC
	log_printf(g_log_obj, "0x%08x %10p\n",
		   arg_oh_obj->items[i]->key,
		   arg_oh_obj->items[i]->data);
#else
	log_printf(g_log_obj, "0x%08x %010p\n",
		   arg_oh_obj->items[i]->key,
		   arg_oh_obj->items[i]->data);
#endif
      }
      else
      {
	log_printf(g_log_obj, "\n");
      }
    }
  }
  log_printf(g_log_obj,
	     "============================================================\n");
  rwl_runlock(&arg_oh_obj->rw_lock);
}

/* cw_uint32_t */
/* oh_h1_priv(oh_t * arg_oh_obj, void * arg_key) */
/* { */
/*   cw_uint32_t retval; */

/*   _cw_check_ptr(arg_oh_obj); */

/*   retval = (arg_key >> 4) % (1 << arg_oh_obj->curr_power); */

/*   return retval; */
/* } */

cw_uint32_t
oh_h1_priv(oh_t * arg_oh_obj, void * arg_key)
{
  cw_uint32_t retval;
  char * str;

  for (str = (char *) arg_key, retval = 0; *str != 0; str++)
  {
    retval = retval * 33 + *str;
  }

  retval = retval % (1 << arg_oh_obj->curr_power);
  return retval;
}

cw_bool_t
oh_coalesce_priv(oh_t * arg_oh_obj)
{
  cw_bool_t retval;
  
  retval = oh_shrink_priv(arg_oh_obj, 0);
  if (retval == TRUE)
  {
    goto RETURN;
  }

  retval = oh_rehash_priv(arg_oh_obj, FALSE);
  if (retval == TRUE)
  {
    goto RETURN;
  }

  retval = oh_grow_priv(arg_oh_obj, 0);
  if (retval == TRUE)
  {
    goto RETURN;
  }

 RETURN:
  return retval;
}

cw_bool_t
oh_grow_priv(oh_t * arg_oh_obj,
	     cw_sint32_t arg_num_doublings)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t old_size, i;
  cw_bool_t retval;

  _cw_check_ptr(arg_oh_obj);
  _cw_assert
    (arg_num_doublings + arg_oh_obj->curr_power < sizeof(cw_uint32_t) * 8);
  _cw_assert(arg_num_doublings >= 0);

  if (arg_num_doublings == 0)
  {
    /* Should we grow? */
    if (arg_oh_obj->num_items >= arg_oh_obj->curr_grow_point)
    {
      _cw_assert(arg_oh_obj->curr_grow_point != 0);
      retval = oh_grow_priv(arg_oh_obj,
			    (arg_oh_obj->num_items
			     / arg_oh_obj->curr_grow_point));
    }
    else
    {
      retval = FALSE;
    }
    goto RETURN;
  }
  
#ifdef _OH_PERF_
  arg_oh_obj->num_grows++;
#endif

  old_items = arg_oh_obj->items;

  old_size = arg_oh_obj->size;
  arg_oh_obj->size <<= arg_num_doublings;
  
  /* Allocate new table */
  arg_oh_obj->items
    = (cw_oh_item_t **) _cw_malloc(arg_oh_obj->size
				    * sizeof(cw_oh_item_t *));
  _cw_check_ptr(arg_oh_obj->items);
  bzero(arg_oh_obj->items, (arg_oh_obj->size * sizeof(cw_oh_item_t *)));

  /* Re-calculate curr_* fields. */
  arg_oh_obj->curr_power += arg_num_doublings;
  arg_oh_obj->curr_h2 = (((arg_oh_obj->base_h2 + 1)
			  << (arg_oh_obj->curr_power
			      - arg_oh_obj->base_power))
			 - 1);
  arg_oh_obj->curr_shrink_point
    = arg_oh_obj->curr_shrink_point << arg_num_doublings;
  arg_oh_obj->curr_grow_point
    = arg_oh_obj->curr_grow_point << arg_num_doublings;
  arg_oh_obj->curr_rehash_point
    = arg_oh_obj->curr_rehash_point << arg_num_doublings;

  /* Reset other fields. */
  arg_oh_obj->num_items = 0;
  arg_oh_obj->num_invalid = 0;
  
  /* Iterate through old table and insert items into new table. */
  for (i = 0; i < old_size; i++)
  {
    if (old_items[i] != NULL)
    {
      if (old_items[i]->is_valid == TRUE)
      {
	retval = oh_item_insert_priv(arg_oh_obj, old_items[i]);
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
  return retval;
}

cw_bool_t
oh_shrink_priv(oh_t * arg_oh_obj,
	       cw_sint32_t arg_num_halvings)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t old_size, i;
  cw_bool_t retval;

  _cw_check_ptr(arg_oh_obj);
  _cw_assert(arg_num_halvings >= 0);

  if (arg_num_halvings == 0)
  {
    /* Should we shrink? */
    if ((arg_oh_obj->num_items < arg_oh_obj->curr_shrink_point)
	&& (arg_oh_obj->curr_power > arg_oh_obj->base_power))
    {
      if (arg_oh_obj->num_items == 0)
      {
	retval = oh_shrink_priv(arg_oh_obj,
				(arg_oh_obj->curr_power
				 - arg_oh_obj->base_power));
      }
      else
      {
	retval = oh_shrink_priv(arg_oh_obj,
				(arg_oh_obj->curr_shrink_point
				 / arg_oh_obj->num_items));
      }
    }
    else
    {
      retval = FALSE;
    }
    goto RETURN;
  }

#ifdef _OH_PERF_
  arg_oh_obj->num_shrinks++;
#endif

  if (arg_num_halvings - arg_oh_obj->curr_power
      < arg_oh_obj->base_power)
  {
    arg_num_halvings = arg_oh_obj->curr_power - arg_oh_obj->base_power;
  }

  old_items = arg_oh_obj->items;
  old_size = arg_oh_obj->size;
  arg_oh_obj->size >>= arg_num_halvings;
  
  /* Allocate new table */
  arg_oh_obj->items
    = (cw_oh_item_t **) _cw_malloc(arg_oh_obj->size
				    * sizeof(cw_oh_item_t *));
  _cw_check_ptr(arg_oh_obj->items);
  bzero(arg_oh_obj->items, (arg_oh_obj->size * sizeof(cw_oh_item_t *)));
  
  /* Re-calculate curr_* fields. */
  arg_oh_obj->curr_power -= arg_num_halvings;
  arg_oh_obj->curr_h2 = (((arg_oh_obj->base_h2 + 1)
			  << (arg_oh_obj->curr_power
			      - arg_oh_obj->base_power))
			 - 1);
  arg_oh_obj->curr_shrink_point
    = arg_oh_obj->curr_shrink_point >> arg_num_halvings;
  arg_oh_obj->curr_grow_point
    = arg_oh_obj->curr_grow_point >> arg_num_halvings;
  arg_oh_obj->curr_rehash_point
    = arg_oh_obj->curr_rehash_point >> arg_num_halvings;

  /* Reset other fields. */
  arg_oh_obj->num_items = 0;
  arg_oh_obj->num_invalid = 0;
  
  /* Iterate through old table and insert items into new table. */
  for (i = 0; i < old_size; i++)
  {
    if (old_items[i] != NULL)
    {
      if (old_items[i]->is_valid == TRUE)
      {
	retval = oh_item_insert_priv(arg_oh_obj, old_items[i]);
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
  return retval;
}

cw_bool_t
oh_item_insert_priv(oh_t * arg_oh_obj,
		    cw_oh_item_t * arg_item)
{
  cw_uint32_t slot, i, j, junk;
  cw_bool_t retval;
  
  _cw_check_ptr(arg_oh_obj);
  _cw_check_ptr(arg_item);

#ifdef _OH_PERF_
  arg_oh_obj->num_inserts++;
#endif

  slot = arg_oh_obj->curr_h1(arg_oh_obj, arg_item->key);

  for (i = 0, j = slot;
       i < arg_oh_obj->size;
       i++, j = (j + arg_oh_obj->curr_h2) % arg_oh_obj->size)
  {
    if (arg_oh_obj->items[j] == NULL)
    {
      arg_oh_obj->items[j] = arg_item;
      arg_oh_obj->num_items++;
      retval = FALSE;
      goto RETURN;
    }
    else if (arg_oh_obj->items[j]->is_valid == FALSE)
    {
      if (TRUE == oh_item_search_priv(arg_oh_obj, arg_item->key,
				      &junk))
      {
	/* No duplicate in the table.  Go ahead and use this slot. */
	free(arg_oh_obj->items[j]);
	arg_oh_obj->items[j] = arg_item;
	arg_oh_obj->num_items++;
	arg_oh_obj->num_invalid--;
	retval = FALSE;
	goto RETURN;
      }
      else
      {
	/* Detected attempt to insert a duplicate. */
	retval = TRUE;
	goto RETURN;
      }
    }
#ifdef _OH_PERF_
    else
    {
      arg_oh_obj->num_collisions++;
    }
#endif
  }
  retval = TRUE;
  
 RETURN:
  return retval;
}

cw_bool_t
oh_item_search_priv(oh_t * arg_oh_obj,
		    void * arg_key,
		    cw_uint32_t * arg_slot)
{
  cw_uint32_t slot, i;
  cw_bool_t retval;
  
  _cw_check_ptr(arg_oh_obj);

  slot = arg_oh_obj->curr_h1(arg_oh_obj, arg_key);

  for (i = slot;
       i < arg_oh_obj->size;
       i = (i + arg_oh_obj->curr_h2) % arg_oh_obj->size)
  {
    if (arg_oh_obj->items[i] == NULL)
    {
      break;
    }
    else if ((arg_oh_obj->items[i]->is_valid == TRUE) &&
	     (arg_oh_obj->items[i]->key == arg_key))
    {
      *arg_slot = i;
      retval = FALSE;
      goto RETURN;
    }
  }

  retval = TRUE;
  
 RETURN:
  return retval;
}

cw_bool_t
oh_rehash_priv(oh_t * arg_oh_obj, cw_bool_t arg_force)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t i;
  cw_bool_t retval = FALSE;

  _cw_check_ptr(arg_oh_obj);

  /* Should we rehash? */
  if ((arg_force == TRUE)
      || ((arg_oh_obj->num_invalid > 0)
	  && (arg_oh_obj->num_items <= arg_oh_obj->curr_grow_point)
	  && (((arg_oh_obj->num_items + arg_oh_obj->num_invalid)
	       >= arg_oh_obj->curr_rehash_point)
	      || (0)
	      )
	  )
      )
  {
#ifdef _OH_PERF_
    arg_oh_obj->num_rehashes++;
#endif
    old_items = arg_oh_obj->items;

    /* Allocate new table */
    arg_oh_obj->items
      = (cw_oh_item_t **) _cw_malloc(arg_oh_obj->size
				      * sizeof(cw_oh_item_t *));
    _cw_check_ptr(arg_oh_obj->items);
    bzero(arg_oh_obj->items,
	  (arg_oh_obj->size * sizeof(cw_oh_item_t *)));

    arg_oh_obj->num_items = 0;
    
    /* Iterate through old table and insert items into new table. */
    for (i = 0; i < arg_oh_obj->size; i++)
    {
      if (old_items[i] != NULL)
      {
	if (old_items[i]->is_valid == TRUE)
	{
	  retval = oh_item_insert_priv(arg_oh_obj, old_items[i]);
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

    arg_oh_obj->num_invalid = 0;

    _cw_free(old_items);
  }

 RETURN:
  return retval;
}

#ifdef _OH_PERF_
cw_uint32_t
oh_get_num_collisions(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->num_collisions;
}

cw_uint32_t oh_get_num_inserts(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->num_inserts;
}

cw_uint32_t oh_get_num_deletes(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->num_deletes;
}

cw_uint32_t oh_get_num_grows(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->num_grows;
}

cw_uint32_t oh_get_num_shrinks(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->num_shrinks;
}

cw_uint32_t oh_get_num_rehashes(oh_t * arg_oh_obj)
{
  _cw_check_ptr(arg_oh_obj);
  return arg_oh_obj->num_rehashes;
}

#endif
