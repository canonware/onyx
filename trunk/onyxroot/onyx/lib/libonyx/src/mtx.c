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
    if (pth_mutex_init(&a_mtx->mutex) == false)
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
    if (pth_mutex_acquire(&a_mtx->mutex, false, NULL) == false)
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

bool
mtx_trylock(cw_mtx_t *a_mtx)
{
    bool retval;
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_mtx);

#ifdef CW_PTH
    if (pth_mutex_acquire(&a_mtx->mutex, true, NULL))
    {
	retval = false;
    }
    else if (errno == EBUSY)
    {
	retval = true;
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
	retval = false;
    }
    else if (error == EBUSY)
    {
	retval = true;
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
    if (pth_mutex_release(&a_mtx->mutex) == false)
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
