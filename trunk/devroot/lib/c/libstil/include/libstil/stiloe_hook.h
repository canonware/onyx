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
