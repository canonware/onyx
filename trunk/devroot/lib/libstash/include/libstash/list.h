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
 * Public interface for the (doubly linked) list and list_item classes.
 *
 ****************************************************************************/

/* Opaque type. */
typedef struct cw_list_item_s cw_list_item_t;

/* Pseudo-opaque type. */
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

/****************************************************************************
 *
 * list_item constructor.
 *
 ****************************************************************************/
#define list_item_new _CW_NS_STASH(list_item_new)
cw_list_item_t *
list_item_new();

/****************************************************************************
 *
 * list_item_destructor.
 *
 ****************************************************************************/
#define list_item_delete _CW_NS_STASH(list_item_delete)
void
list_item_delete(cw_list_item_t * a_cont);

/****************************************************************************
 *
 * Get the value of the data pointer.
 *
 ****************************************************************************/
#define list_item_get _CW_NS_STASH(list_item_get)
void *
list_item_get(cw_list_item_t * a_cont);

/****************************************************************************
 *
 * Set the value of the data pointer.
 *
 ****************************************************************************/
#define list_item_set _CW_NS_STASH(list_item_set)
void
list_item_set(cw_list_item_t * a_cont, void * a_data);

/****************************************************************************
 *
 * list constructor.
 *
 ****************************************************************************/
#define list_new _CW_NS_STASH(list_new)
#ifdef _CW_REENTRANT
cw_list_t *
list_new(cw_list_t * a_list, cw_bool_t a_is_thread_safe);
#else
cw_list_t *
list_new(cw_list_t * a_list);
#endif

/****************************************************************************
 *
 * list destructor.
 *
 ****************************************************************************/
#define list_delete _CW_NS_STASH(list_delete)
void
list_delete(cw_list_t * a_list);

/****************************************************************************
 *
 * Get the value of the data pointer.
 *
 ****************************************************************************/
#define list_count _CW_NS_STASH(list_count)
cw_uint64_t
list_count(cw_list_t * a_list);

/****************************************************************************
 *
 * Pushes an item onto the head of the list.
 *
 ****************************************************************************/
#define list_hpush _CW_NS_STASH(list_hpush)
cw_list_item_t *
list_hpush(cw_list_t * a_list, void * a_data);

/****************************************************************************
 *
 * Pops an item off the head of the list.
 *
 ****************************************************************************/
#define list_hpop _CW_NS_STASH(list_hpop)
void *
list_hpop(cw_list_t * a_list);

/****************************************************************************
 *
 * Returns the item at the head of the list, without removing it.
 *
 ****************************************************************************/
#define list_hpeek _CW_NS_STASH(list_hpeek)
void *
list_hpeek(cw_list_t * a_list);

/****************************************************************************
 *
 * Pushes an item onto the tail of the list.
 *
 ****************************************************************************/
#define list_tpush _CW_NS_STASH(list_tpush)
cw_list_item_t *
list_tpush(cw_list_t * a_list, void * a_data);

/****************************************************************************
 *
 * Pops an item off the tail of the list.
 *
 ****************************************************************************/
#define list_tpop _CW_NS_STASH(list_tpop)
void *
list_tpop(cw_list_t * a_list);

/****************************************************************************
 *
 * Returns the item at the tail of the list without removing it.
 *
 ****************************************************************************/
#define list_tpeek _CW_NS_STASH(list_tpeek)
void *
list_tpeek(cw_list_t * a_list);

/****************************************************************************
 *
 * Inserts an item before the list node pointed to by a_in_list.
 *
 ****************************************************************************/
#define list_insert_before _CW_NS_STASH(list_insert_before)
cw_list_item_t *
list_insert_before(cw_list_t * a_list, cw_list_item_t * a_in_list,
		   void * a_data);

/****************************************************************************
 *
 * Inserts an item after the list node pointed to by a_in_list.
 *
 ****************************************************************************/
#define list_insert_after _CW_NS_STASH(list_insert_after)
cw_list_item_t *
list_insert_after(cw_list_t * a_list, cw_list_item_t * a_in_list,
		  void * a_data);

/****************************************************************************
 *
 * Given a pointer to an item, removes the item from the list and returns
 * the data pointer.
 *
 ****************************************************************************/
#define list_remove _CW_NS_STASH(list_remove)
void *
list_remove(cw_list_t * a_list, cw_list_item_t * a_to_remove);

/****************************************************************************
 *
 * Free the space used by the free item list.
 *
 ****************************************************************************/
#define list_purge_spares _CW_NS_STASH(list_purge_spares)
void
list_purge_spares(cw_list_t * a_list);

/****************************************************************************
 *
 * Print debugging spew.  Note that the 64 bit values don't print correctly 
 * when using long long for 64 bit variables.
 *
 ****************************************************************************/
#define list_dump _CW_NS_STASH(list_dump)
void
list_dump(cw_list_t * a_list);
