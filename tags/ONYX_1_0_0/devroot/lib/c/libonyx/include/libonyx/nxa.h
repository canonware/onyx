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

/*
 * Memory allocator.
 */
struct cw_nxa_s {
#ifdef _CW_DBG
	cw_uint32_t	magic;
#define	_CW_NXA_MAGIC	0x63935743
#endif
	cw_mtx_t	lock;

	/* Various pools. */
	cw_pool_t	chi_pool;
	cw_pool_t	dicto_pool;
	cw_pool_t	stacko_pool;

	/*
	 * Dictionary that contains stats and flags, available from within the
	 * interpreter.
	 */
	cw_nxo_t	gcdict;

	/* Actual state of gcdict. */
	cw_bool_t	gcdict_active;
	cw_nxoi_t	gcdict_period;
	cw_nxoi_t	gcdict_threshold;
	cw_nxoi_t	gcdict_collections;
	cw_nxoi_t	gcdict_new;
	cw_nxoi_t	gcdict_current[3];
	cw_nxoi_t	gcdict_maximum[3];
	cw_nxoi_t	gcdict_sum[3];

	/* Sequence set. */
	ql_head(cw_nxoe_t) seq_set;
	cw_bool_t	white;	/* Current value for white (alternates). */

	cw_mq_t		gc_mq;
	cw_nx_t		*nx;
	cw_thd_t	*gc_thd;
};

void	nxa_new(cw_nxa_t *a_nxa, cw_nx_t *a_nx);
void	nxa_delete(cw_nxa_t *a_nxa);

void	nxa_collect(cw_nxa_t *a_nxa);
void	nxa_dump(cw_nxa_t *a_nxa, cw_nxo_t *a_thread);

cw_bool_t nxa_active_get(cw_nxa_t *a_nxa);
void	nxa_active_set(cw_nxa_t *a_nxa, cw_bool_t a_active);
cw_nxoi_t nxa_period_get(cw_nxa_t *a_nxa);
void	nxa_period_set(cw_nxa_t *a_nxa, cw_nxoi_t a_period);
cw_nxoi_t nxa_threshold_get(cw_nxa_t *a_nxa);
void	nxa_threshold_set(cw_nxa_t *a_nxa, cw_nxoi_t a_threshold);
void	nxa_stats_get(cw_nxa_t *a_nxa, cw_nxoi_t *r_collections, cw_nxoi_t
    *r_new, cw_nxoi_t *r_ccount, cw_nxoi_t *r_cmark, cw_nxoi_t *r_csweep,
    cw_nxoi_t *r_mcount, cw_nxoi_t *r_mmark, cw_nxoi_t *r_msweep, cw_nxoi_t
    *r_scount, cw_nxoi_t *r_smark, cw_nxoi_t *r_ssweep);

#ifndef _CW_USE_INLINES
cw_nxo_t *nxa_gcdict_get(cw_nxa_t *a_nxa);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NXA_C_))
_CW_INLINE cw_nxo_t *
nxa_gcdict_get(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	return &a_nxa->gcdict;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXA_C_)) */
