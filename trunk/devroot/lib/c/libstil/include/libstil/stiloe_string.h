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

struct cw_stiloe_string_s {
	cw_stiloe_t	stiloe;
	cw_uint32_t	iterations;	/*
					 * Used for remembering the current
					 * state of reference iteration.
					 */
	union {
		cw_stiloec_t	stiloec;
		struct {
			cw_uint8_t	*str;
			cw_sint32_t	len;	/* -1 if unset. */
		}	s;
	}	e;
};

cw_sint32_t	stiloe_string_len_get(cw_stiloe_t *a_stiloe);

void		stiloe_string_len_set(cw_stiloe_t *a_stiloe, cw_uint32_t a_len);

cw_uint8_t	*stiloe_string_el_get(cw_stiloe_t *a_stiloe, cw_uint32_t
    a_offset);

cw_uint8_t	*stiloe_string_get(cw_stiloe_t *a_stiloe);

void		stiloe_string_set(cw_stiloe_t *a_stiloe, cw_uint32_t a_offset,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
