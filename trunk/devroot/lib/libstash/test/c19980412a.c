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
 *
 *
 ****************************************************************************/

#ifndef _CW_DEBUG
#  define _CW_DEBUG
#endif

#define _INC_OH_H_
#define _INC_GLOB_H_
#define _INC_THREAD_H_
#include <libstash.h>

#define NUM_STRINGS 100
#define NUM_THREADS 20

struct foo
{
  cw_oh_t * hash_o;
  cw_uint32_t thread_num;
};


void *
insert_items(void * arg)
{
  cw_uint32_t i, thread_num;
  char * string;
  cw_oh_t * hash_o;
  struct foo * bar = (struct foo *) arg;

  hash_o = (cw_oh_t *) bar->hash_o;
  thread_num = (cw_uint32_t) bar->thread_num;

  for (i = 0; i < NUM_STRINGS; i++)
  {
    string = (char *) _cw_malloc(20);
    sprintf(string, "%d String %d", thread_num, i);
    _cw_assert(oh_item_insert(hash_o, (void *) string, (void *) string)
	       == FALSE);
  }
  return NULL;
}

int
main()
{
  cw_oh_t * hash_o;
  cw_thd_t threads[NUM_THREADS];
  cw_uint32_t i;
  struct foo * bar;

  glob_new();
  hash_o = oh_new(NULL, TRUE);

  for (i = 0; i < NUM_THREADS; i++)
  {
    bar = (struct foo *) _cw_malloc(sizeof(struct foo));
    bar->hash_o = hash_o;
    bar->thread_num = i;
    
    thd_new(&threads[i], insert_items, (void *) bar);
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
