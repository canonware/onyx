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
 * $Revision: 156 $
 * $Date: 1998-07-29 16:59:01 -0700 (Wed, 29 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Private header for bhp class (binomial heap).
 *
 ****************************************************************************/

#ifndef _BHP_PRIV_H_
#define _BHP_PRIV_H_

#define bhp_bin_link _CW_NS_ANY(bhp_bin_link)
#define bhp_merge _CW_NS_ANY(bhp_merge)

void bhp_bin_link(cw_bhpi_t * a_root, cw_bhpi_t * a_non_root);
void bhp_merge(cw_bhp_t * a_bhp_o, cw_bhp_t * a_other);
cw_sint32_t bhp_priority_compare(cw_bhpi_t * a_a, cw_bhpi_t * a_b);

#endif /* _BHP_PRIV_H_ */
