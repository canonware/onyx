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
 * $Revision: 33 $
 * $Date: 1998-04-19 01:43:20 -0700 (Sun, 19 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_LIST_H_
#define _INC_LIST_PRIV_H_
#include <config.h>

cw_list_item_t *
list_item_new()
{
  cw_list_item_t * retval;

  retval = (cw_list_item_t *) _cw_malloc(sizeof(cw_list_item_t));

  return retval;
}

void
list_item_delete(cw_list_item_t * a_cont)
{
  _cw_check_ptr(a_cont);

  _cw_free(a_cont);
}

void *
list_item_get(cw_list_item_t * a_cont)
{
  _cw_check_ptr(a_cont);

  return a_cont->item;
}

void
list_item_set(cw_list_item_t * a_cont, void * a_item)
{
  _cw_check_ptr(a_cont);
  _cw_check_ptr(a_item);

  a_cont->item = a_item;
}

cw_list_t *
list_new()
{
  cw_list_t * retval;

  retval = (cw_list_t *) _cw_malloc(sizeof(cw_list_t));

  retval->head = NULL;
  retval->tail = NULL;
  retval->count = 0;

  return retval;
}
     
void
list_delete(cw_list_t * a_list)
{
  cw_sint32_t i;

  _cw_check_ptr(a_list);

  for (i = list_count(a_list); i > 0; i--)
  {
    _cw_free(list_hpop(a_list));
  }
  _cw_free(a_list);
}

cw_sint32_t
list_count(cw_list_t * a_list)
{
  _cw_check_ptr(a_list);
  
  return a_list->count;
}

void
list_hpush(cw_list_t * a_list, cw_list_item_t * a_item)
{
  _cw_check_ptr(a_list);
  _cw_check_ptr(a_item);
  
  if (a_list->head != NULL)
  {
    a_item->prev = NULL;
    a_item->next = a_list->head;
    a_list->head->prev = a_item;
    a_list->head = a_item;
  }
  else
  {
    a_item->prev = NULL;
    a_item->next = NULL;
    a_list->head = a_item;
    a_list->tail = a_item;
  }
  a_list->count++;
}

cw_list_item_t *
list_hpop(cw_list_t * a_list)
{
  cw_list_item_t * retval;

  _cw_check_ptr(a_list);

  if (a_list->head == NULL)
  {
    retval = NULL;
    goto RETURN;
  }
  else if (a_list->head == a_list->tail)
  {
    retval = a_list->head;
    a_list->head = NULL;
    a_list->tail = NULL;
  }
  else
  {
    retval = a_list->head;
    a_list->head = a_list->head->next;
    _cw_assert(a_list->head != NULL);
    _cw_assert(a_list->head->prev != NULL);
    a_list->head->prev = NULL;
    retval->next = NULL;
  }
  a_list->count--;

 RETURN:
  return retval;
}

void
list_tpush(cw_list_t * a_list, cw_list_item_t * a_item)
{
  _cw_check_ptr(a_list);
  _cw_check_ptr(a_item);

  if (a_list->tail != NULL)
  {
    a_item->next = NULL;
    a_item->prev = a_list->tail;
    a_list->tail->next = a_item;
    a_list->tail = a_item;
  }
  else
  {
    a_item->prev = NULL;
    a_item->next = NULL;
    a_list->head = a_item;
    a_list->tail = a_item;
  }
  a_list->count++;
}

cw_list_item_t *
list_tpop(cw_list_t * a_list)
{
  cw_list_item_t * retval;

  _cw_check_ptr(a_list);

  if (a_list->tail == NULL)
  {
    retval = NULL;
  }
  else if (a_list->tail == a_list->head)
  {
    retval = a_list->tail;
    a_list->head = NULL;
    a_list->tail = NULL;
  }
  else
  {
    retval = a_list->tail;
    a_list->tail = a_list->tail->prev;
    a_list->tail->next = NULL;
    retval->prev = NULL;
  }
  a_list->count--;

  return retval;
}

void
list_insert_after(cw_list_t * a_list,
		  cw_list_item_t * a_in_list,
		  cw_list_item_t * a_to_insert)
{
  _cw_check_ptr(a_list);
  _cw_check_ptr(a_in_list);
  _cw_check_ptr(a_to_insert);

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
  a_list->count++;
}

cw_list_item_t *
list_remove(cw_list_t * a_list, cw_list_item_t * a_to_remove)
{
  cw_list_item_t * retval;

  _cw_check_ptr(a_list);
  _cw_check_ptr(a_to_remove);

  if (a_to_remove->prev == NULL)
  {
    retval = list_hpop(a_list);
  }
  else if (a_to_remove->next == NULL)
  {
    retval = list_tpop(a_list);
  }
  else
  {
    a_to_remove->prev->next = a_to_remove->next;
    a_to_remove->next->prev = a_to_remove->prev;
    a_to_remove->prev = NULL;
    a_to_remove->next = NULL;
    retval = a_to_remove;
    a_list->count--;
  }

  return retval;
}

