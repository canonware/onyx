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

#include <sys/time.h>
#include <errno.h>

cw_tsd_t *
tsd_new(cw_tsd_t * a_tsd, void (*a_func)(void *))
{
  cw_tsd_t * retval;
  int error;

  if (a_tsd == NULL)
  {
    retval = (cw_tsd_t *) _cw_malloc(sizeof(cw_tsd_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_tsd;
    retval->is_malloced = FALSE;
  }

  error = pthread_key_create(&retval->key, a_func);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_key_create(): [s]\n", strerror(error));
    abort();
  }

  RETURN:
  return retval;
}

void
tsd_delete(cw_tsd_t * a_tsd)
{
  int error;
  
  _cw_check_ptr(a_tsd);

  error = pthread_key_delete(a_tsd->key);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_key_delete(): [s]\n", strerror(error));
    abort();
  }

  if (a_tsd->is_malloced == TRUE)
  {
    _cw_free(a_tsd);
  }
}

void *
tsd_get(cw_tsd_t * a_tsd)
{
  void * retval;
  
  _cw_check_ptr(a_tsd);

  retval = pthread_getspecific(a_tsd->key);
  
  return retval;
}

void
tsd_set(cw_tsd_t * a_tsd, void * a_val)
{
  int error;
  
  _cw_check_ptr(a_tsd);

  error = pthread_setspecific(a_tsd->key, a_val);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_setspecific(): [s]\n", strerror(error));
    abort();
  }
}
