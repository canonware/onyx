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
typedef struct cw_stiloe_lock_s cw_stiloe_lock_t;

#endif

struct cw_stiloe_lock_s {
	cw_stiloe_t stiloe;
	cw_mtx_t lock;
};

cw_stiloe_lock_t	*stiloe_lock_new(cw_stiloe_lock_t *a_stiloe_lock);

void			stiloe_lock_delete(cw_stiloe_lock_t *a_stiloe_lock);
