/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Accept connections and throw away incoming data as fast as possible.
 *
 ****************************************************************************/

#define _LIBSOCK_USE_SOCKS
#include "libsock/libsock.h"

/* Command line defaults. */
#define _LIBSOCK_BLOW_DEFAULT_BSIZE 4096

/* Function Prototypes. */
void *
accept_entry_func(void * a_arg);
void
usage(const char * a_progname);
void
version(const char * a_progname);
const char *
basename(const char * a_str);

/* Global. */
cw_sock_t * sock_vec[FD_SETSIZE];
cw_mq_t * mq;
cw_socks_t * socks;
cw_uint32_t opt_bsize = _LIBSOCK_BLOW_DEFAULT_BSIZE;

int
main(int argc, char ** argv)
{
  int retval = 0, bytes_read, sockfd;
  struct timespec tout;
  cw_buf_t buf;
  cw_bool_t did_work;
  cw_thd_t accept_thd;

  /* Command line parsing variables. */
  int c;
  cw_bool_t cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
  int opt_port = 0;

  bzero(sock_vec, FD_SETSIZE * sizeof(cw_sock_t *));
  
  libstash_init();
  dbg_register(cw_g_dbg, "mem_error");
  dbg_register(cw_g_dbg, "prog_error");
  dbg_register(cw_g_dbg, "sockb_error");
/*    dbg_register(cw_g_dbg, "sockb_verbose"); */
  dbg_register(cw_g_dbg, "socks_error");
  dbg_register(cw_g_dbg, "sock_error");

  /* Parse command line. */
  while (-1 != (c = getopt(argc, argv, "hVb:p:")))
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
      case 'b':
      {
	opt_bsize = strtoul(optarg, NULL, 10);
	break;
      }
      case 'p':
      {
	opt_port = strtol(optarg, NULL, 10);
	break;
      }
      default:
      {
	out_put(cw_g_out, "Unrecognized option '[c]'\n", c);
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

  if (0 == opt_bsize)
  {
    out_put(cw_g_out, "Invalid block size\n");
    retval = 1;
    goto CLERROR;
  }
    
  if (sockb_init(opt_bsize, 8))
  {
    _cw_error("Initialization failure in sockb_init()");
  }

  socks = socks_new();
  if (NULL == socks)
  {
    _cw_error("Memory allocation error");
  }
  
  if (TRUE == socks_listen(socks, &opt_port))
  {
    exit(1);
  }
  out_put(cw_g_out, "[s]: Listening on port [i32]\n",
	  basename(argv[0]), opt_port);

  tout.tv_sec = 0;
  tout.tv_nsec = 0;

  /* Loop, accepting connections, reading from the open sock's, and closing
   * sock's on error. */
  buf_new(&buf, FALSE);

  mq = mq_new(NULL);
  if (NULL == mq)
  {
    _cw_error("Memory allocation error");
  }

  /* Start thread to accept connections. */
  thd_new(&accept_thd, accept_entry_func, NULL);

  while (1)
  {
    did_work = FALSE;
    
    while (NULL != (void *) (sockfd = (int) mq_get(mq)))
    {
      did_work = TRUE;

      if (NULL != sock_vec[sockfd])
      {
	bytes_read = sock_read(sock_vec[sockfd], &buf, 0, &tout);
	if (0 < bytes_read)
	{
	  /* Throw the data away. */
	  buf_release_head_data(&buf, bytes_read);
	}
	else if (-1 == bytes_read)
	{
	  while (TRUE == sockb_in_notify(NULL, sockfd))
	  {
	    thd_yield();
	  }

	  sock_delete(sock_vec[sockfd]);
	  sock_vec[sockfd] = NULL;

	  out_put_l(cw_g_out, "Connection closed\n");
	}
      }
    }

  }
  mq_delete(mq);
  buf_delete(&buf);

  sockb_shutdown();
  CLERROR:
  libstash_shutdown();
  return retval;
}

void *
accept_entry_func(void * a_arg)
{
  cw_sock_t * sock;

  sock = sock_new(NULL, opt_bsize * 8);
  if (NULL == sock)
  {
    _cw_error("Memory allocation error");
  }
  
  while (1)
  {
    if (sock == socks_accept(socks, NULL, sock))
    {
      out_put_l(cw_g_out, "New connection\n");
      
      sock_vec[sock_get_fd(sock)] = sock;

      while (TRUE == sockb_in_notify(mq, sock_get_fd(sock)))
      {
	thd_yield();
      }

      /* Create another sock object for the next time we call
       * socks_accept(). */
      sock = sock_new(NULL, opt_bsize * 8);
      if (NULL == sock)
      {
	_cw_error("Memory allocation error");
      }
    }
  }
  
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
     "    [s] [[-b <bsize>] [[-p <port>]\n"
     "\n"
     "    Option               | Description\n"
     "    ---------------------+------------------------------------------\n"
     "    -h                   | Print usage and exit.\n"
     "    -V                   | Print version information and exit.\n"
     "    -b <bsize>           | Send blocks of size <bsize>.\n"
     "                         | (Defaults to [i32].)\n"
     "    -p <port>            | Port to listen on.\n",
     a_progname, a_progname, a_progname, a_progname,
     _LIBSOCK_BLOW_DEFAULT_BSIZE
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
