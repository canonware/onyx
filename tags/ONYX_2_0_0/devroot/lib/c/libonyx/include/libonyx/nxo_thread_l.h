/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

void	nxoe_l_thread_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa);
cw_nxoe_t *nxoe_l_thread_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
cw_uint32_t nxo_l_thread_token(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
