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

typedef cw_nxoe_t *cw_nxo_class_ref_iter_t (void *a_opaque, cw_bool_t a_reset);

typedef cw_bool_t cw_nxo_class_delete_t (void *a_opaque, cw_uint32_t a_iter);

void
nxo_class_new(cw_nxo_t *a_nxo, void *a_opaque,
	      cw_nxo_class_ref_iter_t *a_ref_iter_f,
	      cw_nxo_class_delete_t *a_delete_f);

cw_nxo_t *
nxo_class_super_get(const cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_class_methods_get(const cw_nxo_t *a_nxo);

cw_nxo_t *
nxo_class_data_get(const cw_nxo_t *a_nxo);

void *
nxo_class_opaque_get(const cw_nxo_t *a_nxo);

void
nxo_class_opaque_set(cw_nxo_t *a_nxo, void *a_opaque);
