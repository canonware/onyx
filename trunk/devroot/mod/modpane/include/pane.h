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

/* Predicates. */

cw_nxn_t
pane_type(cw_nxo_t *a_nxo);

/* Handles. */

/* pane. */
void
modpane_pane(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_p(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_aux_get(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_aux_set(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_size(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_display(void *a_data, cw_nxo_t *a_thread);
