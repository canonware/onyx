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
nxo_pmark_new(cw_nxo_t *a_nxo);

cw_uint32_t
nxo_pmark_line_get(cw_nxo_t *a_nxo);

void
nxo_pmark_line_set(cw_nxo_t *a_nxo, cw_uint32_t a_line);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_PMARK_C_))
CW_INLINE void
nxo_pmark_new(cw_nxo_t *a_nxo)
{
    cw_check_ptr(a_nxo);

    nxo_p_new(a_nxo, NXOT_PMARK);
}

CW_INLINE cw_uint32_t
nxo_pmark_line_get(cw_nxo_t *a_nxo)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_PMARK);

    return a_nxo->o.pmark.line;
}

CW_INLINE void
nxo_pmark_line_set(cw_nxo_t *a_nxo, cw_uint32_t a_line)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_PMARK);

    a_nxo->o.pmark.line = a_line;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_PMARK_C_)) */
