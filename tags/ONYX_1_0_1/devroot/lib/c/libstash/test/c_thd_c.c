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

#define	NSUSPENDIBLE	20
#define	NSUSPENDERS	20
#define	NTHREADS	(NSUSPENDIBLE + NSUSPENDERS)
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
	out_put(out_err, "Test begin\n");

	/* Create threads that can be suspended. */
	for (i = 0; i < NSUSPENDIBLE; i++)
		thds[i] = thd_new(thread_entry_func, NULL, TRUE);
	/* Create threads that cannot be suspended. */
	for (; i < NTHREADS; i++)
		thds[i] = thd_new(thread_entry_func, NULL, FALSE);

	/* The initial thread is suspendible. */
	thread_entry_func(NULL);

	for (i = 0; i < NTHREADS; i++)
		thd_join(thds[i]);

	_cw_assert(count == (NTHREADS + 1) * NITERATIONS);

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
