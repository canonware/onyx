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

#ifndef _CW_USE_INLINES
void	nxo_boolean_new(cw_nxo_t *a_nxo, cw_bool_t a_val);
cw_bool_t nxo_boolean_get(cw_nxo_t *a_nxo);
void	nxo_boolean_set(cw_nxo_t *a_nxo, cw_bool_t a_val);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NXO_BOOLEAN_C_))
_CW_INLINE void
nxo_boolean_new(cw_nxo_t *a_nxo, cw_bool_t a_val)
{
	_cw_check_ptr(a_nxo);

	nxo_p_new(a_nxo, NXOT_BOOLEAN);
	a_nxo->o.boolean.val = a_val;
}

_CW_INLINE cw_bool_t
nxo_boolean_get(cw_nxo_t *a_nxo)
{
	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_BOOLEAN);

	return a_nxo->o.boolean.val;
}

_CW_INLINE void
nxo_boolean_set(cw_nxo_t *a_nxo, cw_bool_t a_val)
{
	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_BOOLEAN);

	a_nxo->o.boolean.val = a_val;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXO_BOOLEAN_C_)) */
