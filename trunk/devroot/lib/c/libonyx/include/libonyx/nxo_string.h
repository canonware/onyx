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

void	nxo_string_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking,
    cw_uint32_t a_len);
void	nxo_string_substring_new(cw_nxo_t *a_nxo, cw_nxo_t *a_string, cw_nx_t
    *a_nx, cw_uint32_t a_offset, cw_uint32_t a_len);
void	nxo_string_copy(cw_nxo_t *a_to, cw_nxo_t *a_from);
cw_uint32_t nxo_string_len_get(cw_nxo_t *a_nxo);
void	nxo_string_el_get(cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_uint8_t
    *r_el);
void	nxo_string_el_set(cw_nxo_t *a_nxo, cw_uint8_t a_el, cw_nxoi_t a_offset);
#ifdef _CW_THREADS
void	nxo_string_lock(cw_nxo_t *a_nxo);
void	nxo_string_unlock(cw_nxo_t *a_nxo);
#else
#define	nxo_string_lock(a_nxo)
#define	nxo_string_unlock(a_nxo)
#endif
cw_uint8_t *nxo_string_get(cw_nxo_t *a_nxo);
void	nxo_string_set(cw_nxo_t *a_nxo, cw_uint32_t a_offset, const cw_uint8_t
    *a_str, cw_uint32_t a_len);
