/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#define	NTHREADS	20
#define	NITERATIONS	200

cw_uint32_t	count = 0;

void *
thread_entry_func(void *a_arg)
{
	cw_uint32_t	i;

	for (i = 0; i < NITERATIONS; i++) {
		thd_single_enter();
		count++;
		thd_single_leave();
	}

	return NULL;
}

int
main()
{
	cw_thd_t	*thds[NTHREADS];
	cw_uint32_t	i;

	libstash_init();
	_cw_out_put("Test begin\n");

	for (i = 0; i < NTHREADS; i++)
		thds[i] = thd_new(thread_entry_func, NULL);

	thread_entry_func(NULL);

	for (i = 0; i < NTHREADS; i++)
		thd_join(thds[i]);

	_cw_assert(count == (NTHREADS + 1) * NITERATIONS);

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
