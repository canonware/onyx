/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 86 $
 * $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
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
