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
 * Multi-threaded concurrent transparent proxy.
 *
 ****************************************************************************/

#define _LIBSOCK_USE_SOCKS
#include "libsock/libsock.h"

#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

struct handler_s
{
  sigset_t hupset;
  cw_thd_t sig_thd;
};

typedef enum
{
  PRETTY, HEX, ASCII
} format_t;

typedef struct
{
  cw_thd_t send_thd;
  cw_thd_t recv_thd;
  cw_mtx_t lock;
  cw_bool_t should_quit;
  cw_out_t * out;
  cw_sock_t client_sock;
  cw_sock_t remote_sock;

  char * rhost;
  int rport;

  cw_bool_t is_verbose;
  format_t format;
} connection_t;

cw_bool_t should_quit = FALSE;

/* Function prototypes. */
void *
sig_handler(void * a_arg);
char *
get_out_str_pretty(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str);
char *
get_out_str_hex(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str);
char *
get_out_str_ascii(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str);
void *
handle_client_send(void * a_arg);
void *
handle_client_recv(void * a_arg);
void
usage(const char * a_progname);
void
version(const char * a_progname);
const char *
basename(const char * a_str);

int
main(int argc, char ** argv)
{
  int retval = 0;
  cw_socks_t * socks;
  connection_t * conn;
  char logfile[2048];
  cw_uint32_t conn_num;
  struct timeval timeout;

  struct handler_s handler_arg;

  int c;
  cw_bool_t cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
  cw_bool_t opt_verbose = FALSE, opt_quiet = FALSE, opt_log = FALSE;
  format_t opt_format = PRETTY;
  int opt_port = 0, opt_rport = 0;
  char * opt_rhost = NULL, * opt_dirname = NULL;

  libstash_init();
  dbg_register(cw_g_dbg, "prog_error");
  dbg_register(cw_g_dbg, "sockb_error");
  dbg_register(cw_g_dbg, "socks_error");
  dbg_register(cw_g_dbg, "sock_error");
/*    dbg_register(cw_g_dbg, "sock_sockopt"); */
/*    dbg_register(cw_g_dbg, "mem_verbose"); */
/*    dbg_register(cw_g_dbg, "pezz_verbose"); */

  /* Parse command line. */
  while (-1 != (c = getopt(argc, argv, "hVvqp:r:lf:d:")))
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
	opt_verbose = TRUE;
	dbg_register(cw_g_dbg, "prog_verbose");
/*  	dbg_register(cw_g_dbg, "mem_verbose"); */
	dbg_register(cw_g_dbg, "sockb_verbose");
	dbg_register(cw_g_dbg, "socks_verbose");
/*  	dbg_register(cw_g_dbg, "sock_sockopt"); */
	/* Nothing uses this flag. */
/*      dbg_register(cw_g_dbg, "sock_verbose"); */
	break;
      }
      case 'q':
      {
	opt_quiet = TRUE;
	dbg_unregister(cw_g_dbg, "prog_error");
	dbg_unregister(cw_g_dbg, "mem_error");
	dbg_unregister(cw_g_dbg, "pezz_error");
	dbg_unregister(cw_g_dbg, "sockb_error");
	dbg_unregister(cw_g_dbg, "socks_error");
	dbg_unregister(cw_g_dbg, "sock_error");
	break;
      }
      case 'p':
      {
	opt_port = strtoul(optarg, NULL, 10);
	break;
      }
      case 'r':
      {
	char * colon;
	
	colon = strstr(optarg, ":");
	if (colon == NULL)
	{
	  opt_rhost = "localhost";
	  opt_rport = strtoul(optarg, NULL, 10);
	}
	else
	{
	  colon[0] = '\0';
	  opt_rhost = optarg;
	  opt_rport = strtoul(colon + 1, NULL, 10);
	}
	
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
      case 'l':
      {
	opt_log = TRUE;
	break;
      }
      case 'd':
      {
	opt_dirname = optarg;
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
    out_put(cw_g_out, "Unrecognized option(s)\n");
    usage(basename(argv[0]));
    retval = 1;
    goto CLERROR;
  }

  if (TRUE == opt_help)
  {
    usage(basename(argv[0]));
    goto CLERROR;
  }

  if (TRUE == opt_version)
  {
    version(basename(argv[0]));
    goto CLERROR;
  }

  /* Check validity of command line options. */
  if ((TRUE == opt_verbose) && (TRUE == opt_quiet))
  {
    out_put(cw_g_out, "\"-v\" and \"-q\" are incompatible\n");
    usage(basename(argv[0]));
    retval = 1;
    goto CLERROR;
  }
  
  if ((TRUE == opt_log) && (NULL != opt_dirname))
  {
    out_put(cw_g_out, "\"-l\" and \"-d\" are incompatible\n");
    usage(basename(argv[0]));
    retval = 1;
    goto CLERROR;
  }

  if ((NULL != opt_dirname) && (512 < strlen(opt_dirname)))
  {
    out_put(cw_g_out,
	    "Argument to \"-d\" flag is too long (512 bytes max)\n");
    usage(basename(argv[0]));
    retval = 1;
    goto CLERROR;
  }
  
  /* Set the per-thread signal masks such that only one thread will catch the
   * signal. */
  sigemptyset(&handler_arg.hupset);
  sigaddset(&handler_arg.hupset, SIGHUP);
  sigaddset(&handler_arg.hupset, SIGINT);
  thd_sigmask(SIG_BLOCK, &handler_arg.hupset);
  thd_new(&handler_arg.sig_thd,
	  sig_handler,
	  (void *) &handler_arg);
  
  if (NULL != opt_dirname)
  {
    cw_sint32_t fd;
    
    out_put_s(cw_g_out, logfile, "[s]/[s].pid_[i].log",
	      opt_dirname, basename(argv[0]), getpid());
    
    fd = (cw_sint32_t) open(logfile, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (-1 == fd)
    {
      if (dbg_is_registered(cw_g_dbg, "prog_error"))
      {
	out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		  "Error opening \"[s]\": [s]\n", logfile,
		  strerror(errno));
      }
    }
    out_set_default_fd(cw_g_out, fd);
  }
  
  if (dbg_is_registered(cw_g_dbg, "prog_verbose"))
  {
    out_put(cw_g_out, "pid: [i]\n", getpid());
  }

  if (sockb_init(1024, 2048, 4096))
  {
    _cw_error("Initialization failure");
  }
  
  socks = socks_new();
  if (TRUE == socks_listen(socks, &opt_port))
  {
    exit(1);
  }
  if (dbg_is_registered(cw_g_dbg, "prog_verbose"))
  {
    out_put_l(cw_g_out, "[s]: Listening on port [i]\n", argv[0], opt_port);
  }

  for (conn_num = 0; should_quit == FALSE; conn_num++)
  {
    conn = _cw_malloc(sizeof(connection_t));
    if (NULL == conn)
    {
      out_put(cw_g_out, "malloc() error\n");
      exit(1);
    }
    
    bzero(conn, sizeof(connection_t));
    sock_new(&conn->client_sock, 4096);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    if (NULL == socks_accept(socks, &timeout, &conn->client_sock)
	|| should_quit)
    {
      sock_delete(&conn->client_sock);
      _cw_free(conn);
    }
    else
    {
      if (NULL != opt_dirname)
      {
	cw_sint32_t fd;
	
	conn->is_verbose = TRUE;
	
	conn->out = out_new(NULL);
	if (NULL == conn->out)
	{
	  out_put(cw_g_out, "malloc() error\n");
	  exit(1);
	}

	out_put_s(cw_g_out, logfile, "[s]/[s].pid_[i].conn[i]",
		  opt_dirname, basename(argv[0]), getpid(), conn_num);

	fd = (cw_sint32_t) open(logfile, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (-1 == fd)
	{
	  if (dbg_is_registered(cw_g_dbg, "prog_error"))
	  {
	    out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		      "Error opening \"[s]\": [s]\n", logfile,
		      strerror(errno));
	  }
	  out_delete(conn->out);
	  conn->out = NULL;
	}
	out_set_default_fd(conn->out, fd);
      }

      conn->rhost = opt_rhost;
      conn->rport = opt_rport;
      if (opt_log)
      {
	conn->is_verbose = TRUE;
      }
      conn->format = opt_format;
      	    
      thd_new(&conn->send_thd, handle_client_send, (void *) conn);
    }
  }

  thd_join(&handler_arg.sig_thd);

  socks_delete(socks);
  
  sockb_shutdown();
  CLERROR:
  libstash_shutdown();
  return retval;
}

void *
sig_handler(void * a_arg)
{
  extern cw_bool_t should_quit;
  struct handler_s * arg = (struct handler_s *) a_arg;
  int sig, error;

  error = sigwait(&arg->hupset, &sig);
  if (error || (sig != SIGHUP && sig != SIGINT))
  {
    _cw_error("sigwait() error");
  }
  if (dbg_is_registered(cw_g_dbg, "prog_verbose"))
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__, "Caught signal\n");
  }
  
  should_quit = TRUE;

  return NULL;
}

char *
get_out_str_pretty(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str)
{
  char * retval;
  char c_trans[4], line_a[81], line_b[81],
    line_sep[81]
    = "         |                 |                 |                 |\n";
  cw_uint32_t str_len, buf_size, i, j;
  cw_uint8_t c;

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
    if (NULL == retval)
    {
      out_put(cw_g_out, "malloc() error\n");
      exit(1);
    }
  }
  else
  {
    /* Re-use a_str. */
    retval = _cw_realloc(a_str, str_len);
    if (NULL == retval)
    {
      out_put(cw_g_out, "malloc() error\n");
      exit(1);
    }
  }
  /* Clear the string. */
  retval[0] = '\0';

  /* First dashed line. */
  strcat(retval,
	 "----------------------------------------"
	 "----------------------------------------\n");

  out_put_s(cw_g_out, line_a, "[s]:0x[i|b:16] ([i]) byte[s]\n",
	    (TRUE == is_send) ? "send" : "recv",
	    buf_size,
	    buf_size,
	    (buf_size != 1) ? "s" : "");
  strcat(retval, line_a);

  /* Blank line. */
  strcat(retval, "\n");

  for (i = 0; i < buf_size; i += j)
  {
    /* Clear the line buffers and get them set up for printing character
     * translations. */
    line_a[0] = '\0';
    line_b[0] = '\0';
    out_put_s(cw_g_out, line_a, "[i|b:16|w:8|p:0]", i);
    strcat(line_b, "        ");
    
    /* Each iteration generates out text for 16 bytes of data. */
    for (j = 0; (j < 16) && ((i + j) < buf_size); j++)
    {
      if ((j % 4) == 0)
      {
	/* Print the word separators. */
	strcat(line_a, " |");
	strcat(line_b, " |");
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

      out_put_s(cw_g_out, line_a + strlen(line_a), "  [i|b:16|w:2|p:0]", c);
      out_put_s(cw_g_out, line_b + strlen(line_b), " [s|w:3]", c_trans);
    }
    /* Actually copy the strings to the final output string. */
    strcat(retval, line_a);
    strcat(retval, "\n");
    
    strcat(retval, line_b);
    strcat(retval, "\n");

    if ((i + j) < buf_size)
    {
      strcat(retval, line_sep);
    }
    else
    {
      strcat(retval, "\n");
    }
  }

  /* Last dashed line. */
  strcat(retval,
	 "----------------------------------------"
	 "----------------------------------------\n");

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
    if (NULL == retval)
    {
      out_put(cw_g_out, "malloc() error\n");
      exit(1);
    }
  }
  else
  {
    /* Re-use a_str. */
    retval = _cw_realloc(a_str, str_len);
    if (NULL == retval)
    {
      out_put(cw_g_out, "malloc() error\n");
      exit(1);
    }
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
/*    p += strlen(t_str); */
  
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
    if (NULL == retval)
    {
      out_put(cw_g_out, "malloc() error\n");
      exit(1);
    }
  }
  else
  {
    /* Re-use a_str. */
    retval = _cw_realloc(a_str, str_len);
    if (NULL == retval)
    {
      out_put(cw_g_out, "malloc() error\n");
      exit(1);
    }
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
/*    p += strlen(t_str); */
  
  return retval;
}

void *
handle_client_send(void * a_arg)
{
  cw_buf_t buf;
  connection_t * conn = (connection_t *) a_arg;
  char * str = NULL;
/*    char * hostname = NULL, * port_str = NULL; */
/*    cw_bool_t parse_error = TRUE; */

  out_put(conn->out, "New connection\n");
  
  buf_new(&buf);

  /* Finish initializing conn. */
  mtx_new(&conn->lock);
  conn->should_quit = FALSE;

  /* XXX Parse the proxy options from the socket stream.  The syntax is:
   * hostname:port[\r]\n */

  if ((conn->rhost == NULL) || (conn->rport == 0))
  {
    /* XXX Parse the options in the socket stream. */
    _cw_error("Stream option parsing unimplemented");
    goto OPTERROR;
  }

  out_put(conn->out, "Connecting to \"[s]\" on port [i]\n",
	  conn->rhost, conn->rport);
      
  /* Connect to the remote end. */
  sock_new(&conn->remote_sock, 4096);
  if (0 != sock_connect(&conn->remote_sock, conn->rhost, conn->rport, NULL))
  {
    out_put_e(conn->out, __FILE__, __LINE__, __FUNCTION__,
	      "Error in sock_connect(&conn->remote_sock, \"[s]\", [i])\n",
	      conn->rhost, conn->rport);
    goto RETURN;
  }
  
  /* Okay, the connection is totally established now.  Create another thread to
   * handle the data being sent from the remote end back to the client.  That
   * allows both this thread and the new one to block, waiting for data, rather
   * than polling. */
  thd_new(&conn->recv_thd, handle_client_recv, (void *) conn);

  /* Continually read data from the socket, create a out string, print to out,
   * then send the data on. */
  while (FALSE == conn->should_quit)
  {
    if (0 > sock_read(&conn->client_sock, &buf, 0, NULL))
    {
      mtx_lock(&conn->lock);
      conn->should_quit = TRUE;
      mtx_unlock(&conn->lock);
      break;
    }
    else if (conn->should_quit)
    {
      break;
    }
    else
    {
      if (conn->is_verbose)
      {
	switch (conn->format)
	{
	  case PRETTY:
	  {
	    str = get_out_str_pretty(&buf, TRUE, str);
	    break;
	  }
	  case HEX:
	  {
	    str = get_out_str_hex(&buf, TRUE, str);
	    break;
	  }
	  case ASCII:
	  {
	    str = get_out_str_ascii(&buf, TRUE, str);
	    break;
	  }
	  default:
	  {
	    _cw_error("Programming error");
	    break;
	  }
	}
	
	out_put(conn->out, "[s]", str);
      }
      
      if ((TRUE == sock_write(&conn->remote_sock, &buf))
	  || (TRUE == sock_flush_out(&conn->remote_sock)))
      {
	mtx_lock(&conn->lock);
	conn->should_quit = TRUE;
	mtx_unlock(&conn->lock);
	break;
      }
    }
  }
  sock_disconnect(&conn->remote_sock);

  if (NULL != str)
  {
    _cw_free(str);
  }
    
/*    JOIN: */
  /* Join on the recv thread. */
  thd_join(&conn->recv_thd);

  RETURN:
  /* Don't do this if the socket wasn't created. */
  sock_delete(&conn->remote_sock);
  
  OPTERROR:
/*    _cw_free(hostname); */
/*    _cw_free(port_str); */
  
  /* Delete this thread. */
  thd_delete(&conn->send_thd);

  /* Finish cleaning up conn. */
  mtx_delete(&conn->lock);
  sock_delete(&conn->client_sock);

  buf_delete(&buf);

  out_put(conn->out, "Connection closed\n");
  if (NULL != conn->out)
  {
    out_delete(conn->out);
  }
  _cw_free(conn);
  
  return NULL;
}

void *
handle_client_recv(void * a_arg)
{
  cw_buf_t buf;
  connection_t * conn = (connection_t *) a_arg;
  char * str = NULL;

  buf_new(&buf);
  
  /* Continually read data from the socket, create a string, print to the log,
   * then send the data on. */
  while (FALSE == conn->should_quit)
  {
    if ((0 > sock_read(&conn->remote_sock, &buf, 0, NULL))
	|| (conn->should_quit))
    {
      mtx_lock(&conn->lock);
      conn->should_quit = TRUE;
      mtx_unlock(&conn->lock);
      break;
    }
    else
    {
      if (conn->is_verbose)
      {
	switch (conn->format)
	{
	  case PRETTY:
	  {
	    str = get_out_str_pretty(&buf, TRUE, str);
	    break;
	  }
	  case HEX:
	  {
	    str = get_out_str_hex(&buf, TRUE, str);
	    break;
	  }
	  case ASCII:
	  {
	    str = get_out_str_ascii(&buf, TRUE, str);
	    break;
	  }
	  default:
	  {
	    _cw_error("Programming error");
	    break;
	  }
	}
	
	out_put(conn->out, "[s]", str);
      }
      
      if ((TRUE == sock_write(&conn->client_sock, &buf))
	  || (TRUE == sock_flush_out(&conn->client_sock)))
      {
	mtx_lock(&conn->lock);
	conn->should_quit = TRUE;
	mtx_unlock(&conn->lock);
	break;
      }
    }
  }
  sock_disconnect(&conn->client_sock);

  if (NULL != str)
  {
    _cw_free(str);
  }
  buf_delete(&buf);
  return NULL;
}

void
usage(const char * a_progname)
{
  out_put
    (cw_g_out,
     "[s] usage:\n"
     "    [s] -h\n"
     "    [s] -V\n"
     "    [s] [[-v | -q] [[-f {p|h|a}] [[-l | -d <dirpath>] -p <port> -r [[<rhost>:]<rport>\n"
     "\n"
     "    Option               | Description\n"
     "    ---------------------+------------------------------------------\n"
     "    -h                   | Print usage and exit.\n"
     "    -V                   | Print version information and exit.\n"
     "    -v                   | Verbose.\n"
     "    -q                   | Quiet.\n"
     "    -f <format>          | Data logging format.\n"
     "                         |   p : Pretty (default).\n"
     "                         |   h : Hex.\n"
     "                         |   a : Ascii.\n"
     "    -l                   | Write logs to stderr.\n"
     "    -d <dirpath>         | Write logs to \"<dirpath>/proxy.*\".\n"
     "    -p <port>            | Listen on port <port>.\n"
     "    -r [[<rhost>:]<rport> | Forward to host <rhost> or \"localhost\",\n"
     "                         | port <rport>.\n",
     a_progname, a_progname, a_progname, a_progname
     );
}

void
version(const char * a_progname)
{
  out_put(cw_g_out,
	  "[s], version [s]\n",
	  a_progname, _LIBSOCK_VERSION);
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
