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
 * Implementation of thread locking primitives.
 *
 * cnd : Condition variable.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_cnd_s cw_cnd_t;

struct cw_cnd_s
{
  cw_bool_t is_malloced;
  pthread_cond_t condition;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cnd : Pointer to space for a cnd, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a cnd.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_cnd_t *
cnd_new(cw_cnd_t * a_cnd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cnd : Pointer to a cnd.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
cnd_delete(cw_cnd_t * a_cnd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cnd : Pointer to a cnd.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Signal one thread waiting on a_cnd.
 *
 ****************************************************************************/
void
cnd_signal(cw_cnd_t * a_cnd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cnd : Pointer to a cnd.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Signal all threads waiting on a_cnd.
 *
 ****************************************************************************/
void
cnd_broadcast(cw_cnd_t * a_cnd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cnd : Pointer to a cnd.
 *
 * a_timeout : Timeout, specified as an absolute time interval.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == timeout.
 *
 * <<< Description >>>
 *
 * Wait for a_cnd for at least a_time.
 *
 ****************************************************************************/
cw_bool_t
cnd_timedwait(cw_cnd_t * a_cnd, cw_mtx_t * a_mtx,
	      const struct timespec * a_timeout);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cnd : Pointer to a cnd.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Wait for a_cnd.
 *
 ****************************************************************************/
void
cnd_wait(cw_cnd_t * a_cnd, cw_mtx_t * a_mtx);
