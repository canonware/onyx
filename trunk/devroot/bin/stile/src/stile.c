/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include <libstil/libstil.h>
#include <libstash/libstash.h>

int
main(int argc, char **argv)
{
	int	retval = 0;

	libstash_init();

	/* XXX Set up oom handler. */

	_cw_out_put("stile breaths its first breath!\n");

	libstash_shutdown();
	return retval;
}
