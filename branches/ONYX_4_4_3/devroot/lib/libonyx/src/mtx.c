/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
******************************************************************************
*
* <Copyright = jasone>
* <License>
*
******************************************************************************
*
* Version: Onyx <Version = onyx>
*
******************************************************************************/

#include "../include/libonyx/libonyx.h"

#include <sys/time.h>
#include <errno.h>

void
mtx_new(cw_mtx_t *a_mtx)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_mtx);

#ifdef CW_PTH
    if (pth_mutex_init(&a_mtx->mutex) == FALSE)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_mutex_init: %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_mutex_init(&a_mtx->mutex, NULL);
    if (error)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_mutex_init: %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

void
mtx_delete(cw_mtx_t *a_mtx)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_mtx);

#ifdef CW_PTHREADS
    error = pthread_mutex_destroy(&a_mtx->mutex);
    if (error)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_mutex_destroy(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

void
mtx_lock(cw_mtx_t *a_mtx)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_mtx);

#ifdef CW_PTH
    if (pth_mutex_acquire(&a_mtx->mutex, FALSE, NULL) == FALSE)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_mutex_acquire(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_mutex_lock(&a_mtx->mutex);
    if (error)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_mutex_lock(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

cw_bool_t
mtx_trylock(cw_mtx_t *a_mtx)
{
    cw_bool_t retval;
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_mtx);

#ifdef CW_PTH
    if (pth_mutex_acquire(&a_mtx->mutex, TRUE, NULL))
    {
	retval = FALSE;
    }
    else if (errno == EBUSY)
    {
	retval = TRUE;
    }
    else
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_mutex_acquire(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
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
	fprintf(stderr, "%s:%d:%s(): Error in pthread_mutex_trylock(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif

    return retval;
}

void
mtx_unlock(cw_mtx_t *a_mtx)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_mtx);

#ifdef CW_PTH
    if (pth_mutex_release(&a_mtx->mutex) == FALSE)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_mutex_release(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_mutex_unlock(&a_mtx->mutex);
    if (error)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_mutex_unlock(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}
