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
 * $Revision: 39 $
 * $Date: 1998-04-19 21:31:10 -0700 (Sun, 19 Apr 1998) $
 *
 * <<< Description >>>
 *
 * Simple test to make sure the test harness is working.
 *
 ****************************************************************************/

#define _INC_OH_H_
#define _INC_GLOB_H_
#include <config.h>

#define NUM_STRINGS 10000

int
main()
{
  oh_t * hash_obj;
  char ** strings, ** junk;
/*   oh_h1_t * h1_ptr; */
  int i;
  cw_bool_t error;

  glob_new();
  
  strings = (char **) _cw_malloc(sizeof(char *) * NUM_STRINGS);

  for (i = 0; i < NUM_STRINGS; i++)
  {
    strings[i] = (char *) _cw_malloc(sizeof(char) * 50);
    sprintf(strings[i], "(%d) This is string %d", i, i);
  }
  
  hash_obj = oh_new();

/*   h1_ptr = oh_get_h1(hash_obj); */
/*   oh_set_h1(hash_obj, new_h1); */
  
  for (i = 0; i < NUM_STRINGS; i++)
  {
    error = oh_item_insert(hash_obj, (void *) strings[i],
				 (void *) &(strings[i]));
    if (error == TRUE)
    {
      log_printf(g_log_obj, "(1) Error at i == %d\n", i);
      exit(1);
    }
  }

  for (i = 0; i < (NUM_STRINGS / 2); i++)
  {
    error = oh_item_delete(hash_obj, (void *) strings[i],
				 (void *) junk);
    if (error == TRUE)
    {
      log_printf(g_log_obj, "(2) Error at i == %d\n", i);
      exit(1);
    }
  }

  for (i = 0; i < NUM_STRINGS / 2; i++)
  {
    error = oh_item_insert(hash_obj, (void *) strings[i],
				 (void *) &(strings[i]));
    if (error == TRUE)
    {
      log_printf(g_log_obj, "(3) Error at i == %d\n", i);
      exit(1);
    }
  }

  for (i = 0; i < NUM_STRINGS; i++)
  {
    error = oh_item_delete(hash_obj, (void *) strings[i],
				 (void *) junk);
    if (error == TRUE)
    {
      log_printf(g_log_obj, "(4) Error at i == %d\n", i);
      exit(1);
    }
  }

  error = oh_item_insert(hash_obj, (void *) strings[0],
			       (void *) &(strings[0]));
  
  log_printf(g_log_obj, "Table size: %d\n",
	     oh_get_size(hash_obj));
  log_printf(g_log_obj, "Number of items: %d\n",
	     oh_get_num_items(hash_obj));
  log_printf(g_log_obj, "Number of invalid slots: %d\n",
	     oh_get_num_invalid(hash_obj));

  for (i = 0; i < NUM_STRINGS; i++)
  {
    _cw_free(strings[i]);
  }

  glob_delete();
  
  return 0;
}
