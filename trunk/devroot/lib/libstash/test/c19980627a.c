/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 101 $
 * $Date: 1998-06-27 23:38:11 -0700 (Sat, 27 Jun 1998) $
 *
 * <<< Description >>>
 *
 * Test for the brbs class.
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#define _INC_BRBS_H_
#define _INC_STRING_H_
#include <config.h>

int
main()
{
  cw_brbs_t brbs_o;
  
  glob_new();

  _cw_marker("Got here.");
  dbg_clear(g_dbg_o);
  dbg_turn_on(g_dbg_o, _CW_DBG_C_BRBS_FUNC);
  dbg_turn_on(g_dbg_o, _CW_DBG_C_BRBS_INIT);
  dbg_turn_on(g_dbg_o, _CW_DBG_C_BRBS_ERROR);
  
  _cw_marker("Got here.");
  brbs_new(&brbs_o);
  _cw_assert(brbs_is_open(&brbs_o) == FALSE);

  _cw_marker("Got here.");
  _cw_assert(brbs_get_filename(&brbs_o) == NULL);
  _cw_assert(brbs_set_filename(&brbs_o, "/dev/rsd1s1e") == FALSE);
  _cw_assert(brbs_open(&brbs_o) == FALSE);

  _cw_marker("Got here.");
  if (brbs_get_is_raw(&brbs_o))
  {
    log_printf(g_log_o, "Raw device, sector size == %d\n",
	       brbs_get_sect_size(&brbs_o));
  }
  
  _cw_marker("Got here.");
  brbs_delete(&brbs_o);
  glob_delete();

  return 0;
}
