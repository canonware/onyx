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
    cw_mema_t *mema;
#ifdef CW_DBG
    uint32_t magic;
#endif
    uint32_t msg_count;
    uint32_t msg_size;
    uint32_t msgs_vec_count;
    uint32_t msgs_beg;
    uint32_t msgs_end;
    union
    {
	uint8_t *one;
	uint16_t *two;
	uint32_t *four;
	uint64_t *eight;
	void *x; /* Don't care. */
    } msgs;

    cw_mtx_t lock;
    cw_cnd_t cond;

    bool get_stop;
    bool put_stop;
};

void
mq_new(cw_mq_t *a_mq, cw_mema_t *a_mema, uint32_t a_msg_size);

void
mq_delete(cw_mq_t *a_mq);

bool
mq_tryget(cw_mq_t *a_mq, ...);

bool
mq_timedget(cw_mq_t *a_mq, const struct timespec *a_timeout, ...);

bool
mq_get(cw_mq_t *a_mq, ...);

bool
mq_put(cw_mq_t *a_mq, ...);

bool
mq_get_start(cw_mq_t *a_mq);

bool
mq_get_stop(cw_mq_t *a_mq);

bool
mq_put_start(cw_mq_t *a_mq);

bool
mq_put_stop(cw_mq_t *a_mq);
