/******************************************************************************
 *
 * <Copyright = jasone>
 * <Copyright = nectar>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Multi-threaded concurrent transparent proxy.
 *
 ******************************************************************************/

#include <libsock/libsock.h>

#include "proxy_defs.h"

#ifdef _CW_IPFILTER
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <netinet/ip_compat.h>
#include <netinet/ip_fil.h>
#include <netinet/ip_proxy.h>
#include <netinet/ip_nat.h>
#endif

#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

struct handler_s {
	sigset_t	hupset;
	cw_thd_t	*sig_thd;
};

typedef enum {
	PRETTY, HEX, ASCII
}       format_t;

typedef struct {
	cw_thd_t	*send_thd;
	cw_thd_t	*recv_thd;
	cw_mtx_t	lock;
	cw_bool_t	should_quit;
	cw_out_t	*out;
	cw_sock_t	client_sock;
	cw_sock_t	remote_sock;

	char		*rhost;
	int		rport;

	cw_bool_t	is_verbose;
	format_t	format;
}       connection_t;

cw_bool_t	should_quit = FALSE;
const char	*g_progname;

/* Function prototypes. */
void	*sig_handler(void *a_arg);
char	*get_out_str_pretty(cw_buf_t *a_buf, cw_bool_t is_send, char *a_str);
char	*get_out_str_hex(cw_buf_t *a_buf, cw_bool_t is_send, char *a_str);
char	*get_out_str_ascii(cw_buf_t *a_buf, cw_bool_t is_send, char *a_str);
void	*handle_client_send(void *a_arg);
void	*handle_client_recv(void *a_arg);
#ifdef _CW_IPFILTER
cw_bool_t ipnat_set_connect(connection_t *a_conn);
#endif
void	usage(void);
void	version(void);
const char *basename(const char *a_str);

int
main(int argc, char **argv)
{
	int		retval = 0;
	cw_socks_t	*socks;
	connection_t	*conn;
	char		logfile[2048];
	cw_uint32_t	conn_num;
	struct timespec	timeout;

	struct handler_s handler_arg;

	int		c;
	cw_bool_t	cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
	cw_bool_t	opt_verbose = FALSE, opt_quiet = FALSE, opt_log = FALSE;
	format_t	opt_format = PRETTY;
	int		opt_port = 0, opt_rport = 0;
	cw_uint32_t	opt_ip = htonl(INADDR_ANY);
	char		*opt_rhost = NULL, *opt_dirname = NULL;
#ifdef _CW_IPFILTER
	cw_bool_t	opt_transparent = FALSE;
#endif

	libstash_init();
	g_progname = basename(argv[0]);
	dbg_register(cw_g_dbg, "prog_error");
	dbg_register(cw_g_dbg, "libsock_error");
	dbg_register(cw_g_dbg, "socks_error");
	dbg_register(cw_g_dbg, "sock_error");
/*  	dbg_register(cw_g_dbg, "sock_sockopt"); */
/*  	dbg_register(cw_g_dbg, "mem_verbose"); */
/*  	dbg_register(cw_g_dbg, "pezz_verbose"); */

	/* Parse command line. */
	while ((c = getopt(argc, argv, "hVvqi:p:r:lf:d:"
#ifdef _CW_IPFILTER
	    "t"
#endif
	    )) != -1) {
		switch (c) {
		case 'h':
			opt_help = TRUE;
			break;
		case 'V':
			opt_version = TRUE;
			break;
		case 'v':
			opt_verbose = TRUE;
			dbg_register(cw_g_dbg, "prog_verbose");
/*  			dbg_register(cw_g_dbg, "mem_verbose"); */
			dbg_register(cw_g_dbg, "libsock_verbose");
			dbg_register(cw_g_dbg, "socks_verbose");
/*  			dbg_register(cw_g_dbg, "sock_sockopt"); */
			break;
		case 'q':
			opt_quiet = TRUE;
			dbg_unregister(cw_g_dbg, "prog_error");
			dbg_unregister(cw_g_dbg, "mem_error");
			dbg_unregister(cw_g_dbg, "pezz_error");
			dbg_unregister(cw_g_dbg, "libsock_error");
			dbg_unregister(cw_g_dbg, "socks_error");
			dbg_unregister(cw_g_dbg, "sock_error");
			break;
		case 'i':
			if (strcmp(optarg, "ANY") == 0) {
				/* Same as default. */
				opt_ip = htonl(INADDR_ANY);
			} else if (strcmp(optarg, "LOOPBACK") == 0)
				opt_ip = htonl(INADDR_LOOPBACK);
			else {
				struct in_addr	addr;

				if (inet_aton(optarg, &addr) == 0) {
					/* Conversion error. */
					_cw_out_put("[s]: Invalid IP address "
					    "specified with \"-i\" flag\n",
					    g_progname);
					usage();
					retval = 1;
					goto CLERROR;
				}
				opt_ip = addr.s_addr;
			}
			break;
		case 'p':
			opt_port = strtoul(optarg, NULL, 10);
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
		case 'f':
			switch (*optarg) {
			case 'p':
				opt_format = PRETTY;
				break;
			case 'h':
				opt_format = HEX;
				break;
			case 'a':
				opt_format = ASCII;
				break;
			default:
				cl_error = TRUE;
				break;
			}

			break;
		case 'l':
			opt_log = TRUE;
			break;
		case 'd':
			opt_dirname = optarg;
			break;
#ifdef _CW_IPFILTER
		case 't':
			opt_transparent = TRUE;
			break;
#endif
		default:
			cl_error = TRUE;
			break;
		}
	}

	if ((cl_error) || (optind < argc)) {
		_cw_out_put("[s]: Unrecognized option(s)\n", g_progname);
		usage();
		retval = 1;
		goto CLERROR;
	}
	if (opt_help) {
		usage();
		goto CLERROR;
	}
	if (opt_version) {
		version();
		goto CLERROR;
	}
	/* Check validity of command line options. */
	if ((opt_verbose) && (opt_quiet)) {
		_cw_out_put("[s]: \"-v\" and \"-q\" are incompatible\n",
		    g_progname);
		usage();
		retval = 1;
		goto CLERROR;
	}
	if ((opt_log) && (opt_dirname != NULL)) {
		_cw_out_put("[s]: \"-l\" and \"-d\" are incompatible\n",
		    g_progname);
		usage();
		retval = 1;
		goto CLERROR;
	}
	if ((opt_dirname != NULL) && (strlen(opt_dirname) > 512)) {
		_cw_out_put("[s]: Argument to \"-d\" flag is too long "
		    "(512 bytes max)\n", g_progname);
		usage();
		retval = 1;
		goto CLERROR;
	}
#ifdef _CW_IPFILTER
	if (opt_transparent && opt_rhost != NULL) {
		_cw_out_put("[s]: \"-r\" and \"-t\" are incompatible\n",
		    g_progname);
		usage();
		retval = 1;
		goto CLERROR;
	} else if (!opt_transparent && opt_rhost == NULL) {
		_cw_out_put("[s]: \"-r\" or \"-t\" must be specified\n",
		    g_progname);
		usage();
		retval = 1;
		goto CLERROR;
	}
#else
	if (opt_rhost == NULL) {
		_cw_out_put("[s]: \"-r\" must be specified\n", g_progname);
		usage();
		retval = 1;
		goto CLERROR;
	}
#endif
	/*
	 * Set the per-thread signal masks such that only one thread will catch
	 * the signal.
	 */
	sigemptyset(&handler_arg.hupset);
	sigaddset(&handler_arg.hupset, SIGHUP);
	sigaddset(&handler_arg.hupset, SIGINT);
	thd_sigmask(SIG_BLOCK, &handler_arg.hupset, NULL);
	handler_arg.sig_thd = thd_new(sig_handler, (void *)&handler_arg);

	if (opt_dirname != NULL) {
		cw_sint32_t	fd;

		_cw_out_put_s(logfile, "[s]/[s].pid_[i].log", opt_dirname,
		    g_progname, getpid());

		fd = (cw_sint32_t)open(logfile, O_RDWR | O_CREAT | O_TRUNC,
		    0644);
		if (fd == -1) {
			if (dbg_is_registered(cw_g_dbg, "prog_error")) {
				_cw_out_put_e("[s]: Error opening \"[s]\": "
				    "[s]\n", g_progname, logfile,
				    strerror(errno));
			}
		}
		out_default_fd_set(cw_g_out, fd);
	}
	if (dbg_is_registered(cw_g_dbg, "prog_verbose"))
		_cw_out_put("[s]: pid: [i]\n", g_progname, getpid());
	libsock_init(1024, 2048, 4096);
	socks = socks_new();
	if (socks_listen(socks, opt_ip, &opt_port))
		exit(1);
	if (dbg_is_registered(cw_g_dbg, "prog_verbose")) {
		_cw_out_put_l("[s]: Listening on port [i]\n", g_progname,
		    opt_port);
	}
	for (conn_num = 0; should_quit == FALSE;) {
		conn = _cw_malloc(sizeof(connection_t));

		memset(conn, 0, sizeof(connection_t));
		sock_new(&conn->client_sock, 16384);

		timeout.tv_sec = 5;
		timeout.tv_nsec = 0;

		if (socks_accept(socks, &timeout, &conn->client_sock) == NULL
		    || should_quit) {
			sock_delete(&conn->client_sock);
			_cw_free(conn);
		} else {
			if (opt_dirname != NULL) {
				cw_sint32_t	fd;

				conn->is_verbose = TRUE;

				conn->out = out_new(NULL, cw_g_mem);

				_cw_out_put_s(logfile,
				    "[s]/[s].pid_[i].conn[i]", opt_dirname,
				    g_progname, getpid(), conn_num);
				conn_num++;

				fd = (cw_sint32_t)open(logfile, O_RDWR | O_CREAT
				    | O_TRUNC, 0644);
				if (fd == -1) {
					if (dbg_is_registered(cw_g_dbg,
					    "prog_error")) {
						_cw_out_put_e("[s]: Error "
						    "opening \"[s]\": [s]\n",
						    g_progname, logfile,
						    strerror(errno));
					}
					out_delete(conn->out);
					conn->out = NULL;
				}
				out_default_fd_set(conn->out, fd);
			}
#ifdef _CW_IPFILTER
			if (opt_transparent) {
				if (ipnat_set_connect(conn)) {
					retval = 1;
					goto CLERROR;
				}
			} else {
#endif
				conn->rhost = strdup(opt_rhost);
				conn->rport = opt_rport;
#ifdef _CW_IPFILTER
			}
#endif
			if (opt_log)
				conn->is_verbose = TRUE;
			conn->format = opt_format;

			conn->send_thd = thd_new(handle_client_send, (void
			    *)conn);
		}
	}

	thd_join(handler_arg.sig_thd);

	socks_delete(socks);

	libsock_shutdown();
	CLERROR:
	libstash_shutdown();
	return retval;
}

void *
sig_handler(void *a_arg)
{
	extern cw_bool_t	should_quit;
	struct handler_s	*arg = (struct handler_s *)a_arg;
	int			sig, error;

	error = sigwait(&arg->hupset, &sig);
	if (error || (sig != SIGHUP && sig != SIGINT))
		_cw_error("sigwait() error");
	if (dbg_is_registered(cw_g_dbg, "prog_verbose"))
		out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "[s]: Caught signal\n", g_progname);
	should_quit = TRUE;

	return NULL;
}

char *
get_out_str_pretty(cw_buf_t *a_buf, cw_bool_t is_send, char *a_str)
{
	char		*retval, *p, *p_a, *p_b, *t_str;
	char		*c_trans, line_a[81], line_b[81];
	char		line_sep[81] = "         |                 |        "
	    "         |                 |\n";
	cw_uint32_t	str_len, buf_size, i, j;
	cw_uint8_t	c;
	static char *c_strs[] = {"nul", "soh", "stx", "etx", "eot", "enq",
				 "ack", "bel", "bs", "ht", "lf", "vt", "ff",
				 "cr", "so", "si", "dle", "dc1", "dc2", "dc3",
				 "dc4", "ack", "syn", "etb", "can", "em", "sub",
				 "ec", "fs", "gs", "rs", "us", "sp", "!", "\"",
				 "#", "$", "%", "&", "'", "(", ")", "*", "+",
				 ",", "-", ".", "/", "0", "1", "2", "3", "4",
				 "5", "6", "7", "8", "9", ":", ";", "<", "=",
				 ">", "?", "@", "A", "B", "C", "D", "E", "F",
				 "G", "H", "I", "J", "K", "L", "M", "N", "O",
				 "P", "Q", "R", "S", "T", "U", "V", "W", "X",
				 "Y", "Z", "[", "\\", "]", "^", "_", "`", "a",
				 "b", "c", "d", "e", "f", "g", "h", "i", "j",
				 "k", "l", "m", "n", "o", "p", "q", "r", "s",
				 "t", "u", "v", "w", "x", "y", "z", "{", "|",
				 "}", "~", "del"};
	size_t		len;

	buf_size = buf_size_get(a_buf);

	/* Re-alloc enough space to hold the out string. */
	str_len = (81		/* First dashed line. */
	    + 35		/* Header. */
	    + 1			/* Blank line. */
	    + 1			/* Blank line. */
	    + 81		/* Last dashed line. */
	    + 1)		/* Terminating \0. */
	    +(((buf_size / 16) + 1) * 227);	/*
						 * 16 bytes data prints as 227
						 * bytes.
						 */

	if (a_str == NULL) {
		/* Allocate for the first time. */
		retval = _cw_malloc(str_len);
	} else {
		/* Re-use a_str. */
		retval = _cw_realloc(a_str, str_len);
	}
	/* Clear the string. */
	retval[0] = '\0';
	p = retval;

	/* First dashed line. */
	t_str = "----------------------------------------"
	    "----------------------------------------\n";
	len = strlen(t_str);
	memcpy(p, t_str, len);
	p += len;

	len = _cw_out_put_s(line_a, "[s]:0x[i|b:16] ([i]) byte[s]\n", (is_send)
	    ? "send" : "recv", buf_size, buf_size, (buf_size != 1) ? "s" : "");
	memcpy(p, line_a, len);
	p += len;

	/* Blank line. */
	p[0] = '\n';
	p++;

	for (i = 0; i < buf_size; i += j) {
		/*
		 * Clear the line buffers and get them set up for printing
		 * character translations.
		 */
		line_a[0] = '\0';
		p_a = line_a;
		p_a += _cw_out_put_s(line_a, "[i|b:16|w:8|p:0]", i);

		line_b[0] = '\0';
		p_b = line_b;
		t_str = "        ";
		len = strlen(t_str);
		memcpy(line_b, t_str, len);
		p_b += len;

		/* Each iteration generates out text for 16 bytes of data. */
		for (j = 0; (j < 16) && ((i + j) < buf_size); j++) {
			if ((j % 4) == 0) {
				/* Print the word separators. */
				t_str = " |";
				len = strlen(t_str);
				memcpy(p_a, t_str, len);
				p_a += len;

				memcpy(p_b, t_str, len);
				p_b += len;
			}
			c = buf_uint8_get(a_buf, i + j);
			if (c < 128)
				c_trans = c_strs[(cw_uint32_t)c];
			else
				c_trans = "---";

			p_a += _cw_out_put_s(p_a, "  [i|b:16|w:2|p:0]", c);
			p_b += _cw_out_put_s(p_b, " [s|w:3]", c_trans);
		}
		/* Actually copy the strings to the final output string. */
		len = p_a - line_a;
		memcpy(p, line_a, len);
		p += len;

		p[0] = '\n';
		p++;

		len = p_b - line_b;
		memcpy(p, line_b, len);
		p += len;

		p[0] = '\n';
		p++;

		if ((i + j) < buf_size) {
			len = strlen(line_sep);
			memcpy(p, line_sep, len);
			p += len;
		} else {
			p[0] = '\n';
			p++;
		}
	}

	/* Last dashed line. */
	t_str = "----------------------------------------"
	    "----------------------------------------\n";
	len = strlen(t_str);
	memcpy(p, t_str, len);
	p += len;

	p[0] = '\0';

	return retval;
}

char *
get_out_str_hex(cw_buf_t *a_buf, cw_bool_t is_send, char *a_str)
{
	char		*retval, *p;
	char		*syms = "0123456789abcdef";
	cw_uint32_t	str_len, buf_size, i;
	cw_uint8_t	c;

	buf_size = buf_size_get(a_buf);

	/* Calculate the total size of the output. */
	str_len = (1		/* '<' or '>'. */
	    + (buf_size << 1)	/* Hex dump. */
	    + 1			/* Newline. */
	    + 1);		/* Null terminator. */

	if (a_str == NULL) {
		/* Allocate for the first time. */
		retval = _cw_malloc(str_len);
	} else {
		/* Re-use a_str. */
		retval = _cw_realloc(a_str, str_len);
	}
	p = retval;

	if (is_send)
		p[0] = '>';
	else
		p[0] = '<';
	p++;

	/* Hex dump. */
	for (i = 0; i < buf_size; i++) {
		c = buf_uint8_get(a_buf, i);
		p[0] = syms[c >> 4];
		p++;
		p[0] = syms[c & 0xf];
		p++;
	}

	/* Newline. */
	p[0] = '\n';
	p++;

	/* Null terminator. */
	p[0] = '\0';

	return retval;
}

char *
get_out_str_ascii(cw_buf_t *a_buf, cw_bool_t is_send, char *a_str)
{
	char		*retval, *t_str, *p;
	cw_uint32_t	str_len, buf_size, i;

	buf_size = buf_size_get(a_buf);

	/* Calculate the total size of the output. */
	str_len = (81		/* First dashed line. */
	    + 35		/* Header. */
	    + buf_size		/* Data. */
	    + 1			/* Newline. */
	    + 81		/* Last dashed line. */
	    + 1);		/* Null terminator. */

	if (a_str == NULL) {
		/* Allocate for the first time. */
		retval = _cw_malloc(str_len);
	} else {
		/* Re-use a_str. */
		retval = _cw_realloc(a_str, str_len);
	}
	/* Clear the string. */
	retval[0] = '\0';
	p = retval;

	/* First dashed line. */
	t_str = "----------------------------------------"
	    "----------------------------------------\n";
	strcpy(p, t_str);
	p += strlen(t_str);

	/* Header. */
	p += _cw_out_put_s(p, "[s]:0x[i|b:16] ([i]) byte[s]\n", (is_send) ?
	    "send" : "recv", buf_size, buf_size, (buf_size != 1) ? "s" : "");

	/* Data. */
	for (i = 0; i < buf_size; i++) {
		*p = buf_uint8_get(a_buf, i);
		p++;
	}

	p += _cw_out_put_s(p, "\n");

	/* Last dashed line. */
	t_str = "----------------------------------------"
	    "----------------------------------------\n";
	strcpy(p, t_str);

	return retval;
}

void *
handle_client_send(void *a_arg)
{
	cw_buf_t	buf;
	connection_t	*conn = (connection_t *) a_arg;
	char		*str = NULL;

	out_put(conn->out, "[s]: New connection\n", g_progname);

	buf_new(&buf, cw_g_mem);

	/* Finish initializing conn. */
	mtx_new(&conn->lock);
	conn->should_quit = FALSE;

	out_put(conn->out, "[s]: Connecting to \"[s]\" on port [i]\n",
	    g_progname, conn->rhost, conn->rport);

	/* Connect to the remote end. */
	sock_new(&conn->remote_sock, 16384);
	if (sock_connect(&conn->remote_sock, conn->rhost, conn->rport, NULL) !=
	    0) {
		out_put_e(conn->out, __FILE__, __LINE__, __FUNCTION__,
		    "[s]: Error in sock_connect(&conn->remote_sock, \"[s]\", "
		    "[i])\n", g_progname, conn->rhost, conn->rport);
		goto RETURN;
	}
	/*
	 * Okay, the connection is totally established now.  Create another
	 * thread to handle the data being sent from the remote end back to the
	 * client.  That allows both this thread and the new one to block,
	 * waiting for data, rather than polling.
	 */
	conn->recv_thd = thd_new(handle_client_recv, (void *)conn);

	/*
	 * Continually read data from the socket, create an out string, print to
	 * out, then send the data on.
	 */
	while (conn->should_quit == FALSE) {
		if (sock_read(&conn->client_sock, &buf, 0, NULL) < 0) {
			mtx_lock(&conn->lock);
			conn->should_quit = TRUE;
			mtx_unlock(&conn->lock);
			break;
		} else if (conn->should_quit)
			break;
		else {
			if (conn->is_verbose) {
				switch (conn->format) {
				case PRETTY:
					str = get_out_str_pretty(&buf, TRUE,
					    str);
					break;
				case HEX:
					str = get_out_str_hex(&buf, TRUE, str);
					break;
				case ASCII:
					str = get_out_str_ascii(&buf, TRUE,
					    str);
					break;
				default:
					_cw_not_reached();
				}

				out_put(conn->out, "[s]", str);
			}
			if ((sock_write(&conn->remote_sock, &buf)) ||
			    (sock_out_flush(&conn->remote_sock))) {
				mtx_lock(&conn->lock);
				conn->should_quit = TRUE;
				mtx_unlock(&conn->lock);
				break;
			}
		}
	}
	sock_disconnect(&conn->remote_sock);

	if (str != NULL)
		_cw_free(str);
	/* Join on the recv thread. */
	thd_join(conn->recv_thd);

	RETURN:
	/* Don't do this if the socket wasn't created. */
	sock_delete(&conn->remote_sock);

	/* Delete this thread. */
	thd_delete(conn->send_thd);

	/* Finish cleaning up conn. */
	mtx_delete(&conn->lock);
	sock_delete(&conn->client_sock);

	buf_delete(&buf);

	/* Allocated via strdup(). */
	free(conn->rhost);

	out_put(conn->out, "[s]: Connection closed\n", g_progname);
	if (conn->out != NULL)
		out_delete(conn->out);
	_cw_free(conn);

	return NULL;
}

void *
handle_client_recv(void *a_arg)
{
	cw_buf_t	buf;
	connection_t	*conn = (connection_t *) a_arg;
	char		*str = NULL;

	buf_new(&buf, cw_g_mem);

	/*
	 * Continually read data from the socket, create a string, print to the
	 * log, then send the data on.
	 */
	while (conn->should_quit == FALSE) {
		if ((sock_read(&conn->remote_sock, &buf, 0, NULL) < 0) ||
		    (conn->should_quit)) {
			mtx_lock(&conn->lock);
			conn->should_quit = TRUE;
			mtx_unlock(&conn->lock);
			break;
		} else {
			if (conn->is_verbose) {
				switch (conn->format) {
				case PRETTY:
					str = get_out_str_pretty(&buf, FALSE,
					    str);
					break;
				case HEX:
					str = get_out_str_hex(&buf, FALSE, str);
					break;
				case ASCII:
					str = get_out_str_ascii(&buf, FALSE,
					    str);
					break;
				default:
					_cw_not_reached();
				}

				out_put(conn->out, "[s]", str);
			}
			if ((sock_write(&conn->client_sock, &buf)) ||
			    (sock_out_flush(&conn->client_sock))) {
				mtx_lock(&conn->lock);
				conn->should_quit = TRUE;
				mtx_unlock(&conn->lock);
				break;
			}
		}
	}
	sock_disconnect(&conn->client_sock);

	if (str != NULL)
		_cw_free(str);
	buf_delete(&buf);
	return NULL;
}

#ifdef _CW_IPFILTER
cw_bool_t
ipnat_set_connect(connection_t *conn)
{
	cw_bool_t		retval;
	struct sockaddr_in	here, there;
	natlookup_t		natl, *natlp;
	int			fd;
	socklen_t		namelen;

	fd = sock_fd_get(&conn->client_sock);
	namelen = sizeof(here);
	if (getsockname(fd, (struct sockaddr *)&here, &namelen) == -1) {
		_cw_out_put_e("[s]: NAT: getsockname failed: [s]\n",
		    g_progname, strerror(errno));
		retval = TRUE;
		goto RETURN;
	}
	namelen = sizeof(there);
	if (getpeername(fd, (struct sockaddr *)&there, &namelen) == -1) {
		_cw_out_put_e("[s]: NAT: getpeername failed: [s]\n",
		    g_progname, strerror(errno));
		retval = TRUE;
		goto RETURN;
	}

	memset(&natl, 0, sizeof(natl));	
	natl.nl_outip = there.sin_addr;
	natl.nl_outport = there.sin_port;
	natl.nl_inip = here.sin_addr;
	natl.nl_inport = here.sin_port;
	natl.nl_flags = IPN_TCP;
	natlp = &natl;
	if (dbg_is_registered(cw_g_dbg, "prog_verbose")) {
		_cw_out_put("[s]: natlookup  out: [s],[i]\n", g_progname,
		    inet_ntoa(natl.nl_outip), ntohs(natl.nl_outport));
		_cw_out_put("[s]: natlookup   in: [s],[i]\n", g_progname,
		    inet_ntoa(natl.nl_inip), ntohs(natl.nl_inport));
	}
	fd = open(IPL_NAT, O_RDONLY);
	if (ioctl(fd, SIOCGNATL, &natlp) == -1) {
		_cw_out_put_e("[s]: NAT: ioctl failed: [s]", g_progname,
		    strerror(errno));
		close(fd);
		retval = TRUE;
		goto RETURN;
	}
	close(fd);
	if (dbg_is_registered(cw_g_dbg, "prog_verbose")) {
		_cw_out_put("[s]: natlookup real: [s],[i]\n", g_progname,
		    inet_ntoa(natl.nl_realip), ntohs(natl.nl_realport));
	}
	conn->rhost = strdup(inet_ntoa(natl.nl_realip));
	conn->rport = ntohs(natl.nl_realport);

	retval = FALSE;
	RETURN:
	return retval;
}
#endif

void
usage(void)
{
	_cw_out_put("[s] usage:\n"
	    "    [s] -h\n"
	    "    [s] -V\n"
	    "    [s] [[-v | -q] [[-f {p|h|a}] [[-l | -d <dirpath>]"
	    " [[-p <port>]", g_progname, g_progname, g_progname, g_progname);
#ifdef _CW_IPFILTER
	_cw_out_put(" [[-r [[<rhost>:]<rport> | -t]\n");
#else
	_cw_out_put(" -r [[<rhost>:]<rport>\n");
#endif
	_cw_out_put(
	    "\n"
	    "    Option               | Description\n"
	    "    ---------------------+------------------------------------------\n"
	    "    -h                   | Print usage and exit.\n"
	    "    -V                   | Print version information and exit.\n"
	    "    -v                   | Verbose.\n"
	    "    -q                   | Quiet.\n"
	    "    -l                   | Write logs to stderr.\n"
	    "    -d <dirpath>         | Write logs to \"<dirpath>/proxy.*\".\n"
	    "    -f <format>          | Data logging format.\n"
	    "                         |   p : Pretty (default).\n"
	    "                         |   h : Hex.\n"
	    "                         |   a : Ascii.\n"
	    "    -i <ip>              | Bind to interface with address <ip>.\n"
	    "                         | (Default \"ANY\".)\n"
	    "    -p <port>            | Listen on port <port>.\n"
	    "    -r [[<rhost>:]<rport> | Forward to host <rhost> or \"localhost\",\n"
	    "                         | port <rport>.\n");
#ifdef _CW_IPFILTER
	_cw_out_put(
	    "    -t                   | Use ipfilter via /dev/ipnat.\n");
#endif
}

void
version(void)
{
	_cw_out_put("[s], version [s]\n", g_progname, "<Version>");
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
