/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include <libstil/libstil.h>
#include <libstash/libstash.h>

int
main(int argc, char **argv)
{
	cw_stil_t	stil;
	cw_stilt_t	stilt;
	cw_stilts_t	stilts;

	libstash_init();

	stil_new(&stil, NULL, NULL);
	stilt_new(&stilt, &stil);
	stilts_new(&stilts, &stilt);

	stilt_start(&stilt, FALSE);

	stilts_delete(&stilts, &stilt);
	stilt_delete(&stilt);
	stil_delete(&stil);
	libstash_shutdown();
	return 0;
}
