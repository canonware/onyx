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
sema_new(cw_sema_t *a_sema, cw_sint32_t a_count)
{
	_cw_check_ptr(a_sema);

	a_sema->count = a_count;
	a_sema->waiters = 0;

	mtx_new(&a_sema->lock);
	cnd_new(&a_sema->gtzero);
}

void
sema_delete(cw_sema_t *a_sema)
{
	_cw_check_ptr(a_sema);

	mtx_delete(&a_sema->lock);
	cnd_delete(&a_sema->gtzero);
}

void
sema_post(cw_sema_t *a_sema)
{
	_cw_check_ptr(a_sema);

	mtx_lock(&a_sema->lock);

	a_sema->count++;
	if ((a_sema->waiters) && (a_sema->count > 0))
		cnd_signal(&a_sema->gtzero);
	mtx_unlock(&a_sema->lock);
}

void
sema_wait(cw_sema_t *a_sema)
{
	_cw_check_ptr(a_sema);

	mtx_lock(&a_sema->lock);

	while (a_sema->count <= 0) {
		a_sema->waiters++;
		cnd_wait(&a_sema->gtzero, &a_sema->lock);
		a_sema->waiters--;
	}
	a_sema->count--;

	mtx_unlock(&a_sema->lock);
}

cw_bool_t
sema_timedwait(cw_sema_t *a_sema, struct timespec *a_timeout)
{
	cw_bool_t	retval;

        _cw_check_ptr(a_sema);
        _cw_check_ptr(a_timeout);

        mtx_lock(&a_sema->lock);

	if (a_sema->count <= 0) {
		a_sema->waiters++;
		cnd_timedwait(&a_sema->gtzero, &a_sema->lock, a_timeout);
		a_sema->waiters--;
	}
	if (a_sema->count > 0) {
		a_sema->count--;
		retval = FALSE;
	} else
		retval = TRUE;

	mtx_unlock(&a_sema->lock);

	return retval;
}

cw_bool_t
sema_trywait(cw_sema_t *a_sema)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_sema);

	mtx_lock(&a_sema->lock);

	if (a_sema->count > 0) {
		/* Success. */
		a_sema->count--;
		retval = FALSE;
	} else {
		/* Failure. */
		retval = TRUE;
	}

	mtx_unlock(&a_sema->lock);

	return retval;
}

cw_sint32_t
sema_getvalue(cw_sema_t *a_sema)
{
	cw_sint32_t	retval;

	_cw_check_ptr(a_sema);

	mtx_lock(&a_sema->lock);
	retval = a_sema->count;
	mtx_unlock(&a_sema->lock);

	return retval;
}

void
sema_adjust(cw_sema_t *a_sema, cw_sint32_t a_adjust)
{
	_cw_check_ptr(a_sema);

	mtx_lock(&a_sema->lock);

	a_sema->count += a_adjust;
	if ((a_sema->waiters) && (a_sema->count > 0)) {
		cw_sint32_t	i;

		for (i = 0; (i < a_sema->count) && (i < a_sema->waiters); i++)
			cnd_signal(&a_sema->gtzero);
	}
	mtx_unlock(&a_sema->lock);
}
