/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libsock/libsock.h"

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

#include "../include/libsock/libsock_l.h"
#include "../include/libsock/sock_l.h"

#ifdef _LIBSOCK_DBG
#define _LIBSOCK_MSG_MAGIC 0xf0010725
#endif

struct cw_libsock_msg_s {
#ifdef _LIBSOCK_DBG
	cw_uint32_t	magic;
#endif
	enum {
		REGISTER, UNREGISTER, OUT_NOTIFY, IN_SPACE, IN_NOTIFY
	}	type;
	union {
		cw_sock_t	*sock;
		cw_uint32_t	sockfd;
		struct {
			int	sockfd;
			cw_mq_t	*mq;
			cw_mtx_t *mtx;
			cw_cnd_t *cnd;
		}	in_notify;
	}	data;
};

struct cw_libsock_reg_s {
	cw_sock_t	*sock;	/* sock pointer. */
	cw_sint32_t	pollfd_pos; /*
				     * Offset in the pollfd struct passed into
				     * poll().
				     */
	cw_mq_t		*notify_mq; /*
				     * mq to notify when readable or closed (or
				     * NULL).
				     */
};

struct cw_libsock_entry_s {
	cw_uint32_t	max_fds;
	struct cw_libsock_reg_s *regs;
	struct pollfd	*fds;
};

struct cw_libsock_s {
	cw_bool_t	should_quit;
	cw_thd_t	thread;
	int		pipe_in;
	int		pipe_out;
	cw_sema_t	pipe_sema;

	cw_pezz_t	bufc_pool;
	cw_pezz_t	buffer_pool;

	cw_pezz_t	messages_pezz;
	cw_mq_t		messages;
	cw_cnd_t	wait_cnd;

	cw_mtx_t	get_ip_addr_lock;
};

static cw_bool_t libsock_p_notify(cw_mq_t *a_mq, int a_sockfd);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_arg : Unused, merely for function prototype compatibility with thd_new().
 *
 * <<< Output(s) >>>
 *
 * retval : NULL, never used.
 *
 * <<< Description >>>
 *
 * Entry point for the back end thread.  The back end thread essentially
 * executes a poll() loop and communicates with other threads via message
 * queues (list's).
 *
 ******************************************************************************/
static void	*libsock_p_entry_func(void *a_arg);

/* Global. */
cw_libsock_t	*g_libsock = NULL;

cw_bool_t
libsock_init(cw_uint32_t a_max_fds, cw_uint32_t a_bufc_size, cw_uint32_t
    a_max_spare_bufcs)
{
	struct cw_libsock_entry_s *arg;
	int		filedes[2];
	int		val;

	_cw_assert(a_bufc_size > 0);

	arg = (struct cw_libsock_entry_s *)_cw_malloc(sizeof(struct
	    cw_libsock_entry_s));

	if (arg == NULL)
		goto OOM_1;
	arg->max_fds = a_max_fds;
	arg->regs = (struct cw_libsock_reg_s *)_cw_calloc(a_max_fds,
	    sizeof(struct cw_libsock_reg_s));

	if (arg->regs == NULL)
		goto OOM_2;
	memset(arg->regs, 0, a_max_fds * sizeof(struct cw_libsock_reg_s));

	arg->fds = (struct pollfd *)_cw_calloc(a_max_fds, sizeof(struct
	    pollfd));

	if (arg->fds == NULL)
		goto OOM_3;
	memset(arg->fds, 0, a_max_fds * sizeof(struct pollfd));

	if (g_libsock == NULL) {
		g_libsock = (cw_libsock_t *)_cw_malloc(sizeof(cw_libsock_t));
		if (g_libsock == NULL)
			goto OOM_4;
		memset(g_libsock, 0, sizeof(cw_libsock_t));

		/*
		 * Ignore SIGPIPE, so that writing to a closed socket won't
		 * crash the program.
		 */
		signal(SIGPIPE, SIG_IGN);

		/*
		 * Create a pipe that will be used in conjunction with the
		 * message queues to make the back end thread return from the
		 * poll() call.
		 */

		if (pipe(filedes) == -1) {
			_cw_out_put_e("Fatal error in pipe(): [s]\n",
			    strerror(errno));
			abort();
		}
		g_libsock->pipe_out = filedes[0];
		g_libsock->pipe_in = filedes[1];

		/* Set g_libsock->pipe_in to non-blocking. */
		val = fcntl(g_libsock->pipe_in, F_GETFL, 0);
		if (val == -1) {
			_cw_out_put_e("Fatal error for F_GETFL in fcntl(): "
			    "[s]\n", strerror(errno));
			abort();
		}
		if (fcntl(g_libsock->pipe_in, F_SETFL, val | O_NONBLOCK)) {
			_cw_out_put_e("Fatal error for F_SETFL in fcntl(): "
			    "[s]\n", strerror(errno));
			abort();
		}

		val = fcntl(g_libsock->pipe_out, F_GETFL, 0);
		if (val == -1) {
			_cw_out_put_e("Fatal error for F_GETFL in fcntl(): "
			    "[s]\n", strerror(errno));
			abort();
		}
		if (fcntl(g_libsock->pipe_in, F_SETFL, val | O_NONBLOCK)) {
			_cw_out_put_e("Fatal error for F_SETFL in fcntl(): "
			    "[s]\n", strerror(errno));
			abort();
		}

		g_libsock->should_quit = FALSE;

		/*
		 * Create the semaphore used for determining whether data should
		 * be written to the pipe in order to force a return from
		 * poll().
		 */
		sema_new(&g_libsock->pipe_sema, 1);

		/*
		 * Create the spare bufc pool and initialize associated
		 * variables.
		 */
		if (pezz_new(&g_libsock->bufc_pool, cw_g_mem, sizeof(cw_bufc_t),
		    (a_max_spare_bufcs)? a_max_spare_bufcs : 1) == NULL)
			goto OOM_5;
		if (pezz_new(&g_libsock->buffer_pool, cw_g_mem, a_bufc_size,
		    (a_max_spare_bufcs) ? a_max_spare_bufcs : 1) == NULL)
			goto OOM_6;
		/* Create the message queues. */
		if (pezz_new(&g_libsock->messages_pezz, cw_g_mem, sizeof(struct
		    cw_libsock_msg_s), 8) == NULL)
			goto OOM_7;
		if (mq_new(&g_libsock->messages, cw_g_mem, sizeof(struct
		    cw_libsock_msg_s *)) == NULL)
			goto OOM_8;

		/* Create the lock used for protecting gethostbyname(). */
		mtx_new(&g_libsock->get_ip_addr_lock);

		/*
		 * Create a new thread to handle all of the back end socket
		 * foo.
		 */
		thd_new(&g_libsock->thread, libsock_p_entry_func, (void *)arg);
	}

	return FALSE;

	OOM_8:
	pezz_delete(&g_libsock->messages_pezz);
	OOM_7:
	pezz_delete(&g_libsock->buffer_pool);
	OOM_6:
	pezz_delete(&g_libsock->bufc_pool);
	OOM_5:
	_cw_free(g_libsock);
	g_libsock = NULL;
	OOM_4:
	_cw_free(arg->fds);
	OOM_3:
	_cw_free(arg->regs);
	OOM_2:
	_cw_free(arg);
	OOM_1:
	return TRUE;
}

void
libsock_shutdown(void)
{
	if (g_libsock != NULL) {
		/* Tell the back end thread to quit, then join on it. */
		g_libsock->should_quit = TRUE;
		libsock_l_wakeup();
		thd_join(&g_libsock->thread);

		sema_delete(&g_libsock->pipe_sema);

		/* Clean up the spare bufc's. */
		pezz_delete(&g_libsock->bufc_pool);
		pezz_delete(&g_libsock->buffer_pool);

		pezz_delete(&g_libsock->messages_pezz);
		mq_delete(&g_libsock->messages);

		/* Delete the gethostbyname() protection lock. */
		mtx_delete(&g_libsock->get_ip_addr_lock);

		_cw_free(g_libsock);
	}
}

cw_bufc_t *
libsock_spare_bufc_get(void)
{
	cw_bufc_t	*retval;
	void		*buffer;

	_cw_check_ptr(g_libsock);

	retval = bufc_new((cw_bufc_t *)pezz_get(&g_libsock->bufc_pool),
	    cw_g_mem, (cw_opaque_dealloc_t *)pezz_put_e, (void
	    *)&g_libsock->bufc_pool);
	if (retval == NULL) {
		retval = NULL;
		goto RETURN;
	}
	buffer = pezz_get(&g_libsock->buffer_pool);
	if (buffer == NULL) {
		bufc_delete(retval);
		retval = NULL;
		goto RETURN;
	}
	bufc_buffer_set(retval,
	    buffer, pezz_buffer_size_get(&g_libsock->buffer_pool), TRUE,
	    (cw_opaque_dealloc_t *)pezz_put_e, (void *)&g_libsock->buffer_pool);

	RETURN:
	return retval;
}

cw_bool_t
libsock_in_notify(cw_mq_t *a_mq, int a_sockfd)
{
	cw_bool_t	retval;
	struct cw_libsock_msg_s *message;
	cw_mtx_t	mtx;
	cw_cnd_t	cnd;

	_cw_check_ptr(g_libsock);
	_cw_assert(a_sockfd >= 0);

	mtx_new(&mtx);
	cnd_new(&cnd);

	message = (struct cw_libsock_msg_s
	    *)pezz_get(&g_libsock->messages_pezz);

	if (message == NULL) {
		retval = TRUE;
		goto RETURN;
	}
#ifdef _LIBSOCK_DBG
	message->magic = _LIBSOCK_MSG_MAGIC;
#endif
	message->type = IN_NOTIFY;
	message->data.in_notify.sockfd = a_sockfd;
	message->data.in_notify.mq = a_mq;
	message->data.in_notify.mtx = &mtx;
	message->data.in_notify.cnd = &cnd;

	mtx_lock(&mtx);
	if (mq_put(&g_libsock->messages, message) != 0) {
		pezz_put(&g_libsock->messages_pezz, (void *)message);
		retval = TRUE;
		goto RETURN;
	}
	libsock_l_wakeup();

	cnd_wait(&cnd, &mtx);
	mtx_unlock(&mtx);

	retval = FALSE;

	RETURN:
	mtx_delete(&mtx);
	cnd_delete(&cnd);
	return retval;
}

void
libsock_l_wakeup(void)
{
	if (sema_trywait(&g_libsock->pipe_sema) == FALSE) {
		if (write(g_libsock->pipe_in, "X", 1) == -1) {
			if (dbg_is_registered(cw_g_dbg, "libsock_error")) {
				_cw_out_put_e("Error in write(): [s]\n",
				    strerror(errno));
			}
		}
	}
}

cw_bool_t
libsock_l_sock_register(cw_sock_t *a_sock)
{
	cw_bool_t		retval;
	struct cw_libsock_msg_s	*message;

	_cw_check_ptr(a_sock);
	_cw_check_ptr(g_libsock);
	_cw_assert(sock_fd_get(a_sock) >= 0);

	message = (struct cw_libsock_msg_s
	    *)pezz_get(&g_libsock->messages_pezz);
	if (message == NULL) {
		retval = TRUE;
		goto RETURN;
	}
#ifdef _LIBSOCK_DBG
	message->magic = _LIBSOCK_MSG_MAGIC;
#endif
	message->type = REGISTER;
	message->data.sock = a_sock;
	if (mq_put(&g_libsock->messages, message) != 0) {
		pezz_put(&g_libsock->messages_pezz, (void *)message);
		retval = TRUE;
		goto RETURN;
	}
	libsock_l_wakeup();

	retval = FALSE;

	RETURN:
	return retval;
}

cw_bool_t
libsock_l_sock_unregister(cw_uint32_t a_sockfd)
{
	cw_bool_t		retval;
	struct cw_libsock_msg_s	*message;

	_cw_check_ptr(g_libsock);

	message = (struct cw_libsock_msg_s
	    *)pezz_get(&g_libsock->messages_pezz);
	if (message == NULL) {
		retval = TRUE;
		goto RETURN;
	}
#ifdef _LIBSOCK_DBG
	message->magic = _LIBSOCK_MSG_MAGIC;
#endif
	message->type = UNREGISTER;
	message->data.sockfd = a_sockfd;
	if (mq_put(&g_libsock->messages, message) != 0) {
		pezz_put(&g_libsock->messages_pezz, (void *)message);
		retval = TRUE;
		goto RETURN;
	}
	libsock_l_wakeup();

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
libsock_l_out_notify(cw_uint32_t a_sockfd)
{
	cw_bool_t		retval;
	struct cw_libsock_msg_s	*message;

	_cw_check_ptr(g_libsock);

	message = (struct cw_libsock_msg_s
	    *)pezz_get(&g_libsock->messages_pezz);

	if (NULL == message) {
		retval = TRUE;
		goto RETURN;
	}
#ifdef _LIBSOCK_DBG
	message->magic = _LIBSOCK_MSG_MAGIC;
#endif
	message->type = OUT_NOTIFY;
	message->data.sockfd = a_sockfd;
	if (mq_put(&g_libsock->messages, message) != 0) {
		pezz_put(&g_libsock->messages_pezz, (void *)message);
		retval = TRUE;
		goto RETURN;
	}
	libsock_l_wakeup();

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
libsock_l_in_space(cw_uint32_t a_sockfd)
{
	cw_bool_t retval;
	struct cw_libsock_msg_s *message;

	_cw_check_ptr(g_libsock);

	message = (struct cw_libsock_msg_s
	    *)pezz_get(&g_libsock->messages_pezz);

	if (message == NULL) {
		retval = TRUE;
		goto RETURN;
	}
#ifdef _LIBSOCK_DBG
	message->magic = _LIBSOCK_MSG_MAGIC;
#endif
	message->type = IN_SPACE;
	message->data.sockfd = a_sockfd;
	if (mq_put(&g_libsock->messages, message) != 0) {
		pezz_put(&g_libsock->messages_pezz, (void *)message);
		retval = TRUE;
		goto RETURN;
	}
	libsock_l_wakeup();

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
libsock_l_host_ip_get(const char *a_host_str, cw_uint32_t *r_host_ip)
{
	cw_bool_t	retval;
	cw_uint32_t	host_ip;

	_cw_check_ptr(g_libsock);

	host_ip = inet_addr(a_host_str);

#ifdef _CW_OS_SOLARIS
	if ((in_addr_t) host_ip == (in_addr_t) - 1)
#else
	if (host_ip == INADDR_NONE)
#endif
	{
		struct hostent	*host_entry;

		/*
		 * Not a dotted number IP address.  Let's try it as a hostname.
		 */

		/* Lock the mutex first. */
		mtx_lock(&g_libsock->get_ip_addr_lock);

		host_entry = gethostbyname(a_host_str);
		if (host_entry == NULL) {
			if (dbg_is_registered(cw_g_dbg, "libsock_error")) {
#ifdef _CW_OS_SOLARIS
				_cw_out_put_e("Error in gethostbyname(): [i]\n",
				    h_errno);
#else
				_cw_out_put_e("Error in gethostbyname(): [s]\n",
				    hstrerror(h_errno));
#endif
				_cw_out_put_e("Host \"[s]\" isn't an IP address"
				    " or a hostname\n", a_host_str);
			}
			retval = TRUE;
		} else {
			struct in_addr	*addr_ptr;

			_cw_assert(host_entry->h_addrtype == AF_INET);
			_cw_assert(host_entry->h_addr_list[0] != NULL);

			addr_ptr = (struct in_addr *)host_entry->h_addr_list[0];

			*r_host_ip = addr_ptr->s_addr;

			retval = FALSE;
		}
		/*
		 * We're done with h_errno and the global structure pointed to
		 * by host_entry, so unlock the mutex now.
		 */
		mtx_unlock(&g_libsock->get_ip_addr_lock);
	} else {
		*r_host_ip = host_ip;
		retval = FALSE;
	}

	if (dbg_is_registered(cw_g_dbg, "libsock_verbose")) {
		cw_uint32_t	t_host_ip = ntohl(*r_host_ip);

		out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "IP address: [i].[i].[i].[i]\n", t_host_ip >> 24, (t_host_ip
		    >> 16) & 0xff, (t_host_ip >> 8) & 0xff, t_host_ip & 0xff);
	}
	return retval;
}

static cw_bool_t
libsock_p_notify(cw_mq_t *a_mq, int a_sockfd)
{
	cw_bool_t		retval;
	volatile cw_sint32_t	error = 0;

	_cw_check_ptr(a_mq);

	xep_begin();
	xep_try {
		error = mq_put(a_mq, a_sockfd);
	}
	xep_catch(_CW_XEPV_OOM) {
		/*
		 * We can't afford to lose the message, since it could end up
		 * causing deadlock.
		 */
		thd_yield();
		xep_retry();
	}
	xep_end();

	if (error == 1) {
		/*
		 * The mq is in the stop state.  We might as well stop
		 * notifying.
		 */
		retval = TRUE;
		goto RETURN;
	}
	retval = FALSE;
#ifdef _LIBSOCK_CONFESS
	_cw_out_put("n");
#endif

	RETURN:
	return retval;
}

static void *
libsock_p_entry_func(void *a_arg)
{
	struct cw_libsock_entry_s *arg = (struct cw_libsock_entry_s *)a_arg;
	cw_uint32_t		max_fds = arg->max_fds;
	struct cw_libsock_reg_s	*regs = arg->regs;
	struct pollfd		*fds = arg->fds;
	volatile unsigned	nfds;
	cw_sock_t		*sock;
	volatile int		sockfd, num_ready;
	cw_buf_t		tmp_buf, buf_in;
	struct cw_libsock_msg_s	*message;

	_cw_free(a_arg);

	/* Initialize data structures. */
	buf_new(&tmp_buf, cw_g_mem);
	buf_new(&buf_in, cw_g_mem);
	{
		cw_uint32_t	i;

		for (i = 0; i < max_fds; i++)
			regs[i].pollfd_pos = -1;
	}

	/*
	 * Add g_libsock->pipe_out for reading, so that this thread will return
	 * from poll() when data is written to g_libsock->pipe_in.
	 */
	fds[0].fd = g_libsock->pipe_out;
	fds[0].events = POLLIN;
	nfds = 1;

	while (g_libsock->should_quit == FALSE) {
		/* Check for messages in the message queues. */
		while (mq_tryget(&g_libsock->messages, &message) == FALSE) {
			_cw_check_ptr(message);
			switch (message->type) {
			case REGISTER:
				sock = message->data.sock;

				sockfd = sock_fd_get(sock);
				if (sockfd >= max_fds) {
					out_put_e(cw_g_out, NULL, 0,
					    __FUNCTION__,
					    "Maximum file descriptor exceeded"
					    " ([i] <= [i])\n", max_fds, sockfd);
					abort();
				}
				if ((regs[sockfd].pollfd_pos == -1) && (nfds <
				    max_fds)) {
					/*
					 * The sock isn't registered.  Register
					 * it.
					 */
					regs[sockfd].sock = sock;
					regs[sockfd].pollfd_pos = nfds;
					regs[sockfd].notify_mq = NULL;

					fds[nfds].fd = sockfd;
					/*
					 * If the buffer size is 0, don't ever
					 * try to read on this descriptor.
					 */
					if (sock_l_in_max_buf_size_get(sock) !=
					    0) {
#ifdef _LIBSOCK_CONFESS
						out_put_e(cw_g_out, __FILE__,
						    __LINE__, NULL,
						    "Register [i] ([i] byte "
						    "input buffer)\n", sockfd,
						    sock_l_in_max_buf_size_get(sock));
#endif
						fds[nfds].events = POLLIN;
					} else {
#ifdef _LIBSOCK_CONFESS
						out_put_e(cw_g_out, __FILE__,
						    __LINE__, NULL,
						    "Register [i](no in buffer"
						    ")\n", sockfd);
#endif
						fds[nfds].events = 0;
					}
					nfds++;

					/*
					 * Notify the sock that it's registered.
					 */
					sock_l_message_callback(sock, FALSE);
				} else {
#ifdef _LIBSOCK_CONFESS
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL, "Refuse to register [i]\n",
					    sockfd);
#endif

					sock_l_message_callback(sock, TRUE);
				}

				break;
			case UNREGISTER:
				sockfd = message->data.sockfd;

				if (regs[sockfd].pollfd_pos != -1) {
#ifdef _LIBSOCK_CONFESS
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL, "Unregister [i]\n", sockfd);
#endif

					nfds--;
					if (regs[sockfd].pollfd_pos != nfds) {
#ifdef _LIBSOCK_CONFESS
						_cw_out_put("h([i]-->[i])",
						    nfds,
						    regs[sockfd].pollfd_pos);
#endif

						regs[fds[nfds].fd].pollfd_pos =
						    regs[sockfd].pollfd_pos;
						memcpy(&fds[regs[sockfd].pollfd_pos],
						    &fds[nfds], sizeof(struct
						    pollfd));
					}
					regs[sockfd].pollfd_pos = -1;

					/*
					 * If this sock has a notification mq
					 * associated with it, send a final
					 * message, then deactivate
					 * notifications.
					 */
					if (regs[sockfd].notify_mq != NULL) {
						libsock_p_notify(regs[sockfd].notify_mq,
						    sockfd);
						regs[sockfd].notify_mq = NULL;
					}
				}
#ifdef _LIBSOCK_CONFESS
				else {
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL, "Refuse to unregister [i]\n",
					    sockfd);
				}
#endif

				/* Notify the sock that it's unregistered. */
				sock_l_message_callback(regs[sockfd].sock,
				    FALSE);
				regs[sockfd].sock = NULL;
				break;
			case OUT_NOTIFY:
				sockfd = message->data.sockfd;

				if (regs[sockfd].pollfd_pos != -1) {
#ifdef _LIBSOCK_CONFESS
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL, "Set [i]w\n", sockfd);
#endif

					fds[regs[sockfd].pollfd_pos].events |=
					    POLLOUT;
				}
#ifdef _LIBSOCK_CONFESS
				else {
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL, "Refuse to set [i]w\n",
					    sockfd);
				}
#endif
				break;
			case IN_SPACE:
				sockfd = message->data.sockfd;

				if (regs[sockfd].pollfd_pos != -1) {
#ifdef _LIBSOCK_CONFESS
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL, "Set [i]r\n", sockfd);
#endif

					fds[regs[sockfd].pollfd_pos].events |=
					    POLLIN;
				}
#ifdef _LIBSOCK_CONFESS
				else {
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL,
					    "Refuse to set [i]r\n", sockfd);
				}
#endif
				break;
			case IN_NOTIFY:
				sockfd = message->data.in_notify.sockfd;

				if (regs[sockfd].pollfd_pos != -1) {
					_cw_check_ptr(regs[sockfd].sock);
					regs[sockfd].notify_mq =
					    message->data.in_notify.mq;
#ifdef _LIBSOCK_CONFESS
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL,
					    "regs[[[i]].notify_mq = 0x[p]\n",
					    sockfd, regs[sockfd].notify_mq);
#endif

					mtx_lock(message->data.in_notify.mtx);
					cnd_signal(message->data.in_notify.cnd);
					mtx_unlock(message->data.in_notify.mtx);

					if (NULL != regs[sockfd].notify_mq) {
						if (sock_l_in_size_get(regs[sockfd].sock)
						    > 0) {
							/*
							 * Send an out
							 * notification, since
							 * there are data
							 * already queued up.
							 */
							if (libsock_p_notify(regs[sockfd].notify_mq,
							    sockfd)) {
								regs[sockfd].notify_mq
								    = NULL;
							}
						}
					}
				} else {
					mtx_lock(message->data.in_notify.mtx);
					cnd_signal(message->data.in_notify.cnd);
					mtx_unlock(message->data.in_notify.mtx);

					/*
					 * Send a message here to avoid a race
					 * condition where a sock is
					 * unregistered due to a failed
					 * readv()/writev(), but the user hasn't
					 * realized this yet, since no sock_*()
					 * calls have been made since the
					 * unregistration.
					 */
					if (message->data.in_notify.mq !=
					    NULL) {
						libsock_p_notify(message->data.in_notify.mq,
						    sockfd);
					}
#ifdef _LIBSOCK_CONFESS
					out_put_e(cw_g_out, __FILE__, __LINE__,
					    NULL,
					    "Refuse to set regs[[[i]].notify_mq = 0x[p]\n",
					    sockfd, message->data.in_notify.mq);
#endif
				}

				break;
			default:
				_cw_not_reached();
			}
#ifdef _LIBSOCK_DBG
			message->magic = 0;
#endif
			pezz_put(&g_libsock->messages_pezz, (void *)message);
		}

#ifdef _LIBSOCK_CONFESS
		{
			cw_uint32_t	i;

			out_put_e(cw_g_out, __FILE__, __LINE__, NULL,
			    "poll fd's:");
			for (i = 1; i < nfds; i++) {
				sockfd = fds[i].fd;

				_cw_out_put(" [i]R", sockfd);

				if (fds[i].events & POLLIN)
					_cw_out_put("r");
				if (fds[i].events & POLLOUT)
					_cw_out_put("w");
			}
			_cw_out_put(" ([i][s])\n", fds[0].fd,
			    (fds[0].events & POLLIN) ? "r" : "");
		}

		out_put_e(cw_g_out, __FILE__, __LINE__, NULL, "poll([i])",
		    nfds);
#endif

		num_ready = poll(fds, nfds, -1);

#ifdef _LIBSOCK_CONFESS
		_cw_out_put("-->([i|s:s])\n", num_ready);
#endif

		if (num_ready == -1) {
			if (errno != EINTR) {
				/* This is an error that should never happen. */
				_cw_out_put_e("Fatal error in poll(): [s]\n",
				    strerror(errno));
				abort();
			}
		} else {
			volatile cw_sint32_t	i, j;

#ifdef _LIBSOCK_CONFESS
			_cw_out_put_e("Check fd:");
#endif

			/* Ready descriptors. */
			if (fds[0].revents & POLLIN) {
				char	t_buf[2];
				ssize_t	bytes_read;

#ifdef _LIBSOCK_CONFESS
				_cw_out_put(" ([i|s:s]r)", g_libsock->pipe_out);
#endif

				/*
				 * Decrement the number of fd's that need
				 * handled in the loop below.
				 */
				num_ready--;

				/*
				 * Read the data out of the pipe so that the
				 * next call doesn't immediately return just
				 * because of data already in the pipe.  Note
				 * that there is no risk of deadlock due to
				 * emptying data from the pipe that is written
				 * after the poll() call, since the message
				 * queues are checked after emptying the pipe,
				 * but before calling poll() again.
				 */
				bytes_read = read(g_libsock->pipe_out, t_buf,
				    2);
				if (bytes_read == -1) {
					if (dbg_is_registered(cw_g_dbg,
					    "libsock_error")) {
						_cw_out_put_e("Error in read(): [s]\n",
						    strerror(errno));
					}
				} else if (bytes_read > 0) {
					/*
					 * Set the semaphore to one.  This will
					 * cause one, and only one byte to be
					 * written to g_libsock->pipe_in and
					 * cause a return from poll() if one or
					 * more messages needs handled.  Note
					 * that we must post the semaophore
					 * before handling the message queues,
					 * since it is possible to have new
					 * messages come in and miss them
					 * otherwise.  Posting first means that
					 * we may execute the poll() loop once
					 * without doing anything, since the
					 * message that caused data to be
					 * written to the pipe may have already
					 * been read.
					 */
					sema_post(&g_libsock->pipe_sema);
				}
				_cw_assert(bytes_read <= 1);
			}
			for (i = 1, j = 0; (j < num_ready) && (i < nfds); i++) {
				sockfd = fds[i].fd;

#ifdef _LIBSOCK_CONFESS
				out_put(cw_g_out,
				    " [i][s][s][s][s][s][s][s][s][s][s]",
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

				if (fds[i].revents & POLLIN) {
					const struct iovec *iov;
					int		iov_cnt;
					ssize_t		bytes_read;
					cw_sint32_t	max_read;
					cw_bufc_t	*bufc;
					

					j++;

#ifdef _LIBSOCK_CONFESS
					_cw_out_put("r");
#endif
					/* Ready for reading. */

					/*
					 * Figure out how much data we're
					 * willing to shove into this sock's
					 * incoming buffer.
					 */
					max_read =
					    sock_l_in_space_get(regs[sockfd].sock);

					/*
					 * Build up buf_in to be at least large
					 * enough for the readv().
					 */
					while (buf_size_get(&buf_in) <
					    max_read) {
						xep_begin();
						xep_try {
							bufc =
							    libsock_spare_bufc_get();
						}
						xep_catch(_CW_XEPV_OOM) {
							thd_yield();
							xep_retry();
						}
						xep_end();

						xep_begin();
						xep_try {
							buf_bufc_append(&buf_in,
							    bufc, 0,
							    pezz_buffer_size_get(&g_libsock->buffer_pool));
						}
						xep_catch(_CW_XEPV_OOM) {
							thd_yield();
							xep_retry();
						}
						xep_end();

						bufc_delete(bufc);
					}

					/*
					 * Get an iovec for reading.  This
					 * somewhat goes against the idea of
					 * never writing the internals of a buf
					 * after the buffers have been inserted.
					 * However, this is quite safe, since as
					 * a result of how we use buf_in, we
					 * know for sure that there are no other
					 * references to the byte ranges of the
					 * buffers we are writing to.
					 */
					iov = buf_iovec_get(&buf_in, max_read,
					    TRUE, &iov_cnt);

					bytes_read = readv(sockfd, iov,
					    iov_cnt);
#ifdef _LIBSOCK_CONFESS
					_cw_out_put("([i|s:s])",
					    bytes_read);
#endif

					if (bytes_read > 0) {
						cw_uint32_t	in_buf_free;

						_cw_assert(buf_size_get(&tmp_buf)
						    == 0);

						xep_begin();
						xep_try {
							buf_split(&tmp_buf,
							    &buf_in,
							    bytes_read);
						}
						xep_catch(_CW_XEPV_OOM) {
							thd_yield();
							xep_retry();
						}
						xep_end();

						/*
						 * Append to the sock's in_buf.
						 */
						in_buf_free =
						    sock_l_in_data_put(regs[sockfd].sock,
						    &tmp_buf);
						if (in_buf_free == 0) {
							/*
							 * Turn off the read bit
							 * for this sock.  The
							 * sock will send a
							 * message when there is
							 * once again space.
							 */
#ifdef _LIBSOCK_CONFESS
							_cw_out_put("u");
#endif
							fds[i].events ^=
							    (fds[i].events &
							    POLLIN);
						}
						/*
						 * Only send a message if the
						 * sock buffer was empty before
						 * we put data in it.
						 */
						if ((sock_l_in_max_buf_size_get(regs[sockfd].sock)
						    - (in_buf_free +
						    bytes_read)) == 0) {
							if (regs[sockfd].notify_mq
								    != NULL) {
								
								if (libsock_p_notify(regs[sockfd].notify_mq,
								    sockfd)) {
									regs[sockfd].notify_mq
									    = NULL;
								}
							}
						}
						_cw_assert(buf_size_get(&tmp_buf)
						    == 0);
					} else if (bytes_read == 0) {
						/* readv() error. */
						if (dbg_is_registered(cw_g_dbg,
						    "libsock_verbose")) {
							_cw_out_put_e("EOF in readv().  Closing sockfd [i]\n",
							    sockfd);
						}
						/*
						 * Fill this hole, decrement i,
						 * continue.
						 */
						nfds--;
						if (regs[sockfd].pollfd_pos !=
						    nfds) {
#ifdef _LIBSOCK_CONFESS
							_cw_out_put("h([i]-->[i])",
							    nfds, i);
#endif

							regs[fds[nfds].fd].pollfd_pos
							    = i;
							memcpy(&fds[i],
							    &fds[nfds],
							    sizeof(struct
							    pollfd));
							i--;
						}
						regs[sockfd].pollfd_pos = -1;

						sock_l_error_callback(regs[sockfd].sock);

						if (regs[sockfd].notify_mq !=
						    NULL) {
							if (libsock_p_notify(regs[sockfd].notify_mq,
							    sockfd)) {
								regs[sockfd].notify_mq
								    = NULL;
							}
						}
#ifdef _LIBSOCK_CONFESS
						_cw_out_put("\n");
#endif
						continue;
					} else {/* if (bytes_read == -1) */
						/* readv() error. */
						if (dbg_is_registered(cw_g_dbg,
						    "libsock_error")) {
							_cw_out_put_e("Error in readv(): [s]\n",
							    strerror(errno));
						}
					}
				} else if (fds[i].revents & POLLHUP) {
					const struct iovec	*iov;
					int			iov_cnt;
					ssize_t			bytes_read;
					cw_uint32_t		buffer_size;
					cw_bufc_t		*bufc;

					j++;

#ifdef _LIBSOCK_CONFESS
					_cw_out_put("r(");
#endif

					/*
					 * It may be that there are buffered
					 * data to be read, even though the
					 * descriptor has been closed.  If we
					 * succeed in reading data, there is no
					 * choice but to buffer all the read
					 * data in the sock's input buffer, even
					 * if it overfills the buffer.  We must
					 * keep reading until there are no more
					 * data, since this is the last time
					 * that this descriptor will be paid any
					 * attention.
					 */

					buffer_size =
					    pezz_buffer_size_get(&g_libsock->buffer_pool);

					do {
						/*
						 * Add some more space to
						 * &buf_in if necessary.
						 */
						if (buf_size_get(&buf_in) ==
						    0) {
							xep_begin();
							xep_try {
								bufc =
								    libsock_spare_bufc_get();
							}
							xep_catch(_CW_XEPV_OOM) {
								thd_yield();
								xep_retry();
							}
							xep_end();

							xep_begin();
							xep_try {
								buf_bufc_append(&buf_in,
								    bufc, 0,
								    buffer_size);
							}
							xep_catch(_CW_XEPV_OOM) {
								thd_yield();
								xep_retry();
							}
							xep_end();

							/*
							 * Drop our reference.
							 */
							bufc_delete(bufc);
						}
						iov = buf_iovec_get(&buf_in,
						    buffer_size, TRUE,
						    &iov_cnt);

						bytes_read = readv(sockfd, iov,
						    iov_cnt);

#ifdef _LIBSOCK_CONFESS
						_cw_out_put("[i|s:s][s]",
						    bytes_read, (0 < bytes_read)
						    ? ", " : ")");
#endif
						_cw_assert(buf_size_get(&tmp_buf)
						    == 0);

						xep_begin();
						xep_try {
							buf_split(&tmp_buf,
							    &buf_in,
							    bytes_read);
						}
						xep_catch(_CW_XEPV_OOM) {
							thd_yield();
							xep_retry();
						}
						xep_end();

						sock_l_in_data_put(regs[sockfd].sock,
						    &tmp_buf);
					} while (bytes_read > 0);

#ifdef _LIBSOCK_CONFESS
					_cw_out_put("c");
#endif

					if (dbg_is_registered(cw_g_dbg,
					    "libsock_verbose")) {
						_cw_out_put_e("POLLHUP.  Closing sockfd [i]\n",
						    sockfd);
					}
					/*
					 * Fill this hole, decrement i,
					 * continue.
					 */
					nfds--;
					if (regs[sockfd].pollfd_pos != nfds) {
#ifdef _LIBSOCK_CONFESS
						_cw_out_put("h([i]-->[i])",
						    nfds, i);
#endif

						regs[fds[nfds].fd].pollfd_pos
						    = i;
						memcpy(&fds[i], &fds[nfds],
						    sizeof(struct pollfd));
						i--;
					}
					regs[sockfd].pollfd_pos = -1;

					sock_l_error_callback(regs[sockfd].sock);

					if (regs[sockfd].notify_mq != NULL) {
						if (libsock_p_notify(regs[sockfd].notify_mq,
						    sockfd)) {
							regs[sockfd].notify_mq
							    = NULL;
						}
					}
#ifdef _LIBSOCK_CONFESS
					_cw_out_put("\n");
#endif
					continue;
				}
				if (fds[i].revents & POLLOUT) {
					const struct iovec *iov;
					int		iov_cnt;
					ssize_t		bytes_written;

					j++;

#ifdef _LIBSOCK_CONFESS
					_cw_out_put("w");
#endif
					/* Ready for writing. */

					/* Get the socket's buf. */
					_cw_assert(buf_size_get(&tmp_buf) == 0);
					sock_l_out_data_get(regs[sockfd].sock,
					    &tmp_buf);

					/* Build an iovec for writing. */
					iov = buf_iovec_get(&tmp_buf,
					    buf_size_get(&tmp_buf), TRUE,
					    &iov_cnt);

					/*
					 * If the buf exceeds the maximum iovec,
					 * it's possible that we'll only write
					 * part of the data, when we could have
					 * written it all.  This is in practice
					 * very unlikely though, and doesn't
					 * cause erroneous behavior.
					 */
					bytes_written = writev(sockfd, iov,
					    iov_cnt);
#ifdef _LIBSOCK_CONFESS
					_cw_out_put("([i|s:s]/[i])",
					    bytes_written,
					    buf_size_get(&tmp_buf));
#endif

					if (bytes_written >= 0) {
						buf_head_data_release(&tmp_buf,
						    bytes_written);

						if (sock_l_out_data_put_back(regs[sockfd].sock,
						    &tmp_buf) == 0) {
							/*
							 * The socket has no
							 * more outgoing data,
							 * so turn the write bit
							 * off in the master
							 * write descriptor set.
							 */
#ifdef _LIBSOCK_CONFESS
							_cw_out_put("u");
#endif
							fds[i].events ^=
							    (fds[i].events &
							    POLLOUT);
						}
#ifdef _LIBSOCK_CONFESS
						else
							_cw_out_put("i");
#endif
					} else {/* if (bytes_written == -1) */
						buf_head_data_release(&tmp_buf,
						    buf_size_get(&tmp_buf));

						if (dbg_is_registered(cw_g_dbg,
						    "libsock_verbose")) {
							_cw_out_put_e("Error in writev(): [s]\n",
							    strerror(errno));
							_cw_out_put_e("Closing sockfd [i]\n",
							    sockfd);
						}
						/*
						 * Fill this hole, decrement i.
						 */
						nfds--;
						if (regs[sockfd].pollfd_pos !=
						    nfds) {
#ifdef _LIBSOCK_CONFESS
							_cw_out_put("h([i]-->[i])",
							    nfds, i);
#endif
							regs[fds[nfds].fd].pollfd_pos
							    = i;
							memcpy(&fds[i],
							    &fds[nfds],
							    sizeof(struct
							    pollfd));
							i--;
						}
						regs[sockfd].pollfd_pos = -1;

						sock_l_error_callback(regs[sockfd].sock);

						if (regs[sockfd].notify_mq != NULL) {
							if (libsock_p_notify(regs[sockfd].notify_mq,
							    sockfd)) {
								regs[sockfd].notify_mq
								    = NULL;
							}
						}
					}
					_cw_assert(buf_size_get(&tmp_buf) == 0);
				}
			}
#ifdef _LIBSOCK_CONFESS
			_cw_out_put("\n");
#endif
		}
	}

	buf_delete(&buf_in);
	buf_delete(&tmp_buf);
	_cw_free(regs);
	_cw_free(fds);
	return NULL;
}
