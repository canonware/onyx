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

typedef cw_stilte_t cw_stilo_hook_exec_t (void *a_data, cw_stilt_t *a_stilt);
typedef cw_stiloe_t *cw_stilo_hook_ref_iter_t (void *a_data, cw_bool_t
    a_reset);
typedef void cw_stilo_hook_delete_t (void *a_data, cw_stil_t *a_stil);

void		stilo_hook_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, void
    *a_data, cw_stilo_hook_exec_t *a_exec_f, cw_stilo_hook_ref_iter_t
    *a_ref_iter_f, cw_stilo_hook_delete_t *a_delete_f);
void		*stilo_hook_get(cw_stilo_t *a_stilo);
cw_stilte_t	stilo_hook_exec(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
