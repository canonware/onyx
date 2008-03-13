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
#ifdef _CW_THREADS
	/* Protects the gcdict_* fields and gc_pending. */
	cw_mtx_t	lock;
#endif

	/*
	 * Dictionary that contains stats and flags, available from within the
	 * interpreter.
	 */
	cw_nxo_t	gcdict;

	/* Actual state of gcdict. */
	cw_bool_t	gcdict_active;
#ifdef _CW_THREADS
	cw_nxoi_t	gcdict_period;
#endif
	cw_nxoi_t	gcdict_threshold;
	cw_nxoi_t	gcdict_collections;
	cw_nxoi_t	gcdict_count;
	cw_nxoi_t	gcdict_current[3];
	cw_nxoi_t	gcdict_maximum[3];
	cw_nxoi_t	gcdict_sum[3];

	/* Sequence set. */
#ifdef _CW_THREADS
	cw_mtx_t	seq_mtx;
#endif
	ql_head(cw_nxoe_t) seq_set;
	cw_bool_t	white;	/* Current value for white (alternates). */

#ifdef _CW_THREADS
	cw_mq_t		gc_mq;
#endif
	cw_bool_t	gc_pending;
	cw_bool_t	gc_allocated;

	cw_nx_t		*nx;
#ifdef _CW_THREADS
	cw_thd_t	*gc_thd;
#endif
};

void	nxa_new(cw_nxa_t *a_nxa, cw_nx_t *a_nx);
void	nxa_delete(cw_nxa_t *a_nxa);

void	*nxa_malloc_e(cw_nxa_t *a_nxa, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num);
void	nxa_free_e(cw_nxa_t *a_nxa, void *a_ptr, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);

#ifdef _CW_DBG
#define	nxa_malloc(a_nxa, a_size)					\
	nxa_malloc_e((a_nxa), (a_size), __FILE__, __LINE__)
#define	nxa_free(a_nxa, a_ptr, a_size)					\
	nxa_free_e((a_nxa), (a_ptr), (a_size), __FILE__, __LINE__)
#else
#define	nxa_malloc(a_nxa, a_size)					\
	nxa_malloc_e((a_nxa), (a_size), NULL, 0)
#define	nxa_free(a_nxa, a_ptr, a_size)					\
	nxa_free_e((a_nxa), (a_ptr), (a_size), NULL, 0)
#endif

void	nxa_collect(cw_nxa_t *a_nxa);

cw_bool_t nxa_active_get(cw_nxa_t *a_nxa);
void	nxa_active_set(cw_nxa_t *a_nxa, cw_bool_t a_active);
#ifdef _CW_THREADS
cw_nxoi_t nxa_period_get(cw_nxa_t *a_nxa);
void	nxa_period_set(cw_nxa_t *a_nxa, cw_nxoi_t a_period);
#endif
cw_nxoi_t nxa_threshold_get(cw_nxa_t *a_nxa);
void	nxa_threshold_set(cw_nxa_t *a_nxa, cw_nxoi_t a_threshold);
void	nxa_stats_get(cw_nxa_t *a_nxa, cw_nxoi_t *r_collections, cw_nxoi_t
    *r_count, cw_nxoi_t *r_ccount, cw_nxoi_t *r_cmark, cw_nxoi_t *r_csweep,
    cw_nxoi_t *r_mcount, cw_nxoi_t *r_mmark, cw_nxoi_t *r_msweep, cw_nxoi_t
    *r_scount, cw_nxoi_t *r_smark, cw_nxoi_t *r_ssweep);

#ifndef _CW_USE_INLINES
cw_nx_t *nxa_nx_get(cw_nxa_t *a_nxa);
cw_nxo_t *nxa_gcdict_get(cw_nxa_t *a_nxa);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NXA_C_))
_CW_INLINE cw_nx_t *
nxa_nx_get(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	return a_nxa->nx;
}

_CW_INLINE cw_nxo_t *
nxa_gcdict_get(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	return &a_nxa->gcdict;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXA_C_)) */
