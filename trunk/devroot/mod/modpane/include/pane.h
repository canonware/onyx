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
modpane_pane_init(cw_nxo_t *a_thread);

/* Helper functions. */
cw_nxn_t
modpane_pane_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread);

/* Methods. */
void
modpane_pane_pane(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_size(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_display(void *a_data, cw_nxo_t *a_thread);
