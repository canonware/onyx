/****************************************************************************
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

#include "../include/libstash/libstash.h"

int
main()
{
	cw_res_t res;
	char    s1[50] = "simple:value";
	char	s2[80] = "less_simple : this has spaces";
	char	s3[80] = "9adv_more_.nasty ::hello there!@$";
	char	s4[80] = "another_resource:A value with a \\\\ protected \\#.";
	char	s5[80] =
	    "  	much_white_Space : the resource value w/ trailing    ";
	char	s6[80] = "trailing.comment : value # This is a comment.";
	char	s7[80] = " 	 #comment:blah";
	char	s8[80] = "  #comment with !@#$% other chars";
	const char *str;

	char    es1[80] = "bogus space in name: value";
	char	es2[80] = "bad_*_character_in_name: value blah";
	char	es3[80] = ": empty name";
	char	es4[80] = " somethging #comment";
	char	es5[80] = "\\name:value";
	char	es6[80] = " :value";
	char	es7[80] = " !:value";
	char	es8[80] = "name#:value";
	char	es9[80] = "name\\:value";
	char	es10[80] = "name";
	char	es11[80] = "name:value\\x";
	char	es12[80] = "name:value\\:";
	char	es13[80] = "name:value\\X";
	char	es14[80] = "name:value\\_";
	char	es15[80] = "name:value\\.";
	char	es16[80] = "name:value\\:";

	libstash_init();
	_cw_out_put("Test begin\n");

/*  	dbg_register(cw_g_dbg, "res_state"); */

	res_new(&res, cw_g_mem);

	_cw_out_put("Merging in a list...\n");
	_cw_assert(res_merge_list(&res, s1, s2, s3, s4, s5, s6, s7, s8, NULL) ==
	    FALSE);

	_cw_out_put("Searching for a resource...\n");
	str = res_get_res_val(&res, "simple");

	_cw_check_ptr(str);
	_cw_out_put("simple:[s]\n", str);
	_cw_assert(!strcmp(str, "value"));

	_cw_out_put("Testing invalid resources handling...\n");

	_cw_assert(res_merge_list(&res, es1, NULL));
	_cw_assert(res_merge_list(&res, es2, NULL));
	_cw_assert(res_merge_list(&res, es3, NULL));
	_cw_assert(res_merge_list(&res, es4, NULL));
	_cw_assert(res_merge_list(&res, es5, NULL));
	_cw_assert(res_merge_list(&res, es6, NULL));
	_cw_assert(res_merge_list(&res, es7, NULL));
	_cw_assert(res_merge_list(&res, es8, NULL));
	_cw_assert(res_merge_list(&res, es9, NULL));
	_cw_assert(res_merge_list(&res, es10, NULL));
	_cw_assert(res_merge_list(&res, es11, NULL));
	_cw_assert(res_merge_list(&res, es12, NULL));
	_cw_assert(res_merge_list(&res, es13, NULL));
	_cw_assert(res_merge_list(&res, es14, NULL));
	_cw_assert(res_merge_list(&res, es15, NULL));
	_cw_assert(res_merge_list(&res, es16, NULL));

	res_delete(&res);
	_cw_out_put("Test end\n");
	libstash_shutdown();

	return 0;
}
