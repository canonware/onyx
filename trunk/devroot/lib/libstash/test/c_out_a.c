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
  out_put(cw_g_out, "Test begin\n");

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

/*      _cw_assert(0 < out_put_le(out_p, "<file>", 42, "<function>", */
/*  			      "Timestamped output [s]\n", "[s]")); */
/*      _cw_assert(0 < out_put_le(out_p, NULL, 42, NULL, "\n")); */
    
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

/*      _cw_assert(0 < out_put_fle(out_p, 2, "<file>", 42, "<function>", */
/*  			       "Timestamped output [s]\n", "[s]")); */
/*      _cw_assert(0 < out_put_fle(out_p, 2, NULL, 42, NULL, "")); */
    
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
    char buf[80];

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    memset(buf, 'x', 80);

    _cw_assert(0 < out_put_s(out_p, buf,
			     "<string> [s]\n", "<trailing string>"));
    _cw_assert(0 < out_put(out_p, "[s]", buf));
    
    out_delete(out_p);
  }

  /* out_put_sa(). */
  out_put(cw_g_out, "out_put_sa()\n");
  {
    cw_out_t * out_p;
    char * buf;

    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(0 < out_put_sa(out_p, &buf,
			      "<string> [s]\n", "<trailing string>"));
    _cw_assert(0 < out_put(out_p, "[s]", buf));

    _cw_free(buf);
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

    /* i8. wjpbs+. */
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "i8: [i8]", 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "int8: [int8]", 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));

    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i8] [s]", 42, "<trailing string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i8|b:2]_2", 0xff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i8|b:36]_36", 0xff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i8]", 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    /* i16. wjpbs+. */
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "i16: [i16]", 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "int16: [int16]", 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));

    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i16] [s]", 42, "<trailing string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i16|b:2]_2", 0xffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i16|b:36]_36", 0xffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i16]", 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    /* i32. wjpbs+. */
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "i32: [i32]", 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "int32: [int32]", 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));

    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32] [s]", 42, "<trailing string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|b:2]_2", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|b:3]_3", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|b:7]_7", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|b:8]_8", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|b:10]_10", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|b:16]_16", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|b:32]_32", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|b:36]_36", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10]:[s]", 42, "[i32|w:10]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:l]:[s]", 42, "[i32|w:10|j:l]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:c]:[s]", 42, "[i32|w:10|j:c]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:r]:[s]", 42, "[i32|w:10|j:r]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|p:_]:[s]", 42, "[i32|w:10|p:_]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:l|p:_]:[s]", 42,
			       "[i32|w:10|j:l|p:_]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:c|p:_]:[s]", 42,
			       "[i32|w:10|j:c|p:_]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:r|p:_]:[s]", 42,
			       "[i32|w:10|j:r|p:_]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:u]:[s]", 42, "[i32|s:u]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s]:[s]", 42, "[i32|s:s]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s]:[s]", -42, "[i32|s:s]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|p:_|j:l]:[s]", -42,
			       "[i32|s:s|w:10|p:_|j:l]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|p:_|j:c]:[s]", -42,
			       "[i32|s:s|w:10|p:_|j:c]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|p:_|j:r]:[s]", -42,
			       "[i32|s:s|w:10|p:_|j:r]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|p:_|+:-]:[s]", 42,
			       "[i32|w:10|p:_|+:-]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:l|p:_|+:-]:[s]", 42,
			       "[i32|w:10|j:l|p:_|+:-]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:c|p:_|+:-]:[s]", 42,
			       "[i32|w:10|j:c|p:_|+:-]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:r|p:_|+:-]:[s]", 42,
			       "[i32|w:10|j:r|p:_|+:-]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|p:_|+:+]:[s]", 42,
			       "[i32|w:10|p:_|+:+]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:l|p:_|+:+]:[s]", 42,
			       "[i32|w:10|j:l|p:_|+:+]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:c|p:_|+:+]:[s]", 42,
			       "[i32|w:10|j:c|p:_|+:+]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|w:10|j:r|p:_|+:+]:[s]", 42,
			       "[i32|w:10|j:r|p:_|+:+]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|p:_|+:-]:[s]", -42,
			       "[i32|s:s|w:10|p:_|+:-]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|j:l|p:_|+:-]:[s]", -42,
			       "[i32|s:s|w:10|j:l|p:_|+:-]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|j:c|p:_|+:-]:[s]", -42,
			       "[i32|s:s|w:10|j:c|p:_|+:-]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|j:r|p:_|+:-]:[s]", -42,
			       "[i32|s:s|w:10|j:r|p:_|+:-]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|p:_|+:+]:[s]", -42,
			       "[i32|s:s|w:10|p:_|+:+]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|j:l|p:_|+:+]:[s]", -42,
			       "[i32|s:s|w:10|j:l|p:_|+:+]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|j:c|p:_|+:+]:[s]", -42,
			       "[i32|s:s|w:10|j:c|p:_|+:+]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i32|s:s|w:10|j:r|p:_|+:+]:[s]", -42,
			       "[i32|s:s|w:10|j:r|p:_|+:+]"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    /* i64. wjpbs+. */
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "i64: [i64]", (cw_uint64_t) 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "int64: [int64]", (cw_uint64_t) 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));

    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i64] [s]", (cw_uint64_t) 42,
			       "<trailing string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i64|b:2]_2",
			       ((cw_uint64_t) 0xffffffff << 32) + 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i64|b:36]_36",
			       ((cw_uint64_t) 0xffffffff << 32) + 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[i64]", (cw_uint64_t) 42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    /* c. wjp. */
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "c: [c]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "char: [char]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));

    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c] [s]", 'c', "<trailing string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c|w:3]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c|w:3]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c|w:3|j:l]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c|w:3|j:c]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c|w:3|j:r]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c|w:3|j:l|p:_]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c|w:3|j:c|p:_]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[c|w:3|j:r|p:_]", 'c'));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    /* s. wjp. */
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "s: [s]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "string: [string]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));

    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s] [s]", "<string>", "<trailing string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s|w:20]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s|w:20|j:l]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s|w:20|j:c]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s|w:20|j:r]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s|w:20|j:l|p:_]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s|w:20|j:c|p:_]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[s|w:20|j:r|p:_]", "<string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    /* p. wjpbs+. */
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "p: [p]", 0x42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "pointer: [pointer]", 0x42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));

    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[p] [s]", 0x42, "<trailing string>"));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[p]_2", 0xffffffff));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    memcpy(buf, ibuf, 81);
    _cw_assert(0 <= out_put_sn(out_p, str, 70,
			       "[p]", 0x42));
    _cw_assert(81 == out_put(out_p, "[s]\n", buf));
    
    out_delete(out_p);
  }

  /* Specifier parse errors. */
  {
    cw_out_t * out_p;
    
    out_p = out_new(NULL);
    _cw_check_ptr(out_p);

    _cw_assert(-2 == out_put(out_p, "["));
    _cw_assert(-2 == out_put(out_p, "[i32"));
    _cw_assert(-2 == out_put(out_p, "[i32|"));
    _cw_assert(-2 == out_put(out_p, "[i32|]"));
    _cw_assert(-2 == out_put(out_p, "[i32|x]"));
    _cw_assert(-2 == out_put(out_p, "[i32|x:|]"));
    _cw_assert(-2 == out_put(out_p, "[foo]"));

    out_delete(out_p);
  }

/*    { */
/*      cw_uint32_t i; */
/*      char buf[65]; */
    
/*      for (i = 0; i < 10000; i++) */
/*      { */
/*        out_put_s(cw_g_out, buf, "[i32|b:16]", (cw_uint32_t) 0xf2135123); */
/*        out_put(cw_g_out, "."); */
/*      } */
/*      out_put(cw_g_out, "\n[s]\n", buf); */
/*    } */
  
  out_put(cw_g_out, "Test end\n");
  libstash_shutdown();
  return 0;
}
