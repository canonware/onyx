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

void		stilo_condition_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil);
void		stilo_condition_signal(cw_stilo_t *a_stilo);
void		stilo_condition_broadcast(cw_stilo_t *a_stilo);
void		stilo_condition_wait(cw_stilo_t *a_stilo, cw_stilo_t *a_mutex);
cw_bool_t	stilo_condition_timedwait(cw_stilo_t *a_stilo, cw_stilo_t
    *a_mutex, const struct timespec *a_timeout);
