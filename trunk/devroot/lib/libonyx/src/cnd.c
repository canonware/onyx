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
cnd_new(cw_cnd_t *a_cnd)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_cnd);

#ifdef CW_PTH
    if (pth_cond_init(&a_cnd->condition) == FALSE)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_cond_init(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_cond_init(&a_cnd->condition, NULL);
    if (error)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_cond_init(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

void
cnd_delete(cw_cnd_t *a_cnd)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_cnd);

#ifdef CW_PTHREADS
    error = pthread_cond_destroy(&a_cnd->condition);
    if (error)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_cond_destroy(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

void
cnd_signal(cw_cnd_t *a_cnd)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_cnd);

#ifdef CW_PTH
    if (pth_cond_notify(&a_cnd->condition, FALSE) == FALSE)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_cond_notify(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_cond_signal(&a_cnd->condition);
    if (error) {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_cond_signal(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

void
cnd_broadcast(cw_cnd_t *a_cnd)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_cnd);

#ifdef CW_PTH
    if (pth_cond_notify(&a_cnd->condition, TRUE) == FALSE)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_cond_notify(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_cond_broadcast(&a_cnd->condition);
    if (error)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_cond_broadcast(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

cw_bool_t
cnd_timedwait(cw_cnd_t *a_cnd, cw_mtx_t *a_mtx,
	      const struct timespec *a_timeout)
{
    cw_bool_t retval;
#ifdef CW_PTH
    pth_event_t event;
#endif
#ifdef CW_PTHREADS
    int error;
#endif
    struct timeval now;
    struct timespec timeout;
    struct timezone tz;

    cw_check_ptr(a_cnd);
    cw_check_ptr(a_mtx);
    cw_check_ptr(a_timeout);

    /* Set timeout. */
    memset(&tz, 0, sizeof(struct timezone));
    gettimeofday(&now, &tz);
    timeout.tv_nsec = now.tv_usec * 1000 + a_timeout->tv_nsec;
    timeout.tv_sec = (now.tv_sec + a_timeout->tv_sec
		      + (timeout.tv_nsec / 1000000000)); /* Carry if nanoseconds
							  * overflowed. */
    /* Chop off the number of nanoseconds to be less than one second. */
    timeout.tv_nsec %= 1000000000;

#ifdef CW_PTH
    event = pth_event(PTH_EVENT_TIME,
		      pth_time(timeout.tv_sec, timeout.tv_nsec / 1000));

    if (pth_cond_await(&a_cnd->condition, &a_mtx->mutex, event) == FALSE)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_cond_await(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }

    if (pth_event_occurred(event) == FALSE)
    {
	retval = FALSE;
    }
    else
    {
	retval = TRUE;
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_cond_timedwait(&a_cnd->condition, &a_mtx->mutex, &timeout);
    if (error == 0)
    {
	retval = FALSE;
    }
    else if (error == ETIMEDOUT)
    {
	retval = TRUE;
    }
    else
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_cond_timedwait(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif

    return retval;
}

void
cnd_wait(cw_cnd_t *a_cnd, cw_mtx_t *a_mtx)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_cnd);
    cw_check_ptr(a_mtx);

#ifdef CW_PTH
    if (pth_cond_await(&a_cnd->condition, &a_mtx->mutex, NULL) == FALSE)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pth_cond_wait: %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_cond_wait(&a_cnd->condition, &a_mtx->mutex);
    if (error)
    {
	fprintf(stderr, "%s:%d:%s(): Error in pthread_cond_wait: %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}
