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
 ****************************************************************************/

#include "libstash/libstash_r.h"

#include <errno.h>

cw_thd_t *
thd_new(cw_thd_t * a_thd,
	void * (*a_start_func)(void *),
	void * a_arg)
{
  cw_thd_t * retval;
  int error;

  if (a_thd == NULL)
  {
    retval = (cw_thd_t *) _cw_malloc(sizeof(cw_thd_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_thd;
    retval->is_malloced = FALSE;
  }

  error = pthread_create(&retval->thread, NULL, a_start_func, a_arg);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_create(): [s]\n", strerror(error));
    abort();
  }

  RETURN:
  return retval;
}

void
thd_delete(cw_thd_t * a_thd)
{
  int error;
  
  _cw_check_ptr(a_thd);

  error = pthread_detach(a_thd->thread);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_detach(): [s]\n", strerror(error));
    abort();
  }
  
  if (a_thd->is_malloced == TRUE)
  {
    _cw_free(a_thd);
  }
}

void *
thd_join(cw_thd_t * a_thd)
{
  void * retval;
  int error;
  
  _cw_check_ptr(a_thd);

  error = pthread_join(a_thd->thread, &retval);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_join(): [s]\n", strerror(error));
    abort();
  }

  if (a_thd->is_malloced == TRUE)
  {
    _cw_free(a_thd);
  }
  return retval;
}

cw_mtx_t *
mtx_new(cw_mtx_t * a_mtx)
{
  cw_mtx_t * retval;
  int error;

  if (a_mtx == NULL)
  {
    retval = (cw_mtx_t *) _cw_malloc(sizeof(cw_mtx_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_mtx;
    retval->is_malloced = FALSE;
  }

  error = pthread_mutex_init(&retval->mutex, NULL);
  if (error)
  {
    out_put_e(NULL, NULL, 0, __FUNCTION__,
	      "Error in pthread_mutex_init: [s]\n", strerror(error));
    abort();
  }

  RETURN:
  return retval;
}

void
mtx_delete(cw_mtx_t * a_mtx)
{
  int error;
  
  _cw_check_ptr(a_mtx);

  error = pthread_mutex_destroy(&a_mtx->mutex);
  if (error)
  {
    out_put_e(NULL, NULL, 0, __FUNCTION__,
	      "Error in pthread_mutex_destroy(): [s]\n", strerror(error));
    abort();
  }

  if (a_mtx->is_malloced == TRUE)
  {
    _cw_free(a_mtx);
  }
}

void
mtx_lock(cw_mtx_t * a_mtx)
{
  int error;
  
  _cw_check_ptr(a_mtx);
  
  error = pthread_mutex_lock(&a_mtx->mutex);
  if (error)
  {
    out_put_e(NULL, NULL, 0, __FUNCTION__,
	      "Error in pthread_mutex_lock(): [s]\n", strerror(error));
    abort();
  }
}

cw_bool_t
mtx_trylock(cw_mtx_t * a_mtx)
{
  cw_bool_t retval;
  int error;
  
  _cw_check_ptr(a_mtx);

  error = pthread_mutex_trylock(&a_mtx->mutex);
  if (error == 0)
  {
    retval = FALSE;
  }
  else if (error == EBUSY)
  {
    retval = TRUE;
  }
  else 
  {
    out_put_e(NULL, NULL, 0, __FUNCTION__,
	      "Error in pthread_mutex_trylock(): [s]\n", strerror(error));
    abort();
  }

  return retval;
}

void
mtx_unlock(cw_mtx_t * a_mtx)
{
  int error;
  
  _cw_check_ptr(a_mtx);

  error = pthread_mutex_unlock(&a_mtx->mutex);
  if (error)
  {
    out_put_e(NULL, NULL, 0, __FUNCTION__,
	      "Error in pthread_mutex_unlock(): [s]\n", strerror(error));
    abort();
  }
}

cw_cnd_t *
cnd_new(cw_cnd_t * a_cnd)
{
  cw_cnd_t * retval;
  int error;

  if (a_cnd == NULL)
  {
    retval = (cw_cnd_t *) _cw_malloc(sizeof(cw_cnd_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_cnd;
    retval->is_malloced = FALSE;
  }

  error = pthread_cond_init(&retval->condition, NULL);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_cond_init(): [s]\n", strerror(error));
    abort();
  }

  RETURN:
  return retval;
}

void
cnd_delete(cw_cnd_t * a_cnd)
{
  int error;

  _cw_check_ptr(a_cnd);

  error = pthread_cond_destroy(&a_cnd->condition);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_cond_destroy(): [s]\n", strerror(error));
    abort();
  }

  if (a_cnd->is_malloced == TRUE)
  {
    _cw_free(a_cnd);
  }
}

void
cnd_signal(cw_cnd_t * a_cnd)
{
  int error;

  _cw_check_ptr(a_cnd);

  error = pthread_cond_signal(&a_cnd->condition);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_cond_signal(): [s]\n", strerror(error));
    abort();
  }
}

void
cnd_broadcast(cw_cnd_t * a_cnd)
{
  int error;

  _cw_check_ptr(a_cnd);

  error = pthread_cond_broadcast(&a_cnd->condition);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_cond_broadcast(): [s]\n", strerror(error));
    abort();
  }
}

cw_bool_t
cnd_timedwait(cw_cnd_t * a_cnd, cw_mtx_t * a_mtx,
	      struct timespec * a_timeout)
{
  int error;
  cw_bool_t retval;
  struct timeval now;
  struct timespec timeout;
  struct timezone tz;

  _cw_check_ptr(a_cnd);
  _cw_check_ptr(a_mtx);
  _cw_check_ptr(a_timeout);

  /* Set timeout. */
  bzero(&tz, sizeof(struct timezone));
  gettimeofday(&now, &tz);
  timeout.tv_nsec = now.tv_usec * 1000 + a_timeout->tv_nsec;
  timeout.tv_sec = (now.tv_sec + a_timeout->tv_sec
		    + (timeout.tv_nsec / 1000000000)); /* Carry if nanoseconds
							* overflowed. */
  /* Chop off the number of nanoseconds to be less than one second. */
  timeout.tv_nsec %= 1000000000;
  
  error = pthread_cond_timedwait(&a_cnd->condition, &a_mtx->mutex, &timeout);
  if (error == 0)
  {
    retval = FALSE;
  }
  else if (error == ETIMEDOUT)
  {
    retval = TRUE;
  }
  else
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_cond_timedwait(): [s]\n", strerror(error));
    abort();
  }
  
  return retval;
}

void
cnd_wait(cw_cnd_t * a_cnd, cw_mtx_t * a_mtx)
{
  int error;

  _cw_check_ptr(a_cnd);
  _cw_check_ptr(a_mtx);

  error = pthread_cond_wait(&a_cnd->condition, &a_mtx->mutex);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_cond_wait: [s]\n", strerror(error));
    abort();
  }
}

cw_sem_t *
sem_new(cw_sem_t * a_sem, cw_sint32_t a_count)
{
  cw_sem_t * retval;

  if (a_sem == NULL)
  {
    retval = (cw_sem_t *) _cw_malloc(sizeof(cw_sem_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_sem;
    retval->is_malloced = FALSE;
  }

  retval->count = a_count;
  retval->waiters = 0;

  mtx_new(&retval->lock);
  cnd_new(&retval->gtzero);

  RETURN:
  return retval;
}

void
sem_delete(cw_sem_t * a_sem)
{
  _cw_check_ptr(a_sem);

  mtx_delete(&a_sem->lock);
  cnd_delete(&a_sem->gtzero);

  if (a_sem->is_malloced == TRUE)
  {
    _cw_free(a_sem);
  }
}

void
sem_post(cw_sem_t * a_sem)
{
  _cw_check_ptr(a_sem);

  mtx_lock(&a_sem->lock);

  a_sem->count++;
  if ((a_sem->waiters) && (a_sem->count > 0))
  {
    cnd_signal(&a_sem->gtzero);
  }

  mtx_unlock(&a_sem->lock);
}

void
sem_wait(cw_sem_t * a_sem)
{
  _cw_check_ptr(a_sem);

  mtx_lock(&a_sem->lock);

  while (a_sem->count <= 0)
  {
    a_sem->waiters++;
    cnd_wait(&a_sem->gtzero, &a_sem->lock);
    a_sem->waiters--;
  }
  a_sem->count--;

  mtx_unlock(&a_sem->lock);
}

cw_bool_t
sem_timedwait(cw_sem_t * a_sem, struct timespec * a_timeout)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sem);
  _cw_check_ptr(a_timeout);

  mtx_lock(&a_sem->lock);

  if (0 >= a_sem->count)
  {
    a_sem->waiters++;
    cnd_timedwait(&a_sem->gtzero, &a_sem->lock, a_timeout);
    a_sem->waiters--;
  }
  
  if (0 < a_sem->count)
  {
    a_sem->count--;
    retval = FALSE;
  }
  else
  {
    retval = TRUE;
  }

  mtx_unlock(&a_sem->lock);

  return retval;
}

cw_bool_t
sem_trywait(cw_sem_t * a_sem)
{
  cw_bool_t retval;

  _cw_check_ptr(a_sem);

  mtx_lock(&a_sem->lock);
  
  if (a_sem->count > 0)
  {
    /* Success. */
    a_sem->count--;
    retval = FALSE;
  }
  else
  {
    /* Fail. */
    retval = TRUE;
  }

  mtx_unlock(&a_sem->lock);
  
  return retval;
}

cw_sint32_t
sem_getvalue(cw_sem_t * a_sem)
{
  cw_sint32_t retval;
  
  _cw_check_ptr(a_sem);

  mtx_lock(&a_sem->lock);
  retval = a_sem->count;
  mtx_unlock(&a_sem->lock);
  
  return retval;
}

void
sem_adjust(cw_sem_t * a_sem, cw_sint32_t a_adjust)
{
  _cw_check_ptr(a_sem);

  mtx_lock(&a_sem->lock);

  a_sem->count += a_adjust;
  if ((a_sem->waiters) && (a_sem->count > 0))
  {
    cw_sint32_t i;
    
    for (i = 0; (i < a_sem->count) && (i < a_sem->waiters); i++)
    {
      cnd_signal(&a_sem->gtzero);
    }
  }
  
  mtx_unlock(&a_sem->lock);
}

cw_tsd_t *
tsd_new(cw_tsd_t * a_tsd, void (*a_func)(void *))
{
  cw_tsd_t * retval;
  int error;

  if (a_tsd == NULL)
  {
    retval = (cw_tsd_t *) _cw_malloc(sizeof(cw_tsd_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_tsd;
    retval->is_malloced = FALSE;
  }

  error = pthread_key_create(&retval->key, a_func);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_key_create(): [s]\n", strerror(error));
    abort();
  }

  RETURN:
  return retval;
}

void
tsd_delete(cw_tsd_t * a_tsd)
{
  int error;
  
  _cw_check_ptr(a_tsd);

  error = pthread_key_delete(a_tsd->key);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_key_delete(): [s]\n", strerror(error));
    abort();
  }

  if (a_tsd->is_malloced == TRUE)
  {
    _cw_free(a_tsd);
  }
}

void *
tsd_get(cw_tsd_t * a_tsd)
{
  void * retval;
  
  _cw_check_ptr(a_tsd);

  retval = pthread_getspecific(a_tsd->key);
  
  return retval;
}

void
tsd_set(cw_tsd_t * a_tsd, void * a_val)
{
  int error;
  
  _cw_check_ptr(a_tsd);

  error = pthread_setspecific(a_tsd->key, a_val);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_setspecific(): [s]\n", strerror(error));
    abort();
  }
}
