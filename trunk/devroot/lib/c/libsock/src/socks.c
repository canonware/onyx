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
 * Implementation of the socks (socket server) class.  socks accepts connections
 * on a port and returns a sock instance for each connection.
 *
 ****************************************************************************/

#define _LIBSOCK_USE_SOCKS
#include "libsock/libsock.h"
#include "libsock/socks_p.h"

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "libsock/sockb_l.h"

/****************************************************************************
 *
 * socks constructor.
 *
 ****************************************************************************/
cw_socks_t *
socks_new()
{
  cw_socks_t * retval;

  retval = (cw_socks_t *) _cw_malloc(sizeof(cw_socks_t));

  bzero(retval, sizeof(cw_socks_t));
  
  return retval;
}

/****************************************************************************
 *
 * socks destructor.
 *
 ****************************************************************************/
void
socks_delete(cw_socks_t * a_socks)
{
  _cw_check_ptr(a_socks);
  
  if (a_socks->is_listening)
  {
    int error;
    
    /* Shut the port down. */
    error = close(a_socks->sockfd);
    if (error)
    {
      log_eprintf(cw_g_log, NULL, 0, "socks_delete",
		   "Error in close(): %s\n", strerror(error));
    }
  }

  _cw_free(a_socks);
}

/****************************************************************************
 *
 * Do setup and start accepting connections on *a_port.  If *a_port is 0, let
 * the OS choose what port number to use, and assign the number to *a_port
 * before returning.
 *
 ****************************************************************************/
cw_bool_t
socks_listen(cw_socks_t * a_socks, int * a_port)
{
  cw_bool_t retval;
  struct sockaddr_in server_addr;

  _cw_check_ptr(a_socks);
  _cw_check_ptr(a_port);
  _cw_assert(a_socks->is_listening == FALSE);

  /* Open a TCP socket stream. */
  a_socks->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (a_socks->sockfd < 0)
  {
    log_eprintf(cw_g_log, NULL, 0, "socks_listen",
		 "Error in socket(): %s\n", strerror(errno));
    retval = TRUE;
    goto RETURN;
  }

  /* Bind the socket's local address. */
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(*a_port);
  
  if (bind(a_socks->sockfd,
	   (struct sockaddr *) &server_addr,
	   sizeof(server_addr)))
  {
    log_eprintf(cw_g_log, NULL, 0, "socks_listen",
		 "Error in bind(): %s\n", strerror(errno));
    retval = TRUE;
    goto RETURN;
  }

  /* Find out what port is being used, if not already known. */
  if (*a_port == 0)
  {
    int server_addr_size = sizeof(server_addr);
    
    /* What port number did the OS choose? */
    if (getsockname(a_socks->sockfd, (struct sockaddr *) &server_addr,
		    &server_addr_size))
    {
      log_eprintf(cw_g_log, NULL, 0, "socks_listen",
		  "Error in getsockname(): %s\n", strerror(errno));
      retval = TRUE;
      goto RETURN;
    }
    *a_port = ntohs(server_addr.sin_port);
  }
  
  /* Now listen.  -1 causes the OS to use the maximum backlog. */
  listen(a_socks->sockfd, -1);
  a_socks->is_listening = TRUE;
  retval = FALSE;

  RETURN:
  return retval;
}

/****************************************************************************
 *
 * Accept a connection.  Don't return until someone connects.  If a_sock is
 * non-NULL, use it for wrapping the socket descriptor.
 *
 ****************************************************************************/
cw_sock_t *
socks_accept_block(cw_socks_t * a_socks, cw_sock_t * a_sock)
{
  cw_sock_t * retval;
  cw_bool_t wrap_error;
  int sockaddr_struct_size = sizeof(struct sockaddr);
  int new_sockfd, spare_fd;
  struct sockaddr_in client_addr;

  _cw_check_ptr(a_socks);

  /* Are we even listening right now? */
  if (a_socks->is_listening == FALSE)
  {
    retval = NULL;
    goto RETURN;
  }

  /* Grab a spare file descriptor to make sure that sockb can actually handle
   * this socket, even if the return value of socket() is >= FD_SETSIZE. */
  spare_fd = sockb_l_get_spare_fd();
  if (spare_fd < 0)
  {
    if (dbg_is_registered(cw_g_dbg, "socks_verbose"))
    {
      log_eprintf(cw_g_log, NULL, 0, "socks_accept_block",
		  "Exceded maximum number of simultaneous connections (%d)\n",
		  FD_SETSIZE);
    }
    retval = NULL;
    goto RETURN;
  }

  /* Wait for a connection. */
  new_sockfd = accept(a_socks->sockfd,
		      (struct sockaddr *) &client_addr, &sockaddr_struct_size);
  if (new_sockfd < 0)
  {
    log_eprintf(cw_g_log, NULL, 0, "socks_accept_block",
		 "Error in accept(): %s\n", strerror(errno));
    retval = NULL;
    goto RETURN;
  }

  if (new_sockfd >= FD_SETSIZE)
  {
    /* This is outside the useable range for sockb, so transfer the socket over
     * to spare_fd. */
    if (dup2(new_sockfd, spare_fd))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, "socks_accept_block",
		  "Fatal error in dup2(): %s\n", strerror(errno));
      abort();
    }
    if (close(new_sockfd))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, "socks_accept_block",
		  "Error in close(): %s\n", strerror(errno));
    }
  }
  else
  {
    /* Hmm, we didn't need spare_fd after all.  Release it. */
    if (close(spare_fd))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, "socks_accept_block",
		  "Error in close(): %s\n", strerror(errno));
    }
  }

  /* Wrap the socket descriptor inside a sock. */
  retval = a_sock;

  wrap_error = sock_wrap(retval, new_sockfd);
  if (wrap_error == TRUE)
  {
    retval = NULL;
  }
  
  RETURN:
  return retval;
}

/****************************************************************************
 *
 * Accept a connection.  If a_sock is non-NULL, use it for wrapping the socket
 * descriptor.  If no one is trying to connect though, return immediately.
 *
 ****************************************************************************/
cw_sock_t *
socks_accept_noblock(cw_socks_t * a_socks, cw_sock_t * a_sock)
{
  cw_sock_t * retval;
  fd_set fd_read_set;
  struct timeval timeout;

  _cw_check_ptr(a_socks);

  /* Are we even listening right now? */
  if (a_socks->is_listening == FALSE)
  {
    retval = NULL;
    goto RETURN;
  }

  /* Set timeout to the shortest possible time. */
  timeout.tv_sec = 0;
  timeout.tv_usec = 1;

  /* Clear the file descriptor set and add our descriptor. */
  FD_ZERO(&fd_read_set);
  FD_SET(a_socks->sockfd, &fd_read_set);

  /* See if someone wants to connect. */
  if (select(a_socks->sockfd + 1,
	     &fd_read_set, NULL, NULL,
	     &timeout))
  {
    int new_sockfd, spare_fd;
    struct sockaddr_in client_addr;
    int sockaddr_struct_size = sizeof(struct sockaddr);
    cw_bool_t wrap_error;
    
    /* Grab a spare file descriptor to make sure that sockb can actually handle
     * this socket, even if the return value of socket() is >= FD_SETSIZE. */
    spare_fd = sockb_l_get_spare_fd();
    if (spare_fd < 0)
    {
      if (dbg_is_registered(cw_g_dbg, "socks_verbose"))
      {
	log_eprintf(cw_g_log, NULL, 0, "socks_accept_block",
		    "Exceded maximum number of simultaneous connections (%d)\n",
		    FD_SETSIZE);
      }
      retval = NULL;
      goto RETURN;
    }

    /* There is someone who wants to connect. */
    new_sockfd = accept(a_socks->sockfd,
			(struct sockaddr *) &client_addr,
			&sockaddr_struct_size);
    if (new_sockfd < 0)
    {
      log_eprintf(cw_g_log, NULL, 0, "socks_accept_noblock",
		   "Error in accept(): %s\n", strerror(errno));
      retval = NULL;
      goto RETURN;
    }

    if (new_sockfd >= FD_SETSIZE)
    {
      /* This is outside the useable range for sockb, so transfer the socket
       * over to spare_fd. */
      if (dup2(new_sockfd, spare_fd))
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, "socks_accept_noblock",
		    "Fatal error in dup2(): %s\n", strerror(errno));
	abort();
      }
      if (close(new_sockfd))
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, "socks_accept_noblock",
		    "Error in close(): %s\n", strerror(errno));
      }
    }
    else
    {
      /* Hmm, we didn't need spare_fd after all.  Release it. */
      if (close(spare_fd))
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, "socks_accept_noblock",
		    "Error in close(): %s\n", strerror(errno));
      }
    }
  
    /* Wrap the socket descriptor inside a sock. */
    retval = a_sock;

    wrap_error = sock_wrap(retval, new_sockfd);
    if (wrap_error == TRUE)
    {
      retval = NULL;
    }
  }
  else
  {
    /* No one wants to connect. */
    retval = NULL;
  }

  RETURN:
  return retval;
}
