/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 112 $
 * $Date: 1998-06-30 15:56:19 -0700 (Tue, 30 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _OH_H_
#define _OH_H_

#define _OH_PERF_

/* Pseudo-opaque type. */
typedef struct cw_oh_s cw_oh_t;

typedef struct
{
  cw_uint64_t slot_num;
  cw_uint64_t jumps;
  void * key;
  void * data;
  cw_list_item_t * list_item;
} cw_oh_item_t;

struct cw_oh_s
{
  cw_bool_t is_malloced;
  cw_bool_t is_thread_safe;
  cw_rwl_t rw_lock;
  cw_oh_item_t ** items;
  cw_list_t items_list;
  cw_list_t spares_list;

  cw_uint64_t (*curr_h1)(cw_oh_t *, void *);
  cw_bool_t (*key_compare)(void *, void *);

  cw_uint64_t size;

  cw_uint32_t base_power;
  cw_uint32_t base_h2;
  cw_uint32_t base_shrink_point;
  cw_uint32_t base_grow_point;

  cw_uint32_t curr_power;
  cw_uint64_t curr_h2;
  cw_uint64_t curr_shrink_point;
  cw_uint64_t curr_grow_point;

#ifdef _OH_PERF_  
  /* Counters used to get an idea of performance. */
  cw_uint64_t num_collisions;
  cw_uint64_t num_inserts;
  cw_uint64_t num_deletes;
  cw_uint64_t num_grows;
  cw_uint64_t num_shrinks;
#endif
};

#define oh_new _CW_NS_CMN(oh_new)
#define oh_delete _CW_NS_CMN(oh_delete)
#define oh_get_size _CW_NS_CMN(oh_get_size)
#define oh_get_num_items _CW_NS_CMN(oh_get_num_items)

#define oh_get_base_size _CW_NS_CMN(oh_get_base_size)
#define oh_get_base_h2 _CW_NS_CMN(oh_get_base_h2)
#define oh_get_base_shrink_point _CW_NS_CMN(oh_get_base_shrink_point)
#define oh_get_base_grow_point _CW_NS_CMN(oh_get_base_grow_point)

#define oh_set_h1 _CW_NS_CMN(oh_set_h1)
#define oh_set_key_compare _CW_NS_CMN(oh_set_key_compare)
#define oh_set_base_h2 _CW_NS_CMN(oh_set_base_h2)
#define oh_set_base_shrink_point _CW_NS_CMN(oh_set_base_shrink_point)
#define oh_set_base_grow_point _CW_NS_CMN(oh_set_base_grow_point)

#define oh_item_insert _CW_NS_CMN(oh_item_insert)
#define oh_item_delete _CW_NS_CMN(oh_item_delete)
#define oh_item_search _CW_NS_CMN(oh_item_search)

#define oh_item_delete_iterate _CW_NS_CMN(oh_item_delete_iterate)
#define oh_dump _CW_NS_CMN(oh_dump)

#ifdef _OH_PERF_
#  define oh_get_num_collisions _CW_NS_CMN(oh_get_num_collisions)
#  define oh_get_num_inserts _CW_NS_CMN(oh_get_num_inserts)
#  define oh_get_num_deletes _CW_NS_CMN(oh_get_num_deletes)
#  define oh_get_num_grows _CW_NS_CMN(oh_get_num_grows)
#  define oh_get_num_shrinks _CW_NS_CMN(oh_get_num_shrinks)
#endif

/* Typedefs to allow easy function pointer passing. */
typedef cw_uint64_t oh_h1_t(cw_oh_t *, void *);
typedef cw_bool_t oh_key_comp_t(void *, void *);

cw_oh_t * oh_new(cw_oh_t * a_oh_o, cw_bool_t a_is_thread_safe);
void oh_delete(cw_oh_t * a_oh_o);
cw_uint64_t oh_get_size(cw_oh_t * a_oh_o);
cw_uint64_t oh_get_num_items(cw_oh_t * a_oh_o);

cw_uint64_t oh_get_base_size(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_base_h2(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_base_shrink_point(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_base_grow_point(cw_oh_t * a_oh_o);

oh_h1_t * oh_set_h1(cw_oh_t * a_oh_o,
		    oh_h1_t * a_new_h1);
oh_key_comp_t * oh_set_key_compare(cw_oh_t * a_oh_o,
				   oh_key_comp_t * a_new_key_compare);
cw_bool_t oh_set_base_h2(cw_oh_t * a_oh_o,
			 cw_uint32_t a_h2);
cw_bool_t oh_set_base_shrink_point(cw_oh_t * a_oh_o,
				   cw_uint32_t a_shrink_point);
cw_bool_t oh_set_base_grow_point(cw_oh_t * a_oh_o,
				 cw_uint32_t a_grow_point);

cw_bool_t oh_item_insert(cw_oh_t * a_oh_o, void * a_key,
			 void * a_data);
cw_bool_t oh_item_delete(cw_oh_t * a_oh_o, void * a_search_key, void ** a_key,
			 void ** a_data);
cw_bool_t oh_item_search(cw_oh_t * a_oh_o, void * a_key,
			 void ** a_data);

cw_bool_t oh_item_delete_iterate(cw_oh_t * a_oh_o, void ** a_key,
				 void ** a_data);
void oh_dump(cw_oh_t * a_oh_o, cw_bool_t a_all);

#ifdef _OH_PERF_
cw_uint64_t oh_get_num_collisions(cw_oh_t * a_oh_o);
cw_uint64_t oh_get_num_inserts(cw_oh_t * a_oh_o);
cw_uint64_t oh_get_num_deletes(cw_oh_t * a_oh_o);
cw_uint64_t oh_get_num_grows(cw_oh_t * a_oh_o);
cw_uint64_t oh_get_num_shrinks(cw_oh_t * a_oh_o);
#endif

#endif /* _OH_H_ */
