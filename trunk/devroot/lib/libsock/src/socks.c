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
#include <limits.h>

#include "libsock/sockb_l.h"

cw_socks_t *
socks_new(void)
{
  cw_socks_t * retval;

  retval = (cw_socks_t *) _cw_malloc(sizeof(cw_socks_t));
  if (NULL == retval)
  {
    goto RETURN;
  }

  bzero(retval, sizeof(cw_socks_t));

  RETURN:
  return retval;
}

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
      if (dbg_is_registered(cw_g_dbg, "socks_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error in close(): [s]\n", strerror(error));
      }
    }
  }

  _cw_free(a_socks);
}

cw_bool_t
socks_listen(cw_socks_t * a_socks, int * r_port)
{
  cw_bool_t retval;
  int val;
  struct sockaddr_in server_addr;

  _cw_check_ptr(a_socks);
  _cw_check_ptr(r_port);
  _cw_assert(a_socks->is_listening == FALSE);

  /* Open a TCP socket stream. */
  a_socks->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (a_socks->sockfd < 0)
  {
    if (dbg_is_registered(cw_g_dbg, "socks_error"))
    {
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		"Error in socket(): [s]\n", strerror(errno));
    }
    retval = TRUE;
    goto RETURN;
  }

  /* Re-use the socket. */
  val = 1;
  if (0 > setsockopt(a_socks->sockfd, SOL_SOCKET, SO_REUSEADDR,
		     (void *) &val, sizeof(val)))
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error for SO_REUSEADDR in setsockopt(): [s]\n",
	      strerror(errno));
    retval = TRUE;
    goto RETURN;
  }

  /* Bind the socket's local address. */
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(*r_port);
  
  if (bind(a_socks->sockfd,
	   (struct sockaddr *) &server_addr,
	   sizeof(server_addr)))
  {
    if (dbg_is_registered(cw_g_dbg, "socks_error"))
    {
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		"Error in bind(): [s]\n", strerror(errno));
    }
    retval = TRUE;
    goto RETURN;
  }

  /* Find out what port is being used, if not already known. */
  if (*r_port == 0)
  {
    int server_addr_size = sizeof(server_addr);
    
    /* What port number did the OS choose? */
    if (getsockname(a_socks->sockfd, (struct sockaddr *) &server_addr,
		    &server_addr_size))
    {
      if (dbg_is_registered(cw_g_dbg, "socks_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error in getsockname(): [s]\n", strerror(errno));
      }
      retval = TRUE;
      goto RETURN;
    }
    *r_port = ntohs(server_addr.sin_port);
  }
  
  /* Now listen. */
  if (-1 == listen(a_socks->sockfd, 511))
  {
    if (dbg_is_registered(cw_g_dbg, "socks_error"))
    {
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
                "Error in listen(): [s]\n", strerror(errno));
    }
    retval = TRUE;
    goto RETURN;
  }
  
  a_socks->is_listening = TRUE;
  retval = FALSE;

  RETURN:
  return retval;
}

cw_sock_t *
socks_accept(cw_socks_t * a_socks, struct timeval * a_timeout,
	     cw_sock_t * r_sock)
{
  cw_sock_t * retval;
  fd_set fd_read_set;

  _cw_check_ptr(a_socks);
  _cw_check_ptr(r_sock);
  
  /* Are we even listening right now? */
  if (a_socks->is_listening == FALSE)
  {
    retval = NULL;
    goto RETURN;
  }

  /* Clear the file descriptor set and add our descriptor. */
  FD_ZERO(&fd_read_set);
  FD_SET(a_socks->sockfd, &fd_read_set);

  /* See if someone wants to connect. */
  if (select(a_socks->sockfd + 1,
	     &fd_read_set, NULL, NULL,
	     a_timeout))
  {
    int sockfd;
    struct sockaddr_in client_addr;
    int sockaddr_struct_size = sizeof(struct sockaddr);
    cw_bool_t wrap_error;
    
    /* There is someone who wants to connect. */
    sockfd = accept(a_socks->sockfd,
		    (struct sockaddr *) &client_addr,
		    &sockaddr_struct_size);
    if (sockfd < 0)
    {
      if (dbg_is_registered(cw_g_dbg, "socks_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error in accept(): [s]\n", strerror(errno));
      }
      retval = NULL;
      goto RETURN;
    }

    /* Wrap the socket descriptor inside a sock. */
    retval = r_sock;

    wrap_error = sock_wrap(retval, sockfd);
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
