/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Slate <Version = slate>
 *
 ******************************************************************************/

/* modslate initialization. */
void
modslate_window_init(cw_nxo_t *a_thread);

/* Hooks. */
void
modslate_window(void *a_data, cw_nxo_t *a_thread);

void
modslate_window_p(void *a_data, cw_nxo_t *a_thread);

void
modslate_window_aux_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_window_aux_set(void *a_data, cw_nxo_t *a_thread);
