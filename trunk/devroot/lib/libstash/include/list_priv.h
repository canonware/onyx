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

#ifndef _LIST_PRIV_H_
#define _LIST_PRIV_H_

struct cw_list_item_s
{
  struct cw_list_item_s * next;
  struct cw_list_item_s * prev;
  void * item;
};

#define list_p_hpop _CW_NS_CMN(list_p_hpop)
#define list_p_tpop _CW_NS_CMN(list_p_tpop)

void * list_p_hpop(cw_list_t * a_list_o);
void * list_p_tpop(cw_list_t * a_list_o);

#endif /* _LIST_PRIV_H_ */
