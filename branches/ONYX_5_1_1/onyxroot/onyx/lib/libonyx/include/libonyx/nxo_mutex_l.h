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

typedef struct cw_nxoe_mutex_s cw_nxoe_mutex_t;

struct cw_nxoe_mutex_s
{
    cw_nxoe_t nxoe;
    cw_mtx_t lock;
};

#ifndef CW_USE_INLINES
bool
nxoe_l_mutex_delete(cw_nxoe_t *a_nxoe, uint32_t a_iter);

cw_nxoe_t *
nxoe_l_mutex_ref_iter(cw_nxoe_t *a_nxoe, bool a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_MUTEX_C_))
CW_INLINE bool
nxoe_l_mutex_delete(cw_nxoe_t *a_nxoe, uint32_t a_iter)
{
    cw_nxoe_mutex_t *mutex;

    mutex = (cw_nxoe_mutex_t *) a_nxoe;

    cw_check_ptr(mutex);
    cw_dassert(mutex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(mutex->nxoe.type == NXOT_MUTEX);

    mtx_delete(&mutex->lock);

    nxa_free(mutex, sizeof(cw_nxoe_mutex_t));

    return false;
}

CW_INLINE cw_nxoe_t *
nxoe_l_mutex_ref_iter(cw_nxoe_t *a_nxo, bool a_reset)
{
    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_MUTEX_C_)) */
