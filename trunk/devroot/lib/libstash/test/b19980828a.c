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

#define _LIBSTASH_USE_RES
#include <libstash/libstash_r.h>

int
main()
{
  cw_res_t res;
  
  libstash_init();
  dbg_register(cw_g_dbg, "res_error");
  
  res_new(&res);
  _cw_assert(FALSE == res_merge_file(&res, "b19980828a.res"));
  _cw_assert(FALSE == res_dump(&res, "b19980828a.dump"));
  res_delete(&res);
      
  libstash_shutdown();
  return 0;
}
