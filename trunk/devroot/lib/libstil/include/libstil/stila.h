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
	cw_pool_t	stilsc_pool;
	cw_pool_t	dicto_pool;

	/* Sequence set. */
	ql_head(cw_stiloe_t) seq_set;
	/* Number of sequence set additions since the last collection. */
	cw_uint32_t	seq_new;
	cw_bool_t	white;	/* Current value for white (alternates). */

	cw_mq_t		gc_mq;
	cw_stil_t	*stil;
	cw_thd_t	gc_thd;
};

void	stila_new(cw_stila_t *a_stila, cw_stil_t *a_stil);
void	stila_delete(cw_stila_t *a_stila);

void	stila_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe);
#define	stila_gc_force(a_stila) mq_put(&(a_stila)->gc_mq, STILAM_FORCE)
#define	stila_gc_suspend(a_stila) thd_suspend(&(a_stila)->gc_thd)
#define	stila_gc_resume(a_stila) thd_resume(&(a_stila)->gc_thd)

#define	stila_chi_pool_get(a_stila)	&(a_stila)->chi_pool
#define	stila_stilsc_pool_get(a_stila)	&(a_stila)->stilsc_pool
#define	stila_dicto_pool_get(a_stila)	&(a_stila)->dicto_pool

#define	stila_chi_get(a_stila)						\
	(cw_chi_t *)pool_get(&(a_stila)->chi_pool)
#define	stila_chi_put(a_stila, a_chi)					\
	pool_put(&(a_stila)->chi_pool, (a_chi))

#define	stila_stilsc_get(a_stila)					\
	(cw_stilsc_t *)pool_get(&(a_stila)->stilsc_pool)
#define	stila_stilsc_put(a_stila, a_stilsc)				\
	pool_put(&(a_stila)->stilsc_pool, (a_stilsc))

#define	stila_dicto_get(a_stila)					\
	(cw_stiloe_dicto_t *)pool_get(&(a_stila)->dicto_pool)
#define	stila_dicto_put(a_stila, a_dicto)				\
	pool_put(&(a_stila)->dicto_pool, (a_dicto))
