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
void	nxo_pmark_new(cw_nxo_t *a_nxo);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NXO_PMARK_C_))
_CW_INLINE void
nxo_pmark_new(cw_nxo_t *a_nxo)
{
	_cw_check_ptr(a_nxo);

	nxo_p_new(a_nxo, NXOT_PMARK);
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXO_PMARK_C_)) */
