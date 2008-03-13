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

#include "../include/libstash/libstash.h"

#include <sys/time.h>
#include <errno.h>

void
sma_new(cw_sma_t *a_sma, cw_sint32_t a_count)
{
	_cw_check_ptr(a_sma);

	a_sma->count = a_count;
	a_sma->waiters = 0;

	mtx_new(&a_sma->lock);
	cnd_new(&a_sma->gtzero);
}

void
sma_delete(cw_sma_t *a_sma)
{
	_cw_check_ptr(a_sma);

	mtx_delete(&a_sma->lock);
	cnd_delete(&a_sma->gtzero);
}

void
sma_post(cw_sma_t *a_sma)
{
	_cw_check_ptr(a_sma);

	mtx_lock(&a_sma->lock);

	a_sma->count++;
	if ((a_sma->waiters) && (a_sma->count > 0))
		cnd_signal(&a_sma->gtzero);
	mtx_unlock(&a_sma->lock);
}

void
sma_wait(cw_sma_t *a_sma)
{
	_cw_check_ptr(a_sma);

	mtx_lock(&a_sma->lock);

	while (a_sma->count <= 0) {
		a_sma->waiters++;
		cnd_wait(&a_sma->gtzero, &a_sma->lock);
		a_sma->waiters--;
	}
	a_sma->count--;

	mtx_unlock(&a_sma->lock);
}

cw_bool_t
sma_timedwait(cw_sma_t *a_sma, struct timespec *a_timeout)
{
	cw_bool_t	retval, timed_out;

        _cw_check_ptr(a_sma);
        _cw_check_ptr(a_timeout);

        mtx_lock(&a_sma->lock);

	/*
	 * A spurious wakeup will cause the timeout interval to start over.
	 * This isn't a big deal as long as spurious wakeups don't occur
	 * continuously, since the timeout period is merely a lower bound on how
	 * long to wait.
	 */
	for (timed_out = FALSE; a_sma->count <= 0 && timed_out == FALSE;) {
		a_sma->waiters++;
		timed_out = cnd_timedwait(&a_sma->gtzero, &a_sma->lock,
		    a_timeout);
		a_sma->waiters--;
	}
	if (a_sma->count > 0) {
		a_sma->count--;
		retval = FALSE;
	} else
		retval = TRUE;

	mtx_unlock(&a_sma->lock);

	return retval;
}

cw_bool_t
sma_trywait(cw_sma_t *a_sma)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_sma);

	mtx_lock(&a_sma->lock);

	if (a_sma->count > 0) {
		/* Success. */
		a_sma->count--;
		retval = FALSE;
	} else {
		/* Failure. */
		retval = TRUE;
	}

	mtx_unlock(&a_sma->lock);

	return retval;
}

cw_sint32_t
sma_getvalue(cw_sma_t *a_sma)
{
	cw_sint32_t	retval;

	_cw_check_ptr(a_sma);

	mtx_lock(&a_sma->lock);
	retval = a_sma->count;
	mtx_unlock(&a_sma->lock);

	return retval;
}

void
sma_adjust(cw_sma_t *a_sma, cw_sint32_t a_adjust)
{
	_cw_check_ptr(a_sma);

	mtx_lock(&a_sma->lock);

	a_sma->count += a_adjust;
	if ((a_sma->waiters) && (a_sma->count > 0)) {
		cw_sint32_t	i;

		for (i = 0; (i < a_sma->count) && (i < a_sma->waiters); i++)
			cnd_signal(&a_sma->gtzero);
	}
	mtx_unlock(&a_sma->lock);
}
