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
/*    char buf[37]; */
  
  libstash_init();
  log_printf(cw_g_log, "Test begin\n");

/*    dbg_register(cw_g_dbg, "mem_error"); */
/*    dbg_register(cw_g_dbg, "mem_verbose"); */

/*    out_put(cw_g_out, "Hello world [t:s|name:value|w:0] blah", "hi"); */
/*    out_put(cw_g_out, "Oog[name:value|t:string|w:0] blah", "hi"); */
/*    out_put(cw_g_out, "Boo[name:value|t:string]", "hoo"); */
/*    out_put(cw_g_out, "[[ A bracket [[ ]", "hi"); */
/*    out_put(cw_g_out, "Boo[name:value|t:buf|s:blah]", "hoo"); */
/*    out_put(cw_g_out, "Boo[name:value|t:unknown|s:blah]", "hoo"); */
/*    out_put(cw_g_out, "[t:s]"); */
  
/*    out_put(cw_g_out, "[t:i32] [t:s]]][[[w:3|t:i64]\n", 64, ":Hi:", 32); */
  
/*    bzero(buf, 37); */
/*    _cw_assert(18 == out_put_sn(cw_g_out, */
/*  			      buf, */
/*  			      18, */
/*  			      "[t:i32] [t:s]]][[[w:3|t:i64]", */
/*  			      64, */
/*  			      "Hi" */
/*  			      )); */

/*    out_put(cw_g_out, "[t:i32]\n", 123); */
/*    out_put(cw_g_out, "[t:i32|b:16]\n", 123); */
  
  out_put(cw_g_out, ":123456789: --> :[t:i32]:\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[t:i32|b:13]:_13\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[t:i32|b:16]:_16\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[t:i32|b:36]:_36\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[t:i32|b:8]:_8\n", 123456789);

  out_put(cw_g_out, ":123456789: --> :[t:i32|b:2]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[t:i32|b:2|w:40]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[t:i32|b:2|w:40|j:c]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[t:i32|b:2|w:40|j:l]:_2\n", 123456789);

  out_put(cw_g_out, ":[t:i32]:\n", 123);
  out_put(cw_g_out, ":[t:i32|+:+]:\n", 123);
  out_put(cw_g_out, ":[t:i32|s:u|+:+]:\n", 123);
  out_put(cw_g_out, ":[t:i32|s:s|+:+]:\n", -123);

  out_put(cw_g_out, ":[t:i32|s:s|+:+|w:2]:\n", -123);
  out_put(cw_g_out, ":[t:i32|s:s|+:+|w:8]:\n", -123);
  out_put(cw_g_out, ":[t:i32|s:s|+:+|w:8|j:l]:\n", -123);
  out_put(cw_g_out, ":[t:i32|s:s|+:+|w:8|j:c]:\n", -123);
  out_put(cw_g_out, ":[t:i32|s:s|+:+|w:8|j:r]:\n", -123);
  
  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
