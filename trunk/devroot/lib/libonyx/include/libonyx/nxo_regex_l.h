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

typedef struct cw_nxoe_regex_s cw_nxoe_regex_t;

struct cw_nxoe_regex_s
{
    cw_nxoe_t nxoe;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_regex_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_regex_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_REGEX_C_))
CW_INLINE cw_bool_t
nxoe_l_regex_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_regex_t *regex;

    regex = (cw_nxoe_regex_t *) a_nxoe;

    cw_check_ptr(regex);
    cw_dassert(regex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(regex->nxoe.type == NXOT_REGEX);

    nxa_free(a_nxa, regex, sizeof(cw_nxoe_regex_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_regex_ref_iter(cw_nxoe_t *a_nxo, cw_bool_t a_reset)
{
    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_REGEX_C_)) */
