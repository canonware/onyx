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

int
main()
{
  libstash_init();

/*    dbg_register(cw_g_dbg, "mem_verbose"); */

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
    
    bufpool_delete(&bufpool);
  }
  
  /* bufc_new(), bufc_delete(), bufc_set_buffer(). */
  {
    cw_bufc_t bufc_a, * bufc_b;
    void * buffer;
    char str[512];

    _cw_assert(&bufc_a == bufc_new(&bufc_a, NULL, NULL));
    bufc_delete(&bufc_a);
    
    _cw_assert(&bufc_a == bufc_new(&bufc_a, NULL, NULL));
    bufc_set_buffer(&bufc_a, str, 512, NULL, NULL);
    bufc_delete(&bufc_a);

    bufc_b = bufc_new(NULL, NULL, NULL);
    _cw_check_ptr(bufc_b);
    bufc_delete(bufc_b);
    
    bufc_b = _cw_malloc(sizeof(cw_bufc_t));
    _cw_assert(bufc_b == bufc_new(bufc_b, mem_dealloc, cw_g_mem));
    bufc_delete(bufc_b);
    
    bufc_b = _cw_malloc(sizeof(cw_bufc_t));
    _cw_assert (bufc_b == bufc_new(bufc_b, mem_dealloc, cw_g_mem));
    buffer = _cw_malloc(789);
    bufc_set_buffer(bufc_b, buffer, 789, mem_dealloc, cw_g_mem);
    bufc_delete(bufc_b);
  }
  
  /* bufel_new(), bufel_delete(). */
  {
    cw_bufel_t bufel, * bufel_p;

    _cw_assert(&bufel == bufel_new(&bufel, NULL, NULL));
    bufel_delete(&bufel);

    bufel_p = bufel_new(NULL, NULL, NULL);
    _cw_check_ptr(bufel_p);
    bufel_delete(bufel_p);
  }

  /* bufel_get_size(), bufel_set_data_ptr(). */
  {
    cw_bufel_t * bufel_p;
    cw_bufpool_t bufpool;
    void * buffer;
    cw_bufc_t bufc;
    
    bufpool_new(&bufpool, 4096, 10);
    
    bufel_p = bufel_new(NULL, NULL, NULL);
    _cw_assert(0 == bufel_get_size(bufel_p));

    bufc_new(&bufc, NULL, NULL);
    buffer = bufpool_get_buffer(&bufpool);
    bufc_set_buffer(&bufc,
		      buffer,
		      bufpool_get_buffer_size(&bufpool),
		      bufpool_put_buffer,
		      (void *) &bufpool);

    bufel_set_bufc(bufel_p, &bufc);
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
    cw_bufc_t * bufc_p;

    bufpool_new(&bufpool, 4096, 10);

    bufel_p = bufel_new(NULL, NULL, NULL);

    bufc_p = bufc_new(NULL, NULL, NULL);
    buffer = bufpool_get_buffer(&bufpool);
    bufc_set_buffer(bufc_p,
		      buffer,
		      bufpool_get_buffer_size(&bufpool),
		      bufpool_put_buffer,
		      (void *) &bufpool);

    bufel_set_bufc(bufel_p, bufc_p);
    
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
    cw_bufc_t * bufc_p;
    char * string;
    cw_bufpool_t bufpool;

    bufpool_new(&bufpool, 50, 10);

    bufel_p = bufel_new(NULL, NULL, NULL);

    string = (char *) bufpool_get_buffer(&bufpool);
    strcpy(string, "Hi ho hum.");
    
    _cw_assert(NULL == bufel_get_data_ptr(bufel_p));

    bufc_p = _cw_malloc(sizeof(cw_bufc_t));
    bufc_new(bufc_p, mem_dealloc, cw_g_mem);
    bufc_set_buffer(bufc_p,
		      string,
		      bufpool_get_buffer_size(&bufpool),
		      bufpool_put_buffer,
		      (void *) &bufpool);
    bufel_set_bufc(bufel_p, bufc_p);
    
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
    cw_bufc_t bufc;
    cw_buf_t * buf_p;
    cw_uint32_t i, j;
    cw_bufpool_t bufpool;
    char * buffer;
    cw_uint32_t * buffer_cast;
    char buf[17];

    bufpool_new(&bufpool, 512, 10);

    bufel_p = bufel_new(NULL, NULL, NULL);
    buf_p = buf_new(NULL, TRUE);

    buffer = (char *) bufpool_get_buffer(&bufpool);
    buffer_cast = (cw_uint32_t *) buffer;

    bufc_new(&bufc, NULL, NULL);
    bufc_set_buffer(&bufc,
		    buffer,
		    bufpool_get_buffer_size(&bufpool),
		    bufpool_put_buffer,
		    (void *) &bufpool);

    bufel_set_bufc(bufel_p, &bufc);
    
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

  /* buf_get_uint32(), buf_get_uint64(),
   * buf_get_iovec(). */
  {
    cw_bufpool_t bufpool;
    cw_buf_t buf;
    cw_bufel_t bufel;
    cw_bufc_t * bufc_p;
    cw_uint8_t * a, b[2], c[3], * d, * e, f[11];
    cw_uint8_t t_buf[17];
    cw_uint32_t i;
    int iov_count;
    const struct iovec * iov;

    /* a  0,
     * b  1,  2,
     * c  3,  4,  5,
     * d  6,  7,  8,  9,
     * e 10, 11, 12, 13, 14,
     * f 42, 15, 16, 17, 18, 19, 20, 21, 22, 23, 42
     */

    bufpool_new(&bufpool, 5, 10);
    buf_new(&buf, TRUE);

    a = (cw_uint8_t *) _cw_malloc(1);
    a[0] = 0;
    bufel_new(&bufel, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p, a, 1, mem_dealloc, cw_g_mem);
    bufel_set_bufc(&bufel, bufc_p);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    b[0] = 1;
    b[1] = 2;
    bufel_new(&bufel, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p, (void *) b, 2, NULL, NULL);
    bufel_set_bufc(&bufel, bufc_p);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    c[0] = 3;
    c[1] = 4;
    c[2] = 5;
    bufel_new(&bufel, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p, (void *) c, 3, NULL, NULL);
    bufel_set_bufc(&bufel, bufc_p);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    d = (cw_uint8_t *) _cw_malloc(4);
    d[0] = 6;
    d[1] = 7;
    d[2] = 8;
    d[3] = 9;
    bufel_new(&bufel, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p, (void *) d, 4, mem_dealloc, cw_g_mem);
    bufel_set_bufc(&bufel, bufc_p);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    e = bufpool_get_buffer(&bufpool);
    e[0] = 10;
    e[1] = 11;
    e[2] = 12;
    e[3] = 13;
    e[4] = 14;
    bufel_new(&bufel, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p, (void *) e, 5, bufpool_put_buffer, &bufpool);
    bufel_set_bufc(&bufel, bufc_p);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

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
    bufel_new(&bufel, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p, (void *) f, 11, NULL, NULL);
    bufel_set_bufc(&bufel, bufc_p);
    bufel_set_beg_offset(&bufel, 1);
    bufel_set_end_offset(&bufel, 10);
    buf_append_bufel(&buf, &bufel);
    bufel_delete(&bufel);

    iov = buf_get_iovec(&buf, 0, &iov_count);
    _cw_assert(iov_count == 0);
    
    iov = buf_get_iovec(&buf, buf_get_size(&buf), &iov_count);
    _cw_assert(iov_count == 6);
    _cw_assert(iov[0].iov_base == (char *) a);
    _cw_assert(iov[0].iov_len == 1);
    _cw_assert(iov[1].iov_base == (char *) b);
    _cw_assert(iov[1].iov_len == 2);
    _cw_assert(iov[2].iov_base == (char *) c);
    _cw_assert(iov[2].iov_len == 3);
    _cw_assert(iov[3].iov_base == (char *) d);
    _cw_assert(iov[3].iov_len == 4);
    _cw_assert(iov[4].iov_base == (char *) e);
    _cw_assert(iov[4].iov_len == 5);
    _cw_assert(iov[5].iov_base == (char *) &f[1]);
    _cw_assert(iov[5].iov_len == 9);

    iov = buf_get_iovec(&buf, buf_get_size(&buf) + 10, &iov_count);
    _cw_assert(iov_count == 6);
    _cw_assert(iov[0].iov_base == (char *) a);
    _cw_assert(iov[0].iov_len == 1);
    _cw_assert(iov[1].iov_base == (char *) b);
    _cw_assert(iov[1].iov_len == 2);
    _cw_assert(iov[2].iov_base == (char *) c);
    _cw_assert(iov[2].iov_len == 3);
    _cw_assert(iov[3].iov_base == (char *) d);
    _cw_assert(iov[3].iov_len == 4);
    _cw_assert(iov[4].iov_base == (char *) e);
    _cw_assert(iov[4].iov_len == 5);
    _cw_assert(iov[5].iov_base == (char *) &f[1]);
    _cw_assert(iov[5].iov_len == 9);

    iov = buf_get_iovec(&buf, buf_get_size(&buf) - 5, &iov_count);
    _cw_assert(iov_count == 6);
    _cw_assert(iov[0].iov_base == (char *) a);
    _cw_assert(iov[0].iov_len == 1);
    _cw_assert(iov[1].iov_base == (char *) b);
    _cw_assert(iov[1].iov_len == 2);
    _cw_assert(iov[2].iov_base == (char *) c);
    _cw_assert(iov[2].iov_len == 3);
    _cw_assert(iov[3].iov_base == (char *) d);
    _cw_assert(iov[3].iov_len == 4);
    _cw_assert(iov[4].iov_base == (char *) e);
    _cw_assert(iov[4].iov_len == 5);
    _cw_assert(iov[5].iov_base == (char *) &f[1]);
    _cw_assert(iov[5].iov_len == 4);

    iov = buf_get_iovec(&buf, buf_get_size(&buf) - 15, &iov_count);
    _cw_assert(iov_count == 4);
    _cw_assert(iov[0].iov_base == (char *) a);
    _cw_assert(iov[0].iov_len == 1);
    _cw_assert(iov[1].iov_base == (char *) b);
    _cw_assert(iov[1].iov_len == 2);
    _cw_assert(iov[2].iov_base == (char *) c);
    _cw_assert(iov[2].iov_len == 3);
    _cw_assert(iov[3].iov_base == (char *) d);
    _cw_assert(iov[3].iov_len == 3);

    _cw_assert(24 == buf_get_size(&buf));

    log_printf(cw_g_log, "Hodge podge buf_get_uint32():\n");
    for (i = 0; i <= 20; i++)
    {
      log_printf(cw_g_log, "%03u->0x%08x\n",
		 i, buf_get_uint32(&buf, i));
    }
    
    log_printf(cw_g_log, "Hodge podge buf_get_uint64():\n");
    for (i = 0; i <= 16; i++)
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
    cw_bufc_t * bufc_p;
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

    bufel_p_a = bufel_new(NULL, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p,
		    (void *) str_a,
		    bufpool_get_buffer_size(&bufpool),
		    bufpool_put_buffer,
		    (void *) &bufpool);
    bufel_set_bufc(bufel_p_a, bufc_p);
    
    bufel_p_b = bufel_new(NULL, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p, 
		    (void *) str_b,
		    bufpool_get_buffer_size(&bufpool),
		    bufpool_put_buffer,
		    (void *) &bufpool);
    bufel_set_bufc(bufel_p_b, bufc_p);
    
    bufel_p_c = bufel_new(NULL, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p,
		    (void *) str_c,
		    bufpool_get_buffer_size(&bufpool),
		    bufpool_put_buffer,
		    (void *) &bufpool);
    bufel_set_bufc(bufel_p_c, bufc_p);

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
   * buf_get_size(). */
  {
    cw_buf_t * buf_p_a, * buf_p_b;
    cw_bufel_t * bufel_p_a, * bufel_p_b, * bufel_p_c;
    cw_bufc_t * bufc_p;
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

    bufel_p_a = bufel_new(NULL, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p,
		    (void *) str_a,
		    bufpool_get_buffer_size(&bufpool),
		    bufpool_put_buffer,
		    (void *) &bufpool);
    bufel_set_bufc(bufel_p_a, bufc_p);
    
    bufel_p_b = bufel_new(NULL, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p,
		    (void *) str_b,
		    bufpool_get_buffer_size(&bufpool),
		    bufpool_put_buffer,
		    (void *) &bufpool);
    bufel_set_bufc(bufel_p_b, bufc_p);
    
    bufel_p_c = bufel_new(NULL, NULL, NULL);
    bufc_p = bufc_new(NULL, NULL, NULL);
    bufc_set_buffer(bufc_p,
		    (void *) str_c,
		    bufpool_get_buffer_size(&bufpool),
		    bufpool_put_buffer,
		    (void *) &bufpool);
    bufel_set_bufc(bufel_p_c, bufc_p);
    
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

  /* buf_catenate_buf(). */
  {
    cw_bufpool_t bufpool;
    cw_buf_t buf_a, buf_b, buf_c;
    cw_bufel_t bufel;
    cw_bufc_t * bufc_p;
    cw_uint32_t i;

    bufpool_new(&bufpool, 8, 10);

    buf_new(&buf_a, TRUE);
    buf_new(&buf_b, FALSE);
    buf_new(&buf_c, TRUE);

    for (i = 0; i < 3; i++)
    {
      bufel_new(&bufel, NULL, NULL);
      bufc_p = bufc_new(NULL, NULL, NULL);
      bufc_set_buffer(bufc_p,
		      bufpool_get_buffer(&bufpool),
		      bufpool_get_buffer_size(&bufpool),
		      bufpool_put_buffer,
		      (void *) &bufpool);
      bufel_set_bufc(&bufel, bufc_p);
      buf_append_bufel(&buf_a, &bufel);
      buf_catenate_buf(&buf_c, &buf_b, FALSE);
      buf_catenate_buf(&buf_b, &buf_a, TRUE);
      bufel_delete(&bufel);
    }

    buf_delete(&buf_c);
    buf_new(&buf_c, TRUE);

    _cw_assert(24 == buf_get_size(&buf_a));
    _cw_assert(24 == buf_get_size(&buf_b));
    _cw_assert(0 == buf_get_size(&buf_c));

    buf_catenate_buf(&buf_a, &buf_b, TRUE);
    _cw_assert(48 == buf_get_size(&buf_a));
    _cw_assert(24 == buf_get_size(&buf_b));

    buf_catenate_buf(&buf_c, &buf_b, FALSE);
    _cw_assert(0 == buf_get_size(&buf_b));
    _cw_assert(24 == buf_get_size(&buf_c));

    buf_catenate_buf(&buf_b, &buf_a, TRUE);
    _cw_assert(48 == buf_get_size(&buf_a));
    _cw_assert(48 == buf_get_size(&buf_b));
    
    buf_catenate_buf(&buf_b, &buf_a, FALSE);
    _cw_assert(0 == buf_get_size(&buf_a));
    _cw_assert(96 == buf_get_size(&buf_b));
    
    buf_catenate_buf(&buf_a, &buf_b, FALSE);
    _cw_assert(96 == buf_get_size(&buf_a));
    _cw_assert(0 == buf_get_size(&buf_b));
    
    buf_delete(&buf_a);
    buf_delete(&buf_b);
    buf_delete(&buf_c);
    bufpool_delete(&bufpool);
  }
  
  /* buf_split(). */
  /* buf_release_head_data(), buf_release_tail_data(). */
  {
    cw_bufpool_t bufpool;
    cw_buf_t buf_a, buf_b, buf_c;
    cw_bufel_t bufel;
    cw_bufc_t * bufc_p;
    cw_uint32_t i;

    bufpool_new(&bufpool, 8, 10);

    buf_new(&buf_a, TRUE);
    buf_new(&buf_b, FALSE);
    buf_new(&buf_c, TRUE);

    for (i = 0; i < 3; i++)
    {
      bufel_new(&bufel, NULL, NULL);
      bufc_p = bufc_new(NULL, NULL, NULL);
      bufc_set_buffer(bufc_p, 
		      bufpool_get_buffer(&bufpool),
		      bufpool_get_buffer_size(&bufpool),
		      bufpool_put_buffer,
		      (void *) &bufpool);
      bufel_set_bufc(&bufel, bufc_p);
      buf_append_bufel(&buf_a, &bufel);
      buf_append_bufel(&buf_b, &bufel);
      buf_prepend_bufel(&buf_c, &bufel);
      bufel_delete(&bufel);
    }

    _cw_assert(24 == buf_get_size(&buf_a));
    _cw_assert(24 == buf_get_size(&buf_b));
    _cw_assert(24 == buf_get_size(&buf_c));

    buf_split(&buf_a, &buf_b, 13);
    _cw_assert(37 == buf_get_size(&buf_a));
    _cw_assert(11 == buf_get_size(&buf_b));

    buf_split(&buf_a, &buf_b, 0);
    _cw_assert(37 == buf_get_size(&buf_a));
    _cw_assert(11 == buf_get_size(&buf_b));
    
    buf_split(&buf_b, &buf_a, 0);
    _cw_assert(37 == buf_get_size(&buf_a));
    _cw_assert(11 == buf_get_size(&buf_b));

    buf_split(&buf_b, &buf_a, 37);
    _cw_assert(0 == buf_get_size(&buf_a));
    _cw_assert(48 == buf_get_size(&buf_b));

    buf_split(&buf_a, &buf_b, 0);
    _cw_assert(0 == buf_get_size(&buf_a));
    _cw_assert(48 == buf_get_size(&buf_b));
    
    buf_split(&buf_b, &buf_a, 0);
    _cw_assert(0 == buf_get_size(&buf_a));
    _cw_assert(48 == buf_get_size(&buf_b));

    buf_split(&buf_a, &buf_b, 17);
    _cw_assert(17 == buf_get_size(&buf_a));
    _cw_assert(31 == buf_get_size(&buf_b));

    buf_split(&buf_a, &buf_b, 30);
    _cw_assert(47 == buf_get_size(&buf_a));
    _cw_assert(1 == buf_get_size(&buf_b));
    
    buf_split(&buf_a, &buf_b, 1);
    _cw_assert(48 == buf_get_size(&buf_a));
    _cw_assert(0 == buf_get_size(&buf_b));

    buf_split(&buf_b, &buf_a, 1);
    _cw_assert(47 == buf_get_size(&buf_a));
    _cw_assert(1 == buf_get_size(&buf_b));

    buf_split(&buf_b, &buf_a, 4);
    _cw_assert(43 == buf_get_size(&buf_a));
    _cw_assert(5 == buf_get_size(&buf_b));

    buf_split(&buf_b, &buf_a, 3);
    _cw_assert(40 == buf_get_size(&buf_a));
    _cw_assert(8 == buf_get_size(&buf_b));

    /* Here. */
    buf_catenate_buf(&buf_b, &buf_c, TRUE);
    _cw_assert(32 == buf_get_size(&buf_b));
    _cw_assert(24 == buf_get_size(&buf_c));

    buf_catenate_buf(&buf_c, &buf_b, FALSE);
    _cw_assert(40 == buf_get_size(&buf_a));
    _cw_assert(0 == buf_get_size(&buf_b));
    _cw_assert(56 == buf_get_size(&buf_c));
    
    _cw_assert(FALSE == buf_release_head_data(&buf_a, 10));
    _cw_assert(30 == buf_get_size(&buf_a));
    
    _cw_assert(FALSE == buf_release_tail_data(&buf_a, 10));
    _cw_assert(20 == buf_get_size(&buf_a));

    buf_catenate_buf(&buf_b, &buf_a, TRUE);
    _cw_assert(20 == buf_get_size(&buf_b));
    
    buf_catenate_buf(&buf_b, &buf_a, FALSE);
    _cw_assert(0 == buf_get_size(&buf_a));
    _cw_assert(40 == buf_get_size(&buf_b));

    _cw_assert(56 == buf_get_size(&buf_c));


    _cw_assert(FALSE == buf_release_tail_data(&buf_c, 10));
    _cw_assert(46 == buf_get_size(&buf_c));
    
    _cw_assert(FALSE == buf_release_head_data(&buf_c, 10));
    _cw_assert(36 == buf_get_size(&buf_c));

    buf_catenate_buf(&buf_a, &buf_c, TRUE);
    _cw_assert(36 == buf_get_size(&buf_a));
    
    buf_catenate_buf(&buf_a, &buf_c, FALSE);
    _cw_assert(0 == buf_get_size(&buf_c));
    _cw_assert(72 == buf_get_size(&buf_a));

    buf_release_tail_data(&buf_a, 16);
    buf_catenate_buf(&buf_c, &buf_a, TRUE);
    buf_release_head_data(&buf_b, 40);
    buf_catenate_buf(&buf_b, &buf_a, TRUE);
    _cw_assert(56 == buf_get_size(&buf_a));
    _cw_assert(56 == buf_get_size(&buf_b));
    _cw_assert(56 == buf_get_size(&buf_c));
    
    _cw_assert(FALSE == buf_release_head_data(&buf_a, 56));
    _cw_assert(0 == buf_get_size(&buf_a));
    
    buf_catenate_buf(&buf_a, &buf_c, FALSE);
    _cw_assert(56 == buf_get_size(&buf_a));
    _cw_assert(0 == buf_get_size(&buf_c));
    buf_catenate_buf(&buf_c, &buf_a, FALSE);
    _cw_assert(0 == buf_get_size(&buf_a));
    _cw_assert(56 == buf_get_size(&buf_c));
    buf_catenate_buf(&buf_a, &buf_c, TRUE);
    _cw_assert(56 == buf_get_size(&buf_a));
    _cw_assert(56 == buf_get_size(&buf_c));
    
    _cw_assert(FALSE == buf_release_tail_data(&buf_b, 56));
    _cw_assert(0 == buf_get_size(&buf_b));

    buf_catenate_buf(&buf_b, &buf_c, FALSE);
    _cw_assert(56 == buf_get_size(&buf_b));
    _cw_assert(0 == buf_get_size(&buf_c));
    buf_catenate_buf(&buf_c, &buf_b, FALSE);
    _cw_assert(0 == buf_get_size(&buf_b));
    _cw_assert(56 == buf_get_size(&buf_c));
    buf_catenate_buf(&buf_b, &buf_c, TRUE);
    _cw_assert(56 == buf_get_size(&buf_b));
    _cw_assert(56 == buf_get_size(&buf_c));
    buf_delete(&buf_a);
    buf_delete(&buf_b);
    buf_delete(&buf_c);
    bufpool_delete(&bufpool);
  }
  
  libstash_shutdown();
  return 0;
}
