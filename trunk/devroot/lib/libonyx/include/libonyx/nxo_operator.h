/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

void	nxo_operator_new(cw_nxo_t *a_nxo, cw_op_t *a_op, cw_nxn_t a_nxn);

#ifndef _CW_USE_INLINES
#endif

#if (defined(_CW_USE_INLINES) || defined(_NXO_OPERATOR_C_))
_CW_INLINE cw_op_t *
nxo_operator_f(cw_nxo_t *a_nxo)
{
	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_OPERATOR);

	return a_nxo->o.operator.f;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXO_OPERATOR_C_)) */
