/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

void
gcdict_active(cw_nxo_t *a_thread);

void
gcdict_collect(cw_nxo_t *a_thread);

#ifdef CW_PTHREADS
void
gcdict_period(cw_nxo_t *a_thread);
#endif

void
gcdict_setactive(cw_nxo_t *a_thread);

#ifdef CW_PTHREADS
void
gcdict_setperiod(cw_nxo_t *a_thread);
#endif

void
gcdict_setthreshold(cw_nxo_t *a_thread);

void
gcdict_stats(cw_nxo_t *a_thread);

void
gcdict_threshold(cw_nxo_t *a_thread);
