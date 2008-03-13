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

cw_nxn_t
nxm_new(cw_nxo_t *a_nxo, cw_nxo_t *a_path, cw_nxo_t *a_sym);

uint32_t
nxm_iter_get(cw_nxo_t *a_nxo);

void
nxm_iter_set(cw_nxo_t *a_nxo, uint32_t a_iter);

void *
nxm_pre_unload_hook_get(cw_nxo_t *a_nxo);

void
nxm_pre_unload_hook_set(cw_nxo_t *a_nxo, void (*a_pre_unload_hook)(void));
