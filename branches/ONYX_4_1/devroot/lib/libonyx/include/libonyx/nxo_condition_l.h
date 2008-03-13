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

typedef struct cw_nxoe_condition_s cw_nxoe_condition_t;

struct cw_nxoe_condition_s
{
    cw_nxoe_t nxoe;
    cw_cnd_t condition;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_condition_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_condition_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_CONDITION_C_))
CW_INLINE cw_bool_t
nxoe_l_condition_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_condition_t *condition;

    condition = (cw_nxoe_condition_t *) a_nxoe;

    cw_check_ptr(condition);
    cw_dassert(condition->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(condition->nxoe.type == NXOT_CONDITION);

    cnd_delete(&condition->condition);

    nxa_free(a_nxa, condition, sizeof(cw_nxoe_condition_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_condition_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_CONDITION_C_)) */
