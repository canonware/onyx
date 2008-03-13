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
extern bool cw_g_nxa_initialized;
#endif

void
nxa_l_nx_insert(cw_nx_t *a_nx);

void
nxa_l_nx_remove(cw_nx_t *a_nx);

void
nxa_l_gc_register(cw_nxoe_t *a_nxoe);

void
nxa_l_count_adjust(cw_nxoi_t a_adjust);
