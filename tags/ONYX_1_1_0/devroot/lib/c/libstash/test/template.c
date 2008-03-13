/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	out_put(out_err, "Test begin\n");


	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
