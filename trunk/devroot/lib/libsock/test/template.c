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

#include <libsock/libsock.h>

int
main()
{
	libstash_init();
	out_put(out_err, "Test begin\n");
	libsock_init(1024,	/* a_max_fds */
	    4096,		/* a_bufc_size */
	    16			/* a_max_spare_bufcs */
	    );



	libsock_shutdown();
	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
