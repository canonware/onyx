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

#include <pcre.h>

typedef struct cw_nxoe_regsub_s cw_nxoe_regsub_t;

struct cw_nxoe_regsub_s
{
    cw_nxoe_t nxoe;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_regsub_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_regsub_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_REGSUB_C_))
CW_INLINE cw_bool_t
nxoe_l_regsub_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_regsub_t *regsub;

    regsub = (cw_nxoe_regsub_t *) a_nxoe;

    cw_check_ptr(regsub);
    cw_dassert(regsub->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(regsub->nxoe.type == NXOT_REGSUB);

    nxa_free(a_nxa, regsub, sizeof(cw_nxoe_regsub_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_regsub_ref_iter(cw_nxoe_t *a_nxo, cw_bool_t a_reset)
{
    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_REGSUB_C_)) */
