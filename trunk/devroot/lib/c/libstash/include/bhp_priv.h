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
 * $Revision: 173 $
 * $Date: 1998-08-26 12:34:42 -0700 (Wed, 26 Aug 1998) $
 *
 * <<< Description >>>
 *
 * Private header for bhp class (binomial heap).
 *
 ****************************************************************************/

#ifndef _BHP_PRIV_H_
#define _BHP_PRIV_H_

#define bhp_p_bin_link _CW_NS_ANY(bhp_p_bin_link)
#define bhp_p_merge _CW_NS_ANY(bhp_p_merge)
#define bhp_p_priority_compare _CW_NS_ANY(bhp_p_priority_compare)

void bhp_p_bin_link(cw_bhpi_t * a_root, cw_bhpi_t * a_non_root);
void bhp_p_merge(cw_bhp_t * a_bhp_o, cw_bhp_t * a_other);
cw_sint32_t bhp_p_priority_compare(cw_bhpi_t * a_a, cw_bhpi_t * a_b);

#endif /* _BHP_PRIV_H_ */
