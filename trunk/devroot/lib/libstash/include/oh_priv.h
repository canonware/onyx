/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 86 $
 * $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _OH_PRIV_H_
#define _OH_PRIV_H_

#define oh_coalesce_priv _CW_NS_CMN(oh_coalesce_priv)
#define oh_grow_priv _CW_NS_CMN(oh_grow_priv)
#define oh_shrink_priv _CW_NS_CMN(oh_shrink_priv)
#define oh_h1_priv _CW_NS_CMN(oh_h1_priv)
#define oh_insert_priv _CW_NS_CMN(oh_insert_priv)
#define oh_search_priv _CW_NS_CMN(oh_search_priv)
#define oh_rehash_priv _CW_NS_CMN(oh_rehash_priv)

/* 2^(_OH_BASE_POWER) is the base table size used for calculations. */
#define _OH_BASE_POWER 8;
#define _OH_BASE_H2 7;

/* Minimum number of items allowable before regrowing. */
#define _OH_BASE_SHRINK_POINT 64;

/* Maximum number of items allowable before growing. */
#define _OH_BASE_GROW_POINT 230;

/* Maximum number of items allowable before we consider rehashing. */
#define _OH_BASE_REHASH_POINT 192;

void oh_coalesce_priv(cw_oh_t * a_oh_o);
void oh_grow_priv(cw_oh_t * a_oh_o);
void oh_shrink_priv(cw_oh_t * a_oh_o);

cw_uint64_t oh_h1_priv(cw_oh_t * a_oh_o, void * a_key);
cw_bool_t oh_key_compare_priv(void * a_k1, void * a_k2);
cw_bool_t oh_item_insert_priv(cw_oh_t * a_oh_o,
			      cw_oh_item_t * a_item);
cw_bool_t oh_item_search_priv(cw_oh_t * a_oh_o,
			      void * a_key,
			      cw_uint64_t * a_slot);
void oh_rehash_priv(cw_oh_t * a_oh_o, cw_bool_t a_force);

#endif /* _OH_PRIV_H_ */
