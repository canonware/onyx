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
modslate_funnel_init(cw_nxo_t *a_thread);

/* C-specific APIs. */
void
modslate_funnel_c_enter(cw_nxo_t *a_funnel);

void
modslate_funnel_c_leave(cw_nxo_t *a_funnel);

/* Hooks. */
void
modslate_funnel(void *a_data, cw_nxo_t *a_thread);

void
modslate_funnel_p(void *a_data, cw_nxo_t *a_thread);

void
modslate_funnel_enter(void *a_data, cw_nxo_t *a_thread);

void
modslate_funnel_leave(void *a_data, cw_nxo_t *a_thread);
