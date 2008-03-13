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
 * Test thread suspend/resume along with critical sections.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#include <time.h>

cw_thd_t *thd;
uint32_t i;
bool done = false;

void *
thread_entry_func(void *a_arg)
{
    for (i = 1; i != 0 && done == false;)
    {
	thd_crit_enter();
	i++;
	thd_crit_leave();

	/* For non-concurrent thread libraries, don't use up the whole time
	 * quantum, in order to make this test complete faster. */
	if (i % 509989 == 0)
	{
	    thd_yield();
	}
    }

    return NULL;
}

int
main()
{
    uint32_t j, count;
    struct timespec tout = {0, 1000000};

    libonyx_init(0, NULL, NULL);
    fprintf(stderr, "Test begin\n");

    thd_crit_enter();
    thd_crit_leave();

    thd = thd_new(thread_entry_func, NULL, true);
    nanosleep(&tout, NULL);

    fprintf(stderr, "thd_suspend()\n");
    for (j = 0; j < 7; j++)
    {
	thd_suspend(thd);
	count = i;
	nanosleep(&tout, NULL);
/* 	fprintf(stderr, "count(%u) == i(%u)\n", count, i); */
	cw_assert(count == i);
	thd_resume(thd);
	nanosleep(&tout, NULL);
	thd_suspend(thd);
/* 	fprintf(stderr, "count(%u) <= i(%u)\n", count, i); */
	cw_assert(count <= i);
	thd_resume(thd);
    }

    fprintf(stderr, "thd_trysuspend()\n");
    for (j = 0; j < 7; j++)
    {
	while (thd_trysuspend(thd));
	{
	    count = i;
	}
	nanosleep(&tout, NULL);
/* 	fprintf(stderr, "count(%u) == i(%u)\n", count, i); */
	cw_assert(count == i);
	thd_resume(thd);
	nanosleep(&tout, NULL);
	while (thd_trysuspend(thd))
	{
	}
/* 	fprintf(stderr, "count(%u) <= i(%u)\n", count, i); */
	cw_assert(count <= i);
	thd_resume(thd);
    }
    done = true;
    thd_join(thd);

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
