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
 *
 *
 ****************************************************************************/

#define _LIBSOCK_USE_SOCKB
#include "libsock/libsock.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libsock/sockb_p.h"
#include "libsock/sockb_l.h"
#include "libsock/sock_l.h"

/* Global. */
cw_sockb_t * g_sockb = NULL;

void
sockb_init(cw_uint32_t a_bufel_size, cw_uint32_t a_max_spare_bufels)
{
  char * tmpfile_name, buf[L_tmpnam];
  
  _cw_assert(g_sockb == NULL);
  _cw_assert(a_bufel_size > 0);

  g_sockb = (cw_sockb_t *) _cw_malloc(sizeof(cw_sockb_t));
  bzero(g_sockb, sizeof(cw_sockb_t));

  /* Open a temp file with poser_fd, such that the file will disappear as soon
   * as the descripter goes away. */
  tmpfile_name = tmpnam(buf);
  if (tmpfile_name == NULL)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		 "Fatal error in tmpnam(): %s\n", strerror(errno));
    abort();
  }

  g_sockb->poser_fd = open(tmpfile_name,
			   O_RDONLY | O_CREAT | O_TRUNC | O_EXCL,
			   0);
  if (g_sockb->poser_fd < 0)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		 "Fatal error in open(): %s\n", strerror(errno));
    abort();
  }

  if (unlink(tmpfile_name))
  {
    /* Not fatal, but make some noise. */
    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		 "Error in unlink(): %s\n", strerror(errno));
  }

  /* Ignore SIGPIPE, so that writing to a closed socket won't crash the
   * program. */
  signal(SIGPIPE, SIG_IGN);
  
  /* Create a pipe that will be used in conjunction with the message queues to
   * make the back end thread return from the select() call. */
  {
    int filedes[2];

    if (-1 == pipe(filedes))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "Fatal error in pipe(): %s\n", strerror(errno));
      abort();
    }
    g_sockb->pipe_out = filedes[0];
    g_sockb->pipe_in = filedes[1];

    /* Set g_sockb->pipe_in to non-blocking. */
    /* XXX Why?  What about pipe_out (below)? */
    {
      int val;
      val = fcntl(g_sockb->pipe_in, F_GETFL, 0);
      if (val == -1)
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error for F_GETFL in fcntl(): %s\n",
		    strerror(errno));
	abort();
      }
      if (fcntl(g_sockb->pipe_in, F_SETFL, val | O_NONBLOCK))
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error for F_SETFL in fcntl(): %s\n",
		    strerror(errno));
	abort();
      }
    }
    {
      int val;
      val = fcntl(g_sockb->pipe_out, F_GETFL, 0);
      if (val == -1)
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error for F_GETFL in fcntl(): %s\n",
		    strerror(errno));
	abort();
      }
      if (fcntl(g_sockb->pipe_in, F_SETFL, val | O_NONBLOCK))
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error for F_SETFL in fcntl(): %s\n",
		    strerror(errno));
	abort();
      }
    }
  }
  
  g_sockb->should_quit = FALSE;

  /* Create the semaphore used for determining whether data should be written to
   * the pipe in order to force a return from select(). */
  sem_new(&g_sockb->pipe_sem, 1);

  /* Create the spare bufel pool and initialize associated variables. */
  bufpool_new(&g_sockb->bufel_pool, sizeof(cw_bufel_t), a_max_spare_bufels);
  bufpool_new(&g_sockb->bufc_pool, sizeof(cw_bufc_t), a_max_spare_bufels);
  bufpool_new(&g_sockb->buffer_pool, a_bufel_size, a_max_spare_bufels);
  
  /* Create the message queues. */
  list_new(&g_sockb->registrations, TRUE);
  list_new(&g_sockb->unregistrations, TRUE);
  list_new(&g_sockb->out_notifications, TRUE);

  /* Create the lock used for protecting gethostbyaddr(). */
  mtx_new(&g_sockb->get_ip_addr_lock);

  /* Create a new thread to handle all of the back end socket foo. */
  thd_new(&g_sockb->thread, sockb_p_entry_func, NULL);
}

void
sockb_shutdown(void)
{
  _cw_check_ptr(g_sockb);

  /* Tell the back end thread to quit, then join on it. */
  g_sockb->should_quit = TRUE;
  sockb_p_select_return();
  thd_join(&g_sockb->thread);
  thd_delete(&g_sockb->thread);

  sem_delete(&g_sockb->pipe_sem);
  
  /* Clean up the spare bufel's. */
  bufpool_delete(&g_sockb->bufel_pool);
  bufpool_delete(&g_sockb->bufc_pool);
  bufpool_delete(&g_sockb->buffer_pool);
  
  list_delete(&g_sockb->registrations);
  list_delete(&g_sockb->unregistrations);
  list_delete(&g_sockb->out_notifications);

  if (close(g_sockb->poser_fd))
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		 "Error in close(): %s\n", strerror(errno));
  }

  /* Delete the gethostbyaddr() protection lock. */
  mtx_delete(&g_sockb->get_ip_addr_lock);
  
  _cw_free(g_sockb);
}

cw_bufel_t *
sockb_get_spare_bufel(void)
{
  cw_bufel_t * retval;
  cw_bufc_t * bufc;
  
  _cw_check_ptr(g_sockb);

  retval = bufel_new((cw_bufel_t *) bufpool_get_buffer(&g_sockb->bufel_pool),
		     bufpool_put_buffer,
		     (void *) &g_sockb->bufel_pool);
  bufc = bufc_new((cw_bufc_t *) bufpool_get_buffer(&g_sockb->bufc_pool),
		  bufpool_put_buffer,
		  (void *) &g_sockb->bufc_pool);
  bufc_set_buffer(bufc,
		  bufpool_get_buffer(&g_sockb->buffer_pool),
		  bufpool_get_buffer_size(&g_sockb->buffer_pool),
		  bufpool_put_buffer,
		  (void *) &g_sockb->buffer_pool);
  bufel_set_bufc(retval, bufc);
  bufel_set_beg_offset(retval, 0);

  return retval;
}

/****************************************************************************
 *
 * Tell g_sockb that a_sock needs to be handled.
 *
 ****************************************************************************/
void
sockb_l_register_sock(cw_sock_t * a_sock)
{
  _cw_check_ptr(a_sock);
  _cw_check_ptr(g_sockb);
  _cw_assert(sock_l_get_fd(a_sock) >= 0);
  _cw_assert(sock_l_get_fd(a_sock) < FD_SETSIZE);

  list_tpush(&g_sockb->registrations, (void *) a_sock);
  sockb_p_select_return();
}

/****************************************************************************
 *
 * Tell g_sockb to stop handling a_sockfd.
 *
 ****************************************************************************/
void
sockb_l_unregister_sock(cw_uint32_t * a_sockfd)
{
  _cw_check_ptr(g_sockb);
  _cw_assert(*a_sockfd < FD_SETSIZE);

  list_tpush(&g_sockb->unregistrations, (void *) a_sockfd);
  sockb_p_select_return();
}

/****************************************************************************
 *
 * Tell g_sockb that there is outgoing data for *a_sockfd.  A pointer to
 * a_sockfd is needed so that we can cleanly shove the descriptor number into
 * the out_notifications list.  This of course means that a_sockfd has to stick
 * around until the notification is handled, so the sock needs to pass a pointer
 * to its internal storage for the sockfd number so we can be sure that it
 * sticks around.
 *
 ****************************************************************************/
void
sockb_l_out_notify(cw_uint32_t * a_sockfd)
{
  _cw_check_ptr(g_sockb);
  _cw_assert(*a_sockfd < FD_SETSIZE);

  list_tpush(&g_sockb->out_notifications, (void *) a_sockfd);
  sockb_p_select_return();
}

/****************************************************************************
 *
 * Convert a_host_str to an IP address and put it in *a_host_ip.  Return TRUE if
 * there is an error.
 *
 * This function is necessary because gethostbyname() is not reentrant.  Of
 * course, there may be gethostbyname_r() on the system, but even if there is,
 * the resolver functions and BIND probably are not reentrant, according to
 * Stevens's UNP, 2nd Ed., Volume 1, page 305.  That being the case, this code
 * doesn't even bother with gethostbyname_r().
 *
 ****************************************************************************/
cw_bool_t
sockb_l_get_host_ip(char * a_host_str, cw_uint32_t * a_host_ip)
{
  cw_bool_t retval;
  cw_uint32_t host_ip;

  _cw_check_ptr(g_sockb);
    
  host_ip = inet_addr(a_host_str);
  if (host_ip == INADDR_NONE)
  {
    struct hostent * host_entry;
    
    /* Not a dotted number IP address.  Let's try it as a hostname. */
    
    /* Lock the mutex first. */
    mtx_lock(&g_sockb->get_ip_addr_lock);
    
    host_entry = gethostbyname(a_host_str);
    if (host_entry == NULL)
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "Error in gethostbyname(): %s\n", hstrerror(h_errno));
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "Host \"%s\" isn't an IP address or a hostname\n",
		  a_host_str);
      retval = TRUE;
    }
    else
    {
      struct in_addr * addr_ptr;
      
      _cw_assert(host_entry->h_addrtype == AF_INET);
      _cw_assert(host_entry->h_addr_list[0] != NULL);

      addr_ptr = (struct in_addr *) host_entry->h_addr_list[0];
      *a_host_ip = addr_ptr->s_addr;

      retval = FALSE;
    }
    /* We're done with h_errno and the global structure pointed to by
     * host_entry, so unlock the mutex now. */
    mtx_unlock(&g_sockb->get_ip_addr_lock);
  }
  else
  {
    *a_host_ip = host_ip;
    retval = FALSE;
  }

  if (dbg_is_registered(cw_g_dbg, "sockb_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"IP address: %d.%d.%d.%d\n",
		*a_host_ip & 0xff,
		(*a_host_ip >> 8) & 0xff,
		(*a_host_ip >> 16) & 0xff,
		*a_host_ip >> 24);
  }

  return retval;
}

/****************************************************************************
 *
 * Returns a useless, but reserved file descriptor within the range that
 * select() can handle.  The caller is responsible for disposing of the file
 * descriptor.  If none are available in the needed range, return -1.
 *
 ****************************************************************************/
int
sockb_l_get_spare_fd(void)
{
  int retval;
  
  _cw_check_ptr(g_sockb);

  retval = dup(g_sockb->poser_fd);

  if (retval < 0)
  {
    if (dbg_is_registered(cw_g_dbg, "sockb_error"))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "Error in dup(): %s\n", strerror(errno));
    }
    retval = -1;
  }
  else if (retval >= FD_SETSIZE)
  {
    if (dbg_is_registered(cw_g_dbg, "sockb_error"))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "Reached maximum number of connections (%d)\n", FD_SETSIZE);
    }
    retval = -1;
  }
  
  return retval;
}

/****************************************************************************
 *
 * Entry point for the back end thread.  The back end thread essentially
 * executes a select() loop.
 *
 ****************************************************************************/
static void *
sockb_p_entry_func(void * a_arg)
{
  cw_sock_t * socks[FD_SETSIZE];
  fd_set fd_m_read_set, fd_read_set;
  fd_set fd_m_write_set, fd_write_set;
  fd_set fd_m_exception_set, fd_exception_set;
  cw_sock_t * sock;
  int sockfd, max_fd = 0, num_ready;
  cw_uint64_t num_msgs, i;
  cw_sint32_t j;
  cw_buf_t tmp_buf, buf_in;

  /* Initialize data structures. */
  FD_ZERO(&fd_m_read_set);
  FD_ZERO(&fd_m_write_set);
  FD_ZERO(&fd_m_exception_set);
  buf_new(&tmp_buf, FALSE);
  buf_new(&buf_in, FALSE);

  /* Add g_sockb->pipe_out to the read set, so that this thread will return from
   * select() when data is written to g_sockb->pipe_in. */
  FD_SET(g_sockb->pipe_out, &fd_m_read_set);
  /* Increase max_fd if necessary. */
  if (g_sockb->pipe_out > max_fd)
  {
    max_fd = g_sockb->pipe_out;
  }

  while (g_sockb->should_quit == FALSE)
  {
    /* Check for messages in the message queues. */

    /* Registration messages. */
    num_msgs = list_count(&g_sockb->registrations);
    for (i = 0; i < num_msgs; i++)
    {
      sock = (cw_sock_t *) list_hpop(&g_sockb->registrations);
      
      sockfd = sock_l_get_fd(sock);
      _cw_assert(!FD_ISSET(sockfd, &fd_m_read_set));

      FD_SET(sockfd, &fd_m_read_set);
      /* XXX Uncomment this if adding out of band data support. */
      /* FD_SET(sockfd, &fd_m_exception_set); */

      /* Increase max_fd if necessary. */
      if (sockfd > max_fd)
      {
	max_fd = sockfd;
      }

      /* Initialize the info for this sockfd. */
      socks[sockfd] = sock;

      /* Notify the sock that it's registered. */
      sock_l_message_callback(sock);
    }

    /* Unregistration messages. */
    num_msgs = list_count(&g_sockb->unregistrations);
    for (i = 0; i < num_msgs; i++)
    {
      sockfd = *(int *) list_hpop(&g_sockb->unregistrations);

      _cw_assert(FD_ISSET(sockfd, &fd_m_read_set));

      FD_CLR(sockfd, &fd_m_read_set);
      FD_CLR(sockfd, &fd_m_write_set); /* May not be set. */
      /* XXX Uncomment this if adding out of band data support. */
      /* FD_CLR(sockfd, &fd_m_exception_set); */

      /* Lower max_fd to the next highest fd, if necessary. */
      if (sockfd == max_fd)
      {
	max_fd = -1; /* In case there aren't any more registered sockets. */
	for (j = sockfd - 1; j >= 0; j--)
	{
	  if (FD_ISSET(j, &fd_m_read_set))
	  {
	    max_fd = j;
	    break;
	  }
	}
      }
	
      /* Notify the sock that it's unregistered. */
      sock_l_message_callback(socks[sockfd]);
    }

    /* Out-going data notifications. */
    num_msgs = list_count(&g_sockb->out_notifications);
    for (i = 0; i < num_msgs; i++)
    {
      sockfd = *(int *) list_hpop(&g_sockb->out_notifications);

      if (FD_ISSET(sockfd, &fd_m_read_set))
      {
	FD_SET(sockfd, &fd_m_write_set);
      }
    }

    /* Copy the master sets of descriptors we care about to the sets that are
     * passed into select(). */
#ifdef _CW_OS_FREEBSD
    FD_COPY(&fd_m_read_set, &fd_read_set);
    FD_COPY(&fd_m_write_set, &fd_write_set);
    FD_COPY(&fd_m_exception_set, &fd_exception_set);
#else
    bcopy(&fd_m_read_set, &fd_read_set, sizeof(fd_m_read_set));
    bcopy(&fd_m_write_set, &fd_write_set, sizeof(fd_m_write_set));
    bcopy(&fd_m_exception_set, &fd_exception_set, sizeof(fd_m_exception_set));
#endif

    num_ready = select(max_fd + 1,
		       &fd_read_set, &fd_write_set, &fd_exception_set,
		       NULL);
    
    if (num_ready == -1)
    {
      if (errno != EINTR)
      {
	/* This is an error that should never happen. */
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error in select(): %s\n", strerror(errno));
	abort();
      }
    }
#if (0)
    else if (num_ready == 0)
    {
      /* This should never happen when there is no timeout. */
      /* Timeout expired.  Oh well. */
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "select() timeout expired\n");
    }
#endif
    else
    {
      cw_sint32_t i, j;
      
      /* Ready descriptors. */
      if (FD_ISSET(g_sockb->pipe_out, &fd_read_set))
      {
	char t_buf[2];
	ssize_t bytes_read;
	
	/* Clear the read bit to avoid attempting to handle it in the loop
	 * below. */
	FD_CLR(g_sockb->pipe_out, &fd_read_set);

	/* Read the data out of the pipe so that the next call doesn't
	 * immediately return just because of data already in the pipe.  Note
	 * that there is no risk of deadlock due to emptying data from the pipe
	 * that is written after the select call, since the message queues are
	 * checked after emptying the pipe, but before calling select()
	 * again. */
	bytes_read = read(g_sockb->pipe_out, t_buf, 2);
	if (bytes_read == -1)
	{
	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		      "Error in read(): %s\n", strerror(errno));
	}
	else if (bytes_read > 0)
	{
	  /* Set the semaphore to one.  This will cause one, and only one byte
	   * to be written to g_sockb->pipe_in and cause a return from select()
	   * if one or more messages needs handled.  Note that we must post the
	   * semophore before handling the message queues, since it is possible
	   * to have new messages come in and miss them otherwise.  Posting
	   * first means that we may execute the select() loop once without
	   * doing anything, since the message that caused data to be written to
	   * the pipe may have already been read. */
	  sem_post(&g_sockb->pipe_sem);
	}
	
	_cw_assert(bytes_read <= 1);
      }

      for (i = j = 0; (j < num_ready) && (i < (max_fd + 1)); i++)
      {
	if (FD_ISSET(i, &fd_read_set))
	{
	  const struct iovec * iovec;
	  int iovec_count;
	  ssize_t bytes_read;
	  cw_uint32_t max_read;
	  cw_bufel_t * bufel;
	  
	  j++;

	  /* Ready for reading. */

	  /* Figure out how much data we're willing to shove into this sock's
	   * incoming buffer. */
	  max_read = (sock_l_get_in_max_buf_size(socks[i])
		      - sock_l_get_in_size(socks[i]));

	  /* Build up buf_in to be at least large enough for the readv(). */
	  while (buf_get_size(&buf_in) < max_read)
	  {
	    bufel = sockb_get_spare_bufel();
	    buf_append_bufel(&buf_in, bufel);
	    bufel_delete(bufel);
	  }

	  /* Get an iovec for reading.  This somewhat goes against the idea of
	   * never writing the internals of a buf after the buffers have been
	   * inserted.  However, this is quite safe, since as a result of how we
	   * use buf_in, we know for sure that there are no other references to
	   * the byte ranges of the buffers we are writing to. */
	  iovec = buf_get_iovec(&buf_in, max_read, &iovec_count);

	  bytes_read = readv(i, iovec, iovec_count);
	  if (bytes_read >= 0)
	  {
	    _cw_assert(buf_get_size(&tmp_buf) == 0);
	    
	    buf_split(&tmp_buf, &buf_in, bytes_read);

	    /* Append to the sock's in_buf. */
	    sock_l_put_in_data(socks[i], &tmp_buf);
	    _cw_assert(buf_get_size(&tmp_buf) == 0);
	  }
	  else /* if (bytes_read == -1) */
	  {
	    /* readv() error. */
	    if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	    {
	      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			  "Error in readv(): %s\n", strerror(errno));
	    }
	  }
	}
	if (FD_ISSET(i, &fd_write_set))
	{
	  const struct iovec * iovec;
	  int iovec_count;
	  ssize_t bytes_written;
	  
	  j++;

	  /* Ready for writing. */

	  /* Get the socket's buf. */
	  _cw_assert(0 == buf_get_size(&tmp_buf));
	  sock_l_get_out_data(socks[i], &tmp_buf);

	  /* Build an iovec for writing. */
	  iovec = buf_get_iovec(&tmp_buf,
				buf_get_size(&tmp_buf),
				&iovec_count);

	  bytes_written = writev(i, iovec, iovec_count);

	  if (bytes_written >= 0)
	  {
	    buf_release_head_data(&tmp_buf, bytes_written);
	    
	    if (0 == sock_l_put_back_out_data(socks[i], &tmp_buf))
	    {
	      /* The socket has no more outgoing data, so turn the write bit
	       * off in the master write descriptor set. */
	      FD_CLR(i, &fd_m_write_set);
	    }
	  }
	  else /* if (bytes_written == -1) */
	  {
	    buf_release_head_data(&tmp_buf,
				  buf_get_size(&tmp_buf));
	    FD_CLR(i, &fd_m_write_set);
	    
	    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			"Error in writev(): %s\n", strerror(errno));
	    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			"Closing sockfd %d\n", i);
	    sock_l_error_callback(socks[i]);
	  }
	  _cw_assert(buf_get_size(&tmp_buf) == 0);
	}
#if (0)
	if (FD_ISSET(i, &fd_exception_set))
	{
	  j++;
	  _cw_error("Out of band data handling unimplemented");
	  /* XXX If we ever need to use out of band data, this must be
	   * implemented. */
	}
#endif
      }
    }
  }

  buf_delete(&buf_in);
  buf_delete(&tmp_buf);
  return NULL;
}

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * If no data has been written to the pipe that causes select() to return, write
 * a byte.  This function makes it possible to use a non-polling select loop.
 *
 ****************************************************************************/
static void
sockb_p_select_return(void)
{
  if (FALSE == sem_trywait(&g_sockb->pipe_sem))
  {
    if (-1 == write(g_sockb->pipe_in, "X", 1))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "Error in write(): %s\n", strerror(errno));
    }
  }
}
