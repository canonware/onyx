/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 107 $
 * $Date: 1998-06-29 21:53:48 -0700 (Mon, 29 Jun 1998) $
 *
 * <<< Description >>>
 *
 * Test for the log class.  Specifically, check to see that "%q" conversion
 * code works correctly.
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

  dbg_clear(g_dbg_o);
  dbg_turn_on(g_dbg_o, _CW_DBG_C_LOG_FUNC);

  

  glob_delete();

  return 0;
}
