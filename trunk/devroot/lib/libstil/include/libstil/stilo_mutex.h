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

void		stilo_mutex_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil);
void		stilo_mutex_lock(cw_stilo_t *a_stilo);
cw_bool_t	stilo_mutex_trylock(cw_stilo_t *a_stilo);
void		stilo_mutex_unlock(cw_stilo_t *a_stilo);
