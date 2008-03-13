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

#define NSUSPENSIBLE 50
#define NSUSPENDERS 1
#define NTHREADS (NSUSPENSIBLE + NSUSPENDERS)
#define NSUSPENDS 25000

uint32_t count = 0;

void *
thread_entry_func_suspensible(void *a_arg)
{
    uint32_t i;

    while (count < NSUSPENDS)
    {
	for (i = 0; i < 100; i++)
	{
	    thd_yield();
	}

	thd_crit_enter();
	for (i = 0; i < 10; i++)
	{
	    thd_yield();
	}
	thd_crit_leave();
    }

    return NULL;
}

void *
thread_entry_func_suspender(void *a_arg)
{
    uint32_t i;

    thd_yield();

    for (i = 0; i < NSUSPENDS; i++)
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
    for (i = 0; i < NSUSPENDERS; i++)
    {
	thds[i] = thd_new(thread_entry_func_suspender, NULL, true);
    }
    /* Create threads that cannot be suspended. */
    for (; i < NTHREADS; i++)
    {
	thds[i] = thd_new(thread_entry_func_suspensible, NULL, false);
    }

    /* The initial thread is suspensible. */
    thread_entry_func_suspensible(NULL);

    for (i = 0; i < NTHREADS; i++)
    {
	thd_join(thds[i]);
    }

    cw_assert(count == NSUSPENDERS * NSUSPENDS);

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
