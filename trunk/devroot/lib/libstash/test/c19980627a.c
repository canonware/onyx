/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 117 $
 * $Date: 1998-06-30 17:46:08 -0700 (Tue, 30 Jun 1998) $
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

  dbg_clear(g_dbg_o);
/*   dbg_turn_on(g_dbg_o, _CW_DBG_C_BRBS_FUNC); */
  dbg_turn_on(g_dbg_o, _CW_DBG_C_BRBS_INIT);
  dbg_turn_on(g_dbg_o, _CW_DBG_C_BRBS_ERROR);
  
  brbs_new(&brbs_o);
  _cw_assert(brbs_is_open(&brbs_o) == FALSE);

  _cw_assert(brbs_get_filename(&brbs_o) == NULL);
  _cw_assert(brbs_set_filename(&brbs_o, "/dev/rsd1s1e") == FALSE);
  _cw_assert(brbs_open(&brbs_o) == FALSE);

  if (brbs_get_is_raw(&brbs_o))
  {
    log_printf(g_log_o, "Raw device, sector size == %d\n",
	       brbs_get_sect_size(&brbs_o));
  }
  {
    char buf[21];
    cw_uint64_t size;
    
    size = brbs_get_size(&brbs_o);
    log_print_uint64(size, 16, buf);
    log_printf(g_log_o, "File size == 0x%s bytes\n", buf);
  }
  
  brbs_delete(&brbs_o);
  glob_delete();

  return 0;
}
