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
 ****************************************************************************/

#define _LIBSOCK_USE_SOCK
#include "libsock/libsock.h"

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "libsock/sock_p.h"
#include "libsock/sock_l.h"
#include "libsock/sockb_l.h"

cw_sock_t *
sock_new(cw_sock_t * a_sock, cw_uint32_t a_in_max_buf_size)
{
  cw_sock_t * retval;
  
  if (a_sock == NULL)
  {
    retval = (cw_sock_t *) _cw_malloc(sizeof(cw_sock_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_sock;
    retval->is_malloced = FALSE;
  }

  mtx_new(&retval->lock);
  cnd_new(&retval->callback_cnd);

  mtx_new(&retval->state_lock);
  retval->sockfd = -1;
  retval->is_connected = FALSE;
  retval->error = TRUE;

  retval->in_max_buf_size = a_in_max_buf_size;
  mtx_new(&retval->in_lock);
  buf_new(&retval->in_buf, FALSE);
  retval->in_need_signal_count = 0;
  cnd_new(&retval->in_cnd);

  mtx_new(&retval->out_lock);
  buf_new(&retval->out_buf, FALSE);
  retval->out_need_broadcast_count = 0;
  retval->out_is_flushed = TRUE;
  cnd_new(&retval->out_cnd);
  
  return retval;
}

void
sock_delete(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);

  if (TRUE == a_sock->is_connected)
  {
    sock_p_disconnect(a_sock);
  }
  
  mtx_delete(&a_sock->lock);
  cnd_delete(&a_sock->callback_cnd);

  mtx_delete(&a_sock->state_lock);

  mtx_delete(&a_sock->in_lock);
  buf_delete(&a_sock->in_buf);
  cnd_delete(&a_sock->in_cnd);
  
  mtx_delete(&a_sock->out_lock);
  buf_delete(&a_sock->out_buf);
  cnd_delete(&a_sock->out_cnd);
  
  if (a_sock->is_malloced)
  {
    _cw_free(a_sock);
  }
}

cw_bool_t
sock_is_connected(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);

  /* Yeah, it's strange naming that results in us not returning
   * a_sock->is_connected.  The problem is that a_sock->error is the first
   * indication that we're disconnected, whereas a_sock->is_connected means that
   * we realize that fact internally. */
  return a_sock->error;
}

cw_uint32_t
sock_get_port(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);

  return a_sock->port;
}

cw_bool_t
sock_connect(cw_sock_t * a_sock, char * a_server_host, int a_port)
{
  cw_bool_t retval;
  cw_uint32_t server_ip;
  struct sockaddr_in server_addr;
  
  _cw_check_ptr(a_sock);
  _cw_check_ptr(a_server_host);
  mtx_lock(&a_sock->state_lock);

  if (TRUE == a_sock->is_connected)
  {
    /* We're already connected to someone! */
    retval = TRUE;
    goto RETURN;
  }

  a_sock->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (a_sock->sockfd < 0)
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		 "Error in socket(): %s\n", strerror(errno));
    retval = TRUE;
    goto RETURN;
  }

  if (a_sock->sockfd >= FD_SETSIZE)
  {
    /* This is outside the acceptable range for sockb, so return an error. */
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		 "Exceded maximum number of simultaneous connections (%d)\n",
		 FD_SETSIZE);
    if (close(a_sock->sockfd))
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error in close(): %s\n", strerror(errno));
    }
    a_sock->sockfd = -1;
    retval = TRUE;
    goto RETURN;
  }

  if (sock_p_config_socket(a_sock))
  {
    a_sock->sockfd = -1;
    retval = TRUE;
    goto RETURN;
  }

  /* Figure out the server's IP address. */
  if (sockb_l_get_host_ip(a_server_host, &server_ip))
  {
    if (close(a_sock->sockfd))
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error in close(): %s\n", strerror(errno));
    }
    a_sock->sockfd = -1;
    retval = TRUE;
    goto RETURN;
  }
  
  bzero(&server_addr, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  
  (server_addr.sin_addr).s_addr = server_ip;
  server_addr.sin_port = htons(a_port);

  if (0 > connect(a_sock->sockfd,
		  (struct sockaddr *) &server_addr,
		  sizeof(struct sockaddr_in)))
  {
    /* XXX According to the connect() manpage, EAGAIN is never returned.
     * However, reality begs to differ under FreeBSD 2.2.8-stable.  Take this
     * hack out once we no longer care about that platform. */
    if ((errno == EINPROGRESS) || (errno == EAGAIN))
    {
      fd_set fd_read_set, fd_write_set;

      /* Need to select() on the sockfd until the connect() completes. */
      FD_ZERO(&fd_read_set);
      FD_ZERO(&fd_write_set);
      FD_SET(a_sock->sockfd, &fd_read_set);
      FD_SET(a_sock->sockfd, &fd_write_set);
      if (0 > select(a_sock->sockfd + 1, &fd_read_set, &fd_write_set,
		     NULL, NULL)) /* XXX Need timeout. */
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "Error in select(): %s\n", strerror(errno));
	a_sock->sockfd = -1;
	retval = TRUE;
	goto RETURN;
      }
      else if (FD_ISSET(a_sock->sockfd, &fd_write_set))
      {
	if (FD_ISSET(a_sock->sockfd, &fd_read_set))
	{
	  int error, len;

	  /* Make sure that the socket is both readable and writeable because
	   * data has already arrived. */
	  len = sizeof(error);
	  error = getsockopt(a_sock->sockfd, SOL_SOCKET, SO_ERROR,
			     &error, &len);
	  if (error < 0)
	  {
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Error in getsockopt(): %s\n", strerror(errno));
	    a_sock->sockfd = -1;
	    retval = TRUE;
	    goto RETURN;
	  }
	  else if (error > 0)
	  {
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Error in getsockopt() due to connect(): %s\n",
			strerror(error));
	    a_sock->sockfd = -1;
	    retval = TRUE;
	    goto RETURN;
	  }
	}
      }
      else
      {
	/* Timed out. */
	/* XXX This can't happen, since no timeout is specified. */
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "select() timeout.  Connection failed\n");
	a_sock->sockfd = -1;
	retval = TRUE;
	goto RETURN;
      }
    }
    else
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error in connect(): %s\n", strerror(errno));
      a_sock->sockfd = -1;
      retval = TRUE;
      goto RETURN;
    }
  }

  /* Get the port number for the socket. */
  {
    struct sockaddr_in name;
    int name_size;

    name_size = sizeof(name);
    if (0 > getsockname(a_sock->sockfd, (struct sockaddr *) &name, &name_size))
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error in getsockname(): %s\n", strerror(errno));
      retval = TRUE;
      goto RETURN;
    }
    else
    {
      a_sock->port = (cw_uint32_t) ntohs(name.sin_port);
    }
  }

  mtx_lock(&a_sock->lock);
  sockb_l_register_sock(a_sock);
  cnd_wait(&a_sock->callback_cnd, &a_sock->lock);
  a_sock->is_registered = TRUE;
  mtx_unlock(&a_sock->lock);
    
  retval = FALSE;

  /* This is the only path through this function that does not handle an error,
   * so we can unconditionally toggle a_sock->is_connected here and only
   * here. */
  a_sock->is_connected = TRUE;
  a_sock->error = FALSE;
  
  RETURN:
  mtx_unlock(&a_sock->state_lock);
  return retval;
}

cw_bool_t
sock_wrap(cw_sock_t * a_sock, int a_sockfd)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock);
  mtx_lock(&a_sock->state_lock);
  
  if (TRUE == a_sock->is_connected)
  {
    /* We're already connected to someone! */
    retval = TRUE;
    goto RETURN;
  }

  a_sock->sockfd = a_sockfd;
  if (sock_p_config_socket(a_sock))
  {
    a_sock->sockfd = -1;
    retval = TRUE;
  }
  else
  {
    /* Get the port number for the socket. */
    {
      struct sockaddr_in name;
      int name_size;

      name_size = sizeof(name);
      if (0 > getsockname(a_sock->sockfd,
			  (struct sockaddr *) &name,
			  &name_size))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "Error in getsockname(): %s\n", strerror(errno));
	retval = TRUE;
	goto RETURN;
      }
      else
      {
	a_sock->port = (cw_uint32_t) ntohs(name.sin_port);
      }
    }

    mtx_lock(&a_sock->lock);
    sockb_l_register_sock(a_sock);
    cnd_wait(&a_sock->callback_cnd, &a_sock->lock);
    a_sock->is_registered = TRUE;
    mtx_unlock(&a_sock->lock);
    
    retval = FALSE;
    
    a_sock->is_connected = TRUE;
    a_sock->error = FALSE;
  }

  RETURN:
  mtx_unlock(&a_sock->state_lock);
  return retval;
}

cw_bool_t
sock_disconnect(cw_sock_t * a_sock)
{
  cw_bool_t retval;

  _cw_check_ptr(a_sock);
  mtx_lock(&a_sock->state_lock);

  retval = sock_p_disconnect(a_sock);
  
  mtx_unlock(&a_sock->state_lock);
  return retval;
}

cw_sint32_t
sock_read_noblock(cw_sock_t * a_sock, cw_buf_t * a_spare,
		  cw_sint32_t a_max_read)
{
  cw_sint32_t retval, size;
  
  _cw_check_ptr(a_sock);
  _cw_check_ptr(a_spare);

  if (a_sock->error)
  {
    retval = -1;

    mtx_lock(&a_sock->state_lock);
    if (TRUE == a_sock->is_connected)
    {
      sock_p_disconnect(a_sock);
    }
    mtx_unlock(&a_sock->state_lock);
  }
  else
  {
    mtx_lock(&a_sock->in_lock);
    size = buf_get_size(&a_sock->in_buf);
    if (0 < size)
    {
      retval = size;
      if ((a_max_read == 0) || (buf_get_size(&a_sock->in_buf) < a_max_read))
      {
	buf_catenate_buf(a_spare, &a_sock->in_buf, FALSE);
      }
      else
      {
	buf_split(a_spare, &a_sock->in_buf, a_max_read);
      }

      mtx_unlock(&a_sock->in_lock);
      
      if (size >= a_sock->in_max_buf_size)
      {
	/* XXX Notify sockb to wake up. */
      }
    }
    else
    {
      retval = -1;
      mtx_unlock(&a_sock->in_lock);
    }
  }
  
  return retval;
}

cw_sint32_t
sock_read_block(cw_sock_t * a_sock, cw_buf_t * a_spare,	cw_sint32_t a_max_read)
{
  cw_sint32_t retval, size;
  
  _cw_check_ptr(a_sock);
  _cw_check_ptr(a_spare);
  
  if (a_sock->error)
  {
    retval = -1;

    mtx_lock(&a_sock->state_lock);
    if (TRUE == a_sock->is_connected)
    {
      sock_p_disconnect(a_sock);
    }
    mtx_unlock(&a_sock->state_lock);
  }
  else
  {
    mtx_lock(&a_sock->in_lock);
    if (buf_get_size(&a_sock->in_buf) == 0)
    {
      /* There's no data available right now. */
      a_sock->in_need_signal_count++;
      cnd_wait(&a_sock->in_cnd, &a_sock->in_lock);
      a_sock->in_need_signal_count--;
    }

    size = buf_get_size(&a_sock->in_buf);
    if (0 < size)
    {
      retval = size;
      if ((a_max_read == 0) || (buf_get_size(&a_sock->in_buf) < a_max_read))
      {
	buf_catenate_buf(a_spare, &a_sock->in_buf, FALSE);
      }
      else
      {
	buf_split(a_spare, &a_sock->in_buf, a_max_read);
      }

      mtx_unlock(&a_sock->in_lock);
      
      if (size >= a_sock->in_max_buf_size)
      {
	/* XXX Notify sockb to wake up. */
      }
    }
    else
    {
    /* XXX Should we disconnect here? */
      retval = -1;
      mtx_unlock(&a_sock->in_lock);
    }
  }
  
  return retval;
}

cw_bool_t
sock_write(cw_sock_t * a_sock, cw_buf_t * a_buf)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock);
  _cw_check_ptr(a_buf);

  if (0 < buf_get_size(a_buf))
  {
    if (a_sock->error)
    {
      retval = TRUE;

      mtx_lock(&a_sock->state_lock);
      if (TRUE == a_sock->is_connected)
      {
	sock_p_disconnect(a_sock);
      }
      mtx_unlock(&a_sock->state_lock);
    }
    else
    {
      retval = FALSE;
    
      mtx_lock(&a_sock->out_lock);

      if (a_sock->out_is_flushed == TRUE)
      {
	/* Notify the sockb that we now have data. */
	sockb_l_out_notify(&a_sock->sockfd);
	a_sock->out_is_flushed = FALSE;
      }
    
      buf_catenate_buf(&a_sock->out_buf, a_buf, FALSE);

      mtx_unlock(&a_sock->out_lock);
    }
  }
  else
  {
    retval = TRUE;
  }
  
  return retval;
}

cw_bool_t
sock_flush_out(cw_sock_t * a_sock)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock);

  if (a_sock->error)
  {
    retval = TRUE;

    mtx_lock(&a_sock->state_lock);
    if (TRUE == a_sock->is_connected)
    {
      sock_p_disconnect(a_sock);
    }
    mtx_unlock(&a_sock->state_lock);
  }
  else
  {
    mtx_lock(&a_sock->out_lock);
    if (a_sock->out_is_flushed == FALSE)
    {
      /* There's still data in the pipeline somewhere. */
      a_sock->out_need_broadcast_count++;
      cnd_wait(&a_sock->out_cnd, &a_sock->out_lock);
      a_sock->out_need_broadcast_count--;
    }

    if (0 < buf_get_size(&a_sock->out_buf))
    {
      retval = TRUE;
    }
    else
    {
      retval = FALSE;
    }
    mtx_unlock(&a_sock->out_lock);
    
    if (TRUE == retval)
    {
      mtx_lock(&a_sock->state_lock);
      if (TRUE == a_sock->is_connected)
      {
	sock_p_disconnect(a_sock);
      }
      mtx_unlock(&a_sock->state_lock);
    }
  }
  
  return retval;
}

/****************************************************************************
 *
 * Returns the number of the file descriptor for a_sock's socket, or -1 if not
 * connected.
 *
 * Don't lock, since sockb needs to get at this info without causing deadlock.
 * This is safe, since the socket is never closed except after sockb says it's
 * okay, in which case sockb wouldn't ask for this info anyway.
 *
 ****************************************************************************/
int
sock_l_get_fd(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);

  return a_sock->sockfd;
}

cw_uint32_t
sock_l_get_in_size(cw_sock_t * a_sock)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_sock);

  mtx_lock(&a_sock->in_lock);
  retval = buf_get_size(&a_sock->in_buf);
  if (retval > a_sock->in_max_buf_size)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		"Have %lu in bytes buffered, should have max %lu\n",
		retval, a_sock->in_max_buf_size);
  }
  mtx_unlock(&a_sock->in_lock);

  return retval;
}

cw_uint32_t
sock_l_get_in_max_buf_size(cw_sock_t * a_sock)
{
  return a_sock->in_max_buf_size;
}

/****************************************************************************
 *
 * Get the data that is buffered in out_buf.  If there is no data buffered, then
 * signal flush_cond.  Note that this assumes sockb will write all data that it
 * has before asking for more.  If for some reason this needs to change, then a
 * more sophisticated message passing scheme will be necessary for flushing the
 * output buffer.
 *
 ****************************************************************************/
void
sock_l_get_out_data(cw_sock_t * a_sock, cw_buf_t * a_buf)
{
  _cw_check_ptr(a_sock);
  _cw_check_ptr(a_buf);

  mtx_lock(&a_sock->out_lock);

  if (buf_get_size(&a_sock->out_buf) > a_sock->os_outbuf_size)
  {
    buf_split(a_buf, &a_sock->out_buf, a_sock->os_outbuf_size);
  }
  else
  {
    buf_catenate_buf(a_buf, &a_sock->out_buf, FALSE);
  }
  
  if (buf_get_size(a_buf) == 0)
  {
    /* No data was available. */
    a_sock->out_is_flushed = TRUE;

    if (a_sock->out_need_broadcast_count > 0)
    {
      /* One or more threads want a signal. */
      cnd_broadcast(&a_sock->out_cnd);
    }
  }
  mtx_unlock(&a_sock->out_lock);
}

/****************************************************************************
 *
 * Push data back into out_buf.
 *
 ****************************************************************************/
cw_uint32_t
sock_l_put_back_out_data(cw_sock_t * a_sock, cw_buf_t * a_buf)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_sock);
  _cw_check_ptr(a_buf);

  mtx_lock(&a_sock->out_lock);
/*    buf_catenate_buf(a_buf, &a_sock->out_buf, FALSE); */
  buf_catenate_buf(&a_sock->out_buf, a_buf, FALSE);
  _cw_assert(buf_get_size(a_buf) == 0);
  
  retval = buf_get_size(&a_sock->out_buf);
  if (retval == 0)
  {
    a_sock->out_is_flushed = TRUE;
    if (a_sock->out_need_broadcast_count > 0)
    {
      cnd_broadcast(&a_sock->out_cnd);
    }
  }
  mtx_unlock(&a_sock->out_lock);

  return retval;
}

/****************************************************************************
 *
 * Append data to in_buf.
 *
 ****************************************************************************/
void
sock_l_put_in_data(cw_sock_t * a_sock, cw_buf_t * a_buf)
{
  _cw_check_ptr(a_sock);
  _cw_check_ptr(a_buf);

  mtx_lock(&a_sock->in_lock);

  buf_catenate_buf(&a_sock->in_buf, a_buf, FALSE);

  if (a_sock->in_need_signal_count > 0)
  {
    /* Someone wants to know that there are data available. */
    cnd_signal(&a_sock->in_cnd);
  }
  
  mtx_unlock(&a_sock->in_lock);
}

/****************************************************************************
 *
 * sockb calls this function to notify the sock of the result of a message.
 *
 ****************************************************************************/
void
sock_l_message_callback(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);

  mtx_lock(&a_sock->lock);
  cnd_signal(&a_sock->callback_cnd);
  mtx_unlock(&a_sock->lock);
}

void
sock_l_error_callback(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);
  
  mtx_lock(&a_sock->state_lock);
  a_sock->error = TRUE;
  mtx_unlock(&a_sock->state_lock);
}

/****************************************************************************
 *
 * Set socket options correctly.
 *
 ****************************************************************************/
static cw_bool_t
sock_p_config_socket(cw_sock_t * a_sock)
{
  cw_bool_t retval;
  int val, len;
  struct linger linger_struct;
  
  /* Print out all kinds of socket info. */
  if (dbg_is_registered(cw_g_dbg, "sock_sockopt"))
  {
    /* Define a macro to do the drudgery of getting an option and printing it,
     * since we're doing this quite a few times. */
#define _CW_SOCK_GETSOCKOPT(a) \
    len = sizeof(val); \
    if (getsockopt(a_sock->sockfd, SOL_SOCKET, (a), \
		   (void *) &val, &len)) \
    { \
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__, \
		  "Error for %s in getsockopt(): %s\n", #a, strerror(errno)); \
      retval = TRUE; \
      goto RETURN; \
    } \
    else \
    { \
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__, \
		  "%s: %d\n", #a, val); \
    }

    _CW_SOCK_GETSOCKOPT(SO_REUSEADDR);
#ifndef _CW_OS_SOLARIS
    _CW_SOCK_GETSOCKOPT(SO_REUSEPORT);
#endif
    _CW_SOCK_GETSOCKOPT(SO_KEEPALIVE);
    /* SO_LINGER uses a different data structure, so handle this one
     * separately. */
    len = sizeof(linger_struct);
    if (getsockopt(a_sock->sockfd, SOL_SOCKET, SO_LINGER,
		   (void *) &linger_struct, &len))
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error for SO_LINGER in getsockopt(): %s\n",
		  strerror(errno));
      retval = TRUE;
      goto RETURN;
    }
    else
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "SO_LINGER: %s, %d second%s\n",
		  linger_struct.l_onoff ? "on" : "off",
		  linger_struct.l_linger,
		  linger_struct.l_linger != 1 ? "s" : "");
    }
    _CW_SOCK_GETSOCKOPT(SO_OOBINLINE);
    _CW_SOCK_GETSOCKOPT(SO_SNDBUF);
    _CW_SOCK_GETSOCKOPT(SO_RCVBUF);
#ifndef _CW_OS_SOLARIS
    _CW_SOCK_GETSOCKOPT(SO_SNDLOWAT);
    _CW_SOCK_GETSOCKOPT(SO_RCVLOWAT);
    _CW_SOCK_GETSOCKOPT(SO_SNDTIMEO);
    _CW_SOCK_GETSOCKOPT(SO_RCVTIMEO);
#endif

#undef _CW_SOCK_GETSOCKOPT
  }

  /* Get the size of the OS's outgoing buffer, so that we can hand no more than
   * that amount to sockb each time it asks for data. */
  {
    int len;
    
    len = sizeof(a_sock->os_outbuf_size);
    if (getsockopt(a_sock->sockfd, SOL_SOCKET, SO_SNDBUF,
		   (void *) &a_sock->os_outbuf_size, &len))
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error for SO_SNDBUF in getsockopt(): %s\n", strerror(errno));
      retval = TRUE;
      goto RETURN;
    }
  }

  /* Set the socket to non-blocking, so that we don't have to worry about
   * sockb's select loop locking up. */
  val = fcntl(a_sock->sockfd, F_GETFL, 0);
  if (val == -1)
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"Error for F_GETFL in fcntl(): %s\n", strerror(errno));
    retval = TRUE;
    goto RETURN;
  }
  if (fcntl(a_sock->sockfd, F_SETFL, val | O_NONBLOCK))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"Error for F_SETFL in fcntl(): %s\n", strerror(errno));
    retval = TRUE;
    goto RETURN;
  }

  /* Tell the socket to wait and try to flush buffered data before closing at
   * the end. */
  /* XXX 10 seconds is a bit arbitrary, eh? */
  linger_struct.l_onoff = 1;
  linger_struct.l_linger = 10;
  if (setsockopt(a_sock->sockfd, SOL_SOCKET, SO_LINGER,
		 (void *) &linger_struct, sizeof(linger_struct)))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"Error for SO_LINGER in setsockopt(): %s\n", strerror(errno));
    retval = TRUE;
    goto RETURN;
  }
  else if (dbg_is_registered(cw_g_dbg, "sock_sockopt"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"SO_LINGER: %s, %d second%s\n",
		linger_struct.l_onoff ? "on" : "off",
		linger_struct.l_linger,
		linger_struct.l_linger != 1 ? "s" : "");
  }

#ifndef _CW_OS_SOLARIS
  /* Set the socket to not buffer outgoing data without trying to send it. */
  val = 1;
  if (setsockopt(a_sock->sockfd, SOL_SOCKET, SO_SNDLOWAT,
		 (void *) &val, sizeof(val)))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"Error for SO_SNDLOWAT in setsockopt(): %s\n", strerror(errno));
    retval = TRUE;
    goto RETURN;
  }
  else if (dbg_is_registered(cw_g_dbg, "sock_sockopt"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"SO_SNDLOWAT: %d\n", val);
  }
#endif

  /* Re-use the socket. */
  val = 1;
  if (0 > setsockopt(a_sock->sockfd, SOL_SOCKET, SO_REUSEADDR,
		     (void *) &val, sizeof(val)))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"Error for SO_REUSEADDR in setsockopt(): %s\n",
		strerror(errno));
    retval = TRUE;
    goto RETURN;
  }
  
  retval = FALSE;
  
  RETURN:
  return retval;
}

static cw_bool_t
sock_p_disconnect(cw_sock_t * a_sock)
{
  cw_bool_t retval;
  cw_bool_t did_broadcast, done;
  int val;

  if (TRUE == a_sock->is_connected)
  {
    /* Unregister the sock with sockb. */
    mtx_lock(&a_sock->lock);
    sockb_l_unregister_sock(&a_sock->sockfd);
    cnd_wait(&a_sock->callback_cnd, &a_sock->lock);
    a_sock->is_registered = FALSE;
    mtx_unlock(&a_sock->lock);
    
    /* Make sure there are no threads blocked inside a_sock before cleaning
     * up. */
    for (did_broadcast = done = FALSE; done == FALSE;)
    {
      mtx_lock(&a_sock->in_lock);
      if ((did_broadcast == FALSE) && (a_sock->in_need_signal_count > 0))
      {
	cnd_broadcast(&a_sock->in_cnd);
      }
      else if (a_sock->in_need_signal_count == 0)
      {
	done = TRUE;
      }
      mtx_unlock(&a_sock->in_lock);
    }
    for (did_broadcast = done = FALSE; done == FALSE;)
    {
      mtx_lock(&a_sock->out_lock);
      if ((did_broadcast == FALSE) && (a_sock->out_need_broadcast_count > 0))
      {
	cnd_broadcast(&a_sock->out_cnd);
      }
      else if (a_sock->out_need_broadcast_count == 0)
      {
	done = TRUE;
      }
      mtx_unlock(&a_sock->out_lock);
    }

    /* Set the socket to blocking again so that we can close the socket without
     * having to worry about an EAGAIN error. */
    val = fcntl(a_sock->sockfd, F_GETFL, 0);
    if (val == -1)
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error for F_GETFL in fcntl(): %s\n", strerror(errno));
      retval = TRUE;
    }
    else if (fcntl(a_sock->sockfd, F_SETFL, val & ~O_NONBLOCK))
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error for F_SETFL in fcntl(): %s\n", strerror(errno));
      retval = TRUE;
    }
    else if (close(a_sock->sockfd))
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Error in close(): %s\n", strerror(errno));
      retval = TRUE;
    }
    else
    {
      a_sock->is_connected = FALSE;
      a_sock->sockfd = -1;
      a_sock->out_is_flushed = TRUE;
      retval = FALSE;
    }
  }
  else
  {
    retval = TRUE;
  }

  /* Make *sure* this a_sock->error is set to TRUE. */
  a_sock->error = TRUE;
  
  return retval;
}
