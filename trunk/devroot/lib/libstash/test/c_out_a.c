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

#define _LIBSTASH_USE_BUF
#include <libstash/libstash_r.h>

int
main()
{
  libstash_init();
  log_printf(cw_g_log, "Test begin\n");

/*    dbg_register(cw_g_dbg, "mem_verbose"); */

  /* out_new(), out_delete(). */
  out_put(cw_g_out, "out_new(), out_delete()\n");
  {
    cw_out_t out, * out_p;

    _cw_assert(&out == out_new(&out));
    out_delete(&out);

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);
    out_delete(out_p);
  }

  /* out_register(), out_merge(). */
  out_put(cw_g_out, "out_register(), out_merge()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(FALSE == out_register(out_p, "buf", sizeof(cw_buf_t *),
				     buf_out_metric, buf_out_render));

    _cw_assert(FALSE == out_merge(out_p, cw_g_out));

    out_delete(out_p);
  }

  /* out_get_default_fd(), out_set_default_fd(). */
  out_put(cw_g_out, "out_get_default_fd(), out_set_default_fd()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(2 == out_get_default_fd(out_p));

    out_set_default_fd(out_p, 1);
    _cw_assert(1 == out_get_default_fd(out_p));

    out_delete(out_p);
  }

  /* out_put(). */
  out_put(cw_g_out, "out_put()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(16 == out_put(out_p, "16 Bytes output\n"));
    _cw_assert(0 < out_put(out_p, "[s]\n", "[s]"));
    _cw_assert(0 == out_put(out_p, ""));
    
    out_delete(out_p);
  }

  /* out_put_e(). */
  out_put(cw_g_out, "out_put_e()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(0 < out_put_e(out_p, "<file>", 42, "<function>",
			     "Extended output with filename and funcname\n"));
    _cw_assert(0 < out_put_e(out_p, "<file>", 42, NULL,
			     "Extended output with filename\n"));
    _cw_assert(0 < out_put_e(out_p, NULL, 42, "<function>",
			     "Extended output with funcname\n"));
    _cw_assert(0 < out_put_e(out_p, NULL, 42, NULL,
			     "Extended output, NULL args\n"));
    _cw_assert(0 == out_put_e(out_p, NULL, 42, NULL, ""));
    
    out_delete(out_p);
  }

  /* out_put_le(). */
  out_put(cw_g_out, "out_put_le()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(0 < out_put_le(out_p, "<file>", 42, "<function>",
			      "Timestamped output [s]\n", "[s]"));
    _cw_assert(0 < out_put_le(out_p, NULL, 42, NULL, ""));
    _cw_assert(0 < out_put(out_p, "\n"));
    
    out_delete(out_p);
  }

  /* out_put_n(). */
  out_put(cw_g_out, "out_put_n()\n");
  {
    cw_out_t * out_p;
    char buf[81] =
      "0123456789012345678901234567890123456789"
      "0123456789012345678901234567890123456789";

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(60 == out_put_n(out_p, 60, "[s]", buf));
    _cw_assert(2 == out_put_n(out_p, 2, ":[s]", "\n:"));
    _cw_assert(0 == out_put_n(out_p, 80, ""));
    
    out_delete(out_p);
  }

  /* out_put_f(). */
  out_put(cw_g_out, "out_put_f()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);
    
    _cw_assert(16 == out_put_f(out_p, 2, "16 Bytes output\n"));
    _cw_assert(0 < out_put_f(out_p, 2, "[s]\n", "[s]"));
    _cw_assert(0 == out_put_f(out_p, 2, ""));
    
    out_delete(out_p);
  }

  /* out_put_fe(). */
  out_put(cw_g_out, "out_put_fe()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);
    
    _cw_assert(0 < out_put_fe(out_p, 2, "<file>", 42, "<function>",
			      "Extended output with filename and funcname\n"));
    _cw_assert(0 < out_put_fe(out_p, 2, "<file>", 42, NULL,
			      "Extended output with filename\n"));
    _cw_assert(0 < out_put_fe(out_p, 2, NULL, 42, "<function>",
			      "Extended output with funcname\n"));
    _cw_assert(0 < out_put_fe(out_p, 2, NULL, 42, NULL,
			      "Extended output, NULL args\n"));
    _cw_assert(0 < out_put_fe(out_p, 2, NULL, 42, NULL,
			      "Extended output, NULL args, [s]\n",
			      "[s]"));
    _cw_assert(0 == out_put_fe(out_p, 2, NULL, 42, NULL, ""));
    
    out_delete(out_p);
  }

  /* out_put_fle(). */
  out_put(cw_g_out, "out_put_fle()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(0 < out_put_fle(out_p, 2, "<file>", 42, "<function>",
			       "Timestamped output [s]\n", "[s]"));
    _cw_assert(0 < out_put_fle(out_p, 2, NULL, 42, NULL, ""));
    
    out_delete(out_p);
  }

  /* out_put_fn(). */
  out_put(cw_g_out, "out_put_fn()\n");
  {
    cw_out_t * out_p;
    char buf[81] =
      "0123456789012345678901234567890123456789"
      "0123456789012345678901234567890123456789";

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(60 == out_put_fn(out_p, 2, 60, "[s]", buf));
    _cw_assert(2 == out_put_fn(out_p, 2, 2, ":[s]", "\n:"));
    _cw_assert(0 == out_put_fn(out_p, 2, 80, ""));
    
    out_delete(out_p);
  }

  /* out_put_s(). */
  out_put(cw_g_out, "out_put_s()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);
    
    out_delete(out_p);
  }

  /* out_put_sa(). */
  out_put(cw_g_out, "out_put_sa()\n");
  {
    cw_out_t * out_p;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);
    
    out_delete(out_p);
  }

  /* out_put_sn(). */
  out_put(cw_g_out, "out_put_sn()\n");
  {
    cw_out_t * out_p;
    char * str, buf[81], ibuf[81] =
      "........................................"
      "........................................";

    str = &buf[10];

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    /* All type strings. */
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i8]", 42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[int8]", 42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i16]", 42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[int16]", 42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32]", 42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[int32]", 42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i64]", (cw_uint64_t) 42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[int64]", (cw_uint64_t) 42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s]", "<string>"));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c]", 'c'));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[char]", 'c'));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s]", "<string>"));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[string]", "<string>"));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[p]", 0x42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[pointer]", 0x42));
    _cw_assert(80 == out_put(out_p, "[s]", buf));
    

    /* All combinations of specifier args. */
    /* All combinations of specifier types (make sure the va_list processing
     * works. */
    /* All type strings. */
    
    
    out_delete(out_p);
  }

/*    out_put(cw_g_out, "Hello world [s|name:value|w:0] blah", "hi"); */
/*    out_put(cw_g_out, "Oog[string|name:value|w:0] blah", "hi"); */
/*    out_put(cw_g_out, "Boo[string|name:value]", "hoo"); */
/*    out_put(cw_g_out, "[[ A bracket [[ ][s]\n", "hi"); */
  
/*    { */
/*      char buf[37]; */

/*      bzero(buf, 37); */
/*      _cw_assert(18 == out_put_sn(cw_g_out, */
/*  				buf, */
/*  				18, */
/*  				"[i32] [s]]][[[i64|w:3]", */
/*  				64, */
/*  				"Hi" */
/*  				)); */
/*    } */
  
/*    out_put(cw_g_out, "[i32]\n", 123); */
/*    out_put(cw_g_out, "[i32|b:16]\n", 123); */
  
/*    out_put(cw_g_out, ":123456789: --> :[i32]:\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:13]:_13\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:16]:_16\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:36]:_36\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:8]:_8\n", 123456789); */

/*    out_put(cw_g_out, ":123456789: --> :[i32|b:2]:_2\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40]:_2\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:c]:_2\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:l]:_2\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:l|p:_]:_2\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:c|p:-]:_2\n", 123456789); */
/*    out_put(cw_g_out, ":123456789: --> :[i32|b:2|w:40|j:r|p:+]:_2\n", 123456789); */

/*    out_put(cw_g_out, ":[i32]:\n", 123); */
/*    out_put(cw_g_out, ":[i32|+:+]:\n", 123); */
/*    out_put(cw_g_out, ":[i32|s:u|+:+]:\n", 123); */
/*    out_put(cw_g_out, ":[i32|s:s|+:+]:\n", -123); */

/*    out_put(cw_g_out, ":[i32|s:s|+:+|w:2]:\n", -123); */
/*    out_put(cw_g_out, ":[i32|s:s|+:+|w:8]:\n", -123); */
/*    out_put(cw_g_out, ":[i32|s:s|+:+|w:8|j:l]:\n", -123); */
/*    out_put(cw_g_out, ":[i32|s:s|+:+|w:8|j:c]:\n", -123); */
/*    out_put(cw_g_out, ":[i32|s:s|+:+|w:8|j:r]:\n", -123); */

/*    out_put(cw_g_out, ":[i8]:\n", 43); */
/*    out_put(cw_g_out, ":[i8|s:u]:\n", 43); */
/*    out_put(cw_g_out, ":[i8|s:s]:\n", -43); */
/*    out_put(cw_g_out, ":[i8|s:u|b:2|p:0|w:8]: [[i8|s:u|b:2|p:0|w:8]\n", 43); */
/*    out_put(cw_g_out, ":[i8|s:u|b:2|p:0|w:8]: [[i8|s:u|b:2|p:0|w:8]\n", -43); */
/*    out_put(cw_g_out, ":[i8|s:s|b:2]:\n", -43); */
  
/*    out_put(cw_g_out, ":[i16]:\n", 43); */
/*    out_put(cw_g_out, ":[i16|s:u]:\n", 43); */
/*    out_put(cw_g_out, ":[i16|s:s]:\n", -43); */
/*    out_put(cw_g_out, ":[i16|s:u|b:2|p:0|w:16]: [[i16|s:u|b:2|p:0|w:16]\n", 43); */
/*    out_put(cw_g_out, ":[i16|s:u|b:2|p:0|w:16]: [[i16|s:u|b:2|p:0|w:16]\n", -43); */
/*    out_put(cw_g_out, ":[i16|s:s|b:2]:\n", -43); */

/*    { */
/*      cw_uint32_t i; */
/*      char buf[65]; */
    
/*      for (i = 0; i < 1; i++) */
/*      { */
/*        out_put_s(cw_g_out, buf, "[i32|b:16]", (cw_uint32_t) 0xf2135123); */
/*        out_put(cw_g_out, "."); */
/*      } */
/*      out_put(cw_g_out, "\n[s]\n", buf); */
/*    } */

/*    out_put(cw_g_out, ":[c]:\n", 'A'); */
/*    out_put(cw_g_out, ":[c|w:3]:\n", 'A'); */
/*    out_put(cw_g_out, ":[c|w:3|j:l]:\n", 'A'); */
/*    out_put(cw_g_out, ":[c|w:3|j:c]:\n", 'A'); */
/*    out_put(cw_g_out, ":[c|w:3|j:r]:\n", 'A'); */
/*    out_put(cw_g_out, ":[c|w:3|j:l|p:_]:\n", 'A'); */
/*    out_put(cw_g_out, ":[c|w:3|j:c|p:+]:\n", 'A'); */
/*    out_put(cw_g_out, ":[c|w:3|j:r|p:-]:\n", 'A'); */

/*    out_put(cw_g_out, "0x:[p]:\n", 0x12345678); */
/*    out_put(cw_g_out, "0x:[p|w:12|p:_|j:c]:\n", 0x12345678); */

/*    out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__, */
/*  	    "Extended\n"); */
  
/*    out_put_fle(cw_g_out, 2, __FILE__, __LINE__, __FUNCTION__, */
/*  	      "Yo, timestamped\n"); */
  
  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
