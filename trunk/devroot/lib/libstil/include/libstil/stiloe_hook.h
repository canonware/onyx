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
	void		(*exec) (void *);
	void		(*dealloc) (void *);
	cw_stiloe_t	*(*ref_iterator) (void *);
};

cw_stiloe_hook_t *stiloe_hook_new(cw_stiloe_hook_t *a_stiloe_hook, void *a_data,
    void (*a_exec)(void *), void (*a_dealloc)(void *), cw_stiloe_t
    *(*ref_iterator)(void *));

void		stiloe_hook_delete(cw_stiloe_hook_t *a_stiloe_hook);
