/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * tsd test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#define NTHREADS	20
#define NITERATIONS	1000

cw_tsd_t	tsd;

void *
thread_entry_func(void *a_arg)
{
	cw_uint32_t	i;

	for (i = 0; i < NITERATIONS; i++) {
		tsd_set(&tsd, (void *)i);
		thd_yield();
		_cw_assert((cw_uint32_t)tsd_get(&tsd) == i);
		thd_yield();
	}

	return NULL;
}

int
main()
{
	cw_thd_t	thds[NTHREADS];
	cw_uint32_t	i;

	libstash_init();
	_cw_out_put("Test begin\n");

	tsd_new(&tsd, NULL);

	for (i = 0; i < NTHREADS; i++)
		thd_new(&thds[i], thread_entry_func, NULL);
	
	for (i = 0; i < NTHREADS; i++)
		thd_join(&thds[i]);

	tsd_delete(&tsd);

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
