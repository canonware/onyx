/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Test file merging and dumping for the res class.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

int
main(int argc, char **argv)
{
	cw_res_t	res;
	char		*res_file, *dump_file;

	libstash_init();
	dbg_register(cw_g_dbg, "res_error");
/*    dbg_register(cw_g_dbg, "res_state"); */

	_cw_assert(argc == 3);
	res_file = argv[1];
	dump_file = argv[2];

	res_new(&res, cw_g_mem);
	_cw_assert(res_file_merge(&res, res_file) == FALSE);
	_cw_assert(res_dump(&res, dump_file) == FALSE);
	res_delete(&res);

	libstash_shutdown();
	return 0;
}
