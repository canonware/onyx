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
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#define NSUSPENDIBLE 10
#define NSUSPENDERS 10
#define NTHREADS (NSUSPENDIBLE + NSUSPENDERS)
#define NITERATIONS 100

cw_uint32_t count = 0;

void *
thread_entry_func(void *a_arg)
{
    cw_uint32_t i;

    for (i = 0; i < NITERATIONS; i++)
    {
	thd_single_enter();
	count++;
	thd_single_leave();
    }

    return NULL;
}

int
main()
{
    cw_thd_t *thds[NTHREADS];
    cw_uint32_t i;

    libonyx_init(0, NULL, NULL);
    fprintf(stderr, "Test begin\n");

    /* Create threads that can be suspended. */
    for (i = 0; i < NSUSPENDIBLE; i++)
    {
	thds[i] = thd_new(thread_entry_func, NULL, TRUE);
    }
    /* Create threads that cannot be suspended. */
    for (; i < NTHREADS; i++)
    {
	thds[i] = thd_new(thread_entry_func, NULL, FALSE);
    }

    /* The initial thread is suspendible. */
    thread_entry_func(NULL);

    for (i = 0; i < NTHREADS; i++)
    {
	thd_join(thds[i]);
    }

    cw_assert(count == (NTHREADS + 1) * NITERATIONS);

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
