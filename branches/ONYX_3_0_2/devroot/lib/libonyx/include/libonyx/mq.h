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

/* Pseudo-opaque type. */
typedef struct cw_mq_s cw_mq_t;

struct cw_mq_s
{
    cw_mem_t *mem;
#ifdef CW_DBG
    cw_uint32_t magic;
#endif
    cw_uint32_t msg_count;
    cw_uint32_t msg_size;
    cw_uint32_t msgs_vec_count;
    cw_uint32_t msgs_beg;
    cw_uint32_t msgs_end;
    union
    {
	cw_uint8_t *one;
	cw_uint16_t *two;
	cw_uint32_t *four;
	cw_uint64_t *eight;
	void *x; /* Don't care. */
    } msgs;

    cw_mtx_t lock;
    cw_cnd_t cond;

    cw_bool_t get_stop;
    cw_bool_t put_stop;
};

void
mq_new(cw_mq_t *a_mq, cw_mem_t *a_mem, cw_uint32_t a_msg_size);

void
mq_delete(cw_mq_t *a_mq);

cw_bool_t
mq_tryget(cw_mq_t *a_mq, ...);

cw_bool_t
mq_timedget(cw_mq_t *a_mq, const struct timespec *a_timeout, ...);

cw_bool_t
mq_get(cw_mq_t *a_mq, ...);

cw_bool_t
mq_put(cw_mq_t *a_mq, ...);

cw_bool_t
mq_get_start(cw_mq_t *a_mq);

cw_bool_t
mq_get_stop(cw_mq_t *a_mq);

cw_bool_t
mq_put_start(cw_mq_t *a_mq);

cw_bool_t
mq_put_stop(cw_mq_t *a_mq);
