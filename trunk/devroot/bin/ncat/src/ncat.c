/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Like cat, but either stdin or stdout is instead a TCP socket.
 *
 ******************************************************************************/

#include <libsock/libsock.h>

#include "ncat_defs.h"

#include <netdb.h>
#ifdef _CW_OS_LINUX
#include <netinet/in.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

typedef enum {
	NONE, PRETTY, HEX, ASCII
}       format_t;

/* Function prototypes. */
cw_sock_t 	*client_setup(const char *a_rhost, int a_rport, struct timespec
    *a_timeout);
cw_sock_t	*server_setup(int a_port, cw_uint32_t a_ip, struct timespec
    *a_timeout);
char		*get_out_str_pretty(cw_buf_t *a_buf, cw_bool_t is_send, char
    *a_str);
char		*get_out_str_hex(cw_buf_t *a_buf, cw_bool_t is_send, char
    *a_str);
char		*get_out_str_ascii(cw_buf_t *a_buf, cw_bool_t is_send, char
    *a_str);
void		usage(void);
void		version(void);
const char	*basename(const char *a_str);

/* Global. */
const char	*g_progname;
cw_uint32_t	g_verbosity = 1; /* 0: silent, 1: error, 2: verbose */
cw_out_t	*log_out = NULL;

int
main(int argc, char **argv)
{
	int		retval = 0;
	cw_buf_t	*buf = NULL;
	cw_sock_t	*sock = NULL;
	cw_sock_t	*sock_stdin = NULL;
	cw_sock_t	*sock_stdout = NULL;
	cw_mq_t		mq;
	struct timespec	*tout = NULL;

	int		c;
	cw_bool_t	opt_client = FALSE, opt_server = FALSE;
	cw_bool_t	cl_error = FALSE;
	cw_bool_t	opt_verbose = FALSE, opt_quiet = FALSE;
	int		opt_port = 0;
	cw_uint32_t	opt_ip = htonl(INADDR_ANY);
	char		*opt_rhost = "localhost";
	char		*opt_log = NULL;
	format_t	opt_format = NONE;

	struct timespec	zero;
	cw_sint32_t	bytes_read;
	cw_bool_t	done_reading = FALSE;
	char		*str = NULL;
	cw_sock_t	*notify_sock;

	libstash_init();

	g_progname = basename(argv[0]);

	while ((c = getopt(argc, argv, "hVvql:f:r:p:i:t:")) != -1) {
		switch (c) {
		case 'h':
			usage();
			goto RETURN;
		case 'V':
			version();
			goto RETURN;
		case 'v':
			opt_verbose = TRUE;
			g_verbosity = 2;

			break;
		case 'q':
			opt_quiet = TRUE;
			g_verbosity = 0;

			break;
		case 'l':
			if (opt_format == NONE) {
				/*
				 * Set the default logging format if the user
				 * hasn't already specified a logging format.
				 */
				opt_format = PRETTY;
			}
			opt_log = optarg;
			break;
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
		case 'r': {
			char		*colon, *port_str;
			struct servent	*ent;

			opt_client = TRUE;

			colon = strstr(optarg, ":");
			if (colon == NULL) {
				opt_rhost = "localhost";
				port_str = optarg;
			} else {
				colon[0] = '\0';
				opt_rhost = optarg;
				port_str = colon + 1;
			}

			if ((ent = getservbyname(port_str, "tcp")) != NULL)
				opt_port = (cw_uint32_t)ntohs(ent->s_port);
			else
				opt_port = strtoul(port_str, NULL, 10);

			break;
		}
		case 'p': {
			struct servent	*ent;

			opt_server = TRUE;

			if ((ent = getservbyname(optarg, "tcp")) != NULL)
				opt_port = (cw_uint32_t)ntohs(ent->s_port);
			else
				opt_port = strtoul(optarg, NULL, 10);

			break;
		}
		case 'i':
			if (strcmp(optarg, "ANY") == 0) {
				/* Same as default. */
				opt_ip = htonl(INADDR_ANY);
			} else if (strcmp(optarg, "LOOPBACK") == 0)
				opt_ip = htonl(INADDR_LOOPBACK);
			else {
#ifdef _CW_HAVE_INET_ATON
				struct in_addr	addr;

				if (inet_aton(optarg, &addr) == 0) {
					/* Conversion error. */
					out_put(out_err, "[s]: Invalid IP "
					    "address specified with \"-i\" "
					    "flag\n", g_progname);
					usage();
					retval = 1;
					goto RETURN;
				}
				opt_ip = addr.s_addr;
#else
				opt_ip = inet_addr(optarg);
#endif
			}
			break;
		case 't':
			tout = _cw_malloc(sizeof(struct timespec));
			tout->tv_sec = strtoul(optarg, NULL, 10);
			tout->tv_nsec = 0;
			break;
		default:
			cl_error = TRUE;
			break;
		}
	}

	if ((cl_error) || (optind < argc)) {
		out_put(out_err, "[s]: Unrecognized option(s)\n", g_progname);
		usage();
		retval = 1;
		goto RETURN;
	}
	/* Check validity of command line options. */
	if (opt_verbose && opt_quiet) {
		out_put(out_err, "[s]: \"-v\" and \"-q\" are incompatible\n",
		    g_progname);
		usage();
		retval = 1;
		goto RETURN;
	}
	if (opt_client == FALSE) {
		if (opt_server == FALSE) {
			out_put(out_err, "[s]: -p or -r must be specified\n",
			    g_progname);
			usage();
			retval = 1;
			goto RETURN;
		}
	} else if (opt_server) {
		out_put(out_err, "[s]: -p and -r are incompatible\n",
		    g_progname);
		usage();
		retval = 1;
		goto RETURN;
	}
	if ((opt_log == NULL) && (opt_format != NONE)) {
		out_put(out_err, "[s]: -f requires -l to be specified\n",
		    g_progname);
		usage();
		retval = 1;
		goto RETURN;
	}
	/* Open log file if specified. */
	if (opt_log != NULL) {
		int	fd;

		log_out = out_new(NULL, cw_g_mem);

		fd = open(opt_log, O_CREAT | O_WRONLY | O_TRUNC, 0644);

		if (fd == -1) {
			if (g_verbosity == 2) {
				out_put(out_err, "[s]: Unable to open log file"
				    " \"[s]\"\n", g_progname, opt_log);
			}
			retval = 1;
			goto RETURN;
		}
		out_default_fd_set(log_out, fd);
	}
	if (g_verbosity == 2)
		_cw_out_put("[s]: pid: [i]\n", g_progname, getpid());
	libsock_init(16, 4096, 4);

	if (opt_client) {
		/* Try to connect to the server. */
		sock = client_setup(opt_rhost, opt_port, tout);
	} else {
		/* Solicit a client connection. */
		sock = server_setup(opt_port, opt_ip, tout);
	}

	if (sock == NULL) {
		if (g_verbosity > 0) {
			if (opt_client) {
				out_put(out_err, "[s]: Connection failure or "
				    "timeout\n", g_progname);
			} else {
				out_put(out_err, "[s]: Error listening on port"
				    " [i] or timeout\n", g_progname, opt_port);
			}
		}
		goto RETURN;
	}

	mq_new(&mq, cw_g_mem, sizeof(cw_sock_t *));
	buf = buf_new(NULL, cw_g_mem);

	sock_stdin = sock_new(NULL, 16384);
	sock_wrap(sock_stdin, 0, FALSE);
	libsock_in_notify(&mq, sock_stdin, sock_stdin);

	sock_stdout = sock_new(NULL, 0);
	sock_wrap(sock_stdout, 1, FALSE);

	libsock_in_notify(&mq, sock, sock);

	zero.tv_sec = 0;
	zero.tv_nsec = 0;

	for (;;) {
		if ((tout != NULL) && (done_reading)) {
			if (mq_timedget(&mq, tout, &notify_sock))
				break;
		} else
			mq_get(&mq, &notify_sock);

		if (notify_sock == sock_stdin) {
			do {
				bytes_read = sock_read(sock_stdin, buf, 0,
				    &zero);

				/* Log. */
				switch (opt_format) {
				case NONE:
					break;
				case PRETTY:
					str = get_out_str_pretty(buf, TRUE,
					    str);
					out_put(log_out, str);
					break;
				case HEX:
					str = get_out_str_hex(buf, TRUE, str);
					out_put(log_out, str);
					break;
				case ASCII:
					str = get_out_str_ascii(buf, TRUE, str);
					out_put(log_out, str);
					break;
				default:
					_cw_not_reached();
				}

				if (sock_write(sock, buf)) {
					sock_out_flush(sock_stdout);

					/*
					 * We're two loops deep here, so our
					 * choices are to check a variable every
					 * time around the outer while loop, or
					 * use a goto (not much different than
					 * break, really).
					 */
					goto DONE;
				}
			} while (sock_buffered_in(sock_stdin) > 0);

			if (bytes_read < 0) {
				sock_out_flush(sock_stdout);
				done_reading = TRUE;
			}
		} else if (notify_sock == sock) {
			do {
				bytes_read = sock_read(sock, buf, 0, &zero);

				/* Log. */
				switch (opt_format) {
				case NONE:
					break;
				case PRETTY:
					str = get_out_str_pretty(buf, FALSE,
					    str);
					out_put(log_out, str);
					break;
				case HEX:
					str = get_out_str_hex(buf, FALSE, str);
					out_put(log_out, str);
					break;
				case ASCII:
					str = get_out_str_ascii(buf, FALSE,
					    str);
					out_put(log_out, str);
					break;
				default:
					_cw_not_reached();
				}

				if (sock_write(sock_stdout, buf))
					break;
			} while (sock_buffered_in(sock) > 0);

			if (bytes_read < 0) {
				/*
				 * The peer has disconnected.  Flush any
				 * buffered data to stdout, then quit.
				 */
				sock_out_flush(sock_stdout);
				break;
			}
		} else {
			/* Timeout. */
			break;
		}
	}

	DONE:
	if (str != NULL)
		_cw_free(str);

	RETURN:
	if (buf != NULL)
		buf_delete(buf);
	if (sock != NULL)
		sock_delete(sock);
	if (sock_stdin != NULL)
		sock_delete(sock_stdin);
	if (sock_stdout != NULL)
		sock_delete(sock_stdout);
	if (tout != NULL)
		_cw_free(tout);
	if (log_out != NULL) {
		close(out_default_fd_get(log_out));
		out_delete(log_out);
	}
	libsock_shutdown();
	libstash_shutdown();
	return retval;
}

cw_sock_t *
client_setup(const char *a_rhost, int a_rport, struct timespec * a_timeout)
{
	cw_sock_t	*retval;
	cw_sint32_t	error;

        retval = sock_new(NULL, 32768);

        error = sock_connect(retval, a_rhost, a_rport, a_timeout);
	if (error == -1) {
		if (g_verbosity > 0) {
			out_put(out_err, "[s]: Error connecting to [s]:[i]\n",
			    g_progname, a_rhost, a_rport);
		}
		sock_delete(retval);

		retval = NULL;
	} else if (error == 1) {
		if (g_verbosity > 0) {
			out_put(out_err, "[s]: Timeout connecting to [s]:[i]\n",
			    g_progname, a_rhost, a_rport);
		}
		sock_delete(retval);
		retval = NULL;
	}
	if (g_verbosity == 2) {
		_cw_out_put("[s]: Connected to [s]:[i]\n", g_progname, a_rhost,
		    a_rport);
	}
	return retval;
}

cw_sock_t *
server_setup(int a_port, cw_uint32_t a_ip, struct timespec *a_timeout)
{
	cw_sock_t	*retval;
	cw_socks_t	*socks;
	int		port, ask_port;

        socks = socks_new();

        port = ask_port = a_port;
	if (socks_listen(socks, a_ip, &port)) {
		retval = NULL;
		goto RETURN;
	}
	if (ask_port == 0) {
		/*
		 * If the user didn't request a particular port, print out what
		 * port we got, since without this info, running in server mode
		 * is rather useless.
		 */
		if (g_verbosity > 0) {
			_cw_out_put("[s]: Listening on port [i]\n", g_progname,
			    port);
		}
	}
	retval = sock_new(NULL, 32768);

	if (socks_accept(socks, a_timeout, retval) == NULL) {
		sock_delete(retval);
		retval = NULL;
		goto RETURN;
	}
	if (g_verbosity == 2)
		_cw_out_put("[s]: Connection established\n", g_progname);
	RETURN:
	socks_delete(socks);
	return retval;
}

char *
get_out_str_pretty(cw_buf_t *a_buf, cw_bool_t is_send, char *a_str)
{
	char		*retval, *p, *p_a, *p_b, *t_str;
	char		*syms = "0123456789abcdef";
	char		*c_trans, line_a[81], line_b[81];
	char		line_sep[81] = "         |                 |        "
	    "         |                 |\n";
	cw_uint32_t	str_len, buf_size, i, j;
	cw_uint8_t	c;
	static char *c_strs[] = {"nul", "soh", "stx", "etx", "eot", "enq",
				 "ack", "bel", " bs", " ht", " lf", " vt",
				 " ff", " cr", " so", " si", "dle", "dc1",
				 "dc2", "dc3", "dc4", "ack", "syn", "etb",
				 "can", " em", "sub", " ec", " fs", " gs",
				 " rs", " us", " sp", "  !", "  \"", "  #",
				 "  $", "  %", "  &", "  '", "  (", "  )",
				 "  *", "  +", "  ,", "  -", "  .", "  /",
				 "  0", "  1", "  2", "  3", "  4", "  5",
				 "  6", "  7", "  8", "  9", "  :", "  ;",
				 "  <", "  =", "  >", "  ?", "  @", "  A",
				 "  B", "  C", "  D", "  E", "  F", "  G",
				 "  H", "  I", "  J", "  K", "  L", "  M",
				 "  N", "  O", "  P", "  Q", "  R", "  S",
				 "  T", "  U", "  V", "  W", "  X", "  Y",
				 "  Z", "  [", "  \\", "  ]", "  ^", "  _",
				 "  `", "  a", "  b", "  c", "  d", "  e",
				 "  f", "  g", "  h", "  i", "  j", "  k",
				 "  l", "  m", "  n", "  o", "  p", "  q",
				 "  r", "  s", "  t", "  u", "  v", "  w",
				 "  x", "  y", "  z", "  {", "  |", "  }",
				 "  ~", "del"};
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

		/*
		 * Each iteration through the enclosing loop generates out text
		 * for 16 bytes of data.  Optimize the inner loop to not use
		 * out_put_s(), though it would be easier to use it.
		 */
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

			/* Hex codes. */
			*p_a = ' ';
			p_a++;
			*p_a = ' ';
			p_a++;
			*p_a = syms[c >> 4];
			p_a++;
			*p_a = syms[c & 0xf];
			p_a++;

			/* Symbols. */
			*p_b = ' ';
			p_b++;
			memcpy(p_b, c_trans, 3);
			p_b += 3;
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

void
usage(void)
{
	_cw_out_put("[s]: Usage:\n"
	    "  [s] -h\n"
	    "  [s] -V\n"
	    "  [s] [[-v | -q] [[-l <logfile> [[-f <format>]] [[-t <timeout>] -r [[<rhost>:]<rport>\n"
	    "  [s] [[-v | -q] [[-l <logfile> [[-f <format>]] [[-t <timeout>] [[-i <ip>] -p <port>\n"
	    "\n"
	    "  Option               | Description\n"
	    "  ---------------------+-----------------------------------------------------\n"
	    "  -h                   | Print usage and exit.\n"
	    "  -V                   | Print version information and exit.\n"
	    "  -v                   | Verbose.\n"
	    "  -q                   | Quiet.\n"
	    "  -l <logfile>         | Write a log to <logfile>.\n"
	    "  -f <format>          | Data logging format.\n"
	    "                       |   p : Pretty (default).\n"
	    "                       |   h : Hex.\n"
	    "                       |   a : Ascii.\n"
	    "  -t <timeout>         | Quit after <timeout> seconds, if stdin is closed, or\n"
	    "                       | if when listening, no one connects.\n"
	    "  -i <ip>              | Bind to interface with address <ip>.\n"
	    "                       | (Default \"ANY\".)\n"
	    "  -r [[<rhost>:]<rport> | Connect to <rhost>:<rport> or localhost:<rport>.\n"
	    "  -p <port>            | Listen for a connection on port <port>.\n",
	    g_progname, g_progname, g_progname, g_progname, g_progname);
}

void
version(void)
{
	_cw_out_put("[s]: Version [s]\n", g_progname, "<Version>");
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
