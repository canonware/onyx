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
 * $Revision: 218 $
 * $Date: 1998-09-11 00:24:05 -0700 (Fri, 11 Sep 1998) $
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

#include <string.h>

#include <libstash.h>
#include <locks_priv.h>

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

  while ((a_rwl_o->num_readers > 0) || (a_rwl_o->num_writers > 0))
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
  cnd_new(&retval->slock_wait);
  list_new(&retval->tlock_wait, FALSE);
  cnd_new(&retval->dlock_wait);
  cnd_new(&retval->qlock_wait);
  cnd_new(&retval->rlock_wait);
  cnd_new(&retval->wlock_wait);
  cnd_new(&retval->xlock_wait);

  return retval;
}

void
jtl_delete(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  /* Clean up structures. */
  mtx_delete(&a_jtl_o->lock);
  cnd_delete(&a_jtl_o->slock_wait);
  list_delete(&a_jtl_o->tlock_wait);
  cnd_delete(&a_jtl_o->dlock_wait);
  cnd_delete(&a_jtl_o->qlock_wait);
  cnd_delete(&a_jtl_o->rlock_wait);
  cnd_delete(&a_jtl_o->wlock_wait);
  cnd_delete(&a_jtl_o->xlock_wait);
  
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
  while ((a_jtl_o->tlock_holders > 0)
	 || (a_jtl_o->tlock_waiters > 0))
  {
    a_jtl_o->slock_waiters++;
    cnd_wait(&a_jtl_o->slock_wait, &a_jtl_o->lock);
    a_jtl_o->slock_waiters--;
  }
  a_jtl_o->slock_holders++;
  
  mtx_unlock(&a_jtl_o->lock);
}

cw_jtl_tq_el_t *
jtl_get_tq_el(cw_jtl_t * a_jtl_o)
{
  cw_jtl_tq_el_t * retval;
  
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  retval = (cw_jtl_tq_el_t *) _cw_malloc(sizeof(cw_jtl_tq_el_t));
  retval->is_blocked = FALSE;
  cnd_new(&retval->tlock_wait);

  list_tpush(&a_jtl_o->tlock_wait, retval);

  mtx_unlock(&a_jtl_o->lock);
  return retval;
}

void
jtl_tlock(cw_jtl_t * a_jtl_o, cw_jtl_tq_el_t * a_tq_el)
{
  cw_jtl_tq_el_t * tq_el;
  
  _cw_check_ptr(a_jtl_o);
  _cw_check_ptr(a_tq_el);

  mtx_lock(&a_jtl_o->lock);

  if ((a_jtl_o->tlock_holders == 0)
      && (a_jtl_o->tlock_waiters == 0))
  {
    /* No other threads are waiting for a tlock.  Help ourselves. */
    cnd_delete(&a_tq_el->tlock_wait);
    _cw_free(a_tq_el);
    a_jtl_o->tlock_holders++;
  }
  else
  {
    a_tq_el->is_blocked = TRUE;
    while(a_jtl_o->tlock_holders > 0)
    {
      a_jtl_o->tlock_waiters++;
      cnd_wait(&a_tq_el->tlock_wait, &a_jtl_o->lock);
      a_jtl_o->tlock_waiters--;
    }
    a_jtl_o->tlock_holders++;
    tq_el = (cw_jtl_tq_el_t *) list_hpop(&a_jtl_o->tlock_wait);
    cnd_delete(&tq_el->tlock_wait);
    _cw_free(tq_el);
  }

  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_s2dlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);
  
  mtx_lock(&a_jtl_o->lock);

  while (a_jtl_o->dlock_holders >= a_jtl_o->dlock_holders)
  {
    a_jtl_o->dlock_waiters++;
    cnd_wait(&a_jtl_o->dlock_wait, &a_jtl_o->lock);
    a_jtl_o->dlock_waiters--;
  }
  a_jtl_o->dlock_holders++;

  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_2qlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  while ((a_jtl_o->wlock_holders > 0)
	 || (a_jtl_o->xlock_holders > 0))
  {
    a_jtl_o->qlock_waiters++;
    cnd_wait(&a_jtl_o->qlock_wait, &a_jtl_o->lock);
    a_jtl_o->qlock_waiters--;
  }
  a_jtl_o->qlock_holders++;
  
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_2rlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  while (a_jtl_o->xlock_holders > 0)
  {
    a_jtl_o->rlock_waiters++;
    cnd_wait(&a_jtl_o->rlock_wait, &a_jtl_o->lock);
    a_jtl_o->rlock_waiters--;
  }
  a_jtl_o->rlock_holders++;

  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_2wlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  while ((a_jtl_o->qlock_holders > 0)
	 || (a_jtl_o->wlock_holders > 0)
	 || (a_jtl_o->xlock_holders > 0))
  {
    a_jtl_o->wlock_waiters++;
    cnd_wait(&a_jtl_o->wlock_wait, &a_jtl_o->lock);
    a_jtl_o->wlock_waiters--;
  }
  a_jtl_o->wlock_holders++;

  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_2xlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  while ((a_jtl_o->qlock_holders > 0)
	 || (a_jtl_o->rlock_holders > 0)
	 || (a_jtl_o->wlock_holders > 0)
	 || (a_jtl_o->xlock_holders > 0))
  {
    a_jtl_o->xlock_waiters++;
    cnd_wait(&a_jtl_o->xlock_wait, &a_jtl_o->lock);
    a_jtl_o->xlock_waiters--;
  }
  a_jtl_o->xlock_holders++;

  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_sunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  a_jtl_o->slock_holders--;

  if ((a_jtl_o->slock_holders == 0)
      && (a_jtl_o->tlock_waiters > 0))
  {
    cw_jtl_tq_el_t * tq_el;

    _cw_assert(list_count(&a_jtl_o->tlock_wait) > 0);
    tq_el = (cw_jtl_tq_el_t *) list_hpop(&a_jtl_o->tlock_wait);
    _cw_check_ptr(tq_el);

    if (tq_el->is_blocked == TRUE)
    {
      cnd_signal(&tq_el->tlock_wait);
    }
    /* Unconditionally push tq_el back on the list, because we can't delete
     * it, no matter what, since the condition variable must exist until
     * after the blocked thread wakes up.  That thread is responsible for
     * cleaning up. */
    /* XXX Perhaps a more efficient way of getting at the head/tail is in
     * order. */
    list_hpush(&a_jtl_o->tlock_wait, tq_el);
  }
  
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_tunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  a_jtl_o->tlock_holders--;

  if (a_jtl_o->tlock_waiters > 0)
  {
    cw_jtl_tq_el_t * tq_el;

    _cw_assert(list_count(&a_jtl_o->tlock_wait) > 0);
    tq_el = (cw_jtl_tq_el_t *) list_hpop(&a_jtl_o->tlock_wait);
    _cw_check_ptr(tq_el);

    if (tq_el->is_blocked == TRUE)
    {
      cnd_signal(&tq_el->tlock_wait);
    }
    
    /* Unconditionally push tq_el back on the list. */
    list_hpush(&a_jtl_o->tlock_wait, tq_el);
  }

  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_dunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  a_jtl_o->dlock_holders--;

  if ((a_jtl_o->dlock_waiters > 0)
      && (a_jtl_o->dlock_holders < a_jtl_o->max_dlocks))
  {
    cnd_signal(&a_jtl_o->dlock_wait);
  }

  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_qunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  a_jtl_o->qlock_holders--;

  /* XXX Implement. */
  
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_runlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  /* XXX Implement. */

  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_wunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  /* XXX Implement. */
  
  mtx_unlock(&a_jtl_o->lock);
}

void
jtl_xunlock(cw_jtl_t * a_jtl_o)
{
  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  /* XXX Implement. */

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
  mtx_unlock(&a_jtl_o->lock);

  return retval;
}

cw_uint32_t
jtl_get_num_stlocks(cw_jtl_t * a_jtl_o)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_jtl_o);

  mtx_lock(&a_jtl_o->lock);

  /* XXX Implement. */
  retval = 0; /* XXX Gets rid of warning. */

  mtx_unlock(&a_jtl_o->lock);

  return retval;
}
