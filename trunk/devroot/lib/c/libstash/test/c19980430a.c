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
 * $Revision: 177 $
 * $Date: 1998-08-29 20:58:01 -0700 (Sat, 29 Aug 1998) $
 *
 * <<< Description >>>
 *
 * Test for the res class.
 *
 ****************************************************************************/

#include <string.h>

#define _INC_GLOB_H_
#define _INC_RES_H_
#include <libstash.h>

int
main()
{
  cw_res_t res_o;
  char s1[50] = "simple:value",
    s2[80] = "less_simple : this has spaces",
    s3[80] = "9adv_more_.nasty ::hello there!@$",
    s4[80] = "another_resource:A value with a \\\\ protected \\#.",
    s5[80] = "  	much_white_Space : the resource value w/ trailing    ",
    s6[80] = "trailing.comment : value # This is a comment.",
    s7[80] = " 	 #comment:blah",
    s8[80] = "  #comment with !@#$% other chars",
    * str;

  char es1[80] = "bogus space in name: value",
    es2[80] = "bad_*_character_in_name: value blah",
    es3[80] = ": empty name",
    es4[80] = " somethging #comment",
    es5[80] = "\\name:value",
    es6[80] = " :value",
    es7[80] = " !:value",
    es8[80] = "name#:value",
    es9[80] = "name\\:value",
    es10[80] = "name",
    es11[80] = "name:value\\x",
    es12[80] = "name:value\\:",
    es13[80] = "name:value\\X",
    es14[80] = "name:value\\_",
    es15[80] = "name:value\\.",
    es16[80] = "name:value\\:";

  glob_new();

  dbg_clear(g_dbg_o);
  dbg_turn_on(g_dbg_o, _STASH_DBG_C_RES_ERROR);
/*   dbg_turn_on(g_dbg_o, _STASH_DBG_C_OH_FUNC); */
/*   dbg_turn_on(g_dbg_o, _STASH_DBG_C_OH_SLOT); */
/*   dbg_turn_on(g_dbg_o, _STASH_DBG_C_RES_STATE); */
  
  res_new(&res_o);

  log_printf(g_log_o, "Merging in a list...\n");
  _cw_assert(FALSE == res_merge_list(&res_o, s1, s2, s3, s4, s5, s6, s7,
				     s8, NULL));

  log_printf(g_log_o, "Searching for a resource...\n");
  str = res_get_res_val(&res_o, "simple");

  _cw_check_ptr(str);
  log_printf(g_log_o, "simple:%s\n", str);
  _cw_assert(!strcmp(str, "value"));

  log_printf(g_log_o, "Testing invalid resources handling...\n");

  _cw_assert(TRUE == res_merge_list(&res_o, es1, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es2, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es3, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es4, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es5, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es6, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es7, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es8, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es9, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es10, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es11, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es12, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es13, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es14, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es15, NULL));
  _cw_assert(TRUE == res_merge_list(&res_o, es16, NULL));
  
  res_delete(&res_o);
  glob_delete();

  return 0;
}
