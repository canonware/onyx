/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 94 $
 * $Date: 1998-06-26 17:18:43 -0700 (Fri, 26 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _BRBS_PRIV_H_
#define _BRBS_PRIV_H_

#define brbs_p_get_sector_size _CW_NS_CMN(brbs_p_get_sector_size)
#define brbs_p_get_is_raw _CW_NS_CMN(brbs_p_get_is_raw)

void brbs_p_get_sector_size(cw_brbs_t * a_brbs_o);
void brbs_p_get_is_raw(cw_brbs_t * a_brbs_o);

#endif /* _BRBS_PRIV_H_ */
