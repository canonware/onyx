/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

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
	 * iterating on the child's sibling ring.
	 */
	cw_treen_t	*child;

	cw_ring_t	siblings;	/* Linkage for the sibling ring. */

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
cw_treen_t	*treen_get_parent(cw_treen_t *a_treen);
cw_treen_t	*treen_get_child(cw_treen_t *a_treen);
cw_treen_t	*treen_get_sibling(cw_treen_t *a_treen);
void		*treen_get_data_ptr(cw_treen_t *a_treen);
void		treen_set_data_ptr(cw_treen_t *a_treen, void *a_data);
