/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Public interface for the bhp (binomial heap) class, and its helper, the bhpi
 * class.
 *
 ******************************************************************************/

/* Typedef to allow easy function pointer passing. */
typedef cw_sint32_t bhp_prio_comp_t (const void *, const void *);

/* Pseudo-opaque types. */
typedef struct cw_bhp_s cw_bhp_t;
typedef struct cw_bhpi_s cw_bhpi_t;

struct cw_bhp_s {
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t	magic_a;
#endif
	cw_mem_t	*mem;
	cw_bool_t	is_malloced;
	cw_bool_t	is_thread_safe;
	cw_mtx_t	lock;
	cw_bhpi_t	*head;
	cw_uint64_t	num_nodes;
	bhp_prio_comp_t	*priority_compare;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t	size_of;
	cw_uint32_t	magic_b;
#endif
};

struct cw_bhpi_s {
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t magic_a;
#endif
	cw_opaque_dealloc_t *dealloc_func;
	void		*dealloc_arg;

        const void	*priority;
        const void	*data;
	struct cw_bhpi_s *parent;
	struct cw_bhpi_s *child;
	struct cw_bhpi_s *sibling;
	cw_uint32_t	degree;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t	size_of;
	cw_uint32_t	magic_b;
#endif
};

cw_bhpi_t	*bhpi_new(cw_bhpi_t *a_bhpi, cw_mem_t *a_mem, const void
    *a_priority, const void *a_data, cw_opaque_dealloc_t *a_dealloc_func, void
    *a_dealloc_arg);

void		bhpi_delete(cw_bhpi_t *a_bhpi);

cw_bhp_t	*bhp_new(cw_bhp_t *a_bhp, cw_mem_t *a_mem, bhp_prio_comp_t
    *a_prio_comp);
cw_bhp_t	*bhp_new_r(cw_bhp_t *a_bhp, cw_mem_t *a_mem, bhp_prio_comp_t
    *a_prio_comp);
void		bhp_delete(cw_bhp_t *a_bhp);

void		bhp_dump(cw_bhp_t *a_bhp);

void		bhp_insert(cw_bhp_t *a_bhp, cw_bhpi_t *a_bhpi);
cw_bool_t	bhp_min_find(cw_bhp_t *a_bhp, void **r_priority, void **r_data);
cw_bool_t	bhp_min_del(cw_bhp_t *a_bhp, void **r_priority, void **r_data);

cw_uint64_t	bhp_size_get(cw_bhp_t *a_bhp);

void		bhp_union(cw_bhp_t *a_a, cw_bhp_t *a_b);

cw_sint32_t	bhp_uint32_priority_compare(const void *a_a, const void *a_b);
cw_sint32_t	bhp_sint32_priority_compare(const void *a_a, const void *a_b);
cw_sint32_t	bhp_uint64_priority_compare(const void *a_a, const void *a_b);
