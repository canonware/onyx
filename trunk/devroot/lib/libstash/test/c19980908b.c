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
 * $Revision: 214 $
 * $Date: 1998-09-08 20:23:54 -0700 (Tue, 08 Sep 1998) $
 *
 * <<< Description >>>
 *
 * lwq test.
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#include <libstash.h>

#define _STASH_TEST_COUNT 50
#define _STASH_TEST_NUM_THREADS 20

cw_uint32_t g_count = 0;

struct foo_s
{
  cw_uint32_t tid;
  cw_lwq_t * lock;
};

void *
thread_queue_entry_func(void * a_arg)
{
  struct foo_s * foo_var = (struct foo_s *) a_arg;

  log_printf(g_log_o, "Thread %u about to call lwq_lock()\n",
	     foo_var->tid);
  lwq_lock(foo_var->lock);
  log_printf(g_log_o, "Thread %u has lock\n", foo_var->tid);
  lwq_unlock(foo_var->lock);

  _cw_free(foo_var);
  
  return NULL;
}

void *
thread_entry_func(void * a_arg)
{
  cw_uint32_t i, temp;
  cw_lwq_t * lock = (cw_lwq_t *) a_arg;

  for (i = 0; i < _STASH_TEST_COUNT; i++)
  {
    lwq_lock(lock);
    temp = g_count;
    usleep(1);
    temp++;
    g_count = temp;
    lwq_unlock(lock);
  }

  return NULL;
}

int
main()
{
  cw_thd_t thread_a, thread_b, threads[_STASH_TEST_NUM_THREADS];
  cw_lwq_t lock_a, * lock_b;
  struct foo_s * foo_var;
  cw_uint32_t i;
  
  glob_new();

  _cw_assert(&lock_a == lwq_new(&lock_a));
  /* Unlocked. */
  lwq_lock(&lock_a);
  /* Locked. */
  lwq_unlock(&lock_a);
  /* Unlocked. */
  lwq_delete(&lock_a);

  lock_b = lwq_new(NULL);
  lwq_lock(lock_b);
  lwq_unlock(lock_b);
  lwq_delete(lock_b);

  /* See if the lwq actually locks other threads out of critical
   * sections. */
  lwq_new(&lock_a);

  thd_new(&thread_a, thread_entry_func, (void *) &lock_a);
  thd_new(&thread_b, thread_entry_func, (void *) &lock_a);

  thd_join(&thread_a);
  thd_join(&thread_b);
  thd_delete(&thread_a);
  thd_delete(&thread_b);
  lwq_delete(&lock_a);

  log_printf(g_log_o, "g_count: %u\n", g_count);

  lwq_new(&lock_a);
  
  lwq_lock(&lock_a);
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    foo_var = (struct foo_s *) _cw_malloc(sizeof(struct foo_s));
    foo_var->tid = i;
    foo_var->lock = &lock_a;
    thd_new(&threads[i], thread_queue_entry_func, (void *) foo_var);

    /* Wait until we know that the new thread has blocked on the lock. */
    while (lwq_num_waiters(&lock_a) <= i)
    {
    }
  }
  lwq_unlock(&lock_a);
  
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
    thd_delete(&threads[i]);
  }
  
  lwq_delete(&lock_a);
  
  glob_delete();
  return 0;
}
