/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

struct cw_stiloe_array_s {
	cw_stiloe_t	stiloe;
	cw_uint32_t	iterations;	/*
					 * Used for remembering the current
					 * state of reference iteration.
					 */
	union {
		cw_stiloec_t	stiloec;
		struct {
			cw_stilo_t	*arr;
			cw_uint32_t	len;
		}	a;
	}	e;
};

cw_stiloe_t	*stiloe_array_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);

cw_sint32_t	stiloe_array_len_get(cw_stiloe_t *a_stiloe);

void		stiloe_array_len_set(cw_stiloe_t *a_stiloe, cw_uint32_t a_len);

cw_stilo_t	*stiloe_array_el_get(cw_stiloe_t *a_stiloe, cw_uint32_t
    a_offset);

cw_stilo_t	*stiloe_array_get(cw_stiloe_t *a_stiloe);

void		stiloe_array_set(cw_stiloe_t *a_stiloe, cw_uint32_t a_offset,
    cw_stilo_t *a_arr, cw_uint32_t a_len);
