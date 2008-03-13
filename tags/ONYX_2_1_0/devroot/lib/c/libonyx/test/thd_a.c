/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * thd test.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

cw_thd_t	*thread_a, *thread_b;

void *
thread_entry_func(void *a_arg)
{
	char	*arg_str = (char *)a_arg;

	fprintf(stderr, "%s(): Argument string: \"%s\"\n", __FUNCTION__,
	    arg_str);

	fprintf(stderr, "%s(): thd_self() returns %s\n", __FUNCTION__,
	    (thd_self() == thread_a) ? "thread_a" : (thd_self() == thread_b) ?
	    "thread_b" : "<error>");

	return NULL;
}

int
main()
{
	libonyx_init();
	fprintf(stderr, "Test begin\n");

	thread_b = NULL;
	thread_a = thd_new(thread_entry_func, (void *)"Thread A argument",
	    TRUE);
	_cw_assert(thd_join(thread_a) == NULL);
	thread_a = NULL;

	thread_b = thd_new(thread_entry_func, (void *)"Thread B argument",
	    TRUE);
	_cw_assert(thd_join(thread_b) == NULL);

	fprintf(stderr, "Test end\n");
	libonyx_shutdown();

	return 0;
}
