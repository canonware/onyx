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
#ifdef _CW_OS_SOLARIS
/*  #  include <sched.h> */
#endif

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

  buf = buf_new(NULL, FALSE);
  
  for (i = 0;
       i < (_LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER
	    * _LIBSTASH_TEST_NUM_CIRCULATIONS);
       /* Increment in the body. */)
  {
/*      log_printf(cw_g_log, "%s", foo_struct->thread_name); */

    size = buf_get_size(foo_struct->buf_a);

    if (size <= ((_LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER
		 * _LIBSTASH_TEST_NUM_CIRCULATIONS) - i))
    {
      buf_catenate_buf(buf, foo_struct->buf_a, FALSE);
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
#ifdef _CW_OS_FREEBSD
      pthread_yield();
#endif
#ifdef _CW_OS_SOLARIS
      /* XXX This is what we need, but it requires -lposix4 for linking, and I'm
       * too lazy to deal with it right now, especially since this really
       * belongs in the thd class. */
/*        sched_yield(); */
#endif
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
  cw_bufel_t * bufel;
  cw_bufc_t * bufc;
  cw_uint32_t i, j, n;
  char * buffer;
  struct foo_s foo_a, foo_b;
  cw_thd_t thd_a, thd_b;
  cw_uint32_t c;
  cw_mtx_t rand_lock;
  cw_uint32_t seed;
  
  libstash_init();
  log_printf(cw_g_log, "Test begin\n");

/*    dbg_register(cw_g_dbg, "mem_verbose"); */

  /* Create a buf with a known pattern of data in it. */
  buf_a = buf_new(NULL, TRUE);

  for (i = n = 0; i < _LIBSTASH_TEST_NUM_BUFELS; i++)
  {
    /* Create a bufel, fill it with data, and append it to buf_a. */
    bufel = bufel_new(NULL, NULL, NULL);
    bufc = bufc_new(NULL, NULL, NULL);
    buffer = _cw_malloc(_LIBSTASH_TEST_SIZEOF_BUFFER);
    for (j = 0; j < _LIBSTASH_TEST_SIZEOF_BUFFER; j++, n++)
    {
      buffer[j] = (char) (n % _LIBSTASH_TEST_DATA_MODULUS);
    }
    bufc_set_buffer(bufc,
		    (void *) buffer,
		    _LIBSTASH_TEST_SIZEOF_BUFFER,
		    mem_dealloc,
		    cw_g_mem);
    bufel_set_bufc(bufel, bufc);
    buf_append_bufel(buf_a, bufel);
    bufel_delete(bufel);
  }

  buf_new(&buf_b, TRUE);
  mtx_new(&rand_lock);

  if (argc > 1)
  {
    seed = strtoul(argv[1], NULL, 10);
  }
  else
  {
    seed = getpid();
  }
/*    log_printf(cw_g_log, "seed == %lu\n", seed); */
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
    log_printf(cw_g_log, "buf_get_size(buf_a) == %lu (should be %lu)\n",
	       buf_get_size(buf_a),
	       _LIBSTASH_TEST_NUM_BUFELS * _LIBSTASH_TEST_SIZEOF_BUFFER);
    buf_dump(buf_a, "buf_a ");
    log_printf(cw_g_log, "seed == %lu\n", seed);
  }
  if (0 != buf_get_size(&buf_b))
  {
    log_printf(cw_g_log, "buf_get_size(&buf_b) == %lu (should be 0)\n",
	       buf_get_size(&buf_b));
    buf_dump(&buf_b, "buf_b ");
    log_printf(cw_g_log, "seed == %lu\n", seed);
  }
  
  for (i = 0; i < buf_get_size(buf_a); i++)
  {
    c = (cw_uint32_t) buf_get_uint8(buf_a, i);
    
    if (c != i % _LIBSTASH_TEST_DATA_MODULUS)
    {
      log_printf(cw_g_log, "buf_a[%u] == %u, should be %u\n",
		 i, c, i % _LIBSTASH_TEST_DATA_MODULUS);
      buf_dump(buf_a, "buf_a ");
      log_printf(cw_g_log, "seed == %lu\n", seed);
      break;
    }
  }

  buf_delete(buf_a);
  buf_delete(&buf_b);
  mtx_delete(&rand_lock);
  
  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
