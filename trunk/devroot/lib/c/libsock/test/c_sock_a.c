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
 * Verify that a sock object can be re-used.
 *
 ****************************************************************************/

#include <libsock/libsock.h>

/* Number of times to connect to the server thread. */
#define _CW_NCONNS 10

void *
handle_client(void *a_arg)
{
	cw_socks_t	*socks = (cw_socks_t *)a_arg;
	cw_sock_t	sock;
	cw_uint32_t	i, message;
	cw_buf_t	buf;

	buf_new(&buf, cw_g_mem);
	sock_new(&sock, 16384);

	for (i = 0; i < _CW_NCONNS; i++) {
		_cw_assert(socks_accept(socks, NULL, &sock) != NULL);
		buf_release_head_data(&buf, buf_get_size(&buf));
		while (sock_read(&sock, &buf, 0, NULL) > 0);

		_cw_assert(buf_get_size(&buf) >= 4);
		message = buf_get_uint32(&buf, 0);
		out_put(cw_g_out, "Connection [i], message [i]\n", i, message);

		_cw_assert(sock_disconnect(&sock) == FALSE);
	}

	sock_delete(&sock);
	buf_delete(&buf);
	return NULL;
}

int
main()
{
	cw_socks_t	*socks = NULL;
	int		port = 0;
	cw_sock_t	sock;
	cw_thd_t	thd;
	cw_buf_t	buf;
	cw_uint32_t	i;

	libstash_init();
/*  	dbg_register(cw_g_dbg, "sock_error"); */
/*  	dbg_register(cw_g_dbg, "sock_sockopt"); */
	out_put(cw_g_out, "Test begin\n");
	libsock_init(1024,	/* a_max_fds */
	    4096,		/* a_bufc_size */
	    16			/* a_max_spare_bufcs */
	    );

	buf_new(&buf, cw_g_mem);

	socks = socks_new();
	_cw_check_ptr(socks);
/*  	if (socks_listen(socks, INADDR_LOOPBACK, &port)) */
	if (socks_listen(socks, INADDR_ANY, &port)) {
		out_put(cw_g_out, "Error listening on port [i]\n", port);
		goto RETURN;
	}
	thd_new(&thd, handle_client, (void *)socks);

	_cw_assert(sock_new(&sock, 16384) != NULL);
	for (i = 0; i < _CW_NCONNS; i++) {
		_cw_assert(sock_connect(&sock, "localhost", port, NULL) == 0);

		/* Write some data. */
		buf_set_uint32(&buf, 0, i);
		_cw_assert(sock_write(&sock, &buf) == FALSE);

		_cw_assert(sock_flush_out(&sock) == FALSE);
		_cw_assert(sock_disconnect(&sock) == FALSE);
	}
	sock_delete(&sock);
	thd_join(&thd);

	RETURN:
	if (socks != NULL)
		socks_delete(socks);
	libsock_shutdown();
	buf_delete(&buf);
	out_put(cw_g_out, "Test end\n");
	libstash_shutdown();
	return 0;
}
