/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Multi-threaded concurrent line echo server that uses the sock and socks
 * classes.
 *
 ******************************************************************************/

#include <libsock/libsock.h>

#include "echos_defs.h"

void *
handle_client(void *a_arg)
{
	cw_buf_t	buf;
	cw_sock_t	*sock = (cw_sock_t *)a_arg;
	const struct iovec *iovec;
	int		iovec_count;
	cw_uint32_t	i;

	_cw_out_put("New connection\n");

	buf_new(&buf, cw_g_mem);

	for (;;) {
		if (sock_read(sock, &buf, 0, NULL) < 0)
			break;
		iovec = buf_iovec_get(&buf, 0xffffffff, buf_size_get(&buf),
		    &iovec_count);

		_cw_out_put("Got :");
		for (i = 0; i < iovec_count; i++) {
			_cw_out_put_n(iovec[i].iov_len, "[s]",
			    iovec[i].iov_base);
		}
		_cw_out_put(":\n");

		if (sock_write(sock, &buf))
			break;
		_cw_assert(buf_size_get(&buf) == 0);
	}

	buf_delete(&buf);
	sock_delete(sock);

	_cw_out_put("Connection closed\n");

	return NULL;
}

int
main(int argc, char **argv)
{
	cw_socks_t	*socks;
	cw_sock_t	*sock, *sock_ptr;
	int		port;
	cw_thd_t	thd;

	_cw_out_put("[s]: pid [i]\n", argv[0], getpid());

	if (argc != 2)
		port = 0;
	else
		port = strtol(argv[1], (char **)NULL, 10);

	libstash_init();
	libsock_init(64, 512, 0);

	dbg_register(cw_g_dbg, "libsock_verbose");
	dbg_register(cw_g_dbg, "libsock_error");
	dbg_register(cw_g_dbg, "socks_verbose");
	dbg_register(cw_g_dbg, "socks_error");
	dbg_register(cw_g_dbg, "sock_verbose");
	dbg_register(cw_g_dbg, "sock_error");
/*  	dbg_register(cw_g_dbg, "mem_verbose"); */

	socks = socks_new();
	if (socks_listen(socks, INADDR_ANY, &port)) {
		_cw_out_put("[s]: Error listening on port [i]\n", argv[0],
		    port);
		goto RETURN;
	}
	_cw_out_put("[s]: Listening on port [i]\n", argv[0], port);

	for (sock_ptr = NULL; sock_ptr == NULL; sock_ptr = NULL) {
		sock = sock_new(NULL, 1024);

		sock_ptr = socks_accept(socks, NULL, sock);
		_cw_check_ptr(sock_ptr);

		thd_new(&thd, handle_client, (void *)sock);

		/* Detach the thread. */
		thd_delete(&thd);
	}

	sock_delete(sock);
	socks_delete(socks);

	RETURN:
	libsock_shutdown();
	libstash_shutdown();
	return 0;
}
