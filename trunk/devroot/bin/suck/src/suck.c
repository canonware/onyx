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
void
usage(const char * a_progname);
void
version(const char * a_progname);
const char *
basename(const char * a_str);

int
main(int argc, char ** argv)
{
  int retval = 0, bytes_read;
  cw_socks_t * socks;
  cw_sock_t * sock;
  struct timeval timeout;
  struct timespec tout;
  cw_buf_t buf;
  cw_ring_t * sock_ring = NULL, * t_ring;
  int fd_vec[FD_SETSIZE];
  cw_uint32_t nfds = 0;
  
  /* Command line parsing variables. */
  int c;
  cw_bool_t cl_error = FALSE, opt_help = FALSE, opt_version = FALSE;
  cw_uint32_t opt_bsize = _LIBSOCK_BLOW_DEFAULT_BSIZE;
  int opt_port = 0;

  libstash_init();
  dbg_register(cw_g_dbg, "mem_error");
  dbg_register(cw_g_dbg, "prog_error");
  dbg_register(cw_g_dbg, "sockb_error");
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
	log_printf(cw_g_log, "Unrecognized option '%c'\n", c);
	cl_error = TRUE;
	break;
      }
    }
  }

  if ((TRUE == cl_error) || (optind < argc))
  {
    log_printf(cw_g_log, "Unrecognized option(s)\n");
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
    log_printf(cw_g_log, "Invalid block size\n");
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
  log_printf(cw_g_log, "%s: Listening on port %d\n",
	     basename(argv[0]), opt_port);

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  tout.tv_sec = 0;
  tout.tv_nsec = 0;

  /* Loop, accepting connections, reading from the open sock's, and closing
   * sock's on error. */
  buf_new(&buf, FALSE);
  sock = sock_new(NULL, opt_bsize * 8);
  if (NULL == sock)
  {
    _cw_error("Memory allocation error");
  }

  timeout.tv_usec = 0;

  while (1)
  {
    if (NULL != sock_ring)
    {
      sockb_wait(fd_vec, nfds, NULL);
      t_ring = sock_ring;
      do
      {
	tout.tv_sec = 0;
	tout.tv_nsec = 0;
	bytes_read = sock_read((cw_sock_t *) ring_get_data(t_ring),
			       &buf, 0, &tout);
	if (0 < bytes_read)
	{
	  /* Throw the data away. */
	  buf_release_head_data(&buf, bytes_read);
	  
	  t_ring = ring_next(t_ring);
	}
	else if (-1 == bytes_read)
	{
	  cw_ring_t * old_ring;
	  int sockfd;
	  cw_sock_t * t_sock;
	  
	  log_printf(cw_g_log, "Connection closed\n");
	  
	  /* Socket error.  Remove this sock from the ring. */
	  t_sock = (cw_sock_t *) ring_get_data(t_ring);
	  sockfd = sock_get_fd(t_sock);
	  sock_delete(t_sock);

	  /* This advances us to the next ring item, so there is no need to
	   * call ring_next(). */
	  old_ring = t_ring;
	  t_ring = ring_cut(old_ring);
	  ring_delete(old_ring);
	  if (t_ring == old_ring)
	  {
	    t_ring = NULL;
	    sock_ring = NULL;
	  }
	  else if (nfds > 1)
	  {
	    cw_uint32_t j;
	    
	    /* Search through fd_vec and find the corresponding entry for this
	     * sock. */
	    for (j = 0; j < nfds; j++)
	    {
	      if (fd_vec[j] == sockfd)
	      {
		fd_vec[j] = fd_vec[nfds - 1];
		break;
	      }
	    }
	    _cw_assert(j < nfds);
	  }
	  nfds--;
	  timeout.tv_sec = 5;
	}
      } while (t_ring != sock_ring);
    }
  
    if (sock == socks_accept(socks, &timeout, sock))
    {
      log_printf(cw_g_log, "New connection\n");
      /* New connection.  Add it to the sock ring. */
      t_ring = ring_new(NULL, NULL, NULL);
      if (NULL == t_ring)
      {
	_cw_error("Memory allocation error");
      }
      ring_set_data(t_ring, sock);
      
      if (NULL != sock_ring)
      {
	ring_meld(sock_ring, t_ring);
      }
      else
      {
	sock_ring = t_ring;
      }

      fd_vec[nfds] = sock_get_fd(sock);
      nfds++;
      timeout.tv_sec = 0;

      /* Create another sock object for the next time we call
	 * socks_accept(). */
      sock = sock_new(NULL, opt_bsize * 8);
      if (NULL == sock)
      {
	_cw_error("Memory allocation error");
      }
    }
  }
  buf_delete(&buf);

  sockb_shutdown();
  CLERROR:
  libstash_shutdown();
  return retval;
}

void
usage(const char * a_progname)
{
  log_printf
    (cw_g_log,
     "%s usage:\n"
     "    %s -h\n"
     "    %s -V\n"
     "    %s [-b <bsize>] [-p <port>]\n"
     "\n"
     "    Option               | Description\n"
     "    ---------------------+------------------------------------------\n"
     "    -h                   | Print usage and exit.\n"
     "    -V                   | Print version information and exit.\n"
     "    -b <bsize>           | Send blocks of size <bsize>.\n"
     "                         | (Defaults to %lu.)\n"
     "    -p <port>            | Port to listen on.\n",
     a_progname, a_progname, a_progname, a_progname,
     _LIBSOCK_BLOW_DEFAULT_BSIZE
     );
}

void
version(const char * a_progname)
{
  log_printf(cw_g_log,
	     "%s, version %s\n",
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
