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
 * rwl test.
 *
 ****************************************************************************/

#include <libstash/libstash_r.h>

#define _STASH_TEST_NUM_THREADS 20

void *
thread_entry_func(void * a_arg)
{
  cw_rwl_t * lock = (cw_rwl_t *) a_arg;

  rwl_wlock(lock);
  log_eprintf(g_log, NULL, 0, "thread_entry_func",
	      "Got wlock\n");
  rwl_wunlock(lock);

  return NULL;
}

int
main()
{
  cw_thd_t threads[_STASH_TEST_NUM_THREADS];
  cw_rwl_t lock_a, * lock_b;
  cw_uint32_t i;
  
  libstash_init();

  lock_b = rwl_new(NULL);
  _cw_check_ptr(lock_b);
  rwl_rlock(lock_b);
  rwl_rlock(lock_b);
  rwl_runlock(lock_b);
  rwl_runlock(lock_b);
  rwl_wlock(lock_b);
  rwl_wunlock(lock_b);
  rwl_delete(lock_b);
  
  _cw_assert(&lock_a == rwl_new(&lock_a));
  rwl_rlock(&lock_a);
  rwl_rlock(&lock_a);
  rwl_rlock(&lock_a);
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_new(&threads[i], thread_entry_func, (void *) &lock_a);
  }
  log_eprintf(g_log, NULL, 0, "main", "About to release rlock\n");
  rwl_runlock(&lock_a);
  usleep(1);
  log_eprintf(g_log, NULL, 0, "main", "About to release rlock\n");
  rwl_runlock(&lock_a);
  usleep(1);
  log_eprintf(g_log, NULL, 0, "main", "About to release rlock\n");
  rwl_runlock(&lock_a);
  
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
    thd_delete(&threads[i]);
  }

  rwl_wlock(&lock_a);
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_new(&threads[i], thread_entry_func, (void *) &lock_a);
  }
  log_eprintf(g_log, NULL, 0, "main", "About to release wlock\n");
  usleep(1);
  rwl_wunlock(&lock_a);

  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
    thd_delete(&threads[i]);
  }
  
  rwl_delete(&lock_a);

  libstash_shutdown();
  return 0;
}
