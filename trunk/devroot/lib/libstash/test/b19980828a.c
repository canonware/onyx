/* -*-mode:c-*-
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
 * Test file merging and dumping for the res class.
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#define _INC_RES_H_
#include <libstash_r.h>

int
main()
{
  glob_new();
  dbg_clear(g_dbg_o);
  dbg_turn_on(g_dbg_o, _STASH_DBG_R_RES_ERROR);
/*   dbg_turn_on(g_dbg_o, _STASH_DBG_R_RES_FUNC); */
  
  {
    cw_res_t res_o;

    res_new(&res_o);
    _cw_assert(FALSE == res_merge_file(&res_o, "b19980828a.res"));
    _cw_assert(FALSE == res_dump(&res_o, "b19980828a.dump"));
    res_delete(&res_o);
  }
      
  glob_delete();
  return 0;
}
