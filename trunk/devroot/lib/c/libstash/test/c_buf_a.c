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
 * bufc and buf test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#ifdef _CW_OS_SOLARIS
/* For ntohl(). */
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef _CW_OS_LINUX
/* For ntohl(). */
#include <netinet/in.h>
#endif

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

/*  	dbg_register(cw_g_dbg, "mem_verbose"); */

	/* bufc_new(), bufc_delete(), bufc_set_buffer(), bufc_get_size(). */
	{
		cw_bufc_t	bufc_a, *bufc_b;
		void		*buffer;
		char		str[512];

		_cw_assert(bufc_new(&bufc_a, cw_g_mem, NULL, NULL) == &bufc_a);
		bufc_delete(&bufc_a);

		_cw_assert(bufc_new(&bufc_a, cw_g_mem, NULL, NULL) == &bufc_a);
		bufc_set_buffer(&bufc_a, str, 512, FALSE, NULL, NULL);
		_cw_assert(bufc_get_size(&bufc_a) == 512);
		bufc_delete(&bufc_a);

		bufc_b = bufc_new(NULL, cw_g_mem, NULL, NULL);
		_cw_check_ptr(bufc_b);
		_cw_assert(bufc_get_size(bufc_b) == 0);
		bufc_delete(bufc_b);

		bufc_b = _cw_malloc(sizeof(cw_bufc_t));
		_cw_assert(bufc_new(bufc_b, cw_g_mem, (cw_opaque_dealloc_t
		    *)mem_free, cw_g_mem) == bufc_b);
		_cw_assert(bufc_get_size(bufc_b) == 0);
		bufc_delete(bufc_b);

		bufc_b = _cw_malloc(sizeof(cw_bufc_t));
		_cw_assert(bufc_new(bufc_b, cw_g_mem, (cw_opaque_dealloc_t
		    *)mem_free, cw_g_mem) == bufc_b);
		buffer = _cw_malloc(789);
		bufc_set_buffer(bufc_b, buffer, 789, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		_cw_assert(bufc_get_size(bufc_b) == 789);
		bufc_delete(bufc_b);
	}

	/* buf_new[_r](), buf_delete(). */
	{
		cw_buf_t	buf, *buf_p;

		_cw_assert(buf_new(&buf, cw_g_mem) == &buf);
		buf_delete(&buf);

		_cw_assert(buf_new_r(&buf, cw_g_mem) == &buf);
		buf_delete(&buf);

		buf_p = buf_new(NULL, cw_g_mem);
		_cw_check_ptr(buf_p);
		buf_delete(buf_p);

		buf_p = buf_new_r(NULL, cw_g_mem);
		_cw_check_ptr(buf_p);
		buf_delete(buf_p);
	}

	/* buf_get_uint8(), buf_get_uint32(), buf_get_uint64(). */
	{
		cw_bufc_t	bufc;
		cw_buf_t	*buf_p;
		cw_uint32_t	i, t_uint32_a, t_uint32_b;
		cw_uint64_t	t_uint64;
		char		*buffer;
		cw_uint32_t	*buffer_cast;

		buf_p = buf_new_r(NULL, cw_g_mem);

		buffer = (char *)_cw_malloc(512);
		buffer_cast = (cw_uint32_t *)buffer;

		bufc_new(&bufc, cw_g_mem, NULL, NULL);
		bufc_set_buffer(&bufc, buffer, 512, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, (void *)cw_g_mem);

		for (i = 0; i < 256; i++)
			buffer[i] = i;

		for (i = 0; i < 256; i++)
			buffer[511 - i] = buffer[i];

		buf_append_bufc(buf_p, &bufc, 0, 512);
		bufc_delete(&bufc);

		_cw_out_put("lower char dump:\n");
		for (i = 0; i < 256; i += 8) {
			_cw_out_put("[i|w:3|p:0]->0x[i|w:2|p:0|b:16]:"
			    "[i|w:3|p:0]->0x[i|w:2|p:0|b:16]:"
			    "[i|w:3|p:0]->0x[i|w:2|p:0|b:16]:"
			    "[i|w:3|p:0]->0x[i|w:2|p:0|b:16]:"
			    "[i|w:3|p:0]->0x[i|w:2|p:0|b:16]:"
			    "[i|w:3|p:0]->0x[i|w:2|p:0|b:16]:"
			    "[i|w:3|p:0]->0x[i|w:2|p:0|b:16]:"
			    "[i|w:3|p:0]->0x[i|w:2|p:0|b:16]\n", i,
			    buf_get_uint8(buf_p, i), i + 1, buf_get_uint8(buf_p,
			    i + 1), i + 2, buf_get_uint8(buf_p, i + 2), i + 3,
			    buf_get_uint8(buf_p, i + 3), i + 4,
			    buf_get_uint8(buf_p, i + 4), i + 5,
			    buf_get_uint8(buf_p, i + 5), i + 6,
			    buf_get_uint8(buf_p, i + 6), i + 7,
			    buf_get_uint8(buf_p, i + 7));
		}

		_cw_out_put("upper long dump:\n");
		for (i = 256; i < 512; i += 8) {
			t_uint32_a = buf_get_uint32(buf_p, i);
			t_uint32_a = ntohl(t_uint32_a);
			t_uint32_b = buf_get_uint32(buf_p, i + 4);
			t_uint32_b = ntohl(t_uint32_b);

			_cw_out_put("[i|w:3|p:0]->0x[i|w:8|p:0|b:16]:"
			    "[i|w:3|p:0]->0x[i|w:8|p:0|b:16]\n",
			    i, t_uint32_a, i + 4, t_uint32_b);
		}

		_cw_out_put("upper quad dump:\n");
		for (i = 256; i < 512; i += 8) {
			t_uint64 = buf_get_uint64(buf_p, i);
			t_uint64 = _cw_ntohq(t_uint64);

			_cw_out_put("[i|w:3|p:0]->0x[q|b:16|w:16|p:0]\n", i,
			    t_uint64);
			
		}

		/* Unaligned gets. */
		_cw_out_put("Unaligned buf_get_uint32():\n");
		for (i = 1; i < 4; i++) {
			t_uint32_a = buf_get_uint32(buf_p, 256 + i);
			t_uint32_a = ntohl(t_uint32_a);

			_cw_out_put("[i|w:3|p:0]->0x[i|w:8|p:0|b:16]\n", 256 +
			    i, t_uint32_a);
		}

		_cw_out_put("Unaligned buf_get_uint64():\n");
		for (i = 1; i < 8; i++) {
			t_uint64 = buf_get_uint64(buf_p, 256 + i);
			t_uint64 = _cw_ntohq(t_uint64);

			_cw_out_put("[i|w:3|p:0]->0x[q|b:16|w:16|p:0]\n", 256 +
			    i, t_uint64);
		}

		buf_delete(buf_p);
	}

	/*
	 * buf_get_uint32(), buf_get_uint64(), buf_get_iovec().
	 */
	{
		cw_buf_t	buf;
		cw_bufc_t	*bufc_p;
		cw_uint8_t	*a, b[2], c[3], *d, *e, f[11];
		cw_uint32_t	i;
		int		iov_count;
		const struct iovec *iov;
		cw_uint32_t	t_uint32;
		cw_uint64_t	t_uint64;

		/*
		 * a  0, b  1,  2, c  3,  4,  5, d  6,  7,  8,  9, e 10, 11,
		 * 12, 13, 14, f 42, 15, 16, 17, 18, 19, 20, 21, 22, 23, 42
		 */

		buf_new_r(&buf, cw_g_mem);

		a = (cw_uint8_t *)_cw_malloc(1);
		a[0] = 0;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, a, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		_cw_assert(buf_get_num_bufels(&buf) == 0);
		buf_append_bufc(&buf, bufc_p, 0, 1);
		_cw_assert(buf_get_num_bufels(&buf) == 1);
		bufc_delete(bufc_p);

		b[0] = 1;
		b[1] = 2;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)b, 2, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 0, 2);
		_cw_assert(buf_get_num_bufels(&buf) == 2);
		bufc_delete(bufc_p);

		c[0] = 3;
		c[1] = 4;
		c[2] = 5;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)c, 3, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 0, 3);
		_cw_assert(buf_get_num_bufels(&buf) == 3);
		bufc_delete(bufc_p);

		d = (cw_uint8_t *)_cw_malloc(4);
		d[0] = 6;
		d[1] = 7;
		d[2] = 8;
		d[3] = 9;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)d, 4, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		buf_append_bufc(&buf, bufc_p, 0, 4);
		_cw_assert(buf_get_num_bufels(&buf) == 4);
		bufc_delete(bufc_p);

		e = _cw_malloc(5);
		e[0] = 10;
		e[1] = 11;
		e[2] = 12;
		e[3] = 13;
		e[4] = 14;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)e, 5, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		buf_append_bufc(&buf, bufc_p, 0, 5);
		_cw_assert(buf_get_num_bufels(&buf) == 5);
		bufc_delete(bufc_p);

		f[0] = 42;
		f[1] = 15;
		f[2] = 16;
		f[3] = 17;
		f[4] = 18;
		f[5] = 19;
		f[6] = 20;
		f[7] = 21;
		f[8] = 22;
		f[9] = 23;
		f[10] = 42;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)f, 11, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 1, 10);
		_cw_assert(buf_get_num_bufels(&buf) == 6);
		bufc_delete(bufc_p);

		iov = buf_get_iovec(&buf, 0, FALSE, &iov_count);
		_cw_assert(iov_count == 0);
		_cw_assert(buf_get_num_bufels(&buf) == 6);

		iov = buf_get_iovec(&buf, buf_get_size(&buf), FALSE,
		    &iov_count);
		_cw_assert(iov_count == 6);
		_cw_assert(buf_get_num_bufels(&buf) == 6);
		_cw_assert(iov[0].iov_base == (char *)a);
		_cw_assert(iov[0].iov_len == 1);
		_cw_assert(iov[1].iov_base == (char *)b);
		_cw_assert(iov[1].iov_len == 2);
		_cw_assert(iov[2].iov_base == (char *)c);
		_cw_assert(iov[2].iov_len == 3);
		_cw_assert(iov[3].iov_base == (char *)d);
		_cw_assert(iov[3].iov_len == 4);
		_cw_assert(iov[4].iov_base == (char *)e);
		_cw_assert(iov[4].iov_len == 5);
		_cw_assert(iov[5].iov_base == (char *)&f[1]);
		_cw_assert(iov[5].iov_len == 9);

		iov = buf_get_iovec(&buf, buf_get_size(&buf) + 10, FALSE,
		    &iov_count);
		_cw_assert(iov_count == 6);
		_cw_assert(6 == buf_get_num_bufels(&buf));
		_cw_assert(iov[0].iov_base == (char *)a);
		_cw_assert(iov[0].iov_len == 1);
		_cw_assert(iov[1].iov_base == (char *)b);
		_cw_assert(iov[1].iov_len == 2);
		_cw_assert(iov[2].iov_base == (char *)c);
		_cw_assert(iov[2].iov_len == 3);
		_cw_assert(iov[3].iov_base == (char *)d);
		_cw_assert(iov[3].iov_len == 4);
		_cw_assert(iov[4].iov_base == (char *)e);
		_cw_assert(iov[4].iov_len == 5);
		_cw_assert(iov[5].iov_base == (char *)&f[1]);
		_cw_assert(iov[5].iov_len == 9);

		iov = buf_get_iovec(&buf, buf_get_size(&buf) - 5, FALSE,
		    &iov_count);
		_cw_assert(iov_count == 6);
		_cw_assert(6 == buf_get_num_bufels(&buf));
		_cw_assert(iov[0].iov_base == (char *)a);
		_cw_assert(iov[0].iov_len == 1);
		_cw_assert(iov[1].iov_base == (char *)b);
		_cw_assert(iov[1].iov_len == 2);
		_cw_assert(iov[2].iov_base == (char *)c);
		_cw_assert(iov[2].iov_len == 3);
		_cw_assert(iov[3].iov_base == (char *)d);
		_cw_assert(iov[3].iov_len == 4);
		_cw_assert(iov[4].iov_base == (char *)e);
		_cw_assert(iov[4].iov_len == 5);
		_cw_assert(iov[5].iov_base == (char *)&f[1]);
		_cw_assert(iov[5].iov_len == 4);

		iov = buf_get_iovec(&buf, buf_get_size(&buf) - 15, FALSE,
		    &iov_count);
		_cw_assert(iov_count == 4);
		_cw_assert(buf_get_num_bufels(&buf) == 6);
		_cw_assert(iov[0].iov_base == (char *)a);
		_cw_assert(iov[0].iov_len == 1);
		_cw_assert(iov[1].iov_base == (char *)b);
		_cw_assert(iov[1].iov_len == 2);
		_cw_assert(iov[2].iov_base == (char *)c);
		_cw_assert(iov[2].iov_len == 3);
		_cw_assert(iov[3].iov_base == (char *)d);
		_cw_assert(iov[3].iov_len == 3);

		_cw_assert(buf_get_size(&buf) == 24);

		_cw_out_put("Hodge podge buf_get_uint32():\n");
		for (i = 0; i <= 20; i++) {
			t_uint32 = buf_get_uint32(&buf, i);
			t_uint32 = ntohl(t_uint32);

			_cw_out_put("[i|w:3|p:0]->0x[i|w:8|p:0|b:16]\n", i,
			    t_uint32);
		}

		_cw_out_put("Hodge podge buf_get_uint64():\n");
		for (i = 0; i <= 16; i++) {
			t_uint64 = buf_get_uint64(&buf, i);
			t_uint64 = _cw_ntohq(t_uint64);

			_cw_out_put("[i|w:3|p:0]->0x[q|b:16|w:16|p:0]\n", i,
			    t_uint64);
		}

		buf_delete(&buf);
	}
	/* buf_set_uint8(). */
	{
		cw_buf_t	buf;
		cw_bufc_t	*bufc_p;
		cw_uint8_t	*a, b[2], c[3], *d, *e, f[13];
		cw_uint32_t	i;

		/*
		 * r- a  0, r- b  1,  2, rw c  3,  4,  5, r- d  6,  7,  8,
		 * 9, rw e 10, 11, 12, 13, 14, r- f (42), 15, 16, 17, 18,
		 * 19, 20, 21, 22, 23, 24, 25, (42)
		 */

		buf_new_r(&buf, cw_g_mem);

		a = (cw_uint8_t *)_cw_malloc(1);
		a[0] = 0;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, a, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		_cw_assert(buf_get_num_bufels(&buf) == 0);
		buf_append_bufc(&buf, bufc_p, 0, 1);
		_cw_assert(buf_get_num_bufels(&buf) == 1);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 1);

		b[0] = 1;
		b[1] = 2;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)b, 2, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 0, 2);
		_cw_assert(buf_get_num_bufels(&buf) == 2);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 3);

		c[0] = 3;
		c[1] = 4;
		c[2] = 5;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)c, 3, TRUE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 0, 3);
		_cw_assert(buf_get_num_bufels(&buf) == 3);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 6);

		d = (cw_uint8_t *)_cw_malloc(4);
		d[0] = 6;
		d[1] = 7;
		d[2] = 8;
		d[3] = 9;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)d, 4, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		buf_append_bufc(&buf, bufc_p, 0, 4);
		_cw_assert(buf_get_num_bufels(&buf) == 4);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 10);

		e = _cw_malloc(5);
		e[0] = 10;
		e[1] = 11;
		e[2] = 12;
		e[3] = 13;
		e[4] = 14;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)e, 5, TRUE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		buf_append_bufc(&buf, bufc_p, 0, 5);
		_cw_assert(buf_get_num_bufels(&buf) == 5);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 15);

		f[0] = 42;
		f[1] = 15;
		f[2] = 16;
		f[3] = 17;
		f[4] = 18;
		f[5] = 19;
		f[6] = 20;
		f[7] = 21;
		f[8] = 22;
		f[9] = 23;
		f[10] = 24;
		f[11] = 25;
		f[12] = 42;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)f, 13, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 1, 12);
		_cw_assert(buf_get_num_bufels(&buf) == 6);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 26);

		_cw_assert(buf_get_uint8(&buf, 0) == 0);
		_cw_assert(buf_get_uint8(&buf, 1) == 1);
		_cw_assert(buf_get_uint8(&buf, 2) == 2);
		_cw_assert(buf_get_uint8(&buf, 3) == 3);
		_cw_assert(buf_get_uint8(&buf, 4) == 4);
		_cw_assert(buf_get_uint8(&buf, 5) == 5);
		_cw_assert(buf_get_uint8(&buf, 6) == 6);
		_cw_assert(buf_get_uint8(&buf, 7) == 7);
		_cw_assert(buf_get_uint8(&buf, 8) == 8);
		_cw_assert(buf_get_uint8(&buf, 9) == 9);
		_cw_assert(buf_get_uint8(&buf, 10) == 10);
		_cw_assert(buf_get_uint8(&buf, 11) == 11);
		_cw_assert(buf_get_uint8(&buf, 12) == 12);
		_cw_assert(buf_get_uint8(&buf, 13) == 13);
		_cw_assert(buf_get_uint8(&buf, 14) == 14);
		_cw_assert(buf_get_uint8(&buf, 15) == 15);
		_cw_assert(buf_get_uint8(&buf, 16) == 16);
		_cw_assert(buf_get_uint8(&buf, 17) == 17);
		_cw_assert(buf_get_uint8(&buf, 18) == 18);
		_cw_assert(buf_get_uint8(&buf, 19) == 19);
		_cw_assert(buf_get_uint8(&buf, 20) == 20);
		_cw_assert(buf_get_uint8(&buf, 21) == 21);
		_cw_assert(buf_get_uint8(&buf, 22) == 22);
		_cw_assert(buf_get_uint8(&buf, 23) == 23);
		_cw_assert(buf_get_uint8(&buf, 24) == 24);
		_cw_assert(buf_get_uint8(&buf, 25) == 25);

		for (i = 0; i < 26; i++) {
			_cw_assert(i == buf_get_uint8(&buf, i));
			_cw_assert(buf_set_uint8(&buf, i, (~i) & 0xff) ==
			    FALSE);
			_cw_assert(buf_get_uint8(&buf, i) == ((~i) & 0xff));
		}

		for (i = 0; i < 26; i++) {
			_cw_assert(buf_get_uint8(&buf, i) == ((~i) & 0xff));
			buf_set_uint8(&buf, i, ~i);
		}

		for (; i < 50; i++) {
			buf_set_uint8(&buf, i, i);
			_cw_assert(buf_get_uint8(&buf, i) == i);
		}

		buf_delete(&buf);
	}

	/* buf_set_uint32(). */
	{
		cw_buf_t	buf;
		cw_bufc_t	*bufc_p;
		cw_uint8_t	*a, b[2], c[3], *d, *e, f[13];
		cw_uint32_t	i;
		cw_uint32_t	t_uint32;

		/*
		 * r- a  0, r- b  1,  2, rw c  3,  4,  5, r- d  6,  7,  8,
		 * 9, rw e 10, 11, 12, 13, 14, r- f (42), 15, 16, 17, 18,
		 * 19, 20, 21, 22, 23, 24, 25, (42)
		 */

		buf_new_r(&buf, cw_g_mem);

		a = (cw_uint8_t *)_cw_malloc(1);
		a[0] = 0;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, a, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		_cw_assert(buf_get_num_bufels(&buf) == 0);
		buf_append_bufc(&buf, bufc_p, 0, 1);
		_cw_assert(buf_get_num_bufels(&buf) == 1);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 1);

		b[0] = 1;
		b[1] = 2;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)b, 2, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 0, 2);
		_cw_assert(buf_get_num_bufels(&buf) == 2);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 3);

		c[0] = 3;
		c[1] = 4;
		c[2] = 5;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)c, 3, TRUE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 0, 3);
		_cw_assert(buf_get_num_bufels(&buf) == 3);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 6);

		d = (cw_uint8_t *)_cw_malloc(4);
		d[0] = 6;
		d[1] = 7;
		d[2] = 8;
		d[3] = 9;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)d, 4, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		buf_append_bufc(&buf, bufc_p, 0, 4);
		_cw_assert(buf_get_num_bufels(&buf) == 4);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 10);

		e = _cw_malloc(5);
		e[0] = 10;
		e[1] = 11;
		e[2] = 12;
		e[3] = 13;
		e[4] = 14;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)e, 5, TRUE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		buf_append_bufc(&buf, bufc_p, 0, 5);
		_cw_assert(buf_get_num_bufels(&buf) == 5);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 15);

		f[0] = 42;
		f[1] = 15;
		f[2] = 16;
		f[3] = 17;
		f[4] = 18;
		f[5] = 19;
		f[6] = 20;
		f[7] = 21;
		f[8] = 22;
		f[9] = 23;
		f[10] = 24;
		f[11] = 25;
		f[12] = 42;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)f, 13, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 1, 12);
		_cw_assert(buf_get_num_bufels(&buf) == 6);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 26);

		_cw_assert(buf_get_uint8(&buf, 0) == 0);
		_cw_assert(buf_get_uint8(&buf, 1) == 1);
		_cw_assert(buf_get_uint8(&buf, 2) == 2);
		_cw_assert(buf_get_uint8(&buf, 3) == 3);
		_cw_assert(buf_get_uint8(&buf, 4) == 4);
		_cw_assert(buf_get_uint8(&buf, 5) == 5);
		_cw_assert(buf_get_uint8(&buf, 6) == 6);
		_cw_assert(buf_get_uint8(&buf, 7) == 7);
		_cw_assert(buf_get_uint8(&buf, 8) == 8);
		_cw_assert(buf_get_uint8(&buf, 9) == 9);
		_cw_assert(buf_get_uint8(&buf, 10) == 10);
		_cw_assert(buf_get_uint8(&buf, 11) == 11);
		_cw_assert(buf_get_uint8(&buf, 12) == 12);
		_cw_assert(buf_get_uint8(&buf, 13) == 13);
		_cw_assert(buf_get_uint8(&buf, 14) == 14);
		_cw_assert(buf_get_uint8(&buf, 15) == 15);
		_cw_assert(buf_get_uint8(&buf, 16) == 16);
		_cw_assert(buf_get_uint8(&buf, 17) == 17);
		_cw_assert(buf_get_uint8(&buf, 18) == 18);
		_cw_assert(buf_get_uint8(&buf, 19) == 19);
		_cw_assert(buf_get_uint8(&buf, 20) == 20);
		_cw_assert(buf_get_uint8(&buf, 21) == 21);
		_cw_assert(buf_get_uint8(&buf, 22) == 22);
		_cw_assert(buf_get_uint8(&buf, 23) == 23);
		_cw_assert(buf_get_uint8(&buf, 24) == 24);
		_cw_assert(buf_get_uint8(&buf, 25) == 25);

		for (i = 0; i < 23; i++) {
			t_uint32 = ~buf_get_uint32(&buf, i);

			_cw_assert(buf_set_uint32(&buf, i, t_uint32) == FALSE);
/*  			_cw_out_put("t_uint32: 0x[i|b:16], " */
/*  			    "buf_get_uint32(&buf, [i]): 0x[i|b:16]\n", */
/*  			    t_uint32, i, buf_get_uint32(&buf, i)); */
			_cw_assert(buf_get_uint32(&buf, i) == t_uint32);
		}

		for (i = 0; i < 26; i++) {
			buf_set_uint32(&buf, i, ~i);
			_cw_assert(buf_get_uint32(&buf, i) == ~i);
		}

		for (; i < 50; i++) {
			buf_set_uint32(&buf, i, i);
			_cw_assert(buf_get_uint32(&buf, i) == i);
		}

		buf_delete(&buf);
	}

	/* buf_set_uint64(). */
	{
		cw_buf_t	buf;
		cw_bufc_t	*bufc_p;
		cw_uint8_t	*a, b[2], c[3], *d, *e, f[13];
		cw_uint32_t	i;
		cw_uint64_t	t_uint64;

		/*
		 * r- a  0, r- b  1,  2, rw c  3,  4,  5, r- d  6,  7,  8,
		 * 9, rw e 10, 11, 12, 13, 14, r- f (42), 15, 16, 17, 18,
		 * 19, 20, 21, 22, 23, 24, 25, (42)
		 */

		buf_new_r(&buf, cw_g_mem);

		a = (cw_uint8_t *)_cw_malloc(1);
		a[0] = 0;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, a, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		_cw_assert(buf_get_num_bufels(&buf) == 0);
		buf_append_bufc(&buf, bufc_p, 0, 1);
		_cw_assert(buf_get_num_bufels(&buf) == 1);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 1);

		b[0] = 1;
		b[1] = 2;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)b, 2, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 0, 2);
		_cw_assert(buf_get_num_bufels(&buf) == 2);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 3);

		c[0] = 3;
		c[1] = 4;
		c[2] = 5;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)c, 3, TRUE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 0, 3);
		_cw_assert(buf_get_num_bufels(&buf) == 3);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 6);

		d = (cw_uint8_t *)_cw_malloc(4);
		d[0] = 6;
		d[1] = 7;
		d[2] = 8;
		d[3] = 9;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)d, 4, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		buf_append_bufc(&buf, bufc_p, 0, 4);
		_cw_assert(buf_get_num_bufels(&buf) == 4);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 10);

		e = _cw_malloc(5);
		e[0] = 10;
		e[1] = 11;
		e[2] = 12;
		e[3] = 13;
		e[4] = 14;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)e, 5, TRUE,
		    (cw_opaque_dealloc_t *)mem_free, cw_g_mem);
		buf_append_bufc(&buf, bufc_p, 0, 5);
		_cw_assert(buf_get_num_bufels(&buf) == 5);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 15);

		f[0] = 42;
		f[1] = 15;
		f[2] = 16;
		f[3] = 17;
		f[4] = 18;
		f[5] = 19;
		f[6] = 20;
		f[7] = 21;
		f[8] = 22;
		f[9] = 23;
		f[10] = 24;
		f[11] = 25;
		f[12] = 42;
		bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p, (void *)f, 13, FALSE, NULL,
		    NULL);
		buf_append_bufc(&buf, bufc_p, 1, 12);
		_cw_assert(buf_get_num_bufels(&buf) == 6);
		bufc_delete(bufc_p);
		_cw_assert(buf_get_size(&buf) == 26);

		_cw_assert(buf_get_uint8(&buf, 0) == 0);
		_cw_assert(buf_get_uint8(&buf, 1) == 1);
		_cw_assert(buf_get_uint8(&buf, 2) == 2);
		_cw_assert(buf_get_uint8(&buf, 3) == 3);
		_cw_assert(buf_get_uint8(&buf, 4) == 4);
		_cw_assert(buf_get_uint8(&buf, 5) == 5);
		_cw_assert(buf_get_uint8(&buf, 6) == 6);
		_cw_assert(buf_get_uint8(&buf, 7) == 7);
		_cw_assert(buf_get_uint8(&buf, 8) == 8);
		_cw_assert(buf_get_uint8(&buf, 9) == 9);
		_cw_assert(buf_get_uint8(&buf, 10) == 10);
		_cw_assert(buf_get_uint8(&buf, 11) == 11);
		_cw_assert(buf_get_uint8(&buf, 12) == 12);
		_cw_assert(buf_get_uint8(&buf, 13) == 13);
		_cw_assert(buf_get_uint8(&buf, 14) == 14);
		_cw_assert(buf_get_uint8(&buf, 15) == 15);
		_cw_assert(buf_get_uint8(&buf, 16) == 16);
		_cw_assert(buf_get_uint8(&buf, 17) == 17);
		_cw_assert(buf_get_uint8(&buf, 18) == 18);
		_cw_assert(buf_get_uint8(&buf, 19) == 19);
		_cw_assert(buf_get_uint8(&buf, 20) == 20);
		_cw_assert(buf_get_uint8(&buf, 21) == 21);
		_cw_assert(buf_get_uint8(&buf, 22) == 22);
		_cw_assert(buf_get_uint8(&buf, 23) == 23);
		_cw_assert(buf_get_uint8(&buf, 24) == 24);
		_cw_assert(buf_get_uint8(&buf, 25) == 25);

		for (i = 0; i < 19; i++) {
			t_uint64 = ~buf_get_uint64(&buf, i);

			_cw_assert(buf_set_uint64(&buf, i, t_uint64) == FALSE);
			_cw_assert(buf_get_uint64(&buf, i) == t_uint64);
		}

		for (i = 0; i < 26; i++) {
			buf_set_uint64(&buf, i, ~i);
			_cw_assert(buf_get_uint64(&buf, i) == ~i);
		}

		for (; i < 50; i++) {
			buf_set_uint64(&buf, i, i);
			_cw_assert(buf_get_uint64(&buf, i) == i);
		}

		buf_delete(&buf);
	}

	/* buf_set_range(). */
	{
		cw_buf_t	*buf;
		char		str_a[57] =
		    "This is string A.  Blah blah.  Okay, this is now longer.";
		char		*str_b =
		    "And following is string B.  Mumble mumble.";

		buf = buf_new_r(NULL, cw_g_mem);
		_cw_check_ptr(buf);

		_cw_assert(buf_set_range(buf, 0, strlen(str_a) + 1, (cw_uint8_t
		    *)str_a, FALSE) == FALSE);
		_cw_assert(buf_set_range(buf, strlen(str_a) + 1, strlen(str_b) +
		    1, (cw_uint8_t *)str_b, FALSE) == FALSE);
		_cw_assert(buf_get_size(buf) == (strlen(str_a) + strlen(str_b) +
		    2));
		buf_release_head_data(buf, buf_get_size(buf));

		_cw_assert(buf_set_range(buf, 0, strlen(str_a) + 1, (cw_uint8_t
		    *)str_a, TRUE) == FALSE);
		_cw_assert(buf_set_range(buf, 4, strlen(str_b) + 1, (cw_uint8_t
		    *)str_b, TRUE) == FALSE);

		buf_delete(buf);
	}

	/*
	 * buf_prepend_bufc(), buf_append_bufc(), buf_get_size().
	 */
	{
		cw_buf_t	*buf_p;
		cw_bufc_t	*bufc_p_a, *bufc_p_b, *bufc_p_c, *bufc_p_d;
		char		*str_a, *str_b, *str_c;

		str_a = (char *)_cw_malloc(1);
		str_a[0] = 'A';

		str_b = (char *)_cw_malloc(1);
		str_b[0] = 'B';

		str_c = (char *)_cw_malloc(1);
		str_c[0] = 'C';

		buf_p = buf_new_r(NULL, cw_g_mem);
		_cw_assert(buf_get_size(buf_p) == 0);

		bufc_p_a = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p_a, (void *)str_a, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, (void *)cw_g_mem);

		bufc_p_b = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p_b, (void *)str_b, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, (void *)cw_g_mem);

		bufc_p_c = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p_c, (void *)str_c, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, (void *)cw_g_mem);

		buf_append_bufc(buf_p, bufc_p_a, 0, 1);
		_cw_assert(buf_get_size(buf_p) == 1);
		buf_append_bufc(buf_p, bufc_p_b, 0, 1);
		_cw_assert(buf_get_size(buf_p) == 2);
		buf_append_bufc(buf_p, bufc_p_c, 0, 1);
		_cw_assert(buf_get_size(buf_p) == 3);

		bufc_p_d = bufc_new(NULL, cw_g_mem, NULL, NULL);
		buf_append_bufc(buf_p, bufc_p_d, 0, 0);
		_cw_assert(buf_get_size(buf_p) == 3);
		buf_prepend_bufc(buf_p, bufc_p_d, 0, 0);
		_cw_assert(buf_get_size(buf_p) == 3);
		bufc_delete(bufc_p_d);

		buf_prepend_bufc(buf_p, bufc_p_a, 0, 1);
		_cw_assert(buf_get_size(buf_p) == 4);
		buf_prepend_bufc(buf_p, bufc_p_b, 0, 1);
		_cw_assert(buf_get_size(buf_p) == 5);
		buf_prepend_bufc(buf_p, bufc_p_c, 0, 1);
		_cw_assert(buf_get_size(buf_p) == 6);

		_cw_assert(buf_get_uint8(buf_p, 0) == 'C');
		_cw_assert(buf_get_uint8(buf_p, 1) == 'B');
		_cw_assert(buf_get_uint8(buf_p, 2) == 'A');
		_cw_assert(buf_get_uint8(buf_p, 3) == 'A');
		_cw_assert(buf_get_uint8(buf_p, 4) == 'B');
		_cw_assert(buf_get_uint8(buf_p, 5) == 'C');

		bufc_delete(bufc_p_a);
		bufc_delete(bufc_p_b);
		bufc_delete(bufc_p_c);
		buf_delete(buf_p);
	}

	/*
	 * buf_catenate_buf(), buf_get_size().
	 */
	{
		cw_buf_t	*buf_p_a, *buf_p_b;
		cw_bufc_t	*bufc_p_a, *bufc_p_b, *bufc_p_c;
		char		*str_a, *str_b, *str_c;

		str_a = (char *)_cw_malloc(1);
		str_a[0] = 'A';

		str_b = (char *)_cw_malloc(1);
		str_b[0] = 'B';

		str_c = (char *)_cw_malloc(1);
		str_c[0] = 'C';

		buf_p_a = buf_new_r(NULL, cw_g_mem);
		buf_p_b = buf_new(NULL, cw_g_mem);
		_cw_assert(buf_get_size(buf_p_a) == 0);
		_cw_assert(buf_get_size(buf_p_b) == 0);

		bufc_p_a = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p_a, (void *)str_a, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, (void *)cw_g_mem);

		bufc_p_b = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p_b, (void *)str_b, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, (void *)cw_g_mem);

		bufc_p_c = bufc_new(NULL, cw_g_mem, NULL, NULL);
		bufc_set_buffer(bufc_p_c, (void *)str_c, 1, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, (void *)cw_g_mem);

		buf_append_bufc(buf_p_a, bufc_p_a, 0, 1);
		buf_append_bufc(buf_p_a, bufc_p_b, 0, 1);
		buf_append_bufc(buf_p_b, bufc_p_c, 0, 1);
		_cw_assert(buf_get_size(buf_p_a) == 2);
		_cw_assert(buf_get_size(buf_p_b) == 1);
		buf_catenate_buf(buf_p_a, buf_p_b, TRUE);
		_cw_assert(buf_get_size(buf_p_a) == 3);
		_cw_assert(buf_get_size(buf_p_b) == 1);
		buf_catenate_buf(buf_p_a, buf_p_b, FALSE);
		_cw_assert(buf_get_size(buf_p_a) == 4);
		_cw_assert(buf_get_size(buf_p_b) == 0);
		buf_catenate_buf(buf_p_a, buf_p_b, FALSE);
		_cw_assert(buf_get_size(buf_p_a) == 4);
		_cw_assert(buf_get_size(buf_p_b) == 0);

		bufc_delete(bufc_p_a);
		bufc_delete(bufc_p_b);
		bufc_delete(bufc_p_c);
		buf_delete(buf_p_a);
		buf_delete(buf_p_b);
	}

	/* buf_catenate_buf(). */
	{
		cw_buf_t	buf_a, buf_b, buf_c;
		cw_bufc_t	*bufc_p;
		cw_uint32_t	i;

		buf_new_r(&buf_a, cw_g_mem);
		buf_new(&buf_b, cw_g_mem);
		buf_new_r(&buf_c, cw_g_mem);

		for (i = 0; i < 3; i++) {
			bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
			bufc_set_buffer(bufc_p, _cw_malloc(8), 8,
			    FALSE, (cw_opaque_dealloc_t *)mem_free, (void
			    *)cw_g_mem);
			buf_append_bufc(&buf_a, bufc_p, 0, 8);
			buf_catenate_buf(&buf_c, &buf_b, FALSE);
			buf_catenate_buf(&buf_b, &buf_a, TRUE);
			bufc_delete(bufc_p);
		}

		buf_delete(&buf_c);
		buf_new_r(&buf_c, cw_g_mem);

		_cw_assert(buf_get_size(&buf_a) == 24);
		_cw_assert(buf_get_size(&buf_b) == 24);
		_cw_assert(buf_get_size(&buf_c) == 0);

		buf_catenate_buf(&buf_a, &buf_b, TRUE);
		_cw_assert(buf_get_size(&buf_a) == 48);
		_cw_assert(buf_get_size(&buf_b) == 24);

		buf_catenate_buf(&buf_c, &buf_b, FALSE);
		_cw_assert(buf_get_size(&buf_b) == 0);
		_cw_assert(buf_get_size(&buf_c) == 24);

		buf_catenate_buf(&buf_b, &buf_a, TRUE);
		_cw_assert(buf_get_size(&buf_a) == 48);
		_cw_assert(buf_get_size(&buf_b) == 48);

		buf_catenate_buf(&buf_b, &buf_a, FALSE);
		_cw_assert(buf_get_size(&buf_a) == 0);
		_cw_assert(buf_get_size(&buf_b) == 96);

		buf_catenate_buf(&buf_a, &buf_b, FALSE);
		_cw_assert(buf_get_size(&buf_a) == 96);
		_cw_assert(buf_get_size(&buf_b) == 0);

		buf_delete(&buf_a);
		buf_delete(&buf_b);
		buf_delete(&buf_c);
	}

	/* buf_split(). */
	/* buf_release_head_data(), buf_release_tail_data(). */
	{
		cw_buf_t	buf_a, buf_b, buf_c;
		cw_bufc_t	*bufc_p;
		cw_uint32_t	i;

		buf_new_r(&buf_a, cw_g_mem);
		buf_new(&buf_b, cw_g_mem);
		buf_new_r(&buf_c, cw_g_mem);

		for (i = 0; i < 3; i++) {
			bufc_p = bufc_new(NULL, cw_g_mem, NULL, NULL);
			bufc_set_buffer(bufc_p, _cw_malloc(8), 8,
			    FALSE, (cw_opaque_dealloc_t *)mem_free, (void
			    *)cw_g_mem);
			buf_append_bufc(&buf_a, bufc_p, 0, 8);
			buf_append_bufc(&buf_b, bufc_p, 0, 8);
			buf_prepend_bufc(&buf_c, bufc_p, 0, 8);
			bufc_delete(bufc_p);
		}

		_cw_assert(buf_get_size(&buf_a) == 24);
		_cw_assert(buf_get_size(&buf_b) == 24);
		_cw_assert(buf_get_size(&buf_c) == 24);

		buf_split(&buf_a, &buf_b, 13);
		_cw_assert(buf_get_size(&buf_a) == 37);
		_cw_assert(buf_get_size(&buf_b) == 11);

		buf_split(&buf_a, &buf_b, 0);
		_cw_assert(buf_get_size(&buf_a) == 37);
		_cw_assert(buf_get_size(&buf_b) == 11);

		buf_split(&buf_b, &buf_a, 0);
		_cw_assert(buf_get_size(&buf_a) == 37);
		_cw_assert(buf_get_size(&buf_b) == 11);

		buf_split(&buf_b, &buf_a, 37);
		_cw_assert(buf_get_size(&buf_a) == 0);
		_cw_assert(buf_get_size(&buf_b) == 48);

		buf_split(&buf_a, &buf_b, 0);
		_cw_assert(buf_get_size(&buf_a) == 0);
		_cw_assert(buf_get_size(&buf_b) == 48);

		buf_split(&buf_b, &buf_a, 0);
		_cw_assert(buf_get_size(&buf_a) == 0);
		_cw_assert(buf_get_size(&buf_b) == 48);

		buf_split(&buf_a, &buf_b, 17);
		_cw_assert(buf_get_size(&buf_a) == 17);
		_cw_assert(buf_get_size(&buf_b) == 31);

		buf_split(&buf_a, &buf_b, 30);
		_cw_assert(buf_get_size(&buf_a) == 47);
		_cw_assert(buf_get_size(&buf_b) == 1);

		buf_split(&buf_a, &buf_b, 1);
		_cw_assert(buf_get_size(&buf_a) == 48);
		_cw_assert(buf_get_size(&buf_b) == 0);

		buf_split(&buf_b, &buf_a, 1);
		_cw_assert(buf_get_size(&buf_a) == 47);
		_cw_assert(buf_get_size(&buf_b) == 1);

		buf_split(&buf_b, &buf_a, 4);
		_cw_assert(buf_get_size(&buf_a) == 43);
		_cw_assert(buf_get_size(&buf_b) == 5);

		buf_split(&buf_b, &buf_a, 3);
		_cw_assert(buf_get_size(&buf_a) == 40);
		_cw_assert(buf_get_size(&buf_b) == 8);

		buf_catenate_buf(&buf_b, &buf_c, TRUE);
		_cw_assert(buf_get_size(&buf_b) == 32);
		_cw_assert(buf_get_size(&buf_c) == 24);

		buf_catenate_buf(&buf_c, &buf_b, FALSE);
		_cw_assert(buf_get_size(&buf_a) == 40);
		_cw_assert(buf_get_size(&buf_b) == 0);
		_cw_assert(buf_get_size(&buf_c) == 56);

		_cw_assert(buf_release_head_data(&buf_a, 10) == FALSE);
		_cw_assert(buf_get_size(&buf_a) == 30);

		_cw_assert(buf_release_tail_data(&buf_a, 10) == FALSE);
		_cw_assert(buf_get_size(&buf_a) == 20);

		buf_catenate_buf(&buf_b, &buf_a, TRUE);
		_cw_assert(buf_get_size(&buf_b) == 20);

		buf_catenate_buf(&buf_b, &buf_a, FALSE);
		_cw_assert(buf_get_size(&buf_a) == 0);
		_cw_assert(buf_get_size(&buf_b) == 40);

		_cw_assert(buf_get_size(&buf_c) == 56);

		_cw_assert(buf_release_tail_data(&buf_c, 10) == FALSE);
		_cw_assert(buf_get_size(&buf_c) == 46);

		_cw_assert(FALSE == buf_release_head_data(&buf_c, 10));
		_cw_assert(buf_get_size(&buf_c) == 36);

		buf_catenate_buf(&buf_a, &buf_c, TRUE);
		_cw_assert(buf_get_size(&buf_a) == 36);

		buf_catenate_buf(&buf_a, &buf_c, FALSE);
		_cw_assert(buf_get_size(&buf_c) == 0);
		_cw_assert(buf_get_size(&buf_a) == 72);

		buf_release_tail_data(&buf_a, 16);
		buf_catenate_buf(&buf_c, &buf_a, TRUE);
		buf_release_head_data(&buf_b, 40);
		buf_catenate_buf(&buf_b, &buf_a, TRUE);
		_cw_assert(buf_get_size(&buf_a) == 56);
		_cw_assert(buf_get_size(&buf_b) == 56);
		_cw_assert(buf_get_size(&buf_c) == 56);

		_cw_assert(buf_release_head_data(&buf_a, 56) == FALSE);
		_cw_assert(buf_get_size(&buf_a) == 0);

		buf_catenate_buf(&buf_a, &buf_c, FALSE);
		_cw_assert(buf_get_size(&buf_a) == 56);
		_cw_assert(buf_get_size(&buf_c) == 0);
		buf_catenate_buf(&buf_c, &buf_a, FALSE);
		_cw_assert(buf_get_size(&buf_a) == 0);
		_cw_assert(buf_get_size(&buf_c) == 56);
		buf_catenate_buf(&buf_a, &buf_c, TRUE);
		_cw_assert(buf_get_size(&buf_a) == 56);
		_cw_assert(buf_get_size(&buf_c) == 56);

		_cw_assert(buf_release_tail_data(&buf_b, 56) == FALSE);
		_cw_assert(buf_get_size(&buf_b) == 0);

		buf_catenate_buf(&buf_b, &buf_c, FALSE);
		_cw_assert(buf_get_size(&buf_b) == 56);
		_cw_assert(buf_get_size(&buf_c) == 0);
		buf_catenate_buf(&buf_c, &buf_b, FALSE);
		_cw_assert(buf_get_size(&buf_b) == 0);
		_cw_assert(buf_get_size(&buf_c) == 56);
		buf_catenate_buf(&buf_b, &buf_c, TRUE);
		_cw_assert(buf_get_size(&buf_b) == 56);
		_cw_assert(buf_get_size(&buf_c) == 56);
		buf_delete(&buf_a);
		buf_delete(&buf_b);
		buf_delete(&buf_c);
	}

	/* out_put("[b]"). */
	{
		cw_bufc_t	bufc;
		cw_buf_t	*buf_p, buf;
		char		*buffer;

		buf_p = buf_new_r(NULL, cw_g_mem);
		_cw_check_ptr(buf_p);
		buf_new_r(&buf, cw_g_mem);
		buffer = (char *)_cw_malloc(512);
		bufc_new(&bufc, cw_g_mem, NULL, NULL);
		bufc_set_buffer(&bufc, buffer, 512, FALSE,
		    (cw_opaque_dealloc_t *)mem_free, (void *)cw_g_mem);

		/* Fill buffer. */
		memcpy(buffer, "This is a whack of text in a buffer, which is"
		    " inside a buf.  Now here is some more text.  Since this "
		    "is a 512 byte buf, we might as well fill it with some re"
		    "adable text, as opposed to random garbage.  Then again, "
		    "writing prose just to fill a space is rather reminiscent"
		    " of the good ol' school days.  Perhaps some margin inden"
		    "tation or slightly wider line spacing is in order.  Oh w"
		    "ait, this is ascii text.  Oh well.  Who says technology "
		    "can solve any problem?  Well, I'm about to run out of sp"
		    "ace.  Live long an", 512);

		buf_append_bufc(buf_p, &bufc, 0, 512);
		bufc_delete(&bufc);

		/* Print. */
		_cw_assert(_cw_out_put("Here is the buf :[b]:\n", buf_p) > 0);
		buf_split(&buf, buf_p, 23);
		buf_catenate_buf(buf_p, &buf, TRUE);
		_cw_assert(_cw_out_put(":[b]:\n", &buf) > 0);
		_cw_assert(_cw_out_put(":[b|w:40]:\n", &buf) > 0);
		_cw_assert(_cw_out_put(":[b|w:40|p:_]:\n", &buf) > 0);
		_cw_assert(_cw_out_put(":[b|w:40|p:+|j:l]:\n", &buf) > 0);
		_cw_assert(_cw_out_put(":[b|w:40|p:-|j:c]:\n", &buf) > 0);
		_cw_assert(_cw_out_put(":[b|w:40|p:_|j:r]:\n", &buf) > 0);

		buf_split(&buf, buf_p, 3);
		_cw_assert(_cw_out_put(":[b]:\n", &buf) > 0);
		buf_catenate_buf(buf_p, &buf, TRUE);
		buf_catenate_buf(buf_p, &buf, FALSE);
		buf_split(&buf, buf_p, 4);
		_cw_assert(_cw_out_put(":[b]:\n", &buf) > 0);
		buf_catenate_buf(buf_p, &buf, TRUE);
		buf_catenate_buf(buf_p, &buf, FALSE);
		buf_split(&buf, buf_p, 5);
		_cw_assert(_cw_out_put(":[b]:\n", &buf) > 0);
		buf_catenate_buf(buf_p, &buf, TRUE);
		buf_catenate_buf(buf_p, &buf, FALSE);
		buf_split(&buf, buf_p, 6);
		_cw_assert(_cw_out_put(":[b]:\n", &buf) > 0);
		buf_catenate_buf(buf_p, &buf, TRUE);
		buf_catenate_buf(buf_p, &buf, FALSE);
		buf_release_head_data(buf_p, buf_get_size(buf_p) - 40);

		_cw_assert(_cw_out_put("buf_p :[b]:\n", buf_p) > 0);

		buf_delete(buf_p);
		buf_delete(&buf);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
