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
 *
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	out_put(cw_g_out, "Test begin\n");

	/* oh_new(), oh_delete(). */
	{
	}


	out_put(cw_g_out, "Test end\n");
	libstash_shutdown();
	return 0;
}
