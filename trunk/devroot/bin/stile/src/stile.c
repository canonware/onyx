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

#include <libstil/libstil.h>
#include <libstash/libstash.h>

#include "stile_defs.h"

int
main(int argc, char **argv, char **envp)
{
	cw_stil_t	stil;
	cw_stilts_t	stilts;

	libstash_init();
	stil_new(&stil, argc, argv, envp, NULL, NULL, NULL, NULL);
	stilts_new(&stilts);

	stilt_start(stil_stilt_get(&stil));

	stilts_delete(&stilts, stil_stilt_get(&stil));
	stil_delete(&stil);
	libstash_shutdown();
	return 0;
}
