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
 * mtx test.
 *
 ****************************************************************************/

#include <libstash/libstash_r.h>

#define _STASH_TEST_COUNT 50

cw_uint32_t g_count = 0;

void *
thread_entry_func(void * a_arg)
{
  cw_uint32_t i, temp;
  cw_mtx_t * mutex = (cw_mtx_t *) a_arg;

  for (i = 0; i < _STASH_TEST_COUNT; i++)
  {
    mtx_lock(mutex);
    temp = g_count;
    usleep(1);
    temp++;
    g_count = temp;
    mtx_unlock(mutex);
  }

  return NULL;
}

int
main()
{
  cw_thd_t thread_a, thread_b;
  cw_mtx_t mutex_a, * mutex_b;
  
  libstash_init();

  _cw_assert(&mutex_a == mtx_new(&mutex_a));
  /* Unlocked. */
  mtx_lock(&mutex_a);
  /* Locked. */
  mtx_unlock(&mutex_a);
  /* Unlocked. */
  _cw_assert(FALSE == mtx_trylock(&mutex_a));
  /* Locked. */
  _cw_assert(TRUE == mtx_trylock(&mutex_a));
  mtx_unlock(&mutex_a);
  /* Unlocked. */
  _cw_assert(FALSE == mtx_trylock(&mutex_a));
  /* Locked. */
  mtx_unlock(&mutex_a);
  /* Unlocked. */
  mtx_delete(&mutex_a);

  mutex_b = mtx_new(NULL);
  mtx_lock(mutex_b);
  _cw_assert(TRUE == mtx_trylock(mutex_b));
  mtx_unlock(mutex_b);
  mtx_delete(mutex_b);

  /* See if the mutex actually locks other threads out of critical
   * sections. */
  mtx_new(&mutex_a);

  thd_new(&thread_a, thread_entry_func, (void *) &mutex_a);
  thd_new(&thread_b, thread_entry_func, (void *) &mutex_a);

  thd_join(&thread_a);
  thd_join(&thread_b);
  thd_delete(&thread_a);
  thd_delete(&thread_b);
  mtx_delete(&mutex_a);

  log_printf(g_log_o, "g_count: %u\n", g_count);
  
  libstash_shutdown();
  return 0;
}
