/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Multi-threaded buf test.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#include <limits.h>

/*
 * (_CW_TEST_NUM_BUFELS * _CW_TEST_SIZEOF_BUFFER *
 * _CW_TEST_NUM_CIRCULATIONS) must fit within a 32 bit unsigned
 * variable.
 */

#define _CW_TEST_DATA_MODULUS 103
#define _CW_TEST_NUM_BUFELS 251
#define _CW_TEST_SIZEOF_BUFFER 283
#define _CW_TEST_NUM_CIRCULATIONS 53

struct foo_s {
	cw_buf_t	*buf_a;
	cw_buf_t	*buf_b;
	cw_mtx_t	*rand_lock;
	char		*thread_name;
};

void *
thread_entry_func(void *a_arg)
{
	struct foo_s	*foo_struct = (struct foo_s *)a_arg;
	cw_uint32_t	i, size, split;
	cw_buf_t	*buf;

	buf = buf_new(NULL, cw_g_mem);

	for (i = 0; i < (_CW_TEST_NUM_BUFELS *
	    _CW_TEST_SIZEOF_BUFFER * _CW_TEST_NUM_CIRCULATIONS);
	     /* Increment in the body. */ ) {
/*  		out_put(out_err, "[s]", foo_struct->thread_name); */

		size = buf_size_get(foo_struct->buf_a);

		if (i <= (_CW_TEST_NUM_BUFELS *
		    _CW_TEST_SIZEOF_BUFFER *
		    (_CW_TEST_NUM_CIRCULATIONS - 1)))
			buf_buf_catenate(buf, foo_struct->buf_a, FALSE);
		else if ((size + i) < (_CW_TEST_NUM_BUFELS *
		    _CW_TEST_SIZEOF_BUFFER *
		    _CW_TEST_NUM_CIRCULATIONS))
			buf_split(buf, foo_struct->buf_a, size);
		else {
			buf_split(buf, foo_struct->buf_a,
			    ((_CW_TEST_NUM_BUFELS *
			    _CW_TEST_SIZEOF_BUFFER *
			    _CW_TEST_NUM_CIRCULATIONS) - i));
		}

		size = buf_size_get(buf);
		i += size;
		if (size > 0) {
			mtx_lock(foo_struct->rand_lock);
			split = random() % size;
			mtx_unlock(foo_struct->rand_lock);

			buf_split(foo_struct->buf_b, buf, split);
			buf_buf_catenate(foo_struct->buf_b, buf, FALSE);
		} else {
			/* Hope for a context switch. */
			thd_yield();
		}
	}

	_cw_assert((_CW_TEST_NUM_BUFELS * _CW_TEST_SIZEOF_BUFFER *
	    _CW_TEST_NUM_CIRCULATIONS) == i);
	_cw_assert(buf_size_get(buf) == 0);

	buf_delete(buf);

	return NULL;
}

int
main(int argc, char **argv)
{
	cw_buf_t	*buf_a, buf_b;
	cw_bufc_t	*bufc;
	cw_uint32_t	i, j, n;
	char		*buffer;
	struct foo_s	foo_a, foo_b;
	cw_thd_t	*thd_a, *thd_b;
	cw_uint32_t	c;
	cw_mtx_t	rand_lock;
	cw_uint32_t	seed;

	libstash_init();
	out_put(out_err, "Test begin\n");

	/* Create a buf with a known pattern of data in it. */
	buf_a = buf_new_r(NULL, cw_g_mem);

	for (i = n = 0; i < _CW_TEST_NUM_BUFELS; i++) {
		/* Create a bufc, fill it with data, and append it to buf_a. */
		bufc = bufc_new(NULL, cw_g_mem, NULL, NULL);
		buffer = _cw_malloc(_CW_TEST_SIZEOF_BUFFER);
		for (j = 0; j < _CW_TEST_SIZEOF_BUFFER; j++, n++)
			buffer[j] = (char)(n % _CW_TEST_DATA_MODULUS);
		bufc_buffer_set(bufc, (void *)buffer,
		    _CW_TEST_SIZEOF_BUFFER, FALSE, (cw_opaque_dealloc_t
		    *)mem_free_e, cw_g_mem);
		buf_bufc_append(buf_a, bufc, 0, _CW_TEST_SIZEOF_BUFFER);
		bufc_delete(bufc);
	}

	buf_new_r(&buf_b, cw_g_mem);
	mtx_new(&rand_lock);

	if (argc > 1)
		seed = strtoul(argv[1], NULL, 10);
	else
		seed = getpid();
/*  	out_put(out_err, "seed == [i]\n", seed); */
	srandom(seed);

	foo_a.buf_a = buf_a;
	foo_a.buf_b = &buf_b;
	foo_a.rand_lock = &rand_lock;
	foo_a.thread_name = ".";

	foo_b.buf_a = &buf_b;
	foo_b.buf_b = buf_a;
	foo_b.rand_lock = &rand_lock;
	foo_b.thread_name = "*";

	thd_a = thd_new(thread_entry_func, (void *)&foo_a, TRUE);
	thd_b = thd_new(thread_entry_func, (void *)&foo_b, TRUE);

	thd_join(thd_a);
	thd_join(thd_b);

	/* Make sure the data hasn't been corrupted. */
	if (buf_size_get(buf_a) != _CW_TEST_NUM_BUFELS *
	    _CW_TEST_SIZEOF_BUFFER) {
		out_put(out_err, "buf_size_get(buf_a) == [i] (should be [i])\n",
		    buf_size_get(buf_a), _CW_TEST_NUM_BUFELS *
		    _CW_TEST_SIZEOF_BUFFER);
		buf_dump(buf_a, "buf_a ");
		out_put(out_err, "seed == [i]\n", seed);
	}
	if (buf_size_get(&buf_b) != 0) {
		out_put(out_err, "buf_size_get(&buf_b) == [i] (should be 0)\n",
		    buf_size_get(&buf_b));
		buf_dump(&buf_b, "buf_b ");
		out_put(out_err, "seed == [i]\n", seed);
	}
	for (i = 0; i < buf_size_get(buf_a); i++) {
		c = (cw_uint32_t)buf_uint8_get(buf_a, i);

		if (c != i % _CW_TEST_DATA_MODULUS) {
			out_put(out_err, "buf_a[[[i]] == %u, should be %u\n", i,
			    c, i % _CW_TEST_DATA_MODULUS);
			buf_dump(buf_a, "buf_a ");
			out_put(out_err, "seed == [i]\n", seed);
			break;
		}
	}

	buf_delete(buf_a);
	buf_delete(&buf_b);
	mtx_delete(&rand_lock);

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}