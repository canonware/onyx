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

typedef struct cw_stiloe_condition_s cw_stiloe_condition_t;

struct cw_stiloe_condition_s {
	cw_stiloe_t	stiloe;
	cw_cnd_t	condition;
};

void	stiloe_l_condition_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_condition_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
void	stilo_l_condition_print(cw_stilo_t *a_thread);
