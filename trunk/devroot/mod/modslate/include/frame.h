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

/* Helper functions. */
cw_nxn_t
modslate_frame_p(cw_nxo_t *a_instance, cw_nxo_t *a_thread);

/* Methods. */
void
modslate_frame_frame(void *a_data, cw_nxo_t *a_thread);

void
modslate_frame_focus(void *a_data, cw_nxo_t *a_thread);
