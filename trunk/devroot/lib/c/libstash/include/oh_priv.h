/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 90 $
 * $Date: 1998-06-24 23:45:26 -0700 (Wed, 24 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _OH_PRIV_H_
#define _OH_PRIV_H_

#define oh_p_quiesce _CW_NS_CMN(oh_p_quiesce)
#define oh_p_grow _CW_NS_CMN(oh_p_grow)
#define oh_p_shrink _CW_NS_CMN(oh_p_shrink)
#define oh_p_nh1 _CW_NS_CMN(oh_p_h1)
#define oh_p_insert _CW_NS_CMN(oh_p_insert)
#define oh_p_search _CW_NS_CMN(oh_p_search)
#define oh_p_rehash _CW_NS_CMN(oh_p_rehash)

/* 2^(_OH_BASE_POWER) is the base table size used for calculations. */
#define _OH_BASE_POWER 8;
#define _OH_BASE_H2 7;

/* Minimum number of items allowable before regrowing. */
#define _OH_BASE_SHRINK_POINT 64;

/* Maximum number of items allowable before growing. */
#define _OH_BASE_GROW_POINT 230;

/* Maximum number of items allowable before we consider rehashing. */
#define _OH_BASE_REHASH_POINT 192;

void oh_p_quiesce(cw_oh_t * a_oh_o);
void oh_p_grow(cw_oh_t * a_oh_o);
void oh_p_shrink(cw_oh_t * a_oh_o);

cw_uint64_t oh_p_h1(cw_oh_t * a_oh_o, void * a_key);
cw_bool_t oh_p_key_compare(void * a_k1, void * a_k2);
cw_bool_t oh_p_item_insert(cw_oh_t * a_oh_o,
			   cw_oh_item_t * a_item);
cw_bool_t oh_p_item_search(cw_oh_t * a_oh_o,
			   void * a_key,
			   cw_uint64_t * a_slot);
void oh_p_rehash(cw_oh_t * a_oh_o, cw_bool_t a_force);

#endif /* _OH_PRIV_H_ */
