/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1998
 * Jason Evans <jasone@canonware.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JASON EVANS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL JASON EVANS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 26 $
 * $Date: 1998-04-12 04:09:42 -0700 (Sun, 12 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_THREAD_H_
#define _INC_THREAD_PRIV_H_
#define _INC_PTHREAD_H_
#include <config.h>

cw_thd_t *
thd_new(cw_thd_t * arg_thd_obj,
	void * (*arg_start_func)(void *),
	void * arg_arg)
{
  cw_thd_t * retval;
  int error;

  if (arg_thd_obj == NULL)
  {
    retval = (cw_thd_t *) _cw_malloc(sizeof(cw_thd_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = arg_thd_obj;
    retval->is_malloced = FALSE;
  }

  error = pthread_create(&retval->thread, NULL, arg_start_func, arg_arg);

  if (error)
  {
    log_eprintf(g_log_obj, __FILE__, __LINE__, "thd_new",
		"Cannot create thread, error %d\n", error);
    abort();
  }

/*   error = pthread_detach(retval->thread); */

/*   if (error) */
/*   { */
/*     log_eprintf(g_log_obj, __FILE__, __LINE__, "thd_new", */
/* 		"Cannot detach thread, error %d\n", error); */
/*     abort(); */
/*   } */
  
  return retval;
}

void
thd_delete(cw_thd_t * arg_thd_obj)
{
  int exit_val = 0;

  _cw_check_ptr(arg_thd_obj);


  /* Check to make sure we're exiting ourself, not another thread. */
  if (!pthread_equal(pthread_self(), arg_thd_obj->thread))
  {
    log_printf(g_log_obj, __FILE__, __LINE__, "thd_delete",
	       "Tried to exit another thread\n");
    abort();
  }

  /* XXX Need a variable to indicate whether we are responsible for
   * deallocation. */
  if (arg_thd_obj->is_malloced == TRUE)
  {
    _cw_free(arg_thd_obj);
  }
  
  pthread_exit(&exit_val);
}

cw_mtx_t *
mtx_new(cw_mtx_t * arg_mtx_obj)
{
}

void
mtx_delete(cw_mtx_t * arg_mtx_obj)
{
}

void
mtx_lock(cw_mtx_t * arg_mtx_obj)
{
}

cw_bool_t
mtx_trylock(cw_mtx_t * arg_mtx_obj)
{
}

void
mtx_unlock(cw_mtx_t * arg_mtx_obj)
{
}

cw_cnd_t *
cnd_new(cw_cnd_t * arg_cnt_obj)
{
}

void
cnd_delete(cw_cnd_t * arg_cnt_obj)
{
}

void
cnd_signal(cw_cnd_t * arg_cnt_obj)
{
}

void
cnd_broadcast(cw_cnd_t * arg_cnt_obj)
{
}

cw_bool_t
cnd_timedwait(cw_cnd_t * arg_cnt_obj, cw_mtx_t * arg_mtx_obj,
	      struct timespec arg_time)
{
}

void
cnd_wait(cw_cnd_t * arg_cnt_obj, cw_mtx_t * arg_mtx_obj)
{
}

cw_sem_t *
sem_new(cw_sem_t * arg_sem_obj, cw_uint32_t arg_count)
{
}

void
sem_delete(cw_sem_t * arg_sem_obj)
{
}

void
sem_post(cw_sem_t * arg_sem_obj)
{
}

void
sem_wait(cw_sem_t * arg_sem_obj)
{
}

cw_rwl_t *
rwl_new(cw_rwl_t * arg_rwl_obj)
{
}

void
rwl_delete(cw_rwl_t * arg_rwl_obj)
{
}

void
rwl_rlock(cw_rwl_t * arg_rwl_obj)
{
}

void
rwl_runlock(cw_rwl_t * arg_rwl_obj)
{
}

void
rwl_wlock(cw_rwl_t * arg_rwl_obj)
{
}

void
rwl_wunlock(cw_rwl_t * arg_rwl_obj)
{
}

cw_tsd_t *
tsd_new(cw_tsd_t * arg_tsd_obj)
{
}

void
tsd_delete(cw_tsd_t * arg_tsd_obj)
{
}

void **
tsd_get(cw_tsd_t * arg_tsd_obj, cw_thd_t * arg_thd_obj)
{
}

void
tsd_set(cw_tsd_t * arg_tsd_obj, cw_thd_t * arg_thd_obj,
	void ** arg_val)
{
}

