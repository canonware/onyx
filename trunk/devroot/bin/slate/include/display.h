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

void	slate_display_init(cw_nxo_t *a_thread);
cw_nxn_t display_type(cw_nxo_t *a_nxo);

void	slate_display(void *a_data, cw_nxo_t *a_thread);
void	slate_display_start(void *a_data, cw_nxo_t *a_thread);
void	slate_display_stop(void *a_data, cw_nxo_t *a_thread);
void	slate_display_redisplay(void *a_data, cw_nxo_t *a_thread);
