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

  out_put(cw_g_out, "Hello world [t:s|name:value|w:0] blah", "hi");
  out_put(cw_g_out, "Oog[name:value|t:string|w:0] blah", "hi");
  out_put(cw_g_out, "Boo[name:value|t:string]", "hoo");
  out_put(cw_g_out, "[[ A bracket [[ ]", "hi");

  out_put(cw_g_out, "Boo[name:value|t:buf|s:blah]", "hoo");
  out_put(cw_g_out, "Boo[name:value|t:unknown|s:blah]", "hoo");

  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
