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

#ifndef _LIST_PRIV_H_
#define _LIST_PRIV_H_

struct cw_list_item_s
{
  struct cw_list_item_s * next;
  struct cw_list_item_s * prev;
  void * item;
};

#define list_hpop_priv _CW_NS_CMN(list_hpop_priv)
#define list_tpop_priv _CW_NS_CMN(list_tpop_priv)

void * list_hpop_priv(cw_list_t * a_list_o);
void * list_tpop_priv(cw_list_t * a_list_o);

#endif /* _LIST_PRIV_H_ */
