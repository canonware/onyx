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
 * thd : Thread.
 *
 * mtx : Mutex.
 *
 * cnd : Condition variable.
 * 
 * sem : Semaphore.  This implementation is a bit different than normal, in 
 * that it is possible to decrement the count to less than zero.  This
 * allows dynamic modification of resource pools locked by semaphores.
 *
 * tsd : Thread-specific data.
 *
 ****************************************************************************/

#ifndef _PTHREAD_H_
#  include <pthread.h>
#  define _PTHREAD_H_
#endif

/* Pseudo-opaque types. */
typedef struct cw_thd_s cw_thd_t;
typedef struct cw_mtx_s cw_mtx_t;
typedef struct cw_cnd_s cw_cnd_t;
typedef struct cw_sem_s cw_sem_t;
typedef struct cw_tsd_s cw_tsd_t;

struct cw_thd_s
{
  cw_bool_t is_malloced;
  pthread_t thread;
};

struct cw_mtx_s
{
  cw_bool_t is_malloced;
  pthread_mutex_t mutex;
};

struct cw_cnd_s
{
  cw_bool_t is_malloced;
  pthread_cond_t condition;
};

struct cw_sem_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_cnd_t gtzero;
  cw_sint32_t count;
  cw_uint32_t waiters;
};

struct cw_tsd_s
{
  cw_bool_t is_malloced;
  pthread_key_t key;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_thd : Pointer to space for a thd, or NULL.
 *
 * a_start_func : Pointer to a start function.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a thd, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor (creates a new thread).
 *
 ****************************************************************************/
cw_thd_t *
thd_new(cw_thd_t * a_thd, void * (*a_start_func)(void *), void * a_arg);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_thd : Pointer to a thd.
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
thd_delete(cw_thd_t * a_thd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_thd : Pointer to a thd.
 *
 * <<< Output(s) >>>
 *
 * retval : Return value from thread entry function.
 *
 * <<< Description >>>
 *
 * Join (wait for) the thread associated with a_thd.
 *
 ****************************************************************************/
void *
thd_join(cw_thd_t * a_thd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Give up the rest of this thread's time slice.
 *
 ****************************************************************************/
#define thd_yield() pthread_yield()

#define thd_sigmask(a, b, c) pthread_sigmask((a), (b), (c))

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * Pointer to space for a mtx, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a mtx.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_mtx_t *
mtx_new(cw_mtx_t * a_mtx);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mtx : Pointer to a mtx.
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
mtx_delete(cw_mtx_t * a_mtx);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mtx : Pointer to a mtx.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Lock a_mtx.
 *
 ****************************************************************************/
void
mtx_lock(cw_mtx_t * a_mtx);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mtx : Pointer to a mtx.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == failure.
 *
 * <<< Description >>>
 *
 * Try to lock a_mtx, but return immediately instead of blocking if a_mtx is
 * already locked.
 *
 ****************************************************************************/
cw_bool_t
mtx_trylock(cw_mtx_t * a_mtx);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mtx : Pointer to a mtx.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Unlock a_mtx.
 *
 ****************************************************************************/
void
mtx_unlock(cw_mtx_t * a_mtx);

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
 * a_time : Timeout.
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
cnd_timedwait(cw_cnd_t * a_cnd, cw_mtx_t * a_mtx, struct timespec * a_time);

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

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_tsd : Pointer to space for a tsd, or NULL.
 *
 * a_func : Pointer to a cleanup function, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a tsd.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_tsd_t *
tsd_new(cw_tsd_t * a_tsd, void (*a_func)(void *));

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_tsd : Pointer to a tsd.
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
tsd_delete(cw_tsd_t * a_tsd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_tsd : Pointer to a tsd.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to thread-specific data.
 *
 * <<< Description >>>
 *
 * Get thread-specific data pointer.
 *
 ****************************************************************************/
void *
tsd_get(cw_tsd_t * a_tsd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_tsd : Pointer to a tsd.
 *
 * a_val : Pointer to thread-specific data.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set thread-specific data pointer.
 *
 ****************************************************************************/
void
tsd_set(cw_tsd_t * a_tsd, void * a_val);
