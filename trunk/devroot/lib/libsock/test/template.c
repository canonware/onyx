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

#include <libsock/libsock.h>

int
main()
{
	libstash_init();
	out_put(cw_g_out, "Test begin\n");
	libsock_init(1024,	/* a_max_fds */
	    4096,		/* a_bufc_size */
	    16			/* a_max_spare_bufcs */
	    );



	libsock_shutdown();
	out_put(cw_g_out, "Test end\n");
	libstash_shutdown();
	return 0;
}
