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

/* Defined in stilo.h to resolve a circular dependency. */
#if (0)
typedef struct cw_stiloe_array_s cw_stiloe_array_t;
#endif

struct cw_stiloe_array_s {
	cw_stiloe_t	stiloe;
};

cw_stiloe_array_t	*stiloe_array_new(cw_stiloe_array_t *a_stiloe_array);
void			stiloe_array_delete(cw_stiloe_array_t *a_stiloe_array);
