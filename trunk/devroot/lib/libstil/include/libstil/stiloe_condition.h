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
typedef struct cw_stiloe_condition_s cw_stiloe_condition_t;

#endif

struct cw_stiloe_condition_s {
	cw_stiloe_t stiloe;
	cw_cnd_t condition;
};

cw_stiloe_condition_t *stiloe_condition_new(cw_stiloe_condition_t
    *a_stiloe_condition);

void	stiloe_condition_ref(cw_stiloe_condition_t *a_stiloe_condition);

void	stiloe_condition_unref(cw_stiloe_condition_t *a_stiloe_condition);
