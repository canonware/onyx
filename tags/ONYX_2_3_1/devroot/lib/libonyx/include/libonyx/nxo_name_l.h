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

typedef struct cw_nxoe_name_s cw_nxoe_name_t;

struct cw_nxoe_name_s {
	cw_nxoe_t		nxoe;
	/*
	 * name is not required to be NULL-terminated, so we keep track of the
	 * length.
	 */
	const cw_uint8_t	*str;
	cw_uint32_t		len;
};

cw_bool_t nxoe_l_name_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t
    a_iter);
cw_nxoe_t *nxoe_l_name_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
cw_uint32_t nxo_l_name_hash(const void *a_key);
cw_bool_t nxo_l_name_key_comp(const void *a_k1, const void *a_k2);
