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
modslate_frame_init(cw_nxo_t *a_thread);

/* Hooks. */
void
modslate_frame(void *a_data, cw_nxo_t *a_thread);

void
modslate_frame_p(void *a_data, cw_nxo_t *a_thread);

void
modslate_frame_aux_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_frame_aux_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_frame_focus(void *a_data, cw_nxo_t *a_thread);
