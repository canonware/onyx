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

#include "libstash/libstash.h"

#include <sys/time.h>
#include <errno.h>

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
