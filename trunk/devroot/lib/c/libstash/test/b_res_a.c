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
 * Test file merging and dumping for the res class.
 *
 ****************************************************************************/

#include <libstash/libstash_r.h>

int
main()
{
  cw_res_t res;
  
  libstash_init();
  dbg_register(cw_g_dbg, "res_error");
/*    dbg_register(cw_g_dbg, "res_state"); */

  res_new(&res);
  _cw_assert(FALSE == res_merge_file(&res, "b_res_a.res"));
  _cw_assert(FALSE == res_dump(&res, "b_res_a.dump"));
  res_delete(&res);
      
  libstash_shutdown();
  return 0;
}
