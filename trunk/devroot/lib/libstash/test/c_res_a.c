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
 * Test for the res class.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_RES
#include <libstash/libstash_r.h>

int
main()
{
  cw_res_t res;
  char s1[50] = "simple:value",
    s2[80] = "less_simple : this has spaces",
    s3[80] = "9adv_more_.nasty ::hello there!@$",
    s4[80] = "another_resource:A value with a \\\\ protected \\#.",
    s5[80] = "  	much_white_Space : the resource value w/ trailing    ",
    s6[80] = "trailing.comment : value # This is a comment.",
    s7[80] = " 	 #comment:blah",
    s8[80] = "  #comment with !@#$% other chars";
  const char * str;

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

  libstash_init();
  log_printf(cw_g_log, "Test begin\n");

/*   dbg_register(cw_g_dbg, "res_state"); */
  
  res_new(&res);

  log_printf(cw_g_log, "Merging in a list...\n");
  _cw_assert(FALSE == res_merge_list(&res, s1, s2, s3, s4, s5, s6, s7,
				     s8, NULL));

  log_printf(cw_g_log, "Searching for a resource...\n");
  str = res_get_res_val(&res, "simple");

  _cw_check_ptr(str);
  log_printf(cw_g_log, "simple:%s\n", str);
  _cw_assert(!strcmp(str, "value"));

  log_printf(cw_g_log, "Testing invalid resources handling...\n");

  _cw_assert(TRUE == res_merge_list(&res, es1, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es2, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es3, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es4, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es5, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es6, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es7, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es8, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es9, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es10, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es11, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es12, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es13, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es14, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es15, NULL));
  _cw_assert(TRUE == res_merge_list(&res, es16, NULL));
  
  res_delete(&res);
  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();

  return 0;
}
