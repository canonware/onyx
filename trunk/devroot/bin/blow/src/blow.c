/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Send a specified amount of data through one or more sockets as fast as
 * possible.
 *
 ******************************************************************************/

#include <libsock/libsock.h>

#include "blow_defs.h"

#define _LIBSOCK_BLOW_MAX_CONNS		1000

/* Command line defaults. */
#define _LIBSOCK_BLOW_DEFAULT_NSOCKS	1
#define _LIBSOCK_BLOW_DEFAULT_BSIZE	4096
#define _LIBSOCK_BLOW_DEFAULT_NBLOCKS	65536

/* Function Prototypes. */
void		usage(const char *a_progname);
void		version(const char *a_progname);
const char	*basename(const char *a_str);

int
main(int argc, char **argv)
{
	int		retval = 0;
	cw_uint32_t	i, j;
	cw_sock_t	*sock_array = NULL;
	struct timespec	timeout;
	cw_buf_t	buf, t_buf;
	cw_bufc_t	bufc;
	void		*buffer;

	/* Command line parsing variables. */
	int		c;
	cw_bool_t	cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
	cw_uint32_t	opt_nsocks = _LIBSOCK_BLOW_DEFAULT_NSOCKS;
	cw_uint32_t	opt_bsize = _LIBSOCK_BLOW_DEFAULT_BSIZE;
	cw_uint32_t	opt_nblocks = _LIBSOCK_BLOW_DEFAULT_NBLOCKS;
	int		opt_rport = 0;
	char		*opt_rhost = NULL;

	libstash_init();

	/* Parse command line. */
	while ((c = getopt(argc, argv, "hVn:b:c:r:")) != -1) {
		switch (c) {
		case 'h':
			opt_help = TRUE;
			break;
		case 'V':
			opt_version = TRUE;
			break;
		case 'n':
			opt_nsocks = strtoul(optarg, NULL, 10);
			break;
		case 'b':
			opt_bsize = strtoul(optarg, NULL, 10);
			break;
		case 'c':
			opt_nblocks = strtoul(optarg, NULL, 10);
			break;
		case 'r': {
			char	*colon;

			colon = strstr(optarg, ":");
			if (colon == NULL) {
				opt_rhost = "localhost";
				opt_rport = strtoul(optarg, NULL, 10);
			} else {
				colon[0] = '\0';
				opt_rhost = optarg;
				opt_rport = strtoul(colon + 1, NULL, 10);
			}

			break;
		}
		default:
			cl_error = TRUE;
			break;
		}
	}

	if ((cl_error) || (optind < argc)) {
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
	if (opt_rhost == NULL) {
		out_put(out_err, "Remote host:port not specified\n");
		retval = 1;
		goto CLERROR;
	}
	if (opt_bsize == 0) {
		out_put(out_err, "Invalid block size\n");
		retval = 1;
		goto CLERROR;
	}
	if (opt_nsocks == 0) {
		out_put(out_err, "Invalid number of connections\n");
		retval = 1;
		goto CLERROR;
	}
	if (opt_nblocks == 0) {
		out_put(out_err, "Invalid number of blocks\n");
		retval = 1;
		goto CLERROR;
	}
	libsock_init(_LIBSOCK_BLOW_MAX_CONNS, opt_bsize, 8);
	/* Open the connections. */
	sock_array = (cw_sock_t *)_cw_calloc(opt_nsocks, sizeof(cw_sock_t));
	for (i = 0; i < opt_nsocks; i++) {
		sock_new(&sock_array[i], opt_bsize);
		timeout.tv_sec = 10;
		timeout.tv_nsec = 0;
		if (sock_connect(&sock_array[i], opt_rhost, opt_rport, &timeout)
		    != 0) {
			_cw_out_put_e("Error in sock_connect()\n");
			retval = 1;
			for (j = 0; j < i; j++)
				sock_delete(&sock_array[j]);
			goto RETURN;
		} else
			_cw_out_put_l("New connection ([i])\n", i);
	}

	buf_new(&buf, cw_g_mem);
	buf_new(&t_buf, cw_g_mem);
	bufc_new(&bufc, cw_g_mem, NULL, NULL);
	buffer = _cw_malloc(opt_bsize);
	bufc_buffer_set(&bufc, buffer, opt_bsize, TRUE, (cw_opaque_dealloc_t
	    *)mem_free_e, cw_g_mem);
	buf_bufc_append(&buf, &bufc, 0, opt_bsize);
	bufc_delete(&bufc);

	/*
	 * Cycle through the connections, writing a block to each socket.
	 * Flush every four times through the loop to make sure we don't
	 * buffer too much data (and thereby incur extra memory allocation
	 * overhead).
	 */
	for (i = 0; i < opt_nblocks; i++) {
		if ((i & 0xf) == 0xf) {
			for (j = 0; j < opt_nsocks; j++) {
				if (sock_out_flush(&sock_array[j])) {
					out_put_e(out_err, __FILE__, __LINE__,
					    NULL, "Error in sock_flush_out() "
					    "for connection [i]\n", j);
					goto SHUTDOWN;
				}
				buf_buf_catenate(&t_buf, &buf, TRUE);
				if (sock_write(&sock_array[j], &t_buf)) {
					out_put_e(out_err, __FILE__, __LINE__,
					    NULL, "Error in sock_write() "
					    "for connection [i]\n", j);
					goto SHUTDOWN;
				}
			}
		} else {
			for (j = 0; j < opt_nsocks; j++) {
				buf_buf_catenate(&t_buf, &buf, TRUE);
				if (sock_write(&sock_array[j], &t_buf)) {
					out_put_e(out_err, __FILE__, __LINE__,
					    NULL, "Error in sock_write() "
					    "for connection [i]\n", j);
					goto SHUTDOWN;
				}
			}
		}
	}
	buf_delete(&buf);

	SHUTDOWN:
	/* Flush and close all the sockets. */
	for (i = 0; i < opt_nsocks; i++) {
		sock_out_flush(&sock_array[i]);
		sock_delete(&sock_array[i]);
		_cw_out_put_l("Connection closed ([i])\n", i);
	}

	RETURN:
	if (sock_array != NULL)
		_cw_free(sock_array);
	libsock_shutdown();
	CLERROR:
	libstash_shutdown();
	return retval;
}

void
usage(const char *a_progname)
{
	_cw_out_put("[s] usage:\n"
	    "    [s] -h\n"
	    "    [s] -V\n"
	    "    [s] [[-n <nsocks>] [[-b <bsize>] [[-c <nblocks>] -r [[<rhost>:]<rport>\n"
	    "\n"
	    "    Option               | Description\n"
	    "    ---------------------+------------------------------------------\n"
	    "    -h                   | Print usage and exit.\n"
	    "    -V                   | Print version information and exit.\n"
	    "    -n <nsocks>          | Number of connections to open.\n"
	    "                         | (Defaults to [i].)\n"
	    "    -b <bsize>           | Send blocks of size <bsize>.\n"
	    "                         | (Defaults to [i].)\n"
	    "    -c <nblocks>         | Number of blocks to send over each socket.\n"
	    "                         | (Defaults to [i].)\n"
	    "    -r [[<rhost>:]<rport> | Forward to host <rhost> or \"localhost\",\n"
	    "                         | port <rport>.\n",
	    a_progname, a_progname, a_progname, a_progname,
	    _LIBSOCK_BLOW_DEFAULT_NSOCKS,
	    _LIBSOCK_BLOW_DEFAULT_BSIZE,
	    _LIBSOCK_BLOW_DEFAULT_NBLOCKS
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
