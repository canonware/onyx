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
 * <<< Description >>>
 *
 * Dynamic open hashing class.  This is a somewhat sophisticated
 * implementation of hashing.  All internal consistency, growth, shrinkage,
 * etc. issues are taken care of internally.  The hash functions to use, as
 * well as just about any other useful parameter, can be modified on the
 * fly with no worries of inconsistency.  This class is thread safe, thanks
 * to read/write locks.  That is, multiple readers can be in the code
 * simultaneously, but only one locker (with no readers) can be in the code
 * at any given time.
 *
 * This implementation uses a secondary hash function instead of bucket
 * chaining.  As a result, the table needs to be kept a little emptier than
 * some implementations in order to avoid excessive secondary hashing.
 * This seems like a good tradeoff though, since it avoids list management,
 * pointer chasing, and calls to malloc().
 *
 * This code never rehashes during normal operation, because it is careful
 * to shuffle slot contents (items) whenever items are deleted.  This code
 * also keeps an internal list of all items to allow fast table rebuilding
 * when growing, shrinking, and rehashing (done only when hashing functions
 * are changed).  This also makes it possible to use realloc() instead of
 * malloc(), since the table can be bzero()ed, then rebuilt from the list.
 *
 * A useful side effect of the internal list is that calling
 * oh_item_delete_iterate() is guaranteed to remove the oldest item in the 
 * hash table, which means that the hash code has an integrated FIFO
 * queue.
 *
 * A list of spare item containers is kept around to avoid excessive calls
 * to malloc during insertion/deletion.  All internal lists and buffers are
 * kept tidy and at reasonable sizes.
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
  cw_list_item_t * list_item;
} cw_oh_item_t;

struct cw_oh_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_rwl_t rw_lock;
#endif
  cw_oh_item_t ** items;
  cw_list_t items_list;
  cw_list_t spares_list;

  cw_uint64_t (*curr_h1)(cw_oh_t *, const void *);
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
typedef cw_uint64_t oh_h1_t(cw_oh_t *, const void *);
typedef cw_bool_t oh_key_comp_t(const void *, const void *);

/****************************************************************************
 *
 * oh constructor.
 *
 ****************************************************************************/
#ifdef _CW_REENTRANT
cw_oh_t *
oh_new(cw_oh_t * a_oh, cw_bool_t a_is_thread_safe);
#else
cw_oh_t *
oh_new(cw_oh_t * a_oh);
#endif

/****************************************************************************
 *
 * oh destructor.
 *
 ****************************************************************************/
void
oh_delete(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the internal table size.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_size(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the number of items in the hash table.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_items(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the base table size on which the expansion and contraction parameters
 * depend.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_base_size(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the value of the base secondary hashing function.
 *
 ****************************************************************************/
cw_uint32_t
oh_get_base_h2(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the value of the base shrink point, which is used to determine when
 * to shrink the internal table.
 *
 ****************************************************************************/
cw_uint32_t
oh_get_base_shrink_point(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the value of the base grow point, which is used to determine when to
 * grow the internal table.
 *
 ****************************************************************************/
cw_uint32_t
oh_get_base_grow_point(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Sets the primary hashing function.  Calling this function causes an internal
 * rehash.
 *
 ****************************************************************************/
oh_h1_t *
oh_set_h1(cw_oh_t * a_oh, oh_h1_t * a_new_h1);

/****************************************************************************
 *
 * Sets the key comparison function.
 *
 ****************************************************************************/
oh_key_comp_t *
oh_set_key_compare(cw_oh_t * a_oh, oh_key_comp_t * a_new_key_compare);

/****************************************************************************
 *
 * Sets the base secondary hashing function.
 *
 ****************************************************************************/
cw_bool_t
oh_set_base_h2(cw_oh_t * a_oh, cw_uint32_t a_h2);

/****************************************************************************
 *
 * Sets the base shrink point.
 *
 ****************************************************************************/
cw_bool_t
oh_set_base_shrink_point(cw_oh_t * a_oh, cw_uint32_t a_shrink_point);

/****************************************************************************
 *
 * Sets the base grow point.
 *
 ****************************************************************************/
cw_bool_t
oh_set_base_grow_point(cw_oh_t * a_oh, cw_uint32_t a_grow_point);

/****************************************************************************
 *
 * Insert an item, unless an item with the same key already exists.
 *
 ****************************************************************************/
cw_bool_t
oh_item_insert(cw_oh_t * a_oh, const void * a_key, const void * a_data);

/****************************************************************************
 *
 * Delete an item with key a_search_key.  If successful, set *a_key and
 * *a_data to point to the key and data, respectively.
 *
 ****************************************************************************/
cw_bool_t
oh_item_delete(cw_oh_t * a_oh, const void * a_search_key, void ** a_key,
	       void ** a_data);

/****************************************************************************
 *
 * Search for an item with key a_key.  If found, set *a_data to point to
 * the associated data.
 *
 ****************************************************************************/
cw_bool_t
oh_item_search(cw_oh_t * a_oh, const void * a_key, void ** a_data);

/****************************************************************************
 *
 * Searches linearly through the hash table and deletes the first valid
 * item found.
 *
 ****************************************************************************/
cw_bool_t
oh_item_get_iterate(cw_oh_t * a_oh, void ** a_key, void ** a_data);

/****************************************************************************
 *
 * Searches linearly through the hash table and deletes the first valid
 * item found.
 *
 ****************************************************************************/
cw_bool_t
oh_item_delete_iterate(cw_oh_t * a_oh, void ** a_key, void ** a_data);

/****************************************************************************
 *
 * Print the internal state of the hash table.
 *
 ****************************************************************************/
void
oh_dump(cw_oh_t * a_oh, cw_bool_t a_all);

/****************************************************************************
 *
 * Returns the number of collisions that have occurred.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_collisions(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the number of inserts that have been done.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_inserts(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the number of deletes that have been done.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_deletes(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the number of table grows that have happened.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_grows(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Returns the number of table shrinks that have happened.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_shrinks(cw_oh_t * a_oh);
