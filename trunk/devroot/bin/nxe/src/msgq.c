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

#include "../include/nxe.h"

/* msgq. */
void
msgq_new(cw_msgq_t *a_msgq, cw_opaque_dealloc_t *a_dealloc, void *a_arg)
{
	_cw_error("XXX Not implemented");
}

void
msgq_delete(cw_msgq_t *a_msgq)
{
	_cw_error("XXX Not implemented");
}

cw_msg_t *
msgq_get(cw_msgq_t *a_msgq)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_msg_t *
msgq_tryget(cw_msgq_t *a_msgq)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

cw_msg_t *
msgq_timedget(cw_msgq_t *a_msgq, const struct timespec *a_timeout)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}

void
msgq_put(cw_msgq_t *a_msgq, cw_msg_t *a_msg)
{
	_cw_error("XXX Not implemented");
}

/* msg. */
void
msg_new(cw_msg_t *a_msg, cw_opaque_dealloc_t *a_dealloc, void *a_dealloc_arg,
    cw_uint32_t a_type, void *a_data)
{
	_cw_error("XXX Not implemented");
}

void
msg_delete(cw_msg_t *a_msg)
{
	_cw_error("XXX Not implemented");
}

cw_uint32_t
msg_type(cw_msg_t *a_msg)
{
	_cw_error("XXX Not implemented");
	return 0; /* XXX */
}

void *
msg_data(cw_msg_t *a_msg)
{
	_cw_error("XXX Not implemented");
	return NULL; /* XXX */
}
