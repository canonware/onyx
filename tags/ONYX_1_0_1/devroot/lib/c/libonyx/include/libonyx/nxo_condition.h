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

void	nxo_condition_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx);
void	nxo_condition_signal(cw_nxo_t *a_nxo);
void	nxo_condition_broadcast(cw_nxo_t *a_nxo);
void	nxo_condition_wait(cw_nxo_t *a_nxo, cw_nxo_t *a_mutex);
cw_bool_t nxo_condition_timedwait(cw_nxo_t *a_nxo, cw_nxo_t *a_mutex, const
    struct timespec *a_timeout);
