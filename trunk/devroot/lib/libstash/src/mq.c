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

#include <stdarg.h>

#ifdef _LIBSTASH_DBG
#define _LIBSTASH_MQ_MAGIC 0xab01cd23
#endif

/*
 * Number of messages to allocate space for when a message queue is first
 * created.  Since message space is doubled when more space is needed, this
 * number isn't very important to performance, but if it is too high, space may
 * be wasted.
 */
#define _LIBSTASH_MQ_ARRAY_MIN_SIZE 8

cw_mq_t *
mq_new(cw_mq_t *a_mq, cw_uint32_t a_msg_size)
{
	cw_mq_t	*retval;

	if (a_mq != NULL) {
		retval = a_mq;
		a_mq->is_malloced = FALSE;
	} else {
		retval = (cw_mq_t *)_cw_malloc(sizeof(cw_mq_t));
		if (retval == NULL)
			goto OOM_1;
		retval->is_malloced = TRUE;
	}

	retval->msg_vec_count = _LIBSTASH_MQ_ARRAY_MIN_SIZE;
	retval->msg_count = 0;
	retval->msgs_beg = 0;
	retval->msgs_end = 0;

	switch (a_msg_size) {
	case 1: case 2: case 4:
		retval->msg_size = 4;
		break;
	case 8:
		retval->msg_size = 8;
		break;
	default:
		_cw_error("Programming error");
	}

	retval->msgs.x = (cw_uint32_t *)_cw_malloc(retval->msg_vec_count *
	    retval->msg_size);
	if (retval->msgs.x == NULL)
		goto OOM_2;

	mtx_new(&retval->lock);
	cnd_new(&retval->cond);

	retval->get_stop = FALSE;
	retval->put_stop = FALSE;

#ifdef _LIBSTASH_DBG
	retval->magic = _LIBSTASH_MQ_MAGIC;
#endif

	return retval;

	OOM_2:
	if (a_mq->is_malloced)
		_cw_free(retval);
	retval = NULL;
	OOM_1:
	return retval;
}

void
mq_delete(cw_mq_t *a_mq)
{
	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	mtx_delete(&a_mq->lock);
	cnd_delete(&a_mq->cond);

	_cw_free(a_mq->msgs.x);

	if (a_mq->is_malloced)
		_cw_free(a_mq);
#ifdef _LIBSTASH_DBG
	else
		memset(a_mq, 0x5a, sizeof(cw_mq_t));
#endif
}

cw_bool_t
mq_tryget(cw_mq_t *a_mq, ...)
{
	cw_bool_t	retval;
	union {
		cw_uint32_t	**four;
		cw_uint64_t	**eight;
		void		**x;	/* Don't care. */
	}		r_msg;
	va_list		ap;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	va_start(ap, a_mq);
	r_msg.x = (void **)&va_arg(ap, void *);
	va_end(ap);

	mtx_lock(&a_mq->lock);

	if (a_mq->get_stop) {
		retval = TRUE;
		goto RETURN;
	}
	if (a_mq->msg_count > 0) {
		switch (a_mq->msg_size) {
		case 4:
			*r_msg.four = &a_mq->msgs.four[a_mq->msgs_beg];
			break;
		case 8:
			*r_msg.eight = &a_mq->msgs.eight[a_mq->msgs_beg];
			break;
		default:
			_cw_error("Programming error");
		}
		a_mq->msg_count--;
		a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msg_vec_count;
	} else {
		retval = TRUE;
		goto RETURN;
	}

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_bool_t
mq_timedget(cw_mq_t *a_mq, const struct timespec * a_timeout, ...)
{
	cw_bool_t	retval;
	union {
		cw_uint32_t	**four;
		cw_uint64_t	**eight;
		void		**x;	/* Don't care. */
	}		r_msg;
	va_list		ap;

        _cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
        _cw_check_ptr(a_timeout);

	va_start(ap, a_timeout);
	r_msg.x = (void **)&va_arg(ap, void *);
	va_end(ap);

        mtx_lock(&a_mq->lock);

	if (a_mq->get_stop) {
		retval = TRUE;
		goto RETURN;
	}
	if (a_mq->msg_count == 0) {
		cnd_timedwait(&a_mq->cond, &a_mq->lock, a_timeout);
		if (a_mq->get_stop) {
			retval = TRUE;
			goto RETURN;
		}
	}
	if (a_mq->msg_count > 0) {
		switch (a_mq->msg_size) {
		case 4:
			*r_msg.four = &a_mq->msgs.four[a_mq->msgs_beg];
			break;
		case 8:
			*r_msg.eight = &a_mq->msgs.eight[a_mq->msgs_beg];
			break;
		default:
			_cw_error("Programming error");
		}
		a_mq->msg_count--;
		a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msg_vec_count;
	} else {
		retval = TRUE;
		goto RETURN;
	}

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_bool_t
mq_get(cw_mq_t *a_mq, ...)
{
	cw_bool_t	retval;
	union {
		cw_uint32_t	**four;
		cw_uint64_t	**eight;
		void		**x;	/* Don't care. */
	}		r_msg;
	va_list		ap;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	va_start(ap, a_mq);
	r_msg.x = (void **)&va_arg(ap, void *);
	va_end(ap);

	mtx_lock(&a_mq->lock);

	if (a_mq->get_stop) {
		retval = TRUE;
		goto RETURN;
	}
	while (a_mq->msg_count == 0) {
		cnd_wait(&a_mq->cond, &a_mq->lock);
		if (a_mq->get_stop) {
			retval = TRUE;
			goto RETURN;
		}
	}

	switch (a_mq->msg_size) {
	case 4:
		*r_msg.four = &a_mq->msgs.four[a_mq->msgs_beg];
		break;
	case 8:
		*r_msg.eight = &a_mq->msgs.eight[a_mq->msgs_beg];
		break;
	default:
		_cw_error("Programming error");
	}
	a_mq->msg_count--;
	a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msg_vec_count;

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

/* XXX */
cw_sint32_t
mq_put(cw_mq_t *a_mq, ...)
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
