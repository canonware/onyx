/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Test the out (printf-alike) class.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	out_put(out_err, "Test begin\n");

	/* out_new(), out_delete(). */
	out_put(out_err, "out_new(), out_delete()\n");
	{
		cw_out_t	out, *out_p;

		_cw_assert(out_new(&out, cw_g_mem) == &out);
		out_delete(&out);

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);
		out_delete(out_p);
	}

	/* out_register(), out_merge(). */
	out_put(out_err, "out_register(), out_merge()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

/*  		_cw_assert(out_register(out_p, "buf", sizeof(cw_buf_t *), */
/*  		    out_p_buf_render) == FALSE); */

		out_merge(out_p, out_err);

		out_delete(out_p);
	}

	/* out_default_fd_get(), out_default_fd_set(). */
	out_put(out_err, "out_default_fd_get(), out_default_fd_set()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_default_fd_get(out_p) == 2);

		out_default_fd_set(out_p, 1);
		_cw_assert(out_default_fd_get(out_p) == 1);

		out_delete(out_p);
	}

	/* out_put(). */
	out_put(out_err, "out_put()\n");
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
	out_put(out_err, "out_put_e()\n");
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

	/* out_put_n(). */
	out_put(out_err, "out_put_n()\n");
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
	out_put(out_err, "out_put_f()\n");
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		_cw_assert(out_put_f(out_p, 2, "16 Bytes output\n") == 16);
		_cw_assert(out_put_f(out_p, 2, "[s]\n", "[s]") > 0);
		_cw_assert(out_put_f(out_p, 2, "") == 0);

		out_delete(out_p);
	}

	/* out_put_fn(). */
	out_put(out_err, "out_put_fn()\n");
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
	out_put(out_err, "out_put_s()\n");
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
	out_put(out_err, "out_put_sa()\n");
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
	out_put(out_err, "out_put_sn()\n");
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
		_cw_assert(out_put_sn(out_p, str, 5, "i: [i]", 42) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[i] [s]", 42,
		    "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 34, "[i|b:2]_2", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 23, "[i|b:3]_3", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 14, "[i|b:7]_7", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 13, "[i|b:8]_8", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 13, "[i|b:10]_10", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 11, "[i|b:16]_16", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 10, "[i|b:32]_32", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 10, "[i|b:36]_36", 0xffffffff)
		    >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 19, "[i|w:10]:[s]", 42,
		    "[i|w:10]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 23, "[i|w:10|j:l]:[s]", 42,
		    "[i|w:10|j:l]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 23, "[i|w:10|j:c]:[s]", 42,
		    "[i|w:10|j:c]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 23, "[i|w:10|j:r]:[s]", 42,
		    "[i|w:10|j:r]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 23, "[i|w:10|p:_]:[s]", 42,
		    "[i|w:10|p:_]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 27, "[i|w:10|j:l|p:_]:[s]",
		    42, "[i|w:10|j:l|p:_]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 27, "[i|w:10|j:c|p:_]:[s]",
		    42, "[i|w:10|j:c|p:_]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 27, "[i|w:10|j:r|p:_]:[s]",
		    42, "[i|w:10|j:r|p:_]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 10, "[i|s:u]:[s]", 42,
		    "[i|s:u]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 10, "[i|s:s]:[s]", 42,
		    "[i|s:s]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 11, "[i|s:s]:[s]", -42,
		    "[i|s:s]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|s:s|w:10|p:_|j:l]:[s]", -42, "[i|s:s|w:10|p:_|j:l]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|s:s|w:10|p:_|j:c]:[s]", -42, "[i|s:s|w:10|p:_|j:c]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|s:s|w:10|p:_|j:r]:[s]", -42, "[i|s:s|w:10|p:_|j:r]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 27, "[i|w:10|p:_|+:-]:[s]",
		    42, "[i|w:10|p:_|+:-]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|w:10|j:l|p:_|+:-]:[s]", 42, "[i|w:10|j:l|p:_|+:-]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|w:10|j:c|p:_|+:-]:[s]", 42, "[i|w:10|j:c|p:_|+:-]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|w:10|j:r|p:_|+:-]:[s]", 42, "[i|w:10|j:r|p:_|+:-]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 27, "[i|w:10|p:_|+:+]:[s]",
		    42, "[i|w:10|p:_|+:+]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|w:10|j:l|p:_|+:+]:[s]", 42, "[i|w:10|j:l|p:_|+:+]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|w:10|j:c|p:_|+:+]:[s]", 42, "[i|w:10|j:c|p:_|+:+]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|w:10|j:r|p:_|+:+]:[s]", 42, "[i|w:10|j:r|p:_|+:+]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|s:s|w:10|p:_|+:-]:[s]", -42, "[i|s:s|w:10|p:_|+:-]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 35,
		    "[i|s:s|w:10|j:l|p:_|+:-]:[s]", -42,
		    "[i|s:s|w:10|j:l|p:_|+:-]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 35,
		    "[i|s:s|w:10|j:c|p:_|+:-]:[s]", -42,
		    "[i|s:s|w:10|j:c|p:_|+:-]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 35,
		    "[i|s:s|w:10|j:r|p:_|+:-]:[s]", -42,
		    "[i|s:s|w:10|j:r|p:_|+:-]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 31,
		    "[i|s:s|w:10|p:_|+:+]:[s]", -42, "[i|s:s|w:10|p:_|+:+]") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 35,
		    "[i|s:s|w:10|j:l|p:_|+:+]:[s]", -42,
		    "[i|s:s|w:10|j:l|p:_|+:+]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 35,
		    "[i|s:s|w:10|j:c|p:_|+:+]:[s]", -42,
		    "[i|s:s|w:10|j:c|p:_|+:+]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 35,
		    "[i|s:s|w:10|j:r|p:_|+:+]:[s]", -42,
		    "[i|s:s|w:10|j:r|p:_|+:+]") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* i64. wjpbs+. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 7, "i64: [q]",
		    (cw_uint64_t)42) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[q] [s]",
		    (cw_uint64_t)42, "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 66, "[q|b:2]_2",
		    ((cw_uint64_t)0xffffffff << 32) + 0xffffffff) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 16, "[q|b:36]_36",
		    ((cw_uint64_t)0xffffffff << 32) + 0xffffffff) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 2, "[q]", (cw_uint64_t)42) >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* c. wjp. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 4, "c: [c]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 19, "[c] [s]", 'c',
		    "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "[c|w:3]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "[c|w:3]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "[c|w:3|j:l]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "[c|w:3|j:c]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "[c|w:3|j:r]", 'c') >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "[c|w:3|j:l|p:_]", 'c') >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "[c|w:3|j:c|p:_]", 'c') >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "[c|w:3|j:r|p:_]", 'c') >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* s. wjp. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 11, "s: [s]", "<string>") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 26, "[s] [s]", "<string>",
		    "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[s|w:20]", "<string>") >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[s|w:20|j:l]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[s|w:20|j:c]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[s|w:20|j:r]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[s|w:20|j:l|p:_]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[s|w:20|j:c|p:_]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20, "[s|w:20|j:r|p:_]",
		    "<string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* p. wjpbs+. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 5, "p: [p]", (void *)0x42) >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 20,"[p] [s]", (void *)0x42,
		    "<trailing string>") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 11, "[p]_16", (void
		    *)0xffffffff) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 2, "[p]", (void *)0x42) >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 8, "[p|w:8]", (void *)0x42) >=
		    0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		/* Overflow. */
		memcpy(buf, ibuf, 81);
		_cw_assert(out_put_sn(out_p, str, 3, "won't fit") >= 0);
		_cw_assert(out_put(out_p, "[s]\n", buf) == 81);

		out_delete(out_p);
	}

	/* Specifier parse errors. */
	{
		cw_out_t	*out_p;

		out_p = out_new(NULL, cw_g_mem);
		_cw_check_ptr(out_p);

		xep_begin();
		volatile cw_bool_t	caught = FALSE;
		xep_try {
			out_put(out_p, "[");
		}
		xep_catch(_CW_STASHX_OUT_PARSE) {
			xep_handled();
			caught = TRUE;
		}
		xep_finally {
			_cw_assert(caught == TRUE);
		}
		xep_end();

		xep_begin();
		volatile cw_bool_t	caught = FALSE;
		xep_try {
			out_put(out_p, "[i");
		}
		xep_catch(_CW_STASHX_OUT_PARSE) {
			xep_handled();
			caught = TRUE;
		}
		xep_finally {
			_cw_assert(caught == TRUE);
		}
		xep_end();

		xep_begin();
		volatile cw_bool_t	caught = FALSE;
		xep_try {
			out_put(out_p, "[i|");
		}
		xep_catch(_CW_STASHX_OUT_PARSE) {
			xep_handled();
			caught = TRUE;
		}
		xep_finally {
			_cw_assert(caught == TRUE);
		}
		xep_end();

		xep_begin();
		volatile cw_bool_t	caught = FALSE;
		xep_try {
			out_put(out_p, "[i|]");
		}
		xep_catch(_CW_STASHX_OUT_PARSE) {
			xep_handled();
			caught = TRUE;
		}
		xep_finally {
			_cw_assert(caught == TRUE);
		}
		xep_end();

		xep_begin();
		volatile cw_bool_t	caught = FALSE;
		xep_try {
			out_put(out_p, "[i|x]");
		}
		xep_catch(_CW_STASHX_OUT_PARSE) {
			xep_handled();
			caught = TRUE;
		}
		xep_finally {
			_cw_assert(caught == TRUE);
		}
		xep_end();

		xep_begin();
		volatile cw_bool_t	caught = FALSE;
		xep_try {
			out_put(out_p, "[i|x:|]");
		}
		xep_catch(_CW_STASHX_OUT_PARSE) {
			xep_handled();
			caught = TRUE;
		}
		xep_finally {
			_cw_assert(caught == TRUE);
		}
		xep_end();

		xep_begin();
		volatile cw_bool_t	caught = FALSE;
		xep_try {
			out_put(out_p, "[foo]");
		}
		xep_catch(_CW_STASHX_OUT_PARSE) {
			xep_handled();
			caught = TRUE;
		}
		xep_finally {
			_cw_assert(caught == TRUE);
		}
		xep_end();

		out_delete(out_p);
	}

/*  	{ */
/*  		cw_uint32_t	i; */
/*  		char		buf[65]; */

/*  		for (i = 0; i < 10000; i++) { */
/*  			_cw_out_put_s(buf, "[i|b:16]", (cw_uint32_t) */
/*  			    0xf2135123); */
/*  			out_put(out_err, "."); */
/*  		} */
/*  		out_put(out_err, "\n[s]\n", buf); */
/*  	} */

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}