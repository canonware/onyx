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

#define _LIBSTASH_USE_OH
#include <libstash/libstash_r.h>

int
main()
{
  libstash_init();
  log_printf(cw_g_log, "Test begin\n");

  /* oh_new(), oh_delete(). */
  {
  }
  

  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
