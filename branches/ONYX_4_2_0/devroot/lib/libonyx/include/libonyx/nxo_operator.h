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

void
nxo_operator_new(cw_nxo_t *a_nxo, cw_op_t *a_op, cw_nxn_t a_nxn);

#ifndef CW_USE_INLINES
cw_op_t *
nxo_operator_f(const cw_nxo_t *a_nxo);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_OPERATOR_C_))
CW_INLINE cw_op_t *
nxo_operator_f(const cw_nxo_t *a_nxo)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_OPERATOR);

    return a_nxo->o.operator.f;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_OPERATOR_C_)) */
