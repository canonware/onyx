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
 *
 *
 ****************************************************************************/

#include <libstash/libstash_r.h>

int
main()
{
  libstash_init();
  log_printf(cw_g_log, "Test begin\n");


  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
