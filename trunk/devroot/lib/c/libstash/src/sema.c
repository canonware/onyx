/****************************************************************************
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

cw_sem_t *
sem_new(cw_sem_t *a_sem, cw_sint32_t a_count)
{
	cw_sem_t	*retval;

	if (a_sem == NULL) {
		retval = (cw_sem_t *)_cw_malloc(sizeof(cw_sem_t));
		if (retval == NULL)
			goto RETURN;
		retval->is_malloced = TRUE;
	} else {
		retval = a_sem;
		retval->is_malloced = FALSE;
	}

	retval->count = a_count;
	retval->waiters = 0;

	mtx_new(&retval->lock);
	cnd_new(&retval->gtzero);

	RETURN:
	return retval;
}

void
sem_delete(cw_sem_t *a_sem)
{
	_cw_check_ptr(a_sem);

	mtx_delete(&a_sem->lock);
	cnd_delete(&a_sem->gtzero);

	if (a_sem->is_malloced)
		_cw_free(a_sem);
}

void
sem_post(cw_sem_t *a_sem)
{
	_cw_check_ptr(a_sem);

	mtx_lock(&a_sem->lock);

	a_sem->count++;
	if ((a_sem->waiters) && (a_sem->count > 0))
		cnd_signal(&a_sem->gtzero);
	mtx_unlock(&a_sem->lock);
}

void
sem_wait(cw_sem_t *a_sem)
{
	_cw_check_ptr(a_sem);

	mtx_lock(&a_sem->lock);

	while (a_sem->count <= 0) {
		a_sem->waiters++;
		cnd_wait(&a_sem->gtzero, &a_sem->lock);
		a_sem->waiters--;
	}
	a_sem->count--;

	mtx_unlock(&a_sem->lock);
}

cw_bool_t
sem_timedwait(cw_sem_t *a_sem, struct timespec *a_timeout)
{
	cw_bool_t	retval;

        _cw_check_ptr(a_sem);
        _cw_check_ptr(a_timeout);

        mtx_lock(&a_sem->lock);

	if (a_sem->count <= 0) {
		a_sem->waiters++;
		cnd_timedwait(&a_sem->gtzero, &a_sem->lock, a_timeout);
		a_sem->waiters--;
	}
	if (a_sem->count > 0) {
		a_sem->count--;
		retval = FALSE;
	} else
		retval = TRUE;

	mtx_unlock(&a_sem->lock);

	return retval;
}

cw_bool_t
sem_trywait(cw_sem_t *a_sem)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_sem);

	mtx_lock(&a_sem->lock);

	if (a_sem->count > 0) {
		/* Success. */
		a_sem->count--;
		retval = FALSE;
	} else {
		/* Failure. */
		retval = TRUE;
	}

	mtx_unlock(&a_sem->lock);

	return retval;
}

cw_sint32_t
sem_getvalue(cw_sem_t *a_sem)
{
	cw_sint32_t	retval;

	_cw_check_ptr(a_sem);

	mtx_lock(&a_sem->lock);
	retval = a_sem->count;
	mtx_unlock(&a_sem->lock);

	return retval;
}

void
sem_adjust(cw_sem_t *a_sem, cw_sint32_t a_adjust)
{
	_cw_check_ptr(a_sem);

	mtx_lock(&a_sem->lock);

	a_sem->count += a_adjust;
	if ((a_sem->waiters) && (a_sem->count > 0)) {
		cw_sint32_t	i;

		for (i = 0; (i < a_sem->count) && (i < a_sem->waiters); i++)
			cnd_signal(&a_sem->gtzero);
	}
	mtx_unlock(&a_sem->lock);
}
