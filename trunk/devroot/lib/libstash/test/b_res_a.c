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

#include "../include/libstash/libstash.h"

int
main(int argc, char ** argv)
{
  cw_res_t res;
  char * res_file, * dump_file;
  
  libstash_init();
  dbg_register(cw_g_dbg, "res_error");
/*    dbg_register(cw_g_dbg, "res_state"); */

  _cw_assert(3 == argc);
  res_file = argv[1];
  dump_file = argv[2];
  
  

  res_new(&res);
  _cw_assert(FALSE == res_merge_file(&res, res_file));
  _cw_assert(FALSE == res_dump(&res, dump_file));
  res_delete(&res);
      
  libstash_shutdown();
  return 0;
}
