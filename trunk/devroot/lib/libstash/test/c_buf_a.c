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

  /* XXX */
  /* bufpool_new(), bufpool_delete(). */
  /* bufpool_get_buffer_size(). */
  /* bufpool_get_max_spare_buffers(), bufpool_set_max_spare_buffers(). */
  /* bufpool_get_buffer(), bufpool_put_buffer(). */

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

  /* buf_get_uint8(), buf_set_uint8(),
   * buf_get_uint32(), buf_set_uint32(). */
  {
    cw_bufel_t * bufel_p;
    cw_buf_t * buf_p;
    cw_uint32_t i;
    cw_bufpool_t bufpool;

    bufpool_new(&bufpool, 512, 10);

    bufel_p = bufel_new(NULL);
    buf_p = buf_new(NULL, TRUE);

    bufel_set_data_ptr(bufel_p,
		       bufpool_get_buffer(&bufpool),
		       bufpool_get_buffer_size(&bufpool),
		       bufpool_put_buffer,
		       (void *) &bufpool);
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 512));
    buf_append_bufel(buf_p, bufel_p);
    
    for (i = 0; i < 256; i++)
    {
      buf_set_uint8(buf_p, i, i);
    }

    log_printf(g_log, "lower char dump:\n");
    for (i = 0; i < 256; i += 8)
    {
      log_printf(g_log,
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

    /* Copy the bytes from the lower 256 bytes into the upper 256 bytes,
     * but reverse them. */
    for (i = 0; i < 256; i += 4)
    {
      buf_set_uint32(buf_p, (512 - 4) - i,
		     (((cw_uint32_t) (buf_get_uint8(buf_p, i)) << 24)
		      | (((cw_uint32_t) buf_get_uint8(buf_p, i + 1)) << 16)
		      | (((cw_uint32_t) buf_get_uint8(buf_p, i + 2)) << 8)
		      | ((cw_uint32_t) buf_get_uint8(buf_p, i + 3))));
    }

    log_printf(g_log, "upper long dump:\n");
    for (i = 256; i < 512; i += 8)
    {
      log_printf(g_log, "%03u->0x%08x:%03u->0x%08x\n",
		 i, buf_get_uint32(buf_p, i),
		 i + 4, buf_get_uint32(buf_p, i + 4));
    }

    buf_delete(buf_p);
    bufel_delete(bufel_p);
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

    _cw_marker("Got here");
    buf_dump(buf_p);
    buf_append_bufel(buf_p, bufel_p_a);
    _cw_assert(1 == buf_get_size(buf_p));
    _cw_marker("Got here");
    buf_dump(buf_p);
    buf_append_bufel(buf_p, bufel_p_b);
    _cw_assert(2 == buf_get_size(buf_p));
    _cw_marker("Got here");
    buf_dump(buf_p);
    buf_append_bufel(buf_p, bufel_p_c);
    _cw_assert(3 == buf_get_size(buf_p));

/*      _cw_error("Enough"); */
    _cw_marker("Got here");
    buf_dump(buf_p);
    buf_prepend_bufel(buf_p, bufel_p_a);
    _cw_assert(4 == buf_get_size(buf_p));
    _cw_marker("Got here");
    buf_dump(buf_p);
    buf_prepend_bufel(buf_p, bufel_p_b);
    _cw_assert(5 == buf_get_size(buf_p));
    _cw_marker("Got here");
    buf_dump(buf_p);
    buf_prepend_bufel(buf_p, bufel_p_c);
    _cw_assert(6 == buf_get_size(buf_p));

    _cw_marker("Got here");
    buf_dump(buf_p);
    _cw_assert('C' == buf_get_uint8(buf_p, 0));
    _cw_marker("Got here");
    _cw_assert('B' == buf_get_uint8(buf_p, 1));
    _cw_marker("Got here");
    _cw_assert('A' == buf_get_uint8(buf_p, 2));
    _cw_marker("Got here");
    _cw_assert('A' == buf_get_uint8(buf_p, 3));
    _cw_marker("Got here");
    _cw_assert('B' == buf_get_uint8(buf_p, 4));
    _cw_marker("Got here");
    _cw_assert('C' == buf_get_uint8(buf_p, 5));
    _cw_marker("Got here");

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

  libstash_shutdown();
  return 0;
}
