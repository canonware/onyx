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
#ifdef _CW_REENTRANT
  list_new(&retval->list, FALSE);
#else
  list_new(&retval->list);
#endif

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
  
  list_delete(&a_mq->list);
  
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
  
  retval = list_hpop(&a_mq->list);

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
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
  mtx_lock(&a_mq->lock);

  if (a_mq->get_stop == TRUE)
  {
    retval = NULL;
    goto RETURN;
  }
  
  while (NULL == (retval = list_hpop(&a_mq->list)))
  {
    cnd_wait(&a_mq->cond, &a_mq->lock);
    if (a_mq->get_stop == TRUE)
    {
      retval = NULL;
      goto RETURN;
    }
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
  
  _cw_check_ptr(a_mq);
  _cw_assert(_LIBSTASH_MQ_MAGIC == a_mq->magic);
#ifdef _CW_REENTRANT
  mtx_lock(&a_mq->lock);
#endif

#ifdef _CW_REENTRANT
  if (0 == list_count(&a_mq->list))
  {
    cnd_broadcast(&a_mq->cond);
  }
#endif
  if (a_mq->put_stop == TRUE)
  {
    retval = 1;
    goto RETURN;
  }
  else if (NULL == list_tpush(&a_mq->list, (void *) a_message))
  {
    retval = -1;
    goto RETURN;
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
