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
 * $Revision: 69 $
 * $Date: 1998-05-02 02:08:51 -0700 (Sat, 02 May 1998) $
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
