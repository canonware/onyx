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
 * Private data structures and methods for the sockb class.
 *
 ****************************************************************************/

#ifdef _LIBSOCK_DBG
#  define _LIBSOCK_SOCKB_MSG_MAGIC 0xf0010725
#endif

struct cw_sockb_msg_s
{
#ifdef _LIBSOCK_DBG
  cw_uint32_t magic;
#endif
  enum {REGISTER, UNREGISTER, OUT_NOTIFY, IN_SPACE, IN_NOTIFY} type;
  union
  {
    cw_sock_t * sock;
    cw_uint32_t sockfd;
    struct
    {
      int sockfd;
      cw_mq_t * mq;
      cw_mtx_t * mtx;
      cw_cnd_t * cnd;
    } in_notify;
  } data;
};

struct cw_sockb_reg_s
{
  cw_sock_t * sock; /* sock pointer. */
  cw_sint32_t pollfd_pos; /* Offset in the pollfd struct passed into poll(). */
  cw_mq_t * notify_mq; /* mq to notify when readable or closed (or NULL). */
};

struct cw_sockb_entry_s
{
  cw_uint32_t max_fds;
  struct cw_sockb_reg_s * regs;
  struct pollfd * fds;
};

struct cw_sockb_s
{
  cw_bool_t should_quit;
  cw_thd_t thread;
  int pipe_in;
  int pipe_out;
  cw_sem_t pipe_sem;

  cw_pezz_t bufc_pool;
  cw_pezz_t buffer_pool;

  cw_pezz_t messages_pezz;
  cw_mq_t messages;
  cw_cnd_t wait_cnd;
  
  cw_mtx_t get_ip_addr_lock;
};

static cw_bool_t
sockb_p_notify(cw_mq_t * a_mq, int a_sockfd);

/****************************************************************************
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
 * executes a select() loop and communicates with other threads via message
 * queues (list's).
 *
 ****************************************************************************/
static void *
sockb_p_entry_func(void * a_arg);

