/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
 * Simple program that pipes input from stdin to a TCP socket, and pipes the
 * result to stdout.
 *
 ****************************************************************************/

#include <netdb.h>
#ifdef _CW_OS_LINUX
#  include <netinet/in.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define  _LIBSOCK_USE_SOCKS
#include "libsock/libsock.h"

typedef enum
{
  PRETTY, HEX, ASCII
} format_t;

/* Function prototypes. */
cw_sock_t *
client_setup(const char * a_rhost, int a_rport, struct timeval * a_timeout);
cw_sock_t *
server_setup(int a_port, struct timeval * a_timeout);
char *
get_out_str_pretty(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str);
char *
get_out_str_hex(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str);
char *
get_out_str_ascii(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str);
cw_bool_t
oom_handler(const void * a_data, cw_uint32_t a_size);
void
usage(void);
void
version(void);
const char *
basename(const char * a_str);

/* Global. */
const char * g_progname;
cw_out_t * log_out = NULL;

int
main(int argc, char ** argv)
{
  int retval = 0;
  cw_buf_t * buf = NULL;
  cw_sock_t * sock = NULL;
  cw_sock_t * sock_stdin = NULL;
  cw_sock_t * sock_stdout = NULL;
  cw_mq_t * mq = NULL;
  struct timeval * tout = NULL;

  int c;
  cw_bool_t opt_client = FALSE, opt_server = FALSE;
  cw_bool_t cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
  cw_bool_t opt_verbose = FALSE, opt_quiet = FALSE;
  int opt_rport = 0, opt_port = 0;
  char * opt_rhost = "localhost";
  cw_uint32_t opt_timeout = 0;
  char * opt_log = NULL;
  format_t opt_format = PRETTY;

  if (TRUE == libstash_init())
  {
    retval = 1;
    goto RETURN;
  }
  mem_set_oom_handler(cw_g_mem, oom_handler, NULL);

  g_progname = basename(argv[0]);
  
  dbg_register(cw_g_dbg, "ncat_error");
  
  while (-1 != (c = getopt(argc, argv, "hVvql:f:r:p:t:")))
  {
    switch (c)
    {
      case 'h':
      {
	opt_help = TRUE;
	break;
      }
      case 'V':
      {
	opt_version = TRUE;
	break;
      }
      case 'v':
      {
	if (TRUE == opt_verbose)
	{
	  dbg_register(cw_g_dbg, "sockb_verbose");
	  dbg_register(cw_g_dbg, "sockb_error");
	  dbg_register(cw_g_dbg, "socks_error");
	  dbg_register(cw_g_dbg, "sock_error");
	}
	else
	{
	  opt_verbose = TRUE;
	  dbg_register(cw_g_dbg, "ncat_verbose");
	}
	
	break;
      }
      case 'q':
      {
	opt_quiet = TRUE;
	dbg_unregister(cw_g_dbg, "ncat_error");
	
	break;
      }
      case 'l':
      {
	opt_log = optarg;
	break;
      }
      case 'f':
      {
	switch (*optarg)
	{
	  case 'p':
	  {
	    opt_format = PRETTY;
	    break;
	  }
	  case 'h':
	  {
	    opt_format = HEX;
	    break;
	  }
	  case 'a':
	  {
	    opt_format = ASCII;
	    break;
	  }
	  default:
	  {
	    cl_error = TRUE;
	    break;
	  }
	}
	    
	break;
      }
      case 'r':
      {
	char * colon, * port_str;
	struct servent * ent;

	opt_client = TRUE;
	
	colon = strstr(optarg, ":");
	if (NULL == colon)
	{
	  opt_rhost = "localhost";
	  port_str = optarg;
	}
	else
	{
	  colon[0] = '\0';
	  opt_rhost = optarg;
	  port_str = colon + 1;
	}

	if (NULL != (ent = getservbyname(port_str, "tcp")))
	{
	  opt_rport = (cw_uint32_t) ntohs(ent->s_port);
	}
	else
	{
	  opt_rport = strtoul(port_str, NULL, 10);
	}
	
	break;
      }
      case 'p':
      {
	struct servent * ent;
	
	opt_server = TRUE;

	if (NULL != (ent = getservbyname(optarg, "tcp")))
	{
	  opt_port = (cw_uint32_t) ntohs(ent->s_port);
	}
	else
	{
	  opt_port = strtoul(optarg, NULL, 10);
	}
	
	break;
      }
      case 't':
      {
	opt_timeout = strtoul(optarg, NULL, 10);
	
	tout = _cw_malloc(sizeof(struct timeval));
	tout->tv_sec = opt_timeout;
	tout->tv_usec = 0;
	break;
      }
      default:
      {
	cl_error = TRUE;
	break;
      }
    }
  }
  
  if ((TRUE == cl_error) || (optind < argc))
  {
    out_put(cw_g_out, "[s]: Unrecognized option(s)\n", g_progname);
    usage();
    retval = 1;
    goto RETURN;
  }
  
  if (TRUE == opt_help)
  {
    usage();
    goto RETURN;
  }

  if (TRUE == opt_version)
  {
    version();
    goto RETURN;
  }

  /* Check validity of command line options. */
  if ((TRUE == opt_verbose) && (TRUE == opt_quiet))
  {
    out_put(cw_g_out, "[s]: \"-v\" and \"-q\" are incompatible\n",
	    g_progname);
    usage();
    retval = 1;
    goto RETURN;
  }

  if (FALSE == opt_client)
  {
    if (FALSE == opt_server)
    {
      out_put(cw_g_out, "[s]: -p or -r must be specified\n",
	      g_progname);
      usage();
      retval = 1;
      goto RETURN;
    }
  }
  else if (TRUE == opt_server)
  {
    out_put(cw_g_out, "[s]: -p and -r are incompatible\n",
	    g_progname);
    usage();
    retval = 1;
    goto RETURN;
  }

  /* Open hex dump file if specified. */
  if (NULL != opt_log)
  {
    int fd;
    
    log_out = out_new(NULL);

    fd = open(opt_log, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    
    if (-1 == fd)
    {
      if (dbg_is_registered(cw_g_dbg, "ncat_verbose"))
      {
	out_put(cw_g_out, "[s]: Unable to open dump file \"[s]\"\n", opt_log);
      }
      retval = 1;
      goto RETURN;
    }

    out_set_default_fd(log_out, fd);
  }
  
  if (dbg_is_registered(cw_g_dbg, "ncat_verbose"))
  {
    out_put(cw_g_out, "[s]: pid: [i]\n", g_progname, getpid());
  }
  
  sockb_init(16, 4096, 4);

  if (TRUE == opt_client)
  {
    /* Try to connect to the server. */
    sock = client_setup(opt_rhost, opt_rport, tout);
  }
  else
  {
    /* Solicit a client connection. */
    sock = server_setup(opt_port, tout);
  }

  if (NULL == sock)
  {
    if (dbg_is_registered(cw_g_dbg, "ncat_error"))
    {
      out_put(cw_g_out, "[s]: Connection failure or timeout\n",
	      g_progname);
    }
  }
  else
  {
    struct timeval now;
    struct timezone tz;
    struct timespec timeout;
    struct timespec zero;
    int fd;
    cw_bool_t done_reading = FALSE;
    char * str = NULL;
    
    mq = mq_new(NULL);
    buf = buf_new(NULL);

    sock_stdin = sock_new(NULL, 16384);
    sock_wrap(sock_stdin, dup(0), FALSE);
    sockb_in_notify(mq, sock_get_fd(sock_stdin));
    
    sock_stdout = sock_new(NULL, 0);
    sock_wrap(sock_stdout, 1, FALSE);

    sockb_in_notify(mq, sock_get_fd(sock));

    zero.tv_sec = 0;
    zero.tv_nsec = 0;

    while (1)
    {
      if (NULL != tout)
      {
	bzero(&tz, sizeof(struct timezone));
	gettimeofday(&now, &tz);
	timeout.tv_sec = now.tv_sec + opt_timeout;
	timeout.tv_nsec = now.tv_usec * 1000;
      
	fd = (int) mq_timedget(mq, &timeout);
	if (0 == fd)
	{
	  /* Timeout. */
	  if (TRUE == done_reading)
	  {
	    /* We've sent all the input data to the peer, and have timed out
	     * waiting for response data.  Give up. */
	    break;
	  }
	  else
	  {
	    continue;
	  }
	}
      }
      else
      {
	fd = (int) mq_get(mq);

	if (0 == fd)
	{
	  break;
	}
      }

      if (sock_get_fd(sock_stdin) == fd)
      {
	if (0 >= sock_read(sock_stdin, buf, 0, &zero))
	{
	  sock_flush_out(sock_stdout);
	  done_reading = TRUE;
	}
	else
	{
	  if (NULL != opt_log)
	  {
	    /* Log. */
	    switch (opt_format)
	    {
	      case PRETTY:
	      {
		str = get_out_str_pretty(buf, TRUE, str);
		break;
	      }
	      case HEX:
	      {
		str = get_out_str_hex(buf, TRUE, str);
		break;
	      }
	      case ASCII:
	      {
		str = get_out_str_ascii(buf, TRUE, str);
		break;
	      }
	      default:
	      {
		_cw_error("Programming error");
	      }
	    }
	    
	    out_put(log_out, str);
	  }
	  
	  if (TRUE == sock_write(sock, buf))
	  {
	    sock_flush_out(sock_stdout);
	    done_reading = TRUE;
	  }
	}
      }
      else if (sock_get_fd(sock) == fd)
      {
	if (0 >= sock_read(sock, buf, 0, &zero))
	{
	  break;
	}
	else
	{
	  if (NULL != opt_log)
	  {
	    /* Log. */
	    switch (opt_format)
	    {
	      case PRETTY:
	      {
		str = get_out_str_pretty(buf, FALSE, str);
		break;
	      }
	      case HEX:
	      {
		str = get_out_str_hex(buf, FALSE, str);
		break;
	      }
	      case ASCII:
	      {
		str = get_out_str_ascii(buf, FALSE, str);
		break;
	      }
	      default:
	      {
		_cw_error("Programming error");
	      }
	    }
	    
	    out_put(log_out, str);
	  }
	  
	  if (TRUE == sock_write(sock_stdout, buf))
	  {
	    break;
	  }
	}
      }
      else /*  if (-1 == fd) */
      {
	break;
      }
    }
    if (NULL != str)
    {
      _cw_free(str);
    }
  }

  RETURN:
  if (NULL != buf)
  {
    buf_delete(buf);
  }
  if (NULL != sock)
  {
    sock_delete(sock);
  }
  if (NULL != sock_stdin)
  {
    sock_delete(sock_stdin);
  }
  if (NULL != sock_stdout)
  {
    sock_delete(sock_stdout);
  }
  if (NULL != mq)
  {
    mq_delete(mq);
  }
  if (NULL != tout)
  {
    _cw_free(tout);
  }
  if (NULL != log_out)
  {
    close(out_get_default_fd(log_out));
    out_delete(log_out);
  }
  sockb_shutdown();
  libstash_shutdown();
  return retval;
}

cw_sock_t *
client_setup(const char * a_rhost, int a_rport, struct timeval * a_timeout)
{
  cw_sock_t * retval;
  cw_sint32_t error;
  
  retval = sock_new(NULL, 32768);

  error = sock_connect(retval, a_rhost, a_rport, a_timeout);
  if (-1 == error)
  {
    if (dbg_is_registered(cw_g_dbg, "ncat_error"))
    {
      out_put(cw_g_out, "[s]: Error connecting to [s]:[i]\n",
	      g_progname, a_rhost, a_rport);
    }
    sock_delete(retval);
    retval = NULL;
  }
  else if (1 == error)
  {
    if (dbg_is_registered(cw_g_dbg, "ncat_error"))
    {
      out_put(cw_g_out, "[s]: Timeout connecting to [s]:[i]\n",
	      g_progname, a_rhost, a_rport);
    }
    sock_delete(retval);
    retval = NULL;
  }

  if (dbg_is_registered(cw_g_dbg, "ncat_verbose"))
  {
    out_put(cw_g_out, "[s]: Connected to [s]:[i]\n",
	    g_progname, a_rhost, a_rport);
  }
    
  return retval;
}

cw_sock_t *
server_setup(int a_port, struct timeval * a_timeout)
{
  cw_sock_t * retval;
  cw_socks_t * socks;
  int port, ask_port;

  socks = socks_new();

  port = ask_port = a_port;
  if (TRUE == socks_listen(socks, &port))
  {
    retval = NULL;
    goto RETURN;
  }

  if (0 == ask_port)
  {
    /* If the user didn't request a particular port, print out what port we got,
     * since without this info, running in server mode is rather useless. */
    if (dbg_is_registered(cw_g_dbg, "ncat_error"))
    {
      out_put(cw_g_out, "[s]: Listening on port [i]\n", g_progname, port);
    }
  }
  
  retval = sock_new(NULL, 32768);

  if (NULL == socks_accept(socks, a_timeout, retval))
  {
    sock_delete(retval);
    retval = NULL;
    goto RETURN;
  }

  if (dbg_is_registered(cw_g_dbg, "ncat_verbose"))
  {
    out_put(cw_g_out, "[s]: Connection established\n", g_progname);
  }

  RETURN:
  socks_delete(socks);
  return retval;
}

char *
get_out_str_pretty(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str)
{
  char * retval, * p, * p_a, * p_b, * t_str;
  char c_trans[4], line_a[81], line_b[81],
    line_sep[81]
    = "         |                 |                 |                 |\n";
  cw_uint32_t str_len, buf_size, i, j;
  cw_uint8_t c;
  size_t len;

  buf_size = buf_get_size(a_buf);

  /* Re-alloc enough space to hold the out string. */
  str_len = (81 /* First dashed line. */
	     + 35 /* Header. */
	     + 1 /* Blank line. */
	     + 1 /* Blank line. */
	     + 81 /* Last dashed line. */
	     + 1) /* Terminating \0. */
    + (((buf_size / 16) + 1) * 227); /* 16 bytes data prints as 227 bytes. */
  
  if (a_str == NULL)
  {
    /* Allocate for the first time. */
    retval = _cw_malloc(str_len);
  }
  else
  {
    /* Re-use a_str. */
    retval = _cw_realloc(a_str, str_len);
  }
  /* Clear the string. */
  retval[0] = '\0';
  p = retval;

  /* First dashed line. */
  t_str =
    "----------------------------------------"
    "----------------------------------------\n";
  len = strlen(t_str);
  memcpy(p, t_str, len);
  p += len;

  len = out_put_s(cw_g_out, line_a, "[s]:0x[i|b:16] ([i]) byte[s]\n",
		  (TRUE == is_send) ? "send" : "recv",
		  buf_size,
		  buf_size,
		  (buf_size != 1) ? "s" : "");
  memcpy(p, line_a, len);
  p += len;

  /* Blank line. */
  p[0] = '\n';
  p++;

  for (i = 0; i < buf_size; i += j)
  {
    /* Clear the line buffers and get them set up for printing character
     * translations. */
    line_a[0] = '\0';
    p_a = line_a;
    p_a += out_put_s(cw_g_out, line_a, "[i|b:16|w:8|p:0]", i);

    line_b[0] = '\0';
    p_b = line_b;
    t_str = "        ";
    len = strlen(t_str);
    memcpy(line_b, t_str, len);
    p_b += len;
    
    /* Each iteration generates out text for 16 bytes of data. */
    for (j = 0; (j < 16) && ((i + j) < buf_size); j++)
    {
      if ((j % 4) == 0)
      {
	/* Print the word separators. */
	t_str = " |";
	len = strlen(t_str);
	memcpy(p_a, t_str, len);
	p_a += len;

	memcpy(p_b, t_str, len);
	p_b += len;
      }
	
      c = buf_get_uint8(a_buf, i + j);
      
      switch (c)
      {
	case 0x00:
	  strcpy(c_trans, "nul");
	  break;
	case 0x01:
	  strcpy(c_trans, "soh");
	  break;
	case 0x02:
	  strcpy(c_trans, "stx");
	  break;
	case 0x03:
	  strcpy(c_trans, "etx");
	  break;
	case 0x04:
	  strcpy(c_trans, "eot");
	  break;
	case 0x05:
	  strcpy(c_trans, "enq");
	  break;
	case 0x06:
	  strcpy(c_trans, "ack");
	  break;
	case 0x07:
	  strcpy(c_trans, "bel");
	  break;
	case 0x08:
	  strcpy(c_trans, "bs");
	  break;
	case 0x09:
	  strcpy(c_trans, "ht");
	  break;
	case 0x0a:
	  strcpy(c_trans, "lf");
	  break;
	case 0x0b:
	  strcpy(c_trans, "vt");
	  break;
	case 0x0c:
	  strcpy(c_trans, "ff");
	  break;
	case 0x0d:
	  strcpy(c_trans, "cr");
	  break;
	case 0x0e:
	  strcpy(c_trans, "so");
	  break;
	case 0x0f:
	  strcpy(c_trans, "si");
	  break;
	  
	case 0x10:
	  strcpy(c_trans, "dle");
	  break;
	case 0x11:
	  strcpy(c_trans, "dc1");
	  break;
	case 0x12:
	  strcpy(c_trans, "dc2");
	  break;
	case 0x13:
	  strcpy(c_trans, "dc3");
	  break;
	case 0x14:
	  strcpy(c_trans, "dc4");
	  break;
	case 0x15:
	  strcpy(c_trans, "ack");
	  break;
	case 0x16:
	  strcpy(c_trans, "syn");
	  break;
	case 0x17:
	  strcpy(c_trans, "etb");
	  break;
	case 0x18:
	  strcpy(c_trans, "can");
	  break;
	case 0x19:
	  strcpy(c_trans, "em");
	  break;
	case 0x1a:
	  strcpy(c_trans, "sub");
	  break;
	case 0x1b:
	  strcpy(c_trans, "ec");
	  break;
	case 0x1c:
	  strcpy(c_trans, "fs");
	  break;
	case 0x1d:
	  strcpy(c_trans, "gs");
	  break;
	case 0x1e:
	  strcpy(c_trans, "rs");
	  break;
	case 0x1f:
	  strcpy(c_trans, "us");
	  break;

	case 0x20:
	  strcpy(c_trans, "sp");
	  break;
	case 0x21:
	  strcpy(c_trans, "!");
	  break;
	case 0x22:
	  strcpy(c_trans, "\"");
	  break;
	case 0x23:
	  strcpy(c_trans, "#");
	  break;
	case 0x24:
	  strcpy(c_trans, "$");
	  break;
	case 0x25:
	  strcpy(c_trans, "%");
	  break;
	case 0x26:
	  strcpy(c_trans, "&");
	  break;
	case 0x27:
	  strcpy(c_trans, "'");
	  break;
	case 0x28:
	  strcpy(c_trans, "(");
	  break;
	case 0x29:
	  strcpy(c_trans, ")");
	  break;
	case 0x2a:
	  strcpy(c_trans, "*");
	  break;
	case 0x2b:
	  strcpy(c_trans, "+");
	  break;
	case 0x2c:
	  strcpy(c_trans, ",");
	  break;
	case 0x2d:
	  strcpy(c_trans, "-");
	  break;
	case 0x2e:
	  strcpy(c_trans, ".");
	  break;
	case 0x2f:
	  strcpy(c_trans, "/");
	  break;

	case 0x30:
	  strcpy(c_trans, "0");
	  break;
	case 0x31:
	  strcpy(c_trans, "1");
	  break;
	case 0x32:
	  strcpy(c_trans, "2");
	  break;
	case 0x33:
	  strcpy(c_trans, "3");
	  break;
	case 0x34:
	  strcpy(c_trans, "4");
	  break;
	case 0x35:
	  strcpy(c_trans, "5");
	  break;
	case 0x36:
	  strcpy(c_trans, "6");
	  break;
	case 0x37:
	  strcpy(c_trans, "7");
	  break;
	case 0x38:
	  strcpy(c_trans, "8");
	  break;
	case 0x39:
	  strcpy(c_trans, "9");
	  break;
	case 0x3a:
	  strcpy(c_trans, ":");
	  break;
	case 0x3b:
	  strcpy(c_trans, ";");
	  break;
	case 0x3c:
	  strcpy(c_trans, "<");
	  break;
	case 0x3d:
	  strcpy(c_trans, "=");
	  break;
	case 0x3e:
	  strcpy(c_trans, ">");
	  break;
	case 0x3f:
	  strcpy(c_trans, "?");
	  break;

  	case 0x40:
	  strcpy(c_trans, "@");
	  break;
	case 0x41:
	  strcpy(c_trans, "A");
	  break;
	case 0x42:
	  strcpy(c_trans, "B");
	  break;
	case 0x43:
	  strcpy(c_trans, "C");
	  break;
	case 0x44:
	  strcpy(c_trans, "D");
	  break;
	case 0x45:
	  strcpy(c_trans, "E");
	  break;
	case 0x46:
	  strcpy(c_trans, "F");
	  break;
	case 0x47:
	  strcpy(c_trans, "G");
	  break;
	case 0x48:
	  strcpy(c_trans, "H");
	  break;
	case 0x49:
	  strcpy(c_trans, "I");
	  break;
	case 0x4a:
	  strcpy(c_trans, "J");
	  break;
	case 0x4b:
	  strcpy(c_trans, "K");
	  break;
	case 0x4c:
	  strcpy(c_trans, "L");
	  break;
	case 0x4d:
	  strcpy(c_trans, "M");
	  break;
	case 0x4e:
	  strcpy(c_trans, "N");
	  break;
	case 0x4f:
	  strcpy(c_trans, "O");
	  break;

  	case 0x50:
	  strcpy(c_trans, "P");
	  break;
	case 0x51:
	  strcpy(c_trans, "Q");
	  break;
	case 0x52:
	  strcpy(c_trans, "R");
	  break;
	case 0x53:
	  strcpy(c_trans, "S");
	  break;
	case 0x54:
	  strcpy(c_trans, "T");
	  break;
	case 0x55:
	  strcpy(c_trans, "U");
	  break;
	case 0x56:
	  strcpy(c_trans, "V");
	  break;
	case 0x57:
	  strcpy(c_trans, "W");
	  break;
	case 0x58:
	  strcpy(c_trans, "X");
	  break;
	case 0x59:
	  strcpy(c_trans, "Y");
	  break;
	case 0x5a:
	  strcpy(c_trans, "Z");
	  break;
	case 0x5b:
	  strcpy(c_trans, "[");
	  break;
	case 0x5c:
	  strcpy(c_trans, "\\");
	  break;
	case 0x5d:
	  strcpy(c_trans, "]");
	  break;
	case 0x5e:
	  strcpy(c_trans, "^");
	  break;
	case 0x5f:
	  strcpy(c_trans, "_");
	  break;

  	case 0x60:
	  strcpy(c_trans, "`");
	  break;
	case 0x61:
	  strcpy(c_trans, "a");
	  break;
	case 0x62:
	  strcpy(c_trans, "b");
	  break;
	case 0x63:
	  strcpy(c_trans, "c");
	  break;
	case 0x64:
	  strcpy(c_trans, "d");
	  break;
	case 0x65:
	  strcpy(c_trans, "e");
	  break;
	case 0x66:
	  strcpy(c_trans, "f");
	  break;
	case 0x67:
	  strcpy(c_trans, "g");
	  break;
	case 0x68:
	  strcpy(c_trans, "h");
	  break;
	case 0x69:
	  strcpy(c_trans, "i");
	  break;
	case 0x6a:
	  strcpy(c_trans, "j");
	  break;
	case 0x6b:
	  strcpy(c_trans, "k");
	  break;
	case 0x6c:
	  strcpy(c_trans, "l");
	  break;
	case 0x6d:
	  strcpy(c_trans, "m");
	  break;
	case 0x6e:
	  strcpy(c_trans, "n");
	  break;
	case 0x6f:
	  strcpy(c_trans, "o");
	  break;

  	case 0x70:
	  strcpy(c_trans, "p");
	  break;
	case 0x71:
	  strcpy(c_trans, "q");
	  break;
	case 0x72:
	  strcpy(c_trans, "r");
	  break;
	case 0x73:
	  strcpy(c_trans, "s");
	  break;
	case 0x74:
	  strcpy(c_trans, "t");
	  break;
	case 0x75:
	  strcpy(c_trans, "u");
	  break;
	case 0x76:
	  strcpy(c_trans, "v");
	  break;
	case 0x77:
	  strcpy(c_trans, "w");
	  break;
	case 0x78:
	  strcpy(c_trans, "x");
	  break;
	case 0x79:
	  strcpy(c_trans, "y");
	  break;
	case 0x7a:
	  strcpy(c_trans, "z");
	  break;
	case 0x7b:
	  strcpy(c_trans, "{");
	  break;
	case 0x7c:
	  strcpy(c_trans, "|");
	  break;
	case 0x7d:
	  strcpy(c_trans, "}");
	  break;
	case 0x7e:
	  strcpy(c_trans, "~");
	  break;
	case 0x7f:
	  strcpy(c_trans, "del");
	  break;

	default:
	  strcpy(c_trans, "---");
	  break;
      }

      p_a += out_put_s(cw_g_out, p_a, "  [i|b:16|w:2|p:0]", c);
      p_b += out_put_s(cw_g_out, p_b, " [s|w:3]", c_trans);
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

    if ((i + j) < buf_size)
    {
      len = strlen(line_sep);
      memcpy(p, line_sep, len);
      p += len;
    }
    else
    {
      p[0] = '\n';
      p++;
    }
  }

  /* Last dashed line. */
  t_str =
    "----------------------------------------"
    "----------------------------------------\n";
  len = strlen(t_str);
  memcpy(p, t_str, len);
  p += len;

  p[0] = '\0';
  
  return retval;
}

char *
get_out_str_hex(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str)
{
  char * retval, * t_str, * p;
  cw_uint32_t str_len, buf_size, i;

  buf_size = buf_get_size(a_buf);
  
  /* Calculate the total size of the output. */
  str_len = (81 /* First dashed line. */
	     + 35 /* Header. */
	     + (buf_size * 3) /* Hex dump. */
	     + 1 /* Newline. */
	     + 81 /* Last dashed line. */
	     );

  if (NULL == a_str)
  {
    /* Allocate for the first time. */
    retval = _cw_malloc(str_len);
  }
  else
  {
    /* Re-use a_str. */
    retval = _cw_realloc(a_str, str_len);
  }
  /* Clear the string. */
  retval[0] = '\0';
  p = retval;

  /* First dashed line. */
  t_str =
    "----------------------------------------"
    "----------------------------------------\n";
  strcpy(p, t_str);
  p += strlen(t_str);
  
  /* Header. */
  p += out_put_s(cw_g_out, p, "[s]:0x[i|b:16] ([i]) byte[s]\n",
		 (TRUE == is_send) ? "send" : "recv",
		 buf_size,
		 buf_size,
		 (buf_size != 1) ? "s" : "");

  /* Hex dump. */
  for (i = 0; i < buf_size; i++)
  {
    p += out_put_s(cw_g_out, p, "[i|b:16|w:2|p:0] ", buf_get_uint8(a_buf, i));
  }

  p += out_put_s(cw_g_out, p, "\n");
  
  /* Last dashed line. */
  t_str =
    "----------------------------------------"
    "----------------------------------------\n";
  strcpy(p, t_str);
  
  return retval;
}

char *
get_out_str_ascii(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str)
{
  char * retval, * t_str, * p;
  cw_uint32_t str_len, buf_size, i;

  buf_size = buf_get_size(a_buf);
  
  /* Calculate the total size of the output. */
  str_len = (81 /* First dashed line. */
	     + 35 /* Header. */
	     + buf_size /* Data. */
	     + 1 /* Newline. */
	     + 81 /* Last dashed line. */
	     );

  if (NULL == a_str)
  {
    /* Allocate for the first time. */
    retval = _cw_malloc(str_len);
  }
  else
  {
    /* Re-use a_str. */
    retval = _cw_realloc(a_str, str_len);
  }
  /* Clear the string. */
  retval[0] = '\0';
  p = retval;

  /* First dashed line. */
  t_str =
    "----------------------------------------"
    "----------------------------------------\n";
  strcpy(p, t_str);
  p += strlen(t_str);
  
  /* Header. */
  p += out_put_s(cw_g_out, p, "[s]:0x[i|b:16] ([i]) byte[s]\n",
		 (TRUE == is_send) ? "send" : "recv",
		 buf_size,
		 buf_size,
		 (buf_size != 1) ? "s" : "");

  /* Data. */
  for (i = 0; i < buf_size; i++)
  {
    *p = buf_get_uint8(a_buf, i);
    p++;
  }

  p += out_put_s(cw_g_out, p, "\n");
  
  /* Last dashed line. */
  t_str =
    "----------------------------------------"
    "----------------------------------------\n";
  strcpy(p, t_str);
  
  return retval;
}

cw_bool_t
oom_handler(const void * a_data, cw_uint32_t a_size)
{
  if (dbg_is_registered(cw_g_dbg, "ncat_error"))
  {
    out_put(cw_g_out, "[s]: Memory allocation error for size [i]\n",
	    g_progname, a_size);
  }
  exit(1);
  
  return FALSE;
}

void
usage(void)
{
  out_put
    (cw_g_out,
     "[s]: Usage:\n"
     "    [s] -h\n"
     "    [s] -V\n"
     "    [s] [[-v [[-v] | -q] [[-t <timeout>] -r [[<rhost>:]<rport>\n"
     "    [s] [[-v [[-v] | -q] [[-t <timeout>] -p <port>\n"
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
  out_put(cw_g_out,
	  "[s]: Version [s]\n",
	  g_progname, _LIBSOCK_VERSION);
}

/* Doesn't strip trailing '/' characters. */
const char *
basename(const char * a_str)
{
  const char * retval = NULL;
  cw_uint32_t i;

  _cw_check_ptr(a_str);

  i = strlen(a_str);
  if (i > 0)
  {
    for (i--; /* Back up to last non-null character. */
	 i > 0;
	 i--)
    {
      if (a_str[i] == '/')
      {
	retval = &a_str[i + 1];
	break;
      }
    }
  }

  if (retval == NULL)
  {
    retval = a_str;
  }

  return retval;
}
