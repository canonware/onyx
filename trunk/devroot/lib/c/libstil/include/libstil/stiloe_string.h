/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Defined in stilo.h to resolve a circular dependency. */
#if (0)
typedef struct cw_stiloe_string_s cw_stiloe_string_t;
#endif

struct cw_stiloe_string_s {
	cw_stiloe_t stiloe;
};

cw_stiloe_string_t *stiloe_string_new(cw_stiloe_string_t *a_stiloe_string);

void	stiloe_string_delete(cw_stiloe_string_t *a_stiloe_string);
