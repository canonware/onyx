/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Test thread suspend/resume along with critical sections.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#include <time.h>

cw_thd_t	*thd;
cw_uint32_t	i;
cw_bool_t	done = FALSE;

void *
thread_entry_func(void *a_arg)
{
	for (i = 1; i != 0 && done == FALSE;) {
		thd_crit_enter();
		i++;
		thd_crit_leave();

		/*
		 * For non-concurrent thread libraries, don't use up the whole
		 * time quantum, in order to make this test complete faster.
		 */
		if (i % 509989 == 0)
			thd_yield();
	}

	return NULL;
}

int
main()
{
	cw_uint32_t	j, count;
	struct timespec	tout = {0, 1000000};
		
	libstash_init();
	out_put(out_err, "Test begin\n");

	thd_crit_enter();
	thd_crit_leave();

	thd = thd_new(thread_entry_func, NULL);
	nanosleep(&tout, NULL);

	out_put(out_err, "thd_suspend()\n");
	for (j = 0; j < 7; j++) {
		thd_suspend(thd);
		count = i;
		nanosleep(&tout, NULL);
/*  		out_put(out_err, "count([i]) == i([i])\n", count, i); */
		_cw_assert(count == i);
		thd_resume(thd);
		nanosleep(&tout, NULL);
		thd_suspend(thd);
/*  		out_put(out_err, "count([i]) != i([i])\n", count, i); */
		_cw_assert(count != i);
		thd_resume(thd);
	}
	
	out_put(out_err, "thd_trysuspend()\n");
	for (j = 0; j < 7; j++) {
		while (thd_trysuspend(thd));
		count = i;
		nanosleep(&tout, NULL);
/*  		out_put(out_err, "count([i]) == i([i])\n", count, i); */
		_cw_assert(count == i);
		thd_resume(thd);
		nanosleep(&tout, NULL);
		while (thd_trysuspend(thd));
/*  		out_put(out_err, "count([i]) != i([i])\n", count, i); */
		_cw_assert(count != i);
		thd_resume(thd);
	}
	done = TRUE;
	thd_join(thd);

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
