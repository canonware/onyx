/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Accept connections and throw away incoming data as fast as possible.
 *
 ******************************************************************************/

#include <libsock/libsock.h>

#include "suck_defs.h"

#define _LIBSOCK_SUCK_MAX_CONNS 1000

/* Command line defaults. */
#define _LIBSOCK_BLOW_DEFAULT_BSIZE 4096

/* Function Prototypes. */
void	*accept_entry_func(void *a_arg);
void	usage(const char *a_progname);
void	version(const char *a_progname);
const char * basename(const char *a_str);

/* Global. */
cw_sock_t	*sock_vec[_LIBSOCK_SUCK_MAX_CONNS];
cw_mq_t		*mq;
cw_socks_t	*socks;
cw_uint32_t	opt_bsize = _LIBSOCK_BLOW_DEFAULT_BSIZE;

int
main(int argc, char **argv)
{
	int		retval = 0, bytes_read, sockfd;
	struct timespec	tout;
	cw_buf_t	buf;
	cw_bool_t	did_work;
	cw_thd_t	*accept_thd;

	/* Command line parsing variables. */
	int		c;
	cw_bool_t	cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
	int		opt_port = 0;

	memset(sock_vec, 0, _LIBSOCK_SUCK_MAX_CONNS * sizeof(cw_sock_t *));

	libstash_init();

	/* Parse command line. */
	while ((c = getopt(argc, argv, "hVb:p:")) != -1) {
		switch (c) {
		case 'h':
			opt_help = TRUE;
			break;
		case 'V':
			opt_version = TRUE;
			break;
		case 'b':
			opt_bsize = strtoul(optarg, NULL, 10);
			break;
		case 'p':
			opt_port = strtol(optarg, NULL, 10);
			break;
		default:
			out_put(out_err, "Unrecognized option '[c]'\n", c);
			cl_error = TRUE;
			break;
		}
	}

	if (cl_error || (optind < argc)) {
		out_put(out_err, "Unrecognized option(s)\n");
		usage(basename(argv[0]));
		retval = 1;
		goto CLERROR;
	}
	if (opt_help) {
		usage(basename(argv[0]));
		goto CLERROR;
	}
	if (opt_version) {
		version(basename(argv[0]));
		goto CLERROR;
	}
	if (opt_bsize == 0) {
		out_put(out_err, "Invalid block size\n");
		retval = 1;
		goto CLERROR;
	}
	libsock_init(_LIBSOCK_SUCK_MAX_CONNS, opt_bsize, 8);
	socks = socks_new();
	if (socks == NULL)
		_cw_error("Memory allocation error");
	if (socks_listen(socks, INADDR_ANY, &opt_port))
		exit(1);
	_cw_out_put("[s]: Listening on port [i]\n", basename(argv[0]),
	    opt_port);

	tout.tv_sec = 0;
	tout.tv_nsec = 0;

	/*
	 * Loop, accepting connections, reading from the open sock's, and
	 * closing sock's on error.
	 */
	buf_new(&buf, cw_g_mem);

	mq = mq_new(NULL, cw_g_mem, sizeof(int));
	if (mq == NULL)
		_cw_error("Memory allocation error");
	/* Start thread to accept connections. */
	accept_thd = thd_new(accept_entry_func, NULL);

	for (;;) {
		did_work = FALSE;

		while (mq_get(mq, &sockfd) == FALSE) {
			did_work = TRUE;

			if (sock_vec[sockfd] != NULL) {
				bytes_read = sock_read(sock_vec[sockfd], &buf,
				    0, &tout);
				if (bytes_read > 0) {
					/* Throw the data away. */
					buf_head_data_release(&buf, bytes_read);
				} else if (bytes_read < 0) {
					libsock_in_notify(NULL, sockfd);

					sock_delete(sock_vec[sockfd]);
					sock_vec[sockfd] = NULL;

					_cw_out_put_l("Connection closed\n");
				}
			}
		}
	}
	mq_delete(mq);
	buf_delete(&buf);

	libsock_shutdown();
	CLERROR:
	libstash_shutdown();
	return retval;
}

void *
accept_entry_func(void *a_arg)
{
	cw_sock_t	*sock;

	sock = sock_new(NULL, opt_bsize * 8);
	if (sock == NULL)
		_cw_error("Memory allocation error");
	for (;;) {
		if (socks_accept(socks, NULL, sock) == sock) {
			_cw_out_put_l("New connection\n");

			sock_vec[sock_fd_get(sock)] = sock;

			libsock_in_notify(mq, sock_fd_get(sock));

			/*
			 * Create another sock object for the next time we call
			 * socks_accept().
			 */
			sock = sock_new(NULL, opt_bsize * 8);
			if (sock == NULL)
				_cw_error("Memory allocation error");
		}
	}

	return NULL;
}

void
usage(const char *a_progname)
{
	_cw_out_put("[s] usage:\n"
	    "    [s] -h\n"
	    "    [s] -V\n"
	    "    [s] [[-b <bsize>] [[-p <port>]\n"
	    "\n"
	    "    Option               | Description\n"
	    "    ---------------------+------------------------------------------\n"
	    "    -h                   | Print usage and exit.\n"
	    "    -V                   | Print version information and exit.\n"
	    "    -b <bsize>           | Send blocks of size <bsize>.\n"
	    "                         | (Defaults to [i].)\n"
	    "    -p <port>            | Port to listen on.\n",
	    a_progname, a_progname, a_progname, a_progname,
	    _LIBSOCK_BLOW_DEFAULT_BSIZE
	);
}

void
version(const char *a_progname)
{
	_cw_out_put("[s], version [s]\n", a_progname, _LIBSOCK_VERSION);
}

/* Doesn't strip trailing '/' characters. */
const char *
basename(const char *a_str)
{
	const char	*retval = NULL;
	cw_uint32_t	i;

	_cw_check_ptr(a_str);

	i = strlen(a_str);
	if (i > 0) {
		for (i--; i > 0; i--) {
			if (a_str[i] == '/') {
				retval = &a_str[i + 1];
				break;
			}
		}
	}
	if (retval == NULL)
		retval = a_str;
	return retval;
}
