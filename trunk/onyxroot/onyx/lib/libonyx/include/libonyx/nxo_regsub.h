/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version = onyx>
 *
 ******************************************************************************/

cw_nxn_t
nxo_regsub_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_uint8_t *a_pattern,
	       cw_uint32_t a_plen, cw_bool_t a_global, cw_bool_t a_insensitive,
	       cw_bool_t a_multiline, cw_bool_t a_singleline,
	       const cw_uint8_t *a_template,
	       cw_uint32_t a_tlen);

void
nxo_regsub_subst(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *a_input,
		 cw_nxo_t *r_output);

cw_nxn_t
nxo_regsub_nonew_subst(cw_nxo_t *a_thread, const cw_uint8_t *a_pattern,
		       cw_uint32_t a_plen, cw_bool_t a_global,
		       cw_bool_t a_insensitive, cw_bool_t a_multiline,
		       cw_bool_t a_singleline, const cw_uint8_t *a_template,
		       cw_uint32_t a_tlen, cw_nxo_t *a_input,
		       cw_nxo_t *r_output);
