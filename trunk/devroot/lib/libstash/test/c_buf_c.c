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
 * buf test.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_BUF
#include <libstash/libstash.h>

#include <time.h>

int
main()
{
/*    time_t beg_time, end_time, bef_time, aft_time, secs; */
  cw_buf_t * buf_a, * buf_b, * buf_c, * buf_d;

  libstash_init();
  out_put(cw_g_out, "Test begin\n");
  
  buf_a = buf_new(NULL, TRUE);
  buf_b = buf_new(NULL, TRUE);
  buf_c = buf_new(NULL, TRUE);
  buf_d = buf_new(NULL, TRUE);

  /* Create a whole bunch of data. */
  {
    cw_uint32_t i, j;
    cw_bufc_t * bufc;
    void * data;

    for (i = 1; i < 32; i++)
    {
      for (j = 1; j < 32; j++)
      {
	bufc = bufc_new((cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t)),
			mem_dealloc,
			cw_g_mem);
	data = _cw_malloc(i * j);
	bufc_set_buffer(bufc, data, i * j, FALSE, mem_dealloc, cw_g_mem);

	buf_append_bufc(buf_a, bufc, 0, i * j);
	bufc_delete(bufc);
      }
    }

    for (i = 0; i < 64; i++)
    {
      bufc = bufc_new((cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t)),
		      mem_dealloc,
		      cw_g_mem);
      data = _cw_malloc(4096);
      bufc_set_buffer(bufc, data, 4096, FALSE, mem_dealloc, cw_g_mem);
      buf_append_bufc(buf_b, bufc, 0, 4096);
      bufc_delete(bufc);
    }

    for (i = 0; i < 16384; i++)
    {
      bufc = bufc_new((cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t)),
		      mem_dealloc,
		      cw_g_mem);
      data = _cw_malloc(16);
      bufc_set_buffer(bufc, data, 16, FALSE, mem_dealloc, cw_g_mem);
      buf_append_bufc(buf_c, bufc, 0, 16);
      bufc_delete(bufc);
    }
  }

  out_put(cw_g_out, "[i32] bytes of data in buf_a (various sized bufc's)\n",
	  buf_get_size(buf_a));
  out_put(cw_g_out, "[i32] bytes of data in buf_b (4kB bufc's)\n",
	  buf_get_size(buf_b));
  out_put(cw_g_out, "[i32] bytes of data in buf_c (16B bufc's)\n",
	  buf_get_size(buf_c));

  /* buf_get_uint8() inc loop for each buf. */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_a); j < size; j++)
      {
	buf_get_uint8(buf_a, j);
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_b); j < size; j++)
      {
	buf_get_uint8(buf_b, j);
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_c); j < size; j++)
      {
	buf_get_uint8(buf_c, j);
      }
    }
  }

  /* buf_get_uint8() dec loop for each buf. */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_a); j < size; j++)
      {
	buf_get_uint8(buf_a, size - (1 + j));
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_b); j < size; j++)
      {
	buf_get_uint8(buf_b, size - (1 + j));
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_c); j < size; j++)
      {
	buf_get_uint8(buf_c, size - (1 + j));
      }
    }
  }
  
  /* Stride-wise buf_get_uint8(). */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0, size = buf_get_size(buf_a); i < 1; i++)
    {
      for (j = i; j < size; j += 13)
      {
	buf_get_uint8(buf_a, j);
      }
    }

    for (i = 0, size = buf_get_size(buf_b); i < 1; i++)
    {
      for (j = i; j < size; j += 13)
      {
	buf_get_uint8(buf_b, j);
      }
    }

    for (i = 0, size = buf_get_size(buf_c); i < 1; i++)
    {
      for (j = i; j < size; j += 13)
      {
	buf_get_uint8(buf_c, j);
      }
    }
  }

  /* Takes a long time to run. */
#if (0)
  /* Multi-stride buf_get_uint8(). */
  {
    cw_uint32_t h, i, j, k, size;
    
    for (h = 0; h < 1; h++)
    {
      for (i = 1, size = buf_get_size(buf_a); i < size; i <<= 1)
      {
	for (j = 0; j < i; j++)
	{
	  for (k = 0; k < size; k += i)
	  {
	    buf_get_uint8(buf_a, k);
	  }
	}
      }
    }

    for (h = 0; h < 1; h++)
    {
      for (i = 1, size = buf_get_size(buf_b); i < size; i <<= 1)
      {
	for (j = 0; j < i; j++)
	{
	  for (k = 0; k < size; k += i)
	  {
	    buf_get_uint8(buf_b, k);
	  }
	}
      }
    }

    for (h = 0; h < 1; h++)
    {
      for (i = 1, size = buf_get_size(buf_c); i < size; i <<= 1)
      {
	for (j = 0; j < i; j++)
	{
	  for (k = 0; k < size; k += i)
	  {
	    buf_get_uint8(buf_c, k);
	  }
	}
      }
    }
  }
#endif
  
  /* buf_get_uint32() inc loop for each buf. */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_a); j < size - 3; j++)
      {
	buf_get_uint32(buf_a, j);
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_b); j < size - 3; j++)
      {
	buf_get_uint32(buf_b, j);
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_c); j < size - 3; j++)
      {
	buf_get_uint32(buf_c, j);
      }
    }
  }
  
  /* buf_get_uint32() dec loop for each buf. */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_a); j < size - 3; j++)
      {
	buf_get_uint32(buf_a, size - (4 + j));
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_b); j < size - 3; j++)
      {
	buf_get_uint32(buf_b, size - (4 + j));
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_c); j < size - 3; j++)
      {
	buf_get_uint32(buf_c, size - (4 + j));
      }
    }
  }
  
  /* Stride-wise buf_get_uint32(). */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0, size = buf_get_size(buf_a); i < 1; i++)
    {
      for (j = i; j < size - 3; j += 13)
      {
	buf_get_uint32(buf_a, j);
      }
    }

    for (i = 0, size = buf_get_size(buf_b); i < 1; i++)
    {
      for (j = i; j < size - 3; j += 13)
      {
	buf_get_uint32(buf_b, j);
      }
    }

    for (i = 0, size = buf_get_size(buf_c); i < 1; i++)
    {
      for (j = i; j < size - 3; j += 13)
      {
	buf_get_uint32(buf_c, j);
      }
    }
  }

  /* buf_get_uint64() inc loop for each buf. */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_a); j < size - 7; j++)
      {
	buf_get_uint64(buf_a, j);
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_b); j < size - 7; j++)
      {
	buf_get_uint64(buf_b, j);
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_c); j < size - 7; j++)
      {
	buf_get_uint64(buf_c, j);
      }
    }
  }
  
  /* buf_get_uint64() dec loop for each buf. */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_a); j < size - 7; j++)
      {
	buf_get_uint64(buf_a, size - (8 + j));
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_b); j < size - 7; j++)
      {
	buf_get_uint64(buf_b, size - (8 + j));
      }
    }

    for (i = 0; i < 1; i++)
    {
      for (j = 0, size = buf_get_size(buf_c); j < size - 7; j++)
      {
	buf_get_uint64(buf_c, size - (8 + j));
      }
    }
  }

  /* Stride-wise buf_get_uint64(). */
  {
    cw_uint32_t i, j, size;
    
    for (i = 0, size = buf_get_size(buf_a); i < 1; i++)
    {
      for (j = i; j < size - 7; j += 13)
      {
	buf_get_uint64(buf_a, j);
      }
    }

    for (i = 0, size = buf_get_size(buf_b); i < 1; i++)
    {
      for (j = i; j < size - 7; j += 13)
      {
	buf_get_uint64(buf_b, j);
      }
    }

    for (i = 0, size = buf_get_size(buf_c); i < 1; i++)
    {
      for (j = i; j < size - 7; j += 13)
      {
	buf_get_uint64(buf_c, j);
      }
    }
  }

  {
    cw_uint32_t i;

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_a, FALSE);
      buf_catenate_buf(buf_a, buf_d, FALSE);
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_b, FALSE);
      buf_catenate_buf(buf_b, buf_d, FALSE);
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_c, FALSE);
      buf_catenate_buf(buf_c, buf_d, FALSE);
    }
  }

  /* buf_split() loop. */
  {
    cw_uint32_t i, j, size;
    
    for (i = 2099, size = buf_get_size(buf_a); i < size; i += 2099)
    {
      for (j = 0; (j + i) < size; j += i)
      {
	buf_split(buf_d, buf_a, i);
      }
      buf_catenate_buf(buf_d, buf_a, FALSE);
      buf_catenate_buf(buf_a, buf_d, FALSE);
    }
    
    for (i = 2099, size = buf_get_size(buf_b); i < size; i += 2099)
    {
      for (j = 0; (j + i) < size; j += i)
      {
	buf_split(buf_d, buf_b, i);
      }
      buf_catenate_buf(buf_d, buf_b, FALSE);
      buf_catenate_buf(buf_b, buf_d, FALSE);
    }
    
    for (i = 2099, size = buf_get_size(buf_c); i < size; i += 2099)
    {
      for (j = 0; (j + i) < size; j += i)
      {
	buf_split(buf_d, buf_c, i);
      }
      buf_catenate_buf(buf_d, buf_c, FALSE);
      buf_catenate_buf(buf_c, buf_d, FALSE);
    }
  }

  /* buf_release_head_data() loop. */
  {
    cw_uint32_t i, j;

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_a, TRUE);
      for (j = 0; j < buf_get_size(buf_d); j += 13)
      {
	buf_release_head_data(buf_d, j);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_b, TRUE);
      for (j = 0; j < buf_get_size(buf_d); j += 13)
      {
	buf_release_head_data(buf_d, j);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_c, TRUE);
      for (j = 0; j < buf_get_size(buf_d); j += 13)
      {
	buf_release_head_data(buf_d, j);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }
  }

  /* buf_release_tail_data() loop. */
  {
    cw_uint32_t i, j;

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_a, TRUE);
      for (j = 0; j < buf_get_size(buf_d); j += 13)
      {
	buf_release_tail_data(buf_d, j);
      }
      buf_release_tail_data(buf_d, buf_get_size(buf_d));
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_b, TRUE);
      for (j = 0; j < buf_get_size(buf_d); j += 13)
      {
	buf_release_tail_data(buf_d, j);
      }
      buf_release_tail_data(buf_d, buf_get_size(buf_d));
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_c, TRUE);
      for (j = 0; j < buf_get_size(buf_d); j += 13)
      {
	buf_release_tail_data(buf_d, j);
      }
      buf_release_tail_data(buf_d, buf_get_size(buf_d));
    }
  }

  /* Cumulative index rebuild. */
  {
    cw_uint32_t i, j;
    
    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_a, TRUE);
      for (j = 0; buf_get_size(buf_d) > 349; j++)
      {
	buf_release_head_data(buf_d, 349);
	buf_get_uint8(buf_d, buf_get_size(buf_d) - 1);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_b, TRUE);
      for (j = 0; buf_get_size(buf_d) > 349; j++)
      {
	buf_release_head_data(buf_d, 349);
	buf_get_uint8(buf_d, buf_get_size(buf_d) - 1);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_c, TRUE);
      for (j = 0; buf_get_size(buf_d) > 349; j++)
      {
	buf_release_head_data(buf_d, 349);
	buf_get_uint8(buf_d, buf_get_size(buf_d) - 1);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }
  }
  
  /* buf_get_iovec(). */
  {
    cw_uint32_t i, j;
    int iov_cnt;
    
    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_a, TRUE);
      for (j = 0; buf_get_size(buf_d) > 11; j++)
      {
	buf_release_head_data(buf_d, 11);
	buf_get_iovec(buf_d, 0, FALSE, &iov_cnt);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_b, TRUE);
      for (j = 0; buf_get_size(buf_d) > 11; j++)
      {
	buf_release_head_data(buf_d, 11);
	buf_get_iovec(buf_d, 0, FALSE, &iov_cnt);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }

    for (i = 0; i < 1; i++)
    {
      buf_catenate_buf(buf_d, buf_c, TRUE);
      for (j = 0; buf_get_size(buf_d) > 11; j++)
      {
	buf_release_head_data(buf_d, 11);
	buf_get_iovec(buf_d, 0, FALSE, &iov_cnt);
      }
      buf_release_head_data(buf_d, buf_get_size(buf_d));
    }
  }

  buf_delete(buf_a);
  buf_delete(buf_b);
  buf_delete(buf_c);
  buf_delete(buf_d);

  out_put(cw_g_out, "Test end\n");

  libstash_shutdown();
  return 0;
}
