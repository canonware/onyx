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
nxo_real_new(cw_nxo_t *a_nxo, cw_nxor_t a_val);

cw_nxor_t
nxo_real_get(cw_nxo_t *a_nxo);

void
nxo_real_set(cw_nxo_t *a_nxo, cw_nxor_t a_val);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_REAL_C_))
CW_INLINE void
nxo_real_new(cw_nxo_t *a_nxo, cw_nxor_t a_val)
{
    cw_check_ptr(a_nxo);

    nxo_p_new(a_nxo, NXOT_REAL);
    a_nxo->o.real.r = a_val;
}

CW_INLINE cw_nxor_t
nxo_real_get(cw_nxo_t *a_nxo)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_REAL);

    return a_nxo->o.real.r;
}

CW_INLINE void
nxo_real_set(cw_nxo_t *a_nxo, cw_nxor_t a_val)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_REAL);

    a_nxo->o.real.r = a_val;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_REAL_C_)) */
