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
 *
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#include "libstash.h"

/* Globals. */
cw_mem_t * g_mem_o = NULL;
cw_dbg_t * g_dbg_o = NULL;
cw_log_t * g_log_o = NULL;

void
glob_new()
{
  /* Start up global modules. */
  g_mem_o = mem_new();
  g_dbg_o = dbg_new();
  g_log_o = log_new();
}

void
glob_delete()
{
  /* Shut down global modules in reverse order. */
  log_delete(g_log_o);
  dbg_delete(g_dbg_o);
  mem_delete(g_mem_o);
}
