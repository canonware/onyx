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
cw_log_t * cw_g_log = NULL;
cw_out_t * cw_g_out = NULL;

cw_bool_t
libstash_init(void)
{
  cw_bool_t retval;
  
  /* Start up global modules. */
  if (cw_g_out == NULL)
  {
    cw_g_out = out_new(NULL);
  }
  if (cw_g_log == NULL)
  {
    cw_g_log = log_new();
  }
  if (cw_g_dbg == NULL)
  {
    cw_g_dbg = dbg_new();
  }
  if (cw_g_mem == NULL)
  {
    cw_g_mem = mem_new();
  }

  if ((NULL == cw_g_out)
      || (NULL == cw_g_log)
      || (NULL == cw_g_dbg)
      || (NULL == cw_g_mem))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
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

  log_delete(cw_g_log);
  cw_g_log = NULL;

  out_delete(cw_g_out);
  cw_g_out = NULL;
}
