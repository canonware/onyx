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

typedef struct cw_stiloe_mutex_s cw_stiloe_mutex_t;

struct cw_stiloe_mutex_s {
	cw_stiloe_t	stiloe;
	cw_mtx_t	lock;
};

void	stiloe_l_mutex_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_mutex_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);
void	stilo_l_mutex_print(cw_stilo_t *a_thread);
