/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

struct cw_list_item_s
{
  struct cw_list_item_s * next;
  struct cw_list_item_s * prev;
  void * item;
};

#define list_item_p_get_prev(a) (a)->prev
#define list_item_p_set_prev(a, b) (a)->prev = (b)
#define list_item_p_get_next(a) (a)->next
#define list_item_p_set_next(a, b) (a)->next = (b)

#define list_p_hpop _CW_NS_STASH(list_p_hpop)
void *
list_p_hpop(cw_list_t * a_list);

#define list_p_tpop _CW_NS_STASH(list_p_tpop)
void *
list_p_tpop(cw_list_t * a_list);
