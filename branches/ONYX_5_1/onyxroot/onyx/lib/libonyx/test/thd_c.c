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

#define NSUSPENSIBLE 10
#define NSUSPENDERS 10
#define NTHREADS (NSUSPENSIBLE + NSUSPENDERS)
#define NITERATIONS 100

uint32_t count = 0;

void *
thread_entry_func(void *a_arg)
{
    uint32_t i;

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
    uint32_t i;

    libonyx_init(0, NULL, NULL);
    fprintf(stderr, "Test begin\n");

    /* Create threads that can be suspended. */
    for (i = 0; i < NSUSPENSIBLE; i++)
    {
	thds[i] = thd_new(thread_entry_func, NULL, true);
    }
    /* Create threads that cannot be suspended. */
    for (; i < NTHREADS; i++)
    {
	thds[i] = thd_new(thread_entry_func, NULL, false);
    }

    /* The initial thread is suspensible. */
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