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
 * $Revision: 233 $
 * $Date: 1998-09-23 16:22:28 -0700 (Wed, 23 Sep 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _BR_PRIV_H_
#define _BR_PRIV_H_

#define br_p_fsck _CW_NS_ANY(br_p_fsck)
#define br_p_map_brbs _CW_NS_ANY(br_p_map_brbs)
#define br_p_unmap_brbs _CW_NS_ANY(br_p_unmap_brbs)

cw_bool_t br_p_fsck(cw_br_t * a_br_o);
cw_bool_t br_p_map_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_o,
			cw_uint64_t a_base_addr);
cw_bool_t br_p_unmap_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_o);

#endif /* _BR_PRIV_H_ */
