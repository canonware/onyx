/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Public interface for the bhp (binomial heap) class, and its helper, the bhpi
 * class.
 *
 ****************************************************************************/

/* Typedef to allow easy function pointer passing. */
typedef cw_sint32_t bhp_prio_comp_t (const void *, const void *);

/* Pseudo-opaque types. */
typedef struct cw_bhp_s cw_bhp_t;
typedef struct cw_bhpi_s cw_bhpi_t;

struct cw_bhp_s {
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t magic_a;
#endif
	cw_bool_t is_malloced;
	cw_bool_t is_thread_safe;
	cw_mtx_t lock;
	cw_bhpi_t *head;
	cw_uint64_t num_nodes;
	bhp_prio_comp_t *priority_compare;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t size_of;
	cw_uint32_t magic_b;
#endif
};

struct cw_bhpi_s {
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t magic_a;
#endif
	void    (*dealloc_func) (void *, void *);
	void   *dealloc_arg;

        const void *priority;
        const void *data;
	struct cw_bhpi_s *parent;
	struct cw_bhpi_s *child;
	struct cw_bhpi_s *sibling;
	cw_uint32_t degree;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t size_of;
	cw_uint32_t magic_b;
#endif
};

cw_bhpi_t *bhpi_new(cw_bhpi_t *a_bhpi, const void *a_priority, const void
    *a_data, void (*a_dealloc_func) (void *dealloc_arg, void *bhpi), void
    *a_dealloc_arg);

void    bhpi_delete(cw_bhpi_t *a_bhpi);

cw_bhp_t *bhp_new(cw_bhp_t *a_bhp, bhp_prio_comp_t *a_prio_comp);

cw_bhp_t *bhp_new_r(cw_bhp_t *a_bhp, bhp_prio_comp_t *a_prio_comp);

void    bhp_delete(cw_bhp_t *a_bhp);

void    bhp_dump(cw_bhp_t *a_bhp);

void    bhp_insert(cw_bhp_t *a_bhp, cw_bhpi_t *a_bhpi);

cw_bool_t bhp_find_min(cw_bhp_t *a_bhp, void **r_priority, void **r_data);

cw_bool_t bhp_del_min(cw_bhp_t *a_bhp, void **r_priority, void **r_data);

cw_uint64_t bhp_get_size(cw_bhp_t *a_bhp);

void    bhp_union(cw_bhp_t *a_a, cw_bhp_t *a_b);

cw_sint32_t bhp_priority_compare_uint32(const void *a_a, const void *a_b);

cw_sint32_t bhp_priority_compare_sint32(const void *a_a, const void *a_b);

cw_sint32_t bhp_priority_compare_uint64(const void *a_a, const void *a_b);
