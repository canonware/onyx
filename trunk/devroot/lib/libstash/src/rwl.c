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

#include "../include/libstash/libstash.h"

cw_rwl_t *
rwl_new(cw_rwl_t * a_rwl)
{
  cw_rwl_t * retval;

  if (a_rwl == NULL)
  {
    retval = (cw_rwl_t *) _cw_malloc(sizeof(cw_rwl_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
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

  RETURN:
  return retval;
}

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
