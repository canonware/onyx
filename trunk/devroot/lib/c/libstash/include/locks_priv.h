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
 * $Revision: 223 $
 * $Date: 1998-09-15 17:27:27 -0700 (Tue, 15 Sep 1998) $
 *
 * <<< Description >>>
 *
 * Private header for rwl and jtl classes.
 *
 ****************************************************************************/

#ifndef _LOCKS_PRIV_H_
#define _LOCKS_PRIV_H_

#define jtl_p_qrwx_unlock _CW_NS_ANY(jtl_p_qrwx_unlock)

void jtl_p_qrwx_unlock(cw_jtl_t * a_jtl_o);

#endif /* _LOCKS_PRIV_H_ */
