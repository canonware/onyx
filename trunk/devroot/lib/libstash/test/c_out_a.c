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
 * Test the out (printf-alike) class.
 *
 ****************************************************************************/

#include <libstash/libstash_r.h>

int
main()
{
  char buf[37];
  
  libstash_init();
  log_printf(cw_g_log, "Test begin\n");

/*    dbg_register(cw_g_dbg, "mem_error"); */
/*    dbg_register(cw_g_dbg, "mem_verbose"); */

/*    out_put(cw_g_out, "Hello world [s|name:value|w:0] blah", "hi"); */
/*    out_put(cw_g_out, "Oog[string|name:value|w:0] blah", "hi"); */
  out_put(cw_g_out, "Boo[string|name:value]", "hoo");
  out_put(cw_g_out, "[[ A bracket [[ ][s]\n", "hi");
/*    out_put(cw_g_out, "Boo[buf|name:value|s:blah]", "hoo"); */
  
/*    out_put(cw_g_out, "[i32] [s]]][[[i64|w:3]\n", 64, ":Hi:", 32); */
  
  bzero(buf, 37);
  _cw_assert(18 == out_put_sn(cw_g_out,
			      buf,
			      18,
			      "[i32] [s]]][[[i64|w:3]",
			      64,
			      "Hi"
			      ));

  out_put(cw_g_out, "[i32]\n", 123);
  out_put(cw_g_out, "[i32|b:16]\n", 123);
  
  out_put(cw_g_out, ":123456789: --> :[i32]:\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:13]:_13\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:16]:_16\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:36]:_36\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:8]:_8\n", 123456789);

  out_put(cw_g_out, ":123456789: --> :[i32|b:2]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:c]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:l]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:l|p:_]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:c|p:-]:_2\n", 123456789);
  out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:r|p:+]:_2\n", 123456789);

  out_put(cw_g_out, ":[i32]:\n", 123);
  out_put(cw_g_out, ":[i32|+:+]:\n", 123);
  out_put(cw_g_out, ":[i32|s:u|+:+]:\n", 123);
  out_put(cw_g_out, ":[i32|s:s|+:+]:\n", -123);

  out_put(cw_g_out, ":[i32|s:s|+:+|w:2]:\n", -123);
  out_put(cw_g_out, ":[i32|s:s|+:+|w:8]:\n", -123);
  out_put(cw_g_out, ":[i32|s:s|+:+|w:8|j:l]:\n", -123);
  out_put(cw_g_out, ":[i32|s:s|+:+|w:8|j:c]:\n", -123);
  out_put(cw_g_out, ":[i32|s:s|+:+|w:8|j:r]:\n", -123);

  out_put(cw_g_out, ":[i8]:\n", 43);
  out_put(cw_g_out, ":[i8|s:u]:\n", 43);
  out_put(cw_g_out, ":[i8|s:s]:\n", -43);
  out_put(cw_g_out, ":[i8|s:u|b:2|p:0|w:8]: [[i8|s:u|b:2|p:0|w:8]\n", 43);
  out_put(cw_g_out, ":[i8|s:u|b:2|p:0|w:8]: [[i8|s:u|b:2|p:0|w:8]\n", -43);
  out_put(cw_g_out, ":[i8|s:s|b:2]:\n", -43);
  
  out_put(cw_g_out, ":[i16]:\n", 43);
  out_put(cw_g_out, ":[i16|s:u]:\n", 43);
  out_put(cw_g_out, ":[i16|s:s]:\n", -43);
  out_put(cw_g_out, ":[i16|s:u|b:2|p:0|w:16]: [[i16|s:u|b:2|p:0|w:16]\n", 43);
  out_put(cw_g_out, ":[i16|s:u|b:2|p:0|w:16]: [[i16|s:u|b:2|p:0|w:16]\n", -43);
  out_put(cw_g_out, ":[i16|s:s|b:2]:\n", -43);

  {
    cw_uint32_t i;
    char buf[65];
    
    for (i = 0; i < 1; i++)
    {
      out_put_s(cw_g_out, buf, "[i32|b:16]", (cw_uint32_t) 0xf2135123);
      out_put(cw_g_out, ".");
    }
    out_put(cw_g_out, "\n[s]\n", buf);
  }

  out_put(cw_g_out, ":[c]:\n", 'A');
  out_put(cw_g_out, ":[c|w:3]:\n", 'A');
  out_put(cw_g_out, ":[c|w:3|j:l]:\n", 'A');
  out_put(cw_g_out, ":[c|w:3|j:c]:\n", 'A');
  out_put(cw_g_out, ":[c|w:3|j:r]:\n", 'A');
  out_put(cw_g_out, ":[c|w:3|j:l|p:_]:\n", 'A');
  out_put(cw_g_out, ":[c|w:3|j:c|p:+]:\n", 'A');
  out_put(cw_g_out, ":[c|w:3|j:r|p:-]:\n", 'A');

  out_put(cw_g_out, "0x:[p]:\n", 0x12345678);
  out_put(cw_g_out, "0x:[p|w:12|p:_|j:c]:\n", 0x12345678);

  out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
	    "Extended\n");
  
/*    out_put_fle(cw_g_out, 2, __FILE__, __LINE__, __FUNCTION__, */
/*  	      "Yo, timestamped\n"); */
  
  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
