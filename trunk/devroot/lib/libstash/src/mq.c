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

#ifdef _LIBSTASH_DBG
#define _LIBSTASH_MQ_MAGIC 0xab01cd23
#endif

cw_mq_t *
mq_new(cw_mq_t *a_mq)
{
	cw_mq_t	*retval;

	if (a_mq != NULL) {
		retval = a_mq;
		a_mq->is_malloced = FALSE;
	} else {
		retval = (cw_mq_t *)_cw_malloc(sizeof(cw_mq_t));
		if (retval == NULL)
			goto RETURN;
		retval->is_malloced = TRUE;
	}

	mtx_new(&retval->lock);
	cnd_new(&retval->cond);

	retval->get_stop = FALSE;
	retval->put_stop = FALSE;
	retval->ring = NULL;
	retval->spares_ring = NULL;

#ifdef _LIBSTASH_DBG
	retval->magic = _LIBSTASH_MQ_MAGIC;
#endif

	RETURN:
	return retval;
}

void
mq_delete(cw_mq_t *a_mq)
{
	cw_ring_t	*t_ring;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	mtx_delete(&a_mq->lock);
	cnd_delete(&a_mq->cond);

	if (a_mq->ring != NULL) {
		do {
			t_ring = a_mq->ring;
			a_mq->ring = ring_cut(t_ring);
			ring_delete(t_ring);
			_cw_free(t_ring);
		} while (t_ring != a_mq->ring);
	}
	if (a_mq->spares_ring != NULL) {
		do {
			t_ring = a_mq->spares_ring;
			a_mq->spares_ring = ring_cut(t_ring);
			ring_delete(t_ring);
			_cw_free(t_ring);
		} while (t_ring != a_mq->spares_ring);
	}
	if (a_mq->is_malloced)
		_cw_free(a_mq);
#ifdef _LIBSTASH_DBG
	else
		memset(a_mq, 0x5a, sizeof(cw_mq_t));
#endif
}

void *
mq_tryget(cw_mq_t *a_mq)
{
	void		*retval;
	cw_ring_t	*t_ring;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
	mtx_lock(&a_mq->lock);

	if (a_mq->get_stop) {
		retval = NULL;
		goto RETURN;
	}
	if (a_mq->ring != NULL) {
		t_ring = a_mq->ring;
		a_mq->ring = ring_cut(t_ring);
		if (t_ring == a_mq->ring)
			a_mq->ring = NULL;
		retval = ring_get_data(t_ring);

		if (a_mq->spares_ring != NULL)
			ring_meld(t_ring, a_mq->spares_ring);
		a_mq->spares_ring = t_ring;
	} else
		retval = NULL;

	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

void *
mq_get(cw_mq_t *a_mq)
{
	void		*retval;
	cw_ring_t	*t_ring;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
	mtx_lock(&a_mq->lock);

	if (a_mq->get_stop) {
		retval = NULL;
		goto RETURN;
	}
	while (a_mq->ring == NULL) {
		cnd_wait(&a_mq->cond, &a_mq->lock);
		if (a_mq->get_stop) {
			retval = NULL;
			goto RETURN;
		}
	}

	t_ring = a_mq->ring;
	a_mq->ring = ring_cut(t_ring);
	if (t_ring == a_mq->ring)
		a_mq->ring = NULL;
	retval = ring_get_data(t_ring);

	if (a_mq->spares_ring != NULL)
		ring_meld(t_ring, a_mq->spares_ring);
	a_mq->spares_ring = t_ring;

	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

void *
mq_timedget(cw_mq_t *a_mq, const struct timespec * a_timeout)
{
	void		*retval;
	cw_ring_t	*t_ring;

        _cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
        _cw_check_ptr(a_timeout);

        mtx_lock(&a_mq->lock);

	if (a_mq->get_stop) {
		retval = NULL;
		goto RETURN;
	}
	if (a_mq->ring == NULL) {
		cnd_timedwait(&a_mq->cond, &a_mq->lock, a_timeout);
		if (a_mq->get_stop) {
			retval = NULL;
			goto RETURN;
		}
	}
	if (a_mq->ring != NULL) {
		t_ring = a_mq->ring;
		a_mq->ring = ring_cut(t_ring);
		if (t_ring == a_mq->ring)
			a_mq->ring = NULL;
		retval = ring_get_data(t_ring);

		if (a_mq->spares_ring != NULL)
			ring_meld(t_ring, a_mq->spares_ring);
		a_mq->spares_ring = t_ring;
	} else
		retval = NULL;

	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_sint32_t
mq_put(cw_mq_t *a_mq, const void *a_message)
{
	cw_sint32_t	retval;
	cw_ring_t	*t_ring;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
	mtx_lock(&a_mq->lock);

	if (a_mq->ring == NULL)
		cnd_broadcast(&a_mq->cond);
	if (a_mq->put_stop) {
		retval = 1;
		goto RETURN;
	} else {
		if (a_mq->spares_ring != NULL) {
			t_ring = a_mq->spares_ring;
			a_mq->spares_ring = ring_cut(t_ring);
			if (t_ring == a_mq->spares_ring)
				a_mq->spares_ring = NULL;
		} else {
			t_ring = (cw_ring_t *)_cw_malloc(sizeof(cw_ring_t));
			if (t_ring == NULL) {
				retval = -1;
				goto RETURN;
			}
			ring_new(t_ring);
		}

		ring_set_data(t_ring, (void *)a_message);
		if (a_mq->ring != NULL)
			ring_meld(t_ring, a_mq->ring);
		else
			a_mq->ring = t_ring;
	}

	retval = 0;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_bool_t
mq_start_get(cw_mq_t *a_mq)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
	mtx_lock(&a_mq->lock);

	if (a_mq->get_stop == FALSE) {
		retval = TRUE;
		goto RETURN;
	}
	a_mq->get_stop = FALSE;

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_bool_t
mq_stop_get(cw_mq_t *a_mq)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
	mtx_lock(&a_mq->lock);

	if (a_mq->get_stop) {
		retval = TRUE;
		goto RETURN;
	}
	cnd_broadcast(&a_mq->cond);
	a_mq->get_stop = TRUE;

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_bool_t
mq_start_put(cw_mq_t *a_mq)
{
	cw_bool_t retval;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
	mtx_lock(&a_mq->lock);

	if (a_mq->put_stop == FALSE) {
		retval = TRUE;
		goto RETURN;
	}
	a_mq->put_stop = FALSE;

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_bool_t
mq_stop_put(cw_mq_t *a_mq)
{
	cw_bool_t retval;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
	mtx_lock(&a_mq->lock);

	if (a_mq->put_stop) {
		retval = TRUE;
		goto RETURN;
	}
	a_mq->put_stop = TRUE;

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}
