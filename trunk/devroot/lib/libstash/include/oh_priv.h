/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 125 $
 * $Date: 1998-07-02 16:55:52 -0700 (Thu, 02 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Private definitions for oh.
 *
 ****************************************************************************/

#ifndef _OH_PRIV_H_
#define _OH_PRIV_H_

#define oh_p_grow _CW_NS_CMN(oh_p_grow)
#define oh_p_shrink _CW_NS_CMN(oh_p_shrink)
#define oh_p_h1 _CW_NS_CMN(oh_p_h1)
#define oh_p_insert _CW_NS_CMN(oh_p_insert)
#define oh_p_search _CW_NS_CMN(oh_p_search)
#define oh_p_rehash _CW_NS_CMN(oh_p_rehash)
#define oh_p_slot_shuffle _CW_NS_CMN(oh_p_slot_shuffle)

/* 2^(_OH_BASE_POWER) is the base table size used for calculations. */
#define _OH_BASE_POWER 8;
#define _OH_BASE_H2 7;

/* Minimum number of items allowable before shrinking. */
#define _OH_BASE_SHRINK_POINT 64;

/* Maximum number of items allowable before growing. */
#define _OH_BASE_GROW_POINT 230;

void oh_p_grow(cw_oh_t * a_oh_o);
void oh_p_shrink(cw_oh_t * a_oh_o);

cw_uint64_t oh_p_h1(cw_oh_t * a_oh_o, void * a_key);
cw_bool_t oh_p_key_compare(void * a_k1, void * a_k2);
void oh_p_item_insert(cw_oh_t * a_oh_o,
			   cw_oh_item_t * a_item);
cw_bool_t oh_p_item_search(cw_oh_t * a_oh_o,
			   void * a_key,
			   cw_uint64_t * a_slot);
void oh_p_rehash(cw_oh_t * a_oh_o);
void oh_p_slot_shuffle(cw_oh_t * a_oh_o, cw_uint64_t a_slot);

#endif /* _OH_PRIV_H_ */
