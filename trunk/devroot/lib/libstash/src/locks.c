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
 * $Revision: 145 $
 * $Date: 1998-07-15 17:26:27 -0700 (Wed, 15 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Implementation of some complex locking classes.
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
 * jtl : B-tree lock.  These are used by the block repository to provide
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

#include <string.h>

#include <libstash.h>
#include <locks_priv.h>

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
  list_new(&retval->list, FALSE);

  return retval;
}

void
lwq_delete(cw_lwq_t * a_lwq_o)
{
  _cw_check_ptr(a_lwq_o);

  mtx_delete(&a_lwq_o->lock);

  /* Clean up wait list. */
  _cw_assert(list_count(&a_lwq_o->list) == 0);
  list_delete(&a_lwq_o->list);

  if (a_lwq_o->is_malloced == TRUE)
  {
    _cw_free(a_lwq_o);
  }
}

void
lwq_lock(cw_lwq_t * a_lwq_o)
{
  cw_cnd_t condition;
  
  _cw_check_ptr(a_lwq_o);
  
  mtx_lock(&a_lwq_o->lock);

  if ((a_lwq_o->num_lockers > 0) ||(a_lwq_o->num_lockers > 0))
  {
    /* Create a condition variable. */
    cnd_new(&condition);

    list_tpush(&a_lwq_o->list, (void *) &condition);

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
  cw_cnd_t * condition;

  _cw_check_ptr(a_lwq_o);
  
  mtx_lock(&a_lwq_o->lock);

  a_lwq_o->num_lockers--;

  if (list_count(&a_lwq_o->list) > 0)
  {
    condition = (cw_cnd_t *) list_hpop(&a_lwq_o->list);
    
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

  retval = list_count(&a_lwq_o->list);

  mtx_unlock(&a_lwq_o->lock);
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Frees the list items in the list spares list, if any.  Normally we
 * shouldn't care about this too much, but in the case of B-trees, there
 * may be long queues for the root node, then the root node may become a
 * node with much less contention.  In such a case, it's nice to be able to
 * reclaim space that will likely never be needed again.
 *
 ****************************************************************************/
void lwq_purge_spares(cw_lwq_t * a_lwq_o)
{
  _cw_check_ptr(a_lwq_o);
  mtx_lock(&a_lwq_o->lock);

  list_purge_spares(&a_lwq_o->list);
  
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

cw_jtl_t *
jtl_new(cw_jtl_t * a_jtl_o)
{
  cw_jtl_t * retval;

  if (a_jtl_o == NULL)
  {
    retval = (cw_jtl_t *) _cw_malloc(sizeof(cw_jtl_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_jtl_o;
    retval->is_malloced = FALSE;
  }

  /* Initialize various structures and variables. */
  bzero(retval, sizeof(cw_jtl_t)); /* So that we don't have to individually 
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
jtl_delete(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  /* Clean up structures. */
  mtx_delete(&a_jtl_o->lock);
  rwq_delete(&a_jtl_o->stlock);
  sem_delete(&a_jtl_o->dlock_sem);
  rwl_delete(&a_jtl_o->rxlock);
  sem_delete(&a_jtl_o->wlock_sem);
  
  if (a_jtl_o->is_malloced == TRUE)
  {
    _cw_free(a_jtl_o);
  }
}

void
jtl_slock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  rwq_rlock(&a_jtl_o->stlock);
  a_jtl_o->num_stlocks++;
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_tlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  rwq_wlock(&a_jtl_o->stlock);
  a_jtl_o->num_stlocks++;
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_s2dlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);
  
  mtx_lock(&a_jtl_o->lock);
  sem_wait(&a_jtl_o->dlock_sem);
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_2rlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  rwl_rlock(&a_jtl_o->rxlock);
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_2wlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  /* Grab an a read lock on rxlock to assure that there are no xlockers. */
  rwl_rlock(&a_jtl_o->rxlock);
  sem_wait(&a_jtl_o->wlock_sem);
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_2xlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  rwl_wlock(&a_jtl_o->rxlock);
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_sunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  rwq_runlock(&a_jtl_o->stlock);
  a_jtl_o->num_stlocks--;
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_tunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  rwq_wunlock(&a_jtl_o->stlock);
  a_jtl_o->num_stlocks--;
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_dunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  sem_post(&a_jtl_o->dlock_sem);
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_runlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  rwl_runlock(&a_jtl_o->rxlock);
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_wunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  sem_post(&a_jtl_o->wlock_sem);
  /* Release the lock we grabbed earlier. */
  rwl_runlock(&a_jtl_o->rxlock);
  
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_xunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  rwl_wunlock(&a_jtl_o->rxlock);
  mtx_unlock(&a_jtl_o->lock);
}

cw_uint32_t
jtl_get_max_dlocks(cw_jtl_t * a_jtl_o)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_jtl_o);

  /* No need to lock, since we're just reading. */
/*   mtx_lock(&a_jtl_o->lock); */
  retval = a_jtl_o->max_dlocks;
/*   mtx_unlock(&a_jtl_o->lock); */

  return retval;
}

cw_uint32_t
jtl_set_max_dlocks(cw_jtl_t * a_jtl_o, cw_uint32_t a_dlocks)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  retval = a_jtl_o->max_dlocks;
  a_jtl_o->max_dlocks = a_dlocks;
  sem_adjust(&a_jtl_o->dlock_sem, retval - a_jtl_o->max_dlocks);
  mtx_unlock(&a_jtl_o->lock);

  return retval;
}

cw_uint32_t
jtl_get_num_stlocks(cw_jtl_t * a_jtl_o)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);
  retval = a_jtl_o->num_stlocks;
  mtx_unlock(&a_jtl_o->lock);

  return retval;
}
