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
 * Dynamic open hashing class.  This is a somewhat sophisticated implementation
 * of hashing.  All internal consistency, growth, shrinkage, etc. issues are
 * taken care of internally.  The hash functions to use, as well as just about
 * any other useful parameter, can be modified on the fly with no worries of
 * inconsistency.  This class is thread safe, thanks to read/write locks.  That
 * is, multiple readers can be in the code simultaneously, but only one locker
 * (with no readers) can be in the code at any given time.
 *
 * This implementation uses a secondary hash function instead of bucket
 * chaining.  As a result, the table needs to be kept a little emptier than some
 * implementations in order to avoid excessive secondary hashing.  This seems
 * like a good tradeoff though, since it avoids list management, pointer
 * chasing, and calls to malloc().
 *
 * This code never rehashes during normal operation, because it is careful to
 * shuffle slot contents (items) whenever items are deleted.  This code also
 * keeps an internal list of all items to allow fast table rebuilding when
 * growing, shrinking, and rehashing (done only when hashing functions are
 * changed).  This also makes it possible to use realloc() instead of malloc(),
 * since the table can be bzero()ed, then rebuilt from the list.
 *
 * A useful side effect of the internal list is that calling
 * oh_item_delete_iterate() and oh_item_get_iterate() are guaranteed to operate
 * on the oldest item in the hash table, which means that the hash code has an
 * integrated FIFO queue.
 *
 * A list of spare item containers is kept around to avoid excessive calls to
 * malloc() during insertion/deletion.  All internal lists and buffers are kept
 * tidy and at reasonable sizes.
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
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to space for an oh, or NULL.
 *
 * a_is_thread_safe : FALSE == not thread-safe, TRUE == thread-safe.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to an oh, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
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
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
oh_delete(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Internal table size.
 *
 * <<< Description >>>
 *
 * Return the internal table size.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_size(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of items in a_oh.
 *
 * <<< Description >>>
 *
 * Return the number of items in a_oh.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_items(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Base table size.
 *
 * <<< Description >>>
 *
 * Return the base table size on which the expansion and contraction parameters
 * depend.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_base_size(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Base secondary hash.
 *
 * <<< Description >>>
 *
 * Return the value of the base secondary hashing function.
 *
 ****************************************************************************/
cw_uint32_t
oh_get_base_h2(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Base shrink point.
 *
 * <<< Description >>>
 *
 * Return the value of the base shrink point, which is used to determine when to
 * shrink the internal table.
 *
 ****************************************************************************/
cw_uint32_t
oh_get_base_shrink_point(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Base grow point.
 *
 * <<< Description >>>
 *
 * Return the value of the base grow point, which is used to determine when to
 * grow the internal table.
 *
 ****************************************************************************/
cw_uint32_t
oh_get_base_grow_point(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_new_h1 : Pointer to a primary hashing function.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to the old primary hashing function.
 *
 * <<< Description >>>
 *
 * Set the primary hashing function.  Calling this function causes an internal
 * rehash.
 *
 ****************************************************************************/
oh_h1_t *
oh_set_h1(cw_oh_t * a_oh, oh_h1_t * a_new_h1);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_new_key_compare : Pointer to a key comparison function.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to the old key comparison function.
 *
 * <<< Description >>>
 *
 * Set the key comparison function.
 *
 ****************************************************************************/
oh_key_comp_t *
oh_set_key_compare(cw_oh_t * a_oh, oh_key_comp_t * a_new_key_compare);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_h2 : Secondary hash.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : a_h2 is not an odd number less than a_oh's base table size.
 *
 * <<< Description >>>
 *
 * Set the base secondary hash.
 *
 ****************************************************************************/
cw_bool_t
oh_set_base_h2(cw_oh_t * a_oh, cw_uint32_t a_h2);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_shrink_point : Base shrink point.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : a_shrink_point is > the base grow point or table size.
 *
 * <<< Description >>>
 *
 * Set the base shrink point.
 *
 ****************************************************************************/
cw_bool_t
oh_set_base_shrink_point(cw_oh_t * a_oh, cw_uint32_t a_shrink_point);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_grow_point : Base grow point.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : a_grow_point is <= the base shrink point or >= the base table
 *                 size.
 *
 * <<< Description >>>
 *
 * Set the base grow point.
 *
 ****************************************************************************/
cw_bool_t
oh_set_base_grow_point(cw_oh_t * a_oh, cw_uint32_t a_grow_point);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_key : Pointer to a key.
 *
 * a_data : Pointer to data associated with a_key.
 *
 * <<< Output(s) >>>
 *
 * retval : -1 : Memory allocation error.
 *           0 : Success.
 *           1 : a_key already exists in a_oh.
 *
 * <<< Description >>>
 *
 * Insert an item, unless an item with the same key already exists in a_oh.
 *
 ****************************************************************************/
cw_sint32_t
oh_item_insert(cw_oh_t * a_oh, const void * a_key, const void * a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_search_key : Pointer to a key.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Item with a_search_key not found.
 *
 * *r_key : Pointer to a key, or invalid if retval == TRUE.
 *
 * *r_data : Pointer to data associated with *r_key, or invalid if retval ==
 *           TRUE.
 *
 * <<< Description >>>
 *
 * Delete an item with key a_search_key.  If successful, set *r_key and
 * *r_data to point to the key and data, respectively.
 *
 ****************************************************************************/
cw_bool_t
oh_item_delete(cw_oh_t * a_oh, const void * a_search_key, void ** r_key,
	       void ** r_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_key : Pointer to a key.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Item with key a_key not found in a_oh.
 *
 * *r_data : Data associated with a_key, or invalid if retval == TRUE.
 *
 * <<< Description >>>
 *
 * Search for an item with key a_key.  If found, set *r_data to point to
 * the associated data.
 *
 ****************************************************************************/
cw_bool_t
oh_item_search(cw_oh_t * a_oh, const void * a_key, void ** r_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : a_oh is empty.
 *
 * *r_key : Pointer to a key, or invalid if retval == TRUE.
 *
 * *r_data : Pointer to data associated with *r_key, or invalid if retval ==
 *           TRUE.
 *
 * <<< Description >>>
 *
 * Set *r_key and *r_data to point to the oldest item in a_oh.
 *
 ****************************************************************************/
cw_bool_t
oh_item_get_iterate(cw_oh_t * a_oh, void ** r_key, void ** r_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : a_oh is empty.
 *
 * *r_key : Pointer to a key, or invalid if retval == TRUE.
 *
 * *r_data : Pointer to data associated with *r_key, or invalid if retval ==
 *           TRUE.
 *
 * <<< Description >>>
 *
 * Set *r_key and *r_data to point to the oldest item in a_oh, and delete the
 * item from a_oh.
 *
 ****************************************************************************/
cw_bool_t
oh_item_delete_iterate(cw_oh_t * a_oh, void ** a_key, void ** a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_all : If TRUE, dump the contents of the hash table, otherwise, just dump
 *         the metadata.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Print the internal state of the hash table.
 *
 ****************************************************************************/
void
oh_dump(cw_oh_t * a_oh, cw_bool_t a_all);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of collisions.
 *
 * <<< Description >>>
 *
 * Return the number of collisions that have occurred.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_collisions(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of insertions.
 *
 * <<< Description >>>
 *
 * Return the number of insertions.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_inserts(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of deletions.
 *
 * <<< Description >>>
 *
 * Return the number of deletions.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_deletes(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of table grows.
 *
 * <<< Description >>>
 *
 * Return the number of table grows.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_grows(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of table shrinks.
 *
 * <<< Description >>>
 *
 * Return the number of table shrinks.
 *
 ****************************************************************************/
cw_uint64_t
oh_get_num_shrinks(cw_oh_t * a_oh);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_key : Pointer to a key.
 *
 * <<< Output(s) >>>
 *
 * retval : Hash result.
 *
 * <<< Description >>>
 *
 * Default primary hash function.  This is a string hash, so if the keys
 * being used for an oh instance aren't strings, don't use this.
 *
 ****************************************************************************/
cw_uint64_t
oh_h1_string(cw_oh_t * a_oh, const void * a_key);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_oh : Pointer to an oh.
 *
 * a_key : Pointer to a key.
 *
 * <<< Output(s) >>>
 *
 * retval : Hash result.
 *
 * <<< Description >>>
 *
 * Alternate primary hash function.  This is a direct pointer hash.
 *
 ****************************************************************************/
cw_uint64_t
oh_h1_direct(cw_oh_t * a_oh, const void * a_key);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_k1 : Pointer to a key.
 *
 * a_k2 : Pointer to a key.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == not equal, TRUE == equal.
 *
 * <<< Description >>>
 *
 * Compare two keys (strings) for equality.
 *
 ****************************************************************************/
cw_bool_t
oh_key_compare_string(const void * a_k1, const void * a_k2);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_k1 : Pointer to a key.
 *
 * a_k2 : Pointer to a key.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == not equal, TRUE == equal.
 *
 * <<< Description >>>
 *
 * Compare two keys (pointers) for equality.
 *
 ****************************************************************************/
cw_bool_t
oh_key_compare_direct(const void * a_k1, const void * a_k2);
