/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 125 $
 * $Date: 1998-07-02 16:55:52 -0700 (Thu, 02 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Definition of all thread primitives.
 *
 ****************************************************************************/

#ifndef _THREAD_H_
#define _THREAD_H_

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


/* Namespace definitions. */
#define thd_new _CW_NS_CMN(thd_new)
#define thd_delete _CW_NS_CMN(thd_delete)
#define thd_join _CW_NS_CMN(thd_join)

#define mtx_new _CW_NS_CMN(mtx_new)
#define mtx_delete _CW_NS_CMN(mtx_delete)
#define mtx_lock _CW_NS_CMN(mtx_lock)
#define mtx_trylock _CW_NS_CMN(mtx_trylock)
#define mtx_unlock _CW_NS_CMN(mtx_unlock)

#define cnd_new _CW_NS_CMN(cnd_new)
#define cnd_delete _CW_NS_CMN(cnd_delete)
#define cnd_signal _CW_NS_CMN(cnd_signal)
#define cnd_broadcast _CW_NS_CMN(cnd_broadcast)
#define cnd_timedwait _CW_NS_CMN(cnd_timedwait)
#define cnd_wait _CW_NS_CMN(cnd_wait)

#define sem_new _CW_NS_CMN(sem_new)
#define sem_delete _CW_NS_CMN(sem_delete)
#define sem_post _CW_NS_CMN(sem_post)
#define sem_wait _CW_NS_CMN(sem_wait)
#define sem_trywait _CW_NS_CMN(sem_trywait)
#define sem_getvalue _CW_NS_CMN(sem_getvalue)
#define sem_adjust _CW_NS_CMN(sem_adjust)

#define tsd_new _CW_NS_CMN(tsd_new)
#define tsd_delete _CW_NS_CMN(tsd_delete)
#define tsd_get _CW_NS_CMN(tsd_get)
#define tsd_set _CW_NS_CMN(tsd_set)

/* Function prototypes. */
cw_thd_t * thd_new(cw_thd_t * a_thd_o, void * (*a_start_func)(void *),
		   void * a_arg);
void thd_delete(cw_thd_t * a_thd_o);
void * thd_join(cw_thd_t * a_thd_o);

cw_mtx_t * mtx_new(cw_mtx_t * a_mtx_o);
void mtx_delete(cw_mtx_t * a_mtx_o);
void mtx_lock(cw_mtx_t * a_mtx_o);
cw_bool_t mtx_trylock(cw_mtx_t * a_mtx_o);
void mtx_unlock(cw_mtx_t * a_mtx_o);

cw_cnd_t * cnd_new(cw_cnd_t * a_cnd_o);
void cnd_delete(cw_cnd_t * a_cnd_o);
void cnd_signal(cw_cnd_t * a_cnd_o);
void cnd_broadcast(cw_cnd_t * a_cnd_o);
cw_bool_t cnd_timedwait(cw_cnd_t * a_cnd_o, cw_mtx_t * a_mtx_o,
			struct timespec * a_time);
void cnd_wait(cw_cnd_t * a_cnd_o, cw_mtx_t * a_mtx_o);

cw_sem_t * sem_new(cw_sem_t * a_sem_o, cw_sint32_t a_count);
void sem_delete(cw_sem_t * a_sem_o);
void sem_post(cw_sem_t * a_sem_o);
void sem_wait(cw_sem_t * a_sem_o);
cw_bool_t sem_trywait(cw_sem_t * a_sem_o);
cw_sint32_t sem_getvalue(cw_sem_t * a_sem_o);
void sem_adjust(cw_sem_t * a_sem_o, cw_sint32_t a_adjust);

cw_tsd_t * tsd_new(cw_tsd_t * a_tsd_o, void (*a_func)(void *));
void tsd_delete(cw_tsd_t * a_tsd_o);
void * tsd_get(cw_tsd_t * a_tsd_o);
void tsd_set(cw_tsd_t * a_tsd_o, void * a_val);

#endif /* _THREAD_H_ */
