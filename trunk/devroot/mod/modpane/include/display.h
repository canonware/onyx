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

/* Predicates. */

cw_nxn_t
display_type(cw_nxo_t *a_nxo);

/* Hooks. */

/* display. */
void
modpane_display(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_p(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_aux_get(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_aux_set(void *a_data, cw_nxo_t *a_thread);

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
