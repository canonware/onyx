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
 * sem test.
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#include <libstash.h>

#define _STASH_TEST_NUM_THREADS 10

void *
thread_entry_func(void * a_arg)
{
  cw_sem_t * sem = (cw_sem_t *) a_arg;

  sem_wait(sem);
  log_printf(g_log_o, "Got semaphore.\n");
  
  return NULL;
}

int
main()
{
  cw_sem_t sem_a, * sem_b;
  cw_thd_t threads[_STASH_TEST_NUM_THREADS];
  cw_uint32_t i;
  
  glob_new();

  sem_b = sem_new(NULL, 0);
  _cw_check_ptr(sem_b);
  _cw_assert(0 == sem_getvalue(sem_b));
  sem_post(sem_b);
  _cw_assert(1 == sem_getvalue(sem_b));
  sem_adjust(sem_b, -2);
  _cw_assert(-1 == sem_getvalue(sem_b));
  sem_post(sem_b);
  _cw_assert(TRUE == sem_trywait(sem_b));
  sem_post(sem_b);
  _cw_assert(FALSE == sem_trywait(sem_b));
  sem_post(sem_b);
  sem_wait(sem_b);
  sem_delete(sem_b);
  
  _cw_assert(&sem_a == sem_new(&sem_a, 0));

  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_new(&threads[i], thread_entry_func, (void *) &sem_a);
  }

  sem_adjust(&sem_a, _STASH_TEST_NUM_THREADS);
  
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
    thd_delete(&threads[i]);
  }
  
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_new(&threads[i], thread_entry_func, (void *) &sem_a);
  }

  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    sem_post(&sem_a);
  }
  
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_join(&threads[i]);
    thd_delete(&threads[i]);
  }
  
  sem_delete(&sem_a);

  glob_delete();
  return 0;
}
