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
 * $Revision: 28 $
 * $Date: 1998-04-13 01:22:55 -0700 (Mon, 13 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_THREAD_H_
#define _INC_THREAD_PRIV_H_
#define _INC_PTHREAD_H_
#include <config.h>

cw_thd_t *
thd_new(cw_thd_t * arg_thd_obj,
	void * (*arg_start_func)(void *),
	void * arg_arg)
{
  cw_thd_t * retval;
  int error;

  if (arg_thd_obj == NULL)
  {
    retval = (cw_thd_t *) _cw_malloc(sizeof(cw_thd_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = arg_thd_obj;
    retval->is_malloced = FALSE;
  }

  error = pthread_create(&retval->thread, NULL, arg_start_func, arg_arg);

  if (error)
  {
    log_eprintf(g_log_obj, __FILE__, __LINE__, "thd_new",
		"Cannot create thread, error %d\n", error);
    abort();
  }

/*   error = pthread_detach(retval->thread); */

/*   if (error) */
/*   { */
/*     log_eprintf(g_log_obj, __FILE__, __LINE__, "thd_new", */
/* 		"Cannot detach thread, error %d\n", error); */
/*     abort(); */
/*   } */
  
  return retval;
}

void
thd_delete(cw_thd_t * arg_thd_obj)
{
  _cw_check_ptr(arg_thd_obj);

  pthread_detach(arg_thd_obj->thread);

  if (arg_thd_obj->is_malloced == TRUE)
  {
    _cw_free(arg_thd_obj);
  }
}

void *
thd_join(cw_thd_t * arg_thd_obj)
{
  void * retval;
  
  _cw_check_ptr(arg_thd_obj);

  pthread_join(arg_thd_obj->thread, &retval);

  return retval;
}

cw_mtx_t *
mtx_new(cw_mtx_t * arg_mtx_obj)
{
  cw_mtx_t * retval;
  int error;

  if (arg_mtx_obj == NULL)
  {
    retval = (cw_mtx_t *) _cw_malloc(sizeof(cw_mtx_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = arg_mtx_obj;
    retval->is_malloced = FALSE;
  }

  error = pthread_mutex_init(&retval->mutex, NULL);

  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "mtx_new",
	       "Unable to create mutex, error %d\n", error);
    abort();
  }

  return retval;
}

void
mtx_delete(cw_mtx_t * arg_mtx_obj)
{
  int error;
  
  _cw_check_ptr(arg_mtx_obj);

  error = pthread_mutex_destroy(&arg_mtx_obj->mutex);

  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "mtx_delete",
	       "Unable to destroy mutex, error %d\n", error);
    abort();
  }

  if (arg_mtx_obj->is_malloced == TRUE)
  {
    _cw_free(arg_mtx_obj);
  }
}

void
mtx_lock(cw_mtx_t * arg_mtx_obj)
{
  int error;
  
  _cw_check_ptr(arg_mtx_obj);

  error = pthread_mutex_lock(&arg_mtx_obj->mutex);

  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "mtx_lock",
	       "Error locking mutex, error %d\n", error);
    abort();
  }
}

cw_bool_t
mtx_trylock(cw_mtx_t * arg_mtx_obj)
{
  cw_bool_t retval;
  
  _cw_check_ptr(arg_mtx_obj);

  retval = pthread_mutex_trylock(&arg_mtx_obj->mutex);

  return retval;
}

void
mtx_unlock(cw_mtx_t * arg_mtx_obj)
{
  int error;
  
  _cw_check_ptr(arg_mtx_obj);

  error = pthread_mutex_unlock(&arg_mtx_obj->mutex);
  
  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "mtx_unlock",
	       "Error unlocking mutex, error %d\n", error);
    abort();
  }
}

cw_cnd_t *
cnd_new(cw_cnd_t * arg_cnd_obj)
{
  cw_cnd_t * retval;
  int error;

  if (arg_cnd_obj == NULL)
  {
    retval = (cw_cnd_t *) _cw_malloc(sizeof(cw_cnd_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = arg_cnd_obj;
    retval->is_malloced = FALSE;
  }

  error = pthread_cond_init(&retval->condition, NULL);

  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "cnd_new",
	       "Error creating condition variable, error %d\n", error);
    abort();
  }

  return retval;
}

void
cnd_delete(cw_cnd_t * arg_cnd_obj)
{
  int error;

  _cw_check_ptr(arg_cnd_obj);

  error = pthread_cond_destroy(&arg_cnd_obj->condition);
  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "cnd_delete",
	       "Error destroying condition variable, error %d\n", error);
    abort();
  }

  if (arg_cnd_obj->is_malloced == TRUE)
  {
    _cw_free(arg_cnd_obj);
  }
}

void
cnd_signal(cw_cnd_t * arg_cnd_obj)
{
  int error;

  _cw_check_ptr(arg_cnd_obj);

  pthread_cond_signal(&arg_cnd_obj->condition);
  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "cnd_signal",
	       "Error signalling condition variable, error %d\n", error);
    abort();
  }
}

void
cnd_broadcast(cw_cnd_t * arg_cnd_obj)
{
  int error;

  _cw_check_ptr(arg_cnd_obj);

  pthread_cond_broadcast(&arg_cnd_obj->condition);
  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "cnd_broadcast",
	       "Error broadcasting condition variable, error %d\n", error);
    abort();
  }
}

cw_bool_t
cnd_timedwait(cw_cnd_t * arg_cnd_obj, cw_mtx_t * arg_mtx_obj,
	      struct timespec * arg_time)
{
  int error;

  _cw_check_ptr(arg_cnd_obj);
  _cw_check_ptr(arg_mtx_obj);

  pthread_cond_timedwait(&arg_cnd_obj->condition, &arg_mtx_obj->mutex,
			 arg_time);
  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "cnd_timedwait",
	       "Error timed waiting on condition variable, error %d\n", error);
    abort();
  }

  /* XXX Should we indicate whether a timeout occurred? */
  return FALSE;
}

void
cnd_wait(cw_cnd_t * arg_cnd_obj, cw_mtx_t * arg_mtx_obj)
{
  int error;

  _cw_check_ptr(arg_cnd_obj);
  _cw_check_ptr(arg_mtx_obj);

  pthread_cond_wait(&arg_cnd_obj->condition, &arg_mtx_obj->mutex);
  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "cnd_wait",
	       "Error waiting on condition variable, error %d\n", error);
    abort();
  }
}

cw_sem_t *
sem_new(cw_sem_t * arg_sem_obj, cw_uint32_t arg_count)
{
  cw_sem_t * retval;

  _cw_assert(arg_count >= 0);

  if (arg_sem_obj == NULL)
  {
    retval = (cw_sem_t *) _cw_malloc(sizeof(cw_sem_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = arg_sem_obj;
    retval->is_malloced = FALSE;
  }

  retval->count = 0;
  retval->waiters = 0;

  mtx_new(&retval->lock);
  cnd_new(&retval->nonzero);
  
  return retval;
}

void
sem_delete(cw_sem_t * arg_sem_obj)
{
  _cw_check_ptr(arg_sem_obj);

  mtx_delete(&arg_sem_obj->lock);
  cnd_delete(&arg_sem_obj->nonzero);

  if (arg_sem_obj->is_malloced == TRUE)
  {
    _cw_free(arg_sem_obj);
  }
}

void
sem_post(cw_sem_t * arg_sem_obj)
{
  _cw_check_ptr(arg_sem_obj);

  mtx_lock(&arg_sem_obj->lock);

  if (arg_sem_obj->waiters)
  {
    cnd_signal(&arg_sem_obj->nonzero);
  }
  arg_sem_obj++;

  mtx_unlock(&arg_sem_obj->lock);
}

void
sem_wait(cw_sem_t * arg_sem_obj)
{
  _cw_check_ptr(arg_sem_obj);

  mtx_lock(&arg_sem_obj->lock);

  while (arg_sem_obj->count == 0)
  {
    arg_sem_obj->waiters++;
    cnd_wait(&arg_sem_obj->nonzero, &arg_sem_obj->lock);
    arg_sem_obj->waiters--;
  }
  arg_sem_obj->count--;

  mtx_unlock(&arg_sem_obj->lock);
}

cw_bool_t
sem_trywait(cw_sem_t * arg_sem_obj)
{
  cw_bool_t retval;

  _cw_check_ptr(arg_sem_obj);

  mtx_lock(&arg_sem_obj->lock);
  
  if (arg_sem_obj->count > 0)
  {
    /* Success. */
    arg_sem_obj->count--;
    retval = FALSE;
  }
  else
  {
    /* Fail. */
    retval = TRUE;
  }

  mtx_unlock(&arg_sem_obj->lock);
  
  return retval;
}

cw_uint32_t
sem_getvalue(cw_sem_t * arg_sem_obj)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(arg_sem_obj);

  /* I don't think we need to lock, since we're just reading. */
/*   mtx_lock(&arg_sem_obj->lock); */

  retval = arg_sem_obj->count;

/*   mtx_unlock(&arg_sem_obj->lock); */
  return retval;
}

cw_rwl_t *
rwl_new(cw_rwl_t * arg_rwl_obj)
{
  cw_rwl_t * retval;

  if (arg_rwl_obj == NULL)
  {
    retval = (cw_rwl_t *) _cw_malloc(sizeof(cw_rwl_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = arg_rwl_obj;
    retval->is_malloced = FALSE;
  }

  mtx_new(&retval->lock);
  cnd_new(&retval->read_wait);
  cnd_new(&retval->write_wait);

  retval->readers = 0;
  retval->writers = 0;
  retval->read_waiters = 0;
  retval->write_waiters = 0;
  
  return retval;
}

void
rwl_delete(cw_rwl_t * arg_rwl_obj)
{
  _cw_check_ptr(arg_rwl_obj);

  mtx_delete(&arg_rwl_obj->lock);
  cnd_delete(&arg_rwl_obj->read_wait);
  cnd_delete(&arg_rwl_obj->write_wait);

  if (arg_rwl_obj->is_malloced)
  {
    _cw_free(arg_rwl_obj);
  }
}

void
rwl_rlock(cw_rwl_t * arg_rwl_obj)
{
  _cw_check_ptr(arg_rwl_obj);

  mtx_lock(&arg_rwl_obj->lock);

  while (arg_rwl_obj->writers > 0)
  {
    arg_rwl_obj->read_waiters++;
    cnd_wait(&arg_rwl_obj->read_wait, &arg_rwl_obj->lock);
    arg_rwl_obj->read_waiters--;
  }
  arg_rwl_obj->readers++;
  
  mtx_unlock(&arg_rwl_obj->lock);
}

void
rwl_runlock(cw_rwl_t * arg_rwl_obj)
{
  _cw_check_ptr(arg_rwl_obj);

  mtx_lock(&arg_rwl_obj->lock);

  arg_rwl_obj->readers--;

  if ((arg_rwl_obj->readers == 0) && (arg_rwl_obj->write_waiters > 0))
  {
    cnd_signal(&arg_rwl_obj->write_wait);
  }
  
  mtx_unlock(&arg_rwl_obj->lock);
}

void
rwl_wlock(cw_rwl_t * arg_rwl_obj)
{
  _cw_check_ptr(arg_rwl_obj);

  mtx_lock(&arg_rwl_obj->lock);

  while (arg_rwl_obj->readers > 0)
  {
    arg_rwl_obj->write_waiters++;
    cnd_wait(&arg_rwl_obj->write_wait, &arg_rwl_obj->lock);
    arg_rwl_obj->write_waiters--;
  }
  arg_rwl_obj->writers++;
  
  mtx_unlock(&arg_rwl_obj->lock);
}

void
rwl_wunlock(cw_rwl_t * arg_rwl_obj)
{
  _cw_check_ptr(arg_rwl_obj);

  mtx_lock(&arg_rwl_obj->lock);

  arg_rwl_obj->writers--;

  if (arg_rwl_obj->write_waiters > 0)
  {
    cnd_signal(&arg_rwl_obj->write_wait);
  }
  else if (arg_rwl_obj->read_waiters > 0)
  {
    cnd_broadcast(&arg_rwl_obj->read_wait);
  }
  
  mtx_unlock(&arg_rwl_obj->lock);
}

cw_tsd_t *
tsd_new(cw_tsd_t * arg_tsd_obj, void (*arg_func)(void *))
{
  cw_tsd_t * retval;

  if (arg_tsd_obj == NULL)
  {
    retval = (cw_tsd_t *) _cw_malloc(sizeof(cw_tsd_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = arg_tsd_obj;
    retval->is_malloced = FALSE;
  }

  pthread_key_create(&retval->key, arg_func);
  
  return retval;
}

void
tsd_delete(cw_tsd_t * arg_tsd_obj)
{
  _cw_check_ptr(arg_tsd_obj);

  pthread_key_delete(arg_tsd_obj->key);

  if (arg_tsd_obj->is_malloced == TRUE)
  {
    _cw_free(arg_tsd_obj);
  }
}

void *
tsd_get(cw_tsd_t * arg_tsd_obj)
{
  void * retval;
  
  _cw_check_ptr(arg_tsd_obj);

  retval = pthread_getspecific(arg_tsd_obj->key);
  
  return retval;
}

void
tsd_set(cw_tsd_t * arg_tsd_obj, void * arg_val)
{
  int error;
  
  _cw_check_ptr(arg_tsd_obj);

  error = pthread_setspecific(arg_tsd_obj->key, arg_val);
  if (error)
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "tsd_set",
	       "Error setting thread-specific data, error %d\n", error);
    abort();
  }
}
