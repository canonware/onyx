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
nxo_regsub_new(cw_nxo_t *a_nxo, const char *a_pattern, uint32_t a_plen,
	       bool a_global, bool a_insensitive,
	       bool a_multiline, bool a_singleline,
	       const char *a_template, uint32_t a_tlen);

void
nxo_regsub_subst(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *a_input,
		 cw_nxo_t *r_output, uint32_t *r_count);

cw_nxn_t
nxo_regsub_nonew_subst(cw_nxo_t *a_thread, const char *a_pattern,
		       uint32_t a_plen, bool a_global,
		       bool a_insensitive, bool a_multiline,
		       bool a_singleline, const char *a_template,
		       uint32_t a_tlen, cw_nxo_t *a_input,
		       cw_nxo_t *r_output, uint32_t *r_count);
