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

typedef cw_nxoe_t *cw_nxo_instance_ref_iter_t (void *a_opaque,
					       bool a_reset);

typedef bool cw_nxo_instance_delete_t (void *a_opaque, uint32_t a_iter);

void
nxo_instance_new(cw_nxo_t *a_nxo, void *a_opaque,
		 cw_nxo_instance_ref_iter_t *a_ref_iter_f,
		 cw_nxo_instance_delete_t *a_delete_f);

cw_nxo_t *
nxo_instance_isa_get(const cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_instance_data_get(const cw_nxo_t *a_nxo);

void *
nxo_instance_opaque_get(const cw_nxo_t *a_nxo);

void
nxo_instance_opaque_set(cw_nxo_t *a_nxo, void *a_opaque);
