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
 * Test the out (printf-alike) class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	dbg_unregister(cw_g_dbg, "mem_error");
	_cw_out_put("Test begin\n");

/*  	dbg_register(cw_g_dbg, "mem_verbose"); */

	/* out_new(), out_delete(). */
	_cw_out_put("out_new(), out_delete()\n");
	{
		cw_out_t	out, *out_p;

		_cw_assert(out_new(&out, cw_g_mem) == &out);
		out_delete(&out);

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);
		out_delete(out_p);
	}

	/* out_register(), out_merge(). */
	_cw_out_put("out_register(), out_merge()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

/*  		_cw_assert(out_register(out_p, "buf", sizeof(cw_buf_t *), */
/*  		    out_p_buf_render) == FALSE); */

		_cw_assert(out_merge(out_p, cw_g_out) == FALSE);

		out_delete(out_p);
	}

	/* out_get_default_fd(), out_set_default_fd(). */
	_cw_out_put("out_get_default_fd(), out_set_default_fd()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_get_default_fd(out_p) == 2);

		out_set_default_fd(out_p, 1);
		_cw_assert(out_get_default_fd(out_p) == 1);

		out_delete(out_p);
	}

	/* out_put(). */
	_cw_out_put("out_put()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put(out_p, "16 Bytes output\n") == 16);
		_cw_assert(out_put(out_p, "[s]\n", "[s]") > 0);
		_cw_assert(out_put(out_p, "") == 0);

		out_delete(out_p);
	}

	/* out_put_e(). */
	_cw_out_put("out_put_e()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put_e(out_p, "<file>", 42, "<function>",
			"Extended output with filename and funcname\n") > 0);
		_cw_assert(out_put_e(out_p, "<file>", 42, NULL,
			"Extended output with filename\n") > 0);
		_cw_assert(out_put_e(out_p, NULL, 42, "<function>",
			"Extended output with funcname\n") > 0);
		_cw_assert(out_put_e(out_p, NULL, 42, NULL,
			"Extended output, NULL args\n") > 0);
		_cw_assert(out_put_e(out_p, NULL, 42, NULL, "") == 0);

		out_delete(out_p);
	}

	/* out_put_l(). */
	_cw_out_put("out_put_l()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

/*  		_cw_assert(out_put_l(out_p, "Timestamped output [s]\n", "[s]") > */
/*  		    0); */
/*  		_cw_assert(out_put_l(out_p, "\n") > 0); */

		out_delete(out_p);
	}

	/* out_put_le(). */
	_cw_out_put("out_put_le()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

/*  		_cw_assert(out_put_le(out_p, "<file>", 42, "<function>", */
/*  		    "Timestamped output [s]\n", "[s]") > 0); */
/*  		_cw_assert(out_put_le(out_p, NULL, 42, NULL, "\n") > 0); */

		out_delete(out_p);
	}

	/* out_put_n(). */
	_cw_out_put("out_put_n()\n");
	{
		cw_out_t	*out_p;
		char		buf[71] =
		    "0123456789012345678901234567890123456789"
		    "012345678901234567890123456789";

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put_n(out_p, 60, "[s]", buf) == 60);
		_cw_assert(out_put_n(out_p, 2, ":[s]", "\n:") == 2);
		_cw_assert(out_put_n(out_p, 70, "[s]", buf) == 70);
		_cw_assert(out_put_n(out_p, 2, ":[s]", "\n:") == 2);
		_cw_assert(out_put_n(out_p, 71, "[s]", buf) == 70);
		_cw_assert(out_put_n(out_p, 2, ":[s]", "\n:") == 2);
		_cw_assert(out_put_n(out_p, 80, "") == 0);

		out_delete(out_p);
	}

	/* out_put_f(). */
	_cw_out_put("out_put_f()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put_f(out_p, 2, "16 Bytes output\n") == 16);
		_cw_assert(out_put_f(out_p, 2, "[s]\n", "[s]") > 0);
		_cw_assert(out_put_f(out_p, 2, "") == 0);

		out_delete(out_p);
	}

	/* out_put_fe(). */
	_cw_out_put("out_put_fe()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put_fe(out_p, 2, "<file>", 42, "<function>",
			"Extended output with filename and funcname\n") > 0);
		_cw_assert(out_put_fe(out_p, 2, "<file>", 42, NULL,
			"Extended output with filename\n") > 0);
		_cw_assert(out_put_fe(out_p, 2, NULL, 42, "<function>",
			"Extended output with funcname\n") > 0);
		_cw_assert(out_put_fe(out_p, 2, NULL, 42, NULL,
			"Extended output, NULL args\n") > 0);
		_cw_assert(out_put_fe(out_p, 2, NULL, 42, NULL,
			"Extended output, NULL args, [s]\n",
			"[s]"));
		_cw_assert(out_put_fe(out_p, 2, NULL, 42, NULL, "") == 0);

		out_delete(out_p);
	}

	/* out_put_fl(). */
	_cw_out_put("out_put_fl()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

/*  		_cw_assert(out_put_fl(out_p, 2, "Timestamped output [s]\n", */
/*  		    "[s]") > 0); */
/*  		_cw_assert(out_put_fl(out_p, 2, "\n") > 0); */

		out_delete(out_p);
	}

	/* out_put_fle(). */
	_cw_out_put("out_put_fle()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

/*  		_cw_assert(out_put_fle(out_p, 2, "<file>", 42, "<function>", */
/*  		    "Timestamped output [s]\n", "[s]") > 0); */
/*  		_cw_assert(out_put_fle(out_p, 2, NULL, 42, NULL, "\n") > 0); */

		out_delete(out_p);
	}

	/* out_put_fn(). */
	_cw_out_put("out_put_fn()\n");
	{
		cw_out_t	*out_p;
		char		buf[71] =
		    "0123456789012345678901234567890123456789"
		    "012345678901234567890123456789";

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put_fn(out_p, 2, 60, "[s]", buf) == 60);
		_cw_assert(out_put_fn(out_p, 2, 2, ":[s]", "\n:") == 2);
		_cw_assert(out_put_fn(out_p, 2, 70, "[s]", buf) == 70);
		_cw_assert(out_put_fn(out_p, 2, 2, ":[s]", "\n:") == 2);
		_cw_assert(out_put_fn(out_p, 2, 71, "[s]", buf) == 70);
		_cw_assert(out_put_fn(out_p, 2, 2, ":[s]", "\n:") == 2);
		_cw_assert(out_put_fn(out_p, 2, 80, "") == 0);

		out_delete(out_p);
	}

	/* out_put_s(). */
	_cw_out_put("out_put_s()\n");
	{
		cw_out_t	*out_p;
		char		buf[80];

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		memset(buf, 'x', 80);

		_cw_assert(out_put_s(out_p, buf, "<string> [s]\n",
		    "<trailing string>") > 0);
		_cw_assert(out_put(out_p, "[s]", buf) > 0);

		out_delete(out_p);
	}

	/* out_put_sa(). */
	_cw_out_put("out_put_sa()\n");
	{
		cw_out_t	*out_p;
		char		*buf;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put_sa(out_p, &buf, "<string> [s]\n",
		    "<trailing string>") > 0);
		_cw_assert(out_put(out_p, "[s]", buf) > 0);

		_cw_free(buf);
		out_delete(out_p);
	}

	/* out_put_sn(). */
	_cw_out_put("out_put_sn()\n");
	{
		cw_out_t	*out_p;
		char		*str, buf[81], ibuf[81] =
		    "........................................"
		    "........................................";

		str = &buf[10];

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		/* i. wjpbs+. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "i: [i]", 42) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i] [s]", 42,
		    "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|b:2]_2", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|b:3]_3", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|b:7]_7", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|b:8]_8", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|b:10]_10", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|b:16]_16", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|b:32]_32", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|b:36]_36", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10]:[s]", 42,
		    "[i|w:10]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|j:l]:[s]", 42,
		    "[i|w:10|j:l]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|j:c]:[s]", 42,
		    "[i|w:10|j:c]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|j:r]:[s]", 42,
		    "[i|w:10|j:r]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|p:_]:[s]", 42,
		    "[i|w:10|p:_]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|j:l|p:_]:[s]",
		    42, "[i|w:10|j:l|p:_]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|j:c|p:_]:[s]",
		    42, "[i|w:10|j:c|p:_]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|j:r|p:_]:[s]",
		    42, "[i|w:10|j:r|p:_]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|s:u]:[s]", 42,
		    "[i|s:u]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|s:s]:[s]", 42,
		    "[i|s:s]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|s:s]:[s]", -42,
		    "[i|s:s]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|p:_|j:l]:[s]", -42, "[i|s:s|w:10|p:_|j:l]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|p:_|j:c]:[s]", -42, "[i|s:s|w:10|p:_|j:c]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|p:_|j:r]:[s]", -42, "[i|s:s|w:10|p:_|j:r]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|p:_|+:-]:[s]",
		    42, "[i|w:10|p:_|+:-]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|w:10|j:l|p:_|+:-]:[s]", 42, "[i|w:10|j:l|p:_|+:-]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|w:10|j:c|p:_|+:-]:[s]", 42, "[i|w:10|j:c|p:_|+:-]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|w:10|j:r|p:_|+:-]:[s]", 42, "[i|w:10|j:r|p:_|+:-]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[i|w:10|p:_|+:+]:[s]",
		    42, "[i|w:10|p:_|+:+]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|w:10|j:l|p:_|+:+]:[s]", 42, "[i|w:10|j:l|p:_|+:+]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|w:10|j:c|p:_|+:+]:[s]", 42, "[i|w:10|j:c|p:_|+:+]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|w:10|j:r|p:_|+:+]:[s]", 42, "[i|w:10|j:r|p:_|+:+]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|p:_|+:-]:[s]", -42, "[i|s:s|w:10|p:_|+:-]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|j:l|p:_|+:-]:[s]", -42,
		    "[i|s:s|w:10|j:l|p:_|+:-]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|j:c|p:_|+:-]:[s]", -42,
		    "[i|s:s|w:10|j:c|p:_|+:-]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|j:r|p:_|+:-]:[s]", -42,
		    "[i|s:s|w:10|j:r|p:_|+:-]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|p:_|+:+]:[s]", -42, "[i|s:s|w:10|p:_|+:+]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|j:l|p:_|+:+]:[s]", -42,
		    "[i|s:s|w:10|j:l|p:_|+:+]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|j:c|p:_|+:+]:[s]", -42,
		    "[i|s:s|w:10|j:c|p:_|+:+]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,
		    "[i|s:s|w:10|j:r|p:_|+:+]:[s]", -42,
		    "[i|s:s|w:10|j:r|p:_|+:+]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* i64. wjpbs+. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "i64: [q]",
		    (cw_uint64_t)42) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[q] [s]",
		    (cw_uint64_t)42, "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[q|b:2]_2",
		    ((cw_uint64_t)0xffffffff << 32) + 0xffffffff) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[q|b:36]_36",
		    ((cw_uint64_t)0xffffffff << 32) + 0xffffffff) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[q]", (cw_uint64_t)42) >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* c. wjp. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "c: [c]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c] [s]", 'c',
		    "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c|w:3]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c|w:3]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c|w:3|j:l]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c|w:3|j:c]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c|w:3|j:r]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c|w:3|j:l|p:_]", 'c') >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c|w:3|j:c|p:_]", 'c') >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[c|w:3|j:r|p:_]", 'c') >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* s. wjp. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "s: [s]", "<string>") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[s] [s]", "<string>",
		    "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[s|w:20]", "<string>") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[s|w:20|j:l]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[s|w:20|j:c]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[s|w:20|j:r]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[s|w:20|j:l|p:_]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[s|w:20|j:c|p:_]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[s|w:20|j:r|p:_]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* p. wjpbs+. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "p: [p]", 0x42) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70,"[p] [s]", 0x42,
		    "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[p]_16", 0xffffffff) >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 70, "[p]", 0x42) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		out_delete(out_p);
	}

	/* Specifier parse errors. */
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put(out_p, "[") == -2);
		_cw_assert(out_put(out_p, "[i") == -2);
		_cw_assert(out_put(out_p, "[i|") == -2);
		_cw_assert(out_put(out_p, "[i|]") == -2);
		_cw_assert(out_put(out_p, "[i|x]") == -2);
		_cw_assert(out_put(out_p, "[i|x:|]") == -2);
		_cw_assert(out_put(out_p, "[foo]") == -2);

		out_delete(out_p);
	}

/*  	{ */
/*  		cw_uint32_t	i; */
/*  		char		buf[65]; */

/*  		for (i = 0; i < 10000; i++) { */
/*  			_cw_out_put_s(buf, "[i|b:16]", (cw_uint32_t) */
/*  			    0xf2135123); */
/*  			_cw_out_put("."); */
/*  		} */
/*  		_cw_out_put("\n[s]\n", buf); */
/*  	} */

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
