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

#define thd_new _CW_NS_LIBSTASH(thd_new)
cw_thd_t *
thd_new(cw_thd_t * a_thd, void * (*a_start_func)(void *), void * a_arg);

#define thd_delete _CW_NS_LIBSTASH(thd_delete)
void
thd_delete(cw_thd_t * a_thd);

#define thd_join _CW_NS_LIBSTASH(thd_join)
void *
thd_join(cw_thd_t * a_thd);

#define mtx_new _CW_NS_LIBSTASH(mtx_new)
cw_mtx_t *
mtx_new(cw_mtx_t * a_mtx);

#define mtx_delete _CW_NS_LIBSTASH(mtx_delete)
void
mtx_delete(cw_mtx_t * a_mtx);

#define mtx_lock _CW_NS_LIBSTASH(mtx_lock)
void
mtx_lock(cw_mtx_t * a_mtx);

#define mtx_trylock _CW_NS_LIBSTASH(mtx_trylock)
cw_bool_t
mtx_trylock(cw_mtx_t * a_mtx);

#define mtx_unlock _CW_NS_LIBSTASH(mtx_unlock)
void
mtx_unlock(cw_mtx_t * a_mtx);

#define cnd_new _CW_NS_LIBSTASH(cnd_new)
cw_cnd_t *
cnd_new(cw_cnd_t * a_cnd);

#define cnd_delete _CW_NS_LIBSTASH(cnd_delete)
void
cnd_delete(cw_cnd_t * a_cnd);

#define cnd_signal _CW_NS_LIBSTASH(cnd_signal)
void
cnd_signal(cw_cnd_t * a_cnd);

#define cnd_broadcast _CW_NS_LIBSTASH(cnd_broadcast)
void
cnd_broadcast(cw_cnd_t * a_cnd);

#define cnd_timedwait _CW_NS_LIBSTASH(cnd_timedwait)
cw_bool_t
cnd_timedwait(cw_cnd_t * a_cnd, cw_mtx_t * a_mtx, struct timespec * a_time);

#define cnd_wait _CW_NS_LIBSTASH(cnd_wait)
void
cnd_wait(cw_cnd_t * a_cnd, cw_mtx_t * a_mtx);

#define sem_new _CW_NS_LIBSTASH(sem_new)
cw_sem_t *
sem_new(cw_sem_t * a_sem, cw_sint32_t a_count);

#define sem_delete _CW_NS_LIBSTASH(sem_delete)
void
sem_delete(cw_sem_t * a_sem);

#define sem_post _CW_NS_LIBSTASH(sem_post)
void
sem_post(cw_sem_t * a_sem);

#define sem_wait _CW_NS_LIBSTASH(sem_wait)
void
sem_wait(cw_sem_t * a_sem);

#define sem_trywait _CW_NS_LIBSTASH(sem_trywait)
cw_bool_t
sem_trywait(cw_sem_t * a_sem);

#define sem_getvalue _CW_NS_LIBSTASH(sem_getvalue)
cw_sint32_t
sem_getvalue(cw_sem_t * a_sem);

#define sem_adjust _CW_NS_LIBSTASH(sem_adjust)
void
sem_adjust(cw_sem_t * a_sem, cw_sint32_t a_adjust);

#define tsd_new _CW_NS_LIBSTASH(tsd_new)
cw_tsd_t *
tsd_new(cw_tsd_t * a_tsd, void (*a_func)(void *));

#define tsd_delete _CW_NS_LIBSTASH(tsd_delete)
void
tsd_delete(cw_tsd_t * a_tsd);

#define tsd_get _CW_NS_LIBSTASH(tsd_get)
void *
tsd_get(cw_tsd_t * a_tsd);

#define tsd_set _CW_NS_LIBSTASH(tsd_set)
void
tsd_set(cw_tsd_t * a_tsd, void * a_val);
