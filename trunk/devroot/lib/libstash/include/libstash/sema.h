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
typedef struct cw_sem_s cw_sem_t;

struct cw_sem_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_cnd_t gtzero;
  cw_sint32_t count;
  cw_uint32_t waiters;
};

cw_sem_t *
sem_new(cw_sem_t * a_sem, cw_sint32_t a_count);

void
sem_delete(cw_sem_t * a_sem);

void
sem_post(cw_sem_t * a_sem);

void
sem_wait(cw_sem_t * a_sem);

cw_bool_t
sem_timedwait(cw_sem_t * a_sem, struct timespec * a_timeout);

cw_bool_t
sem_trywait(cw_sem_t * a_sem);

cw_sint32_t
sem_getvalue(cw_sem_t * a_sem);

void
sem_adjust(cw_sem_t * a_sem, cw_sint32_t a_adjust);
