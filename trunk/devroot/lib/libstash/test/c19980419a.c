/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1998
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

#define _INC_GLOB_H_
#define _INC_LIST_H_
#include <config.h>

#define NUM_ITEMS 50

int
main()
{
  char * strings[NUM_ITEMS];
  cw_list_t * list1, * list2;
  cw_list_item_t * item;
  int i;
  
  glob_new();

  for (i = 0; i < NUM_ITEMS; i++)
  {
    strings[i] = (char *) _cw_malloc(NUM_ITEMS);
    sprintf(strings[i], "This is string %d", i);
  }

  list1 = list_new();
  list2 = list_new();
  _cw_assert(list_count(list1) == 0);
  _cw_assert(list_count(list2) == 0);
	     
  for (i = 0; i < NUM_ITEMS; i++)
  {
    _cw_assert(list_count(list1) == i);

    item = list_item_new();
    list_item_set(item, (void *) strings[i]);
    list_tpush(list1, item);
  }
  _cw_assert(list_count(list1) == NUM_ITEMS);
  
  log_printf(g_log_o, "hpop()ping from list1 and hpush()ing to list2\n");
  
  for (i = 0; i < NUM_ITEMS; i++)
  {
    item = list_hpop(list1);
    _cw_check_ptr(item);
    log_printf(g_log_o, "%s\n", (char *) list_item_get(item));
    list_hpush(list2, item);
  }

  _cw_assert(list_count(list1) == 0);
  _cw_assert(list_count(list2) == NUM_ITEMS);

  log_printf(g_log_o, "tpop()ping from list2 and hpush()ing to list2\n");
  
  for (i = 0; i < NUM_ITEMS; i++)
  {
    item = list_tpop(list2);
    _cw_check_ptr(item);
    log_printf(g_log_o, "%s\n", (char *) list_item_get(item));
    list_hpush(list2, item);
  }

  _cw_assert(list_count(list1) == 0);
  log_printf(g_log_o, "list2->count == %d\n", list_count(list2));
  _cw_assert(list_count(list2) == NUM_ITEMS);

  log_printf(g_log_o, "tpop()ping from list2 and tpush()ing to list1\n");
  
  for (i = 0; i < NUM_ITEMS; i++)
  {
    item = list_tpop(list2);
    _cw_check_ptr(item);
    log_printf(g_log_o, "%s\n", (char *) list_item_get(item));
    list_tpush(list1, item);
  }

  _cw_assert(list_count(list1) == NUM_ITEMS);
  _cw_assert(list_count(list2) == 0);
  
  list_delete(list1);
  list_delete(list2);
    
  glob_delete();
  
  return 0;
}
