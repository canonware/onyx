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

typedef struct cw_stiloe_name_s cw_stiloe_name_t;

struct cw_stiloe_name_s {
	cw_stiloe_t	stiloe;
	/*
	 * name is not required to be NULL-terminated, so we keep track of the
	 * length.
	 */
	const cw_uint8_t *str;
	cw_uint32_t	len;
};

void	stiloe_l_name_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_name_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);
cw_stilte_t stilo_l_name_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);
cw_uint32_t stilo_l_name_hash(const void *a_key);
cw_bool_t stilo_l_name_key_comp(const void *a_k1, const void *a_k2);
