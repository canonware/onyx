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

#define	nxa_l_nx_get(a_nxa) (a_nxa)->nx

#define	nxa_l_chi_get(a_nxa)						\
	(cw_chi_t *)pool_get(&(a_nxa)->chi_pool)
#define	nxa_l_chi_put(a_nxa, a_chi)					\
	pool_put(&(a_nxa)->chi_pool, (a_chi))

#define	nxa_l_dicto_get(a_nxa)						\
	(cw_nxoe_dicto_t *)pool_get(&(a_nxa)->dicto_pool)
#define	nxa_l_dicto_put(a_nxa, a_dicto)					\
	pool_put(&(a_nxa)->dicto_pool, (a_dicto))

#define	nxa_l_stackc_get(a_nxa)						\
	(cw_nxoe_stackc_t *)pool_get(&(a_nxa)->stackc_pool)
#define	nxa_l_stackc_put(a_nxa, a_stackc)				\
	pool_put(&(a_nxa)->stackc_pool, (a_stackc))
