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

#include "../include/libstash/libstash.h"

cw_thd_t	*thread_a, *thread_b;

void *
thread_entry_func(void *a_arg)
{
	char	*arg_str = (char *)a_arg;

	out_put_e(out_err, NULL, 0, "thread_entry_func",
	    "Argument string: \"[s]\"\n", arg_str);

	out_put_e(out_err, NULL, 0, "thread_entry_func",
	    "thd_self() returns [s]\n", (thd_self() == thread_a) ? "thread_a" :
	    (thd_self() == thread_b) ? "thread_b" : "<error>");

	return NULL;
}

int
main()
{
	libstash_init();
	out_put(out_err, "Test begin\n");

	thread_b = NULL;
	thread_a = thd_new(thread_entry_func, (void *)"Thread A argument",
	    TRUE);
	_cw_assert(thd_join(thread_a) == NULL);
	thread_a = NULL;

	thread_b = thd_new(thread_entry_func, (void *)"Thread B argument",
	    TRUE);
	_cw_assert(thd_join(thread_b) == NULL);

	out_put(out_err, "Test end\n");
	libstash_shutdown();

	return 0;
}
