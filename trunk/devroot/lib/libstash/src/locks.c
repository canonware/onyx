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
 * Implementation of some complex locking classes.
 *
 * rwl : Read/write lock.  Multiple simultaneous readers are allowed, but
 * only one locker (with no readers) is allowed.  This implementation
 * toggles back and forth between read locks and write locks to assure
 * deterministic locking.
 *
 * jtl : JOE-tree lock.  These are used by the block repository to provide
 * the necessary locking semantics for concurrent JOE-trees.  The following
 * lock types are encapsulated by jtl:
 *   s : Non-serialized place holder lock.
 *   t : Serialized place holder lock.
 *   d : Potential deletion lock (only needed when holding an s lock).
 *   q : Non-exclusive read lock.
 *   r : Non-exclusive read lock.
 *   w : Write lock that allows simultaneous q locks.
 *   x : Exclusive write lock.
 *
 * jtl lock compatibility matrix:
 *
 * (X == compatible)
 * (q == queued, incompatible)
 *
 * | s | t | d | q | r | w | x |
 * +---+---+---+---+---+---+---+--
 * | X |   | X | X | X | X | X | s
 * +---+---+---+---+---+---+---+--
 *     | q |   | X | X | X | X | t
 *     +---+---+---+---+---+---+--
 *         | X | X | X | X | X | d
 *         +---+---+---+---+---+--
 *             | X | X |   |   | q
 *             +---+---+---+---+--
 *                 | X | X |   | r
 *                 +---+---+---+--
 *                     |   |   | w
 *                     +---+---+--
 *                         |   | x
 *                         +---+--
 *
 ****************************************************************************/

#include "libstash/libstash_r.h"
#include "libstash/locks_priv.h"

/****************************************************************************
 *
 * rwl constructor.
 *
 ****************************************************************************/
cw_rwl_t *
rwl_new(cw_rwl_t * a_rwl)
{
  cw_rwl_t * retval;

  if (a_rwl == NULL)
  {
    retval = (cw_rwl_t *) _cw_malloc(sizeof(cw_rwl_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_rwl;
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

/****************************************************************************
 *
 * rwl destructor.
 *
 ****************************************************************************/
void
rwl_delete(cw_rwl_t * a_rwl)
{
  _cw_check_ptr(a_rwl);

  mtx_delete(&a_rwl->lock);
  cnd_delete(&a_rwl->read_wait);
  cnd_delete(&a_rwl->write_wait);

  if (a_rwl->is_malloced)
  {
    _cw_free(a_rwl);
  }
}

/****************************************************************************
 *
 * Get an r-lock.
 *
 ****************************************************************************/
void
rwl_rlock(cw_rwl_t * a_rwl)
{
  _cw_check_ptr(a_rwl);

  mtx_lock(&a_rwl->lock);

  while (a_rwl->num_writers > 0)
  {
    a_rwl->read_waiters++;
    cnd_wait(&a_rwl->read_wait, &a_rwl->lock);
    a_rwl->read_waiters--;
  }
  a_rwl->num_readers++;
  
  mtx_unlock(&a_rwl->lock);
}

/****************************************************************************
 *
 * Release r-lock.
 *
 ****************************************************************************/
void
rwl_runlock(cw_rwl_t * a_rwl)
{
  _cw_check_ptr(a_rwl);

  mtx_lock(&a_rwl->lock);

  a_rwl->num_readers--;

  if ((a_rwl->num_readers == 0) && (a_rwl->write_waiters > 0))
  {
    cnd_signal(&a_rwl->write_wait);
  }
  
  mtx_unlock(&a_rwl->lock);
}

/****************************************************************************
 *
 * Get a w-lock.
 *
 ****************************************************************************/
void
rwl_wlock(cw_rwl_t * a_rwl)
{
  _cw_check_ptr(a_rwl);

  mtx_lock(&a_rwl->lock);

  while ((a_rwl->num_readers > 0) || (a_rwl->num_writers > 0))
  {
    a_rwl->write_waiters++;
    cnd_wait(&a_rwl->write_wait, &a_rwl->lock);
    a_rwl->write_waiters--;
  }
  a_rwl->num_writers++;
  
  mtx_unlock(&a_rwl->lock);
}

/****************************************************************************
 *
 * Release w-lock.
 *
 ****************************************************************************/
void
rwl_wunlock(cw_rwl_t * a_rwl)
{
  _cw_check_ptr(a_rwl);

  mtx_lock(&a_rwl->lock);

  a_rwl->num_writers--;

  /* Doing this in reverse order could potentially be more efficient, but
   * by using this order, we get rid of any non-determinism, i.e. we don't
   * have to worry about a read lock waiter never getting the lock. */
  if (a_rwl->read_waiters > 0)
  {
    cnd_broadcast(&a_rwl->read_wait);
  }
  else if (a_rwl->write_waiters > 0)
  {
    cnd_signal(&a_rwl->write_wait);
  }
  
  mtx_unlock(&a_rwl->lock);
}

/****************************************************************************
 *
 * jtl constructor.
 *
 ****************************************************************************/
cw_jtl_t *
jtl_new(cw_jtl_t * a_jtl)
{
  cw_jtl_t * retval;

  if (a_jtl == NULL)
  {
    retval = (cw_jtl_t *) _cw_malloc(sizeof(cw_jtl_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_jtl;
    retval->is_malloced = FALSE;
  }

  /* Initialize various structures and variables. */
  bzero(retval, sizeof(cw_jtl_t)); /* So that we don't have to individually 
				    * set lots of variables to 0. */
  mtx_new(&retval->lock);
  cnd_new(&retval->slock_wait);
  list_new(&retval->tlock_wait, FALSE);
  cnd_new(&retval->dlock_wait);
  cnd_new(&retval->qlock_wait);
  cnd_new(&retval->rlock_wait);
  cnd_new(&retval->wlock_wait);
  cnd_new(&retval->xlock_wait);

  return retval;
}

/****************************************************************************
 *
 * jtl destructor.
 *
 ****************************************************************************/
void
jtl_delete(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  /* Clean up structures. */
  mtx_delete(&a_jtl->lock);
  cnd_delete(&a_jtl->slock_wait);
  list_delete(&a_jtl->tlock_wait);
  cnd_delete(&a_jtl->dlock_wait);
  cnd_delete(&a_jtl->qlock_wait);
  cnd_delete(&a_jtl->rlock_wait);
  cnd_delete(&a_jtl->wlock_wait);
  cnd_delete(&a_jtl->xlock_wait);
  
  if (a_jtl->is_malloced == TRUE)
  {
    _cw_free(a_jtl);
  }
}

/****************************************************************************
 *
 * Get an s-lock.
 *
 ****************************************************************************/
void
jtl_slock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);
  while ((a_jtl->tlock_holders > 0)
	 || (list_count(&a_jtl->tlock_wait) > 0)
	 )
  {
    a_jtl->slock_waiters++;
    cnd_wait(&a_jtl->slock_wait, &a_jtl->lock);
    a_jtl->slock_waiters--;
  }
  a_jtl->slock_holders++;
  
  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Reserve a place in line for a tlock.
 *
 ****************************************************************************/
cw_jtl_tq_el_t *
jtl_get_tq_el(cw_jtl_t * a_jtl)
{
  cw_jtl_tq_el_t * retval;
  
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  retval = (cw_jtl_tq_el_t *) _cw_malloc(sizeof(cw_jtl_tq_el_t));
  retval->is_blocked = FALSE;
  cnd_new(&retval->tlock_wait);

  list_tpush(&a_jtl->tlock_wait, retval);

  mtx_unlock(&a_jtl->lock);
  return retval;
}

/****************************************************************************
 *
 * Get a t-lock, using the place holder returned by jtl_get_tq_el().
 *
 ****************************************************************************/
void
jtl_tlock(cw_jtl_t * a_jtl, cw_jtl_tq_el_t * a_tq_el)
{
  _cw_check_ptr(a_jtl);
  _cw_check_ptr(a_tq_el);

  mtx_lock(&a_jtl->lock);

  if ((a_jtl->tlock_holders == 0)
      && (list_count(&a_jtl->tlock_wait) == 0)
      )
  {
    /* No other threads are waiting for a tlock.  Help ourselves. */
  }
  else if ((a_jtl->tlock_holders == 0)
	   && (a_tq_el == list_hpeek(&a_jtl->tlock_wait)))
  {
    /* This thread is first in line. */
    list_hpop(&a_jtl->tlock_wait);
  }
  else
  {
    a_tq_el->is_blocked = TRUE;
    a_jtl->tlock_waiters++;
    cnd_wait(&a_tq_el->tlock_wait, &a_jtl->lock);
    a_jtl->tlock_waiters--;
    list_hpop(&a_jtl->tlock_wait);
  }
  a_jtl->tlock_holders++;
  cnd_delete(&a_tq_el->tlock_wait);
  _cw_free(a_tq_el);

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Convert an s-lock to an sd-lock..
 *
 ****************************************************************************/
void
jtl_s2dlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);
  
  mtx_lock(&a_jtl->lock);

  while (a_jtl->dlock_holders >= a_jtl->dlock_holders)
  {
    a_jtl->dlock_waiters++;
    cnd_wait(&a_jtl->dlock_wait, &a_jtl->lock);
    a_jtl->dlock_waiters--;
  }
  a_jtl->dlock_holders++;

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Get a q-lock.
 *
 ****************************************************************************/
void
jtl_2qlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  while ((a_jtl->wlock_holders > 0)
	 || (a_jtl->xlock_holders > 0))
  {
    a_jtl->qlock_waiters++;
    cnd_wait(&a_jtl->qlock_wait, &a_jtl->lock);
    a_jtl->qlock_waiters--;
  }
  a_jtl->qlock_holders++;
  
  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Get an r-lock.
 *
 ****************************************************************************/
void
jtl_2rlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  while (a_jtl->xlock_holders > 0)
  {
    a_jtl->rlock_waiters++;
    cnd_wait(&a_jtl->rlock_wait, &a_jtl->lock);
    a_jtl->rlock_waiters--;
  }
  a_jtl->rlock_holders++;

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Get a w-lock.
 *
 ****************************************************************************/
void
jtl_2wlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  while ((a_jtl->qlock_holders > 0)
	 || (a_jtl->wlock_holders > 0)
	 || (a_jtl->xlock_holders > 0))
  {
    a_jtl->wlock_waiters++;
    cnd_wait(&a_jtl->wlock_wait, &a_jtl->lock);
    a_jtl->wlock_waiters--;
  }
  a_jtl->wlock_holders++;

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Get an x-lock.
 *
 ****************************************************************************/
void
jtl_2xlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  while ((a_jtl->qlock_holders > 0)
	 || (a_jtl->rlock_holders > 0)
	 || (a_jtl->wlock_holders > 0)
	 || (a_jtl->xlock_holders > 0))
  {
    a_jtl->xlock_waiters++;
    cnd_wait(&a_jtl->xlock_wait, &a_jtl->lock);
    a_jtl->xlock_waiters--;
  }
  a_jtl->xlock_holders++;

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Release s-lock.
 *
 ****************************************************************************/
void
jtl_sunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->slock_holders--;

  if ((a_jtl->slock_holders == 0)
      && (list_count(&a_jtl->tlock_wait) > 0)
      && (((cw_jtl_tq_el_t *)
	   list_hpeek(&a_jtl->tlock_wait))->is_blocked == TRUE))
  {
    cnd_signal(&((cw_jtl_tq_el_t *)
		 list_hpeek(&a_jtl->tlock_wait))->tlock_wait);
  }
  else if (a_jtl->slock_waiters > 0)
  {
    cnd_broadcast(&a_jtl->slock_wait);
  }
  
  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Release t-lock.
 *
 ****************************************************************************/
void
jtl_tunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->tlock_holders--;

  if (a_jtl->tlock_waiters > 0)
  {
    cw_jtl_tq_el_t * tq_el;

    _cw_assert(list_count(&a_jtl->tlock_wait) > 0);
    tq_el = (cw_jtl_tq_el_t *) list_hpeek(&a_jtl->tlock_wait);
    _cw_check_ptr(tq_el);

    if (tq_el->is_blocked == TRUE)
    {
      cnd_signal(&tq_el->tlock_wait);
    }
  }
  else if ((a_jtl->slock_waiters > 0)
	   && (list_count(&a_jtl->tlock_wait) == 0))
  {
    cnd_broadcast(&a_jtl->slock_wait);
  }

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Release d-lock.
 *
 ****************************************************************************/
void
jtl_dunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->dlock_holders--;

  if ((a_jtl->dlock_waiters > 0)
      && (a_jtl->dlock_holders < a_jtl->max_dlocks))
  {
    cnd_signal(&a_jtl->dlock_wait);
  }

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Release q-lock.
 *
 ****************************************************************************/
void
jtl_qunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->qlock_holders--;
  jtl_p_qrwx_unlock(a_jtl);
  
  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Release r-lock.
 *
 ****************************************************************************/
void
jtl_runlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->rlock_holders--;
  jtl_p_qrwx_unlock(a_jtl);

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Release w-lock.
 *
 ****************************************************************************/
void
jtl_wunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->wlock_holders--;
  jtl_p_qrwx_unlock(a_jtl);
  
  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Release x-lock.
 *
 ****************************************************************************/
void
jtl_xunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->xlock_holders--;
  jtl_p_qrwx_unlock(a_jtl);

  mtx_unlock(&a_jtl->lock);
}

/****************************************************************************
 *
 * Return the maximum number of d-locks this a_jtl will grant.
 *
 ****************************************************************************/
cw_uint32_t
jtl_get_max_dlocks(cw_jtl_t * a_jtl)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);
  retval = a_jtl->max_dlocks;
  mtx_unlock(&a_jtl->lock);

  return retval;
}

/****************************************************************************
 *
 * Set the maximum number of d-locks a_jtl will grant to a_dlocks and return
 * the old value.
 *
 ****************************************************************************/
cw_uint32_t
jtl_set_max_dlocks(cw_jtl_t * a_jtl, cw_uint32_t a_dlocks)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);
  retval = a_jtl->max_dlocks;
  a_jtl->max_dlocks = a_dlocks;
  mtx_unlock(&a_jtl->lock);

  return retval;
}

/****************************************************************************
 *
 * Do the work of lock granting when a lock is released.
 *
 ****************************************************************************/
void
jtl_p_qrwx_unlock(cw_jtl_t * a_jtl)
{
  /* Grant locks in this order: x, w, r & q. */
  if ((a_jtl->qlock_holders == 0) && (a_jtl->rlock_holders == 0))
  {
    if (a_jtl->wlock_holders == 0)
    {
      if (a_jtl->xlock_holders == 0)
      {
	if (a_jtl->xlock_waiters > 0)
	{
	  /* Grant xlock. */
	  cnd_signal(&a_jtl->xlock_wait);
	}
	else if (a_jtl->wlock_waiters > 0)
	{
	  /* Grant wlock. */
	  cnd_signal(&a_jtl->wlock_wait);
	}
	else
	{
	  if (a_jtl->qlock_waiters > 0)
	  {
	    /* Grant qlocks. */
	    cnd_broadcast(&a_jtl->qlock_wait);
	  }
	  if (a_jtl->rlock_waiters > 0)
	  {
	    /* Grant rlocks. */
	    cnd_broadcast(&a_jtl->rlock_wait);
	  }
	}
      }
    }
    else if (a_jtl->rlock_waiters > 0)
    {
      /* Grant rlocks. */
      cnd_broadcast(&a_jtl->rlock_wait);
    }
  }
  else if ((a_jtl->wlock_waiters == 0) && (a_jtl->xlock_waiters == 0))
  {
    if (a_jtl->qlock_waiters > 0)
    {
      /* Grant qlocks. */
      cnd_broadcast(&a_jtl->qlock_wait);
    }
    if (a_jtl->rlock_waiters > 0)
    {
      /* Grant rlocks. */
      cnd_broadcast(&a_jtl->rlock_wait);
    }
  }
}
