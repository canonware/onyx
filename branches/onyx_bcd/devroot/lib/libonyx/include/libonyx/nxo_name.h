/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

void
nxo_name_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_uint8_t *a_str,
	     cw_uint32_t a_len, cw_bool_t a_is_static);

const cw_uint8_t *
nxo_name_str_get(cw_nxo_t *a_nxo);

cw_uint32_t
nxo_name_len_get(cw_nxo_t *a_nxo);
