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
origin_l_insert(void *a_obj, const cw_uint8_t *a_origin, cw_uint32_t a_olen,
		cw_uint32_t a_line_num);

void
origin_l_remove(void *a_obj);

cw_bool_t
origin_l_lookup(void *a_obj, const cw_uint8_t **r_origin,
		cw_uint32_t *r_olen, cw_uint32_t *r_line_num);
