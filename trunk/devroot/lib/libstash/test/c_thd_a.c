/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * thd test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

void *
thread_entry_func(void *a_arg)
{
	char	*arg_str = (char *)a_arg;

	out_put_e(cw_g_out, NULL, 0, "thread_entry_func",
	    "Argument string: \"[s]\"\n", arg_str);

	return NULL;
}

int
main()
{
	cw_thd_t	thread_a, thread_b;

	libstash_init();
	_cw_out_put("Test begin\n");

	thd_new(&thread_a, thread_entry_func, (void *)"Thread A argument");
	_cw_assert(thd_join(&thread_a) == NULL);

	thd_new(&thread_b, thread_entry_func, (void *)"Thread B argument");
	_cw_assert(thd_join(&thread_b) == NULL);

	_cw_out_put("Test end\n");
	libstash_shutdown();

	return 0;
}
