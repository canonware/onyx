/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Public interface for the mq (message queue) class.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_mq_s cw_mq_t;

struct cw_mq_s
{
  cw_bool_t is_malloced;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
#ifdef _CW_REENTRANT
  cw_mtx_t lock;
  cw_cnd_t cond;
#endif

  cw_bool_t get_stop;
  cw_bool_t put_stop;
  
  cw_ring_t * ring;
  cw_ring_t * spares_ring;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to space for a mq, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_mq_t *
mq_new(cw_mq_t * a_mq);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq.
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
mq_delete(cw_mq_t * a_mq);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a message, or NULL.
 *          NULL : No messages in the queue, or get is in the stop state.
 *
 * <<< Description >>>
 *
 * Try to get a message, but return NULL if none are available.
 *
 ****************************************************************************/
void *
mq_tryget(cw_mq_t * a_mq);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a message, or NULL.
 *          NULL : get is in the stop state.
 *
 * <<< Description >>>
 *
 * Get a message.  If none are available, block until a message is available.
 *
 * Note: This function is only available in the threaded versions of libstash.
 *
 ****************************************************************************/
#ifdef _CW_REENTRANT
void *
mq_get(cw_mq_t * a_mq);
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq.
 *
 * a_message : Pointer to a message.
 *
 * <<< Output(s) >>>
 *
 * retval : -1 == Memory allocation error.
 *           0 == Success.
 *           1 == Failure due to put being in the stop state.
 *
 * <<< Description >>>
 *
 * Put a message in the queue.
 *
 ****************************************************************************/
cw_sint32_t
mq_put(cw_mq_t * a_mq, const void * a_message);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error (already in start state).
 *
 * <<< Description >>>
 *
 * Change the get operation to the start state (mq_get() will not return 1).
 *
 ****************************************************************************/
cw_bool_t
mq_start_get(cw_mq_t * a_mq);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error (already in stop state).
 *
 * <<< Description >>>
 *
 * Change the get operation to the stop state (mq_get() will return 1).
 *
 ****************************************************************************/
cw_bool_t
mq_stop_get(cw_mq_t * a_mq);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error (already in start state).
 *
 * <<< Description >>>
 *
 * Change the put operation to the start state (mq_put() will not return 1).
 *
 ****************************************************************************/
cw_bool_t
mq_start_put(cw_mq_t * a_mq);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error (already in stop state).
 *
 * <<< Description >>>
 *
 * Change the put operation to the stop state (mq_put() will return 1).
 *
 ****************************************************************************/
cw_bool_t
mq_stop_put(cw_mq_t * a_mq);
