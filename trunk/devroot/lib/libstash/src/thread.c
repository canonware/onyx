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
 * $Revision: 213 $
 * $Date: 1998-09-08 20:22:29 -0700 (Tue, 08 Sep 1998) $
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

#include <libstash.h>

#include <string.h>

cw_thd_t *
thd_new(cw_thd_t * a_thd_o,
	void * (*a_start_func)(void *),
	void * a_arg)
{
  cw_thd_t * retval;
  int error;

  if (a_thd_o == NULL)
  {
    retval = (cw_thd_t *) _cw_malloc(sizeof(cw_thd_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_thd_o;
    retval->is_malloced = FALSE;
  }

  error = pthread_create(&retval->thread, NULL, a_start_func, a_arg);

  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "thd_new",
		"Cannot create thread: %s\n", strerror(error));
    abort();
  }

  /*   error = pthread_detach(retval->thread); */

  /*   if (error) */
  /*   { */
  /*     log_eprintf(g_log_o, NULL, 0, "thd_new", */
  /* 		"Cannot detach thread: %s\n", strerror(error)); */
  /*     abort(); */
  /*   } */
  
  return retval;
}

void
thd_delete(cw_thd_t * a_thd_o)
{
  _cw_check_ptr(a_thd_o);

  pthread_detach(a_thd_o->thread);

  if (a_thd_o->is_malloced == TRUE)
  {
    _cw_free(a_thd_o);
  }
}

void *
thd_join(cw_thd_t * a_thd_o)
{
  void * retval;
  
  _cw_check_ptr(a_thd_o);

  pthread_join(a_thd_o->thread, &retval);

  return retval;
}

cw_mtx_t *
mtx_new(cw_mtx_t * a_mtx_o)
{
  cw_mtx_t * retval;
  int error;

  if (a_mtx_o == NULL)
  {
    retval = (cw_mtx_t *) _cw_malloc(sizeof(cw_mtx_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_mtx_o;
    retval->is_malloced = FALSE;
  }

  error = pthread_mutex_init(&retval->mutex, NULL);

  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "mtx_new",
		"Unable to create mutex: %s\n", strerror(error));
    abort();
  }

  return retval;
}

void
mtx_delete(cw_mtx_t * a_mtx_o)
{
  int error;
  
  _cw_check_ptr(a_mtx_o);

  error = pthread_mutex_destroy(&a_mtx_o->mutex);

  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "mtx_delete",
		"Unable to destroy mutex: %s\n", strerror(error));
    abort();
  }

  if (a_mtx_o->is_malloced == TRUE)
  {
    _cw_free(a_mtx_o);
  }
}

void
mtx_lock(cw_mtx_t * a_mtx_o)
{
  int error;
  
  _cw_check_ptr(a_mtx_o);

  error = pthread_mutex_lock(&a_mtx_o->mutex);

  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "mtx_lock",
		"Error locking mutex: %s\n", strerror(error));
    abort();
  }
}

cw_bool_t
mtx_trylock(cw_mtx_t * a_mtx_o)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_mtx_o);

  if (EBUSY == pthread_mutex_trylock(&a_mtx_o->mutex))
  {
    retval = TRUE;
  }
  else 
  {
    retval = FALSE;
  }

  return retval;
}

void
mtx_unlock(cw_mtx_t * a_mtx_o)
{
  int error;
  
  _cw_check_ptr(a_mtx_o);

  error = pthread_mutex_unlock(&a_mtx_o->mutex);
  
  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "mtx_unlock",
		"Error unlocking mutex: %s\n", strerror(error));
    abort();
  }
}

cw_cnd_t *
cnd_new(cw_cnd_t * a_cnd_o)
{
  cw_cnd_t * retval;
  int error;

  if (a_cnd_o == NULL)
  {
    retval = (cw_cnd_t *) _cw_malloc(sizeof(cw_cnd_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_cnd_o;
    retval->is_malloced = FALSE;
  }

  error = pthread_cond_init(&retval->condition, NULL);

  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "cnd_new",
		"Error creating condition variable: %s\n", strerror(error));
    abort();
  }

  return retval;
}

void
cnd_delete(cw_cnd_t * a_cnd_o)
{
  int error;

  _cw_check_ptr(a_cnd_o);

  error = pthread_cond_destroy(&a_cnd_o->condition);
  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "cnd_delete",
		"Error destroying condition variable %s\n", strerror(error));
    abort();
  }

  if (a_cnd_o->is_malloced == TRUE)
  {
    _cw_free(a_cnd_o);
  }
}

void
cnd_signal(cw_cnd_t * a_cnd_o)
{
  int error;

  _cw_check_ptr(a_cnd_o);

  error = pthread_cond_signal(&a_cnd_o->condition);
  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "cnd_signal",
		"Error signalling condition variable: %s\n", strerror(error));
    abort();
  }
}

void
cnd_broadcast(cw_cnd_t * a_cnd_o)
{
  int error;

  _cw_check_ptr(a_cnd_o);

  error = pthread_cond_broadcast(&a_cnd_o->condition);
  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "cnd_broadcast",
		"Error broadcasting condition variable: %s\n", strerror(error));
    abort();
  }
}

cw_bool_t
cnd_timedwait(cw_cnd_t * a_cnd_o, cw_mtx_t * a_mtx_o,
	      struct timespec * a_time)
{
  int error;
  cw_bool_t retval;

  _cw_check_ptr(a_cnd_o);
  _cw_check_ptr(a_mtx_o);

  error = pthread_cond_timedwait(&a_cnd_o->condition, &a_mtx_o->mutex, a_time);

  if (error == 0)
  {
    retval = FALSE;
  }
  else if (error == ETIMEDOUT)
  {
    retval = TRUE;
  }
  else /* if (error == EINTR) */
  {
    log_eprintf(g_log_o, NULL, 0, "cnd_timedwait",
		"Error waiting on condition variable: %s\n", strerror(error));
    abort();
  }
  
  return retval;
}

void
cnd_wait(cw_cnd_t * a_cnd_o, cw_mtx_t * a_mtx_o)
{
  int error;

  _cw_check_ptr(a_cnd_o);
  _cw_check_ptr(a_mtx_o);

  error = pthread_cond_wait(&a_cnd_o->condition, &a_mtx_o->mutex);
  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "cnd_wait",
		"Error waiting on condition variable: %s\n", strerror(error));
    abort();
  }
}

cw_sem_t *
sem_new(cw_sem_t * a_sem_o, cw_sint32_t a_count)
{
  cw_sem_t * retval;

  if (a_sem_o == NULL)
  {
    retval = (cw_sem_t *) _cw_malloc(sizeof(cw_sem_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_sem_o;
    retval->is_malloced = FALSE;
  }

  retval->count = a_count;
  retval->waiters = 0;

  mtx_new(&retval->lock);
  cnd_new(&retval->gtzero);
  
  return retval;
}

void
sem_delete(cw_sem_t * a_sem_o)
{
  _cw_check_ptr(a_sem_o);

  mtx_delete(&a_sem_o->lock);
  cnd_delete(&a_sem_o->gtzero);

  if (a_sem_o->is_malloced == TRUE)
  {
    _cw_free(a_sem_o);
  }
}

void
sem_post(cw_sem_t * a_sem_o)
{
  _cw_check_ptr(a_sem_o);

  mtx_lock(&a_sem_o->lock);

  a_sem_o->count++;
  if ((a_sem_o->waiters) && (a_sem_o->count > 0))
  {
    cnd_signal(&a_sem_o->gtzero);
  }

  mtx_unlock(&a_sem_o->lock);
}

void
sem_wait(cw_sem_t * a_sem_o)
{
  _cw_check_ptr(a_sem_o);

  mtx_lock(&a_sem_o->lock);

  while (a_sem_o->count <= 0)
  {
    a_sem_o->waiters++;
    cnd_wait(&a_sem_o->gtzero, &a_sem_o->lock);
    a_sem_o->waiters--;
  }
  a_sem_o->count--;

  mtx_unlock(&a_sem_o->lock);
}

cw_bool_t
sem_trywait(cw_sem_t * a_sem_o)
{
  cw_bool_t retval;

  _cw_check_ptr(a_sem_o);

  mtx_lock(&a_sem_o->lock);
  
  if (a_sem_o->count > 0)
  {
    /* Success. */
    a_sem_o->count--;
    retval = FALSE;
  }
  else
  {
    /* Fail. */
    retval = TRUE;
  }

  mtx_unlock(&a_sem_o->lock);
  
  return retval;
}

cw_sint32_t
sem_getvalue(cw_sem_t * a_sem_o)
{
  cw_sint32_t retval;
  
  _cw_check_ptr(a_sem_o);

  /* I don't think we need to lock, since we're just reading. */
  /*   mtx_lock(&a_sem_o->lock); */

  retval = a_sem_o->count;

  /*   mtx_unlock(&a_sem_o->lock); */
  return retval;
}

void
sem_adjust(cw_sem_t * a_sem_o, cw_sint32_t a_adjust)
{
  _cw_check_ptr(a_sem_o);

  mtx_lock(&a_sem_o->lock);

  a_sem_o->count += a_adjust;
  if ((a_sem_o->waiters) && (a_sem_o->count > 0))
  {
    cw_sint32_t i;
    
    for (i = 0; (i < a_sem_o->count) && (i < a_sem_o->waiters); i++)
    {
      cnd_signal(&a_sem_o->gtzero);
    }
  }
  
  mtx_unlock(&a_sem_o->lock);
}

cw_tsd_t *
tsd_new(cw_tsd_t * a_tsd_o, void (*a_func)(void *))
{
  cw_tsd_t * retval;

  if (a_tsd_o == NULL)
  {
    retval = (cw_tsd_t *) _cw_malloc(sizeof(cw_tsd_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_tsd_o;
    retval->is_malloced = FALSE;
  }

  pthread_key_create(&retval->key, a_func);
  
  return retval;
}

void
tsd_delete(cw_tsd_t * a_tsd_o)
{
  int error;
  
  _cw_check_ptr(a_tsd_o);

  error = pthread_key_delete(a_tsd_o->key);
  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "tsd_delete",
		"Error deleting key: %s\n", strerror(error));
    abort();
  }

  if (a_tsd_o->is_malloced == TRUE)
  {
    _cw_free(a_tsd_o);
  }
}

void *
tsd_get(cw_tsd_t * a_tsd_o)
{
  void * retval;
  
  _cw_check_ptr(a_tsd_o);

  retval = pthread_getspecific(a_tsd_o->key);
  
  return retval;
}

void
tsd_set(cw_tsd_t * a_tsd_o, void * a_val)
{
  int error;
  
  _cw_check_ptr(a_tsd_o);

  error = pthread_setspecific(a_tsd_o->key, a_val);
  if (error)
  {
    log_eprintf(g_log_o, NULL, 0, "tsd_set",
		"Error setting thread-specific data: %s\n", strerror(error));
    abort();
  }
}
