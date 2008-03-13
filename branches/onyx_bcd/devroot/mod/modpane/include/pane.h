/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

/* modpane initialization. */
void
modpane_pane_init(cw_nxo_t *a_thread);

/* Predicates. */

cw_nxn_t
modpane_pane_type(cw_nxo_t *a_nxo);

/* Hooks. */

void
modpane_pane(void *a_data, cw_nxo_t *a_thread);

void
modpane_subpane(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_aux(void *a_data, cw_nxo_t *a_thread);

void
modpane_pane_setaux(void *a_data, cw_nxo_t *a_thread);
