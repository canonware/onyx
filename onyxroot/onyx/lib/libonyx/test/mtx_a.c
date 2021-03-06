/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 * mtx test.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#define CW_TEST_COUNT 50

uint32_t g_count = 0;

void *
thread_entry_func(void *a_arg)
{
    uint32_t i, temp;
    cw_mtx_t *mutex = (cw_mtx_t *) a_arg;

    for (i = 0; i < CW_TEST_COUNT; i++)
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
    cw_thd_t *thread_a, *thread_b;
    cw_mtx_t mutex_a, mutex_b;

    libonyx_init(0, NULL, NULL);
    fprintf(stderr, "Test begin\n");

    mtx_new(&mutex_a);
    /* Unlocked. */
    mtx_lock(&mutex_a);
    /* Locked. */
    mtx_unlock(&mutex_a);
    /* Unlocked. */
    cw_assert(mtx_trylock(&mutex_a) == false);
    /* Locked. */
#ifndef CW_PTH
    /* pth only has recursive mutexes, so don't check for failure of recursive
     * locks when using pth. */
    cw_assert(mtx_trylock(&mutex_a));
#endif
    mtx_unlock(&mutex_a);
    /* Unlocked. */
    cw_assert(mtx_trylock(&mutex_a) == false);
    /* Locked. */
    mtx_unlock(&mutex_a);
    /* Unlocked. */
    mtx_delete(&mutex_a);

    mtx_new(&mutex_b);
    mtx_lock(&mutex_b);
#ifndef CW_PTH
    /* pth only has recursive mutexes, so don't check for failure of recursive
     * locks when using pth. */
    cw_assert(mtx_trylock(&mutex_b));
#endif
    mtx_unlock(&mutex_b);
    mtx_delete(&mutex_b);

    /* See if the mutex actually locks other threads out of critical
     * sections. */
    mtx_new(&mutex_a);

    thread_a = thd_new(thread_entry_func, (void *) &mutex_a, true);
    thread_b = thd_new(thread_entry_func, (void *) &mutex_a, true);

    thd_join(thread_a);
    thd_join(thread_b);
    mtx_delete(&mutex_a);

    fprintf(stderr, "g_count: %u\n", g_count);

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
