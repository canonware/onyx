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
typedef struct cw_cnd_s cw_cnd_t;

struct cw_cnd_s
{
    pthread_cond_t condition;
};

void
cnd_new(cw_cnd_t *a_cnd);

void
cnd_delete(cw_cnd_t *a_cnd);

void
cnd_signal(cw_cnd_t *a_cnd);

void
cnd_broadcast(cw_cnd_t *a_cnd);

cw_bool_t
cnd_timedwait(cw_cnd_t *a_cnd, cw_mtx_t *a_mtx,
	      const struct timespec *a_timeout);

void
cnd_wait(cw_cnd_t *a_cnd, cw_mtx_t *a_mtx);
