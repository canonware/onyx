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

/* modpane initialization. */
void
modpane_display_init(cw_nxo_t *a_thread);

/* Helper functions. */
cw_nxn_t
modpane_display_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread);

/* Methods. */
void
modpane_display_display(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_size(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_pane(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_start(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_stop(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_refresh(void *a_data, cw_nxo_t *a_thread);
