/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 118 $
 * $Date: 1998-07-01 00:06:33 -0700 (Wed, 01 Jul 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _BRBS_PRIV_H_
#define _BRBS_PRIV_H_

#define brbs_p_get_raw_info _CW_NS_CMN(brbs_p_get_raw_info)
#define brbs_p_get_is_raw _CW_NS_CMN(brbs_p_get_is_raw)

cw_bool_t brbs_p_get_raw_info(cw_brbs_t * a_brbs_o);
cw_bool_t brbs_p_get_is_raw(cw_brbs_t * a_brbs_o);

#endif /* _BRBS_PRIV_H_ */
