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

struct cw_sockb_el_s
{
  cw_sock_t * sock;
};

struct cw_sockb_s
{
  cw_bool_t should_quit;
  cw_thd_t thread;
  int poser_fd;
  int pipe_in;
  int pipe_out;
  cw_sem_t pipe_sem;

  cw_pezz_t bufel_pool;
  cw_pezz_t bufc_pool;
  cw_pezz_t buffer_pool;
  
  cw_list_t registrations;
  cw_list_t unregistrations;
  cw_list_t out_notifications;
  cw_mtx_t get_ip_addr_lock;
};

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
