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
 * $Revision: 226 $
 * $Date: 1998-09-19 18:10:12 -0700 (Sat, 19 Sep 1998) $
 *
 * <<< Description >>>
 *
 * cnd test.
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#include <libstash.h>

#include <sys/time.h>
#include <string.h>

#define _STASH_TEST_NUM_THREADS 10

struct cw_foo_s
{
  cw_cnd_t * cond;
  cw_mtx_t * mutex;
};

void *
thread_entry_func(void * a_arg)
{
  struct cw_foo_s * foo_var = (struct cw_foo_s *) a_arg;

  cnd_wait(foo_var->cond, foo_var->mutex);

  log_eprintf(g_log_o, NULL, 0, "thread_entry_func",
	      "After cnd_wait() call.\n");

  mtx_unlock(foo_var->mutex);

  return NULL;
}

int
main()
{
  cw_cnd_t cond_a, * cond_b;
  cw_mtx_t mutex;
  cw_thd_t threads[_STASH_TEST_NUM_THREADS], thread;
  struct cw_foo_s foo_var;
  struct timeval now;
  struct timespec timeout;
  struct timezone tz;
  cw_uint32_t i;
  
  glob_new();

  mtx_new(&mutex);
  
  cond_b = cnd_new(NULL);
  _cw_check_ptr(cond_b);
  cnd_delete(cond_b);
  
  _cw_assert(&cond_a == cnd_new(&cond_a));
  /* These should do nothing. */
  cnd_signal(&cond_a);
  cnd_broadcast(&cond_a);

  /* Set timeout for 1 second. */
  bzero(&tz, sizeof(struct timezone));
  gettimeofday(&now, &tz);
  timeout.tv_sec = now.tv_sec + 1;
  timeout.tv_nsec = now.tv_usec * 1000;

  mtx_lock(&mutex);
  _cw_assert(TRUE == cnd_timedwait(&cond_a, &mutex, &timeout));
  mtx_unlock(&mutex);

  /* Create argument for thd_new(). */
  foo_var.cond = &cond_a;
  foo_var.mutex = &mutex;

  /* Test cnd_signal. */
  mtx_lock(&mutex);
  thd_new(&thread, thread_entry_func, (void *) &foo_var);
  
  mtx_lock(&mutex);
  cnd_signal(&cond_a);
  mtx_unlock(&mutex);
  thd_join(&thread);
  thd_delete(&thread);

  /* Test cnd_broadcast. */
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    mtx_lock(&mutex);
    thd_new(&threads[i], thread_entry_func, (void *) &foo_var);
  }

  mtx_lock(&mutex);
  cnd_broadcast(&cond_a);
  mtx_unlock(&mutex);

  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
    thd_delete(&threads[i]);
  }

  cnd_delete(&cond_a);
  mtx_delete(&mutex);
  
  glob_delete();
  return 0;
}
