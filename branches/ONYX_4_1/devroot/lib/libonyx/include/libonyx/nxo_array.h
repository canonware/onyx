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
nxo_array_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking,
	      cw_uint32_t a_len);

void
nxo_array_subarray_new(cw_nxo_t *a_nxo, cw_nxo_t *a_array, cw_nx_t *a_nx,
		       cw_uint32_t a_offset, cw_uint32_t a_len);

void
nxo_array_copy(cw_nxo_t *a_to, cw_nxo_t *a_from);

cw_uint32_t
nxo_array_len_get(const cw_nxo_t *a_nxo);

void
nxo_array_el_get(const cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el);

void
nxo_array_el_set(cw_nxo_t *a_nxo, cw_nxo_t *a_el, cw_nxoi_t a_offset);
