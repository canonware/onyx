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

#include "libstash/libstash_r.h"

#include <sys/time.h>
#include <errno.h>

cw_thd_t *
thd_new(cw_thd_t * a_thd,
	void * (*a_start_func)(void *),
	void * a_arg)
{
  cw_thd_t * retval;
  int error;

  if (a_thd == NULL)
  {
    retval = (cw_thd_t *) _cw_malloc(sizeof(cw_thd_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_thd;
    retval->is_malloced = FALSE;
  }

  error = pthread_create(&retval->thread, NULL, a_start_func, a_arg);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_create(): [s]\n", strerror(error));
    abort();
  }

  RETURN:
  return retval;
}

void
thd_delete(cw_thd_t * a_thd)
{
  int error;
  
  _cw_check_ptr(a_thd);

  error = pthread_detach(a_thd->thread);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_detach(): [s]\n", strerror(error));
    abort();
  }
  
  if (a_thd->is_malloced == TRUE)
  {
    _cw_free(a_thd);
  }
}

void *
thd_join(cw_thd_t * a_thd)
{
  void * retval;
  int error;
  
  _cw_check_ptr(a_thd);

  error = pthread_join(a_thd->thread, &retval);
  if (error)
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Error in pthread_join(): [s]\n", strerror(error));
    abort();
  }

  if (a_thd->is_malloced == TRUE)
  {
    _cw_free(a_thd);
  }
  return retval;
}
