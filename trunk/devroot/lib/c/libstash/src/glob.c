/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 125 $
 * $Date: 1998-07-02 16:55:52 -0700 (Thu, 02 Jul 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#include <config.h>

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
