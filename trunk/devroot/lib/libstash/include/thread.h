/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1998
 * Jason Evans <jasone@canonware.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JASON EVANS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL JASON EVANS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 31 $
 * $Date: 1998-04-16 22:55:53 -0700 (Thu, 16 Apr 1998) $
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

/*
 * Opaque types.
 */
typedef struct cw_thd_s cw_thd_t;
typedef struct cw_mtx_s cw_mtx_t;
typedef struct cw_cnd_s cw_cnd_t;
typedef struct cw_sem_s cw_sem_t;
typedef struct cw_rwl_s cw_rwl_t;
typedef struct cw_tsd_s cw_tsd_t;

/*
 * But they aren't really opaque, so that we can automatically allocate them.
 */
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
  cw_cnd_t nonzero;
  cw_uint32_t count;
  cw_uint32_t waiters;
};

struct cw_rwl_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_cnd_t read_wait;
  cw_cnd_t write_wait;
  cw_uint32_t num_readers;
  cw_uint32_t num_writers;
  cw_uint32_t read_waiters;
  cw_uint32_t write_waiters;
};

struct cw_tsd_s
{
  cw_bool_t is_malloced;
  pthread_key_t key;
};

/*
 * Namespace definitions.
 */
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

#define rwl_new _CW_NS_CMN(rwl_new)
#define rwl_delete _CW_NS_CMN(rwl_delete)
#define rwl_rlock _CW_NS_CMN(rwl_rlock)
#define rwl_runlock _CW_NS_CMN(rwl_runlock)
#define rwl_wlock _CW_NS_CMN(rwl_wlock)
#define rwl_wunlock _CW_NS_CMN(rwl_wunlock)

#define tsd_new _CW_NS_CMN(tsd_new)
#define tsd_delete _CW_NS_CMN(tsd_delete)
#define tsd_get _CW_NS_CMN(tsd_get)
#define tsd_set _CW_NS_CMN(tsd_set)

/*
 * Function prototypes.
 */
/* thd : Thread. */
cw_thd_t * thd_new(cw_thd_t * arg_thd_obj, void * (*arg_start_func)(void *),
		   void * arg_arg);
void thd_delete(cw_thd_t * arg_thd_obj);
void * thd_join(cw_thd_t * arg_thd_obj);

/* mtx : Mutex. */
cw_mtx_t * mtx_new(cw_mtx_t * arg_mtx_obj);
void mtx_delete(cw_mtx_t * arg_mtx_obj);
void mtx_lock(cw_mtx_t * arg_mtx_obj);
cw_bool_t mtx_trylock(cw_mtx_t * arg_mtx_obj);
void mtx_unlock(cw_mtx_t * arg_mtx_obj);

/* cnd : Condition variable. */
cw_cnd_t * cnd_new(cw_cnd_t * arg_cnd_obj);
void cnd_delete(cw_cnd_t * arg_cnd_obj);
void cnd_signal(cw_cnd_t * arg_cnd_obj);
void cnd_broadcast(cw_cnd_t * arg_cnd_obj);
cw_bool_t cnd_timedwait(cw_cnd_t * arg_cnd_obj, cw_mtx_t * arg_mtx_obj,
			struct timespec * arg_time);
void cnd_wait(cw_cnd_t * arg_cnd_obj, cw_mtx_t * arg_mtx_obj);

/* sem : Semaphore. */
cw_sem_t * sem_new(cw_sem_t * arg_sem_obj, cw_uint32_t arg_count);
void sem_delete(cw_sem_t * arg_sem_obj);
void sem_post(cw_sem_t * arg_sem_obj);
void sem_wait(cw_sem_t * arg_sem_obj);
cw_bool_t sem_trywait(cw_sem_t * arg_sem_obj);
cw_uint32_t sem_getvalue(cw_sem_t * arg_sem_obj);

/* rwl : Read/write lock.  Multiple readers allowed, but write lock
 * requires exclusive access. */
cw_rwl_t * rwl_new(cw_rwl_t * arg_rwl_obj);
void rwl_delete(cw_rwl_t * arg_rwl_obj);
void rwl_rlock(cw_rwl_t * arg_rwl_obj);
void rwl_runlock(cw_rwl_t * arg_rwl_obj);
void rwl_wlock(cw_rwl_t * arg_rwl_obj);
void rwl_wunlock(cw_rwl_t * arg_rwl_obj);

/* tsd : Thread-specific data. */
cw_tsd_t * tsd_new(cw_tsd_t * arg_tsd_obj, void (*arg_func)(void *));
void tsd_delete(cw_tsd_t * arg_tsd_obj);
void * tsd_get(cw_tsd_t * arg_tsd_obj);
void tsd_set(cw_tsd_t * arg_tsd_obj, void * arg_val);

#endif /* _THREAD_PRIV_H_ */
