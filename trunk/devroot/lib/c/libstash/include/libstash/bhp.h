/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Header file for bhp class (binomial heap).
 *
 ****************************************************************************/

/* Typedef to allow easy function pointer passing. */
typedef cw_sint32_t bhp_prio_comp_t(void *, void *);

/* Pseudo-opaque type. */
typedef struct cw_bhp_s cw_bhp_t;

/* Opaque type. */
typedef struct cw_bhpi_s cw_bhpi_t;

struct cw_bhp_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_rwl_t rw_lock;
#endif
  cw_bhpi_t * head;
  cw_uint64_t num_nodes;
  bhp_prio_comp_t * priority_compare;
/*    cw_sint32_t (*priority_compare)(cw_bhpi_t *, cw_bhpi_t *); */
};

#define bhp_new _CW_NS_LIBSTASH(bhp_new)
#ifdef _CW_REENTRANT
cw_bhp_t *
bhp_new(cw_bhp_t * a_bhp, cw_bool_t a_is_thread_safe);
#else
cw_bhp_t *
bhp_new(cw_bhp_t * a_bhp);
#endif

#define bhp_delete _CW_NS_LIBSTASH(bhp_delete)
void
bhp_delete(cw_bhp_t * a_bhp);

#define bhp_dump _CW_NS_LIBSTASH(bhp_dump)
void
bhp_dump(cw_bhp_t * a_bhp);

#define bhp_insert _CW_NS_LIBSTASH(bhp_insert)
void
bhp_insert(cw_bhp_t * a_bhp, void * a_priority, void * a_data);

#define bhp_find_min _CW_NS_LIBSTASH(bhp_find_min)
cw_bool_t
bhp_find_min(cw_bhp_t * a_bhp, void ** a_priority, void ** a_data);

#define bhp_del_min _CW_NS_LIBSTASH(bhp_del_min)
cw_bool_t
bhp_del_min(cw_bhp_t * a_bhp, void ** a_priority, void ** a_data);

#define bhp_get_size _CW_NS_LIBSTASH(bhp_get_size)
cw_uint64_t
bhp_get_size(cw_bhp_t * a_bhp);

#define bhp_union _CW_NS_LIBSTASH(bhp_union)
void
bhp_union(cw_bhp_t * a_bhp, cw_bhp_t * a_other);

#define bhp_set_priority_compare _CW_NS_LIBSTASH(bhp_set_priority_compare)
void
bhp_set_priority_compare(cw_bhp_t * a_bhp, bhp_prio_comp_t * a_new_prio_comp);
