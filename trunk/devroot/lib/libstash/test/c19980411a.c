/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 136 $
 * $Date: 1998-07-10 13:11:48 -0700 (Fri, 10 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Simple test to make sure the test harness is working.
 *
 ****************************************************************************/

#ifndef _CW_DEBUG
#  define _CW_DEBUG
#endif

#define _INC_OH_H_
#define _INC_GLOB_H_
#include <libstash.h>

#define NUM_STRINGS 5000

int
main()
{
  cw_oh_t hash_o;
  char ** strings, * junk;
/*   oh_h1_t * h1_ptr; */
  int i;
  cw_bool_t error;

  glob_new();
  oh_new(&hash_o, FALSE);
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

/*   log_printf(g_log_o, "<<< Begin first insertion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS; i++)
  {
/*     log_printf(g_log_o, "<<< Iteration %d >>>\n", i); */
    error = oh_item_insert(&hash_o, (void *) strings[i],
				 (void *) &(strings[i]));
    if (error == TRUE)
    {
      log_printf(g_log_o, "(1) Error at i == %d\n", i);
      oh_dump(&hash_o, FALSE);
      exit(1);
    }
  }

/*   log_printf(g_log_o, "<<< Begin first deletion loop >>>\n"); */
  for (i = 0; i < (NUM_STRINGS / 2); i++)
  {
    error = oh_item_delete(&hash_o, (void *) strings[i],
			   (void **) &junk, (void **) &junk);
    if (error == TRUE)
    {
      log_printf(g_log_o, "(2) Error at i == %d\n", i);
      oh_dump(&hash_o, FALSE);
      exit(1);
    }
  }

/*   log_printf(g_log_o, "<<< Begin second insertion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS / 2; i++)
  {
    error = oh_item_insert(&hash_o, (void *) strings[i],
				 (void *) &(strings[i]));
    if (error == TRUE)
    {
      log_printf(g_log_o, "(3) Error at i == %d\n", i);
      oh_dump(&hash_o, FALSE);
      exit(1);
    }
  }

/*   log_printf(g_log_o, "<<< Begin second deletion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS; i++)
  {
    error = oh_item_delete(&hash_o, (void *) strings[i],
				 (void **) &junk, (void **) &junk);
    if (error == TRUE)
    {
      log_printf(g_log_o, "(4) Error at i == %d\n", i);
      oh_dump(&hash_o, FALSE);
      exit(1);
    }
  }

/*   log_printf(g_log_o, "<<< Final insertion >>>\n"); */
  error = oh_item_insert(&hash_o, (void *) strings[0],
			       (void *) &(strings[0]));
  
  log_printf(g_log_o, "Table size: %d\n",
	     oh_get_size(&hash_o));
  log_printf(g_log_o, "Number of items: %d\n",
	     oh_get_num_items(&hash_o));

/*   oh_dump(&hash_o, FALSE); */
  
  for (i = 0; i < NUM_STRINGS; i++)
  {
    _cw_free(strings[i]);
  }

  oh_delete(&hash_o);
  glob_delete();
  
  return 0;
}
