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

#include "../include/libsock/libsock.h"

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>

#include "../include/libsock/sock_p.h"
#include "../include/libsock/sock_l.h"
#include "../include/libsock/sockb_l.h"

cw_sock_t *
sock_new(cw_sock_t * a_sock, cw_uint32_t a_in_max_buf_size)
{
  cw_sock_t * retval;
  
  if (a_sock == NULL)
  {
    retval = (cw_sock_t *) _cw_malloc(sizeof(cw_sock_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_sock;
    retval->is_malloced = FALSE;
  }

  if (NULL == buf_new(&retval->in_buf))
  {
    if (retval->is_malloced)
    {
      _cw_free(retval);
      retval = NULL;
      goto RETURN;
    }
  }
  if (NULL == buf_new(&retval->out_buf))
  {
    buf_delete(&retval->in_buf);
    if (retval->is_malloced)
    {
      _cw_free(retval);
      retval = NULL;
      goto RETURN;
    }
  }
  
  mtx_new(&retval->lock);
  cnd_new(&retval->callback_cnd);

  mtx_new(&retval->state_lock);
  retval->sockfd = -1;
  retval->is_connected = FALSE;
  retval->in_progress = FALSE;
  retval->error = TRUE;

  retval->in_max_buf_size = a_in_max_buf_size;
  mtx_new(&retval->in_lock);
  retval->in_need_signal_count = 0;
  cnd_new(&retval->in_cnd);

  mtx_new(&retval->out_lock);
  retval->sockb_in_progress = FALSE;
  retval->out_need_broadcast_count = 0;
  retval->out_is_flushed = TRUE;
  cnd_new(&retval->out_cnd);

#ifdef _LIBSOCK_DBG
  retval->magic = _LIBSOCK_SOCK_MAGIC;
#endif

  RETURN:
  return retval;
}

void
sock_delete(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

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
#ifdef _LIBSOCK_DBG
  else
  {
    a_sock->magic = _LIBSOCK_SOCK_MAGIC;
  }
#endif
}

cw_bool_t
sock_is_connected(cw_sock_t * a_sock)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  /* Yeah, it's strange naming that results in us not returning
   * a_sock->is_connected.  The problem is that a_sock->error is the first
   * indication that we're disconnected, whereas a_sock->is_connected means that
   * we realize that fact internally. */
  mtx_lock(&a_sock->state_lock);
  retval = !a_sock->error;
  mtx_unlock(&a_sock->state_lock);
  
  return retval;
}

cw_uint32_t
sock_get_port(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  return a_sock->port;
}

cw_sint32_t
sock_connect(cw_sock_t * a_sock, const char * a_server_host, int a_port,
	     struct timespec * a_timeout)
{
  cw_sint32_t retval;
  int error = 0;
  cw_uint32_t server_ip;
  struct sockaddr_in server_addr;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  _cw_check_ptr(a_server_host);
  mtx_lock(&a_sock->state_lock);

  if (TRUE == a_sock->is_connected)
  {
    /* We're already connected to someone! */
    retval = -1;
    goto RETURN;
  }

  if (FALSE == a_sock->in_progress)
  {
    a_sock->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (a_sock->sockfd < 0)
    {
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error in socket(): [s]\n", strerror(errno));
      }
      retval = -1;
      goto RETURN;
    }

    if (sock_p_config_socket(a_sock, TRUE))
    {
      a_sock->sockfd = -1;
      retval = -1;
      goto RETURN;
    }

    /* Figure out the server's IP address. */
    if (sockb_l_get_host_ip(a_server_host, &server_ip))
    {
      if (close(a_sock->sockfd))
      {
	if (dbg_is_registered(cw_g_dbg, "sock_error"))
	{
	  out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "Error in close(): [s]\n", strerror(errno));
	}
      }
      a_sock->sockfd = -1;
      retval = -1;
      goto RETURN;
    }
  
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
  
    (server_addr.sin_addr).s_addr = server_ip;
    server_addr.sin_port = htons(a_port);

    error = connect(a_sock->sockfd,
		    (struct sockaddr *) &server_addr,
		    sizeof(struct sockaddr_in));
  }
  
  if ((0 > error) || (TRUE == a_sock->in_progress))
  {
    if ((errno == EINPROGRESS) || (TRUE == a_sock->in_progress))
    {
      struct pollfd pfd;
      int timeout;

      bzero(&pfd, sizeof(struct pollfd));
      pfd.fd = a_sock->sockfd;
      pfd.events = POLLIN | POLLOUT;

      /* Convert a_timeout to something useful to poll(). */
      if (NULL == a_timeout)
      {
	timeout = -1;
      }
      else
      {
	timeout = (a_timeout->tv_sec * 1000) + (a_timeout->tv_nsec / 1000000);

	if (0 > timeout)
	{
	  timeout = INT_MAX;
	}
      }
      
      if (0 > poll(&pfd, 1, timeout))
      {
	if (dbg_is_registered(cw_g_dbg, "sock_error"))
	{
	  out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "Error in poll(): [s]\n", strerror(errno));
	}
	a_sock->sockfd = -1;
	retval = -1;
	goto RETURN;
      }
      else if (pfd.revents & POLLOUT)
      {
	if (pfd.revents & POLLIN)
	{
	  int error, len;

	  /* Make sure that the socket is both readable and writeable because
	   * data has already arrived. */
	  len = sizeof(error);
	  error = getsockopt(a_sock->sockfd, SOL_SOCKET, SO_ERROR,
			     (void *) &error, &len);
	  if (error < 0)
	  {
	    if (dbg_is_registered(cw_g_dbg, "sock_error"))
	    {
	      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			"Error in getsockopt(): [s]\n", strerror(errno));
	    }
	    a_sock->sockfd = -1;
	    retval = -1;
	    goto RETURN;
	  }
	  else if (error > 0)
	  {
	    if (dbg_is_registered(cw_g_dbg, "sock_error"))
	    {
	      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			"Error in getsockopt() due to connect(): [s]\n",
			strerror(error));
	    }
	    a_sock->sockfd = -1;
	    retval = -1;
	    goto RETURN;
	  }
	}
      }
      else
      {
	/* Timed out. */
	if (dbg_is_registered(cw_g_dbg, "sock_error"))
	{
	  out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "poll() timeout.  Connection failed\n");
	}
	a_sock->in_progress = TRUE;
	retval = 1;
	goto RETURN;
      }
    }
    else
    {
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error in connect(): [s]\n", strerror(errno));
      }
      
      if (-1 == close(a_sock->sockfd))
      {
	if (dbg_is_registered(cw_g_dbg, "sock_error"))
	{
	  out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "Error in close(): [s]\n", strerror(errno));
	}
      }
      
      a_sock->sockfd = -1;
      retval = -1;
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
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error in getsockname(): [s]\n", strerror(errno));
      }
      retval = -1;
      goto RETURN;
    }
    else
    {
      a_sock->port = (cw_uint32_t) ntohs(name.sin_port);
    }
  }

  /* This is the only path through this function that does not handle an error,
   * so we can unconditionally toggle a_sock->is_connected here and only
   * here. */
  a_sock->is_connected = TRUE;
  a_sock->in_progress = FALSE;
  a_sock->error = FALSE;
  
  mtx_lock(&a_sock->lock);
  if (TRUE == sockb_l_register_sock(a_sock))
  {
    mtx_unlock(&a_sock->lock);
    retval = -1;
    goto RETURN;
  }
  cnd_wait(&a_sock->callback_cnd, &a_sock->lock);
  if (TRUE == a_sock->error)
  {
    mtx_unlock(&a_sock->lock);
    retval = -1;
    goto RETURN;
  }
  
  a_sock->is_registered = TRUE;
  mtx_unlock(&a_sock->lock);
    
  retval = 0;

  RETURN:
  mtx_unlock(&a_sock->state_lock);
  return retval;
}

cw_bool_t
sock_wrap(cw_sock_t * a_sock, int a_sockfd, cw_bool_t a_init)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  mtx_lock(&a_sock->state_lock);
  
  if (TRUE == a_sock->is_connected)
  {
    /* We're already connected to someone! */
    retval = TRUE;
    goto RETURN;
  }

  a_sock->sockfd = a_sockfd;
  if (sock_p_config_socket(a_sock, a_init))
  {
    a_sock->sockfd = -1;
    retval = TRUE;
  }
  else
  {
    if (TRUE == a_init)
    {
      struct sockaddr_in name;
      int name_size;
      
      /* Get the port number for the socket. */

      name_size = sizeof(name);
      if (0 > getsockname(a_sock->sockfd,
			  (struct sockaddr *) &name,
			  &name_size))
      {
	if (dbg_is_registered(cw_g_dbg, "sock_error"))
	{
	  out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "Error in getsockname(): [s]\n", strerror(errno));
	}
	retval = TRUE;
	goto RETURN;
      }
      else
      {
	a_sock->port = (cw_uint32_t) ntohs(name.sin_port);
      }
    }
    else
    {
      a_sock->port = 0;
    }
    
    a_sock->is_connected = TRUE;
    a_sock->error = FALSE;

    mtx_lock(&a_sock->lock);
    if (TRUE == sockb_l_register_sock(a_sock))
    {
      mtx_unlock(&a_sock->lock);
      retval = TRUE;
      goto RETURN;
    }
    cnd_wait(&a_sock->callback_cnd, &a_sock->lock);
    if (TRUE == a_sock->error)
    {
      mtx_unlock(&a_sock->lock);
      retval = TRUE;
      goto RETURN;
    }
    
    a_sock->is_registered = TRUE;
    mtx_unlock(&a_sock->lock);
    
    retval = FALSE;
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
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  retval = sock_p_disconnect(a_sock);
  
  return retval;
}

cw_uint32_t
sock_buffered_in(cw_sock_t * a_sock)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  
  mtx_lock(&a_sock->in_lock);
  retval = buf_get_size(&a_sock->in_buf);
  mtx_unlock(&a_sock->in_lock);

  return retval;
}

cw_sint32_t
sock_read(cw_sock_t * a_sock, cw_buf_t * a_spare, cw_sint32_t a_max_read,
	  struct timespec * a_timeout)
{
  cw_sint32_t retval, size;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  _cw_check_ptr(a_spare);

  mtx_lock(&a_sock->in_lock);
  if (a_sock->error)
  {
    /* Even though the sock has been closed, give the user back any queued data
     * before indicating an error. */
    
    size = buf_get_size(&a_sock->in_buf);
    if (0 == size)
    {
      retval = -2;
    }
    else if ((a_max_read == 0) || (size < a_max_read))
    {
      if (TRUE == (buf_catenate_buf(a_spare, &a_sock->in_buf, FALSE)))
      {
	retval = -1;
      }
      else
      {
	retval = size;
      }
    }
    else
    {
      if (TRUE == (buf_split(a_spare, &a_sock->in_buf, a_max_read)))
      {
	retval = -1;
      }
      else
      {
	retval = a_max_read;
      }
    }
    
    mtx_unlock(&a_sock->in_lock);
  }
  else
  {
    if (0 == buf_get_size(&a_sock->in_buf))
    {
      /* There's no data available right now. */
      a_sock->in_need_signal_count++;
      if (NULL == a_timeout)
      {
	cnd_wait(&a_sock->in_cnd, &a_sock->in_lock);
      }
      else if ((0 != a_timeout->tv_sec) || (0 != a_timeout->tv_nsec))
      {
	/* a_timeout is non-zero, so wait.  We could call cnd_timedwait()
	 * unconditionally, but there's no real need to. */
	cnd_timedwait(&a_sock->in_cnd, &a_sock->in_lock, a_timeout);
      }
      a_sock->in_need_signal_count--;
    }

    size = buf_get_size(&a_sock->in_buf);
    if (0 < size)
    {
      _cw_assert(a_sock->in_max_buf_size >= size);
      if (a_sock->in_max_buf_size == size)
      {
	/* The incoming buffer was maxed, but now there is space. */
	while (TRUE == sockb_l_in_space(a_sock->sockfd))
	{
	}
      }
      
      if ((a_max_read == 0) || (size < a_max_read))
      {
	if (TRUE == (buf_catenate_buf(a_spare, &a_sock->in_buf, FALSE)))
	{
	  retval = -1;
	}
	else
	{
	  retval = size;
	}
      }
      else
      {
	if (TRUE == (buf_split(a_spare, &a_sock->in_buf, a_max_read)))
	{
	  retval = -1;
	}
	else
	{
	  retval = a_max_read;
	}
      }

      mtx_unlock(&a_sock->in_lock);
      
      if (size >= a_sock->in_max_buf_size)
      {
	sockb_l_wakeup();
      }
    }
    else
    {
      /* Make sure there wasn't an error. */
      if (TRUE == a_sock->error)
      {
	retval = -2;
      }
      else
      {
	retval = 0;
      }
      
      mtx_unlock(&a_sock->in_lock);
    }
  }
  
  return retval;
}

cw_bool_t
sock_write(cw_sock_t * a_sock, cw_buf_t * a_buf)
{
  cw_bool_t retval;
  cw_uint32_t out_buf_size;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  _cw_check_ptr(a_buf);

  mtx_lock(&a_sock->out_lock);
      
  if (TRUE == a_sock->error)
  {
    retval = TRUE;
    goto RETURN;
  }

  if (0 < buf_get_size(a_buf))
  {
    if (TRUE == buf_catenate_buf(&a_sock->out_buf, a_buf, FALSE))
    {
      retval = TRUE;
      goto RETURN;
    }

    out_buf_size = buf_get_size(&a_sock->out_buf);

    if ((FALSE == a_sock->sockb_in_progress)
	&& (TRUE == a_sock->out_is_flushed))
    {
      const struct iovec * iovec;
      int iovec_count;
      cw_uint32_t bytes_written;
     
      /* Try to write the data immediately, instead of always context switching
       * to the sockb thread. */
      iovec = buf_get_iovec(&a_sock->out_buf,
			    out_buf_size,
			    TRUE,
			    &iovec_count);
      
      bytes_written = writev(a_sock->sockfd, iovec, iovec_count);
#ifdef _LIBSOCK_CONFESS
      out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		"[i]w?([i|s:s]/[i])\n",
		a_sock->sockfd, bytes_written, buf_get_size(&a_sock->out_buf));
#endif

      if (0 < (cw_sint32_t) bytes_written)
      {
	buf_release_head_data(&a_sock->out_buf, bytes_written);
	out_buf_size -= bytes_written;
      }
      else if ((0 > (cw_sint32_t) bytes_written) && (EAGAIN != errno))
      {
	/* Socket error.  Unregister the socket. */

	/* sock_p_disconnect() locks out_lock, so avoid deadlock. */
	mtx_unlock(&a_sock->out_lock);
	
	sock_p_disconnect(a_sock);
	retval = TRUE;
	goto WRITE_ERROR;
      }
      
      if (0 < out_buf_size)
      {
	if (TRUE == a_sock->out_is_flushed)
	{
	  /* Notify the sockb that we now have data. */
	  if (TRUE == sockb_l_out_notify(a_sock->sockfd))
	  {
	    retval = TRUE;
	    goto RETURN;
	  }
	  a_sock->out_is_flushed = FALSE;
	}
      }
      else
      {
	a_sock->out_is_flushed = TRUE;
      }
    }
    else if (a_sock->out_is_flushed == TRUE)
    {
      /* Notify the sockb that we now have data. */
      if (TRUE == sockb_l_out_notify(a_sock->sockfd))
      {
	retval = TRUE;
	goto RETURN;
      }
      a_sock->out_is_flushed = FALSE;
    }
  }
  
  retval = FALSE;

  RETURN:
  mtx_unlock(&a_sock->out_lock);
  WRITE_ERROR:
  return retval;
}

cw_bool_t
sock_flush_out(cw_sock_t * a_sock)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  mtx_lock(&a_sock->out_lock);
  if (a_sock->error)
  {
    retval = TRUE;
    goto RETURN;
  }

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
    goto RETURN;
  }

  retval = FALSE;

  RETURN:
  mtx_unlock(&a_sock->out_lock);
  return retval;
}

int
sock_get_fd(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  return a_sock->sockfd;
}

cw_uint32_t
sock_l_get_in_space(cw_sock_t * a_sock)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  retval = a_sock->in_max_buf_size;

  mtx_lock(&a_sock->in_lock);
  retval -= buf_get_size(&a_sock->in_buf);
  mtx_unlock(&a_sock->in_lock);
  
  if (retval > a_sock->os_inbuf_size)
  {
    retval = a_sock->os_inbuf_size;
  }

  return retval;
}

cw_uint32_t
sock_l_get_in_size(cw_sock_t * a_sock)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  mtx_lock(&a_sock->in_lock);
  retval = buf_get_size(&a_sock->in_buf);
  mtx_unlock(&a_sock->in_lock);

  return retval;
}

cw_uint32_t
sock_l_get_in_max_buf_size(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  
  return a_sock->in_max_buf_size;
}

void
sock_l_get_out_data(cw_sock_t * a_sock, cw_buf_t * r_buf)
{
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  _cw_check_ptr(r_buf);

  mtx_lock(&a_sock->out_lock);

  /* If buf_split() or buf_catenate() has a memory allocation, it isn't fatal,
   * since it will simply cause the sockb thread to try to write 0 bytes.  In
   * actuality, this is extremely unlikely to happen in the steady state, but
   * even if it does, oh well. */
  if (buf_get_size(&a_sock->out_buf) > a_sock->os_outbuf_size)
  {
    buf_split(r_buf, &a_sock->out_buf, a_sock->os_outbuf_size);
  }
  else
  {
    buf_catenate_buf(r_buf, &a_sock->out_buf, FALSE);
  }

  /* Make a note that the sockb thread currently has data. */
  a_sock->sockb_in_progress = TRUE;

  mtx_unlock(&a_sock->out_lock);
}

cw_uint32_t
sock_l_put_back_out_data(cw_sock_t * a_sock, cw_buf_t * a_buf)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  _cw_check_ptr(a_buf);

  mtx_lock(&a_sock->out_lock);

  /* Make a note that the sockb thread gave our data back, or got rid of it for
   * us. */
  a_sock->sockb_in_progress = FALSE;
  
  /* It's very unlikely that a memory error would occur in buf_catenate_buf()
   * here, since we previously had at least as much data buffered in a_sock, and
   * a_buf tends to have a sufficiently expanded bufel array.  However, an error
   * could occur, and there's no good way to deal with it. */
  if (0 < buf_get_size(&a_sock->out_buf))
  {
    /* There are still data in out_buf, so preserve the order. */
    while (TRUE == buf_catenate_buf(a_buf, &a_sock->out_buf, FALSE))
    {
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		  "Memory allocation error; yielding\n");
      }
      thd_yield();
    }
  }
  while (TRUE == buf_catenate_buf(&a_sock->out_buf, a_buf, FALSE))
  {
    if (dbg_is_registered(cw_g_dbg, "sock_error"))
    {
      out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		"Memory allocation error; yielding\n");
    }
    thd_yield();
  }
  _cw_assert(0 == buf_get_size(a_buf));
  
  retval = buf_get_size(&a_sock->out_buf);
  if (0 == retval)
  {
    a_sock->out_is_flushed = TRUE;
    if (0 < a_sock->out_need_broadcast_count)
    {
      cnd_broadcast(&a_sock->out_cnd);
    }
  }
  mtx_unlock(&a_sock->out_lock);

  return retval;
}

cw_uint32_t
sock_l_put_in_data(cw_sock_t * a_sock, cw_buf_t * a_buf)
{
  cw_uint32_t retval;
  cw_uint32_t buffered_in;
  
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);
  _cw_check_ptr(a_buf);

  mtx_lock(&a_sock->in_lock);

  while (TRUE == buf_catenate_buf(&a_sock->in_buf, a_buf, FALSE))
  {
    if (dbg_is_registered(cw_g_dbg, "sock_error"))
    {
      out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		"Memory allocation error; yielding\n");
    }
    thd_yield();
  }

  if (a_sock->in_need_signal_count > 0)
  {
    /* Someone wants to know that there are data available. */
    cnd_signal(&a_sock->in_cnd);
  }

  /* If a socket is closed by the peer, it is possible that the sockb thread
   * will be forced to over-fill our input buffer.  If we're over-full, avoid
   * wrapping the retval. */
  buffered_in = buf_get_size(&a_sock->in_buf);
  if (a_sock->in_max_buf_size > buffered_in)
  {
    retval = a_sock->in_max_buf_size - buffered_in;
  }
  else
  {
    retval = 0;
  }
  
  mtx_unlock(&a_sock->in_lock);

  return retval;
}

void
sock_l_message_callback(cw_sock_t * a_sock, cw_bool_t a_error)
{
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  mtx_lock(&a_sock->lock);
  mtx_lock(&a_sock->in_lock);
  mtx_lock(&a_sock->out_lock);
  
  if (TRUE == a_error)
  {
    a_sock->error = TRUE;
  }
  cnd_signal(&a_sock->callback_cnd);
  
  mtx_unlock(&a_sock->out_lock);
  mtx_unlock(&a_sock->in_lock);
  mtx_unlock(&a_sock->lock);
}

void
sock_l_error_callback(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);
  _cw_assert(_LIBSOCK_SOCK_MAGIC == a_sock->magic);

  mtx_lock(&a_sock->state_lock);
  mtx_lock(&a_sock->lock);
  mtx_lock(&a_sock->in_lock);
  mtx_lock(&a_sock->out_lock);
  a_sock->error = TRUE;
  mtx_unlock(&a_sock->out_lock);
  mtx_unlock(&a_sock->in_lock);
  mtx_unlock(&a_sock->lock);
  mtx_unlock(&a_sock->state_lock);

  /* Wake up waiting threads, if there are any. */
  if (FALSE == mtx_trylock(&a_sock->in_lock))
  {
    if (0 != a_sock->in_need_signal_count)
    {
      cnd_signal(&a_sock->in_cnd);
    }
    mtx_unlock(&a_sock->in_lock);
  }
  if (FALSE == mtx_trylock(&a_sock->out_lock))
  {
    if (0 != a_sock->out_need_broadcast_count)
    {
      cnd_broadcast(&a_sock->out_cnd);
    }
    mtx_unlock(&a_sock->out_lock);
  }
}

static cw_bool_t
sock_p_config_socket(cw_sock_t * a_sock, cw_bool_t a_init)
{
  cw_bool_t retval;
  int val, len;
  struct linger linger_struct;

  if (TRUE == a_init)
  {
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
        out_put_e(cw_g_out, NULL, 0, __FUNCTION__, \
		  "Error for [s] in getsockopt(): [s]\n", \
                  #a, strerror(errno)); \
        retval = TRUE; \
        goto RETURN; \
      } \
      else \
      { \
        out_put_e(cw_g_out, NULL, 0, __FUNCTION__, \
		  "[s]: [i]\n", #a, val); \
      }

      _CW_SOCK_GETSOCKOPT(SO_REUSEADDR);
#ifdef SO_REUSEPORT
      _CW_SOCK_GETSOCKOPT(SO_REUSEPORT);
#endif
      _CW_SOCK_GETSOCKOPT(SO_KEEPALIVE);
      _CW_SOCK_GETSOCKOPT(SO_OOBINLINE);
      _CW_SOCK_GETSOCKOPT(SO_SNDBUF);
      _CW_SOCK_GETSOCKOPT(SO_RCVBUF);
#ifdef SO_SNDLOWAIT
      _CW_SOCK_GETSOCKOPT(SO_SNDLOWAT);
#endif
#ifdef SO_RCVLOWAIT
      _CW_SOCK_GETSOCKOPT(SO_RCVLOWAT);
#endif
#ifdef SO_SNDTIMEO
      _CW_SOCK_GETSOCKOPT(SO_SNDTIMEO);
#endif
#ifdef SO_RCVTIMEO
      _CW_SOCK_GETSOCKOPT(SO_RCVTIMEO);
#endif

#undef _CW_SOCK_GETSOCKOPT
    }
  
    /* Tell the socket to wait and try to flush buffered data before closing at
     * the end.
     *
     * 10 seconds is a bit arbitrary, eh? */
    linger_struct.l_onoff = 1;
    linger_struct.l_linger = 10;
    if (setsockopt(a_sock->sockfd, SOL_SOCKET, SO_LINGER,
		   (void *) &linger_struct, sizeof(linger_struct)))
    {
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error for SO_LINGER in setsockopt(): [s]\n",
		  strerror(errno));
      }
      retval = TRUE;
      goto RETURN;
    }
    else if (dbg_is_registered(cw_g_dbg, "sock_sockopt"))
    {
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		"SO_LINGER: [s], [i] second[s]\n",
		linger_struct.l_onoff ? "on" : "off",
		linger_struct.l_linger,
		linger_struct.l_linger != 1 ? "s" : "");
    }

#ifdef SO_SNDLOWAIT
    /* Set the socket to not buffer outgoing data without trying to send it. */
    val = 1;
    if (setsockopt(a_sock->sockfd, SOL_SOCKET, SO_SNDLOWAT,
		   (void *) &val, sizeof(val)))
    {
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error for SO_SNDLOWAT in setsockopt(): [s]\n",
		  strerror(errno));
      }
      retval = TRUE;
      goto RETURN;
    }
    else if (dbg_is_registered(cw_g_dbg, "sock_sockopt"))
    {
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		"SO_SNDLOWAT: [d]\n", val);
    }
#endif

    /* Re-use the socket. */
    val = 1;
    if (0 > setsockopt(a_sock->sockfd, SOL_SOCKET, SO_REUSEADDR,
		       (void *) &val, sizeof(val)))
    {
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		"Error for SO_REUSEADDR in setsockopt(): [s]\n",
		strerror(errno));
      retval = TRUE;
      goto RETURN;
    }
  }

  /* Get the size of the OS's incoming buffer, so that we don't place undo
   * strain on the sockb thread when it asks how much it should try to read. */
  {
    int len;
    
    len = sizeof(a_sock->os_inbuf_size);
    if (getsockopt(a_sock->sockfd, SOL_SOCKET, (SO_RCVBUF),
		   (void *) &a_sock->os_inbuf_size, &len))
    {
      /* Just choose some number... */
      a_sock->os_inbuf_size = 65536;
    }
  }
  
  /* Get the size of the OS's outgoing buffer, so that we can hand no more than
   * that amount to sockb each time it asks for data. */
  {
    int len;
    
    len = sizeof(a_sock->os_outbuf_size);
    if (getsockopt(a_sock->sockfd, SOL_SOCKET, SO_SNDBUF,
		   (void *) &a_sock->os_outbuf_size, &len))
    {
      /* Just choose some number... */
      a_sock->os_outbuf_size = 65536;
    }
  }

  /* Set the socket to non-blocking, so that we don't have to worry about
   * sockb's poll loop locking up. */
  val = fcntl(a_sock->sockfd, F_GETFL, 0);
  if (val == -1)
  {
    if (dbg_is_registered(cw_g_dbg, "sock_error"))
    {
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		"Error for F_GETFL in fcntl(): [s]\n", strerror(errno));
    }
    retval = TRUE;
    goto RETURN;
  }
  if (fcntl(a_sock->sockfd, F_SETFL, val | O_NONBLOCK))
  {
    if (dbg_is_registered(cw_g_dbg, "sock_error"))
    {
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		"Error for F_SETFL in fcntl(): [s]\n", strerror(errno));
    }
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
    mtx_lock(&a_sock->state_lock);
    mtx_lock(&a_sock->lock);
    if (FALSE == a_sock->error)
    {
      mtx_lock(&a_sock->in_lock);
      mtx_lock(&a_sock->out_lock);
      a_sock->error = TRUE;
      mtx_unlock(&a_sock->out_lock);
      mtx_unlock(&a_sock->in_lock);
      mtx_unlock(&a_sock->state_lock);
  
      if (TRUE == sockb_l_unregister_sock(a_sock->sockfd))
      {
	mtx_unlock(&a_sock->lock);
	retval = TRUE;
	goto RETURN;
      }
      cnd_wait(&a_sock->callback_cnd, &a_sock->lock);
    }
    else
    {
      mtx_unlock(&a_sock->state_lock);
    }
    
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
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error for F_GETFL in fcntl(): [s]\n", strerror(errno));
      }
      retval = TRUE;
    }
    else if (fcntl(a_sock->sockfd, F_SETFL, val & ~O_NONBLOCK))
    {
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error for F_SETFL in fcntl(): [s]\n", strerror(errno));
      }
      retval = TRUE;
    }
    else if (close(a_sock->sockfd))
    {
      if (dbg_is_registered(cw_g_dbg, "sock_error"))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "Error in close(): [s]\n", strerror(errno));
      }
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

  RETURN:
  return retval;
}
