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

typedef cw_stilo_threade_t cw_stilo_hook_eval_t (void *a_data, cw_stilo_t
    *a_thread);
typedef cw_stiloe_t *cw_stilo_hook_ref_iter_t (void *a_data, cw_bool_t
    a_reset);
typedef void cw_stilo_hook_delete_t (void *a_data, cw_stil_t *a_stil);

void		stilo_hook_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, void
    *a_data, cw_stilo_hook_eval_t *a_eval_f, cw_stilo_hook_ref_iter_t
    *a_ref_iter_f, cw_stilo_hook_delete_t *a_delete_f);
cw_stilo_t	*stilo_hook_tag_get(cw_stilo_t *a_stilo);
void		*stilo_hook_data_get(cw_stilo_t *a_stilo);
cw_stilo_threade_t	stilo_hook_eval(cw_stilo_t *a_stilo, cw_stilo_t
    *a_thread);
