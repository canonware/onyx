/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Test thread suspend/resume along with critical sections.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#include <time.h>

cw_thd_t	thd;
cw_uint32_t	i;

void *
thread_entry_func(void *a_arg)
{
	for (i = 1; i != 0;) {
		thd_crit_enter(&thd);
		i++;
		thd_crit_leave(&thd);

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
	_cw_out_put("Test begin\n");

	thd_new(&thd, thread_entry_func, NULL);
	nanosleep(&tout, NULL);

	_cw_out_put("thd_suspend()\n");
	for (j = 0; j < 7; j++) {
		thd_suspend(&thd);
		count = i;
		nanosleep(&tout, NULL);
/*  		_cw_out_put("count([i]) == i([i])\n", count, i); */
		_cw_assert(count == i);
		thd_resume(&thd);
		nanosleep(&tout, NULL);
		thd_suspend(&thd);
/*  		_cw_out_put("count([i]) != i([i])\n", count, i); */
		_cw_assert(count != i);
		thd_resume(&thd);
	}
	
	_cw_out_put("thd_trysuspend()\n");
	for (j = 0; j < 7; j++) {
		while (thd_trysuspend(&thd));
		count = i;
		nanosleep(&tout, NULL);
/*  		_cw_out_put("count([i]) == i([i])\n", count, i); */
		_cw_assert(count == i);
		thd_resume(&thd);
		nanosleep(&tout, NULL);
		while (thd_trysuspend(&thd));
/*  		_cw_out_put("count([i]) != i([i])\n", count, i); */
		_cw_assert(count != i);
		thd_resume(&thd);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
