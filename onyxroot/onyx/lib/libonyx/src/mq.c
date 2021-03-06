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

#include <stdarg.h>

#ifdef CW_DBG
#define CW_MQ_MAGIC 0xab01cd23
#endif

/* Number of messages to allocate space for when a message queue is first
 * created.  Since message space is doubled when more space is needed, this
 * number isn't very important to performance, but if it is too high, space may
 * be wasted. */
#ifdef CW_DBG
#define CW_MQ_ARRAY_MIN_SIZE 1
#else
#define CW_MQ_ARRAY_MIN_SIZE 8
#endif

void
mq_new(cw_mq_t *a_mq, cw_mema_t *a_mema, uint32_t a_msg_size)
{
    cw_check_ptr(a_mq);

    a_mq->mema = a_mema;
    a_mq->msg_count = 0;

    switch (a_msg_size) {
	case 1:
	{
	    a_mq->msg_size = 1;
	    break;
	}
	case 2:
	{
	    a_mq->msg_size = 2;
	    break;
	}
	case 4:
	{
	    a_mq->msg_size = 4;
	    break;
	}
	case 8:
	{
	    a_mq->msg_size = 8;
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    a_mq->msgs_vec_count = CW_MQ_ARRAY_MIN_SIZE;
    a_mq->msgs_beg = 0;
    a_mq->msgs_end = 0;

    a_mq->msgs.x = (uint32_t *) cw_opaque_alloc(mema_alloc_get(a_mema),
						   mema_arg_get(a_mema),
						   a_mq->msgs_vec_count
						   * a_mq->msg_size);

    mtx_new(&a_mq->lock);
    cnd_new(&a_mq->cond);

    a_mq->get_stop = false;
    a_mq->put_stop = false;

#ifdef CW_DBG
    a_mq->magic = CW_MQ_MAGIC;
#endif
}

void
mq_delete(cw_mq_t *a_mq)
{
    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);

    mtx_delete(&a_mq->lock);
    cnd_delete(&a_mq->cond);

    cw_opaque_dealloc(mema_dealloc_get(a_mq->mema),
		      mema_arg_get(a_mq->mema),
		      a_mq->msgs.x,
		      a_mq->msgs_vec_count * a_mq->msg_size);

#ifdef CW_DBG
    memset(a_mq, 0x5a, sizeof(cw_mq_t));
#endif
}

bool
mq_tryget(cw_mq_t *a_mq, ...)
{
    bool retval;
    union
    {
	uint8_t *one;
	uint16_t *two;
	uint32_t *four;
	uint64_t *eight;
	void *x; /* Don't care. */
    } r_msg;
    va_list ap;

    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);

    va_start(ap, a_mq);
    r_msg.x = (void *) va_arg(ap, void *);
    va_end(ap);

    mtx_lock(&a_mq->lock);

    if (a_mq->get_stop)
    {
	retval = true;
	goto RETURN;
    }
    if (a_mq->msg_count > 0)
    {
	switch (a_mq->msg_size)
	{
	    case 1:
	    {
		*r_msg.one = a_mq->msgs.one[a_mq->msgs_beg];
		break;
	    }
	    case 2:
	    {
		*r_msg.two = a_mq->msgs.two[a_mq->msgs_beg];
		break;
	    }
	    case 4:
	    {
		*r_msg.four = a_mq->msgs.four[a_mq->msgs_beg];
		break;
	    }
	    case 8:
	    {
		*r_msg.eight = a_mq->msgs.eight[a_mq->msgs_beg];
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
	a_mq->msg_count--;
	a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msgs_vec_count;
    }
    else
    {
	retval = true;
	goto RETURN;
    }

    retval = false;
    RETURN:
    mtx_unlock(&a_mq->lock);
    return retval;
}

bool
mq_timedget(cw_mq_t *a_mq, const struct timespec *a_timeout, ...)
{
    bool retval, timed_out;
    union
    {
	uint8_t *one;
	uint16_t *two;
	uint32_t *four;
	uint64_t *eight;
	void *x; /* Don't care. */
    } r_msg;
    va_list ap;

    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);
    cw_check_ptr(a_timeout);

    va_start(ap, a_timeout);
    r_msg.x = (void *) va_arg(ap, void *);
    va_end(ap);

    mtx_lock(&a_mq->lock);

    if (a_mq->get_stop)
    {
	retval = true;
	goto RETURN;
    }
    /* A spurious wakeup will cause the timeout interval to start over.  This
     * isn't a big deal as long as spurious wakeups don't occur continuously,
     * since the timeout period is merely a lower bound on how long to wait. */
    for (timed_out = false; a_mq->msg_count == 0 && timed_out == false;)
    {
	timed_out = cnd_timedwait(&a_mq->cond, &a_mq->lock, a_timeout);
	if (a_mq->get_stop)
	{
	    retval = true;
	    goto RETURN;
	}
    }
    if (a_mq->msg_count > 0)
    {
	switch (a_mq->msg_size)
	{
	    case 1:
	    {
		*r_msg.one = a_mq->msgs.one[a_mq->msgs_beg];
		break;
	    }
	    case 2:
	    {
		*r_msg.two = a_mq->msgs.two[a_mq->msgs_beg];
		break;
	    }
	    case 4:
	    {
		*r_msg.four = a_mq->msgs.four[a_mq->msgs_beg];
		break;
	    }
	    case 8:
	    {
		*r_msg.eight = a_mq->msgs.eight[a_mq->msgs_beg];
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
	a_mq->msg_count--;
	a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msgs_vec_count;
    }
    else
    {
	retval = true;
	goto RETURN;
    }

    retval = false;
    RETURN:
    mtx_unlock(&a_mq->lock);
    return retval;
}

bool
mq_get(cw_mq_t *a_mq, ...)
{
    bool retval;
    union
    {
	uint8_t *one;
	uint16_t *two;
	uint32_t *four;
	uint64_t *eight;
	void *x; /* Don't care. */
    } r_msg;
    va_list ap;

    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);

    va_start(ap, a_mq);
    r_msg.x = (void *) va_arg(ap, void *);
    va_end(ap);

    mtx_lock(&a_mq->lock);

    if (a_mq->get_stop)
    {
	retval = true;
	goto RETURN;
    }
    while (a_mq->msg_count == 0)
    {
	cnd_wait(&a_mq->cond, &a_mq->lock);
	if (a_mq->get_stop)
	{
	    retval = true;
	    goto RETURN;
	}
    }

    switch (a_mq->msg_size)
    {
	case 1:
	{
	    *r_msg.one = a_mq->msgs.one[a_mq->msgs_beg];
	    break;
	}
	case 2:
	{
	    *r_msg.two = a_mq->msgs.two[a_mq->msgs_beg];
	    break;
	}
	case 4:
	{
	    *r_msg.four = a_mq->msgs.four[a_mq->msgs_beg];
	    break;
	}
	case 8:
	{
	    *r_msg.eight = a_mq->msgs.eight[a_mq->msgs_beg];
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
    a_mq->msg_count--;
    a_mq->msgs_beg = (a_mq->msgs_beg + 1) % a_mq->msgs_vec_count;

    retval = false;
    RETURN:
    mtx_unlock(&a_mq->lock);
    return retval;
}

bool
mq_put(cw_mq_t *a_mq, ...)
{
    bool retval;
    union
    {
	uint32_t four;
	uint64_t eight;
    } a_msg;
    va_list ap;

    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);

    va_start(ap, a_mq);
    switch (a_mq->msg_size)
    {
	case 1: /* Compiler-promoted to 32 bits. */
	case 2: /* Compiler-promoted to 32 bits. */
	case 4:
	{
	    a_msg.four = va_arg(ap, uint32_t);
	    break;
	}
	case 8:
	{
	    a_msg.eight = va_arg(ap, uint64_t);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
    va_end(ap);

    mtx_lock(&a_mq->lock);

    if (a_mq->msg_count == 0)
    {
	cnd_broadcast(&a_mq->cond);
    }
    if (a_mq->put_stop)
    {
	retval = true;
	goto RETURN;
    }
    else
    {
	if (a_mq->msg_count >= a_mq->msgs_vec_count)
	{
	    union
	    {
		uint8_t *one;
		uint16_t *two;
		uint32_t *four;
		uint64_t *eight;
		void *x; /* Don't care. */
	    } t_msgs;
	    uint32_t i, offset;

	    /* Array overflow.  Double the array and copy the messages. */
	    t_msgs.x = (void *) cw_opaque_alloc(mema_alloc_get(a_mq->mema),
						mema_arg_get(a_mq->mema),
						a_mq->msgs_vec_count * 2
						* a_mq->msg_size);

	    switch (a_mq->msg_size)
	    {
		case 1:
		{
		    for (i = 0, offset = a_mq->msgs_beg;
			 i < a_mq->msg_count;
			 i++, offset = (offset + 1) % a_mq->msgs_vec_count)
		    {
			t_msgs.one[i] = a_mq->msgs.one[offset];
		    }
		    break;
		}
		case 2:
		{
		    for (i = 0, offset = a_mq->msgs_beg;
			 i < a_mq->msg_count;
			 i++, offset = (offset + 1) % a_mq->msgs_vec_count)
		    {
			t_msgs.two[i] = a_mq->msgs.two[offset];
		    }
		    break;
		}
		case 4:
		{
		    for (i = 0, offset = a_mq->msgs_beg;
			 i < a_mq->msg_count;
			 i++, offset = (offset + 1) % a_mq->msgs_vec_count)
		    {
			t_msgs.four[i] = a_mq->msgs.four[offset];
		    }
		    break;
		}
		case 8:
		{
		    for (i = 0, offset = a_mq->msgs_beg;
			 i < a_mq->msg_count;
			 i++, offset = (offset + 1) % a_mq->msgs_vec_count)
		    {
			t_msgs.eight[i] = a_mq->msgs.eight[offset];
		    }
		    break;
		}
		default:
		{
		    cw_not_reached();
		}
	    }
	    cw_opaque_dealloc(mema_dealloc_get(a_mq->mema),
			      mema_arg_get(a_mq->mema),
			      a_mq->msgs.x,
			      a_mq->msgs_vec_count * a_mq->msg_size);
	    a_mq->msgs.x = t_msgs.x;
	    a_mq->msgs_beg = 0;
	    a_mq->msgs_end = a_mq->msg_count;
	    a_mq->msgs_vec_count *= 2;
	}

	switch (a_mq->msg_size)
	{
	    case 1:
	    {
		/* The compiler promotes 8 bit args to 32 bits. */
		a_mq->msgs.one[a_mq->msgs_end] = a_msg.four;
		break;
	    }
	    case 2:
	    {
		/* The compiler promotes 16 bit args to 32 bits. */
		a_mq->msgs.two[a_mq->msgs_end] = a_msg.four;
		break;
	    }
	    case 4:
	    {
		a_mq->msgs.four[a_mq->msgs_end] = a_msg.four;
		break;
	    }
	    case 8:
	    {
		a_mq->msgs.eight[a_mq->msgs_end] = a_msg.eight;
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
	a_mq->msg_count++;
	a_mq->msgs_end = (a_mq->msgs_end + 1) % a_mq->msgs_vec_count;
    }

    retval = false;
    RETURN:
    mtx_unlock(&a_mq->lock);
    return retval;
}

bool
mq_get_start(cw_mq_t *a_mq)
{
    bool retval;

    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);
    mtx_lock(&a_mq->lock);

    if (a_mq->get_stop == false)
    {
	retval = true;
	goto RETURN;
    }
    a_mq->get_stop = false;

    retval = false;
    RETURN:
    mtx_unlock(&a_mq->lock);
    return retval;
}

bool
mq_get_stop(cw_mq_t *a_mq)
{
    bool retval;

    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);
    mtx_lock(&a_mq->lock);

    if (a_mq->get_stop)
    {
	retval = true;
	goto RETURN;
    }
    cnd_broadcast(&a_mq->cond);
    a_mq->get_stop = true;

    retval = false;
    RETURN:
    mtx_unlock(&a_mq->lock);
    return retval;
}

bool
mq_put_start(cw_mq_t *a_mq)
{
    bool retval;

    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);
    mtx_lock(&a_mq->lock);

    if (a_mq->put_stop == false)
    {
	retval = true;
	goto RETURN;
    }
    a_mq->put_stop = false;

    retval = false;
    RETURN:
    mtx_unlock(&a_mq->lock);
    return retval;
}

bool
mq_put_stop(cw_mq_t *a_mq)
{
    bool retval;

    cw_check_ptr(a_mq);
    cw_dassert(a_mq->magic == CW_MQ_MAGIC);
    mtx_lock(&a_mq->lock);

    if (a_mq->put_stop)
    {
	retval = true;
	goto RETURN;
    }
    a_mq->put_stop = true;

    retval = false;
    RETURN:
    mtx_unlock(&a_mq->lock);
    return retval;
}
