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

#include "libsock/libsock.h"

cw_bool_t
oom_handler(const void * a_data, cw_uint32_t a_size);
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
  cw_sint32_t error;
  cw_buf_t * buf = NULL;
  cw_sock_t * sock = NULL;
  cw_sock_t * sock_stdin = NULL;
  cw_sock_t * sock_stdout = NULL;
  cw_mq_t * mq = NULL;
  struct timeval tout;

  int c;
  cw_bool_t cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
  cw_bool_t opt_verbose = FALSE, opt_quiet = FALSE;
  int opt_rport = 0;
  char * opt_rhost = "localhost";
  cw_uint32_t opt_timeout = 0;

  if (TRUE == libstash_init())
  {
    retval = 1;
    goto RETURN;
  }
  mem_set_oom_handler(cw_g_mem, oom_handler, NULL);

  dbg_register(cw_g_dbg, "prog_error");
  dbg_register(cw_g_dbg, "sockb_error");
  dbg_register(cw_g_dbg, "sock_error");
  
  while (-1 != (c = getopt(argc, argv, "hVvqr:t:")))
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
	dbg_register(cw_g_dbg, "sockb_verbose");
	dbg_register(cw_g_dbg, "socks_verbose");
	break;
      }
      case 'q':
      {
	opt_quiet = TRUE;
	dbg_unregister(cw_g_dbg, "prog_error");
	dbg_unregister(cw_g_dbg, "sockb_error");
	dbg_unregister(cw_g_dbg, "sock_error");
	dbg_unregister(cw_g_dbg, "mem_error");
	dbg_unregister(cw_g_dbg, "pezz_error");
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
      case 't':
      {
	opt_timeout = strtoul(optarg, NULL, 10);
	tout.tv_sec = opt_timeout;
	tout.tv_usec = 0;
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
    out_put(cw_g_out, "[s]: Unrecognized option(s)\n", basename(argv[0]));
    usage(basename(argv[0]));
    retval = 1;
    goto RETURN;
  }
  
  if (TRUE == opt_help)
  {
    usage(basename(argv[0]));
    goto RETURN;
  }

  if (TRUE == opt_version)
  {
    version(basename(argv[0]));
    goto RETURN;
  }

  /* Check validity of command line options. */
  if ((TRUE == opt_verbose) && (TRUE == opt_quiet))
  {
    out_put(cw_g_out, "[s]: \"-v\" and \"-q\" are incompatible\n",
	    basename(argv[0]));
    usage(basename(argv[0]));
    retval = 1;
    goto RETURN;
  }

  if (0 == opt_rport)
  {
    out_put(cw_g_out, "[s]: Invalid <rport> [i]\n",
	    basename(argv[0]), opt_rport);
    usage(basename(argv[0]));
    retval = 1;
    goto RETURN;
  }

  if (dbg_is_registered(cw_g_dbg, "prog_verbose"))
  {
    out_put(cw_g_out, "[s]: pid: [i]\n", basename(argv[0]), getpid());
  }
  
  sockb_init(16, 4096, 4);

  sock = sock_new(NULL, 32768);

  error = sock_connect(sock, opt_rhost, opt_rport, &tout);
  if (-1 == error)
  {
    if (dbg_is_registered(cw_g_dbg, "prog_error"))
    {
      out_put(cw_g_out, "[s]: Error connecting to [s]:[i]\n",
	      basename(argv[0]), opt_rhost, opt_rport);
    }
  }
  else if (1 == error)
  {
    if (dbg_is_registered(cw_g_dbg, "prog_error"))
    {
      out_put(cw_g_out, "[s]: Timeout connecting to [s]:[i]\n",
	      basename(argv[0]), opt_rhost, opt_rport);
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
    cw_bool_t done_writing = FALSE;
    
    if (dbg_is_registered(cw_g_dbg, "prog_verbose"))
    {
      out_put(cw_g_out, "[s]: Connected to [s]:[i]\n",
	      basename(argv[0]), opt_rhost, opt_rport);
    }

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
      if (0 != opt_timeout)
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
	    /* We've written all the data, and have timed out waiting for
	     * response data.  Give up. */
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
	fd = (int) mq_timedget(mq, &timeout);

	if (0 == fd)
	{
	  break;
	}
      }

      if (sock_get_fd(sock_stdin) == fd)
      {
	if ((0 >= sock_read(sock_stdin, buf, 0, &zero))
	    || (TRUE == sock_write(sock, buf)))
	{
	  done_reading = TRUE;
	  if (TRUE == done_writing)
	  {
	    break;
	  }
	}
      }
      else if (sock_get_fd(sock) == fd)
      {
	if ((0 >= sock_read(sock, buf, 0, &zero))
	    || (TRUE == sock_write(sock_stdout, buf)))
	{
	  done_writing = TRUE;
	  if (TRUE == done_reading)
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
  sockb_shutdown();
  libstash_shutdown();
  return retval;
}

cw_bool_t
oom_handler(const void * a_data, cw_uint32_t a_size)
{
  if (dbg_is_registered(cw_g_dbg, "prog_error"))
  {
    out_put(cw_g_out, "Memory allocation error for size [i]; exit()\n", a_size);
  }
  exit(1);
  
  return FALSE;
}

void
usage(const char * a_progname)
{
  out_put
    (cw_g_out,
     "[s] usage:\n"
     "    [s] -h\n"
     "    [s] -V\n"
     "    [s] [[-v | -q] [[-t <timeout>] -r [[<rhost>:]<rport>\n"
     "\n"
     "    Option               | Description\n"
     "    ---------------------+-----------------------------------------------------\n"
     "    -h                   | Print usage and exit.\n"
     "    -V                   | Print version information and exit.\n"
     "    -v                   | Verbose.\n"
     "    -q                   | Quiet.\n"
     "    -t <timeout>         | Quit after <timeout> seconds, if stdin is closed.\n"
     "    -r [[<rhost>:]<rport> | Connect to host <rhost> or \"localhost\",\n"
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
