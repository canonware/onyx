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

#define oh_p_grow _CW_NS_STASH(oh_p_grow)
void
oh_p_grow(cw_oh_t * a_oh);

#define oh_p_shrink _CW_NS_STASH(oh_p_shrink)
void
oh_p_shrink(cw_oh_t * a_oh);

#define oh_p_h1 _CW_NS_STASH(oh_p_h1)
cw_uint64_t
oh_p_h1(cw_oh_t * a_oh, void * a_key);

#define oh_p_key_compare _CW_NS_STASH(oh_p_key_compare)
cw_bool_t
oh_p_key_compare(void * a_k1, void * a_k2);

#define oh_p_item_insert _CW_NS_STASH(oh_p_item_insert)
void
oh_p_item_insert(cw_oh_t * a_oh, cw_oh_item_t * a_item);

#define oh_p_item_search _CW_NS_STASH(oh_p_item_search)
cw_bool_t
oh_p_item_search(cw_oh_t * a_oh, void * a_key, cw_uint64_t * a_slot);

#define oh_p_rehash _CW_NS_STASH(oh_p_rehash)
void
oh_p_rehash(cw_oh_t * a_oh);

#define oh_p_slot_shuffle _CW_NS_STASH(oh_p_slot_shuffle)
void
oh_p_slot_shuffle(cw_oh_t * a_oh, cw_uint64_t a_slot);
