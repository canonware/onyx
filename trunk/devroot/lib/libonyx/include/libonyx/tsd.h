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
typedef struct cw_tsd_s cw_tsd_t;

struct cw_tsd_s
{
#ifdef CW_PTH
    pth_key_t key;
#endif
#ifdef CW_PTHREADS
    pthread_key_t key;
#endif
};

void
tsd_new(cw_tsd_t *a_tsd, void (*a_func)(void *));

void
tsd_delete(cw_tsd_t *a_tsd);

void *
tsd_get(cw_tsd_t *a_tsd);

void
tsd_set(cw_tsd_t *a_tsd, void *a_val);
