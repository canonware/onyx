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
 * Multi-threded oh test.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_OH
#define _LIBSTASH_USE_THREAD
#include <libstash/libstash_r.h>

/* XXX This test leaks memory, but it doesn't affect the results. */

#define NUM_STRINGS 20
#define NUM_THREADS 100

struct foo_s
{
  cw_oh_t * hash;
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
    _cw_assert(FALSE == oh_item_insert(foo_var->hash,
				       (void *) string, (void *) string));
/*     log_eprintf(cw_g_log, NULL, 0, "insert_items", */
/* 		"thread %u, end iteration %u\n", foo_var->thread_num, i); */
  }

  _cw_free(a_arg);
  
  return NULL;
}

int
main()
{
  cw_oh_t * hash;
  cw_thd_t threads[NUM_THREADS];
  cw_uint32_t i;
  struct foo_s * foo_var;
  cw_rwl_t lock;

  libstash_init();
  hash = oh_new(NULL, TRUE);
  rwl_new(&lock);

  for (i = 0; i < NUM_THREADS; i++)
  {
    foo_var = (struct foo_s *) _cw_malloc(sizeof(struct foo_s));
    foo_var->hash = hash;
    foo_var->thread_num = i;
    
    thd_new(&threads[i], insert_items, (void *) foo_var);
/*     log_printf(cw_g_log, "Got to end of for loop, i == %u\n", i); */
  }

  /* Join on threads. */
  for (i = 0; i < NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
  }

  {
    char buf[21];
    
    log_printf(cw_g_log, "Number of items in hash table: %s\n",
	       log_print_uint64(oh_get_num_items(hash), 10, buf));
  }
  
  /* Delete all the strings. */
  {
    cw_uint64_t i, num_strings;
    void * string, * junk;

    for (i = 0, num_strings = oh_get_num_items(hash);
	 i < num_strings;
	 i++)
    {
      _cw_assert(FALSE == oh_item_delete_iterate(hash, &string, &junk));
      _cw_free(string);
    }
  }

  rwl_delete(&lock);
  oh_delete(hash);
  libstash_shutdown();
  
  return 0;
}
