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

#define _STASH_USE_BUF
#include <libstash/libstash_r.h>

int
main()
{
  libstash_init();

  /* bufel_new(), bufel_delete(). */
  {
    cw_bufel_t bufel, * bufel_p;

    _cw_assert(&bufel == bufel_new(&bufel));
    bufel_delete(&bufel);

    bufel_p = bufel_new(NULL);
    _cw_check_ptr(bufel_p);
    bufel_delete(bufel_p);
  }

  /* bufel_get_size(), bufel_set_size(). */
  {
    cw_bufel_t * bufel_p;

    bufel_p = bufel_new(NULL);
    _cw_assert(0 == bufel_get_size(bufel_p));

    _cw_assert(FALSE == bufel_set_size(bufel_p, 4096));

    /* In order to test error conditions on bufel_set_size(), we must use
     * bufel_set_end_offset(), which isn't tested until later on. */
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 2048));
    
    _cw_assert(TRUE == bufel_set_size(bufel_p, 2047));
    _cw_assert(4096 == bufel_get_size(bufel_p));

    _cw_assert(FALSE == bufel_set_size(bufel_p, 2048));
    _cw_assert(2048 == bufel_get_size(bufel_p));

    /* Set the size to the existing size. */
    _cw_assert(FALSE == bufel_set_size(bufel_p, 2048));
    _cw_assert(2048 == bufel_get_size(bufel_p));

    bufel_delete(bufel_p);
  }

  /* bufel_get_beg_offset(), bufel_set_beg_offset(),
   * bufel_get_end_offset(), bufel_set_end_offset(),
   * bufel_get_valid_data_size(). */
  {
    cw_bufel_t * bufel_p;

    bufel_p = bufel_new(NULL);

    _cw_assert(FALSE == bufel_set_size(bufel_p, 4096));

    _cw_assert(0 == bufel_get_beg_offset(bufel_p));
    _cw_assert(0 == bufel_get_end_offset(bufel_p));
    _cw_assert(0 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(TRUE == bufel_set_end_offset(bufel_p, 4097));
    _cw_assert(0 == bufel_get_end_offset(bufel_p));

    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 4096));
    _cw_assert(4096 == bufel_get_end_offset(bufel_p));
    _cw_assert(4096 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 2048));
    _cw_assert(2048 == bufel_get_end_offset(bufel_p));
    _cw_assert(2048 == bufel_get_valid_data_size(bufel_p));

    _cw_assert(TRUE == bufel_set_beg_offset(bufel_p, 2049));
    _cw_assert(0 == bufel_get_beg_offset(bufel_p));
    
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
  }
  
  /* bufel_get_data_ptr(), bufel_set_data_ptr(). */
  {
    cw_bufel_t * bufel_p;
    char * string;

    bufel_p = bufel_new(NULL);

    string = (char *) _cw_malloc(50);
    strcpy(string, "Hi ho hum.");
    
    _cw_assert(NULL == bufel_get_data_ptr(bufel_p));
    
    bufel_set_data_ptr(bufel_p, (void *) string, 50);
    _cw_assert((void *) string == bufel_get_data_ptr(bufel_p));
    _cw_assert(0 == bufel_get_beg_offset(bufel_p));
    _cw_assert(50 == bufel_get_end_offset(bufel_p));
    _cw_assert(50 == bufel_get_valid_data_size(bufel_p));
    
    bufel_set_data_ptr(bufel_p, NULL, 0);
    _cw_assert(NULL == bufel_get_data_ptr(bufel_p));
    _cw_assert(0 == bufel_get_beg_offset(bufel_p));
    _cw_assert(0 == bufel_get_end_offset(bufel_p));
    _cw_assert(0 == bufel_get_valid_data_size(bufel_p));

    /* Don't free string, since bufel_p will do it. */
    
    bufel_delete(bufel_p);
  }

  /* bufel_get_uint8(), bufel_set_uint8(),
   * bufel_get_uint32(), bufel_set_uint32(). */
  {
    cw_bufel_t * bufel_p;
    cw_uint32_t i;

    bufel_p = bufel_new(NULL);

    _cw_assert(FALSE == bufel_set_size(bufel_p, 512));
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p, 512));
    
    for (i = 0; i < 256; i++)
    {
      bufel_set_uint8(bufel_p, i, i);
    }

    log_printf(g_log, "lower char dump:\n");
    for (i = 0; i < 256; i += 8)
    {
      log_printf(g_log,
		 "%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x\n",
		 i, bufel_get_uint8(bufel_p, i),
		 i + 1, bufel_get_uint8(bufel_p, i + 1),
		 i + 2, bufel_get_uint8(bufel_p, i + 2),
		 i + 3, bufel_get_uint8(bufel_p, i + 3),
		 i + 4, bufel_get_uint8(bufel_p, i + 4),
		 i + 5, bufel_get_uint8(bufel_p, i + 5),
		 i + 6, bufel_get_uint8(bufel_p, i + 6),
		 i + 7, bufel_get_uint8(bufel_p, i + 7));
    }

    /* Copy the bytes from the lower 256 bytes into the upper 256 bytes,
     * but reverse them. */
    for (i = 0; i < 256; i += 4)
    {
      bufel_set_uint32(bufel_p, (512 - 4) - i,
		       (((cw_uint32_t) (bufel_get_uint8(bufel_p, i)) << 24)
			| (((cw_uint32_t)
			    bufel_get_uint8(bufel_p, i + 1)) << 16)
			| (((cw_uint32_t) bufel_get_uint8(bufel_p, i + 2)) << 8)
			| ((cw_uint32_t) bufel_get_uint8(bufel_p, i + 3))));
    }

    log_printf(g_log, "upper long dump:\n");
    for (i = 256; i < 512; i += 8)
    {
      log_printf(g_log, "%03u->0x%08x:%03u->0x%08x\n",
		 i, bufel_get_uint32(bufel_p, i),
		 i + 4, bufel_get_uint32(bufel_p, i + 4));
    }
    
    bufel_delete(bufel_p);
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

  
  /* buf_rm_head_bufel(), buf_prepend_bufel(), buf_append_bufel(),
     buf_get_size(). */
  {
    cw_buf_t * buf_p;
    cw_bufel_t * bufel_p_a, * bufel_p_b, * bufel_p_c;

    buf_p = buf_new(NULL, TRUE);
    _cw_assert(0 == buf_get_size(buf_p));

    bufel_p_a = bufel_new(NULL);
    _cw_assert(FALSE == bufel_set_size(bufel_p_a, 1));
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p_a, 1));
    bufel_set_uint8(bufel_p_a, 0, 'A');
    
    bufel_p_b = bufel_new(NULL);
    _cw_assert(FALSE == bufel_set_size(bufel_p_b, 1));
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p_b, 1));
    bufel_set_uint8(bufel_p_b, 0, 'B');
    
    bufel_p_c = bufel_new(NULL);
    _cw_assert(FALSE == bufel_set_size(bufel_p_c, 1));
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p_c, 1));
    bufel_set_uint8(bufel_p_c, 0, 'C');

    buf_append_bufel(buf_p, bufel_p_a);
    _cw_assert(1 == buf_get_size(buf_p));
    buf_append_bufel(buf_p, bufel_p_b);
    _cw_assert(2 == buf_get_size(buf_p));
    buf_append_bufel(buf_p, bufel_p_c);
    _cw_assert(3 == buf_get_size(buf_p));
    _cw_assert(bufel_p_a == buf_rm_head_bufel(buf_p));
    _cw_assert(2 == buf_get_size(buf_p));
    _cw_assert(bufel_p_b == buf_rm_head_bufel(buf_p));
    _cw_assert(1 == buf_get_size(buf_p));
    _cw_assert(bufel_p_c == buf_rm_head_bufel(buf_p));
    _cw_assert(0 == buf_get_size(buf_p));
    _cw_assert(NULL == buf_rm_head_bufel(buf_p));
    
    buf_prepend_bufel(buf_p, bufel_p_a);
    _cw_assert(1 == buf_get_size(buf_p));
    buf_prepend_bufel(buf_p, bufel_p_b);
    _cw_assert(2 == buf_get_size(buf_p));
    buf_prepend_bufel(buf_p, bufel_p_c);
    _cw_assert(3 == buf_get_size(buf_p));
    _cw_assert(bufel_p_c == buf_rm_head_bufel(buf_p));
    _cw_assert(2 == buf_get_size(buf_p));
    _cw_assert(bufel_p_b == buf_rm_head_bufel(buf_p));
    _cw_assert(1 == buf_get_size(buf_p));
    _cw_assert(bufel_p_a == buf_rm_head_bufel(buf_p));
    _cw_assert(0 == buf_get_size(buf_p));
    _cw_assert(NULL == buf_rm_head_bufel(buf_p));

    bufel_delete(bufel_p_a);
    bufel_delete(bufel_p_b);
    bufel_delete(bufel_p_c);
    buf_delete(buf_p);
  }
  
  /* buf_prepend_buf(), buf_append_buf(),
     buf_get_size(). */
  {
    cw_buf_t * buf_p_a, * buf_p_b;
    cw_bufel_t * bufel_p_a, * bufel_p_b, * bufel_p_c;

    buf_p_a = buf_new(NULL, TRUE);
    buf_p_b = buf_new(NULL, FALSE); /* XXX */

    bufel_p_a = bufel_new(NULL);
    _cw_assert(FALSE == bufel_set_size(bufel_p_a, 1));
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p_a, 1));
    bufel_set_uint8(bufel_p_a, 0, 'A');
    
    bufel_p_b = bufel_new(NULL);
    _cw_assert(FALSE == bufel_set_size(bufel_p_b, 1));
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p_b, 1));
    bufel_set_uint8(bufel_p_b, 0, 'B');
    
    bufel_p_c = bufel_new(NULL);
    _cw_assert(FALSE == bufel_set_size(bufel_p_c, 1));
    _cw_assert(FALSE == bufel_set_end_offset(bufel_p_c, 1));
    bufel_set_uint8(bufel_p_c, 0, 'C');

    buf_append_bufel(buf_p_a, bufel_p_a);
    buf_append_bufel(buf_p_a, bufel_p_b);
    buf_append_bufel(buf_p_b, bufel_p_c);
    _cw_assert(2 == buf_get_size(buf_p_a));
    _cw_assert(1 == buf_get_size(buf_p_b));
    buf_append_buf(buf_p_a, buf_p_b);
    _cw_assert(3 == buf_get_size(buf_p_a));
    _cw_assert(0 == buf_get_size(buf_p_b));
    _cw_assert(bufel_p_a == buf_rm_head_bufel(buf_p_a));
    _cw_assert(2 == buf_get_size(buf_p_a));
    _cw_assert(bufel_p_b == buf_rm_head_bufel(buf_p_a));
    _cw_assert(1 == buf_get_size(buf_p_a));
    _cw_assert(bufel_p_c == buf_rm_head_bufel(buf_p_a));
    _cw_assert(0 == buf_get_size(buf_p_a));

    buf_append_bufel(buf_p_a, bufel_p_a);
    buf_append_bufel(buf_p_a, bufel_p_b);
    buf_append_bufel(buf_p_b, bufel_p_c);
    _cw_assert(2 == buf_get_size(buf_p_a));
    _cw_assert(1 == buf_get_size(buf_p_b));
    buf_prepend_buf(buf_p_a, buf_p_b);
    _cw_assert(3 == buf_get_size(buf_p_a));
    _cw_assert(0 == buf_get_size(buf_p_b));
    _cw_assert(bufel_p_c == buf_rm_head_bufel(buf_p_a));
    _cw_assert(2 == buf_get_size(buf_p_a));
    _cw_assert(bufel_p_a == buf_rm_head_bufel(buf_p_a));
    _cw_assert(1 == buf_get_size(buf_p_a));
    _cw_assert(bufel_p_b == buf_rm_head_bufel(buf_p_a));
    _cw_assert(0 == buf_get_size(buf_p_a));

    bufel_delete(bufel_p_a);
    bufel_delete(bufel_p_b);
    bufel_delete(bufel_p_c);
    buf_delete(buf_p_a);
    buf_delete(buf_p_b);
  }

  libstash_shutdown();
  return 0;
}
