/****************************************************************************
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
 * sem test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#define _LIBSTASH_TEST_NUM_THREADS 10

void *
thread_entry_func(void *a_arg)
{
	cw_sem_t	*sem = (cw_sem_t *)a_arg;

	sem_wait(sem);
	_cw_out_put("Got semaphore.\n");

	return NULL;
}

int
main()
{
	cw_sem_t	sem_a, sem_b;
	cw_thd_t	threads[_LIBSTASH_TEST_NUM_THREADS];
	cw_uint32_t	i;
	struct timespec	timeout;

	libstash_init();
	_cw_out_put("Test begin\n");

	sem_new(&sem_b, 0);
	_cw_assert(sem_getvalue(&sem_b) == 0);
	sem_post(&sem_b);
	_cw_assert(sem_getvalue(&sem_b) == 1);
	sem_adjust(&sem_b, -2);
	_cw_assert(sem_getvalue(&sem_b) == -1);
	sem_post(&sem_b);
	_cw_assert(sem_trywait(&sem_b));
	sem_post(&sem_b);
	_cw_assert(sem_trywait(&sem_b) == FALSE);
	sem_post(&sem_b);
	sem_wait(&sem_b);
	sem_post(&sem_b);

	/* Set timeout for 2 seconds. */
	timeout.tv_sec = 2;
	timeout.tv_nsec = 0;

	_cw_assert(sem_timedwait(&sem_b, &timeout) == FALSE);
	_cw_assert(sem_timedwait(&sem_b, &timeout));
	sem_delete(&sem_b);

	sem_new(&sem_a, 0);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_new(&threads[i], thread_entry_func, (void *)&sem_a);

	sem_adjust(&sem_a, _LIBSTASH_TEST_NUM_THREADS);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_join(&threads[i]);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_new(&threads[i], thread_entry_func, (void *)&sem_a);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		sem_post(&sem_a);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_join(&threads[i]);

	sem_delete(&sem_a);

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
