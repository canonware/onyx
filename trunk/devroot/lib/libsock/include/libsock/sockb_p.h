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
 * Private data structures and methods for sockb.
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

  cw_bufpool_t bufel_pool;
  cw_bufpool_t bufc_pool;
  cw_bufpool_t buffer_pool;
  
  cw_list_t registrations;
  cw_list_t unregistrations;
  cw_list_t out_notifications;
  cw_mtx_t get_ip_addr_lock;
};

static void *
sockb_p_entry_func(void * a_arg);

static void
sockb_p_select_return(void);
