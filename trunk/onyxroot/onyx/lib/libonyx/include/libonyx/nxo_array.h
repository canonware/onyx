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
nxo_array_new(cw_nxo_t *a_nxo, bool a_locking, uint32_t a_len);

void
nxo_array_subarray_new(cw_nxo_t *a_nxo, cw_nxo_t *a_array, uint32_t a_offset,
		       uint32_t a_len);

void
nxo_array_copy(cw_nxo_t *a_to, cw_nxo_t *a_from);

uint32_t
nxo_array_len_get(const cw_nxo_t *a_nxo);

void
nxo_array_el_get(const cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el);

void
nxo_array_el_set(cw_nxo_t *a_nxo, cw_nxo_t *a_el, cw_nxoi_t a_offset);

bool
nxo_array_origin_get(cw_nxo_t *a_nxo, const uint8_t **r_origin,
		     uint32_t *r_olen, uint32_t *r_line_num);

void
nxo_array_origin_set(cw_nxo_t *a_nxo, const uint8_t *a_origin,
		     uint32_t a_olen, uint32_t a_line_num);
