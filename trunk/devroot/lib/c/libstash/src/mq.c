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
#ifdef _LIBSTASH_DBG
#define _LIBSTASH_MQ_ARRAY_MIN_SIZE 1
#else
#define _LIBSTASH_MQ_ARRAY_MIN_SIZE 8
#endif

cw_mq_t *
mq_new(cw_mq_t *a_mq, cw_mem_t *a_mem, cw_uint32_t a_msg_size)
{
	cw_mq_t			*retval;
	volatile cw_uint32_t	try_stage = 0;

	xep_begin();
	volatile cw_mq_t	*v_retval;
	xep_try {
		if (a_mq != NULL) {
			v_retval = retval = a_mq;
			a_mq->is_malloced = FALSE;
		} else {
			v_retval = retval = (cw_mq_t *)mem_malloc(a_mem,
			    sizeof(cw_mq_t));
			retval->is_malloced = TRUE;
		}
		try_stage = 1;
	
		retval->mem = a_mem;
		retval->msg_count = 0;

		switch (a_msg_size) {
		case 1:
			retval->msg_size = 1;
			break;
		case 2:
			retval->msg_size = 2;
			break;
		case 4:
			retval->msg_size = 4;
			break;
		case 8:
			retval->msg_size = 8;
			break;
		default:
			_cw_not_reached();
		}

		retval->msgs_vec_count = _LIBSTASH_MQ_ARRAY_MIN_SIZE;
		retval->msgs_beg = 0;
		retval->msgs_end = 0;

		retval->msgs.x = (cw_uint32_t *)mem_malloc(a_mem,
		    retval->msgs_vec_count * retval->msg_size);
		try_stage = 2;

		mtx_new(&retval->lock);
		cnd_new(&retval->cond);

		retval->get_stop = FALSE;
		retval->put_stop = FALSE;

#ifdef _LIBSTASH_DBG
		retval->magic = _LIBSTASH_MQ_MAGIC;
#endif
	}
	xep_catch(_CW_XEPV_OOM) {
		retval = (cw_mq_t *)v_retval;
		switch (try_stage) {
		case 1:
			if (a_mq->is_malloced)
				mem_free(a_mem, retval);
		case 0:
			break;
		default:
			_cw_not_reached();
		}
		break;
	}
	xep_end();

	return retval;
}

void
mq_delete(cw_mq_t *a_mq)
{
	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	mtx_delete(&a_mq->lock);
	cnd_delete(&a_mq->cond);

	mem_free(a_mq->mem, a_mq->msgs.x);

	if (a_mq->is_malloced)
		mem_free(a_mq->mem, a_mq);
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
		cw_uint8_t	*one;
		cw_uint16_t	*two;
		cw_uint32_t	*four;
		cw_uint64_t	*eight;
		void		*x;	/* Don't care. */
	}		r_msg;
	va_list		ap;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	va_start(ap, a_mq);
	r_msg.x = (void *)va_arg(ap, void *);
	va_end(ap);

	mtx_lock(&a_mq->lock);

	if (a_mq->get_stop) {
		retval = TRUE;
		goto RETURN;
	}
	if (a_mq->msg_count > 0) {
		switch (a_mq->msg_size) {
		case 1:
			*r_msg.one = a_mq->msgs.one[a_mq->msgs_beg];
			break;
		case 2:
			*r_msg.two = a_mq->msgs.two[a_mq->msgs_beg];
			break;
		case 4:
			*r_msg.four = a_mq->msgs.four[a_mq->msgs_beg];
			break;
		case 8:
			*r_msg.eight = a_mq->msgs.eight[a_mq->msgs_beg];
			break;
		default:
			_cw_not_reached();
		}
		a_mq->msg_count--;
		a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msgs_vec_count;
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
		cw_uint8_t	*one;
		cw_uint16_t	*two;
		cw_uint32_t	*four;
		cw_uint64_t	*eight;
		void		*x;	/* Don't care. */
	}		r_msg;
	va_list		ap;

        _cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);
        _cw_check_ptr(a_timeout);

	va_start(ap, a_timeout);
	r_msg.x = (void *)va_arg(ap, void *);
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
		case 1:
			*r_msg.one = a_mq->msgs.one[a_mq->msgs_beg];
			break;
		case 2:
			*r_msg.two = a_mq->msgs.two[a_mq->msgs_beg];
			break;
		case 4:
			*r_msg.four = a_mq->msgs.four[a_mq->msgs_beg];
			break;
		case 8:
			*r_msg.eight = a_mq->msgs.eight[a_mq->msgs_beg];
			break;
		default:
			_cw_not_reached();
		}
		a_mq->msg_count--;
		a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msgs_vec_count;
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
		cw_uint8_t	*one;
		cw_uint16_t	*two;
		cw_uint32_t	*four;
		cw_uint64_t	*eight;
		void		*x;	/* Don't care. */
	}		r_msg;
	va_list		ap;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	va_start(ap, a_mq);
	r_msg.x = (void *)va_arg(ap, void *);
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
	case 1:
		*r_msg.one = a_mq->msgs.one[a_mq->msgs_beg];
		break;
	case 2:
		*r_msg.two = a_mq->msgs.two[a_mq->msgs_beg];
		break;
	case 4:
		*r_msg.four = a_mq->msgs.four[a_mq->msgs_beg];
		break;
	case 8:
		*r_msg.eight = a_mq->msgs.eight[a_mq->msgs_beg];
		break;
	default:
		_cw_not_reached();
	}
	a_mq->msg_count--;
	a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msgs_vec_count;

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_bool_t
mq_put(cw_mq_t *a_mq, ...)
{
	cw_bool_t	retval;
	union {
		cw_uint32_t	*four;
		cw_uint64_t	*eight;
		void		*x;	/* Don't care. */
	}		a_msg;
	va_list		ap;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	va_start(ap, a_mq);
	a_msg.x = (void *)&va_arg(ap, void *);
	va_end(ap);

	mtx_lock(&a_mq->lock);

	if (a_mq->msg_count == 0)
		cnd_broadcast(&a_mq->cond);
	if (a_mq->put_stop) {
		retval = TRUE;
		goto RETURN;
	} else {
		if (a_mq->msg_count >= a_mq->msgs_vec_count) {
			union {
				cw_uint8_t	*one;
				cw_uint16_t	*two;
				cw_uint32_t	*four;
				cw_uint64_t	*eight;
				void		*x;	/* Don't care. */
			}		t_msgs;
			cw_uint32_t	i, offset;
			
			/*
			 * Array overflow.  Double the array and copy the
			 * messages.
			 */
			t_msgs.x = (void *)mem_malloc(a_mq->mem,
			    a_mq->msgs_vec_count * 2 * a_mq->msg_size);

			switch (a_mq->msg_size) {
			case 1:
				for (i = 0, offset = a_mq->msgs_beg; i <
				    a_mq->msg_count; i++, offset = (offset
				    + 1) % a_mq->msgs_vec_count) {
					t_msgs.one[i] =
					    a_mq->msgs.one[offset];
				}
				break;
			case 2:
				for (i = 0, offset = a_mq->msgs_beg; i <
				    a_mq->msg_count; i++, offset = (offset
				    + 1) % a_mq->msgs_vec_count) {
					t_msgs.two[i] =
					    a_mq->msgs.two[offset];
				}
				break;
			case 4:
				for (i = 0, offset = a_mq->msgs_beg; i <
				    a_mq->msg_count; i++, offset = (offset
				    + 1) % a_mq->msgs_vec_count) {
					t_msgs.four[i] =
					    a_mq->msgs.four[offset];
				}
				break;
			case 8:
				for (i = 0, offset = a_mq->msgs_beg; i <
				    a_mq->msg_count; i++, offset = (offset
				    + 1) % a_mq->msgs_vec_count) {
					t_msgs.eight[i] =
					    a_mq->msgs.eight[offset];
				}
				break;
			default:
				_cw_not_reached();
			}
			_cw_free(a_mq->msgs.x);
			a_mq->msgs.x = t_msgs.x;
			a_mq->msgs_beg = 0;
			a_mq->msgs_end = a_mq->msg_count;
			a_mq->msgs_vec_count *= 2;
		}

		switch (a_mq->msg_size) {
		case 1:
			/* The compiler promotes 8 bit args to 32 bits. */
			a_mq->msgs.one[a_mq->msgs_end] = *a_msg.four;
			break;
		case 2:
			/* The compiler promotes 16 bit args to 32 bits. */
			a_mq->msgs.two[a_mq->msgs_end] = *a_msg.four;
			break;
		case 4:
			a_mq->msgs.four[a_mq->msgs_end] = *a_msg.four;
			break;
		case 8:
			a_mq->msgs.eight[a_mq->msgs_end] = *a_msg.eight;
			break;
		default:
			_cw_not_reached();
		}
		a_mq->msg_count++;
		a_mq->msgs_end = (a_mq->msgs_end + 1) % a_mq->msgs_vec_count;
	}

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_mq->lock);
	return retval;
}

cw_bool_t
mq_get_start(cw_mq_t *a_mq)
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
mq_get_stop(cw_mq_t *a_mq)
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
mq_put_start(cw_mq_t *a_mq)
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
mq_put_stop(cw_mq_t *a_mq)
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

void
mq_dump(cw_mq_t *a_mq, const char *a_prefix)
{
	cw_uint32_t	i, offset;

	_cw_check_ptr(a_mq);
	_cw_assert(a_mq->magic == _LIBSTASH_MQ_MAGIC);

	_cw_out_put("[s]is_malloced : [s]\n", a_prefix, a_mq->is_malloced ?
	    "TRUE" : "FALSE");
	_cw_out_put("[s]msg_count : [i]\n", a_prefix, a_mq->msg_count);
	_cw_out_put("[s]msg_size : [i]\n", a_prefix, a_mq->msg_size);
	_cw_out_put("[s]msg_msgs_vec_count : [i]\n", a_prefix,
	    a_mq->msgs_vec_count);
	_cw_out_put("[s]msg_beg : [i]\n", a_prefix, a_mq->msgs_beg);
	_cw_out_put("[s]msg_end : [i]\n", a_prefix, a_mq->msgs_end);

	switch (a_mq->msg_size) {
	case 1:
		for (i = 0, offset = a_mq->msgs_beg; i < a_mq->msg_count; i++,
			 offset = (offset + 1) % a_mq->msgs_vec_count) {
			_cw_out_put("[s]  msgs[[[i]]([i]) : [i] (0x[i|b:16])\n",
			    a_prefix, i, offset, a_mq->msgs.one[i],
			    a_mq->msgs.one[i]);
		}
		break;
	case 2:
		for (i = 0, offset = a_mq->msgs_beg; i < a_mq->msg_count; i++,
			 offset = (offset + 1) % a_mq->msgs_vec_count) {
			_cw_out_put("[s]  msgs[[[i]]([i]) : [i] (0x[i|b:16])\n",
			    a_prefix, i, offset, a_mq->msgs.two[i],
			    a_mq->msgs.two[i]);
		}
		break;
	case 4:
		for (i = 0, offset = a_mq->msgs_beg; i < a_mq->msg_count; i++,
			 offset = (offset + 1) % a_mq->msgs_vec_count) {
			_cw_out_put("[s]  msgs[[[i]]([i]) : [i] (0x[i|b:16])\n",
			    a_prefix, i, offset, a_mq->msgs.four[i],
			    a_mq->msgs.four[i]);
		}
		break;
	case 8:
		for (i = 0, offset = a_mq->msgs_beg; i < a_mq->msg_count; i++,
			 offset = (offset + 1) % a_mq->msgs_vec_count) {
			_cw_out_put("[s]  msgs[[[i]]([i]) : [i] (0x[i|b:16])\n",
			    a_prefix, offset, i, a_mq->msgs.eight[i],
			    a_mq->msgs.eight[i]);
		}
		break;
	default:
		_cw_not_reached();
	}

	_cw_out_put("[s]get_stop : [s]\n", a_prefix, a_mq->get_stop ? "TRUE" :
	    "FALSE");
	_cw_out_put("[s]put_stop : [s]\n", a_prefix, a_mq->put_stop ? "TRUE" :
	    "FALSE");
}
