/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Simple program that pipes input from stdin to a TCP socket, and pipes the
 * result to stdout.
 *
 ******************************************************************************/

#include <netdb.h>
#ifdef _CW_OS_LINUX
#include <netinet/in.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <libsock/libsock.h>

typedef enum {
	NONE, PRETTY, HEX, ASCII
}       format_t;

/* Function prototypes. */
cw_sock_t 	*client_setup(const char *a_rhost, int a_rport, struct timespec
    *a_timeout);
cw_sock_t	*server_setup(int a_port, struct timespec *a_timeout);
char		*get_out_str_pretty(cw_buf_t *a_buf, cw_bool_t is_send, char
    *a_str);
char		*get_out_str_hex(cw_buf_t *a_buf, cw_bool_t is_send, char
    *a_str);
char		*get_out_str_ascii(cw_buf_t *a_buf, cw_bool_t is_send, char
    *a_str);
cw_bool_t	oom_handler(const void *a_data, cw_uint32_t a_size);
void		usage(void);
void		version(void);
const char	*basename(const char *a_str);

/* Global. */
const char	*g_progname;
cw_out_t	*log_out = NULL;

int
main(int argc, char **argv)
{
	int		retval = 0;
	cw_buf_t	*buf = NULL;
	cw_sock_t	*sock = NULL;
	cw_sock_t	*sock_stdin = NULL;
	cw_sock_t	*sock_stdout = NULL;
	cw_mq_t		*mq = NULL;
	struct timespec	*tout = NULL;

	int		c;
	cw_bool_t	opt_client = FALSE, opt_server = FALSE;
	cw_bool_t	cl_error = FALSE;
	cw_bool_t	opt_verbose = FALSE, opt_quiet = FALSE;
	int		opt_port = 0;
	char		*opt_rhost = "localhost";
	char		*opt_log = NULL;
	format_t	opt_format = NONE;

	if (libstash_init()) {
		retval = 1;
		goto RETURN;
	}
	mem_set_oom_handler(cw_g_mem, oom_handler, NULL);

	g_progname = basename(argv[0]);

	dbg_register(cw_g_dbg, "ncat_error");

	while ((c = getopt(argc, argv, "hVvql:f:r:p:t:")) != -1) {
		switch (c) {
		case 'h':
			usage();
			goto RETURN;
		case 'V':
			version();
			goto RETURN;
		case 'v':
			if (opt_verbose) {
				dbg_register(cw_g_dbg, "libsock_verbose");
				dbg_register(cw_g_dbg, "libsock_error");
				dbg_register(cw_g_dbg, "socks_error");
				dbg_register(cw_g_dbg, "sock_error");
			} else {
				opt_verbose = TRUE;
				dbg_register(cw_g_dbg, "ncat_verbose");
			}

			break;
		case 'q':
			opt_quiet = TRUE;
			dbg_unregister(cw_g_dbg, "ncat_error");

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
		_cw_out_put("[s]: Unrecognized option(s)\n", g_progname);
		usage();
		retval = 1;
		goto RETURN;
	}
	/* Check validity of command line options. */
	if (opt_verbose && opt_quiet) {
		_cw_out_put("[s]: \"-v\" and \"-q\" are incompatible\n",
		    g_progname);
		usage();
		retval = 1;
		goto RETURN;
	}
	if (opt_client == FALSE) {
		if (opt_server == FALSE) {
			_cw_out_put("[s]: -p or -r must be specified\n",
			    g_progname);
			usage();
			retval = 1;
			goto RETURN;
		}
	} else if (opt_server) {
		_cw_out_put("[s]: -p and -r are incompatible\n", g_progname);
		usage();
		retval = 1;
		goto RETURN;
	}
	if ((opt_log == NULL) && (opt_format != NONE)) {
		_cw_out_put("[s]: -f requires -l to be specified\n",
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
			if (dbg_is_registered(cw_g_dbg, "ncat_verbose")) {
				_cw_out_put("[s]: Unable to open log file"
				    " \"[s]\"\n", opt_log);
			}
			retval = 1;
			goto RETURN;
		}
		out_set_default_fd(log_out, fd);
	}
	if (dbg_is_registered(cw_g_dbg, "ncat_verbose"))
		_cw_out_put("[s]: pid: [i]\n", g_progname, getpid());
	libsock_init(16, 4096, 4);

	if (opt_client) {
		/* Try to connect to the server. */
		sock = client_setup(opt_rhost, opt_port, tout);
	} else {
		/* Solicit a client connection. */
		sock = server_setup(opt_port, tout);
	}

	if (sock == NULL) {
		if (dbg_is_registered(cw_g_dbg, "ncat_error")) {
			if (opt_client) {
				_cw_out_put("[s]: Connection failure or "
				    "timeout\n", g_progname);
			} else {
				_cw_out_put("[s]: Error listening on port"
				    " [i] or timeout\n", g_progname, opt_port);
			}
		}
	} else {
		struct timespec	zero;
		int		fd, fd_sock, fd_sock_stdin;
		cw_sint32_t	bytes_read;
		cw_bool_t	done_reading = FALSE;
		char		*str = NULL;

		mq = mq_new(NULL, cw_g_mem, sizeof(int));
		buf = buf_new(NULL, cw_g_mem);

		sock_stdin = sock_new(NULL, 16384);
		sock_wrap(sock_stdin, dup(0), FALSE);
		fd_sock_stdin = sock_fd_get(sock_stdin);
		libsock_in_notify(mq, fd_sock_stdin);

		sock_stdout = sock_new(NULL, 0);
		sock_wrap(sock_stdout, 1, FALSE);

		fd_sock = sock_fd_get(sock);
		libsock_in_notify(mq, fd_sock);

		zero.tv_sec = 0;
		zero.tv_nsec = 0;

		for (;;) {
			if ((tout != NULL) && (done_reading)) {
				/*
				 * If mq_timedget() times out, set fd to 0,
				 * which will cause the program to quit.
				 */
				if (mq_timedget(mq, tout, &fd))
					fd = 0;
			} else
				mq_get(mq, &fd);

			if (fd == fd_sock_stdin) {
				do {
					bytes_read = sock_read(sock_stdin, buf,
					    0, &zero);

					/* Log. */
					switch (opt_format) {
					case NONE:
						break;
					case PRETTY:
						str = get_out_str_pretty(buf,
						    TRUE, str);
						out_put(log_out, str);
						break;
					case HEX:
						str = get_out_str_hex(buf, TRUE,
						    str);
						out_put(log_out, str);
						break;
					case ASCII:
						str = get_out_str_ascii(buf,
						    TRUE, str);
						out_put(log_out, str);
						break;
					default:
						_cw_error("Programming error");
					}

					if (sock_write(sock, buf)) {
						sock_out_flush(sock_stdout);

						/*
						 * We're two loops deep here, so
						 * our choices are to check a
						 * variable every time around
						 * the outer while loop, or use
						 * a goto (not much different
						 * than break, really).
						 */
						goto DONE;
					}
				} while (sock_buffered_in(sock_stdin) > 0);

				if (bytes_read < 0) {
					sock_out_flush(sock_stdout);
					done_reading = TRUE;
				}
			} else if (fd == fd_sock) {
				do {
					bytes_read = sock_read(sock, buf, 0,
					    &zero);

					/* Log. */
					switch (opt_format) {
					case NONE:
						break;
					case PRETTY:
						str = get_out_str_pretty(buf,
						    FALSE, str);
						out_put(log_out, str);
						break;
					case HEX:
						str = get_out_str_hex(buf,
						    FALSE, str);
						out_put(log_out, str);
						break;
					case ASCII:
						str = get_out_str_ascii(buf,
						    FALSE, str);
						out_put(log_out, str);
						break;
					default:
						_cw_error("Programming error");
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
	}

	RETURN:
	if (buf != NULL)
		buf_delete(buf);
	if (sock != NULL)
		sock_delete(sock);
	if (sock_stdin != NULL)
		sock_delete(sock_stdin);
	if (sock_stdout != NULL)
		sock_delete(sock_stdout);
	if (mq != NULL)
		mq_delete(mq);
	if (tout != NULL)
		_cw_free(tout);
	if (log_out != NULL) {
		close(out_get_default_fd(log_out));
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
		if (dbg_is_registered(cw_g_dbg, "ncat_error")) {
			_cw_out_put("[s]: Error connecting to [s]:[i]\n",
			    g_progname, a_rhost, a_rport);
		}
		sock_delete(retval);

		retval = NULL;
	} else if (error == 1) {
		if (dbg_is_registered(cw_g_dbg, "ncat_error")) {
			_cw_out_put("[s]: Timeout connecting to [s]:[i]\n",
			    g_progname, a_rhost, a_rport);
		}
		sock_delete(retval);
		retval = NULL;
	}
	if (dbg_is_registered(cw_g_dbg, "ncat_verbose")) {
		_cw_out_put("[s]: Connected to [s]:[i]\n", g_progname, a_rhost,
		    a_rport);
	}
	return retval;
}

cw_sock_t *
server_setup(int a_port, struct timespec * a_timeout)
{
	cw_sock_t	*retval;
	cw_socks_t	*socks;
	int		port, ask_port;

        socks = socks_new();

        port = ask_port = a_port;
	if (socks_listen(socks, INADDR_ANY, &port)) {
		retval = NULL;
		goto RETURN;
	}
	if (ask_port == 0) {
		/*
		 * If the user didn't request a particular port, print out what
		 * port we got, since without this info, running in server mode
		 * is rather useless.
		 */
		if (dbg_is_registered(cw_g_dbg, "ncat_error")) {
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
	if (dbg_is_registered(cw_g_dbg, "ncat_verbose"))
		_cw_out_put("[s]: Connection established\n", g_progname);
	RETURN:
	socks_delete(socks);
	return retval;
}

char   *
get_out_str_pretty(cw_buf_t *a_buf, cw_bool_t is_send, char *a_str)
{
	char		*retval, *p, *p_a, *p_b, *t_str;
	char		*c_trans, line_a[81], line_b[81];
	char		line_sep[81] =
	    "         |                 |                 |                 |\n";
	cw_uint32_t	str_len, buf_size, i, j;
	cw_uint8_t	c;
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

	len = _cw_out_put_s(line_a, "[s]:0x[i|b:16] ([i]) byte[s]\n",
	    (is_send) ? "send" : "recv", buf_size, buf_size, (buf_size != 1) ?
	    "s" : "");
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

			switch (c) {
			case 0x00:
				c_trans = "nul";
				break;
			case 0x01:
				c_trans = "soh";
				break;
			case 0x02:
				c_trans = "stx";
				break;
			case 0x03:
				c_trans = "etx";
				break;
			case 0x04:
				c_trans = "eot";
				break;
			case 0x05:
				c_trans = "enq";
				break;
			case 0x06:
				c_trans = "ack";
				break;
			case 0x07:
				c_trans = "bel";
				break;
			case 0x08:
				c_trans = "bs";
				break;
			case 0x09:
				c_trans = "ht";
				break;
			case 0x0a:
				c_trans = "lf";
				break;
			case 0x0b:
				c_trans = "vt";
				break;
			case 0x0c:
				c_trans = "ff";
				break;
			case 0x0d:
				c_trans = "cr";
				break;
			case 0x0e:
				c_trans = "so";
				break;
			case 0x0f:
				c_trans = "si";
				break;

			case 0x10:
				c_trans = "dle";
				break;
			case 0x11:
				c_trans = "dc1";
				break;
			case 0x12:
				c_trans = "dc2";
				break;
			case 0x13:
				c_trans = "dc3";
				break;
			case 0x14:
				c_trans = "dc4";
				break;
			case 0x15:
				c_trans = "ack";
				break;
			case 0x16:
				c_trans = "syn";
				break;
			case 0x17:
				c_trans = "etb";
				break;
			case 0x18:
				c_trans = "can";
				break;
			case 0x19:
				c_trans = "em";
				break;
			case 0x1a:
				c_trans = "sub";
				break;
			case 0x1b:
				c_trans = "ec";
				break;
			case 0x1c:
				c_trans = "fs";
				break;
			case 0x1d:
				c_trans = "gs";
				break;
			case 0x1e:
				c_trans = "rs";
				break;
			case 0x1f:
				c_trans = "us";
				break;

			case 0x20:
				c_trans = "sp";
				break;
			case 0x21:
				c_trans = "!";
				break;
			case 0x22:
				c_trans = "\"";
				break;
			case 0x23:
				c_trans = "#";
				break;
			case 0x24:
				c_trans = "$";
				break;
			case 0x25:
				c_trans = "%";
				break;
			case 0x26:
				c_trans = "&";
				break;
			case 0x27:
				c_trans = "'";
				break;
			case 0x28:
				c_trans = "(";
				break;
			case 0x29:
				c_trans = ")";
				break;
			case 0x2a:
				c_trans = "*";
				break;
			case 0x2b:
				c_trans = "+";
				break;
			case 0x2c:
				c_trans = ",";
				break;
			case 0x2d:
				c_trans = "-";
				break;
			case 0x2e:
				c_trans = ".";
				break;
			case 0x2f:
				c_trans = "/";
				break;

			case 0x30:
				c_trans = "0";
				break;
			case 0x31:
				c_trans = "1";
				break;
			case 0x32:
				c_trans = "2";
				break;
			case 0x33:
				c_trans = "3";
				break;
			case 0x34:
				c_trans = "4";
				break;
			case 0x35:
				c_trans = "5";
				break;
			case 0x36:
				c_trans = "6";
				break;
			case 0x37:
				c_trans = "7";
				break;
			case 0x38:
				c_trans = "8";
				break;
			case 0x39:
				c_trans = "9";
				break;
			case 0x3a:
				c_trans = ":";
				break;
			case 0x3b:
				c_trans = ";";
				break;
			case 0x3c:
				c_trans = "<";
				break;
			case 0x3d:
				c_trans = "=";
				break;
			case 0x3e:
				c_trans = ">";
				break;
			case 0x3f:
				c_trans = "?";
				break;

			case 0x40:
				c_trans = "@";
				break;
			case 0x41:
				c_trans = "A";
				break;
			case 0x42:
				c_trans = "B";
				break;
			case 0x43:
				c_trans = "C";
				break;
			case 0x44:
				c_trans = "D";
				break;
			case 0x45:
				c_trans = "E";
				break;
			case 0x46:
				c_trans = "F";
				break;
			case 0x47:
				c_trans = "G";
				break;
			case 0x48:
				c_trans = "H";
				break;
			case 0x49:
				c_trans = "I";
				break;
			case 0x4a:
				c_trans = "J";
				break;
			case 0x4b:
				c_trans = "K";
				break;
			case 0x4c:
				c_trans = "L";
				break;
			case 0x4d:
				c_trans = "M";
				break;
			case 0x4e:
				c_trans = "N";
				break;
			case 0x4f:
				c_trans = "O";
				break;

			case 0x50:
				c_trans = "P";
				break;
			case 0x51:
				c_trans = "Q";
				break;
			case 0x52:
				c_trans = "R";
				break;
			case 0x53:
				c_trans = "S";
				break;
			case 0x54:
				c_trans = "T";
				break;
			case 0x55:
				c_trans = "U";
				break;
			case 0x56:
				c_trans = "V";
				break;
			case 0x57:
				c_trans = "W";
				break;
			case 0x58:
				c_trans = "X";
				break;
			case 0x59:
				c_trans = "Y";
				break;
			case 0x5a:
				c_trans = "Z";
				break;
			case 0x5b:
				c_trans = "[";
				break;
			case 0x5c:
				c_trans = "\\";
				break;
			case 0x5d:
				c_trans = "]";
				break;
			case 0x5e:
				c_trans = "^";
				break;
			case 0x5f:
				c_trans = "_";
				break;

			case 0x60:
				c_trans = "`";
				break;
			case 0x61:
				c_trans = "a";
				break;
			case 0x62:
				c_trans = "b";
				break;
			case 0x63:
				c_trans = "c";
				break;
			case 0x64:
				c_trans = "d";
				break;
			case 0x65:
				c_trans = "e";
				break;
			case 0x66:
				c_trans = "f";
				break;
			case 0x67:
				c_trans = "g";
				break;
			case 0x68:
				c_trans = "h";
				break;
			case 0x69:
				c_trans = "i";
				break;
			case 0x6a:
				c_trans = "j";
				break;
			case 0x6b:
				c_trans = "k";
				break;
			case 0x6c:
				c_trans = "l";
				break;
			case 0x6d:
				c_trans = "m";
				break;
			case 0x6e:
				c_trans = "n";
				break;
			case 0x6f:
				c_trans = "o";
				break;

			case 0x70:
				c_trans = "p";
				break;
			case 0x71:
				c_trans = "q";
				break;
			case 0x72:
				c_trans = "r";
				break;
			case 0x73:
				c_trans = "s";
				break;
			case 0x74:
				c_trans = "t";
				break;
			case 0x75:
				c_trans = "u";
				break;
			case 0x76:
				c_trans = "v";
				break;
			case 0x77:
				c_trans = "w";
				break;
			case 0x78:
				c_trans = "x";
				break;
			case 0x79:
				c_trans = "y";
				break;
			case 0x7a:
				c_trans = "z";
				break;
			case 0x7b:
				c_trans = "{";
				break;
			case 0x7c:
				c_trans = "|";
				break;
			case 0x7d:
				c_trans = "}";
				break;
			case 0x7e:
				c_trans = "~";
				break;
			case 0x7f:
				c_trans = "del";
				break;

			default:
				c_trans = "---";
				break;
			}

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

char   *
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

cw_bool_t
oom_handler(const void *a_data, cw_uint32_t a_size)
{
	if (dbg_is_registered(cw_g_dbg, "ncat_error")) {
		_cw_out_put("[s]: Memory allocation error for size [i]\n",
		    g_progname, a_size);
	}
	exit(1);

	return FALSE;
}

void
usage(void)
{
	_cw_out_put("[s]: Usage:\n"
	    "    [s] -h\n"
	    "    [s] -V\n"
	    "    [s] [[-v [[-v] | -q] [[-l <logfile> [[-f <format>]] [[-t <timeout>] -r [[<rhost>:]<rport>\n"
	    "    [s] [[-v [[-v] | -q] [[-l <logfile> [[-f <format>]] [[-t <timeout>] -p <port>\n"
	    "\n"
	    "    Option               | Description\n"
	    "    ---------------------+-----------------------------------------------------\n"
	    "    -h                   | Print usage and exit.\n"
	    "    -V                   | Print version information and exit.\n"
	    "    -v                   | Verbose.\n"
	    "    -q                   | Quiet.\n"
	    "    -l <logfile>         | Write a log to <logfile>.\n"
	    "    -f <format>          | Data logging format.\n"
	    "                         |   p : Pretty (default).\n"
	    "                         |   h : Hex.\n"
	    "                         |   a : Ascii.\n"
	    "    -t <timeout>         | Quit after <timeout> seconds, if stdin is closed, or\n"
	    "                         | if when listening, no one connects.\n"
	    "    -r [[<rhost>:]<rport> | Connect to <rhost>:<rport> or localhost:<rport>.\n"
	    "    -p <port>            | Listen for a connection on port <port>.\n",
	    g_progname, g_progname, g_progname, g_progname, g_progname);
}

void
version(void)
{
	_cw_out_put("[s]: Version [s]\n", g_progname, _LIBSOCK_VERSION);
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
