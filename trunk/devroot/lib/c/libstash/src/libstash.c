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
 * Initialization and shutdown functions for the whole library.
 *
 ****************************************************************************/

#include "libstash/libstash.h"

/* Globals. */
cw_mem_t * g_mem = NULL;
cw_dbg_t * g_dbg = NULL;
cw_log_t * g_log = NULL;

void
libstash_init()
{
  /* Start up global modules. */
  g_mem = mem_new();
  g_dbg = dbg_new();
  g_log = log_new();
}

void
libstash_shutdown()
{
  /* Shut down global modules in reverse order. */
  log_delete(g_log);
  dbg_delete(g_dbg);
  mem_delete(g_mem);
}
