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
 * $Revision: 125 $
 * $Date: 1998-07-02 16:55:52 -0700 (Thu, 02 Jul 1998) $
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
