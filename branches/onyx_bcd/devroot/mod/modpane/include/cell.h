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
modpane_cell_init(cw_nxo_t *a_thread);

/* Predicates. */
cw_nxn_t
modpane_cell_type(cw_nxo_t *a_nxo);

/* Hooks. */

void
modpane_cell(void *a_data, cw_nxo_t *a_thread);
