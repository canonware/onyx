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
extern cw_nxa_t *cw_g_nxa;

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
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

    return &cw_g_nxa->name_lock;
}
#endif

CW_INLINE cw_dch_t *
nxa_l_name_hash_get(void)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

    return &cw_g_nxa->name_hash;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXA_C_)) */
