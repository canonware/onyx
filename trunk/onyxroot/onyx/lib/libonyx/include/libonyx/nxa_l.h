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

void	nxa_l_gc_register(cw_nxa_t *a_nxa, cw_nxoe_t *a_nxoe);
cw_bool_t nxa_l_white_get(cw_nxa_t *a_nxa);

/* Private, but the pool allocators need to be able to see it. */
void	nxa_p_collect(cw_nxa_t *a_nxa);

#ifndef _CW_USE_INLINES
cw_nx_t *nxa_l_nx_get(cw_nxa_t *a_nxa);
cw_chi_t *nxa_l_chi_get(cw_nxa_t *a_nxa);
void	nxa_l_chi_put(cw_nxa_t *a_nxa, cw_chi_t *a_chi);
cw_nxoe_dicto_t *nxa_l_dicto_get(cw_nxa_t *a_nxa);
void	nxa_l_dicto_put(cw_nxa_t *a_nxa, cw_nxoe_dicto_t *a_dicto);
cw_nxoe_stacko_t *nxa_l_stacko_get(cw_nxa_t *a_nxa);
void	nxa_l_stacko_put(cw_nxa_t *a_nxa, cw_nxoe_stacko_t *a_stacko);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NXA_C_))
_CW_INLINE cw_nx_t *
nxa_l_nx_get(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	return a_nxa->nx;
}

_CW_INLINE cw_chi_t *
nxa_l_chi_get(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);

	/* Update new. */
	a_nxa->gcdict_new += (cw_nxoi_t)a_nxa->chi_sizeof;

	/* Trigger a collection if the threshold was reached. */
	if (a_nxa->gcdict_new >= a_nxa->gcdict_threshold &&
	    a_nxa->gcdict_active && a_nxa->gcdict_threshold != 0) {
		mtx_lock(&a_nxa->interlock);
		a_nxa->prev_new = 0;
		mtx_unlock(&a_nxa->lock);
		nxa_p_collect(a_nxa);
		mtx_unlock(&a_nxa->interlock);
	} else
		mtx_unlock(&a_nxa->lock);

	return (cw_chi_t *)pool_get(&a_nxa->chi_pool);
}

_CW_INLINE void
nxa_l_chi_put(cw_nxa_t *a_nxa, cw_chi_t *a_chi)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);
	_cw_check_ptr(a_chi);

	pool_put(&a_nxa->chi_pool, a_chi);
}

_CW_INLINE cw_nxoe_dicto_t *
nxa_l_dicto_get(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);

	/* Update new. */
	a_nxa->gcdict_new += (cw_nxoi_t)a_nxa->dicto_sizeof;

	/* Trigger a collection if the threshold was reached. */
	if (a_nxa->gcdict_new >= a_nxa->gcdict_threshold &&
	    a_nxa->gcdict_active && a_nxa->gcdict_threshold != 0) {
		mtx_lock(&a_nxa->interlock);
		a_nxa->prev_new = 0;
		mtx_unlock(&a_nxa->lock);
		nxa_p_collect(a_nxa);
		mtx_unlock(&a_nxa->interlock);
	} else
		mtx_unlock(&a_nxa->lock);

	return (cw_nxoe_dicto_t *)pool_get(&a_nxa->dicto_pool);
}

_CW_INLINE void
nxa_l_dicto_put(cw_nxa_t *a_nxa, cw_nxoe_dicto_t *a_dicto)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);
	_cw_check_ptr(a_dicto);

	pool_put(&a_nxa->dicto_pool, a_dicto);
}

_CW_INLINE cw_nxoe_stacko_t *
nxa_l_stacko_get(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);

	/* Update new. */
	a_nxa->gcdict_new += (cw_nxoi_t)a_nxa->stacko_sizeof;

	/* Trigger a collection if the threshold was reached. */
	if (a_nxa->gcdict_new >= a_nxa->gcdict_threshold &&
	    a_nxa->gcdict_active && a_nxa->gcdict_threshold != 0) {
		mtx_lock(&a_nxa->interlock);
		a_nxa->prev_new = 0;
		mtx_unlock(&a_nxa->lock);
		nxa_p_collect(a_nxa);
		mtx_unlock(&a_nxa->interlock);
	} else
		mtx_unlock(&a_nxa->lock);

	return (cw_nxoe_stacko_t *)pool_get(&a_nxa->stacko_pool);
}

_CW_INLINE void
nxa_l_stacko_put(cw_nxa_t *a_nxa, cw_nxoe_stacko_t *a_stacko)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);
	_cw_check_ptr(a_stacko);

	pool_put(&a_nxa->stacko_pool, a_stacko);
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXA_C_)) */
