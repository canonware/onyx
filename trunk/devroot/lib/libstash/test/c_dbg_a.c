/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Rudimentary test of the dbg class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

	_cw_assert(dbg_is_registered(cw_g_dbg, "mem_error"));
	_cw_assert(dbg_is_registered(cw_g_dbg, "pezz_error"));

	_cw_assert(dbg_is_registered(cw_g_dbg, "foo") == FALSE);
	_cw_assert(dbg_register(cw_g_dbg, "foo") == FALSE);
	_cw_assert(dbg_is_registered(cw_g_dbg, "foo"));
	_cw_assert(dbg_register(cw_g_dbg, "foo") == FALSE);
	_cw_assert(dbg_is_registered(cw_g_dbg, "foo"));
	dbg_unregister(cw_g_dbg, "foo");
	_cw_assert(dbg_is_registered(cw_g_dbg, "foo") == FALSE);
	dbg_unregister(cw_g_dbg, "foo");
	_cw_assert(dbg_is_registered(cw_g_dbg, "foo") == FALSE);

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
