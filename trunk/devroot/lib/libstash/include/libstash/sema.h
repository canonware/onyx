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
 * sem : Semaphore.  This implementation is a bit different than normal, in 
 * that it is possible to decrement the count to less than zero.  This
 * allows dynamic modification of resource pools locked by semaphores.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_sem_s cw_sem_t;

struct cw_sem_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_cnd_t gtzero;
  cw_sint32_t count;
  cw_uint32_t waiters;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sem : Pointer to space for a sem, or NULL.
 *
 * a_count : Initial value of semaphore.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a sem.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_sem_t *
sem_new(cw_sem_t * a_sem, cw_sint32_t a_count);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sem : Pointer to a sem.
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
sem_delete(cw_sem_t * a_sem);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sem : Pointer to a sem.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Post (increment) a_sem.
 *
 ****************************************************************************/
void
sem_post(cw_sem_t * a_sem);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sem : Pointer to a sem.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Wait on (block until able to decrement) a_sem.
 *
 ****************************************************************************/
void
sem_wait(cw_sem_t * a_sem);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sem : Pointer to a sem.
 *
 * a_timeout : Timeout, specified as an absolute time interval.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == failure.
 *
 * <<< Description >>>
 *
 * Try to wait on (decrement) a_sem, but return immediately instead of blocking
 * if unable to.
 *
 ****************************************************************************/
cw_bool_t
sem_timedwait(cw_sem_t * a_sem, struct timespec * a_timeout);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sem : Pointer to a sem.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == failure.
 *
 * <<< Description >>>
 *
 * Try to wait on (decrement) a_sem, but return immediately instead of blocking
 * if unable to.
 *
 ****************************************************************************/
cw_bool_t
sem_trywait(cw_sem_t * a_sem);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sem : Pointer to a sem.
 *
 * <<< Output(s) >>>
 *
 * retval : Value of semaphore.
 *
 * <<< Description >>>
 *
 * Get value of a_sem.
 *
 ****************************************************************************/
cw_sint32_t
sem_getvalue(cw_sem_t * a_sem);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sem : Pointer to a sem.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Adjust a_sem by a_adjust.
 *
 ****************************************************************************/
void
sem_adjust(cw_sem_t * a_sem, cw_sint32_t a_adjust);
