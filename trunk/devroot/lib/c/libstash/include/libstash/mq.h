/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_mq_s cw_mq_t;

struct cw_mq_s
{
  cw_bool_t is_malloced;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
#ifdef _CW_REENTRANT
  cw_mtx_t lock;
  cw_cnd_t cond;
#endif

  cw_bool_t get_stop;
  cw_bool_t put_stop;
  
  cw_ring_t * ring;
  cw_ring_t * spares_ring;
};

cw_mq_t *
mq_new(cw_mq_t * a_mq);

void
mq_delete(cw_mq_t * a_mq);

void *
mq_tryget(cw_mq_t * a_mq);

#ifdef _CW_REENTRANT
void *
mq_timedget(cw_mq_t * a_mq, const struct timespec * a_timeout);

void *
mq_get(cw_mq_t * a_mq);
#endif

cw_sint32_t
mq_put(cw_mq_t * a_mq, const void * a_message);

cw_bool_t
mq_start_get(cw_mq_t * a_mq);

cw_bool_t
mq_stop_get(cw_mq_t * a_mq);

cw_bool_t
mq_start_put(cw_mq_t * a_mq);

cw_bool_t
mq_stop_put(cw_mq_t * a_mq);
