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
 * Send a specified amount of data through one or more sockets as fast as
 * possible.
 *
 ****************************************************************************/

#include <libsock/libsock.h>

/* Command line defaults. */
#define _LIBSOCK_BLOW_DEFAULT_NSOCKS 1
#define _LIBSOCK_BLOW_DEFAULT_BSIZE 4096
#define _LIBSOCK_BLOW_DEFAULT_NBLOCKS 65536

/* Function Prototypes. */
void    usage(const char *a_progname);
void    version(const char *a_progname);
const char *basename(const char *a_str);

int
main(int argc, char **argv)
{
	int     retval = 0;
	cw_uint32_t i, j;
	cw_sock_t *sock_array = NULL;
	struct timespec timeout;
	cw_buf_t buf, t_buf;
	cw_bufc_t bufc;
	void   *buffer;

	/* Command line parsing variables. */
	int     c;
	cw_bool_t cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
	cw_uint32_t opt_nsocks = _LIBSOCK_BLOW_DEFAULT_NSOCKS;
	cw_uint32_t opt_bsize = _LIBSOCK_BLOW_DEFAULT_BSIZE;
	cw_uint32_t opt_nblocks = _LIBSOCK_BLOW_DEFAULT_NBLOCKS;
	int     opt_rport = 0;
	char   *opt_rhost = NULL;

	libstash_init();
	dbg_register(cw_g_dbg, "mem_error");
	dbg_register(cw_g_dbg, "prog_error");
/*    dbg_register(cw_g_dbg, "sockb_verbose"); */
	dbg_register(cw_g_dbg, "sockb_error");
	dbg_register(cw_g_dbg, "sock_error");

	/* Parse command line. */
	while (-1 != (c = getopt(argc, argv, "hVn:b:c:r:"))) {
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
			char   *colon;

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

	if ((TRUE == cl_error) || (optind < argc)) {
		_cw_out_put("Unrecognized option(s)\n");
		usage(basename(argv[0]));
		retval = 1;
		goto CLERROR;
	}
	if (TRUE == opt_help) {
		usage(basename(argv[0]));
		goto CLERROR;
	}
	if (TRUE == opt_version) {
		version(basename(argv[0]));
		goto CLERROR;
	}
	if (NULL == opt_rhost) {
		_cw_out_put("Remote host:port not specified\n");
		retval = 1;
		goto CLERROR;
	}
	if (0 == opt_bsize) {
		_cw_out_put("Invalid block size\n");
		retval = 1;
		goto CLERROR;
	}
	if (0 == opt_nsocks) {
		_cw_out_put("Invalid number of connections\n");
		retval = 1;
		goto CLERROR;
	}
	if (0 == opt_nblocks) {
		_cw_out_put("Invalid number of blocks\n");
		retval = 1;
		goto CLERROR;
	}
	if (sockb_init(100000, opt_bsize, 8))
		_cw_error("Initialization failure in sockb_init()");
	/* Open the connections. */
	sock_array = (cw_sock_t *)_cw_calloc(opt_nsocks, sizeof(cw_sock_t));
	if (NULL == sock_array)
		_cw_error("Memory allocation error");
	for (i = 0; i < opt_nsocks; i++) {
		sock_new(&sock_array[i], opt_bsize);
		timeout.tv_sec = 10;
		timeout.tv_nsec = 0;
		if (0 != sock_connect(&sock_array[i], opt_rhost, opt_rport,
		    &timeout)) {
			_cw_out_put_e("Error in sock_connect()\n");
			retval = 1;
			for (j = 0; j < i; j++)
				sock_delete(&sock_array[j]);
			goto RETURN;
		} else
			_cw_out_put_l("New connection ([i])\n", i);
	}

	buf_new(&buf);
	buf_new(&t_buf);
	bufc_new(&bufc, NULL, NULL);
	buffer = _cw_malloc(opt_bsize);
	if (NULL == buffer)
		_cw_error("Memory allocation error");
	bufc_set_buffer(&bufc, buffer, opt_bsize, TRUE, (cw_opaque_dealloc_t
	    *)mem_free, cw_g_mem);
	if (TRUE == buf_append_bufc(&buf, &bufc, 0, opt_bsize))
		_cw_error("Memory allocation error");
	bufc_delete(&bufc);

	/*
	 * Cycle through the connections, writing a block to each socket.
	 * Flush every four times through the loop to make sure we don't
	 * buffer too much data (and thereby incur extra memory allocation
	 * overhead).
	 */
	for (i = 0; i < opt_nblocks; i++) {
		if (0xf == (i & 0xf)) {
			for (j = 0; j < opt_nsocks; j++) {
				if (TRUE == sock_flush_out(&sock_array[j])) {
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL,
					    "Error in sock_flush_out() for connection [i]\n",
					    j);
					goto SHUTDOWN;
				}
				if (TRUE == buf_catenate_buf(&t_buf, &buf, TRUE))
					_cw_error("Memory allocation error");
				if (TRUE == sock_write(&sock_array[j], &t_buf)) {
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL,
					    "Error in sock_write() for connection [i]\n",
					    j);
					goto SHUTDOWN;
				}
			}
		} else {
			for (j = 0; j < opt_nsocks; j++) {
				if (TRUE == buf_catenate_buf(&t_buf, &buf, TRUE))
					_cw_error("Memory allocation error");
				if (TRUE == sock_write(&sock_array[j], &t_buf)) {
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL,
					    "Error in sock_write() for connection [i]\n",
					    j);
					goto SHUTDOWN;
				}
			}
		}
	}
	buf_delete(&buf);

SHUTDOWN:
	/* Flush and close all the sockets. */
	for (i = 0; i < opt_nsocks; i++) {
		sock_flush_out(&sock_array[i]);
		sock_delete(&sock_array[i]);
		_cw_out_put_l("Connection closed ([i])\n", i);
	}

RETURN:
	if (NULL != sock_array)
		_cw_free(sock_array);
	sockb_shutdown();
CLERROR:
	libstash_shutdown();
	return retval;
}

void
usage(const char *a_progname)
{
	_cw_out_put(
	    "[s] usage:\n"
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
	const char *retval = NULL;
	cw_uint32_t i;

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
