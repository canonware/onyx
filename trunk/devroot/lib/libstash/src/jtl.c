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
#include "../include/libstash/jtl_p.h"

cw_jtl_t *
jtl_new(cw_jtl_t * a_jtl)
{
  cw_jtl_t * retval;

  if (a_jtl == NULL)
  {
    retval = (cw_jtl_t *) _cw_malloc(sizeof(cw_jtl_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_jtl_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_jtl;
    bzero(retval, sizeof(cw_jtl_t));
    retval->is_malloced = FALSE;
  }

  mtx_new(&retval->lock);
  cnd_new(&retval->slock_wait);
  retval->tlock_wait_ring = NULL;
  retval->tlock_wait_count = 0;
  cnd_new(&retval->dlock_wait);
  cnd_new(&retval->qlock_wait);
  cnd_new(&retval->rlock_wait);
  cnd_new(&retval->wlock_wait);
  cnd_new(&retval->xlock_wait);

  RETURN:
  return retval;
}

void
jtl_delete(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  /* Clean up structures. */
  mtx_delete(&a_jtl->lock);
  cnd_delete(&a_jtl->slock_wait);
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

void
jtl_slock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);
  
  while ((a_jtl->tlock_holders > 0)
	 || (a_jtl->tlock_wait_count > 0)
	 )
  {
    a_jtl->slock_waiters++;
    cnd_wait(&a_jtl->slock_wait, &a_jtl->lock);
    a_jtl->slock_waiters--;
  }
  a_jtl->slock_holders++;
  
  mtx_unlock(&a_jtl->lock);
}

cw_jtl_tq_el_t *
jtl_get_tq_el(cw_jtl_t * a_jtl)
{
  cw_jtl_tq_el_t * retval;
  
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  retval = (cw_jtl_tq_el_t *) _cw_malloc(sizeof(cw_jtl_tq_el_t));
  if (NULL == retval)
  {
    goto RETURN;
  }
  
  retval->is_blocked = FALSE;
  cnd_new(&retval->tlock_wait);
  ring_new(&retval->ring_item, NULL, NULL);
  ring_set_data(&retval->ring_item, retval);

  if (0 < a_jtl->tlock_wait_count)
  {
    ring_meld(a_jtl->tlock_wait_ring, &retval->ring_item);
  }
  else
  {
    a_jtl->tlock_wait_ring = &retval->ring_item;
  }
  a_jtl->tlock_wait_count++;

  RETURN:
  mtx_unlock(&a_jtl->lock);
  return retval;
}

void
jtl_tlock(cw_jtl_t * a_jtl, cw_jtl_tq_el_t * a_tq_el)
{
  _cw_check_ptr(a_jtl);
  _cw_check_ptr(a_tq_el);
  _cw_assert(0 < a_jtl->tlock_wait_count);

  mtx_lock(&a_jtl->lock);
  
  if ((a_jtl->tlock_holders == 0)
      && (a_tq_el == ring_get_data(a_jtl->tlock_wait_ring)))
  {
    /* This thread is first in line. */
    a_jtl->tlock_wait_ring = ring_cut(a_jtl->tlock_wait_ring);
    a_jtl->tlock_wait_count--;
    
  }
  else
  {
    a_tq_el->is_blocked = TRUE;
    a_jtl->tlock_waiters++;
    cnd_wait(&a_tq_el->tlock_wait, &a_jtl->lock);
    a_jtl->tlock_waiters--;
    
    a_jtl->tlock_wait_ring = ring_cut(a_jtl->tlock_wait_ring);
    a_jtl->tlock_wait_count--;
    
  }
  a_jtl->tlock_holders++;
  cnd_delete(&a_tq_el->tlock_wait);
  _cw_free(a_tq_el);

  mtx_unlock(&a_jtl->lock);
}

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

void
jtl_sunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->slock_holders--;

  if ((a_jtl->slock_holders == 0)
      && (a_jtl->tlock_wait_count > 0)
      && (((cw_jtl_tq_el_t *)
	   ring_get_data(a_jtl->tlock_wait_ring))->is_blocked == TRUE))
  {
    cnd_signal(&((cw_jtl_tq_el_t *)
		 ring_get_data(a_jtl->tlock_wait_ring))->tlock_wait);
  }
  else if (a_jtl->slock_waiters > 0)
  {
    cnd_broadcast(&a_jtl->slock_wait);
  }
  
  mtx_unlock(&a_jtl->lock);
}

void
jtl_tunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->tlock_holders--;

  if (a_jtl->tlock_waiters > 0)
  {
    cw_jtl_tq_el_t * tq_el;

    _cw_assert(a_jtl->tlock_wait_count > 0);
    tq_el = (cw_jtl_tq_el_t *) ring_get_data(a_jtl->tlock_wait_ring);
    _cw_check_ptr(tq_el);

    if (tq_el->is_blocked == TRUE)
    {
      cnd_signal(&tq_el->tlock_wait);
    }
  }
  else if ((a_jtl->slock_waiters > 0)
	   && (a_jtl->tlock_wait_count == 0))
  {
    cnd_broadcast(&a_jtl->slock_wait);
  }

  mtx_unlock(&a_jtl->lock);
}

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

void
jtl_qunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->qlock_holders--;
  jtl_p_qrwx_unlock(a_jtl);
  
  mtx_unlock(&a_jtl->lock);
}

void
jtl_runlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->rlock_holders--;
  jtl_p_qrwx_unlock(a_jtl);

  mtx_unlock(&a_jtl->lock);
}

void
jtl_wunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->wlock_holders--;
  jtl_p_qrwx_unlock(a_jtl);
  
  mtx_unlock(&a_jtl->lock);
}

void
jtl_xunlock(cw_jtl_t * a_jtl)
{
  _cw_check_ptr(a_jtl);

  mtx_lock(&a_jtl->lock);

  a_jtl->xlock_holders--;
  jtl_p_qrwx_unlock(a_jtl);

  mtx_unlock(&a_jtl->lock);
}

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

static void
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
