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
 * Test the pezz class.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_PEZZ
#include <libstash/libstash_r.h>

int
main()
{
  libstash_init();
  log_printf(cw_g_log, "Test begin\n");

  dbg_register(cw_g_dbg, "pezz_error");
  dbg_register(cw_g_dbg, "pezz_verbose");

  /* pezz_new(), pezz_delete(), pezz_get_buffer_size(). */
  {
    cw_pezz_t pezz, * pezz_p;

    _cw_assert(&pezz == pezz_new(&pezz, 123, 7));
    _cw_assert(123 == pezz_get_buffer_size(&pezz));
    pezz_delete(&pezz);

    pezz_p = pezz_new(NULL, 234, 11);
    _cw_check_ptr(pezz_p);
    _cw_assert(234 == pezz_get_buffer_size(pezz_p));
    pezz_delete(pezz_p);
  }

  /* pezz_get(), pezz_put(). */
  {
    cw_pezz_t pezz;
    void * pointers[100];
    cw_uint32_t i;

    pezz_new(&pezz, 4096, 10);
    _cw_assert(4096 == pezz_get_buffer_size(&pezz));
    for (i = 0; i < 100; i++)
    {
      pointers[i] = pezz_get(&pezz);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 100; i++)
    {
      pezz_put(&pezz, pointers[i]);
      pointers[i] = NULL;
    }
    
    for (i = 0; i < 5; i++)
    {
      pointers[i] = pezz_get(&pezz);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 5; i++)
    {
      pezz_put(&pezz, pointers[i]);
      pointers[i] = NULL;
    }
    
    for (i = 0; i < 6; i++)
    {
      pointers[i] = pezz_get(&pezz);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 6; i++)
    {
      pezz_put(&pezz, pointers[i]);
      pointers[i] = NULL;
    }
    
    for (i = 0; i < 1; i++)
    {
      pointers[i] = pezz_get(&pezz);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 1; i++)
    {
      pezz_put(&pezz, pointers[i]);
      pointers[i] = NULL;
    }
    
    for (i = 0; i < 11; i++)
    {
      pointers[i] = pezz_get(&pezz);
      _cw_check_ptr(pointers[i]);
    }
    for (i = 0; i < 11; i++)
    {
      pezz_put(&pezz, pointers[i]);
      pointers[i] = NULL;
    }

    pezz_delete(&pezz);
  }
  
  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
