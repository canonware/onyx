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
	_cw_out_put("Test begin\n");

	/* oh_new(), oh_delete(). */
	{
	}


	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
