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
void	nxo_fino_new(cw_nxo_t *a_nxo);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_FINO_C_))
CW_INLINE void
nxo_fino_new(cw_nxo_t *a_nxo)
{
	cw_check_ptr(a_nxo);

	nxo_p_new(a_nxo, NXOT_FINO);
}
#endif	/* (defined(CW_USE_INLINES) || defined(CW_NXO_FINO_C_)) */
