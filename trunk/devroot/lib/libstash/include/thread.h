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
 * $Revision: 41 $
 * $Date: 1998-04-26 20:06:13 -0700 (Sun, 26 Apr 1998) $
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
typedef struct cw_lwq_s cw_lwq_t;
typedef struct cw_rwl_s cw_rwl_t;
typedef struct cw_rwq_s cw_rwq_t;
typedef struct cw_tsd_s cw_tsd_t;
typedef struct cw_btl_s cw_btl_t;

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
  cw_cnd_t gtzero;
  cw_sint32_t count;
  cw_uint32_t waiters;
};

struct cw_lwq_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_uint32_t num_lockers;
  cw_uint32_t num_lock_waiters;
  cw_list_t * list;
  cw_list_t * spares_list;
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

struct cw_rwq_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_cnd_t read_wait;
  cw_uint32_t num_readers;
  cw_uint32_t read_waiters;

  cw_lwq_t write_waiters;
};

struct cw_tsd_s
{
  cw_bool_t is_malloced;
  pthread_key_t key;
};

struct cw_btl_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;

  cw_rwq_t stlock;

  cw_uint32_t max_dlocks;
  cw_sem_t dlock_sem;

  cw_rwl_t rxlock;

  cw_sem_t wlock_sem;
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
#define sem_adjust _CW_NS_CMN(sem_adjust)

#define lwq_new _CW_NS_CMN(lwq_new)
#define lwq_delete _CW_NS_CMN(lwq_delete)
#define lwq_lock _CW_NS_CMN(lwq_lock)
#define lwq_unlock _CW_NS_CMN(lwq_unlock)
#define lwq_num_waiters _CW_NS_CMN(lwq_num_waiters)
#define lwq_purge_spares _CW_NS_CMN(lwq_purge_spares)

#define rwl_new _CW_NS_CMN(rwl_new)
#define rwl_delete _CW_NS_CMN(rwl_delete)
#define rwl_rlock _CW_NS_CMN(rwl_rlock)
#define rwl_runlock _CW_NS_CMN(rwl_runlock)
#define rwl_wlock _CW_NS_CMN(rwl_wlock)
#define rwl_wunlock _CW_NS_CMN(rwl_wunlock)

#define rwq_new _CW_NS_CMN(rwq_new)
#define rwq_delete _CW_NS_CMN(rwq_delete)
#define rwq_rlock _CW_NS_CMN(rwq_rlock)
#define rwq_runlock _CW_NS_CMN(rwq_runlock)
#define rwq_wlock _CW_NS_CMN(rwq_wlock)
#define rwq_wunlock _CW_NS_CMN(rwq_wunlock)

#define tsd_new _CW_NS_CMN(tsd_new)
#define tsd_delete _CW_NS_CMN(tsd_delete)
#define tsd_get _CW_NS_CMN(tsd_get)
#define tsd_set _CW_NS_CMN(tsd_set)

#define btl_new _CW_NS_CMN(btl_new)
#define btl_delete _CW_NS_CMN(btl_delete)
#define btl_slock _CW_NS_CMN(btl_slock)
#define btl_tlock _CW_NS_CMN(btl_tlock)
#define btl_s2dlock _CW_NS_CMN(btl_s2dlock)
#define btl_s2rlock _CW_NS_CMN(btl_s2rlock)
#define btl_s2wlock _CW_NS_CMN(btl_s2wlock)
#define btl_s2xlock _CW_NS_CMN(btl_s2xlock)
#define btl_t2rlock _CW_NS_CMN(btl_t2rlock)
#define btl_t2wlock _CW_NS_CMN(btl_t2wlock)
#define btl_t2xlock _CW_NS_CMN(btl_t2xlock)
#define btl_sunlock _CW_NS_CMN(btl_sunlock)
#define btl_tunlock _CW_NS_CMN(btl_tunlock)
#define btl_dunlock _CW_NS_CMN(btl_dunlock)
#define btl_runlock _CW_NS_CMN(btl_runlock)
#define btl_wunlock _CW_NS_CMN(btl_wunlock)
#define btl_xunlock _CW_NS_CMN(btl_xunlock)
#define btl_get_dlocks _CW_NS_CMN(btl_get_dlocks)
#define btl_set_dlocks _CW_NS_CMN(btl_set_dlocks)

/*
 * Function prototypes.
 */
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

cw_lwq_t * lwq_new(cw_lwq_t * a_lwq_o);
void lwq_delete(cw_lwq_t * a_lwq_o);
void lwq_lock(cw_lwq_t * a_lwq_o);
void lwq_unlock(cw_lwq_t * a_lwq_o);
cw_sint32_t lwq_num_waiters(cw_lwq_t * a_lwq_o);
void lwq_purge_spares(cw_lwq_t * a_lwq_o);

cw_rwl_t * rwl_new(cw_rwl_t * a_rwl_o);
void rwl_delete(cw_rwl_t * a_rwl_o);
void rwl_rlock(cw_rwl_t * a_rwl_o);
void rwl_runlock(cw_rwl_t * a_rwl_o);
void rwl_wlock(cw_rwl_t * a_rwl_o);
void rwl_wunlock(cw_rwl_t * a_rwl_o);

cw_rwq_t * rwq_new(cw_rwq_t * a_rwq_o);
void rwq_delete(cw_rwq_t * a_rwq_o);
void rwq_rlock(cw_rwq_t * a_rwq_o);
void rwq_runlock(cw_rwq_t * a_rwq_o);
void rwq_wlock(cw_rwq_t * a_rwq_o);
void rwq_wunlock(cw_rwq_t * a_rwq_o);

cw_tsd_t * tsd_new(cw_tsd_t * a_tsd_o, void (*a_func)(void *));
void tsd_delete(cw_tsd_t * a_tsd_o);
void * tsd_get(cw_tsd_t * a_tsd_o);
void tsd_set(cw_tsd_t * a_tsd_o, void * a_val);

cw_btl_t * btl_new(cw_btl_t * a_btl_o);
void btl_delete(cw_btl_t * a_btl_o);
void btl_slock(cw_btl_t * a_btl_o);
void btl_tlock(cw_btl_t * a_btl_o);
void btl_s2dlock(cw_btl_t * a_btl_o);
void btl_s2rlock(cw_btl_t * a_btl_o);
void btl_s2wlock(cw_btl_t * a_btl_o);
void btl_s2xlock(cw_btl_t * a_btl_o);
void btl_t2rlock(cw_btl_t * a_btl_o);
void btl_t2wlock(cw_btl_t * a_btl_o);
void btl_t2xlock(cw_btl_t * a_btl_o);
void btl_sunlock(cw_btl_t * a_btl_o);
void btl_tunlock(cw_btl_t * a_btl_o);
void btl_dunlock(cw_btl_t * a_btl_o);
void btl_runlock(cw_btl_t * a_btl_o);
void btl_wunlock(cw_btl_t * a_btl_o);
void btl_xunlock(cw_btl_t * a_btl_o);
cw_uint32_t btl_get_dlocks(cw_btl_t * a_btl_o);
cw_uint32_t btl_set_dlocks(cw_btl_t * a_btl_o, cw_uint32_t a_dlocks);

#endif /* _THREAD_H_ */
