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
 *
 *
 ****************************************************************************/

#ifndef _LIST_H_
#define _LIST_H_

/* Pseudo-opaque types. */
typedef struct cw_list_item_s cw_list_item_t;
typedef struct cw_list_s cw_list_t;

struct cw_list_s
{
  cw_bool_t is_malloced;
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
  cw_list_item_t * head;
  cw_list_item_t * tail;
  cw_uint64_t count;
  cw_list_item_t * spares_head;
  cw_uint64_t spares_count;
};

/*
 * Namespace definitions.
 */

#define list_item_new _CW_NS_ANY(list_item_new)
#define list_item_delete _CW_NS_ANY(list_item_delete)
#define list_item_get _CW_NS_ANY(list_item_get)
#define list_item_set _CW_NS_ANY(list_item_set)

#define list_new _CW_NS_ANY(list_new)
#define list_delete _CW_NS_ANY(list_delete)
#define list_count _CW_NS_ANY(list_count)
#define list_hpush _CW_NS_ANY(list_hpush)
#define list_hpop _CW_NS_ANY(list_hpop)
#define list_hpeek _CW_NS_ANY(list_hpeek)
#define list_tpush _CW_NS_ANY(list_tpush)
#define list_tpop _CW_NS_ANY(list_tpop)
#define list_tpeek _CW_NS_ANY(list_tpeek)
#define list_insert_after _CW_NS_ANY(list_insert_after)
#define list_remove _CW_NS_ANY(list_remove)
#define list_purge_spares _CW_NS_ANY(list_purge_spares)
#define list_dump _CW_NS_ANY(list_dump)

cw_list_item_t * list_item_new();
void list_item_delete(cw_list_item_t * a_cont);
void * list_item_get(cw_list_item_t * a_cont);
void list_item_set(cw_list_item_t * a_cont, void * a_data);

cw_list_t * list_new(cw_list_t * a_list_o, cw_bool_t a_is_thread_safe);
void list_delete(cw_list_t * a_list_o);
cw_uint64_t list_count(cw_list_t * a_list_o);
cw_list_item_t * list_hpush(cw_list_t * a_list_o, void * a_data);
void * list_hpop(cw_list_t * a_list_o);
void * list_hpeek(cw_list_t * a_list_o);
cw_list_item_t * list_tpush(cw_list_t * a_list_o, void * a_data);
void * list_tpop(cw_list_t * a_list_o);
void * list_tpeek(cw_list_t * a_list_o);
cw_list_item_t * list_insert_after(cw_list_t * a_list_o,
		       cw_list_item_t * a_in_list,
		       void * a_data);
void * list_remove(cw_list_t * a_list_o,
		   cw_list_item_t * a_to_remove);
void list_purge_spares(cw_list_t * a_list_o);
void list_dump(cw_list_t * a_list_o);

#endif /* _LIST_H_ */
