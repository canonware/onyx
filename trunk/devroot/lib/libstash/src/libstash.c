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
 * Initialization and shutdown functions for the whole library.
 *
 ****************************************************************************/

#include "libstash/libstash.h"

/* Globals. */
cw_mem_t * cw_g_mem = NULL;
cw_dbg_t * cw_g_dbg = NULL;
cw_out_t * cw_g_out = NULL;

cw_bool_t
libstash_init(void)
{
  cw_bool_t retval = FALSE;
  
  /* Start up global modules. */
  if (cw_g_out == NULL)
  {
    cw_g_out = out_new(NULL);
  }
  if (cw_g_dbg == NULL)
  {
    cw_g_dbg = dbg_new();
#ifdef _LIBSTASH_DBG
    if (NULL != cw_g_dbg)
    {
      dbg_register(cw_g_dbg, "mem_error");
    }
    if (NULL != cw_g_dbg)
    {
      dbg_register(cw_g_dbg, "pezz_error");
    }
#endif
  }
  if (cw_g_mem == NULL)
  {
    cw_g_mem = mem_new();
  }

  if ((NULL == cw_g_out)
      || (NULL == cw_g_dbg)
      || (NULL == cw_g_mem))
  {
    retval = TRUE;
  }
  
  return retval;
}

void
libstash_shutdown(void)
{
  /* Shut down global modules in reverse order. */
  mem_delete(cw_g_mem);
  cw_g_mem = NULL;

  dbg_delete(cw_g_dbg);
  cw_g_dbg = NULL;

  out_delete(cw_g_out);
  cw_g_out = NULL;
}
