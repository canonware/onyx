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

#ifndef CW_USE_INLINES
void
nxo_mark_new(cw_nxo_t *a_nxo);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_MARK_C_))
CW_INLINE void
nxo_mark_new(cw_nxo_t *a_nxo)
{
    cw_check_ptr(a_nxo);

    nxo_p_new(a_nxo, NXOT_MARK);
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_MARK_C_)) */
