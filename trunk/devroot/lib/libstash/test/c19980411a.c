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
 * $Revision: 72 $
 * $Date: 1998-05-02 02:15:22 -0700 (Sat, 02 May 1998) $
 *
 * <<< Description >>>
 *
 * Simple test to make sure the test harness is working.
 *
 ****************************************************************************/

#define _INC_OH_H_
#define _INC_GLOB_H_
#include <config.h>

#define NUM_STRINGS 2000

int
main()
{
  cw_oh_t hash_o;
  char ** strings, * junk;
/*   oh_h1_t * h1_ptr; */
  int i;
  cw_bool_t error;

  glob_new();
  oh_new(&hash_o, FALSE, TRUE);
/*   dbg_turn_on(g_dbg_o, _CW_DBG_R_OH_FUNC); */
/*   dbg_turn_on(g_dbg_o, _CW_DBG_R_OH_SLOT); */
  
  strings = (char **) _cw_malloc(sizeof(char *) * NUM_STRINGS);

  for (i = 0; i < NUM_STRINGS; i++)
  {
    strings[i] = (char *) _cw_malloc(sizeof(char) * 50);
    sprintf(strings[i], "(%d) This is string %d", i, i);
  }

/*   h1_ptr = oh_get_h1(hash_o); */
/*   oh_set_h1(hash_o, new_h1); */
  
  for (i = 0; i < NUM_STRINGS; i++)
  {
    error = oh_item_insert(&hash_o, (void *) strings[i],
				 (void *) &(strings[i]));
    if (error == TRUE)
    {
      log_printf(g_log_o, "(1) Error at i == %d\n", i);
      exit(1);
    }
  }

  for (i = 0; i < (NUM_STRINGS / 2); i++)
  {
    error = oh_item_delete(&hash_o, (void *) strings[i],
			   (void **) &junk, (void **) &junk);
    if (error == TRUE)
    {
      log_printf(g_log_o, "(2) Error at i == %d\n", i);
      exit(1);
    }
  }

  for (i = 0; i < NUM_STRINGS / 2; i++)
  {
    error = oh_item_insert(&hash_o, (void *) strings[i],
				 (void *) &(strings[i]));
    if (error == TRUE)
    {
      log_printf(g_log_o, "(3) Error at i == %d\n", i);
      exit(1);
    }
  }

  for (i = 0; i < NUM_STRINGS; i++)
  {
    error = oh_item_delete(&hash_o, (void *) strings[i],
				 (void **) &junk, (void **) &junk);
    if (error == TRUE)
    {
      log_printf(g_log_o, "(4) Error at i == %d\n", i);
      exit(1);
    }
  }

  error = oh_item_insert(&hash_o, (void *) strings[0],
			       (void *) &(strings[0]));
  
  log_printf(g_log_o, "Table size: %d\n",
	     oh_get_size(&hash_o));
  log_printf(g_log_o, "Number of items: %d\n",
	     oh_get_num_items(&hash_o));
  log_printf(g_log_o, "Number of invalid slots: %d\n",
	     oh_get_num_invalid(&hash_o));

/*   oh_dump(&hash_o, TRUE); */
  
  for (i = 0; i < NUM_STRINGS; i++)
  {
    _cw_free(strings[i]);
  }

  oh_delete(&hash_o);
  glob_delete();
  
  return 0;
}
