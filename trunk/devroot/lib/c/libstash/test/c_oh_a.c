/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Simple oh test.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_OH
#include <libstash/libstash_r.h>

#define NUM_STRINGS 5000

int
main()
{
  cw_oh_t hash;
  char ** strings, * junk;
/*   oh_h1_t * h1_ptr; */
  int i;
  cw_bool_t error;

  libstash_init();
  
  oh_new(&hash, FALSE);
/*   dbg_register(cw_g_dbg, "oh_slot"); */
  
  strings = (char **) _cw_malloc(sizeof(char *) * NUM_STRINGS);

  for (i = 0; i < NUM_STRINGS; i++)
  {
    strings[i] = (char *) _cw_malloc(sizeof(char) * 50);
    sprintf(strings[i], "(%d) This is string %d", i, i);
  }

/*   h1_ptr = oh_get_h1(hash); */
/*   oh_set_h1(hash, new_h1); */

/*   log_printf(cw_g_log, "<<< Begin first insertion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS; i++)
  {
/*     log_printf(cw_g_log, "<<< Iteration %d >>>\n", i); */
    error = oh_item_insert(&hash, (void *) strings[i],
				 (void *) &(strings[i]));
    if (error == TRUE)
    {
      log_printf(cw_g_log, "(1) Error at i == %d\n", i);
      oh_dump(&hash, FALSE);
      exit(1);
    }
  }

/*   log_printf(cw_g_log, "<<< Begin first deletion loop >>>\n"); */
  for (i = 0; i < (NUM_STRINGS / 2); i++)
  {
    error = oh_item_delete(&hash, (void *) strings[i],
			   (void **) &junk, (void **) &junk);
    if (error == TRUE)
    {
      log_printf(cw_g_log, "(2) Error at i == %d\n", i);
      oh_dump(&hash, FALSE);
      exit(1);
    }
  }

/*   log_printf(cw_g_log, "<<< Begin second insertion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS / 2; i++)
  {
    error = oh_item_insert(&hash, (void *) strings[i],
				 (void *) &(strings[i]));
    if (error == TRUE)
    {
      log_printf(cw_g_log, "(3) Error at i == %d\n", i);
      oh_dump(&hash, FALSE);
      exit(1);
    }
  }

/*   log_printf(cw_g_log, "<<< Begin second deletion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS; i++)
  {
    error = oh_item_delete(&hash, (void *) strings[i],
				 (void **) &junk, (void **) &junk);
    if (error == TRUE)
    {
      log_printf(cw_g_log, "(4) Error at i == %d\n", i);
      oh_dump(&hash, FALSE);
      exit(1);
    }
  }

/*   log_printf(cw_g_log, "<<< Final insertion >>>\n"); */
  error = oh_item_insert(&hash, (void *) strings[0],
			       (void *) &(strings[0]));
  
  {
    char t_buf[21];
    
    log_printf(cw_g_log, "Table size: %s\n",
	       log_print_uint64(oh_get_size(&hash), 10, t_buf));
    log_printf(cw_g_log, "Number of items: %s\n",
	       log_print_uint64(oh_get_num_items(&hash), 10, t_buf));
  }

/*   oh_dump(&hash, FALSE); */
  
  for (i = 0; i < NUM_STRINGS; i++)
  {
    _cw_free(strings[i]);
  }

  oh_delete(&hash);
  libstash_shutdown();
  
  return 0;
}
