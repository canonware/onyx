/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * rwl test.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#define _LIBSTASH_TEST_NUM_THREADS 20

void *
thread_entry_func(void *a_arg)
{
	cw_rwl_t	*lock = (cw_rwl_t *)a_arg;

	rwl_wlock(lock);
	out_put_e(out_err, NULL, 0, __FUNCTION__, "Got wlock\n");
	rwl_wunlock(lock);

	return NULL;
}

int
main()
{
	cw_thd_t	*threads[_LIBSTASH_TEST_NUM_THREADS];
	cw_rwl_t	lock_a, lock_b;
	cw_uint32_t	i;

	libstash_init();
	out_put(out_err, "Test begin\n");

	rwl_new(&lock_b);
	rwl_rlock(&lock_b);
	rwl_rlock(&lock_b);
	rwl_runlock(&lock_b);
	rwl_runlock(&lock_b);
	rwl_wlock(&lock_b);
	rwl_wunlock(&lock_b);
	rwl_delete(&lock_b);

	rwl_new(&lock_a);
	rwl_rlock(&lock_a);
	rwl_rlock(&lock_a);
	rwl_rlock(&lock_a);
	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		threads[i] = thd_new(thread_entry_func, (void *)&lock_a);
	out_put_e(out_err, NULL, 0, "main", "About to release rlock\n");
	rwl_runlock(&lock_a);
	usleep(1);
	out_put_e(out_err, NULL, 0, "main", "About to release rlock\n");
	rwl_runlock(&lock_a);
	usleep(1);
	out_put_e(out_err, NULL, 0, "main", "About to release rlock\n");
	rwl_runlock(&lock_a);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_join(threads[i]);

	rwl_wlock(&lock_a);
	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		threads[i] = thd_new(thread_entry_func, (void *)&lock_a);
	out_put_e(out_err, NULL, 0, "main", "About to release wlock\n");
	usleep(1);
	rwl_wunlock(&lock_a);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_join(threads[i]);

	rwl_delete(&lock_a);

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
