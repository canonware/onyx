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
 * $Revision: 78 $
 * $Date: 1998-05-15 01:43:21 -0700 (Fri, 15 May 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_OH_H_
#define _INC_GLOB_H_
#define _INC_THREAD_H_
#include <config.h>

#define NUM_STRINGS 100
#define NUM_THREADS 20

void *
insert_items(void * arg)
{
  int i;
  char * string;
  cw_oh_t * hash_o = (cw_oh_t *) arg;

  for (i = 0; i < NUM_STRINGS; i++)
  {
    string = (char *) _cw_malloc(20);
    sprintf(string, "String %d", i);
    oh_item_insert(hash_o, (void *) string, (void *) string);
  }
  return NULL;
}

int
main()
{
  cw_oh_t * hash_o;
  cw_thd_t threads[NUM_THREADS];
  int i;

  glob_new();
  hash_o = oh_new(NULL, TRUE, FALSE);

  for (i = 0; i < NUM_THREADS; i++)
  {
    thd_new(&threads[i], insert_items, (void *) hash_o);
  }

  /* Join on threads, then delete them. */

  for (i = 0; i < NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
    thd_delete(&threads[i]);
  }

  log_printf(g_log_o, "Number of items in hash table: %d\n",
	     oh_get_num_items(hash_o));
  
  oh_delete(hash_o);
  glob_delete();
  
  return 0;
}
