/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_treen_s cw_treen_t;

struct cw_treen_s {
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t	magic_a;
#endif

	/* Automatic deallocation hooks. */
	cw_opaque_dealloc_t *dealloc_func;
	void		*dealloc_arg;

	cw_treen_t	*parent;	/* Pointer to the parent. */

	/*
	 * Pointer to one child.  Getting to other children is achieved by
	 * iterating on the child's sibling qr.
	 */
	cw_treen_t	*child;

	qr(cw_treen_t) sib_link;	/* Linkage for the sibling ring. */

	void		*data;		/* The payload. */

#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t	size_of;
	cw_uint32_t	magic_b;
#endif
};

cw_treen_t	*treen_new(cw_treen_t *a_treen, cw_mem_t *a_mem,
    cw_opaque_dealloc_t *a_dealloc_func, void *a_dealloc_arg);
void		treen_delete(cw_treen_t *a_treen);
void		treen_link(cw_treen_t *a_treen, cw_treen_t *a_parent);
cw_treen_t	*treen_parent_get(cw_treen_t *a_treen);
cw_treen_t	*treen_child_get(cw_treen_t *a_treen);
cw_treen_t	*treen_sibling_get(cw_treen_t *a_treen);
void		*treen_data_ptr_get(cw_treen_t *a_treen);
void		treen_data_ptr_set(cw_treen_t *a_treen, void *a_data);
