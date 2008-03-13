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

typedef struct cw_nxoe_condition_s cw_nxoe_condition_t;

struct cw_nxoe_condition_s {
	cw_nxoe_t	nxoe;
	cw_cnd_t	condition;
};

void	nxoe_l_condition_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx);
cw_nxoe_t *nxoe_l_condition_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
void	nxo_l_condition_print(cw_nxo_t *a_thread);
