/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
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
  cw_sint32_t ins_error;

  libstash_init();
  out_put(cw_g_out, "Test begin\n");
  
  oh_new(&hash, FALSE);
/*   dbg_register(cw_g_dbg, "oh_slot"); */
  
  strings = (char **) _cw_malloc(sizeof(char *) * NUM_STRINGS);

  for (i = 0; i < NUM_STRINGS; i++)
  {
    strings[i] = (char *) _cw_malloc(sizeof(char) * 50);
    out_put_s(cw_g_out, strings[i], "([i32]) This is string [i32]", i, i);
  }

/*   h1_ptr = oh_get_h1(hash); */
/*   oh_set_h1(hash, new_h1); */

/*   out_put(cw_g_out, "<<< Begin first insertion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS; i++)
  {
/*     out_put(cw_g_out, "<<< Iteration [i32] >>>\n", i); */
    ins_error = oh_item_insert(&hash, (void *) strings[i],
			       (void *) &(strings[i]));
    if (ins_error == 1)
    {
      out_put(cw_g_out, "(1) Error at i == [i32]\n", i);
      oh_dump(&hash, FALSE);
      exit(1);
    }
  }

/*   out_put(cw_g_out, "<<< Begin first deletion loop >>>\n"); */
  for (i = 0; i < (NUM_STRINGS / 2); i++)
  {
    error = oh_item_delete(&hash, (void *) strings[i],
			   (void **) &junk, (void **) &junk);
    if (error == TRUE)
    {
      out_put(cw_g_out, "(2) Error at i == [i32]\n", i);
      oh_dump(&hash, FALSE);
      exit(1);
    }
  }

/*   out_put(cw_g_out, "<<< Begin second insertion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS / 2; i++)
  {
    ins_error = oh_item_insert(&hash, (void *) strings[i],
			       (void *) &(strings[i]));
    if (ins_error == 1)
    {
      out_put(cw_g_out, "(3) Error at i == [i32]\n", i);
      oh_dump(&hash, FALSE);
      exit(1);
    }
  }

/*   out_put(cw_g_out, "<<< Begin second deletion loop >>>\n"); */
  for (i = 0; i < NUM_STRINGS; i++)
  {
    error = oh_item_delete(&hash, (void *) strings[i],
			   (void **) &junk, (void **) &junk);
    if (error == TRUE)
    {
      out_put(cw_g_out, "(4) Error at i == [i32]\n", i);
      oh_dump(&hash, FALSE);
      exit(1);
    }
  }

/*   out_put(cw_g_out, "<<< Final insertion >>>\n"); */
  ins_error = oh_item_insert(&hash, (void *) strings[0],
			     (void *) &(strings[0]));
  
  {
    out_put(cw_g_out, "Table size: [i64]\n",
	    oh_get_size(&hash));
    out_put(cw_g_out, "Number of items: [i64]\n",
	    oh_get_num_items(&hash));
  }

/*   oh_dump(&hash, FALSE); */
  
  for (i = 0; i < NUM_STRINGS; i++)
  {
    _cw_free(strings[i]);
  }
  _cw_free(strings);

  oh_delete(&hash);
  out_put(cw_g_out, "Test end\n");
  libstash_shutdown();
  
  return 0;
}
