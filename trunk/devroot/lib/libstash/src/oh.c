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
 * $Revision: 41 $
 * $Date: 1998-04-26 20:06:13 -0700 (Sun, 26 Apr 1998) $
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

cw_oh_t *
oh_new()
{
  cw_oh_t * retval;

  retval = (cw_oh_t *) _cw_malloc(sizeof(cw_oh_t));
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
oh_delete(cw_oh_t * a_oh_o)
{
  cw_uint32_t i;

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
  _cw_free(a_oh_o);
}

cw_bool_t
oh_rehash(cw_oh_t * a_oh_o)
{
  cw_bool_t retval;

  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);

  retval = oh_rehash_priv(a_oh_o, TRUE);

  rwl_wunlock(&a_oh_o->rw_lock);
  
  return retval;
}

cw_uint32_t
oh_get_size(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->size;
}

cw_uint32_t
oh_get_num_invalid(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->num_invalid;
}

cw_uint32_t
oh_get_num_items(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->num_items;
}

oh_h1_t *
oh_get_h1(cw_oh_t * a_oh_o)
{
  oh_h1_t * retval;
  
  _cw_check_ptr(a_oh_o);
  retval = a_oh_o->curr_h1;
  return retval;
}

cw_sint32_t
oh_get_base_h2(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->base_h2;
}

cw_sint32_t
oh_get_base_shrink_point(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->base_shrink_point;
}

cw_sint32_t
oh_get_base_grow_point(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->base_grow_point;
}

cw_sint32_t
oh_get_base_rehash_point(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->base_rehash_point;
}

cw_bool_t
oh_set_h1(cw_oh_t * a_oh_o,
	  oh_h1_t * a_new_h1)
{
  cw_bool_t retval;
  
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

  return retval;
}

cw_bool_t
oh_set_base_h2(cw_oh_t * a_oh_o,
	       cw_uint32_t a_h2)
{
  cw_bool_t retval;
  
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

  return retval;
}

cw_bool_t
oh_set_base_shrink_point(cw_oh_t * a_oh_o,
			 cw_sint32_t a_shrink_point)
{
  cw_bool_t retval;
  
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

  return retval;
}

cw_bool_t
oh_set_base_grow_point(cw_oh_t * a_oh_o,
		       cw_sint32_t a_grow_point)
{
  cw_bool_t retval;
  
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

  return retval;
}

cw_bool_t
oh_set_base_rehash_point(cw_oh_t * a_oh_o,
			 cw_sint32_t a_rehash_point)
{
  cw_bool_t retval;
  
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

  return retval;
}

cw_bool_t
oh_item_insert(cw_oh_t * a_oh_o, void * a_key,
	       void * a_data_addr)
{
  cw_oh_item_t * item;
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);

  retval = oh_coalesce_priv(a_oh_o);
  {
    if (retval == TRUE)
    {
      goto RETURN;
    }
  }
  
  item = (cw_oh_item_t *) _cw_malloc(sizeof(cw_oh_item_t));
  _cw_check_ptr(item);

  item->is_valid = TRUE;
  item->key = a_key;
  item->data = a_data_addr;

  retval = oh_item_insert_priv(a_oh_o, item);
  if (retval == TRUE)
  {
    oh_coalesce_priv(a_oh_o);
  }

 RETURN:
  rwl_wunlock(&a_oh_o->rw_lock);
  return retval;
}

cw_bool_t
oh_item_delete(cw_oh_t * a_oh_o,
	       void * a_key,
	       void ** a_data)
{
  cw_uint32_t slot;
  cw_bool_t error, retval = FALSE;
  
  _cw_check_ptr(a_oh_o);

  rwl_wlock(&a_oh_o->rw_lock);

  error = oh_item_search_priv(a_oh_o, a_key, &slot);

  if (error == FALSE)
  {
#ifdef _OH_PERF_
    a_oh_o->num_deletes++;
#endif
    a_oh_o->items[slot]->is_valid = FALSE;
    a_oh_o->num_invalid++;
    a_oh_o->num_items--;
    *a_data = a_oh_o->items[slot]->data;
  }
  else
  {
    *a_data = NULL;
    retval = TRUE;
  }

  rwl_wunlock(&a_oh_o->rw_lock);

  return retval;
}

cw_bool_t
oh_item_search(cw_oh_t * a_oh_o,
	       void * a_key,
	       void ** a_data)
{
  cw_uint32_t slot;
  cw_bool_t error, retval = FALSE;
  
  _cw_check_ptr(a_oh_o);

  rwl_rlock(&a_oh_o->rw_lock);

  error = oh_item_search_priv(a_oh_o, a_key, &slot);

  if (error == FALSE)
  {
    *a_data = a_oh_o->items[slot]->data;
  }
  else
  {
    *a_data = NULL;
    retval = TRUE;
  }

  rwl_runlock(&a_oh_o->rw_lock);

  return retval;
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == error
 *
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
  return retval;
}

void
oh_dump(cw_oh_t * a_oh_o, cw_bool_t a_all)
{
  cw_uint32_t i;

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
}

/* cw_uint32_t */
/* oh_h1_priv(cw_oh_t * a_oh_o, void * a_key) */
/* { */
/*   cw_uint32_t retval; */

/*   _cw_check_ptr(a_oh_o); */

/*   retval = (a_key >> 4) % (1 << a_oh_o->curr_power); */

/*   return retval; */
/* } */

cw_uint32_t
oh_h1_priv(cw_oh_t * a_oh_o, void * a_key)
{
  cw_uint32_t retval;
  char * str;

  for (str = (char *) a_key, retval = 0; *str != 0; str++)
  {
    retval = retval * 33 + *str;
  }

  retval = retval % (1 << a_oh_o->curr_power);
  return retval;
}

cw_bool_t
oh_coalesce_priv(cw_oh_t * a_oh_o)
{
  cw_bool_t retval;
  
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
  return retval;
}

cw_bool_t
oh_grow_priv(cw_oh_t * a_oh_o,
	     cw_sint32_t a_num_doublings)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t old_size, i;
  cw_bool_t retval;

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
  return retval;
}

cw_bool_t
oh_shrink_priv(cw_oh_t * a_oh_o,
	       cw_sint32_t a_num_halvings)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t old_size, i;
  cw_bool_t retval;

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
  return retval;
}

cw_bool_t
oh_item_insert_priv(cw_oh_t * a_oh_o,
		    cw_oh_item_t * a_item)
{
  cw_uint32_t slot, i, j, junk;
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh_o);
  _cw_check_ptr(a_item);

#ifdef _OH_PERF_
  a_oh_o->num_inserts++;
#endif

  slot = a_oh_o->curr_h1(a_oh_o, a_item->key);

  for (i = 0, j = slot;
       i < a_oh_o->size;
       i++, j = (j + a_oh_o->curr_h2) % a_oh_o->size)
  {
    if (a_oh_o->items[j] == NULL)
    {
      a_oh_o->items[j] = a_item;
      a_oh_o->num_items++;
      retval = FALSE;
      goto RETURN;
    }
    else if (a_oh_o->items[j]->is_valid == FALSE)
    {
      if (TRUE == oh_item_search_priv(a_oh_o, a_item->key,
				      &junk))
      {
	/* No duplicate in the table.  Go ahead and use this slot. */
	free(a_oh_o->items[j]);
	a_oh_o->items[j] = a_item;
	a_oh_o->num_items++;
	a_oh_o->num_invalid--;
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
      a_oh_o->num_collisions++;
    }
#endif
  }
  retval = TRUE;
  
 RETURN:
  return retval;
}

cw_bool_t
oh_item_search_priv(cw_oh_t * a_oh_o,
		    void * a_key,
		    cw_uint32_t * a_slot)
{
  cw_uint32_t slot, i;
  cw_bool_t retval;
  
  _cw_check_ptr(a_oh_o);

  slot = a_oh_o->curr_h1(a_oh_o, a_key);

  for (i = slot;
       i < a_oh_o->size;
       i = (i + a_oh_o->curr_h2) % a_oh_o->size)
  {
    if (a_oh_o->items[i] == NULL)
    {
      break;
    }
    else if ((a_oh_o->items[i]->is_valid == TRUE) &&
	     (a_oh_o->items[i]->key == a_key))
    {
      *a_slot = i;
      retval = FALSE;
      goto RETURN;
    }
  }

  retval = TRUE;
  
 RETURN:
  return retval;
}

cw_bool_t
oh_rehash_priv(cw_oh_t * a_oh_o, cw_bool_t a_force)
{
  cw_oh_item_t ** old_items;
  cw_uint32_t i;
  cw_bool_t retval = FALSE;

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
	    goto RETURN;
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

 RETURN:
  return retval;
}

#ifdef _OH_PERF_
cw_uint32_t
oh_get_num_collisions(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->num_collisions;
}

cw_uint32_t
oh_get_num_inserts(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->num_inserts;
}

cw_uint32_t
oh_get_num_deletes(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->num_deletes;
}

cw_uint32_t
oh_get_num_grows(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->num_grows;
}

cw_uint32_t
oh_get_num_shrinks(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->num_shrinks;
}

cw_uint32_t
oh_get_num_rehashes(cw_oh_t * a_oh_o)
{
  _cw_check_ptr(a_oh_o);
  return a_oh_o->num_rehashes;
}

#endif
