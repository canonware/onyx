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

typedef struct cw_msgq_s cw_msgq_t;
typedef struct cw_msg_s cw_msg_t;

struct cw_msg_s {
	cw_opaque_dealloc_t *dealloc;
	const void	*arg;

	/*
	 * This type is used to determine the type of the data pointer in
	 * cw_msg_t.  It is up to the code receiving the message to act upon the
	 * type and cast to the appropriate type.
	 */
	cw_uint32_t	type;
	/* Message data. */
	void		*data;
};

struct cw_msgq_s {
	cw_opaque_dealloc_t *dealloc;
	const void	*arg;

	cw_mq_t		mq;
};

/* msgq. */
void	msgq_new(cw_msgq_t *a_msgq, cw_opaque_dealloc_t *a_dealloc, void
    *a_arg);
void	msgq_delete(cw_msgq_t *a_msgq);

cw_msg_t *msgq_get(cw_msgq_t *a_msgq);
cw_msg_t *msgq_tryget(cw_msgq_t *a_msgq);
cw_msg_t *msgq_timedget(cw_msgq_t *a_msgq, const struct timespec *a_timeout);
void	msgq_put(cw_msgq_t *a_msgq, cw_msg_t *a_msg);

/* msg. */
void	msg_new(cw_msg_t * a_msg, cw_opaque_dealloc_t *a_dealloc, void *a_arg,
    cw_uint32_t a_type, void *a_data);
void	msg_delete(cw_msg_t *a_msg);

cw_uint32_t msg_type(cw_msg_t *a_msg);
void	*msg_data(cw_msg_t *a_msg);
