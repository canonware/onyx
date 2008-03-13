/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#include <sys/time.h>
#include <errno.h>

void
cnd_new(cw_cnd_t *a_cnd)
{
	int	error;

	_cw_check_ptr(a_cnd);

	error = pthread_cond_init(&a_cnd->condition, NULL);
	if (error) {
		fprintf(stderr, "%s(): Error in pthread_cond_init(): %s\n",
		    __FUNCTION__, strerror(error));
		abort();
	}
}

void
cnd_delete(cw_cnd_t *a_cnd)
{
	int	error;

	_cw_check_ptr(a_cnd);

	error = pthread_cond_destroy(&a_cnd->condition);
	if (error) {
		fprintf(stderr, "%s(): Error in pthread_cond_destroy(): %s\n",
		    __FUNCTION__, strerror(error));
		abort();
	}
}

void
cnd_signal(cw_cnd_t *a_cnd)
{
	int	error;

	_cw_check_ptr(a_cnd);

	error = pthread_cond_signal(&a_cnd->condition);
	if (error) {
		fprintf(stderr, "%s(): Error in pthread_cond_signal(): %s\n",
		    __FUNCTION__, strerror(error));
		abort();
	}
}

void
cnd_broadcast(cw_cnd_t *a_cnd)
{
	int	error;

	_cw_check_ptr(a_cnd);

	error = pthread_cond_broadcast(&a_cnd->condition);
	if (error) {
		fprintf(stderr, "%s(): Error in pthread_cond_broadcast(): %s\n",
		    __FUNCTION__, strerror(error));
		abort();
	}
}

cw_bool_t
cnd_timedwait(cw_cnd_t *a_cnd, cw_mtx_t *a_mtx, const struct timespec
    *a_timeout)
{
	int		error;
	cw_bool_t	retval;
	struct timeval	now;
	struct timespec	timeout;
	struct timezone	tz;

        _cw_check_ptr(a_cnd);
        _cw_check_ptr(a_mtx);
        _cw_check_ptr(a_timeout);

	/* Set timeout. */
        memset(&tz, 0, sizeof(struct timezone));
        gettimeofday(&now, &tz);
        timeout.tv_nsec = now.tv_usec * 1000 + a_timeout->tv_nsec;
        timeout.tv_sec = (now.tv_sec + a_timeout->tv_sec
	    + (timeout.tv_nsec / 1000000000));	/*
						 * Carry if nanoseconds
						 * overflowed.
						 */
	/*
	 * Chop off the number of nanoseconds to be less than one
	 * second.
	 */
        timeout.tv_nsec %= 1000000000;

        error = pthread_cond_timedwait(&a_cnd->condition, &a_mtx->mutex,
	    &timeout);
	if (error == 0)
		retval = FALSE;
	else if (error == ETIMEDOUT)
		retval = TRUE;
	else {
		fprintf(stderr,
		    "%s(): Error in pthread_cond_timedwait(): %s\n",
		    __FUNCTION__, strerror(error));
		abort();
	}

	return retval;
}

void
cnd_wait(cw_cnd_t *a_cnd, cw_mtx_t *a_mtx)
{
	int	error;

	_cw_check_ptr(a_cnd);
	_cw_check_ptr(a_mtx);

	error = pthread_cond_wait(&a_cnd->condition, &a_mtx->mutex);
	if (error) {
		fprintf(stderr, "%s(): Error in pthread_cond_wait: %s\n",
		    __FUNCTION__, strerror(error));
		abort();
	}
}
