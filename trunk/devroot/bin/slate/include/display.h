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

void	slate_display(void *a_data, cw_nxo_t *a_thread);
void	slate_display_suspend(void *a_data, cw_nxo_t *a_thread);
void	slate_display_resume(void *a_data, cw_nxo_t *a_thread);
void	slate_display_redisplay(void *a_data, cw_nxo_t *a_thread);
