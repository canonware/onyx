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

#include "libsock/libsock.h"

#include <sys/types.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "libsock/sockb_p.h"
#include "libsock/sockb_l.h"
#include "libsock/sock_l.h"

/* Global. */
cw_sockb_t * g_sockb = NULL;

cw_bool_t
sockb_init(cw_uint32_t a_max_fds, cw_uint32_t a_bufc_size,
	   cw_uint32_t a_max_spare_bufcs)
{
  cw_bool_t retval;
  struct cw_sockb_entry_s * arg;
  
  _cw_assert(a_bufc_size > 0);

  arg = (struct cw_sockb_entry_s *) _cw_malloc(sizeof(struct cw_sockb_entry_s));
  if (NULL == arg)
  {
    retval = TRUE;
    goto RETURN;
  }
  arg->max_fds = a_max_fds;
  arg->regs
    = (struct cw_sockb_reg_s *) _cw_calloc(a_max_fds,
					   sizeof(struct cw_sockb_reg_s));
  if (NULL == arg->regs)
  {
    _cw_free(arg);
    retval = TRUE;
    goto RETURN;
  }
  bzero(arg->regs, a_max_fds * sizeof(struct cw_sockb_reg_s));

  arg->fds = (struct pollfd *) _cw_calloc(a_max_fds,
					  sizeof(struct pollfd));
  if (NULL == arg->fds)
  {
    _cw_free(arg->regs);
    _cw_free(arg);
    retval = TRUE;
    goto RETURN;
  }
  bzero(arg->fds, a_max_fds * sizeof(struct pollfd));

  if (NULL == g_sockb)
  {
    g_sockb = (cw_sockb_t *) _cw_malloc(sizeof(cw_sockb_t));
    if (NULL == g_sockb)
    {
      _cw_free(arg->fds);
      _cw_free(arg->regs);
      _cw_free(arg);
      retval = TRUE;
      goto RETURN;
    }
    bzero(g_sockb, sizeof(cw_sockb_t));

    /* Ignore SIGPIPE, so that writing to a closed socket won't crash the
     * program. */
    signal(SIGPIPE, SIG_IGN);
  
    /* Create a pipe that will be used in conjunction with the message queues to
     * make the back end thread return from the poll() call. */
    {
      int filedes[2];

      if (-1 == pipe(filedes))
      {
	out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		  "Fatal error in pipe(): [s]\n", strerror(errno));
	abort();
      }
      g_sockb->pipe_out = filedes[0];
      g_sockb->pipe_in = filedes[1];

      /* Set g_sockb->pipe_in to non-blocking. */
      {
	int val;
	val = fcntl(g_sockb->pipe_in, F_GETFL, 0);
	if (val == -1)
	{
	  out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error for F_GETFL in fcntl(): [s]\n",
		    strerror(errno));
	  abort();
	}
	if (fcntl(g_sockb->pipe_in, F_SETFL, val | O_NONBLOCK))
	{
	  out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error for F_SETFL in fcntl(): [s]\n",
		    strerror(errno));
	  abort();
	}
      }
      {
	int val;
	val = fcntl(g_sockb->pipe_out, F_GETFL, 0);
	if (val == -1)
	{
	  out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error for F_GETFL in fcntl(): [s]\n",
		    strerror(errno));
	  abort();
	}
	if (fcntl(g_sockb->pipe_in, F_SETFL, val | O_NONBLOCK))
	{
	  out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		    "Fatal error for F_SETFL in fcntl(): [s]\n",
		    strerror(errno));
	  abort();
	}
      }
    }
  
    g_sockb->should_quit = FALSE;

    /* Create the semaphore used for determining whether data should be written
     * to the pipe in order to force a return from poll(). */
    sem_new(&g_sockb->pipe_sem, 1);

    /* Create the spare bufc pool and initialize associated variables. */
    if (NULL == pezz_new(&g_sockb->bufc_pool,
			 sizeof(cw_bufc_t),
			 (a_max_spare_bufcs) ? a_max_spare_bufcs : 1))
    {
      _cw_free(g_sockb);
      g_sockb = NULL;
      _cw_free(arg->fds);
      _cw_free(arg->regs);
      _cw_free(arg);
      retval = TRUE;
      goto RETURN;
    }
    if (NULL == pezz_new(&g_sockb->buffer_pool,
			 a_bufc_size,
			 (a_max_spare_bufcs) ? a_max_spare_bufcs : 1))
    {
      pezz_delete(&g_sockb->bufc_pool);
      _cw_free(g_sockb);
      g_sockb = NULL;
      _cw_free(arg->fds);
      _cw_free(arg->regs);
      _cw_free(arg);
      retval = TRUE;
      goto RETURN;
    }
  
    /* Create the message queues. */
    if (NULL == pezz_new(&g_sockb->messages_pezz,
			 sizeof(struct cw_sockb_msg_s), 8))
    {
      pezz_delete(&g_sockb->buffer_pool);
      pezz_delete(&g_sockb->bufc_pool);
      _cw_free(g_sockb);
      g_sockb = NULL;
      _cw_free(arg->fds);
      _cw_free(arg->regs);
      _cw_free(arg);
      retval = TRUE;
      goto RETURN;
    }

    mq_new(&g_sockb->messages);

    /* Create the lock used for protecting gethostbyname(). */
    mtx_new(&g_sockb->get_ip_addr_lock);

    /* Create a new thread to handle all of the back end socket foo. */
    thd_new(&g_sockb->thread, sockb_p_entry_func, (void *) arg);
  }

  retval = FALSE;

  RETURN:
  return retval;
}

void
sockb_shutdown(void)
{
  if (NULL != g_sockb)
  {
    /* Tell the back end thread to quit, then join on it. */
    g_sockb->should_quit = TRUE;
    sockb_l_wakeup();
    thd_join(&g_sockb->thread);

    sem_delete(&g_sockb->pipe_sem);
  
    /* Clean up the spare bufc's. */
    pezz_delete(&g_sockb->bufc_pool);
    pezz_delete(&g_sockb->buffer_pool);

    pezz_delete(&g_sockb->messages_pezz);
    mq_delete(&g_sockb->messages);

    /* Delete the gethostbyname() protection lock. */
    mtx_delete(&g_sockb->get_ip_addr_lock);
  
    _cw_free(g_sockb);
  }
}

cw_bufc_t *
sockb_get_spare_bufc(void)
{
  cw_bufc_t * retval;
  void * buffer;
  
  _cw_check_ptr(g_sockb);

  retval = bufc_new((cw_bufc_t *) _cw_pezz_get(&g_sockb->bufc_pool),
		    pezz_put,
		    (void *) &g_sockb->bufc_pool);
  if (NULL == retval)
  {
    retval = NULL;
    goto RETURN;
  }
  buffer = _cw_pezz_get(&g_sockb->buffer_pool);
  if (NULL == buffer)
  {
    bufc_delete(retval);
    retval = NULL;
    goto RETURN;
  }
  
  bufc_set_buffer(retval,
		  buffer,
		  pezz_get_buffer_size(&g_sockb->buffer_pool),
		  TRUE,
		  pezz_put,
		  (void *) &g_sockb->buffer_pool);

  RETURN:
  return retval;
}

cw_bool_t
sockb_in_notify(cw_mq_t * a_mq, int a_sockfd)
{
  cw_bool_t retval;
  struct cw_sockb_msg_s * message;
  cw_mtx_t mtx;
  cw_cnd_t cnd;
  
  _cw_check_ptr(g_sockb);
  _cw_assert(a_sockfd >= 0);

  mtx_new(&mtx);
  cnd_new(&cnd);
  
  message = (struct cw_sockb_msg_s *) _cw_pezz_get(&g_sockb->messages_pezz);
  if (NULL == message)
  {
    retval = TRUE;
    goto RETURN;
  }

#ifdef _LIBSOCK_DBG
  message->magic = _LIBSOCK_SOCKB_MSG_MAGIC;
#endif
  message->type = IN_NOTIFY;
  message->data.in_notify.sockfd = a_sockfd;
  message->data.in_notify.mq = a_mq;
  message->data.in_notify.mtx = &mtx;
  message->data.in_notify.cnd = &cnd;

  mtx_lock(&mtx);
  if (0 != mq_put(&g_sockb->messages, (const void *) message))
  {
    _cw_pezz_put(&g_sockb->messages_pezz, (void *) message);
    retval = TRUE;
    goto RETURN;
  }
  
  sockb_l_wakeup();

  cnd_wait(&cnd, &mtx);
  mtx_unlock(&mtx);
  
  retval = FALSE;

  RETURN:
  mtx_delete(&mtx);
  cnd_delete(&cnd);
  return retval;
}

void
sockb_l_wakeup(void)
{
  if (FALSE == sem_trywait(&g_sockb->pipe_sem))
  {
    if (-1 == write(g_sockb->pipe_in, "X", 1))
    {
      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
      {
	out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		  "Error in write(): [s]\n", strerror(errno));
      }
    }
  }
}

cw_bool_t
sockb_l_register_sock(cw_sock_t * a_sock)
{
  cw_bool_t retval;
  struct cw_sockb_msg_s * message;
  
  _cw_check_ptr(a_sock);
  _cw_check_ptr(g_sockb);
  _cw_assert(sock_get_fd(a_sock) >= 0);

  message = (struct cw_sockb_msg_s *) _cw_pezz_get(&g_sockb->messages_pezz);
  if (NULL == message)
  {
    retval = TRUE;
    goto RETURN;
  }

#ifdef _LIBSOCK_DBG
  message->magic = _LIBSOCK_SOCKB_MSG_MAGIC;
#endif
  message->type = REGISTER;
  message->data.sock = a_sock;
  if (0 != mq_put(&g_sockb->messages, (const void *) message))
  {
    _cw_pezz_put(&g_sockb->messages_pezz, (void *) message);
    retval =TRUE;
    goto RETURN;
  }
  
  sockb_l_wakeup();

  retval = FALSE;

  RETURN:
  return retval;
}

cw_bool_t
sockb_l_unregister_sock(cw_uint32_t a_sockfd)
{
  cw_bool_t retval;
  struct cw_sockb_msg_s * message;

  _cw_check_ptr(g_sockb);

  message = (struct cw_sockb_msg_s *) _cw_pezz_get(&g_sockb->messages_pezz);
  if (NULL == message)
  {
    retval = TRUE;
    goto RETURN;
  }
  
#ifdef _LIBSOCK_DBG
  message->magic = _LIBSOCK_SOCKB_MSG_MAGIC;
#endif
  message->type = UNREGISTER;
  message->data.sockfd = a_sockfd;
  if (0 != mq_put(&g_sockb->messages, (const void *) message))
  {
    _cw_pezz_put(&g_sockb->messages_pezz, (void *) message);
    retval =TRUE;
    goto RETURN;
  }
  
  sockb_l_wakeup();

  retval = FALSE;

  RETURN:
  return retval;
}

cw_bool_t
sockb_l_out_notify(cw_uint32_t a_sockfd)
{
  cw_bool_t retval;
  struct cw_sockb_msg_s * message;

  _cw_check_ptr(g_sockb);

  message = (struct cw_sockb_msg_s *) _cw_pezz_get(&g_sockb->messages_pezz);
  if (NULL == message)
  {
    retval = TRUE;
    goto RETURN;
  }
  
#ifdef _LIBSOCK_DBG
  message->magic = _LIBSOCK_SOCKB_MSG_MAGIC;
#endif
  message->type = OUT_NOTIFY;
  message->data.sockfd = a_sockfd;
  if (0 != mq_put(&g_sockb->messages, (const void *) message))
  {
    _cw_pezz_put(&g_sockb->messages_pezz, (void *) message);
    retval =TRUE;
    goto RETURN;
  }
  
  sockb_l_wakeup();

  retval = FALSE;

  RETURN:
  return retval;
}

cw_bool_t
sockb_l_in_space(cw_uint32_t a_sockfd)
{
  cw_bool_t retval;
  struct cw_sockb_msg_s * message;

  _cw_check_ptr(g_sockb);

  message = (struct cw_sockb_msg_s *) _cw_pezz_get(&g_sockb->messages_pezz);
  if (NULL == message)
  {
    retval = TRUE;
    goto RETURN;
  }
  
#ifdef _LIBSOCK_DBG
  message->magic = _LIBSOCK_SOCKB_MSG_MAGIC;
#endif
  message->type = IN_SPACE;
  message->data.sockfd = a_sockfd;
  if (0 != mq_put(&g_sockb->messages, (const void *) message))
  {
    _cw_pezz_put(&g_sockb->messages_pezz, (void *) message);
    retval =TRUE;
    goto RETURN;
  }
  
  sockb_l_wakeup();

  retval = FALSE;

  RETURN:
  return retval;
}

cw_bool_t
sockb_l_get_host_ip(const char * a_host_str, cw_uint32_t * r_host_ip)
{
  cw_bool_t retval;
  cw_uint32_t host_ip;

  _cw_check_ptr(g_sockb);
    
  host_ip = inet_addr(a_host_str);

#ifdef _CW_OS_SOLARIS
  if ((in_addr_t) host_ip == (in_addr_t) -1)
#else 
  if (host_ip == INADDR_NONE)
#endif
  {
    struct hostent * host_entry;
    
    /* Not a dotted number IP address.  Let's try it as a hostname. */
    
    /* Lock the mutex first. */
    mtx_lock(&g_sockb->get_ip_addr_lock);
    
    host_entry = gethostbyname(a_host_str);
    if (host_entry == NULL)
    {
      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
      {
#ifdef _CW_OS_SOLARIS
	out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		  "Error in gethostbyname(): [i]\n", h_errno);
#else
	out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		  "Error in gethostbyname(): [s]\n", hstrerror(h_errno));
#endif
	out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		  "Host \"[s]\" isn't an IP address or a hostname\n",
		  a_host_str);
      }
      retval = TRUE;
    }
    else
    {
      struct in_addr * addr_ptr;
      
      _cw_assert(host_entry->h_addrtype == AF_INET);
      _cw_assert(host_entry->h_addr_list[0] != NULL);

      addr_ptr = (struct in_addr *) host_entry->h_addr_list[0];
      *r_host_ip = addr_ptr->s_addr;

      retval = FALSE;
    }
    /* We're done with h_errno and the global structure pointed to by
     * host_entry, so unlock the mutex now. */
    mtx_unlock(&g_sockb->get_ip_addr_lock);
  }
  else
  {
    *r_host_ip = host_ip;
    retval = FALSE;
  }

  if (dbg_is_registered(cw_g_dbg, "sockb_verbose"))
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "IP address: [i].[i].[i].[i]\n",
	      *r_host_ip & 0xff,
	      (*r_host_ip >> 8) & 0xff,
	      (*r_host_ip >> 16) & 0xff,
	      *r_host_ip >> 24);
  }

  return retval;
}

static cw_bool_t
sockb_p_notify(cw_mq_t * a_mq, int a_sockfd)
{
  cw_bool_t retval;
  cw_sint32_t error;
  
  _cw_check_ptr(a_mq);

  while (-1 == (error = mq_put(a_mq, (void *) a_sockfd)))
  {
    /* We can't afford to lose the message, since it could end up causing
     * deadlock. */
    if (dbg_is_registered(cw_g_dbg, "sockb_error"))
    {
      out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		"Memory allocation error; yielding\n");
    }
    thd_yield();
  }
  
  if (1 == error)
  {
    /* The mq is in the stop state.  We might as well stop notifying. */
    retval = TRUE;
    goto RETURN;
  }

  retval = FALSE;
#ifdef _LIBSOCK_CONFESS
  out_put(cw_g_out, "n");
#endif

  RETURN:
  return retval;
}

static void *
sockb_p_entry_func(void * a_arg)
{
  struct cw_sockb_entry_s * arg = (struct cw_sockb_entry_s *) a_arg;
  cw_uint32_t max_fds = arg->max_fds;
  struct cw_sockb_reg_s * regs = arg->regs;
  struct pollfd * fds = arg->fds;
  unsigned nfds;
  cw_sock_t * sock;
  int sockfd, num_ready;
  cw_buf_t tmp_buf, buf_in;
  struct cw_sockb_msg_s * message;

  _cw_free(a_arg);

  /* Initialize data structures. */
  buf_new(&tmp_buf);
  buf_new(&buf_in);
  {
    cw_uint32_t i;

    for (i = 0; i < max_fds; i++)
    {
      regs[i].pollfd_pos = -1;
    }
  }

  /* Add g_sockb->pipe_out for readingg, so that this thread will return from
   * poll() when data is written to g_sockb->pipe_in. */
  fds[0].fd = g_sockb->pipe_out;
  fds[0].events = POLLIN;
  nfds = 1;

  while (g_sockb->should_quit == FALSE)
  {
    /* Check for messages in the message queues. */
    while (NULL != (message
		    = (struct cw_sockb_msg_s *) mq_tryget(&g_sockb->messages)))
    {
      switch (message->type)
      {
	case REGISTER:
	{
	  sock = message->data.sock;
      
	  sockfd = sock_get_fd(sock);
	  if (max_fds <= sockfd)
	  {
	    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		      "Maximum file descriptor exceeded ([i] <= [i])\n",
		      max_fds, sockfd);
	    abort();
	  }
	  
	  if ((-1 == regs[sockfd].pollfd_pos) && (max_fds > nfds))
	  {
	    /* The sock isn't registered.  Register it. */
	    regs[sockfd].sock = sock;
	    regs[sockfd].pollfd_pos = nfds;
	    regs[sockfd].notify_mq = NULL;

	    fds[nfds].fd = sockfd;
	    /* If the buffer size is 0, don't ever try to read on this
	     * descriptor. */
	    if (0 != sock_l_get_in_max_buf_size(sock))
	    {
#ifdef _LIBSOCK_CONFESS
	            out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
			      "Register [i] ([i] byte input buffer)\n",
			      sockfd, sock_l_get_in_max_buf_size(sock));
#endif
	      fds[nfds].events = POLLIN;
	    }
	    else
	    {
#ifdef _LIBSOCK_CONFESS
	      out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
			"Register [i](no in buffer)\n", sockfd);
#endif
	      fds[nfds].events = 0;
	    }
	    nfds++;

	    /* Notify the sock that it's registered. */
	    sock_l_message_callback(sock, FALSE);
	  }
	  else
	  {
#ifdef _LIBSOCK_CONFESS
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "Refuse to register [i]\n", sockfd);
#endif
	    
	    sock_l_message_callback(sock, TRUE);
	  }

	  break;
	}
	case UNREGISTER:
	{
	  sockfd = message->data.sockfd;

	  if (-1 != regs[sockfd].pollfd_pos)
	  {
#ifdef _LIBSOCK_CONFESS
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "Unregister [i]\n", sockfd);
#endif

	    nfds--;
	    if (regs[sockfd].pollfd_pos != nfds)
	    {
#ifdef _LIBSOCK_CONFESS
	      out_put(cw_g_out, "h([i]-->[i])", nfds, regs[sockfd].pollfd_pos);
#endif

	      regs[fds[nfds].fd].pollfd_pos = regs[sockfd].pollfd_pos;
	      memcpy(&fds[regs[sockfd].pollfd_pos], &fds[nfds],
		     sizeof(struct pollfd));
	    }
	    regs[sockfd].pollfd_pos = -1;

	    /* If this sock has a notification mq associated with it, send a
	     * final message, then deactivate notifications. */
	    if (NULL != regs[sockfd].notify_mq)
	    {
	      sockb_p_notify(regs[sockfd].notify_mq, sockfd);
	      regs[sockfd].notify_mq = NULL;
	    }
	  }
#ifdef _LIBSOCK_CONFESS
	  else
	  {
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "Refuse to unregister [i]\n", sockfd);
	  }
#endif      

	  /* Notify the sock that it's unregistered. */
	  sock_l_message_callback(regs[sockfd].sock, FALSE);
	  regs[sockfd].sock = NULL;
	  break;
	}
	case OUT_NOTIFY:
	{
	  sockfd = message->data.sockfd;

	  if (-1 != regs[sockfd].pollfd_pos)
	  {
#ifdef _LIBSOCK_CONFESS
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "Set [i]w\n", sockfd);
#endif

	    fds[regs[sockfd].pollfd_pos].events |= POLLOUT;
	  }
#ifdef _LIBSOCK_CONFESS
	  else
	  {
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "Refuse to set [i]w\n", sockfd);
	  }
#endif
	  break;
	}
	case IN_SPACE:
	{
	  sockfd = message->data.sockfd;

	  if (-1 != regs[sockfd].pollfd_pos)
	  {
#ifdef _LIBSOCK_CONFESS
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "Set [i]r\n", sockfd);
#endif

	    fds[regs[sockfd].pollfd_pos].events |= POLLIN;
	  }
#ifdef _LIBSOCK_CONFESS
	  else
	  {
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "Refuse to set [i]r\n", sockfd);
	  }
#endif
	  break;
	}
	case IN_NOTIFY:
	{
	  sockfd = message->data.in_notify.sockfd;

	  if (-1 != regs[sockfd].pollfd_pos)
	  {
	    _cw_check_ptr(regs[sockfd].sock);
	    regs[sockfd].notify_mq = message->data.in_notify.mq;
#ifdef _LIBSOCK_CONFESS
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "regs[[[i]].notify_mq = 0x[p]\n",
		      sockfd,
		      regs[sockfd].notify_mq);
#endif

	    mtx_lock(message->data.in_notify.mtx);
	    cnd_signal(message->data.in_notify.cnd);
	    mtx_unlock(message->data.in_notify.mtx);

	    if (NULL != regs[sockfd].notify_mq)
	    {
	      if (0 < sock_l_get_in_size(regs[sockfd].sock))
	      {
		/* Send an out notification, since there are data already queued
		 * up. */
		if (TRUE == sockb_p_notify(regs[sockfd].notify_mq, sockfd))
		{
		  regs[sockfd].notify_mq = NULL;
		}
	      }
	    }
	  }
	  else
	  {
	    mtx_lock(message->data.in_notify.mtx);
	    cnd_signal(message->data.in_notify.cnd);
	    mtx_unlock(message->data.in_notify.mtx);

	    /* Send a message here to avoid a race condition where a sock is
	     * unregistered due to a failed readv()/writev(), but the user
	     * hasn't realized this yet, since no sock_*() calls have been made
	     * since the unregistration. */
	    if (NULL != message->data.in_notify.mq)
	    {
	      sockb_p_notify(message->data.in_notify.mq, sockfd);
	    }
	    
#ifdef _LIBSOCK_CONFESS
	    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		      "Refuse to set regs[[[i]].notify_mq = 0x[p]\n",
		      sockfd,
		      message->data.in_notify.mq);
#endif
	  }

	  break;
	}
	default:
	{
	  _cw_error("Programming error");
	}
      }
#ifdef _LIBSOCK_DBG
      message->magic = 0;
#endif
      _cw_pezz_put(&g_sockb->messages_pezz, (void *) message);
    }
    
#ifdef _LIBSOCK_CONFESS
    {
      cw_uint32_t i;

      out_put_e(cw_g_out, __FILE__, __LINE__, NULL, "poll fd's:");
      for (i = 1; i < nfds; i++)
      {
	sockfd = fds[i].fd;
	
	out_put(cw_g_out, " [i]R", sockfd);
	  
	if (fds[i].events & POLLIN)
	{
	  out_put(cw_g_out, "r");
	}
	if (fds[i].events & POLLOUT)
	{
	  out_put(cw_g_out, "w");
	}
      }
      out_put(cw_g_out, " ([i][s])\n",
	      fds[0].fd,
	      (fds[0].events & POLLIN) ? "r" : "");
    }
    
    out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
	      "poll([i])", nfds);
#endif

    num_ready = poll(fds, nfds, -1);
    
#ifdef _LIBSOCK_CONFESS
    out_put(cw_g_out, "-->([i|s:s])\n", num_ready);
#endif
    
    if (-1 == num_ready)
    {
      if (errno != EINTR)
      {
	/* This is an error that should never happen. */
	out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		  "Fatal error in poll(): [s]\n", strerror(errno));
	abort();
      }
    }
    else
    {
	cw_sint32_t i, j;
      
#ifdef _LIBSOCK_CONFESS
      out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
		"Check fd:");
#endif
      
      /* Ready descriptors. */
      if (fds[0].revents & POLLIN)
      {
	char t_buf[2];
	ssize_t bytes_read;

#ifdef _LIBSOCK_CONFESS
	out_put(cw_g_out, " ([i|s:s]r)", g_sockb->pipe_out);
#endif
	
	/* Decrement the number of fd's that need handled in the loop below. */
	num_ready--;

	/* Read the data out of the pipe so that the next call doesn't
	 * immediately return just because of data already in the pipe.  Note
	 * that there is no risk of deadlock due to emptying data from the pipe
	 * that is written after the poll() call, since the message queues are
	 * checked after emptying the pipe, but before calling poll()
	 * again. */
	bytes_read = read(g_sockb->pipe_out, t_buf, 2);
	if (bytes_read == -1)
	{
	  if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	  {
	    out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		      "Error in read(): [s]\n", strerror(errno));
	  }
	}
	else if (bytes_read > 0)
	{
	  /* Set the semaphore to one.  This will cause one, and only one byte
	   * to be written to g_sockb->pipe_in and cause a return from poll() if
	   * one or more messages needs handled.  Note that we must post the
	   * semophore before handling the message queues, since it is possible
	   * to have new messages come in and miss them otherwise.  Posting
	   * first means that we may execute the poll() loop once without
	   * doing anything, since the message that caused data to be written to
	   * the pipe may have already been read. */
	  sem_post(&g_sockb->pipe_sem);
	}
	
	_cw_assert(bytes_read <= 1);
      }

      for (i = 1, j = 0; (j < num_ready) && (i < nfds); i++)
      {
	sockfd = fds[i].fd;
	
#ifdef _LIBSOCK_CONFESS
	out_put(cw_g_out, " [i][s][s][s][s][s][s][s][s][s][s]",
		sockfd,
#ifdef POLLIN
		fds[i].revents & POLLIN ? "<IN>" :
#endif
		"",
#ifdef POLLRDNORM
		fds[i].revents & POLLRDNORM ? "<RDNORM>" :
#endif
		"",
#ifdef POLLRDBAND
		fds[i].revents & POLLRDBAND ? "<RDBAND>" :
#endif
		"",
#ifdef POLLPRI
		fds[i].revents & POLLPRI ? "<PRI>" :
#endif
		"",
#ifdef POLLOUT
		fds[i].revents & POLLOUT ? "<OUT>" :
#endif
		"",
#ifdef POLLWRNORM
		fds[i].revents & POLLWRNORM ? "<WRNORM>" :
#endif
		"",
#ifdef POLLWRBAND
		fds[i].revents & POLLWRBAND ? "<WRBAND>" :
#endif
		"",
#ifdef POLLERR
		fds[i].revents & POLLERR ? "<ERR>" :
#endif
		"",
#ifdef POLLHUP
		fds[i].revents & POLLHUP ? "<HUP>" :
#endif
		"",
#ifdef POLLNVAL
		fds[i].revents & POLLNVAL ? "<NVAL>" :
#endif
		"",
#ifdef POLLMSG
		/* Linux-specific. */
		fds[i].revents & POLLMSG ? "<MSG>" :
#endif
		"");
#endif

	if (fds[i].revents & POLLIN)
	{
	  const struct iovec * iov;
	  int iov_cnt;
	  ssize_t bytes_read;
	  cw_sint32_t max_read;
	  cw_bufc_t * bufc;
	  
	  j++;

#ifdef _LIBSOCK_CONFESS
	  out_put(cw_g_out, "r");
#endif
	  /* Ready for reading. */

	  /* Figure out how much data we're willing to shove into this sock's
	   * incoming buffer. */
	  max_read = sock_l_get_in_space(regs[sockfd].sock);
	  
	  /* Build up buf_in to be at least large enough for the readv(). */
	  while (buf_get_size(&buf_in) < max_read)
	  {
	    while (NULL == (bufc = sockb_get_spare_bufc()))
	    {
	      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	      {
		out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			  "Memory allocation error; yielding\n");
	      }
	      thd_yield();
	    }
	    
	    while (TRUE == buf_append_bufc(&buf_in, bufc, 0,
					   pezz_get_buffer_size(
					     &g_sockb->buffer_pool)))
	    {
	      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	      {
		out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			  "Memory allocation error; yielding\n");
	      }
	      thd_yield();
	    }
	    bufc_delete(bufc);
	  }
	  
	  /* Get an iovec for reading.  This somewhat goes against the idea of
	   * never writing the internals of a buf after the buffers have been
	   * inserted.  However, this is quite safe, since as a result of how we
	   * use buf_in, we know for sure that there are no other references to
	   * the byte ranges of the buffers we are writing to. */
	  iov = buf_get_iovec(&buf_in, max_read, TRUE, &iov_cnt);

	  bytes_read = readv(sockfd, iov, iov_cnt);
#ifdef _LIBSOCK_CONFESS
	  out_put(cw_g_out, "([i|s:s])", bytes_read);
#endif

	  if (0 < bytes_read)
	  {
	    cw_uint32_t in_buf_free;
	    
	    _cw_assert(buf_get_size(&tmp_buf) == 0);

	    while (TRUE == buf_split(&tmp_buf, &buf_in, bytes_read))
	    {
	      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	      {
		out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			  "Memory allocation error; yielding\n");
	      }
	      thd_yield();
	    }
	    
	    /* Append to the sock's in_buf. */
	    in_buf_free = sock_l_put_in_data(regs[sockfd].sock, &tmp_buf);
	    if (0 == in_buf_free)
	    {
	      /* Turn off the read bit for this sock.  The sock will send a
	       * message when there is once again space. */
#ifdef _LIBSOCK_CONFESS
	      out_put(cw_g_out, "u");
#endif
	      fds[i].events ^= (fds[i].events & POLLIN);
	    }

	    /* Only send a message if the sock buffer was empty before we put
	     * data in it. */
	    if (0 == (sock_l_get_in_max_buf_size(regs[sockfd].sock)
		      - (in_buf_free + bytes_read)))
	    {
	      if (NULL != regs[sockfd].notify_mq)
	      {
		if (TRUE == sockb_p_notify(regs[sockfd].notify_mq, sockfd))
		{
		  regs[sockfd].notify_mq = NULL;
		}
	      }
	    }
	    _cw_assert(buf_get_size(&tmp_buf) == 0);
	  }
	  else if (0 == bytes_read)
	  {
	    /* readv() error. */
	    if (dbg_is_registered(cw_g_dbg, "sockb_verbose"))
	    {
	      out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			"EOF in readv().  Closing sockfd [i]\n", sockfd);
	    }

	    /* Fill this hole, decrement i, continue. */
	    nfds--;
	    if (regs[sockfd].pollfd_pos != nfds)
	    {
#ifdef _LIBSOCK_CONFESS
	      out_put(cw_g_out, "h([i]-->[i])", nfds, i);
#endif
	      
	      regs[fds[nfds].fd].pollfd_pos = i;
	      memcpy(&fds[i], &fds[nfds], sizeof(struct pollfd));
	      i--;
	    }
	    regs[sockfd].pollfd_pos = -1;

	    sock_l_error_callback(regs[sockfd].sock);
	    
	    if (NULL != regs[sockfd].notify_mq)
	    {
	      if (TRUE == sockb_p_notify(regs[sockfd].notify_mq, sockfd))
	      {
		regs[sockfd].notify_mq = NULL;
	      }
	    }

#ifdef _LIBSOCK_CONFESS
	    out_put(cw_g_out, "\n");
#endif
	    continue;
	  }
	  else /*  if (bytes_read == -1) */
	  {
	    /* readv() error. */
	    if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	    {
	      out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			"Error in readv(): [s]\n", strerror(errno));
	    }
	  }
	}
	else if (fds[i].revents & POLLHUP)
	{
	  const struct iovec * iov;
	  int iov_cnt;
	  ssize_t bytes_read;
	  cw_bufc_t * bufc = NULL;
	  cw_uint32_t buffer_size;

	  j++;
	  
#ifdef _LIBSOCK_CONFESS
	  out_put(cw_g_out, "r(");
#endif
	  
	  /* It may be that there are buffered data to be read, even though the
	   * descriptor has been closed.  If we succeed in reading data, there
	   * is no choice but to buffer all the read data in the sock's input
	   * buffer, even if it overfills the buffer.  We must keep reading
	   * until there are no more data, since this is the last time that this
	   * descriptor will be paid any attention. */

	  buffer_size = pezz_get_buffer_size(&g_sockb->buffer_pool);

	  do
	  {
	    /* Add some more space to &buf_in if necessary. */
	    if (0 == buf_get_size(&buf_in))
	    {
	      while (NULL == (bufc = sockb_get_spare_bufc()))
	      {
		if (dbg_is_registered(cw_g_dbg, "sockb_error"))
		{
		  out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			    "Memory allocation error; yielding\n");
		}
		thd_yield();
	      }

	      while (TRUE == buf_append_bufc(&buf_in, bufc, 0, buffer_size))
	      {
		if (dbg_is_registered(cw_g_dbg, "sockb_error"))
		{
		  out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			    "Memory allocation error; yielding\n");
		}
		thd_yield();
	      }
	      /* Drop our reference. */
	      bufc_delete(bufc);
	    }

	    iov = buf_get_iovec(&buf_in, buffer_size, TRUE, &iov_cnt);

	    bytes_read = readv(sockfd, iov, iov_cnt);

#ifdef _LIBSOCK_CONFESS
	    out_put(cw_g_out, "[i|s:s][s]",
		    bytes_read, (0 < bytes_read) ? ", " : ")");
#endif
	    _cw_assert(buf_get_size(&tmp_buf) == 0);

	    while (TRUE == buf_split(&tmp_buf, &buf_in, bytes_read))
	    {
	      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	      {
		out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			  "Memory allocation error; yielding\n");
	      }
	      thd_yield();
	    }
	    sock_l_put_in_data(regs[sockfd].sock, &tmp_buf);
	  } while (0 < bytes_read);

#ifdef _LIBSOCK_CONFESS
	  out_put(cw_g_out, "c");
#endif

	  if (dbg_is_registered(cw_g_dbg, "sockb_verbose"))
	  {
	    out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
		      "POLLHUP.  Closing sockfd [i]\n", sockfd);
	  }

	  /* Fill this hole, decrement i, continue. */
	  nfds--;
	  if (regs[sockfd].pollfd_pos != nfds)
	  {
#ifdef _LIBSOCK_CONFESS
	    out_put(cw_g_out, "h([i]-->[i])", nfds, i);
#endif
	      
	    regs[fds[nfds].fd].pollfd_pos = i;
	    memcpy(&fds[i], &fds[nfds], sizeof(struct pollfd));
	    i--;
	  }
	  regs[sockfd].pollfd_pos = -1;

	  sock_l_error_callback(regs[sockfd].sock);
	    
	  if (NULL != regs[sockfd].notify_mq)
	  {
	    if (TRUE == sockb_p_notify(regs[sockfd].notify_mq, sockfd))
	    {
	      regs[sockfd].notify_mq = NULL;
	    }
	  }

#ifdef _LIBSOCK_CONFESS
	  out_put(cw_g_out, "\n");
#endif
	  continue;
	}
	if (fds[i].revents & POLLOUT)
	{
	  const struct iovec * iov;
	  int iov_cnt;
	  ssize_t bytes_written;
	  
	  j++;

#ifdef _LIBSOCK_CONFESS
	  out_put(cw_g_out, "w");
#endif	  
	  /* Ready for writing. */

	  /* Get the socket's buf. */
	  _cw_assert(0 == buf_get_size(&tmp_buf));
	  sock_l_get_out_data(regs[sockfd].sock, &tmp_buf);

	  /* Build an iovec for writing. */
	  iov = buf_get_iovec(&tmp_buf,
			      buf_get_size(&tmp_buf),
			      TRUE,
			      &iov_cnt);

	  /* If the buf exceeds the maximum iovec, it's possible that we'll only
	   * write part of the data, when we could have written it all.  This is
	   * in practice very unlikely though, and doesn't cause erroneous
	   * behavior. */
	  bytes_written = writev(sockfd, iov, iov_cnt);
#ifdef _LIBSOCK_CONFESS
	  out_put(cw_g_out, "([i|s:s]/[i])", bytes_written,
		  buf_get_size(&tmp_buf));
#endif

	  if (bytes_written >= 0)
	  {
	    buf_release_head_data(&tmp_buf, bytes_written);
	    
	    if (0 == sock_l_put_back_out_data(regs[sockfd].sock, &tmp_buf))
	    {
	      /* The socket has no more outgoing data, so turn the write bit
	       * off in the master write descriptor set. */
#ifdef _LIBSOCK_CONFESS
	      out_put(cw_g_out, "u");
#endif
	      fds[i].events ^= (fds[i].events & POLLOUT);
	    }
#ifdef _LIBSOCK_CONFESS
	    else
	    {
	      out_put(cw_g_out, "i");
	    }
#endif
	  }
	  else /* if (bytes_written == -1) */
	  {
	    buf_release_head_data(&tmp_buf,
				  buf_get_size(&tmp_buf));
	    
	    if (dbg_is_registered(cw_g_dbg, "sockb_verbose"))
	    {
	      out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			"Error in writev(): [s]\n", strerror(errno));
	      out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
			"Closing sockfd [i]\n", sockfd);
	    }
	    
	    /* Fill this hole, decrement i. */
	    nfds--;
	    if (regs[sockfd].pollfd_pos != nfds)
	    {
#ifdef _LIBSOCK_CONFESS
	      out_put(cw_g_out, "h([i]-->[i])", nfds, i);
#endif
	      regs[fds[nfds].fd].pollfd_pos = i;
	      memcpy(&fds[i], &fds[nfds], sizeof(struct pollfd));
	      i--;
	    }
	    regs[sockfd].pollfd_pos = -1;

	    sock_l_error_callback(regs[sockfd].sock);
	    
	    if (NULL != regs[sockfd].notify_mq)
	    {
	      if (TRUE == sockb_p_notify(regs[sockfd].notify_mq, sockfd))
	      {
		regs[sockfd].notify_mq = NULL;
	      }
	    }
	  }
	  _cw_assert(buf_get_size(&tmp_buf) == 0);
	}
      }
#ifdef _LIBSOCK_CONFESS
      out_put(cw_g_out, "\n");
#endif
    }
  }

  buf_delete(&buf_in);
  buf_delete(&tmp_buf);
  _cw_free(regs);
  _cw_free(fds);
  return NULL;
}
