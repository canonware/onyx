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
 * Implementation of the socks (socket server) class.  socks accepts connections
 * on a port and returns a sock instance for each connection.
 *
 ****************************************************************************/

#include "../include/libsock/libsock.h"

#include <fcntl.h>
#include <sys/time.h>
#include <poll.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>

#include "../include/libsock/libsock_l.h"

#define _LIBSOCK_SOCKS_MAGIC 0x19730803

struct cw_socks_s {
#ifdef _LIBSOCK_DBG
	cw_uint32_t	magic;
#endif
	cw_bool_t	is_listening;
	int		sockfd;
};

cw_socks_t *
socks_new(void)
{
	cw_socks_t	*retval;

	retval = (cw_socks_t *)_cw_malloc(sizeof(cw_socks_t));
	if (retval == NULL)
		goto RETURN;
	memset(retval, 0, sizeof(cw_socks_t));

#ifdef _LIBSOCK_DBG
	retval->magic = _LIBSOCK_SOCKS_MAGIC;
#endif

	RETURN:
	return retval;
}

void
socks_delete(cw_socks_t *a_socks)
{
	_cw_check_ptr(a_socks);
	_cw_assert(a_socks->magic == _LIBSOCK_SOCKS_MAGIC);

	if (a_socks->is_listening) {
		int	error;

		/* Shut the port down. */
		error = close(a_socks->sockfd);
		if (error) {
			if (dbg_is_registered(cw_g_dbg, "socks_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Error in close(): [s]\n", strerror(error));
			}
		}
	}
#ifdef _LIBSOCK_DBG
	a_socks->magic = 0;
#endif

	_cw_free(a_socks);
}

cw_bool_t
socks_listen(cw_socks_t *a_socks, cw_uint32_t a_mask, int *r_port)
{
	cw_bool_t	retval;
	int		val;
	struct sockaddr_in server_addr;

	_cw_check_ptr(a_socks);
	_cw_assert(a_socks->magic == _LIBSOCK_SOCKS_MAGIC);
	_cw_check_ptr(r_port);
	_cw_assert(a_socks->is_listening == FALSE);

	/* Open a TCP socket stream. */
	a_socks->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (a_socks->sockfd < 0) {
		if (dbg_is_registered(cw_g_dbg, "socks_error")) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Error in socket(): [s]\n", strerror(errno));
		}
		retval = TRUE;
		goto RETURN;
	}
	/* Re-use the socket. */
	val = 1;
	if (setsockopt(a_socks->sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&val,
	    sizeof(val)) < 0) {
		out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "Error for SO_REUSEADDR in setsockopt(): [s]\n",
		    strerror(errno));
		retval = TRUE;
		goto RETURN;
	}
	/* Bind the socket's local address. */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(a_mask);
	server_addr.sin_port = htons(*r_port);

	if (bind(a_socks->sockfd, (struct sockaddr *)&server_addr,
	    sizeof(server_addr))) {
		if (dbg_is_registered(cw_g_dbg, "socks_error")) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Error in bind(): [s]\n", strerror(errno));
		}
		retval = TRUE;
		goto RETURN;
	}
	/* Find out what port is being used, if not already known. */
	if (*r_port == 0) {
		int	server_addr_size = sizeof(server_addr);

		/* What port number did the OS choose? */
		if (getsockname(a_socks->sockfd, (struct sockaddr
		    *)&server_addr, &server_addr_size)) {
			if (dbg_is_registered(cw_g_dbg, "socks_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Error in getsockname(): [s]\n",
				    strerror(errno));
			}
			retval = TRUE;
			goto RETURN;
		}
	       *r_port = ntohs(server_addr.sin_port);
	}
	/*
	 * Now listen.  Use 511 for the backlog just in case only the lower 8
	 * bits are heeded.
	 */
	if (listen(a_socks->sockfd, 511) == -1) {
		if (dbg_is_registered(cw_g_dbg, "socks_error")) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Error in listen(): [s]\n", strerror(errno));
		}
		retval = TRUE;
		goto RETURN;
	}
	a_socks->is_listening = TRUE;

	retval = FALSE;
	RETURN:
	return retval;
}

cw_sock_t *
socks_accept(cw_socks_t *a_socks, struct timespec * a_timeout, cw_sock_t
    *r_sock)
{
	cw_sock_t	*retval;
	struct pollfd	pfd;
	int		timeout, nready;

        _cw_check_ptr(a_socks);
	_cw_assert(a_socks->magic == _LIBSOCK_SOCKS_MAGIC);
        _cw_check_ptr(r_sock);

	/* Are we even listening right now? */
	if (a_socks->is_listening == FALSE) {
		retval = NULL;
		goto RETURN;
	}
	memset(&pfd, 0, sizeof(struct pollfd));

	pfd.fd = a_socks->sockfd;
	pfd.events = POLLIN;

	/* Convert a_timeout to something useful to poll(). */
	if (a_timeout == NULL)
		timeout = -1;
	else {
		timeout = (a_timeout->tv_sec * 1000) + (a_timeout->tv_nsec /
		    1000000);

		if (timeout < 0)
			timeout = INT_MAX;
	}

	nready = poll(&pfd, 1, timeout);
	if (nready == -1) {
		if (dbg_is_registered(cw_g_dbg, "socks_error")) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Error in poll(): [s]\n", strerror(errno));
		}
		retval = NULL;
		goto RETURN;
	}
	if (nready > 0) {
		int		sockfd;
		struct sockaddr_in client_addr;
		int		sockaddr_struct_size = sizeof(struct sockaddr);
		cw_bool_t	wrap_error;

		/* There is someone who wants to connect. */
		sockfd = accept(a_socks->sockfd, (struct sockaddr
		    *)&client_addr, &sockaddr_struct_size);

		if (sockfd < 0) {
			if (dbg_is_registered(cw_g_dbg, "socks_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Error in accept(): [s]\n",
				    strerror(errno));
			}
			retval = NULL;
			goto RETURN;
		}
		/* Wrap the socket descriptor inside a sock. */
		retval = r_sock;

		wrap_error = sock_wrap(retval, sockfd, TRUE);
		if (wrap_error)
			retval = NULL;
	} else {
		/* No one wants to connect. */
		retval = NULL;
	}

	RETURN:
	return retval;
}
