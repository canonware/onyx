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
typedef struct cw_stiloe_packedarray_s cw_stiloe_packedarray_t;
#endif

struct cw_stiloe_packedarray_s {
	cw_stiloe_t	stiloe;
};

cw_stiloe_packedarray_t	*stiloe_packedarray_new(cw_stiloe_packedarray_t
    *a_stiloe_packedarray);

void			stiloe_packedarray_delete(cw_stiloe_packedarray_t
    *a_stiloe_packedarray);
