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

#ifndef CW_USE_INLINES
void	nxo_integer_new(cw_nxo_t *a_nxo, cw_nxoi_t a_val);
cw_nxoi_t nxo_integer_get(cw_nxo_t *a_nxo);
void	nxo_integer_set(cw_nxo_t *a_nxo, cw_nxoi_t a_val);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_INTEGER_C_))
CW_INLINE void
nxo_integer_new(cw_nxo_t *a_nxo, cw_nxoi_t a_val)
{
	cw_check_ptr(a_nxo);

	nxo_p_new(a_nxo, NXOT_INTEGER);
	a_nxo->o.integer.i = a_val;
}

CW_INLINE cw_nxoi_t
nxo_integer_get(cw_nxo_t *a_nxo)
{
	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_INTEGER);

	return a_nxo->o.integer.i;
}

CW_INLINE void
nxo_integer_set(cw_nxo_t *a_nxo, cw_nxoi_t a_val)
{
	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_INTEGER);

	a_nxo->o.integer.i = a_val;
}
#endif	/* (defined(CW_USE_INLINES) || defined(CW_NXO_INTEGER_C_)) */
