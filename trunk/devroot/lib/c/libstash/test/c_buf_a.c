/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * bufel and buf test.
 *
 * XXX Need multithreaded tests for buf.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_BUF
#include <libstash/libstash_r.h>

void
free_buffer(void * a_blah, void * a_buffer);

void
free_buffer(void * a_blah, void * a_buffer)
{
  _cw_check_ptr(a_buffer);

  _cw_free(a_buffer);
}

int
main()
{
/*    libstash_init(); */

  /* bufpool_new(), bufpool_delete(). */
  {
    cw_bufpool_t bufpool, * bufpool_p;

    _cw_assert(&bufpool == bufpool_new(&bufpool, 123, 7));
    bufpool_delete(&bufpool);

    bufpool_p = bufpool_new(NULL, 234, 11);
    _cw_check_ptr(bufpool_p);
    bufpool_delete(bufpool_p);
  }
  
  /* bufpool_get_buffer_size(). */
  {
    cw_bufpool_t bufpool;

    bufpool_new(&bufpool, 123, 7);
    _cw_assert(123 == bufpool_get_buffer_size(&bufpool));
    bufpool_delete(&bufpool);
    
    bufpool_new(&bufpool, 123, 10);
    _cw_assert(123 == bufpool_get_buffer_size(&bufpool));
    bufpool_delete(&bufpool);
    
    bufpool_new(&bufpool, 4096, 1024);
    _cw_assert(4096 == bufpool_get_buffer_size(&bufpool));
    bufpool_delete(&bufpool);
  }
  
  /* bufpool_get_max_spare_buffers(), bufpool_set_max_spare_buffers(),
   * bufpool_get_buffer(), bufpool_put_buffer(). */
  {
    cw_bufpool_t bufpool;
    void * pointers[100];
    cw_uint32_t i;

    bufpool_new(&bufpool, 4096, 10);
    _cw_assert(10 == bufpool_get_max_spare_buffers(&bufpool));
    for (i = 0; i < 100; i++)
    {
      pointers[i] = bufpool_get_buffer(&bufpool);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 100; i++)
    {
      bufpool_put_buffer(&bufpool, pointers[i]);
      pointers[i] = NULL;
    }
    for (i = 0; i < 100; i++)
    {
      pointers[i] = bufpool_get_buffer(&bufpool);
      _cw_check_ptr(pointers[i]);
    }
    
    bufpool_set_max_spare_buffers(&bufpool, 500);
    _cw_assert(500 == bufpool_get_max_spare_buffers(&bufpool));
    for (i = 0; i < 100; i++)
    {
      bufpool_put_buffer(&bufpool, pointers[i]);
      pointers[i] = NULL;
    }
    for (i = 0; i < 100; i++)
    {
      pointers[i] = bufpool_get_buffer(&bufpool);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 100; i++)
    {
      bufpool_put_buffer(&bufpool, pointers[i]);
      pointers[i] = NULL;
    }
    
    bufpool_set_max_spare_buffers(&bufpool, 0);
    _cw_assert(0 == bufpool_get_max_spare_buffers(&bufpool));
    for (i = 0; i < 100; i++)
    {
      pointers[i] = bufpool_get_buffer(&bufpool);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 100; i++)
    {
      bufpool_put_buffer(&bufpool, pointers[i]);
      pointers[i] = NULL;
    }
    for (i = 0; i < 100; i++)
    {
      pointers[i] = bufpool_get_buffer(&bufpool);
      _cw_check_ptr(pointers[i]);
    }
    
    bufpool_set_max_spare_buffers(&bufpool, 49);
    _cw_assert(49 == bufpool_get_max_spare_buffers(&bufpool));
    for (i = 0; i < 100; i++)
    {
      bufpool_put_buffer(&bufpool, pointers[i]);
      pointers[i] = NULL;
    }
    for (i = 0; i < 100; i++)
    {
      pointers[i] = bufpool_get_buffer(&bufpool);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 100; i++)
    {
      bufpool_put_buffer(&bufpool, pointers[i]);
      pointers[i] = NULL;
    }
  }
  
  /* bufel_new(), bufel_delete(). */
  {
    cw_bufel_t bufel, * bufel_p;

    _cw_assert(&bufel == bufel_new(&bufel));
    bufel_delete(&bufel);

    bufel_p = bufel_new(NULL);
    _cw_check_ptr(bufel_p);
    bufel_delete(bufel_p);
  }

  /* bufel_get_size(), bufel_set_data_ptr(). */
  {
    cw_bufel_t * bufel_p;
    cw_bufpool_t bufpool;
    void * buffer;

    bufpool_new(&bufpool, 4096, 10);
    
    bufel_p = bufel_new(NULL);
    _cw_assert(0 == bufel_get_size(bufel_p));

    buffer = bufpool_get_buffer(&bufpool);
    
    bufel_set_data_ptr(bufel_p,
		       buffer,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);
    _cw_assert(4096 == bufel_get_size(bufel_p));

    /* In order to test error conditions on bufel_set_data_ptr(), we must use
     * bufel_set_end_offset(), which isn't tested until later on. */
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 4096));
    _cw_assert(TRUE == bufel_set_end_offset(bufel_p, 4097));
    
    bufel_delete(bufel_p);
    bufpool_delete(&bufpool);
  }

  /* bufel_get_beg_offset(), bufel_set_beg_offset(),
   * bufel_get_end_offset(), bufel_set_end_offset(),
   * bufel_get_valid_data_size(). */
  {
    cw_bufel_t * bufel_p;
    cw_bufpool_t bufpool;
    void * buffer;

    bufpool_new(&bufpool, 4096, 10);

    bufel_p = bufel_new(NULL);

    buffer = bufpool_get_buffer(&bufpool);
    
    bufel_set_data_ptr(bufel_p,
		       buffer,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);

    _cw_assert(0 == bufel_get_beg_offset(bufel_p));
    _cw_assert(4096 == bufel_get_end_offset(bufel_p));
    _cw_assert(4096 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(TRUE == bufel_set_end_offset(bufel_p, 4097));
    _cw_assert(4096 == bufel_get_end_offset(bufel_p));

    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 2048));
    _cw_assert(2048 == bufel_get_end_offset(bufel_p));
    _cw_assert(2048 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 3500));
    _cw_assert(3500 == bufel_get_end_offset(bufel_p));
    _cw_assert(3500 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(TRUE == bufel_set_beg_offset(bufel_p, 3501));
    _cw_assert(0 == bufel_get_beg_offset(bufel_p));

    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 2048));
    _cw_assert(FALSE == bufel_set_beg_offset(bufel_p, 2048));
    _cw_assert(2048 == bufel_get_beg_offset(bufel_p));
    _cw_assert(0 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(FALSE == bufel_set_beg_offset(bufel_p, 2047));
    _cw_assert(2047 == bufel_get_beg_offset(bufel_p));
    _cw_assert(1 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(TRUE == bufel_set_end_offset(bufel_p, 2046));
    _cw_assert(2048 == bufel_get_end_offset(bufel_p));
    _cw_assert(1 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 2047));
    _cw_assert(2047 == bufel_get_end_offset(bufel_p));
    _cw_assert(0 == bufel_get_valid_data_size(bufel_p));
    
    bufel_delete(bufel_p);
    bufpool_delete(&bufpool);
  }
  
  /* bufel_get_data_ptr(), bufel_set_data_ptr(). */
  {
    cw_bufel_t * bufel_p;
    char * string;
    cw_bufpool_t bufpool;

    bufpool_new(&bufpool, 50, 10);

    bufel_p = bufel_new(NULL);

    string = (char *) bufpool_get_buffer(&bufpool);
    strcpy(string, "Hi ho hum.");
    
    _cw_assert(NULL == bufel_get_data_ptr(bufel_p));
    
    bufel_set_data_ptr(bufel_p,
		       (void *) string,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);
    _cw_assert((void *) string == bufel_get_data_ptr(bufel_p));
    _cw_assert(0 == bufel_get_beg_offset(bufel_p));
    _cw_assert(50 == bufel_get_end_offset(bufel_p));
    _cw_assert(50 == bufel_get_valid_data_size(bufel_p));
    
    bufel_delete(bufel_p);
    bufpool_delete(&bufpool);
  }

  /* buf_get_uint8(), buf_get_uint32(), buf_get_uint64(). */
  {
    cw_bufel_t * bufel_p;
    cw_buf_t * buf_p;
    cw_uint32_t i, j;
    cw_bufpool_t bufpool;
    char * buffer;
    cw_uint32_t * buffer_cast;
    char buf[17];

    bufpool_new(&bufpool, 512, 10);

    bufel_p = bufel_new(NULL);
    buf_p = buf_new(NULL, TRUE);

    buffer = (char *) bufpool_get_buffer(&bufpool);
    buffer_cast = (cw_uint32_t *) buffer;
    bufel_set_data_ptr(bufel_p,
		       buffer,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 512));

    for (i = 0; i < 256; i++)
    {
      buffer[i] = i;
    }

    /* Copy the bytes from the lower 256 bytes into the upper 256 bytes,
     * but reverse them, and make them host byte order on long boundaries. */
    for (i = j = 0; i < 256; i += 4, j++)
    {
      buffer_cast[128 - 1 - j] = ntohl(buffer_cast[j]);
    }

    buf_append_bufel(buf_p, bufel_p);
    
    log_printf(cw_g_log, "lower char dump:\n");
    for (i = 0; i < 256; i += 8)
    {
      log_printf(cw_g_log,
		 "%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:"
		 "%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x\n",
		 i, buf_get_uint8(buf_p, i),
		 i + 1, buf_get_uint8(buf_p, i + 1),
		 i + 2, buf_get_uint8(buf_p, i + 2),
		 i + 3, buf_get_uint8(buf_p, i + 3),
		 i + 4, buf_get_uint8(buf_p, i + 4),
		 i + 5, buf_get_uint8(buf_p, i + 5),
		 i + 6, buf_get_uint8(buf_p, i + 6),
		 i + 7, buf_get_uint8(buf_p, i + 7));
    }

    log_printf(cw_g_log, "upper long dump:\n");
    for (i = 256; i < 512; i += 8)
    {
      log_printf(cw_g_log, "%03u->0x%08x:%03u->0x%08x\n",
		 i, buf_get_uint32(buf_p, i),
		 i + 4, buf_get_uint32(buf_p, i + 4));
    }

    log_printf(cw_g_log, "upper quad dump:\n");
    for (i = 256; i < 512; i += 8)
    {
      log_printf(cw_g_log, "%03u->0x%s\n",
		 i, log_print_uint64(buf_get_uint64(buf_p, i), 16, buf));
    }
    
    /* Unaligned gets. */
    log_printf(cw_g_log, "Unaligned buf_get_uint32():\n");
    for (i = 1; i < 4; i++)
    {
      log_printf(cw_g_log, "%03u->0x%08x\n",
		 256 + i, buf_get_uint32(buf_p, 256 + i));
    }
    
    log_printf(cw_g_log, "Unaligned buf_get_uint64():\n");
    for (i = 1; i < 8; i++)
    {
      log_printf(cw_g_log, "%03u->0x%s\n",
		 256 + i,
		 log_print_uint64(buf_get_uint64(buf_p, 256 + i), 16, buf));
    }
    
    buf_delete(buf_p);
    bufel_delete(bufel_p);
    bufpool_delete(&bufpool);
  }

  /* buf_get_uint32(), buf_get_uint64(). */
  {
    cw_bufpool_t bufpool;
    cw_buf_t buf;
    cw_bufel_t bufel;
    cw_uint8_t * a, b[2], c[3], * d, * e, f[9];
    cw_uint8_t t_buf[17];
    cw_uint32_t i;

    bufpool_new(&bufpool, 5, 10);
    buf_new(&buf, TRUE);

    a = (cw_uint8_t *) _cw_malloc(1);
    a[0] = 0;
    bufel_new(&bufel);
    bufel_set_data_ptr(&bufel, (void *) a, 1, free_buffer, NULL);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    b[0] = 1;
    b[1] = 2;
    bufel_new(&bufel);
    bufel_set_data_ptr(&bufel, (void *) b, 2, NULL, NULL);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    c[0] = 3;
    c[1] = 4;
    c[2] = 5;
    bufel_new(&bufel);
    bufel_set_data_ptr(&bufel, (void *) c, 3, NULL, NULL);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    d = (cw_uint8_t *) _cw_malloc(4);
    d[0] = 6;
    d[1] = 7;
    d[2] = 8;
    d[3] = 9;
    bufel_new(&bufel);
    bufel_set_data_ptr(&bufel, (void *) d, 4, free_buffer, NULL);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    e = bufpool_get_buffer(&bufpool);
    e[0] = 10;
    e[1] = 11;
    e[2] = 12;
    e[3] = 13;
    e[4] = 14;
    bufel_new(&bufel);
    bufel_set_data_ptr(&bufel, (void *) e, 5, bufpool_put_buffer, &bufpool);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);
    
    f[0] = 15;
    f[1] = 16;
    f[2] = 17;
    f[3] = 18;
    f[4] = 19;
    f[5] = 20;
    f[6] = 21;
    f[7] = 22;
    f[8] = 23;
    bufel_new(&bufel);
    bufel_set_data_ptr(&bufel, (void *) c, 9, NULL, NULL);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    _cw_assert(24 == buf_get_size(&buf));

    log_printf(cw_g_log, "Hodge podge buf_get_uint32():\n");
    for (i = 0; i < 20; i++)
    {
      log_printf(cw_g_log, "%03u->0x%08x\n",
		 i, buf_get_uint32(&buf, i));
    }
    
    log_printf(cw_g_log, "Hodge podge buf_get_uint64():\n");
    for (i = 0; i < 16; i++)
    {
      log_printf(cw_g_log, "%03u->0x%s\n",
		 i,
		 log_print_uint64(buf_get_uint64(&buf, i), 16, t_buf));
    }

    buf_delete(&buf);
    bufpool_delete(&bufpool);
  }
  
  /* buf_new(), buf_delete(). */
  {
    cw_buf_t buf, * buf_p;

    _cw_assert(&buf == buf_new(&buf, FALSE));
    buf_delete(&buf);

    _cw_assert(&buf == buf_new(&buf, TRUE));
    buf_delete(&buf);

    buf_p = buf_new(NULL, FALSE);
    _cw_check_ptr(buf_p);
    buf_delete(buf_p);

    buf_p = buf_new(NULL, TRUE);
    _cw_check_ptr(buf_p);
    buf_delete(buf_p);
  }

  
  /* buf_prepend_bufel(), buf_append_bufel(),
     buf_get_size(). */
  {
    cw_buf_t * buf_p;
    cw_bufel_t * bufel_p_a, * bufel_p_b, * bufel_p_c;
    char * str_a, * str_b, * str_c;
    cw_bufpool_t bufpool;

    bufpool_new(&bufpool, 1, 2);

    str_a = (char *) bufpool_get_buffer(&bufpool);
    str_a[0] = 'A';
    
    str_b = (char *) bufpool_get_buffer(&bufpool);
    str_b[0] = 'B';
    
    str_c = (char *) bufpool_get_buffer(&bufpool);
    str_c[0] = 'C';

    buf_p = buf_new(NULL, TRUE);
    _cw_assert(0 == buf_get_size(buf_p));

    bufel_p_a = bufel_new(NULL);
    bufel_set_data_ptr(bufel_p_a,
		       (void *) str_a,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);
    
    bufel_p_b = bufel_new(NULL);
    bufel_set_data_ptr(bufel_p_b,
		       (void *) str_b,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);
    
    bufel_p_c = bufel_new(NULL);
    bufel_set_data_ptr(bufel_p_c,
		       (void *) str_c,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);

    buf_append_bufel(buf_p, bufel_p_a);
    _cw_assert(1 == buf_get_size(buf_p));
    buf_append_bufel(buf_p, bufel_p_b);
    _cw_assert(2 == buf_get_size(buf_p));
    buf_append_bufel(buf_p, bufel_p_c);
    _cw_assert(3 == buf_get_size(buf_p));

    buf_prepend_bufel(buf_p, bufel_p_a);
    _cw_assert(4 == buf_get_size(buf_p));
    buf_prepend_bufel(buf_p, bufel_p_b);
    _cw_assert(5 == buf_get_size(buf_p));
    buf_prepend_bufel(buf_p, bufel_p_c);
    _cw_assert(6 == buf_get_size(buf_p));

    _cw_assert('C' == buf_get_uint8(buf_p, 0));
/*      buf_dump(buf_p, "H "); */
    _cw_assert('B' == buf_get_uint8(buf_p, 1));
    _cw_assert('A' == buf_get_uint8(buf_p, 2));
    _cw_assert('A' == buf_get_uint8(buf_p, 3));
    _cw_assert('B' == buf_get_uint8(buf_p, 4));
    _cw_assert('C' == buf_get_uint8(buf_p, 5));

    bufel_delete(bufel_p_a);
    bufel_delete(bufel_p_b);
    bufel_delete(bufel_p_c);
    buf_delete(buf_p);
    bufpool_delete(&bufpool);
  }
  
  /* buf_catenate_buf(),
     buf_get_size(). */
  {
    cw_buf_t * buf_p_a, * buf_p_b;
    cw_bufel_t * bufel_p_a, * bufel_p_b, * bufel_p_c;
    char * str_a, * str_b, * str_c;
    cw_bufpool_t bufpool;

    bufpool_new(&bufpool, 1, 2);

    str_a = (char *) bufpool_get_buffer(&bufpool);
    str_a[0] = 'A';
    
    str_b = (char *) bufpool_get_buffer(&bufpool);
    str_b[0] = 'B';
    
    str_c = (char *) bufpool_get_buffer(&bufpool);
    str_c[0] = 'C';

    buf_p_a = buf_new(NULL, TRUE);
    buf_p_b = buf_new(NULL, FALSE); /* XXX */
    _cw_assert(0 == buf_get_size(buf_p_a));
    _cw_assert(0 == buf_get_size(buf_p_b));

    bufel_p_a = bufel_new(NULL);
    bufel_set_data_ptr(bufel_p_a,
		       (void *) str_a,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);
    
    bufel_p_b = bufel_new(NULL);
    bufel_set_data_ptr(bufel_p_b,
		       (void *) str_b,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);
    
    bufel_p_c = bufel_new(NULL);
    bufel_set_data_ptr(bufel_p_c,
		       (void *) str_c,
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);

    buf_append_bufel(buf_p_a, bufel_p_a);
    buf_append_bufel(buf_p_a, bufel_p_b);
    buf_append_bufel(buf_p_b, bufel_p_c);
    _cw_assert(2 == buf_get_size(buf_p_a));
    _cw_assert(1 == buf_get_size(buf_p_b));
    buf_catenate_buf(buf_p_a, buf_p_b, TRUE);
    _cw_assert(3 == buf_get_size(buf_p_a));
    _cw_assert(1 == buf_get_size(buf_p_b));
    buf_catenate_buf(buf_p_a, buf_p_b, FALSE);
    _cw_assert(4 == buf_get_size(buf_p_a));
    _cw_assert(0 == buf_get_size(buf_p_b));

    bufel_delete(bufel_p_a);
    bufel_delete(bufel_p_b);
    bufel_delete(bufel_p_c);
    buf_delete(buf_p_a);
    buf_delete(buf_p_b);
    bufpool_delete(&bufpool);
  }

  /* buf_get_iovec(). */
  /* buf_split(). */
  /* buf_release_head_data(), buf_release_tail_data(). */

/*    libstash_shutdown(); */
  return 0;
}
