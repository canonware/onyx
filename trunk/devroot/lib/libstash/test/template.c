/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	out_put(cw_g_out, "Test begin\n");


	out_put(cw_g_out, "Test end\n");
	libstash_shutdown();
	return 0;
}
