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
 * Public interface for the list and list_item classes.
 *
 ****************************************************************************/

/* Pseudo-opaque types. */
typedef struct cw_list_item_s cw_list_item_t;
typedef struct cw_list_s cw_list_t;

struct cw_list_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
#endif
  cw_list_item_t * head;
  cw_list_item_t * tail;
  cw_uint64_t count;
  cw_list_item_t * spares_head;
  cw_uint64_t spares_count;
};

/*
 * Namespace definitions.
 */

#define list_item_new _CW_NS_STASH(list_item_new)
#define list_item_delete _CW_NS_STASH(list_item_delete)
#define list_item_get _CW_NS_STASH(list_item_get)
#define list_item_set _CW_NS_STASH(list_item_set)

#define list_new _CW_NS_STASH(list_new)
#define list_delete _CW_NS_STASH(list_delete)
#define list_count _CW_NS_STASH(list_count)
#define list_hpush _CW_NS_STASH(list_hpush)
#define list_hpop _CW_NS_STASH(list_hpop)
#define list_hpeek _CW_NS_STASH(list_hpeek)
#define list_tpush _CW_NS_STASH(list_tpush)
#define list_tpop _CW_NS_STASH(list_tpop)
#define list_tpeek _CW_NS_STASH(list_tpeek)
#define list_insert_after _CW_NS_STASH(list_insert_after)
#define list_remove _CW_NS_STASH(list_remove)
#define list_purge_spares _CW_NS_STASH(list_purge_spares)
#define list_dump _CW_NS_STASH(list_dump)

cw_list_item_t * list_item_new();
void list_item_delete(cw_list_item_t * a_cont);
void * list_item_get(cw_list_item_t * a_cont);
void list_item_set(cw_list_item_t * a_cont, void * a_data);

#ifdef _CW_REENTRANT
cw_list_t * list_new(cw_list_t * a_list_o, cw_bool_t a_is_thread_safe);
#else
cw_list_t * list_new(cw_list_t * a_list_o);
#endif
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
