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
 * Private definitions for oh.
 *
 ****************************************************************************/

/* 2^(_OH_BASE_POWER) is the base table size used for calculations. */
#define _OH_BASE_POWER 8;
#define _OH_BASE_H2 7;

/* Minimum number of items allowable before shrinking. */
#define _OH_BASE_SHRINK_POINT 64;

/* Maximum number of items allowable before growing. */
#define _OH_BASE_GROW_POINT 192;

/****************************************************************************
 *
 * Default primary hash function.  This is a string hash, so if the keys
 * being used for an oh instance aren't strings, don't use this.
 *
 ****************************************************************************/
#define oh_p_h1 _CW_NS_STASH(oh_p_h1)
cw_uint64_t
oh_p_h1(cw_oh_t * a_oh, void * a_key);

/****************************************************************************
 *
 * Compares two keys (strings) for equality.  A return value of TRUE means the
 * arguments are equal.
 *
 ****************************************************************************/
#define oh_p_key_compare _CW_NS_STASH(oh_p_key_compare)
cw_bool_t
oh_p_key_compare(void * a_k1, void * a_k2);

/****************************************************************************
 *
 * If the table is too full, double in size and insert into the new table.
 *
 ****************************************************************************/
#define oh_p_grow _CW_NS_STASH(oh_p_grow)
void
oh_p_grow(cw_oh_t * a_oh);

/****************************************************************************
 *
 * If the table is too empty, shrink it as small as possible, without
 * making it so small that the table would need to immediately grow again.
 *
 ****************************************************************************/
#define oh_p_shrink _CW_NS_STASH(oh_p_shrink)
void
oh_p_shrink(cw_oh_t * a_oh);

/****************************************************************************
 *
 * Find the slot that we should insert into, given a_item, and insert.
 *
 ****************************************************************************/
#define oh_p_item_insert _CW_NS_STASH(oh_p_item_insert)
void
oh_p_item_insert(cw_oh_t * a_oh, cw_oh_item_t * a_item);

/****************************************************************************
 *
 * Uses the primary and secondary hash function to search for an item with
 * key == a_key.
 *
 ****************************************************************************/
#define oh_p_item_search _CW_NS_STASH(oh_p_item_search)
cw_bool_t
oh_p_item_search(cw_oh_t * a_oh, void * a_key, cw_uint64_t * a_slot);

/****************************************************************************
 *
 * Rehash.
 *
 ****************************************************************************/
#define oh_p_rehash _CW_NS_STASH(oh_p_rehash)
void
oh_p_rehash(cw_oh_t * a_oh);

/****************************************************************************
 *
 *  Figure out whether there are any items that bounced past this slot
 *  using the secondary hash.  If so, shuffle things backward to fill this
 *  slot in.  We know we've looked far enough forward when we hit an empty
 *  slot.
 *
 ****************************************************************************/
#define oh_p_slot_shuffle _CW_NS_STASH(oh_p_slot_shuffle)
void
oh_p_slot_shuffle(cw_oh_t * a_oh, cw_uint64_t a_slot);
