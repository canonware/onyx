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
origin_l_insert(void *a_obj, const char *a_origin, uint32_t a_olen,
		uint32_t a_line_num);

void
origin_l_remove(void *a_obj);

bool
origin_l_lookup(void *a_obj, const char **r_origin,
		uint32_t *r_olen, uint32_t *r_line_num);
