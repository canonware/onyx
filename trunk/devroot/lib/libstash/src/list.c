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
 * $Revision: 18 $
 * $Date: 1998-03-31 00:27:07 -0800 (Tue, 31 Mar 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_LIST_H_
#define _INC_LIST_PRIV_H_
#include <config.h>

list_item_t *
list_item_init()
{
  list_item_t * retval;

  retval = (list_item_t *) _cw_malloc(sizeof(list_item_t));
  _cw_check_ptr(retval);

  return retval;
}

void
list_item_fini(list_item_t * arg_cont)
{
  _cw_check_ptr(arg_cont);

  _cw_free(arg_cont);
}

void * list_item_get(list_item_t * arg_cont)
{
  _cw_check_ptr(arg_cont);

  return arg_cont->item;
}

void list_item_set(list_item_t * arg_cont, void * arg_item)
{
  _cw_check_ptr(arg_cont);
  _cw_check_ptr(arg_item);

  arg_cont->item = arg_item;
}

list_t *
list_init()
{
  list_t * retval;

  retval = (list_t *) _cw_malloc(sizeof(list_t));
  _cw_check_ptr(retval);

  retval->head = NULL;
  retval->tail = NULL;
  retval->count = 0;

  return retval;
}
     
void
list_fini(list_t * arg_list)
{
  /*   list_item_t * curr_ptr; */
  cw_sint32_t i;

  _cw_check_ptr(arg_list);

  for (i = list_count(arg_list); i > 0; i--)
  {
    _cw_free(list_hpop(arg_list));
  }
  _cw_free(arg_list);
  
  /*   if ((arg_list->head != NULL) && (arg_list->tail != NULL)) */
  /*   { */
  /*     for (curr_ptr = arg_list->head->next; */
  /* 	 curr_ptr != NULL; */
  /* 	 curr_ptr = curr_ptr->next) */
  /*     { */
  /*       _cw_free(curr_ptr->prev); */
  /*     } */
  /*     _cw_free(arg_list->tail); */
  /*   } */

  /*   _cw_free(arg_list); */
}

cw_sint32_t
list_count(list_t * arg_list)
{
  _cw_check_ptr(arg_list);
  
  return arg_list->count;
}

void
list_hpush(list_t * arg_list, list_item_t * arg_item)
{
  _cw_check_ptr(arg_list);
  _cw_check_ptr(arg_item);
  
  if (arg_list->head != NULL)
  {
    arg_item->prev = NULL;
    arg_item->next = arg_list->head;
    arg_list->head->prev = arg_item;
    arg_list->head = arg_item;
  }
  else
  {
    arg_item->prev = NULL;
    arg_item->next = NULL;
    arg_list->head = arg_item;
    arg_list->tail = arg_item;
  }
  arg_list->count++;
}

list_item_t *
list_hpop(list_t * arg_list)
{
  list_item_t * retval;

  _cw_check_ptr(arg_list);

  if (arg_list->head == NULL)
  {
    retval = NULL;
  }
  else if (arg_list->head == arg_list->tail)
  {
    retval = arg_list->head;
    arg_list->head = NULL;
    arg_list->tail = NULL;
  }
  else
  {
    retval = arg_list->head;
    retval->next = NULL;
    arg_list->head = arg_list->head->next;
    arg_list->head->prev = NULL;
  }
  arg_list->count--;

  return retval;
}

void
list_tpush(list_t * arg_list, list_item_t * arg_item)
{
  _cw_check_ptr(arg_list);
  _cw_check_ptr(arg_item);

  if (arg_list->tail != NULL)
  {
    arg_item->next = NULL;
    arg_item->prev = arg_list->tail;
    arg_list->tail->next = arg_item;
    arg_list->tail = arg_item;
  }
  else
  {
    arg_item->prev = NULL;
    arg_item->next = NULL;
    arg_list->head = arg_item;
    arg_list->tail = arg_item;
  }
  arg_list->count++;
}

list_item_t *
list_tpop(list_t * arg_list)
{
  list_item_t * retval;

  _cw_check_ptr(arg_list);

  if (arg_list->tail == NULL)
  {
    retval = NULL;
  }
  else if (arg_list->tail == arg_list->head)
  {
    retval = arg_list->tail;
    arg_list->head = NULL;
    arg_list->tail = NULL;
  }
  else
  {
    retval = arg_list->tail;
    retval->prev = NULL;
    arg_list->tail = arg_list->tail->prev;
    arg_list->tail->next = NULL;
  }
  arg_list->count++;

  return retval;
}

void
list_insert_after(list_t * arg_list,
		  list_item_t * arg_in_list,
		  list_item_t * arg_to_insert)
{
  _cw_check_ptr(arg_list);
  _cw_check_ptr(arg_in_list);
  _cw_check_ptr(arg_to_insert);

  if (arg_in_list->next == NULL)
  {
    arg_in_list->next = arg_to_insert;
    arg_to_insert->prev = arg_in_list;
    arg_to_insert->next = NULL;
  }
  else
  {
    arg_to_insert->prev = arg_in_list;
    arg_to_insert->next = arg_in_list->next;
    arg_in_list->next = arg_to_insert;
    arg_to_insert->next->prev = arg_to_insert;
  }
  arg_list->count++;
}

list_item_t *
list_remove(list_t * arg_list, list_item_t * arg_to_remove)
{
  list_item_t * retval;

  _cw_check_ptr(arg_list);
  _cw_check_ptr(arg_to_remove);

  if (arg_to_remove->prev == NULL)
  {
    retval = list_hpop(arg_list);
  }
  else if (arg_to_remove->next == NULL)
  {
    retval = list_tpop(arg_list);
  }
  else
  {
    arg_to_remove->prev->next = arg_to_remove->next;
    arg_to_remove->next->prev = arg_to_remove->prev;
    arg_to_remove->prev = NULL;
    arg_to_remove->next = NULL;
    retval = arg_to_remove;
    arg_list->count--;
  }

  return retval;
}

