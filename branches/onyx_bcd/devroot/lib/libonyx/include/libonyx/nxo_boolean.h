/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#ifndef CW_USE_INLINES
void
nxo_boolean_new(cw_nxo_t *a_nxo, cw_bool_t a_val);

cw_bool_t
nxo_boolean_get(cw_nxo_t *a_nxo);

void
nxo_boolean_set(cw_nxo_t *a_nxo, cw_bool_t a_val);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_BOOLEAN_C_))
CW_INLINE void
nxo_boolean_new(cw_nxo_t *a_nxo, cw_bool_t a_val)
{
    cw_check_ptr(a_nxo);

    nxo_p_new(a_nxo, NXOT_BOOLEAN);
    a_nxo->o.boolean.val = a_val;
}

CW_INLINE cw_bool_t
nxo_boolean_get(cw_nxo_t *a_nxo)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_BOOLEAN);

    return a_nxo->o.boolean.val;
}

CW_INLINE void
nxo_boolean_set(cw_nxo_t *a_nxo, cw_bool_t a_val)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_BOOLEAN);

    a_nxo->o.boolean.val = a_val;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_BOOLEAN_C_)) */
