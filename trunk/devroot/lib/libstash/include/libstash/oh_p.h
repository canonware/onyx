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
 * Private declarations for oh.
 *
 ****************************************************************************/

/* 2^(_OH_BASE_POWER) is the base table size used for calculations. */
#define _OH_BASE_POWER 8;
#define _OH_BASE_H2 7;

/* Minimum number of items allowable before shrinking. */
#define _OH_BASE_SHRINK_POINT 64;

/* Maximum number of items allowable before growing. */
#define _OH_BASE_GROW_POINT 192;

static cw_oh_t *oh_p_new(cw_oh_t *a_oh, cw_bool_t a_is_thread_safe);

/****************************************************************************
 *
 * If the table is too full, double in size and insert into the new table.
 *
 ****************************************************************************/
static cw_bool_t oh_p_grow(cw_oh_t *a_oh);

/****************************************************************************
 *
 * If the table is too empty, shrink it as small as possible, without
 * making it so small that the table would need to immediately grow again.
 *
 ****************************************************************************/
static void oh_p_shrink(cw_oh_t *a_oh);

/****************************************************************************
 *
 * Find the slot that we should insert into, given a_item, and insert.
 *
 ****************************************************************************/
static void oh_p_item_insert(cw_oh_t *a_oh, cw_oh_item_t * a_item);

/****************************************************************************
 *
 * Uses the primary and secondary hash function to search for an item with
 * key == a_key.
 *
 ****************************************************************************/
static cw_bool_t oh_p_item_search(cw_oh_t *a_oh, const void *a_key, cw_uint64_t
    *a_slot);

/****************************************************************************
 *
 * Rehash.
 *
 ****************************************************************************/
static void oh_p_rehash(cw_oh_t *a_oh);

/****************************************************************************
 *
 * Figure out whether there are any items that bounced past this slot
 * using the secondary hash.  If so, shuffle things backward to fill this
 * slot in.  We know we've looked far enough forward when we hit an empty
 * slot.
 *
 ****************************************************************************/
static void oh_p_slot_shuffle(cw_oh_t *a_oh, cw_uint64_t a_slot);
