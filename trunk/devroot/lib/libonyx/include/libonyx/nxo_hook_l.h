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

typedef struct cw_nxoe_hook_s cw_nxoe_hook_t;

struct cw_nxoe_hook_s {
	cw_nxoe_t		nxoe;
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t		ref_iter;
	cw_nxo_t		tag;
	void			*data;
	cw_nxo_hook_eval_t	*eval_f;
	cw_nxo_hook_ref_iter_t	*ref_iter_f;
	cw_nxo_hook_delete_t	*delete_f;
};

void	nxoe_l_hook_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx);
cw_nxoe_t *nxoe_l_hook_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
void	nxo_l_hook_print(cw_nxo_t *a_thread);
