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
 * $Revision: 68 $
 * $Date: 1998-05-02 02:08:23 -0700 (Sat, 02 May 1998) $
 *
 * <<< Description >>>
 *
 * Doubly linked list implementation.
 *
 * XXX This implementation is slow due to excessive malloc()s and free()s.
 * At some point, this implementation needs changed, either so that it
 * keeps around a pool of spare item structures instead of free()ing them,
 * or chains large structures that contain arrays of items.  Either will
 * reduce the malloc() calls.  The latter has the potential advantage of
 * dynamically contracting its storage requirements autonomously, but the
 * implementation is more complex and requires lots of memory->memory
 * copies.  Probably the best way to go is to keep a pool of free items
 * around, and provide a public method to empty the free pool.
 *
 ****************************************************************************/

#define _INC_LIST_H_
#include <config.h>
#include <list_priv.h>

cw_list_item_t *
list_item_new()
{
  cw_list_item_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_item_new()");
  }
  
  retval = (cw_list_item_t *) _cw_malloc(sizeof(cw_list_item_t));

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_item_new()");
  }
  return retval;
}

void
list_item_delete(cw_list_item_t * a_list_item_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_item_delete()");
  }
  _cw_check_ptr(a_list_item_o);

  _cw_free(a_list_item_o);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_item_delete()");
  }
}

void *
list_item_get(cw_list_item_t * a_list_item_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_item_get()");
  }
  _cw_check_ptr(a_list_item_o);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_item_get()");
  }
  return a_list_item_o->item;
}

void
list_item_set(cw_list_item_t * a_list_item_o, void * a_item)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_item_set()");
  }
  _cw_check_ptr(a_list_item_o);
  _cw_check_ptr(a_item);

  a_list_item_o->item = a_item;
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_item_set()");
  }
}

cw_list_t *
list_new(cw_list_t * a_list_o, cw_bool_t a_is_thread_safe)
{
  cw_list_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_new()");
  }
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

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_new()");
  }
  return retval;
}
     
void
list_delete(cw_list_t * a_list_o)
{
  cw_sint64_t i;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_delete()");
  }
  _cw_check_ptr(a_list_o);

  for (i = list_count(a_list_o); i > 0; i--)
  {
    _cw_free(list_hpop(a_list_o));
  }

  if (a_list_o->is_thread_safe)
  {
    mtx_delete(&a_list_o->lock);
  }
  if (a_list_o->is_malloced)
  {
    _cw_free(a_list_o);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_delete()");
  }
}

cw_sint64_t
list_count(cw_list_t * a_list_o)
{
  cw_sint64_t retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_count()");
  }
  _cw_check_ptr(a_list_o);

  retval = a_list_o->count;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_count()");
  }
  return retval;
}

void
list_hpush(cw_list_t * a_list_o, cw_list_item_t * a_item)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_hpush()");
  }
  _cw_check_ptr(a_list_o);
  _cw_check_ptr(a_item);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }
  
  if (a_list_o->head != NULL)
  {
    a_item->prev = NULL;
    a_item->next = a_list_o->head;
    a_list_o->head->prev = a_item;
    a_list_o->head = a_item;
  }
  else
  {
    a_item->prev = NULL;
    a_item->next = NULL;
    a_list_o->head = a_item;
    a_list_o->tail = a_item;
  }
  a_list_o->count++;

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_hpush()");
  }
}

cw_list_item_t *
list_hpop(cw_list_t * a_list_o)
{
  cw_list_item_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_hpop()");
  }
  _cw_check_ptr(a_list_o);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  if (a_list_o->head == NULL)
  {
    retval = NULL;
    goto RETURN;
  }
  else if (a_list_o->head == a_list_o->tail)
  {
    retval = a_list_o->head;
    a_list_o->head = NULL;
    a_list_o->tail = NULL;
  }
  else
  {
    retval = a_list_o->head;
    a_list_o->head = a_list_o->head->next;
    _cw_assert(a_list_o->head != NULL);
    _cw_assert(a_list_o->head->prev != NULL);
    a_list_o->head->prev = NULL;
    retval->next = NULL;
  }
  a_list_o->count--;

 RETURN:
  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_hpop()");
  }
  return retval;
}

void
list_tpush(cw_list_t * a_list_o, cw_list_item_t * a_item)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_tpush()");
  }
  _cw_check_ptr(a_list_o);
  _cw_check_ptr(a_item);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  if (a_list_o->tail != NULL)
  {
    a_item->next = NULL;
    a_item->prev = a_list_o->tail;
    a_list_o->tail->next = a_item;
    a_list_o->tail = a_item;
  }
  else
  {
    a_item->prev = NULL;
    a_item->next = NULL;
    a_list_o->head = a_item;
    a_list_o->tail = a_item;
  }
  a_list_o->count++;

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_tpush()");
  }
}

cw_list_item_t *
list_tpop(cw_list_t * a_list_o)
{
  cw_list_item_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_tpop()");
  }
  _cw_check_ptr(a_list_o);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  if (a_list_o->tail == NULL)
  {
    retval = NULL;
  }
  else if (a_list_o->tail == a_list_o->head)
  {
    retval = a_list_o->tail;
    a_list_o->head = NULL;
    a_list_o->tail = NULL;
  }
  else
  {
    retval = a_list_o->tail;
    a_list_o->tail = a_list_o->tail->prev;
    a_list_o->tail->next = NULL;
    retval->prev = NULL;
  }
  a_list_o->count--;

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_tpop()");
  }
  return retval;
}

void
list_insert_after(cw_list_t * a_list_o,
		  cw_list_item_t * a_in_list,
		  cw_list_item_t * a_to_insert)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_insert_after()");
  }
  _cw_check_ptr(a_list_o);
  _cw_check_ptr(a_in_list);
  _cw_check_ptr(a_to_insert);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  if (a_in_list->next == NULL)
  {
    a_in_list->next = a_to_insert;
    a_to_insert->prev = a_in_list;
    a_to_insert->next = NULL;
  }
  else
  {
    a_to_insert->prev = a_in_list;
    a_to_insert->next = a_in_list->next;
    a_in_list->next = a_to_insert;
    a_to_insert->next->prev = a_to_insert;
  }
  a_list_o->count++;

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_insert_after()");
  }
}

cw_list_item_t *
list_remove(cw_list_t * a_list_o, cw_list_item_t * a_to_remove)
{
  cw_list_item_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Enter list_remove()");
  }
  _cw_check_ptr(a_list_o);
  _cw_check_ptr(a_to_remove);
  if (a_list_o->is_thread_safe)
  {
    mtx_lock(&a_list_o->lock);
  }

  if (a_to_remove->prev == NULL)
  {
    retval = list_hpop(a_list_o);
  }
  else if (a_to_remove->next == NULL)
  {
    retval = list_tpop(a_list_o);
  }
  else
  {
    a_to_remove->prev->next = a_to_remove->next;
    a_to_remove->next->prev = a_to_remove->prev;
    a_to_remove->prev = NULL;
    a_to_remove->next = NULL;
    retval = a_to_remove;
    a_list_o->count--;
  }

  if (a_list_o->is_thread_safe)
  {
    mtx_unlock(&a_list_o->lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_LIST_FUNC))
  {
    _cw_marker("Exit list_remove()");
  }
  return retval;
}
