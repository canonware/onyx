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
#ifdef _LIBONYX_DBG
	cw_uint32_t	magic;
#endif
	cw_mtx_t	lock;

	/* Various pools. */
	cw_pool_t	chi_pool;
	cw_pool_t	dicto_pool;
	cw_pool_t	stackc_pool;

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
cw_nxoi_t nxa_collections_get(cw_nxa_t *a_nxa);
cw_nxoi_t nxa_new_get(cw_nxa_t *a_nxa);
void	nxa_current_get(cw_nxa_t *a_nxa, cw_nxoi_t *r_count, cw_nxoi_t *r_mark,
    cw_nxoi_t *r_sweep);
void	nxa_maximum_get(cw_nxa_t *a_nxa, cw_nxoi_t *r_count, cw_nxoi_t *r_mark,
    cw_nxoi_t *r_sweep);
void	nxa_sum_get(cw_nxa_t *a_nxa, cw_nxoi_t *r_count, cw_nxoi_t *r_mark,
    cw_nxoi_t *r_sweep);

#define	nxa_gcdict_get(a_nxa) &(a_nxa)->gcdict
