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
modpane_display_init(cw_nxo_t *a_thread);

/* Predicates. */

cw_nxn_t
modpane_display_type(cw_nxo_t *a_nxo);

/* Hooks. */

void
modpane_display(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_aux(void *a_data, cw_nxo_t *a_thread);

void
modpane_display_setaux(void *a_data, cw_nxo_t *a_thread);
