/* -*-mode:c-*-
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

#ifndef _BHP_H_
#define _BHP_H_

/* Pseudo-opaque type. */
typedef struct cw_bhp_s cw_bhp_t;

typedef struct cw_bhpi_s
{
  void * priority;
  void * data;
  struct cw_bhpi_s * parent;
  struct cw_bhpi_s * child;
  struct cw_bhpi_s * sibling;
  cw_uint32_t degree;
} cw_bhpi_t;

struct cw_bhp_s
{
  cw_bool_t is_malloced;
  cw_bool_t is_thread_safe;
  cw_rwl_t rw_lock;
  cw_bhpi_t * head;
  cw_uint64_t num_nodes;
  cw_sint32_t (*priority_compare)(cw_bhpi_t *, cw_bhpi_t *);
};

#define bhp_new _CW_NS_ANY(bhp_new)
#define bhp_delete _CW_NS_ANY(bhp_delete)
#define bhp_dump _CW_NS_ANY(bhp_dump)
#define bhp_insert _CW_NS_ANY(bhp_insert)
#define bhp_find_min _CW_NS_ANY(bhp_find_min)
#define bhp_del_min _CW_NS_ANY(bhp_del_min)
#define bhp_get_size _CW_NS_ANY(bhp_get_size)
#define bhp_union _CW_NS_ANY(bhp_union)
#define bhp_set_priority_compare _CW_NS_ANY(bhp_set_priority_compare)

/* Typedefs to allow easy function pointer passing. */
typedef cw_sint32_t bhp_prio_comp_t(cw_bhpi_t *, cw_bhpi_t *);

cw_bhp_t * bhp_new(cw_bhp_t * a_bhp_o, cw_bool_t a_is_thread_safe);
void bhp_delete(cw_bhp_t * a_bhp_o);
void bhp_dump(cw_bhp_t * a_bhp_o);
void bhp_insert(cw_bhp_t * a_bhp_o, void * a_priority, void * a_data);
cw_bool_t bhp_find_min(cw_bhp_t * a_bhp_o, void ** a_priority, void ** a_data);
cw_bool_t bhp_del_min(cw_bhp_t * a_bhp_o, void ** a_priority, void ** a_data);
cw_uint64_t bhp_get_size(cw_bhp_t * a_bhp_o);
void bhp_union(cw_bhp_t * a_bhp_o, cw_bhp_t * a_other);
void bhp_set_priority_compare(cw_bhp_t * a_bhp_o,
			      bhp_prio_comp_t * a_new_prio_comp);

#endif /* _BHP_H_ */
