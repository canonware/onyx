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

typedef struct cw_stiloe_hook_s cw_stiloe_hook_t;

struct cw_stiloe_hook_s {
	cw_stiloe_t	stiloe;
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;
	cw_stilo_t	tag;
	void		*data;
	cw_stilo_hook_eval_t *eval_f;
	cw_stilo_hook_ref_iter_t *ref_iter_f;
	cw_stilo_hook_delete_t *delete_f;
};

void	stiloe_l_hook_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_hook_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);
void	stilo_l_hook_print(cw_stilo_t *a_thread);
