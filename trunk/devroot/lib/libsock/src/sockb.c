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
 * Implementation of the sockb class.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_PEZZ
#define _LIBSTASH_USE_MQ
#include "libstash/libstash_r.h"

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
#include <signal.h>

#include "libsock/sockb_p.h"
#include "libsock/sockb_l.h"
#include "libsock/sock_l.h"

/* Global. */
cw_sockb_t * g_sockb = NULL;

/*  #define _LIBSTASH_SOCKB_CONFESS */

cw_bool_t
sockb_init(cw_uint32_t a_bufel_size, cw_uint32_t a_max_spare_bufels)
{
  cw_bool_t retval;
  char * tmpfile_name, buf[L_tmpnam];
  
  _cw_assert(a_bufel_size > 0);

  if (NULL == g_sockb)
  {
    g_sockb = (cw_sockb_t *) _cw_malloc(sizeof(cw_sockb_t));
    if (NULL == g_sockb)
    {
      retval = TRUE;
      goto RETURN;
    }
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
      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Error in unlink(): %s\n", strerror(errno));
      }
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

    /* Create the semaphore used for determining whether data should be written
     * to the pipe in order to force a return from select(). */
    sem_new(&g_sockb->pipe_sem, 1);

    /* Create the spare bufc pool and initialize associated variables. */
    if (NULL == pezz_new(&g_sockb->bufc_pool,
			 sizeof(cw_bufc_t), a_max_spare_bufels))
    {
      _cw_free(g_sockb);
      g_sockb = NULL;
      retval = TRUE;
      goto RETURN;
    }
    if (NULL == pezz_new(&g_sockb->buffer_pool,
			 a_bufel_size, a_max_spare_bufels))
    {
      pezz_delete(&g_sockb->bufc_pool);
      _cw_free(g_sockb);
      g_sockb = NULL;
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
      retval = TRUE;
      goto RETURN;
    }
      
    mq_new(&g_sockb->messages);

    /* Create the lock used for protecting gethostbyaddr(). */
    mtx_new(&g_sockb->get_ip_addr_lock);

    /* Create a new thread to handle all of the back end socket foo. */
    thd_new(&g_sockb->thread, sockb_p_entry_func, NULL);
  }

  retval = FALSE;

  RETURN:
  return retval;
}

void
sockb_shutdown(void)
{
  _cw_check_ptr(g_sockb);

  /* Tell the back end thread to quit, then join on it. */
  g_sockb->should_quit = TRUE;
  sockb_l_wakeup();
  thd_join(&g_sockb->thread);
  thd_delete(&g_sockb->thread);

  sem_delete(&g_sockb->pipe_sem);
  
  /* Clean up the spare bufc's. */
  pezz_delete(&g_sockb->bufc_pool);
  pezz_delete(&g_sockb->buffer_pool);

  pezz_delete(&g_sockb->messages_pezz);
  mq_delete(&g_sockb->messages);

  if (close(g_sockb->poser_fd))
  {
    if (dbg_is_registered(cw_g_dbg, "sockb_error"))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "Error in close(): %s\n", strerror(errno));
    }
  }

  /* Delete the gethostbyaddr() protection lock. */
  mtx_delete(&g_sockb->get_ip_addr_lock);
  
  _cw_free(g_sockb);
}

cw_bufc_t *
sockb_get_spare_bufc(void)
{
  cw_bufc_t * retval;
  void * buffer;
  
  _cw_check_ptr(g_sockb);

  retval = bufc_new((cw_bufc_t *) pezz_get(&g_sockb->bufc_pool),
		  pezz_put,
		  (void *) &g_sockb->bufc_pool);
  if (NULL == retval)
  {
    retval = NULL;
    goto RETURN;
  }
  buffer = pezz_get(&g_sockb->buffer_pool);
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
sockb_wait(int * a_sock_vec, cw_uint32_t a_vec_len,
	   struct timespec * a_timeout)
{
  cw_bool_t retval;
  struct timeval now;
  struct timespec tout;
  struct timezone tz;
  struct cw_sockb_msg_s msg_wait;
  cw_cnd_t cnd;
  cw_mtx_t mtx;
  cw_bool_t did_timeout;

  _cw_check_ptr(a_sock_vec);
  
  cnd_new(&cnd);
  mtx_new(&mtx);

#ifdef _LIBSOCK_DBG
  msg_wait.magic = _LIBSOCK_SOCKB_MSG_MAGIC;
#endif
  msg_wait.type = WAIT;
  msg_wait.data.wait.cnd = &cnd;
  msg_wait.data.wait.mtx = &mtx;
  msg_wait.data.wait.fd_array = a_sock_vec;
  msg_wait.data.wait.nfds = a_vec_len;

  mtx_lock(&mtx);
  if (0 != mq_put(&g_sockb->messages, (const void *) &msg_wait))
  {
    retval = TRUE;
    goto RETURN;
  }
  sockb_l_wakeup();

  if (NULL != a_timeout)
  {
    /* Set timeout for 1 second. */
    bzero(&tz, sizeof(struct timezone));
    gettimeofday(&now, &tz);
    tout.tv_sec = now.tv_sec + a_timeout->tv_sec;
    tout.tv_nsec = now.tv_usec * 1000 + a_timeout->tv_nsec;

    did_timeout = cnd_timedwait(&cnd, &mtx, &tout);
  }
  else
  {
    did_timeout = FALSE;
    cnd_wait(&cnd, &mtx);
  }
  mtx_unlock(&mtx);
  
  if (FALSE == did_timeout)
  {
    /* We got something! */
    retval = FALSE;
  }
  else
  {
    struct cw_sockb_msg_s msg_unwait;
    cw_cnd_t cnd;
    cw_mtx_t mtx;

    /* Timeout.  Now send an UNWAIT message to clean things up inside the sockb
     * thread. */
    cnd_new(&cnd);
    mtx_new(&mtx);
    mtx_lock(&mtx);
    
#ifdef _LIBSOCK_DBG
    msg_unwait.magic = _LIBSOCK_SOCKB_MSG_MAGIC;
#endif
    msg_unwait.type = UNWAIT;
    msg_unwait.data.unwait.old_msg_p = &msg_wait;
    msg_unwait.data.unwait.cnd = &cnd;
    msg_unwait.data.unwait.mtx = &mtx;

    while (0 != mq_put(&g_sockb->messages, (const void *) &msg_unwait))
    {
      /* Uh oh.  The msg_wait structure absolutely can't go away until we've
       * removed all references to it in the sockb thread.  This is a very bad
       * error, so we have to loop until the message gets through. */
      thd_yield();
    }
    sockb_l_wakeup();
    cnd_wait(&cnd, &mtx);
    
    mtx_unlock(&mtx);
    cnd_delete(&cnd);
    mtx_delete(&mtx);

    retval = TRUE;
  }
  
  RETURN:
  cnd_delete(&cnd);
  mtx_delete(&mtx);
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
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Error in write(): %s\n", strerror(errno));
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
  _cw_assert(sock_get_fd(a_sock) < FD_SETSIZE);

  message = (struct cw_sockb_msg_s *) pezz_get(&g_sockb->messages_pezz);
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
    pezz_put(&g_sockb->messages_pezz, (void *) message);
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
  _cw_assert(a_sockfd < FD_SETSIZE);

  message = (struct cw_sockb_msg_s *) pezz_get(&g_sockb->messages_pezz);
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
    pezz_put(&g_sockb->messages_pezz, (void *) message);
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
  _cw_assert(a_sockfd < FD_SETSIZE);

  message = (struct cw_sockb_msg_s *) pezz_get(&g_sockb->messages_pezz);
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
    pezz_put(&g_sockb->messages_pezz, (void *) message);
    retval =TRUE;
    goto RETURN;
  }
  
  sockb_l_wakeup();

  retval = FALSE;

  RETURN:
  return retval;
}

cw_bool_t
sockb_l_get_host_ip(char * a_host_str, cw_uint32_t * r_host_ip)
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
      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Error in gethostbyname(): %s\n", hstrerror(h_errno));
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "Host \"%s\" isn't an IP address or a hostname\n",
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
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"IP address: %d.%d.%d.%d\n",
		*r_host_ip & 0xff,
		(*r_host_ip >> 8) & 0xff,
		(*r_host_ip >> 16) & 0xff,
		*r_host_ip >> 24);
  }

  return retval;
}

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

static void
sockb_p_wait_notify(struct cw_sockb_msg_s ** a_wait_vector,
		    struct cw_sockb_msg_s * a_msg)
{
  cw_uint32_t i;
  
  _cw_check_ptr(a_msg);
  _cw_assert(_LIBSOCK_SOCKB_MSG_MAGIC == a_msg->magic);
  _cw_assert(WAIT == a_msg->type);

  mtx_lock(a_msg->data.wait.mtx);
  for (i = 0; i < a_msg->data.wait.nfds; i++)
  {
    /* Clear out the vector entry if it still belongs to us. */
    if (a_wait_vector[a_msg->data.wait.fd_array[i]] == a_msg)
    {
#ifdef _LIBSTASH_SOCKB_CONFESS
      log_printf(cw_g_log,
		 "v",
		 a_msg->data.wait.fd_array[i]);
#endif
      a_wait_vector[a_msg->data.wait.fd_array[i]] = NULL;
    }
  }
  
  cnd_signal(a_msg->data.wait.cnd);
  mtx_unlock(a_msg->data.wait.mtx);
}

static void *
sockb_p_entry_func(void * a_arg)
{
  cw_sock_t * socks[FD_SETSIZE];
  fd_set registered_set;
  fd_set fd_m_read_set, fd_read_set;
  fd_set fd_m_write_set, fd_write_set;
  fd_set fd_m_exception_set, fd_exception_set;
  cw_sock_t * sock;
  int sockfd, max_fd = 0, num_ready;
  cw_sint32_t j;
  cw_buf_t tmp_buf, buf_in;
  struct cw_sockb_msg_s * message;

  /* If a pointer is non-NULL, then there is a message structure that contains
   * the necessary information to be able to remove all the entries in the
   * vector that the waiter specified, as well as wake up the waiter. */
  struct cw_sockb_msg_s * wait_vector[FD_SETSIZE];

  /* Initialize data structures. */
  FD_ZERO(&registered_set);
  FD_ZERO(&fd_m_read_set);
  FD_ZERO(&fd_m_write_set);
  FD_ZERO(&fd_m_exception_set);
  buf_new(&tmp_buf, FALSE);
  buf_new(&buf_in, FALSE);
  bzero(wait_vector, FD_SETSIZE * sizeof(struct cw_sockb_msg_s *));
  
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
    while (NULL != (message
		    = (struct cw_sockb_msg_s *) mq_tryget(&g_sockb->messages)))
    {
      switch (message->type)
      {
	case REGISTER:
	{
	  sock = message->data.sock;
      
	  sockfd = sock_get_fd(sock);
	  if (!FD_ISSET(sockfd, &registered_set))
	  {
	    /* The sock isn't registered.  Register it. */
#ifdef _LIBSTASH_SOCKB_CONFESS
	    log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
			"Register %d\n", sockfd);
#endif

	    FD_SET(sockfd, &registered_set);
	    FD_SET(sockfd, &fd_m_read_set);
/*  	    FD_SET(sockfd, &fd_m_write_set); */
	    /* Uncomment this if adding out of band data support. */
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
#ifdef _LIBSTASH_SOCKB_CONFESS
	  else
	  {
	    log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
			"Refuse to register %d\n", sockfd);
	  }
#endif
#ifdef _LIBSOCK_DBG
	  message->magic = 0;
#endif
	  pezz_put(&g_sockb->messages_pezz, (void *) message);
	  break;
	}
	case UNREGISTER:
	{
	  sockfd = message->data.sockfd;

	  if (FD_ISSET(sockfd, &registered_set))
	  {
#ifdef _LIBSTASH_SOCKB_CONFESS
	    log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
			"Unregister %d\n", sockfd);
#endif
	
	    FD_CLR(sockfd, &registered_set);
	    FD_CLR(sockfd, &fd_m_read_set);
	    FD_CLR(sockfd, &fd_m_write_set); /* May not be set. */
	    /* Uncomment this if adding out of band data support. */
	    /* FD_CLR(sockfd, &fd_m_exception_set); */

	    /* Lower max_fd to the next highest fd, if necessary. */
	    if (sockfd == max_fd)
	    {
	      max_fd = g_sockb->pipe_out;
	      for (j = sockfd - 1; j > g_sockb->pipe_out; j--)
	      {
		if (FD_ISSET(j, &registered_set))
		{
		  max_fd = j;
		  break;
		}
	      }
	    }
	  }
#ifdef _LIBSTASH_SOCKB_CONFESS
	  else
	  {
	    log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
			"Refuse to unregister %d\n", sockfd);
	  }
#endif      
	  /* Notify the sock that it's unregistered. */
	  sock_l_message_callback(socks[sockfd]);
	  socks[sockfd] = NULL;
#ifdef _LIBSOCK_DBG
	  message->magic = 0;
#endif
	  pezz_put(&g_sockb->messages_pezz, (void *) message);
	  break;
	}
	case OUT_NOTIFY:
	{
	  sockfd = message->data.sockfd;

	  if (FD_ISSET(sockfd, &registered_set))
	  {
#ifdef _LIBSTASH_SOCKB_CONFESS
	    log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
			"Set %dw\n", sockfd);
#endif
	
	    FD_SET(sockfd, &fd_m_write_set);
	  }
#ifdef _LIBSTASH_SOCKB_CONFESS
	  else
	  {
	    log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
			"Refuse to set %dw\n", sockfd);
	  }
#endif
#ifdef _LIBSOCK_DBG
	  message->magic = 0;
#endif
	  pezz_put(&g_sockb->messages_pezz, (void *) message);
	  break;
	}
	case WAIT:
	{
	  cw_uint32_t i;

	  for (i = 0; i < message->data.wait.nfds; i++)
	  {
	    /* Set the wait vector entry to this message. */
#ifdef _LIBSTASH_SOCKB_CONFESS
	    log_printf(cw_g_log,
			" (s%lu)",
			message->data.wait.fd_array[i]);
#endif
	    
	    wait_vector[message->data.wait.fd_array[i]] = message;
	  }
	    
	  break;
	}
	case UNWAIT:
	{
	  cw_uint32_t i;

	  mtx_lock(message->data.unwait.mtx);
	  _cw_assert(WAIT == message->data.unwait.old_msg_p->type);
	  
	  for (i = 0; i < message->data.unwait.old_msg_p->data.wait.nfds; i++)
	  {
	    /* Clear the wait_vector entry if it's still for this message. */
	    if (wait_vector
		[message->data.unwait.old_msg_p->data.wait.fd_array[i]]
		== message->data.unwait.old_msg_p)
	    {
#ifdef _LIBSTASH_SOCKB_CONFESS
	    log_printf(cw_g_log,
			" (u%lu)",
			message->data.unwait.old_msg_p->data.wait.fd_array[i]);
#endif
	      wait_vector[message->data.unwait.old_msg_p->
			 data.wait.fd_array[i]] = NULL;
	    }
#ifdef _LIBSTASH_SOCKB_CONFESS
	    else
	    {
	      log_printf(cw_g_log,
			 " (x%lu)",
			 message->data.unwait.old_msg_p->data.wait.fd_array[i]);
	    }
#endif
	    
	  }

	  /* Notify the message sender that all is well now. */
	  cnd_signal(message->data.unwait.cnd);
	  mtx_unlock(message->data.unwait.mtx);
	  break;
	}
	default:
	{
	  _cw_error("Programming error");
	}
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

    {
      cw_uint32_t i, in_size;

#ifdef _LIBSTASH_SOCKB_CONFESS
      log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
		  "select fd's:");
#endif
      for (i = 0; i <= max_fd; i++)
      {
	if (FD_ISSET(i, &registered_set))
	{
#ifdef _LIBSTASH_SOCKB_CONFESS
	  log_printf(cw_g_log, " %dR", i);
#endif
	  
	  /* Is any space available? */
	  in_size = sock_l_get_in_size(socks[i]);
	  
	  if (sock_l_get_in_max_buf_size(socks[i])
	      <= in_size)
	  {
	    /* Nope, no space. */
	    FD_CLR(i, &fd_read_set);
	    
	  }
	  if ((0 < in_size) && (NULL != wait_vector[i]) && (NULL != socks[i]))
	  {
	    sockb_p_wait_notify(wait_vector, wait_vector[i]);
	  }
#ifdef _LIBSTASH_SOCKB_CONFESS
	  else
	  {
	    log_printf(cw_g_log, "r");
	  }
	  if (FD_ISSET(i, &fd_write_set))
	  {
	    log_printf(cw_g_log, "w");
	  }
#endif
	}
      }
#ifdef _LIBSTASH_SOCKB_CONFESS
      log_printf(cw_g_log, " (%dr)\n", g_sockb->pipe_out);
#endif
    }

#ifdef _LIBSTASH_SOCKB_CONFESS
    log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
		"select(%lu)", max_fd + 1);
#endif
    num_ready = select(max_fd + 1,
		       &fd_read_set, &fd_write_set, &fd_exception_set,
		       NULL);
#ifdef _LIBSTASH_SOCKB_CONFESS
    log_printf(cw_g_log, "-->(%lu)\n", num_ready);
#endif
    
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
      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "select() timeout expired\n");
      }
    }
#endif
    else
    {
      cw_sint32_t i, j, k;
      
#ifdef _LIBSTASH_SOCKB_CONFESS
      log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
		  "Check fd:");
#endif
      
      /* Ready descriptors. */
      if (FD_ISSET(g_sockb->pipe_out, &fd_read_set))
      {
	char t_buf[2];
	ssize_t bytes_read;

#ifdef _LIBSTASH_SOCKB_CONFESS
	log_printf(cw_g_log, " (%dr)", g_sockb->pipe_out);
#endif
	
	/* Clear the read bit to avoid attempting to handle it in the loop
	 * below. */
	FD_CLR(g_sockb->pipe_out, &fd_read_set);
	num_ready--;

	/* Read the data out of the pipe so that the next call doesn't
	 * immediately return just because of data already in the pipe.  Note
	 * that there is no risk of deadlock due to emptying data from the pipe
	 * that is written after the select call, since the message queues are
	 * checked after emptying the pipe, but before calling select()
	 * again. */
	bytes_read = read(g_sockb->pipe_out, t_buf, 2);
	if (bytes_read == -1)
	{
	  if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	  {
	    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			"Error in read(): %s\n", strerror(errno));
	  }
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

      /* Perhaps looping downward will save a bit of work. */
      for (i = max_fd, j = 0; (j < num_ready) && (i >= 0); i--)
      {
#ifdef _LIBSTASH_SOCKB_CONFESS
	log_printf(cw_g_log, " %d", i);
#endif

	/* XXX Yuck, fix this. */
	if (!FD_ISSET(i, &registered_set))
	{
	  continue;
	}
	
	if (FD_ISSET(i, &fd_read_set))
	{
	  const struct iovec * iovec;
	  int iovec_count;
	  ssize_t bytes_read;
	  cw_sint32_t max_read;
	  cw_bufc_t * bufc;
	  
	  j++;

#ifdef _LIBSTASH_SOCKB_CONFESS
	  log_printf(cw_g_log, "r");
#endif
	  /* Ready for reading. */

	    /* Figure out how much data we're willing to shove into this sock's
	     * incoming buffer. */
	  max_read = (sock_l_get_in_max_buf_size(socks[i])
		      - sock_l_get_in_size(socks[i]));
	  _cw_assert(max_read > 0);

	    /* Build up buf_in to be at least large enough for the readv(). */
	  while (buf_get_size(&buf_in) < max_read)
	  {
	    bufc = sockb_get_spare_bufc();
	    if (NULL == bufc)
	    {
	      /* There isn't enough free memory to make the incoming buffer as
	       * big as we would like.  As long as the incoming buffer has at
	       * least some space though, continue processing.  Otherwise, we
	       * could loop, trying to allocate buffer space.  For the first
	       * cut at implementing this though, just abort(). */
	      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	      {
		log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			    "Allocation error.  Got %lu/%l desired bytes"
			    " buffer space\n",
			    buf_get_size(&buf_in), max_read);
	      }
	      
	      if (0 < buf_get_size(&buf_in))
	      {
		break;
	      }
	      else
	      {
		_cw_error("No space in &buf_in");
	      }
	    }
	    if (TRUE
		== buf_append_bufc(&buf_in, bufc, 0,
				   pezz_get_buffer_size(&g_sockb->buffer_pool)))
	    {
	      /* As above, we have a memory allocation problem.  Clean up
	       * bufc, but otherwise take the same approach. */
	      bufc_delete(bufc);
	      
	      if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	      {
		log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			    "Allocation error.  Got %lu/%l desired bytes"
			    " buffer space\n",
			    buf_get_size(&buf_in), max_read);
	      }
	      
	      if (0 < buf_get_size(&buf_in))
	      {
		break;
	      }
	      else
	      {
		_cw_error("No space in &buf_in");
	      }
	    }
	    bufc_delete(bufc);
	  }
	  
	  /* Get an iovec for reading.  This somewhat goes against the idea of
	     * never writing the internals of a buf after the buffers have been
	     * inserted.  However, this is quite safe, since as a result of how
	     * we use buf_in, we know for sure that there are no other
	     * references to the byte ranges of the buffers we are writing
	     * to. */
	  iovec = buf_get_iovec(&buf_in, max_read, TRUE, &iovec_count);

	  bytes_read = readv(i, iovec, iovec_count);
#ifdef _LIBSTASH_SOCKB_CONFESS
	  log_printf(cw_g_log, "(%d)", bytes_read);
#endif

	  if (bytes_read > 0)
	  {
	    if ((NULL != wait_vector[i]) && (NULL != socks[i]))
	    {
	      sockb_p_wait_notify(wait_vector, wait_vector[i]);
	    }
	    _cw_assert(buf_get_size(&tmp_buf) == 0);

	    if (TRUE == buf_split(&tmp_buf, &buf_in, bytes_read))
	    {
	      /* XXX */
	      _cw_error("Unhandled error condition");
	    }

	    /* Append to the sock's in_buf. */
	    sock_l_put_in_data(socks[i], &tmp_buf);
	    _cw_assert(buf_get_size(&tmp_buf) == 0);
	  }
	  else if (0 == bytes_read)
	  {
	    /* readv() error. */
	    if (dbg_is_registered(cw_g_dbg, "sockb_verbose"))
	    {
	      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			  "EOF in readv().  Closing sockfd %d\n", i);
	    }
	    FD_CLR(i, &registered_set);
	    FD_CLR(i, &fd_m_read_set);
	    FD_CLR(i, &fd_m_write_set);
/*  	    FD_CLR(i, &fd_m_exception_set); */
	    
	    /* Lower max_fd to the next highest fd, if necessary. */
	    if (i == max_fd)
	    {
#ifdef _LIBSTASH_SOCKB_CONFESS
	      log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
			  "max_fd: %d", max_fd);
#endif
	      max_fd = g_sockb->pipe_out;
	      for (k = i - 1; k > g_sockb->pipe_out; k--)
	      {
		if (FD_ISSET(k, &registered_set))
		{
		  max_fd = k;
		  break;
		}
	      }
#ifdef _LIBSTASH_SOCKB_CONFESS
	      log_printf(cw_g_log, "-->%d\n", max_fd);
#endif
	    }

	    sock_l_error_callback(socks[i]);
	    socks[i] = NULL;

	    /* Make sure not to try to handle outgoing data on this socket,
	       * since we just set the sock pointer to NULL. */
	    if (FD_ISSET(i, &fd_write_set))
	    {
	      j++;
	      FD_CLR(i, &fd_write_set);
	    }
	  }
	  else /*  if (bytes_read == -1) */
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

#ifdef _LIBSTASH_SOCKB_CONFESS
	  log_printf(cw_g_log, "w");
#endif	  
	  /* Ready for writing. */

	  /* Get the socket's buf. */
	  _cw_assert(0 == buf_get_size(&tmp_buf));
	  sock_l_get_out_data(socks[i], &tmp_buf);

	  /* Build an iovec for writing. */
	  iovec = buf_get_iovec(&tmp_buf,
				buf_get_size(&tmp_buf),
				TRUE,
				&iovec_count);

	  bytes_written = writev(i, iovec, iovec_count);
#ifdef _LIBSTASH_SOCKB_CONFESS
	  log_printf(cw_g_log, "(%d/%lu)", bytes_written,
		     buf_get_size(&tmp_buf));
#endif

	  if (bytes_written >= 0)
	  {
	    buf_release_head_data(&tmp_buf, bytes_written);
	    
	    if (0 == sock_l_put_back_out_data(socks[i], &tmp_buf))
	    {
	      /* The socket has no more outgoing data, so turn the write bit
	       * off in the master write descriptor set. */
#ifdef _LIBSTASH_SOCKB_CONFESS
	      log_printf(cw_g_log, "u");
#endif
	      FD_CLR(i, &fd_m_write_set);
	    }
#ifdef _LIBSTASH_SOCKB_CONFESS
	    else
	    {
	      log_printf(cw_g_log, "i");
	    }
#endif
	  }
	  else /* if (bytes_written == -1) */
	  {
	    if ((NULL != wait_vector[i]) && (NULL != socks[i]))
	    {
	      sockb_p_wait_notify(wait_vector, wait_vector[i]);
	    }
	    
	    buf_release_head_data(&tmp_buf,
				  buf_get_size(&tmp_buf));
	    
	    if (dbg_is_registered(cw_g_dbg, "sockb_error"))
	    {
	      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			  "Error in writev(): %s\n", strerror(errno));
	      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
			  "Closing sockfd %d\n", i);
	    }
	    
	    FD_CLR(i, &registered_set);
	    FD_CLR(i, &fd_m_read_set);
	    FD_CLR(i, &fd_m_write_set);
/*  	    FD_CLR(i, &fd_m_exception_set); */

	    /* Lower max_fd to the next highest fd, if necessary. */
	    if (i == max_fd)
	    {
#ifdef _LIBSTASH_SOCKB_CONFESS
	      log_eprintf(cw_g_log, __FILE__, __LINE__, NULL,
			  "max_fd: %d", max_fd);
#endif
	      max_fd = g_sockb->pipe_out;
	      for (k = i - 1; k > g_sockb->pipe_out; k--)
	      {
		if (FD_ISSET(k, &registered_set))
		{
		  max_fd = k;
		  break;
		}
	      }
#ifdef _LIBSTASH_SOCKB_CONFESS
	      log_printf(cw_g_log, "-->%d\n", max_fd);
#endif
	    }

	    sock_l_error_callback(socks[i]);
	    socks[i] = NULL;
	  }
	  _cw_assert(buf_get_size(&tmp_buf) == 0);
	}
#if (0)
	if (FD_ISSET(i, &fd_exception_set))
	{
	  j++;
	  _cw_error("Out of band data handling unimplemented");
	  /* If we ever need to use out of band data, this must be
	   * implemented. */
	}
#endif
      }
#ifdef _LIBSTASH_SOCKB_CONFESS
      log_printf(cw_g_log, "\n");
#endif
    }
  }

  buf_delete(&buf_in);
  buf_delete(&tmp_buf);
  return NULL;
}
