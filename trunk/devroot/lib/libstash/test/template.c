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
	fprintf(stderr, "Test begin\n");


	fprintf(stderr, "Test end\n");
	libstash_shutdown();
	return 0;
}
