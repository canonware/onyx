/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * buf test.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	cw_buf_t	*buf_a, *buf_b, *buf_c, *buf_d;

	libstash_init();
	out_put(out_err, "Test begin\n");

	buf_a = buf_new_r(NULL, cw_g_mem);
	buf_b = buf_new_r(NULL, cw_g_mem);
	buf_c = buf_new_r(NULL, cw_g_mem);
	buf_d = buf_new_r(NULL, cw_g_mem);

	/* Create the data. */
	{
		cw_uint32_t	i, j;
		cw_bufc_t	*bufc;
		void		*data;

		for (i = 1; i < 16; i++) {
			for (j = 1; j < 16; j++) {
				bufc = bufc_new((cw_bufc_t
				    *)_cw_malloc(sizeof(cw_bufc_t)), cw_g_mem,
				    (cw_opaque_dealloc_t *)mem_free_e,
				    cw_g_mem);
				data = _cw_malloc(i * j);
				bufc_buffer_set(bufc, data, i * j, FALSE,
				    (cw_opaque_dealloc_t *)mem_free_e,
				    cw_g_mem);

				buf_bufc_append(buf_a, bufc, 0, i * j);
				bufc_delete(bufc);
			}
		}

		for (i = 0; i < 4; i++) {
			bufc = bufc_new((cw_bufc_t
			    *)_cw_malloc(sizeof(cw_bufc_t)), cw_g_mem,
			    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem);
			data = _cw_malloc(4096);
			bufc_buffer_set(bufc, data, 4096, FALSE,
			    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem);
			buf_bufc_append(buf_b, bufc, 0, 4096);
			bufc_delete(bufc);
		}

		for (i = 0; i < 1024; i++) {
			bufc = bufc_new((cw_bufc_t
			    *)_cw_malloc(sizeof(cw_bufc_t)), cw_g_mem,
			    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem);
			data = _cw_malloc(16);
			bufc_buffer_set(bufc, data, 16, FALSE,
			    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem);
			buf_bufc_append(buf_c, bufc, 0, 16);
			bufc_delete(bufc);
		}
	}

	out_put(out_err, "[i] bytes of data in buf_a (various sized bufc's)\n",
	    buf_size_get(buf_a));
	out_put(out_err, "[i] bytes of data in buf_b (4kB bufc's)\n",
	    buf_size_get(buf_b));
	out_put(out_err, "[i] bytes of data in buf_c (16B bufc's)\n",
	    buf_size_get(buf_c));

	/* buf_uint8_get() inc loop for each buf. */
	{
		cw_uint32_t	i, j, size;

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_a); j < size; j++)
				buf_uint8_get(buf_a, j);
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_b); j < size; j++)
				buf_uint8_get(buf_b, j);
		}

		for (i = 0; i < 1; i++)
			for (j = 0, size = buf_size_get(buf_c); j < size; j++) {
				buf_uint8_get(buf_c, j);
		}
	}

	/* buf_uint8_get() dec loop for each buf. */
	{
		cw_uint32_t	i, j, size;

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_a); j < size; j++)
				buf_uint8_get(buf_a, size - (1 + j));
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_b); j < size; j++)
				buf_uint8_get(buf_b, size - (1 + j));
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_c); j < size; j++)
				buf_uint8_get(buf_c, size - (1 + j));
		}
	}

	/* Stride-wise buf_uint8_get(). */
	{
		cw_uint32_t	i, j, size;

		for (i = 0, size = buf_size_get(buf_a); i < 1; i++) {
			for (j = i; j < size; j += 13)
				buf_uint8_get(buf_a, j);
		}

		for (i = 0, size = buf_size_get(buf_b); i < 1; i++) {
			for (j = i; j < size; j += 13)
				buf_uint8_get(buf_b, j);
		}

		for (i = 0, size = buf_size_get(buf_c); i < 1; i++) {
			for (j = i; j < size; j += 13)
				buf_uint8_get(buf_c, j);
		}
	}

	/* Multi-stride buf_uint8_get(). */
	{
		cw_uint32_t	h, i, j, k, size;

		for (h = 0; h < 1; h++) {
			for (i = 1, size = buf_size_get(buf_a); i < size; i
			    <<= 1) {
				for (j = 0; j < i; j++) {
					for (k = 0; k < size; k += i)
						buf_uint8_get(buf_a, k);
				}
			}
		}

		for (h = 0; h < 1; h++) {
			for (i = 1, size = buf_size_get(buf_b); i < size; i
			    <<= 1) {
				for (j = 0; j < i; j++) {
					for (k = 0; k < size; k += i)
						buf_uint8_get(buf_b, k);
				}
			}
		}

		for (h = 0; h < 1; h++) {
			for (i = 1, size = buf_size_get(buf_c); i < size; i
			    <<= 1) {
				for (j = 0; j < i; j++) {
					for (k = 0; k < size; k += i)
						buf_uint8_get(buf_c, k);
				}
			}
		}
	}

	/* buf_uint32_get() inc loop for each buf. */
	{
		cw_uint32_t	i, j, size;

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_a); j < size - 3;
			    j++)
				buf_uint32_get(buf_a, j);
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_b); j < size - 3;
			    j++)
				buf_uint32_get(buf_b, j);
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_c); j < size - 3;
			    j++)
				buf_uint32_get(buf_c, j);
		}
	}

	/* buf_uint32_get() dec loop for each buf. */
	{
		cw_uint32_t	i, j, size;

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_a); j < size - 3;
			    j++)
				buf_uint32_get(buf_a, size - (4 + j));
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_b); j < size - 3;
			    j++)
				buf_uint32_get(buf_b, size - (4 + j));
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_c); j < size - 3;
			    j++)
				buf_uint32_get(buf_c, size - (4 + j));
		}
	}

	/* Stride-wise buf_uint32_get(). */
	{
		cw_uint32_t	i, j, size;

		for (i = 0, size = buf_size_get(buf_a); i < 1; i++) {
			for (j = i; j < size - 3; j += 13)
				buf_uint32_get(buf_a, j);
		}

		for (i = 0, size = buf_size_get(buf_b); i < 1; i++) {
			for (j = i; j < size - 3; j += 13)
				buf_uint32_get(buf_b, j);
		}

		for (i = 0, size = buf_size_get(buf_c); i < 1; i++) {
			for (j = i; j < size - 3; j += 13)
				buf_uint32_get(buf_c, j);
		}
	}

	/* buf_uint64_get() inc loop for each buf. */
	{
		cw_uint32_t	i, j, size;

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_a); j < size - 7;
			    j++)
				buf_uint64_get(buf_a, j);
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_b); j < size - 7;
			    j++)
				buf_uint64_get(buf_b, j);
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_c); j < size - 7;
			    j++)
				buf_uint64_get(buf_c, j);
		}
	}

	/* buf_uint64_get() dec loop for each buf. */
	{
		cw_uint32_t	i, j, size;

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_a); j < size - 7;
			    j++)
				buf_uint64_get(buf_a, size - (8 + j));
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_b); j < size - 7;
			    j++)
				buf_uint64_get(buf_b, size - (8 + j));
		}

		for (i = 0; i < 1; i++) {
			for (j = 0, size = buf_size_get(buf_c); j < size - 7;
			    j++)
				buf_uint64_get(buf_c, size - (8 + j));
		}
	}

	/* Stride-wise buf_uint64_get(). */
	{
		cw_uint32_t	i, j, size;

		for (i = 0, size = buf_size_get(buf_a); i < 1; i++) {
			for (j = i; j < size - 7; j += 13)
				buf_uint64_get(buf_a, j);
		}

		for (i = 0, size = buf_size_get(buf_b); i < 1; i++) {
			for (j = i; j < size - 7; j += 13)
				buf_uint64_get(buf_b, j);
		}

		for (i = 0, size = buf_size_get(buf_c); i < 1; i++) {
			for (j = i; j < size - 7; j += 13)
				buf_uint64_get(buf_c, j);
		}
	}

	{
		cw_uint32_t	i;

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_a, FALSE);
			buf_buf_catenate(buf_a, buf_d, FALSE);
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_b, FALSE);
			buf_buf_catenate(buf_b, buf_d, FALSE);
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_c, FALSE);
			buf_buf_catenate(buf_c, buf_d, FALSE);
		}
	}

	/* buf_split() loop. */
	{
		cw_uint32_t	i, j, size;

		for (i = 2099, size = buf_size_get(buf_a); i < size; i +=
		    2099) {
			for (j = 0; (j + i) < size; j += i)
				buf_split(buf_d, buf_a, i);
			buf_buf_catenate(buf_d, buf_a, FALSE);
			buf_buf_catenate(buf_a, buf_d, FALSE);
		}

		for (i = 2099, size = buf_size_get(buf_b); i < size; i +=
		    2099) {
			for (j = 0; (j + i) < size; j += i)
				buf_split(buf_d, buf_b, i);
			buf_buf_catenate(buf_d, buf_b, FALSE);
			buf_buf_catenate(buf_b, buf_d, FALSE);
		}

		for (i = 2099, size = buf_size_get(buf_c); i < size; i +=
		    2099) {
			for (j = 0; (j + i) < size; j += i)
				buf_split(buf_d, buf_c, i);
			buf_buf_catenate(buf_d, buf_c, FALSE);
			buf_buf_catenate(buf_c, buf_d, FALSE);
		}
	}

	/* buf_head_data_release() loop. */
	{
		cw_uint32_t	i, j;

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_a, TRUE);
			for (j = 0; j < buf_size_get(buf_d); j += 13)
				buf_head_data_release(buf_d, j);
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_b, TRUE);
			for (j = 0; j < buf_size_get(buf_d); j += 13)
				buf_head_data_release(buf_d, j);
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_c, TRUE);
			for (j = 0; j < buf_size_get(buf_d); j += 13)
				buf_head_data_release(buf_d, j);
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}
	}

	/* buf_tail_data_release() loop. */
	{
		cw_uint32_t	i, j;

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_a, TRUE);
			for (j = 0; j < buf_size_get(buf_d); j += 13)
				buf_tail_data_release(buf_d, j);
			buf_tail_data_release(buf_d, buf_size_get(buf_d));
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_b, TRUE);
			for (j = 0; j < buf_size_get(buf_d); j += 13)
				buf_tail_data_release(buf_d, j);
			buf_tail_data_release(buf_d, buf_size_get(buf_d));
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_c, TRUE);
			for (j = 0; j < buf_size_get(buf_d); j += 13)
				buf_tail_data_release(buf_d, j);
			buf_tail_data_release(buf_d, buf_size_get(buf_d));
		}
	}

	/* Cumulative index rebuild. */
	{
		cw_uint32_t	i, j;

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_a, TRUE);
			for (j = 0; buf_size_get(buf_d) > 349; j++) {
				buf_head_data_release(buf_d, 349);
				buf_uint8_get(buf_d, buf_size_get(buf_d) - 1);
			}
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_b, TRUE);
			for (j = 0; buf_size_get(buf_d) > 349; j++) {
				buf_head_data_release(buf_d, 349);
				buf_uint8_get(buf_d, buf_size_get(buf_d) - 1);
			}
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_c, TRUE);
			for (j = 0; buf_size_get(buf_d) > 349; j++) {
				buf_head_data_release(buf_d, 349);
				buf_uint8_get(buf_d, buf_size_get(buf_d) - 1);
			}
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}
	}

	/* buf_iovec_get(). */
	{
		cw_uint32_t	i, j;
		int		iov_cnt;

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_a, TRUE);
			for (j = 0; buf_size_get(buf_d) > 11; j++) {
				buf_head_data_release(buf_d, 11);
				buf_iovec_get(buf_d, 0, FALSE, &iov_cnt);
			}
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_b, TRUE);
			for (j = 0; buf_size_get(buf_d) > 11; j++) {
				buf_head_data_release(buf_d, 11);
				buf_iovec_get(buf_d, 0, FALSE, &iov_cnt);
			}
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}

		for (i = 0; i < 1; i++) {
			buf_buf_catenate(buf_d, buf_c, TRUE);
			for (j = 0; buf_size_get(buf_d) > 11; j++) {
				buf_head_data_release(buf_d, 11);
				buf_iovec_get(buf_d, 0, FALSE, &iov_cnt);
			}
			buf_head_data_release(buf_d, buf_size_get(buf_d));
		}
	}

	buf_delete(buf_a);
	buf_delete(buf_b);
	buf_delete(buf_c);
	buf_delete(buf_d);

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}