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

typedef struct cw_nxoe_mutex_s cw_nxoe_mutex_t;

struct cw_nxoe_mutex_s {
	cw_nxoe_t	nxoe;
	cw_mtx_t	lock;
};

void	nxoe_l_mutex_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx);
cw_nxoe_t *nxoe_l_mutex_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
