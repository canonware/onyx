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
struct cw_stila_s {
#ifdef _LIBSTIL_DBG
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
	cw_stilo_t	gcdict;

	/* Actual state of gcdict. */
	cw_bool_t	gcdict_active;
	cw_stiloi_t	gcdict_period;
	cw_stiloi_t	gcdict_threshold;
	cw_stiloi_t	gcdict_collections;
	cw_stiloi_t	gcdict_new;
	cw_stiloi_t	gcdict_current[3];
	cw_stiloi_t	gcdict_maximum[3];
	cw_stiloi_t	gcdict_sum[3];

	/* Sequence set. */
	ql_head(cw_stiloe_t) seq_set;
	cw_bool_t	white;	/* Current value for white (alternates). */

	cw_mq_t		gc_mq;
	cw_stil_t	*stil;
	cw_thd_t	*gc_thd;
};

void	stila_new(cw_stila_t *a_stila, cw_stil_t *a_stil);
void	stila_delete(cw_stila_t *a_stila);

void	stila_collect(cw_stila_t *a_stila);
void	stila_dump(cw_stila_t *a_stila, cw_stilo_t *a_thread);

cw_bool_t stila_active_get(cw_stila_t *a_stila);
void	stila_active_set(cw_stila_t *a_stila, cw_bool_t a_active);
cw_stiloi_t stila_period_get(cw_stila_t *a_stila);
void	stila_period_set(cw_stila_t *a_stila, cw_stiloi_t a_period);
cw_stiloi_t stila_threshold_get(cw_stila_t *a_stila);
void	stila_threshold_set(cw_stila_t *a_stila, cw_stiloi_t a_threshold);
cw_stiloi_t stila_collections_get(cw_stila_t *a_stila);
cw_stiloi_t stila_new_get(cw_stila_t *a_stila);
void	stila_current_get(cw_stila_t *a_stila, cw_stiloi_t *r_count, cw_stiloi_t
    *r_mark, cw_stiloi_t *r_sweep);
void	stila_maximum_get(cw_stila_t *a_stila, cw_stiloi_t *r_count, cw_stiloi_t
    *r_mark, cw_stiloi_t *r_sweep);
void	stila_sum_get(cw_stila_t *a_stila, cw_stiloi_t *r_count, cw_stiloi_t
    *r_mark, cw_stiloi_t *r_sweep);

#define	stila_gc_suspend(a_stila) thd_suspend(&(a_stila)->gc_thd)
#define	stila_gc_resume(a_stila) thd_resume(&(a_stila)->gc_thd)

#define	stila_gcdict_get(a_stila) &(a_stila)->gcdict

#define	stila_chi_pool_get(a_stila)	&(a_stila)->chi_pool
#define	stila_dicto_pool_get(a_stila)	&(a_stila)->dicto_pool

#define	stila_chi_get(a_stila)						\
	(cw_chi_t *)pool_get(&(a_stila)->chi_pool)
#define	stila_chi_put(a_stila, a_chi)					\
	pool_put(&(a_stila)->chi_pool, (a_chi))

#define	stila_dicto_get(a_stila)					\
	(cw_stiloe_dicto_t *)pool_get(&(a_stila)->dicto_pool)
#define	stila_dicto_put(a_stila, a_dicto)				\
	pool_put(&(a_stila)->dicto_pool, (a_dicto))

#define	stila_stackc_get(a_stila)					\
	(cw_stiloe_stackc_t *)pool_get(&(a_stila)->stackc_pool)
#define	stila_stackc_put(a_stila, a_stackc)				\
	pool_put(&(a_stila)->stackc_pool, (a_stackc))
