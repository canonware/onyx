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

/*  struct cw_sockb_el_s */
/*  { */
/*    cw_sock_t * sock; */
/*  }; */

#ifdef _LIBSOCK_DBG
#  define _LIBSOCK_SOCKB_MSG_MAGIC 0xf0010725
#endif

struct cw_sockb_msg_s
{
#ifdef _LIBSOCK_DBG
  cw_uint32_t magic;
#endif
  enum {REGISTER, UNREGISTER, OUT_NOTIFY, WAIT, UNWAIT} type;
  union
  {
    cw_sock_t * sock;
    cw_uint32_t sockfd;
    struct
    {
      cw_cnd_t * cnd;
      cw_mtx_t * mtx;
      int * fd_array;
      cw_uint32_t nfds;
    } wait;
    struct
    {
      struct cw_sockb_msg_s * old_msg_p;
      cw_cnd_t * cnd;
      cw_mtx_t * mtx;
    } unwait;
  } data;
};

struct cw_sockb_s
{
  cw_bool_t should_quit;
  cw_thd_t thread;
  int poser_fd;
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

static void
sockb_p_wait_notify(struct cw_sockb_msg_s ** a_wait_vector,
		    struct cw_sockb_msg_s * a_msg);

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

