/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Test for the res class.
 *
 ******************************************************************************/

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
	out_put(out_err, "Test begin\n");

	res_new(&res, cw_g_mem);

	out_put(out_err, "Merging in a list...\n");
	_cw_assert(res_list_merge(&res, s1, s2, s3, s4, s5, s6, s7, s8, NULL) ==
	    FALSE);

	out_put(out_err, "Searching for a resource...\n");
	str = res_res_val_get(&res, "simple");

	_cw_check_ptr(str);
	out_put(out_err, "simple:[s]\n", str);
	_cw_assert(!strcmp(str, "value"));

	out_put(out_err, "Testing invalid resources handling...\n");

	_cw_assert(res_list_merge(&res, es1, NULL));
	_cw_assert(res_list_merge(&res, es2, NULL));
	_cw_assert(res_list_merge(&res, es3, NULL));
	_cw_assert(res_list_merge(&res, es4, NULL));
	_cw_assert(res_list_merge(&res, es5, NULL));
	_cw_assert(res_list_merge(&res, es6, NULL));
	_cw_assert(res_list_merge(&res, es7, NULL));
	_cw_assert(res_list_merge(&res, es8, NULL));
	_cw_assert(res_list_merge(&res, es9, NULL));
	_cw_assert(res_list_merge(&res, es10, NULL));
	_cw_assert(res_list_merge(&res, es11, NULL));
	_cw_assert(res_list_merge(&res, es12, NULL));
	_cw_assert(res_list_merge(&res, es13, NULL));
	_cw_assert(res_list_merge(&res, es14, NULL));
	_cw_assert(res_list_merge(&res, es15, NULL));
	_cw_assert(res_list_merge(&res, es16, NULL));

	res_delete(&res);
	out_put(out_err, "Test end\n");
	libstash_shutdown();

	return 0;
}
