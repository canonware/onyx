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
typedef struct cw_stiloe_mstate_s cw_stiloe_mstate_t;

#endif

struct cw_stiloe_mstate_s {
	cw_stiloe_t	stiloe;
	cw_uint32_t	accuracy;
	cw_uint32_t	point;
	cw_uint32_t	base;
};

cw_stiloe_mstate_t	*stiloe_mstate_new(cw_stiloe_mstate_t *a_stiloe_mstate);
void			stiloe_mstate_delete(cw_stiloe_mstate_t
    *a_stiloe_mstate);
