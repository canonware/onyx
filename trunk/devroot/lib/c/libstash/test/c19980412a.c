/* -*-mode:c-*-
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
 * Multi-threded oh test.
 *
 ****************************************************************************/

#define _INC_OH_H_
#define _INC_GLOB_H_
#define _INC_THREAD_H_
#include <libstash.h>

/* XXX This test leaks memory, but it doesn't affect the results. */

#define NUM_STRINGS 20
#define NUM_THREADS 100

struct foo_s
{
  cw_oh_t * hash_o;
  cw_uint32_t thread_num;
};

void *
insert_items(void * a_arg)
{
  cw_uint32_t i;
  char * string;
  struct foo_s * foo_var = (struct foo_s *) a_arg;

  for (i = 0; i < NUM_STRINGS; i++)
  {
    string = (char *) _cw_malloc(40);
    sprintf(string, "thread %u, string %u",
	    foo_var->thread_num, i);
    _cw_assert(FALSE == oh_item_insert(foo_var->hash_o,
				       (void *) string, (void *) string));
/*     log_eprintf(g_log_o, NULL, 0, "insert_items", */
/* 		"thread %u, end iteration %u\n", foo_var->thread_num, i); */
  }
  
  return NULL;
}

int
main()
{
  cw_oh_t * hash_o;
  cw_thd_t threads[NUM_THREADS];
  cw_uint32_t i;
  struct foo_s * foo_var;
  cw_rwl_t lock;

  glob_new();
  hash_o = oh_new(NULL, TRUE);
  rwl_new(&lock);

  for (i = 0; i < NUM_THREADS; i++)
  {
    foo_var = (struct foo_s *) _cw_malloc(sizeof(struct foo_s));
    foo_var->hash_o = hash_o;
    foo_var->thread_num = i;
    
    thd_new(&threads[i], insert_items, (void *) foo_var);
/*     log_printf(g_log_o, "Got to end of for loop, i == %u\n", i); */
  }

  /* Join on threads, then delete them. */
  for (i = 0; i < NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
    thd_delete(&threads[i]);
  }
  
  {
    char buf[21];
    
    log_printf(g_log_o, "Number of items in hash table: %s\n",
	       log_print_uint64(oh_get_num_items(hash_o), 10, buf));
  }

  rwl_delete(&lock);
  oh_delete(hash_o);
  glob_delete();
  
  return 0;
}
