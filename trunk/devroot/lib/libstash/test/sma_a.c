/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * sma test.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#define _CW_TEST_NUM_THREADS 10

void *
thread_entry_func(void *a_arg)
{
	cw_sma_t	*sma = (cw_sma_t *)a_arg;

	sma_wait(sma);
	out_put(out_err, "Got semaphore.\n");

	return NULL;
}

int
main()
{
	cw_sma_t	sma_a, sma_b;
	cw_thd_t	*threads[_CW_TEST_NUM_THREADS];
	cw_uint32_t	i;
	struct timespec	timeout;

	libstash_init();
	out_put(out_err, "Test begin\n");

	sma_new(&sma_b, 0);
	_cw_assert(sma_getvalue(&sma_b) == 0);
	sma_post(&sma_b);
	_cw_assert(sma_getvalue(&sma_b) == 1);
	sma_adjust(&sma_b, -2);
	_cw_assert(sma_getvalue(&sma_b) == -1);
	sma_post(&sma_b);
	_cw_assert(sma_trywait(&sma_b));
	sma_post(&sma_b);
	_cw_assert(sma_trywait(&sma_b) == FALSE);
	sma_post(&sma_b);
	sma_wait(&sma_b);
	sma_post(&sma_b);

	/* Set timeout for 2 seconds. */
	timeout.tv_sec = 2;
	timeout.tv_nsec = 0;

	_cw_assert(sma_timedwait(&sma_b, &timeout) == FALSE);
	_cw_assert(sma_timedwait(&sma_b, &timeout));
	sma_delete(&sma_b);

	sma_new(&sma_a, 0);

	for (i = 0; i < _CW_TEST_NUM_THREADS; i++)
		threads[i] = thd_new(thread_entry_func, (void *)&sma_a, TRUE);

	sma_adjust(&sma_a, _CW_TEST_NUM_THREADS);

	for (i = 0; i < _CW_TEST_NUM_THREADS; i++)
		thd_join(threads[i]);

	for (i = 0; i < _CW_TEST_NUM_THREADS; i++)
		threads[i] = thd_new(thread_entry_func, (void *)&sma_a, TRUE);

	for (i = 0; i < _CW_TEST_NUM_THREADS; i++)
		sma_post(&sma_a);

	for (i = 0; i < _CW_TEST_NUM_THREADS; i++)
		thd_join(threads[i]);

	sma_delete(&sma_a);

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
