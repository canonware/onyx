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
nxo_name_new(cw_nxo_t *a_nxo, const char *a_str, uint32_t a_len,
	     bool a_is_static);

const char *
nxo_name_str_get(const cw_nxo_t *a_nxo);

uint32_t
nxo_name_len_get(const cw_nxo_t *a_nxo);
