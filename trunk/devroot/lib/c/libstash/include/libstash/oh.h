/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
typedef struct cw_oh_s cw_oh_t;

typedef struct
{
  cw_uint64_t slot_num;
  cw_uint64_t jumps;
  const void * key;
  const void * data;
  cw_ring_t ring_item;
} cw_oh_item_t;

struct cw_oh_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_rwl_t rw_lock;
#endif
  cw_oh_item_t ** items;
  cw_ring_t * items_ring;
  cw_uint64_t items_count;
  cw_ring_t * spares_ring;
  cw_uint64_t spares_count;

  cw_uint64_t (*curr_h1)(const void *);
  cw_bool_t (*key_compare)(const void *, const void *);

  cw_uint64_t size;

  cw_uint32_t base_power;
  cw_uint32_t base_h2;
  cw_uint32_t base_shrink_point;
  cw_uint32_t base_grow_point;

  cw_uint32_t curr_power;
  cw_uint64_t curr_h2;
  cw_uint64_t curr_shrink_point;
  cw_uint64_t curr_grow_point;

  /* Counters used to get an idea of performance. */
  cw_uint64_t num_collisions;
  cw_uint64_t num_inserts;
  cw_uint64_t num_deletes;
  cw_uint64_t num_grows;
  cw_uint64_t num_shrinks;
};

/* Typedefs to allow easy function pointer passing. */
typedef cw_uint64_t oh_h1_t(const void *);
typedef cw_bool_t oh_key_comp_t(const void *, const void *);

cw_oh_t *
oh_new(cw_oh_t * a_oh);

cw_oh_t *
oh_new_r(cw_oh_t * a_oh);

void
oh_delete(cw_oh_t * a_oh);

cw_uint64_t
oh_get_size(cw_oh_t * a_oh);

cw_uint64_t
oh_get_num_items(cw_oh_t * a_oh);

cw_uint64_t
oh_get_base_size(cw_oh_t * a_oh);

cw_uint32_t
oh_get_base_h2(cw_oh_t * a_oh);

cw_uint32_t
oh_get_base_shrink_point(cw_oh_t * a_oh);

cw_uint32_t
oh_get_base_grow_point(cw_oh_t * a_oh);

oh_h1_t *
oh_set_h1(cw_oh_t * a_oh, oh_h1_t * a_new_h1);

oh_key_comp_t *
oh_set_key_compare(cw_oh_t * a_oh, oh_key_comp_t * a_new_key_compare);

cw_bool_t
oh_set_base_h2(cw_oh_t * a_oh, cw_uint32_t a_h2);

cw_bool_t
oh_set_base_shrink_point(cw_oh_t * a_oh, cw_uint32_t a_shrink_point);

cw_bool_t
oh_set_base_grow_point(cw_oh_t * a_oh, cw_uint32_t a_grow_point);

cw_sint32_t
oh_item_insert(cw_oh_t * a_oh, const void * a_key, const void * a_data);

cw_bool_t
oh_item_delete(cw_oh_t * a_oh, const void * a_search_key, void ** r_key,
	       void ** r_data);

cw_bool_t
oh_item_search(cw_oh_t * a_oh, const void * a_key, void ** r_data);

cw_bool_t
oh_item_get_iterate(cw_oh_t * a_oh, void ** r_key, void ** r_data);

cw_bool_t
oh_item_delete_iterate(cw_oh_t * a_oh, void ** r_key, void ** r_data);

void
oh_dump(cw_oh_t * a_oh, cw_bool_t a_all);

cw_uint64_t
oh_get_num_collisions(cw_oh_t * a_oh);

cw_uint64_t
oh_get_num_inserts(cw_oh_t * a_oh);

cw_uint64_t
oh_get_num_deletes(cw_oh_t * a_oh);

cw_uint64_t
oh_get_num_grows(cw_oh_t * a_oh);

cw_uint64_t
oh_get_num_shrinks(cw_oh_t * a_oh);

cw_uint64_t
oh_h1_string(const void * a_key);

cw_uint64_t
oh_h1_direct(const void * a_key);

cw_bool_t
oh_key_compare_string(const void * a_k1, const void * a_k2);

cw_bool_t
oh_key_compare_direct(const void * a_k1, const void * a_k2);
