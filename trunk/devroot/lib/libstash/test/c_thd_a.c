/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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

#include <libstash/libstash_r.h>

void *
thread_entry_func(void * a_arg)
{
  char * arg_str = (char *) a_arg;

  out_put_e(cw_g_out, NULL, 0, "thread_entry_func",
	    "Argument string: \"[s]\"\n", arg_str);

  return NULL;
}

int
main()
{
  cw_thd_t * thread_a, thread_b;
  
  libstash_init();
  out_put(cw_g_out, "Test begin\n");

  thread_a = thd_new(NULL, thread_entry_func, (void *) "Thread A argument");
  _cw_assert(NULL == thd_join(thread_a));

  _cw_assert(&thread_b == thd_new(&thread_b, thread_entry_func,
				  (void *) "Thread B argument"));
  _cw_assert(NULL == thd_join(&thread_b));

  out_put(cw_g_out, "Test end\n");
  libstash_shutdown();

  return 0;
}
