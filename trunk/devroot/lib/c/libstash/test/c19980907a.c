/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
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

#define _INC_GLOB_H_
#include <libstash.h>

void *
thread_entry_func(void * a_arg)
{
  char * arg_str = (char *) a_arg;

  log_eprintf(g_log_o, NULL, 0, "thread_entry_func",
	      "Argument string: \"%s\"\n", arg_str);

  return NULL;
}

int
main()
{
  cw_thd_t * thread_a, thread_b;
  
  glob_new();

  thread_a = thd_new(NULL, thread_entry_func, (void *) "Thread A argument");
  _cw_assert(NULL == thd_join(thread_a));
  thd_delete(thread_a);

  _cw_assert(&thread_b == thd_new(&thread_b, thread_entry_func,
				  (void *) "Thread B argument"));
  _cw_assert(NULL == thd_join(&thread_b));
  thd_delete(&thread_b);

  glob_delete();

  return 0;
}
