/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * sema test.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#define _LIBSTASH_TEST_NUM_THREADS 10

void *
thread_entry_func(void *a_arg)
{
	cw_sema_t	*sema = (cw_sema_t *)a_arg;

	sema_wait(sema);
	out_put(out_err, "Got semaphore.\n");

	return NULL;
}

int
main()
{
	cw_sema_t	sema_a, sema_b;
	cw_thd_t	*threads[_LIBSTASH_TEST_NUM_THREADS];
	cw_uint32_t	i;
	struct timespec	timeout;

	libstash_init();
	out_put(out_err, "Test begin\n");

	sema_new(&sema_b, 0);
	_cw_assert(sema_getvalue(&sema_b) == 0);
	sema_post(&sema_b);
	_cw_assert(sema_getvalue(&sema_b) == 1);
	sema_adjust(&sema_b, -2);
	_cw_assert(sema_getvalue(&sema_b) == -1);
	sema_post(&sema_b);
	_cw_assert(sema_trywait(&sema_b));
	sema_post(&sema_b);
	_cw_assert(sema_trywait(&sema_b) == FALSE);
	sema_post(&sema_b);
	sema_wait(&sema_b);
	sema_post(&sema_b);

	/* Set timeout for 2 seconds. */
	timeout.tv_sec = 2;
	timeout.tv_nsec = 0;

	_cw_assert(sema_timedwait(&sema_b, &timeout) == FALSE);
	_cw_assert(sema_timedwait(&sema_b, &timeout));
	sema_delete(&sema_b);

	sema_new(&sema_a, 0);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		threads[i] = thd_new(thread_entry_func, (void *)&sema_a);

	sema_adjust(&sema_a, _LIBSTASH_TEST_NUM_THREADS);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_join(threads[i]);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		threads[i] = thd_new(thread_entry_func, (void *)&sema_a);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		sema_post(&sema_a);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_join(threads[i]);

	sema_delete(&sema_a);

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
