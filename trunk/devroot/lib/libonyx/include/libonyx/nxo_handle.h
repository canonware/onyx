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

typedef void cw_nxo_handle_eval_t (void *a_data, cw_nxo_t *a_thread);

typedef cw_nxoe_t *cw_nxo_handle_ref_iter_t (void *a_data, cw_bool_t a_reset);

typedef cw_bool_t cw_nxo_handle_delete_t (void *a_data, cw_uint32_t a_iter);

void
nxo_handle_new(cw_nxo_t *a_nxo, void *a_data, cw_nxo_handle_eval_t *a_eval_f,
	       cw_nxo_handle_ref_iter_t *a_ref_iter_f,
	       cw_nxo_handle_delete_t *a_delete_f);

cw_nxo_t *
nxo_handle_tag_get(const cw_nxo_t *a_nxo);

void *
nxo_handle_data_get(const cw_nxo_t *a_nxo);

void
nxo_handle_data_set(cw_nxo_t *a_nxo, void *a_data);

void
nxo_handle_eval(cw_nxo_t *a_nxo, cw_nxo_t *a_thread);
