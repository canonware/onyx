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
 * Multi-threaded buf test.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_BUF
#include <libstash/libstash_r.h>

#include <limits.h>

/* (_LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER *
 *  * _LIBSTASH_TEST_NUM_CIRCULATIONS) must fit within a 32 bit unsigned
 * variable. */

#define _LIBSTASH_TEST_DATA_MODULUS 103
#define _LIBSTASH_TEST_NUM_BUFELS 251
#define _LIBSTASH_TEST_SIZEOF_BUFFER 283
#define _LIBSTASH_TEST_NUM_CIRCULATIONS 53

struct foo_s
{
  cw_buf_t * buf_a;
  cw_buf_t * buf_b;
  cw_mtx_t * rand_lock;
  char * thread_name;
};

void *
thread_entry_func(void * a_arg)
{
  struct foo_s * foo_struct = (struct foo_s *) a_arg;
  cw_uint32_t i, size, split;
  cw_buf_t * buf;

  buf = buf_new(NULL);
  
  for (i = 0;
       i < (_LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER
	    * _LIBSTASH_TEST_NUM_CIRCULATIONS);
       /* Increment in the body. */)
  {
/*      out_put(cw_g_out, "[s]", foo_struct->thread_name); */

    size = buf_get_size(foo_struct->buf_a);

    if (i <= (_LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER
	      * (_LIBSTASH_TEST_NUM_CIRCULATIONS - 1)))
    {
      buf_catenate_buf(buf, foo_struct->buf_a, FALSE);
    }
    else if ((size + i) < (_LIBSTASH_TEST_NUM_BUFELS *
			   _LIBSTASH_TEST_SIZEOF_BUFFER 
			   * _LIBSTASH_TEST_NUM_CIRCULATIONS))
    {
      buf_split(buf, foo_struct->buf_a, size);
    }
    else
    {
      buf_split(buf, foo_struct->buf_a, ((_LIBSTASH_TEST_NUM_BUFELS *
					  _LIBSTASH_TEST_SIZEOF_BUFFER 
					  * _LIBSTASH_TEST_NUM_CIRCULATIONS)
					 - i));
    }

    size = buf_get_size(buf);
    i += size;
    if (0 < size)
    {
      mtx_lock(foo_struct->rand_lock);
      split = random() % size;
      mtx_unlock(foo_struct->rand_lock);

      buf_split(foo_struct->buf_b, buf, split);
      buf_catenate_buf(foo_struct->buf_b, buf, FALSE);
    }
    else
    {
      /* Hope for a context switch. */
      thd_yield();
    }
  }

  _cw_assert(i == (_LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER
		   * _LIBSTASH_TEST_NUM_CIRCULATIONS));
  _cw_assert(0 == buf_get_size(buf));
  
  buf_delete(buf);
  
  return NULL;
}

int
main(int argc, char ** argv)
{
  cw_buf_t * buf_a, buf_b;
  cw_bufc_t * bufc;
  cw_uint32_t i, j, n;
  char * buffer;
  struct foo_s foo_a, foo_b;
  cw_thd_t thd_a, thd_b;
  cw_uint32_t c;
  cw_mtx_t rand_lock;
  cw_uint32_t seed;
  
  libstash_init();
  out_put(cw_g_out, "Test begin\n");

/*    dbg_register(cw_g_dbg, "mem_verbose"); */

  /* Create a buf with a known pattern of data in it. */
  buf_a = buf_new_r(NULL);

  for (i = n = 0; i < _LIBSTASH_TEST_NUM_BUFELS; i++)
  {
    /* Create a bufc, fill it with data, and append it to buf_a. */
    bufc = bufc_new(NULL, NULL, NULL);
    buffer = _cw_malloc(_LIBSTASH_TEST_SIZEOF_BUFFER);
    for (j = 0; j < _LIBSTASH_TEST_SIZEOF_BUFFER; j++, n++)
    {
      buffer[j] = (char) (n % _LIBSTASH_TEST_DATA_MODULUS);
    }
    bufc_set_buffer(bufc,
		    (void *) buffer,
		    _LIBSTASH_TEST_SIZEOF_BUFFER,
		    FALSE,
		    mem_dealloc,
		    cw_g_mem);
    buf_append_bufc(buf_a, bufc, 0, _LIBSTASH_TEST_SIZEOF_BUFFER);
    bufc_delete(bufc);
  }

  buf_new_r(&buf_b);
  mtx_new(&rand_lock);

  if (argc > 1)
  {
    seed = strtoul(argv[1], NULL, 10);
  }
  else
  {
    seed = getpid();
  }
/*    out_put(cw_g_out, "seed == [i]\n", seed); */
  srandom(seed);
  
  foo_a.buf_a = buf_a;
  foo_a.buf_b = &buf_b;
  foo_a.rand_lock = &rand_lock;
  foo_a.thread_name = ".";
  
  foo_b.buf_a = &buf_b;
  foo_b.buf_b = buf_a;
  foo_b.rand_lock = &rand_lock;
  foo_b.thread_name = "*";
  
  thd_new(&thd_a, thread_entry_func, (void *) &foo_a);
  thd_new(&thd_b, thread_entry_func, (void *) &foo_b);

  thd_join(&thd_a);
  thd_join(&thd_b);

  /* Make sure the data hasn't been corrupted. */
  if (_LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER
      != buf_get_size(buf_a))
  {
    out_put(cw_g_out, "buf_get_size(buf_a) == [i] (should be [i])\n",
	    buf_get_size(buf_a),
	    _LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER);
    buf_dump(buf_a, "buf_a ");
    out_put(cw_g_out, "seed == [i]\n", seed);
  }
  if (0 != buf_get_size(&buf_b))
  {
    out_put(cw_g_out, "buf_get_size(&buf_b) == [i] (should be 0)\n",
	    buf_get_size(&buf_b));
    buf_dump(&buf_b, "buf_b ");
    out_put(cw_g_out, "seed == [i]\n", seed);
  }
  
  for (i = 0; i < buf_get_size(buf_a); i++)
  {
    c = (cw_uint32_t) buf_get_uint8(buf_a, i);
    
    if (c != i % _LIBSTASH_TEST_DATA_MODULUS)
    {
      out_put(cw_g_out, "buf_a[[[i]] == %u, should be %u\n",
	      i, c, i % _LIBSTASH_TEST_DATA_MODULUS);
      buf_dump(buf_a, "buf_a ");
      out_put(cw_g_out, "seed == [i]\n", seed);
      break;
    }
  }

  buf_delete(buf_a);
  buf_delete(&buf_b);
  mtx_delete(&rand_lock);
  
  out_put(cw_g_out, "Test end\n");
  libstash_shutdown();
  return 0;
}
