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
modslate_display_init(cw_nxo_t *a_thread);

/* Hooks. */
void
modslate_display(void *a_data, cw_nxo_t *a_thread);

void
modslate_display_p(void *a_data, cw_nxo_t *a_thread);

void
modslate_display_aux_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_display_aux_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_display_start(void *a_data, cw_nxo_t *a_thread);

void
modslate_display_stop(void *a_data, cw_nxo_t *a_thread);

void
modslate_display_redisplay(void *a_data, cw_nxo_t *a_thread);
