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
 * cnd test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#define _LIBSTASH_TEST_NUM_THREADS 10

struct cw_foo_s {
	cw_uint32_t	*num_waiting;
	cw_cnd_t	*cond;
	cw_mtx_t	*mutex;
};

void *
thread_entry_func(void *a_arg)
{
	struct cw_foo_s	*foo_var = (struct cw_foo_s *)a_arg;

	mtx_lock(foo_var->mutex);
	(*foo_var->num_waiting)++;

	cnd_wait(foo_var->cond, foo_var->mutex);

	out_put_e(cw_g_out, NULL, 0, "thread_entry_func",
	    "After cnd_wait() call\n");

	mtx_unlock(foo_var->mutex);

	return NULL;
}

int
main()
{
	cw_cnd_t	cond;
	cw_mtx_t	mutex;
	cw_thd_t	threads[_LIBSTASH_TEST_NUM_THREADS], thread;
	struct cw_foo_s	foo_var;
	struct timespec	timeout;
	cw_uint32_t	i;
	cw_bool_t	num_waiting;

	libstash_init();
	_cw_out_put("Test begin\n");

	mtx_new(&mutex);

	cnd_new(&cond);
	/* These should do nothing. */
	cnd_signal(&cond);
	cnd_broadcast(&cond);

	/* Set timeout for 1 second. */
	timeout.tv_sec = 1;
	timeout.tv_nsec = 0;

	mtx_lock(&mutex);
	_cw_assert(cnd_timedwait(&cond, &mutex, &timeout));
	mtx_unlock(&mutex);

	/* Create argument for thd_new(). */
	num_waiting = 0;
	foo_var.num_waiting = &num_waiting;
	foo_var.cond = &cond;
	foo_var.mutex = &mutex;

	/* Test cnd_signal. */
	thd_new(&thread, thread_entry_func, (void *)&foo_var);

	/* Bad programming practice, but it works for this test. */
	mtx_lock(&mutex);
	while (num_waiting < 1) {
		mtx_unlock(&mutex);
		usleep(10000);
		mtx_lock(&mutex);
	}

	cnd_signal(&cond);
	mtx_unlock(&mutex);
	thd_join(&thread);

	/* Test cnd_broadcast. */
	num_waiting = 0;
	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_new(&threads[i], thread_entry_func, (void *)&foo_var);

	/* Bad programming practice, but it works for this test. */
	mtx_lock(&mutex);
	while (num_waiting < _LIBSTASH_TEST_NUM_THREADS) {
		mtx_unlock(&mutex);
		usleep(10000);
		mtx_lock(&mutex);
	}

	cnd_broadcast(&cond);
	mtx_unlock(&mutex);

	for (i = 0; i < _LIBSTASH_TEST_NUM_THREADS; i++)
		thd_join(&threads[i]);

	cnd_delete(&cond);
	mtx_delete(&mutex);

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
