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

#ifndef CW_THREADS
#endif

void
nxa_l_new(cw_nxa_t *a_nxa, cw_nx_t *a_nx);

void
nxa_l_shutdown(cw_nxa_t *a_nxa);

void
nxa_l_delete(cw_nxa_t *a_nxa);

void
nxa_l_gc_register(cw_nxa_t *a_nxa, cw_nxoe_t *a_nxoe);

void
nxa_l_gc_reregister(cw_nxa_t *a_nxa, cw_nxoe_t *a_nxoe);

cw_bool_t
nxa_l_white_get(cw_nxa_t *a_nxa);
