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
 * $Revision: 52 $
 * $Date: 1998-04-30 02:39:06 -0700 (Thu, 30 Apr 1998) $
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
 * lwq : Lock wait queue.  Same as mtx, except that lock waiters are
 * queued.  This is useful when serializeability is important, or when
 * wishing to avoid the (implementation-independent) non-deterministic
 * order in which lock waiters are granted a mutex.
 * 
 * rwl : Read/write lock.  Multiple simultaneous readers are allowed, but
 * only one locker (with no readers) is allowed.  This implementation
 * toggles back and forth between read locks and write locks to assure
 * deterministic locking.
 *
 * rwq : Same as rwl, except write lock waiters are queued, to ensure
 * serialization.
 *
 * tsd : Thread-specific data.
 *
 * btl : B-tree lock.  These are used by the block repository to provide
 * the necessary locking semantics for concurrent B-trees.  The following
 * lock types are encapsulated by btl:
 *   s : Non-serialized place holder lock.
 *   t : Serialized place holder lock.
 *   d : Potential deletion lock (only needed when holding an s lock).
 *   r : Non-exclusive read lock.
 *   w : Write lock that allows simultaneous r locks.
 *   x : Exclusive write lock.  No simultaneous r or w locks allowed.
 *
 ****************************************************************************/

#define _INC_THREAD_H_
#define _INC_STRING_H_
#include <config.h>
#include <thread_priv.h>

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
    log_eprintf(g_log_o, __FILE__, __LINE__, "thd_new",
		"Cannot create thread, error %d\n", error);
    abort();
  }

/*   error = pthread_detach(retval->thread); */

/*   if (error) */
/*   { */
/*     log_eprintf(g_log_o, __FILE__, __LINE__, "thd_new", */
/* 		"Cannot detach thread, error %d\n", error); */
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
    log_printf(g_log_o, __FILE__, __LINE__, "mtx_new",
	       "Unable to create mutex, error %d\n", error);
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
    log_printf(g_log_o, __FILE__, __LINE__, "mtx_delete",
	       "Unable to destroy mutex, error %d\n", error);
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
    log_printf(g_log_o, __FILE__, __LINE__, "mtx_lock",
	       "Error locking mutex, error %d\n", error);
    abort();
  }
}

cw_bool_t
mtx_trylock(cw_mtx_t * a_mtx_o)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_mtx_o);

  retval = pthread_mutex_trylock(&a_mtx_o->mutex);

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
    log_printf(g_log_o, __FILE__, __LINE__, "mtx_unlock",
	       "Error unlocking mutex, error %d\n", error);
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
    log_printf(g_log_o, __FILE__, __LINE__, "cnd_new",
	       "Error creating condition variable, error %d\n", error);
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
    log_printf(g_log_o, __FILE__, __LINE__, "cnd_delete",
	       "Error destroying condition variable, error %d\n", error);
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

  pthread_cond_signal(&a_cnd_o->condition);
  if (error)
  {
    log_printf(g_log_o, __FILE__, __LINE__, "cnd_signal",
	       "Error signalling condition variable, error %d\n", error);
    abort();
  }
}

void
cnd_broadcast(cw_cnd_t * a_cnd_o)
{
  int error;

  _cw_check_ptr(a_cnd_o);

  pthread_cond_broadcast(&a_cnd_o->condition);
  if (error)
  {
    log_printf(g_log_o, __FILE__, __LINE__, "cnd_broadcast",
	       "Error broadcasting condition variable, error %d\n", error);
    abort();
  }
}

cw_bool_t
cnd_timedwait(cw_cnd_t * a_cnd_o, cw_mtx_t * a_mtx_o,
	      struct timespec * a_time)
{
  int error;

  _cw_check_ptr(a_cnd_o);
  _cw_check_ptr(a_mtx_o);

  pthread_cond_timedwait(&a_cnd_o->condition, &a_mtx_o->mutex,
			 a_time);
  if (error)
  {
    log_printf(g_log_o, __FILE__, __LINE__, "cnd_timedwait",
	       "Error timed waiting on condition variable, error %d\n", error);
    abort();
  }

  /* XXX Should we indicate whether a timeout occurred? */
  return FALSE;
}

void
cnd_wait(cw_cnd_t * a_cnd_o, cw_mtx_t * a_mtx_o)
{
  int error;

  _cw_check_ptr(a_cnd_o);
  _cw_check_ptr(a_mtx_o);

  pthread_cond_wait(&a_cnd_o->condition, &a_mtx_o->mutex);
  if (error)
  {
    log_printf(g_log_o, __FILE__, __LINE__, "cnd_wait",
	       "Error waiting on condition variable, error %d\n", error);
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
  
  mtx_unlock(&a_sem_o->lock);
}

cw_lwq_t *
lwq_new(cw_lwq_t * a_lwq_o)
{
  cw_lwq_t * retval;

  if (a_lwq_o == NULL)
  {
    retval = (cw_lwq_t *) _cw_malloc(sizeof(cw_lwq_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_lwq_o;
    retval->is_malloced = FALSE;
  }

  mtx_new(&retval->lock);
  retval->list = list_new();
  retval->spares_list = list_new();

  return retval;
}

void
lwq_delete(cw_lwq_t * a_lwq_o)
{
  _cw_check_ptr(a_lwq_o);

  mtx_delete(&a_lwq_o->lock);

  /* Clean up wait list. */
  _cw_assert(list_count(a_lwq_o->list) == 0);
  list_delete(a_lwq_o->list);

  /* Clean up spares list. */
  list_delete(a_lwq_o->spares_list);

  if (a_lwq_o->is_malloced == TRUE)
  {
    _cw_free(a_lwq_o);
  }
}

void
lwq_lock(cw_lwq_t * a_lwq_o)
{
  cw_list_item_t * item;
  cw_cnd_t condition;
  
  _cw_check_ptr(a_lwq_o);
  
  mtx_lock(&a_lwq_o->lock);

  if ((a_lwq_o->num_lockers > 0) ||(a_lwq_o->num_lockers > 0))
  {
    /* Create a condition variable. */
    cnd_new(&condition);

    /* Create a list_item and append it to the list.  Use an item from the
     * spares list if one exists. */
    if (list_count(a_lwq_o->spares_list) > 0)
    {
      item = list_hpop(a_lwq_o->spares_list);
    }
    else
    {
      item = list_item_new();
    }
    
    list_item_set(item, (void *) &condition);
    list_tpush(a_lwq_o->list, item);

    /* Wait on the condition variable. */
    a_lwq_o->num_lock_waiters++;
    cnd_wait(&condition, &a_lwq_o->lock);
    a_lwq_o->num_lock_waiters--;
    
    /* Clean this up while we're still in this stack frame. */
    cnd_delete(&condition);
  }

  /* If we didn't enter the above block, it means no one else is holding
   * the lock, so in either case we've got the lock now. */
  a_lwq_o->num_lockers++;
  
  mtx_unlock(&a_lwq_o->lock);
}

void
lwq_unlock(cw_lwq_t * a_lwq_o)
{
  cw_list_item_t * item;
  cw_cnd_t * condition;

  _cw_check_ptr(a_lwq_o);
  
  mtx_lock(&a_lwq_o->lock);

  a_lwq_o->num_lockers--;

  if (list_count(a_lwq_o->list) > 0)
  {
    item = list_hpop(a_lwq_o->list);

    condition = (cw_cnd_t *) list_item_get(item);
    /* Save the item for repeat use. */
    list_hpush(a_lwq_o->spares_list, item);
    
    cnd_signal(condition);

    /* We do NOT free condition here, since it's on another thread's
     * stack.  That thread cleans it up after unblocking. */
  }
  
  mtx_unlock(&a_lwq_o->lock);
}

cw_sint32_t
lwq_num_waiters(cw_lwq_t * a_lwq_o)
{
  cw_sint32_t retval;
  
  _cw_check_ptr(a_lwq_o);

  mtx_lock(&a_lwq_o->lock);

  retval = list_count(a_lwq_o->list);

  mtx_unlock(&a_lwq_o->lock);
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Frees the list items pointed to by a_lwq_o->spares_list, if any.
 * Normally we shouldn't care about this too much, but in the case of
 * B-trees, there may be long queues for the root node, then the root node
 * may become a node with much less contention.  In such a case, it's nice
 * to be able to reclaim space that will likely never be needed again.
 *
 ****************************************************************************/
void lwq_purge_spares(cw_lwq_t * a_lwq_o)
{
  cw_sint32_t count, i;
  cw_list_item_t * item;

  _cw_check_ptr(a_lwq_o);

  mtx_lock(&a_lwq_o->lock);
  
  count = list_count(a_lwq_o->spares_list);
  for (i = 0; i < count; i++)
  {
    item = list_hpop(a_lwq_o->spares_list);
    list_item_delete(item);
  }
  mtx_unlock(&a_lwq_o->lock);
}

cw_rwl_t *
rwl_new(cw_rwl_t * a_rwl_o)
{
  cw_rwl_t * retval;

  if (a_rwl_o == NULL)
  {
    retval = (cw_rwl_t *) _cw_malloc(sizeof(cw_rwl_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_rwl_o;
    retval->is_malloced = FALSE;
  }

  mtx_new(&retval->lock);
  cnd_new(&retval->read_wait);
  cnd_new(&retval->write_wait);

  retval->num_readers = 0;
  retval->num_writers = 0;
  retval->read_waiters = 0;
  retval->write_waiters = 0;
  
  return retval;
}

void
rwl_delete(cw_rwl_t * a_rwl_o)
{
  _cw_check_ptr(a_rwl_o);

  mtx_delete(&a_rwl_o->lock);
  cnd_delete(&a_rwl_o->read_wait);
  cnd_delete(&a_rwl_o->write_wait);

  if (a_rwl_o->is_malloced)
  {
    _cw_free(a_rwl_o);
  }
}

void
rwl_rlock(cw_rwl_t * a_rwl_o)
{
  _cw_check_ptr(a_rwl_o);

  mtx_lock(&a_rwl_o->lock);

  while (a_rwl_o->num_writers > 0)
  {
    a_rwl_o->read_waiters++;
    cnd_wait(&a_rwl_o->read_wait, &a_rwl_o->lock);
    a_rwl_o->read_waiters--;
  }
  a_rwl_o->num_readers++;
  
  mtx_unlock(&a_rwl_o->lock);
}

void
rwl_runlock(cw_rwl_t * a_rwl_o)
{
  _cw_check_ptr(a_rwl_o);

  mtx_lock(&a_rwl_o->lock);

  a_rwl_o->num_readers--;

  if ((a_rwl_o->num_readers == 0) && (a_rwl_o->write_waiters > 0))
  {
    cnd_signal(&a_rwl_o->write_wait);
  }
  
  mtx_unlock(&a_rwl_o->lock);
}

void
rwl_wlock(cw_rwl_t * a_rwl_o)
{
  _cw_check_ptr(a_rwl_o);

  mtx_lock(&a_rwl_o->lock);

  while (a_rwl_o->num_readers > 0)
  {
    a_rwl_o->write_waiters++;
    cnd_wait(&a_rwl_o->write_wait, &a_rwl_o->lock);
    a_rwl_o->write_waiters--;
  }
  a_rwl_o->num_writers++;
  
  mtx_unlock(&a_rwl_o->lock);
}

void
rwl_wunlock(cw_rwl_t * a_rwl_o)
{
  _cw_check_ptr(a_rwl_o);

  mtx_lock(&a_rwl_o->lock);

  a_rwl_o->num_writers--;

  /* Doing this in reverse order could potentially be more efficient, but
   * by using this order, we get rid of any non-determinism, i.e. we don't
   * have to worry about a read lock waiter never getting the lock. */
  if (a_rwl_o->read_waiters > 0)
  {
    cnd_broadcast(&a_rwl_o->read_wait);
  }
  else if (a_rwl_o->write_waiters > 0)
  {
    cnd_signal(&a_rwl_o->write_wait);
  }
  
  mtx_unlock(&a_rwl_o->lock);
}


cw_rwq_t *
rwq_new(cw_rwq_t * a_rwq_o)
{
  cw_rwq_t * retval;

  if (a_rwq_o == NULL)
  {
    retval = (cw_rwq_t *) _cw_malloc(sizeof(cw_rwq_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_rwq_o;
    retval->is_malloced = FALSE;
  }

  bzero(retval, sizeof(cw_rwq_t));
  
  mtx_new(&retval->lock);
  cnd_new(&retval->read_wait);
  lwq_new(&retval->write_waiters);
  
  return retval;
}

void
rwq_delete(cw_rwq_t * a_rwq_o)
{
  _cw_check_ptr(a_rwq_o);

  mtx_delete(&a_rwq_o->lock);
  cnd_delete(&a_rwq_o->read_wait);
  lwq_delete(&a_rwq_o->write_waiters);

  if (a_rwq_o->is_malloced)
  {
    _cw_free(a_rwq_o);
  }
}

void
rwq_rlock(cw_rwq_t * a_rwq_o)
{
  _cw_check_ptr(a_rwq_o);

  mtx_lock(&a_rwq_o->lock);

  while (lwq_num_waiters(&a_rwq_o->write_waiters) > 0)
  {
    a_rwq_o->read_waiters++;
    cnd_wait(&a_rwq_o->read_wait, &a_rwq_o->lock);
    a_rwq_o->read_waiters--;
  }
  if (a_rwq_o->num_readers == 0)
  {
    /* Grab a lwq lock to prevent potential lockers from getting in. */
    lwq_lock(&a_rwq_o->write_waiters);
  }
  a_rwq_o->num_readers++;
  
  mtx_unlock(&a_rwq_o->lock);
}

void
rwq_runlock(cw_rwq_t * a_rwq_o)
{
  _cw_check_ptr(a_rwq_o);

  mtx_lock(&a_rwq_o->lock);

  a_rwq_o->num_readers--;

  if ((a_rwq_o->num_readers == 0)
      && (lwq_num_waiters(&a_rwq_o->write_waiters) > 0))
  {
    /* Release our lock on the lwq so that lockers can get in. */
    lwq_unlock(&a_rwq_o->write_waiters);
  }
  
  mtx_unlock(&a_rwq_o->lock);
}

void
rwq_wlock(cw_rwq_t * a_rwq_o)
{
  _cw_check_ptr(a_rwq_o);

  mtx_lock(&a_rwq_o->lock);

  lwq_lock(&a_rwq_o->write_waiters);
  
  mtx_unlock(&a_rwq_o->lock);
}

void
rwq_wunlock(cw_rwq_t * a_rwq_o)
{
  _cw_check_ptr(a_rwq_o);

  mtx_lock(&a_rwq_o->lock);

  lwq_unlock(&a_rwq_o->write_waiters);

  if ((lwq_num_waiters(&a_rwq_o->write_waiters) == 0)
      && (a_rwq_o->read_waiters > 0))
  {
    cnd_signal(&a_rwq_o->read_wait);
  }
  
  mtx_unlock(&a_rwq_o->lock);
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
  _cw_check_ptr(a_tsd_o);

  pthread_key_delete(a_tsd_o->key);

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
    log_printf(g_log_o, __FILE__, __LINE__, "tsd_set",
	       "Error setting thread-specific data, error %d\n", error);
    abort();
  }
}

cw_btl_t *
btl_new(cw_btl_t * a_btl_o)
{
  cw_btl_t * retval;

  if (a_btl_o == NULL)
  {
    retval = (cw_btl_t *) _cw_malloc(sizeof(cw_btl_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_btl_o;
    retval->is_malloced = FALSE;
  }

  /* Initialize various structures and variables. */
  bzero(retval, sizeof(cw_btl_t)); /* So that we don't have to individually 
				    * set lots of variables to 0. */
  mtx_new(&retval->lock);
  rwq_new(&retval->stlock);
  sem_new(&retval->dlock_sem, 0); /* XXX Do we want to set this
				   * intelligently now or later? */
  rwl_new(&retval->rxlock);
  sem_new(&retval->wlock_sem, 1);

  return retval;
}

void
btl_delete(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  /* Clean up structures. */
  mtx_delete(&a_btl_o->lock);
  rwq_delete(&a_btl_o->stlock);
  sem_delete(&a_btl_o->dlock_sem);
  rwl_delete(&a_btl_o->rxlock);
  sem_delete(&a_btl_o->wlock_sem);
  
  if (a_btl_o->is_malloced == TRUE)
  {
    _cw_free(a_btl_o);
  }
}

void
btl_slock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwq_rlock(&a_btl_o->stlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_tlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwq_wlock(&a_btl_o->stlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_s2dlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);
  
  mtx_lock(&a_btl_o->lock);
  sem_wait(&a_btl_o->dlock_sem);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_s2rlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwl_rlock(&a_btl_o->rxlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_s2wlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  /* Grab an a read lock on rxlock to assure that there are no xlockers. */
  rwl_rlock(&a_btl_o->rxlock);
  sem_wait(&a_btl_o->wlock_sem);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_s2xlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwl_wlock(&a_btl_o->rxlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_t2rlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwl_rlock(&a_btl_o->rxlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_t2wlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  /* Grab an a read lock on rxlock to assure that there are no xlockers. */
  rwl_rlock(&a_btl_o->rxlock);
  sem_wait(&a_btl_o->wlock_sem);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_t2xlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwl_wlock(&a_btl_o->rxlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_sunlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwq_runlock(&a_btl_o->stlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_tunlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwq_wunlock(&a_btl_o->stlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_dunlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  sem_post(&a_btl_o->dlock_sem);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_runlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwl_runlock(&a_btl_o->rxlock);
  mtx_unlock(&a_btl_o->lock);
}

void
btl_wunlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);

  sem_post(&a_btl_o->wlock_sem);
  /* Release the lock we grabbed earlier. */
  rwl_runlock(&a_btl_o->rxlock);
  
  mtx_unlock(&a_btl_o->lock);
}

void
btl_xunlock(cw_btl_t * a_btl_o)
{
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  rwl_wunlock(&a_btl_o->rxlock);
  mtx_unlock(&a_btl_o->lock);
}

cw_uint32_t
btl_get_dlocks(cw_btl_t * a_btl_o)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_btl_o);

  /* No need to lock, since we're just reading. */
/*   mtx_lock(&a_btl_o->lock); */
  retval = a_btl_o->max_dlocks;
/*   mtx_unlock(&a_btl_o->lock); */

  return retval;
}

cw_uint32_t
btl_set_dlocks(cw_btl_t * a_btl_o, cw_uint32_t a_dlocks)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_btl_o);

  mtx_lock(&a_btl_o->lock);
  retval = a_btl_o->max_dlocks;
  a_btl_o->max_dlocks = a_dlocks;
  sem_adjust(&a_btl_o->dlock_sem, retval - a_btl_o->max_dlocks);
  mtx_unlock(&a_btl_o->lock);

  return retval;
}
