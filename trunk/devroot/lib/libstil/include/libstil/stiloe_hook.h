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
typedef struct cw_stiloe_hook_s cw_stiloe_hook_t;

#endif

struct cw_stiloe_hook_s {
	cw_stiloe_t	stiloe;
	void		*data;
	void		(*dealloc_func) (void *);
	cw_stilo_t	*(*ref_iterator) (void *);
};

cw_stiloe_hook_t *stiloe_hook_new(cw_stiloe_hook_t *a_stiloe_hook);

void	stiloe_hook_ref(cw_stiloe_hook_t *a_stiloe_hook);

void	stiloe_hook_unref(cw_stiloe_hook_t *a_stiloe_hook);
