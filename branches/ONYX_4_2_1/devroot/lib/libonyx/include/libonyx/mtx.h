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
typedef struct cw_mtx_s cw_mtx_t;

struct cw_mtx_s
{
#ifdef CW_PTH
    pth_mutex_t mutex;
#endif
#ifdef CW_PTHREADS
    pthread_mutex_t mutex;
#endif
};

void
mtx_new(cw_mtx_t *a_mtx);

void
mtx_delete(cw_mtx_t *a_mtx);

void
mtx_lock(cw_mtx_t *a_mtx);

cw_bool_t
mtx_trylock(cw_mtx_t *a_mtx);

void
mtx_unlock(cw_mtx_t *a_mtx);
