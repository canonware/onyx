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

cw_thd_t	thd;
cw_uint32_t	i;

void *
thread_entry_func(void *a_arg)
{
	for (i = 1; i != 0;) {
		thd_crit_enter(&thd);
		i++;
		thd_crit_leave(&thd);
		if (i % 509989 == 0)
			thd_yield();
	}

	return NULL;
}

int
main()
{
	cw_uint32_t	j, count;

	libstash_init();
	_cw_out_put("Test begin\n");

	thd_new(&thd, thread_entry_func, NULL);
	thd_yield();

	_cw_out_put("thd_suspend()\n");
	for (j = 0; j < 7; j++) {
		thd_suspend(&thd);
		count = i;
		thd_yield();
/*  		_cw_out_put("count([i]) == i([i])\n", count, i); */
		_cw_assert(count == i);
		thd_resume(&thd);
		thd_yield();
		thd_suspend(&thd);
/*  		_cw_out_put("count([i]) != i([i])\n", count, i); */
		_cw_assert(count != i);
		thd_resume(&thd);
	}
	
	_cw_out_put("thd_trysuspend()\n");
	for (j = 0; j < 7; j++) {
		while (thd_trysuspend(&thd));
		count = i;
		thd_yield();
/*  		_cw_out_put("count([i]) == i([i])\n", count, i); */
		_cw_assert(count == i);
		thd_resume(&thd);
		thd_yield();
		while (thd_trysuspend(&thd));
/*  		_cw_out_put("count([i]) != i([i])\n", count, i); */
		_cw_assert(count != i);
		thd_resume(&thd);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
