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

#ifdef CW_THREADS
/* Enumeration of message types for the GC thread event loop. */
typedef enum
{
    NXAM_NONE,
    NXAM_COLLECT,
    NXAM_RECONFIGURE,
    NXAM_SHUTDOWN
} cw_nxam_t;
#endif

/* Global variables. */
#ifdef CW_DBG
extern cw_bool_t cw_g_nxa_initialized;
#endif
#ifdef CW_THREADS
extern cw_mtx_t cw_g_nxa_name_lock;
#endif
extern cw_dch_t cw_g_nxa_name_hash;

void
nxa_l_nx_insert(cw_nx_t *a_nx);

void
nxa_l_nx_remove(cw_nx_t *a_nx);

void
nxa_l_gc_register(cw_nxoe_t *a_nxoe);

void
nxa_l_gc_reregister(cw_nxoe_t *a_nxoe);

void
nxa_l_count_adjust(cw_nxoi_t a_adjust);

cw_bool_t
nxa_l_white_get(void);

#ifndef CW_USE_INLINES
#ifdef CW_THREADS
cw_mtx_t *
nxa_l_name_lock_get(void);
#endif

cw_dch_t *
nxa_l_name_hash_get(void);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXA_C_))
#ifdef CW_THREADS
CW_INLINE cw_mtx_t *
nxa_l_name_lock_get(void)
{
    cw_assert(cw_g_nxa_initialized);

    return &cw_g_nxa_name_lock;
}
#endif

CW_INLINE cw_dch_t *
nxa_l_name_hash_get(void)
{
    cw_assert(cw_g_nxa_initialized);

    return &cw_g_nxa_name_hash;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXA_C_)) */
