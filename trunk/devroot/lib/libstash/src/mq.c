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
 * <<< Description >>>
 *
 * Implementation of the mq (message queue) class.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_MQ
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#ifdef _LIBSTASH_DBG
#  define _LIBSTASH_MQ_MAGIC 0xab01cd23
#endif

cw_mq_t *
mq_new(cw_mq_t * a_mq)
{
  cw_mq_t * retval;
  
  if (NULL != a_mq)
  {
    retval = a_mq;
    a_mq->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_mq_t *) _cw_malloc(sizeof(cw_mq_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }

#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
  cnd_new(&retval->cond);
#endif

  retval->get_stop = FALSE;
  retval->put_stop = FALSE;
  retval->ring = NULL;
  retval->spares_ring = NULL;

#ifdef _LIBSTASH_DBG
  retval->magic = _LIBSTASH_MQ_MAGIC;
#endif

  RETURN:
  return retval;
}

void
mq_delete(cw_mq_t * a_mq)
{
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);

#ifdef _CW_REENTRANT
  mtx_delete(&a_mq->lock);
  cnd_delete(&a_mq->cond);
#endif

  if (NULL != a_mq->ring)
  {
    ring_delete(a_mq->ring);
  }
  if (NULL != a_mq->spares_ring)
  {
    ring_delete(a_mq->spares_ring);
  }
  
  if (TRUE == a_mq->is_malloced)
  {
    _cw_free(a_mq);
  }
#ifdef _LIBSTASH_DBG
  else
  {
    memset(a_mq, 0x5a, sizeof(cw_mq_t));
  }
#endif
}

void *
mq_tryget(cw_mq_t * a_mq)
{
  void * retval;
  cw_ring_t * t_ring;
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
#ifdef _CW_REENTRANT
  mtx_lock(&a_mq->lock);
#endif

  if (a_mq->get_stop == TRUE)
  {
    retval = NULL;
    goto RETURN;
  }

  if (NULL != a_mq->ring)
  {
    t_ring = a_mq->ring;
    a_mq->ring = ring_cut(t_ring);
    if (a_mq->ring == t_ring)
    {
      a_mq->ring = NULL;
    }
    retval = ring_get_data(t_ring);

    if (NULL != a_mq->spares_ring)
    {
      ring_meld(t_ring, a_mq->spares_ring);
    }
    a_mq->spares_ring = t_ring;
  }
  else
  {
    retval = NULL;
  }

  RETURN:
#ifdef _CW_REENTRANT
  mtx_unlock(&a_mq->lock);
#endif
  return retval;
}

#ifdef _CW_REENTRANT
void *
mq_get(cw_mq_t * a_mq)
{
  void * retval;
  cw_ring_t * t_ring;
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
  mtx_lock(&a_mq->lock);

  if (TRUE == a_mq->get_stop)
  {
    retval = NULL;
    goto RETURN;
  }

  while (NULL == a_mq->ring)
  {
    cnd_wait(&a_mq->cond, &a_mq->lock);
    if (a_mq->get_stop == TRUE)
    {
      retval = NULL;
      goto RETURN;
    }
  }

  t_ring = a_mq->ring;
  a_mq->ring = ring_cut(t_ring);
  if (a_mq->ring == t_ring)
  {
    a_mq->ring = NULL;
  }
  retval = ring_get_data(t_ring);

  if (NULL != a_mq->spares_ring)
  {
    ring_meld(t_ring, a_mq->spares_ring);
  }
  a_mq->spares_ring = t_ring;
  
  RETURN:
  mtx_unlock(&a_mq->lock);
  return retval;
}

void *
mq_timedget(cw_mq_t * a_mq, struct timespec * a_timeout)
{
  void * retval;
  cw_ring_t * t_ring;
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
  _cw_check_ptr(a_timeout);
  
  mtx_lock(&a_mq->lock);

  if (a_mq->get_stop == TRUE)
  {
    retval = NULL;
    goto RETURN;
  }

  if (NULL == a_mq->ring)
  {
    cnd_timedwait(&a_mq->cond, &a_mq->lock, a_timeout);
    if (a_mq->get_stop == TRUE)
    {
      retval = NULL;
      goto RETURN;
    }
  }

  if (NULL != a_mq->ring)
  {
    t_ring = a_mq->ring;
    a_mq->ring = ring_cut(t_ring);
    if (a_mq->ring == t_ring)
    {
      a_mq->ring = NULL;
    }
    retval = ring_get_data(t_ring);

    if (NULL != a_mq->spares_ring)
    {
      ring_meld(t_ring, a_mq->spares_ring);
    }
    a_mq->spares_ring = t_ring;
  }
  else
  {
    retval = NULL;
  }
  
  RETURN:
  mtx_unlock(&a_mq->lock);
  return retval;
}
#endif

cw_sint32_t
mq_put(cw_mq_t * a_mq, const void * a_message)
{
  cw_sint32_t retval;
  cw_ring_t * t_ring;
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
#ifdef _CW_REENTRANT
  mtx_lock(&a_mq->lock);
#endif

#ifdef _CW_REENTRANT
  if (NULL == a_mq->ring)
  {
    cnd_broadcast(&a_mq->cond);
  }
#endif
  
  if (a_mq->put_stop == TRUE)
  {
    retval = 1;
    goto RETURN;
  }
  else
  {
    if (NULL != a_mq->spares_ring)
    {
      t_ring = a_mq->spares_ring;
      a_mq->spares_ring = ring_cut(t_ring);
      if (a_mq->spares_ring == t_ring)
      {
	a_mq->spares_ring = NULL;
      }
    }
    else
    {
      t_ring = ring_new(NULL, NULL, NULL);
      if (NULL == t_ring)
      {
	retval = -1;
	goto RETURN;
      }
    }
    
    ring_set_data(t_ring, (void *) a_message);
    if (NULL != a_mq->ring)
    {
      ring_meld(t_ring, a_mq->ring);
    }
    else
    {
      a_mq->ring = t_ring;
    }
  }

  retval = 0;
  
  RETURN:
#ifdef _CW_REENTRANT
  mtx_unlock(&a_mq->lock);
#endif
  return retval;
}

cw_bool_t
mq_start_get(cw_mq_t * a_mq)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
#ifdef _CW_REENTRANT
  mtx_lock(&a_mq->lock);
#endif

  if (FALSE == a_mq->get_stop)
  {
    retval = TRUE;
    goto RETURN;
  }

  a_mq->get_stop = FALSE;
  
  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  mtx_unlock(&a_mq->lock);
#endif
  return retval;
}

cw_bool_t
mq_stop_get(cw_mq_t * a_mq)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
#ifdef _CW_REENTRANT
  mtx_lock(&a_mq->lock);
#endif

  if (TRUE == a_mq->get_stop)
  {
    retval = TRUE;
    goto RETURN;
  }

#ifdef _CW_REENTRANT
  cnd_broadcast(&a_mq->cond);
#endif
  a_mq->get_stop = TRUE;
  
  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  mtx_unlock(&a_mq->lock);
#endif
  return retval;
}

cw_bool_t
mq_start_put(cw_mq_t * a_mq)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
#ifdef _CW_REENTRANT
  mtx_lock(&a_mq->lock);
#endif

  if (FALSE == a_mq->put_stop)
  {
    retval = TRUE;
    goto RETURN;
  }

  a_mq->put_stop = FALSE;
  
  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  mtx_unlock(&a_mq->lock);
#endif
  return retval;
}

cw_bool_t
mq_stop_put(cw_mq_t * a_mq)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
#ifdef _CW_REENTRANT
  mtx_lock(&a_mq->lock);
#endif

  if (TRUE == a_mq->put_stop)
  {
    retval = TRUE;
    goto RETURN;
  }

  a_mq->put_stop = TRUE;
  
  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  mtx_unlock(&a_mq->lock);
#endif
  return retval;
}
