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

void	gcdict_active(cw_nxo_t *a_thread);
void	gcdict_collect(cw_nxo_t *a_thread);
#ifdef CW_THREADS
void	gcdict_period(cw_nxo_t *a_thread);
#endif
void	gcdict_setactive(cw_nxo_t *a_thread);
#ifdef CW_THREADS
void	gcdict_setperiod(cw_nxo_t *a_thread);
#endif
void	gcdict_setthreshold(cw_nxo_t *a_thread);
void	gcdict_stats(cw_nxo_t *a_thread);
void	gcdict_threshold(cw_nxo_t *a_thread);
