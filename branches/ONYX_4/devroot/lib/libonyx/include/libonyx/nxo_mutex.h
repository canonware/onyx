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

void
nxo_mutex_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx);

void
nxo_mutex_lock(cw_nxo_t *a_nxo);

cw_bool_t
nxo_mutex_trylock(cw_nxo_t *a_nxo);

void
nxo_mutex_unlock(cw_nxo_t *a_nxo);
