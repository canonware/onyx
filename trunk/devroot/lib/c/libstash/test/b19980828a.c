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
 * Test file merging and dumping for the res class.
 *
 ****************************************************************************/

#define _STASH_USE_RES
#include <libstash/libstash_r.h>

int
main()
{
  cw_res_t res_o;
  
  libstash_init();
  dbg_register(g_dbg_o, "res_error");
  
  res_new(&res_o);
  _cw_assert(FALSE == res_merge_file(&res_o, "b19980828a.res"));
  _cw_assert(FALSE == res_dump(&res_o, "b19980828a.dump"));
  res_delete(&res_o);
      
  libstash_shutdown();
  return 0;
}
