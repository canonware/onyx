/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
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
cw_list_item_t *
list_item_new();

/****************************************************************************
 *
 * list_item_destructor.
 *
 ****************************************************************************/
void
list_item_delete(cw_list_item_t * a_cont);

/****************************************************************************
 *
 * Get the value of the data pointer.
 *
 ****************************************************************************/
void *
list_item_get(cw_list_item_t * a_cont);

/****************************************************************************
 *
 * Set the value of the data pointer.
 *
 ****************************************************************************/
void
list_item_set(cw_list_item_t * a_cont, void * a_data);

/****************************************************************************
 *
 * list constructor.
 *
 ****************************************************************************/
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
void
list_delete(cw_list_t * a_list);

/****************************************************************************
 *
 * Get the value of the data pointer.
 *
 ****************************************************************************/
cw_uint64_t
list_count(cw_list_t * a_list);

/****************************************************************************
 *
 * Pushes an item onto the head of the list.
 *
 ****************************************************************************/
cw_list_item_t *
list_hpush(cw_list_t * a_list, void * a_data);

/****************************************************************************
 *
 * Pops an item off the head of the list.
 *
 ****************************************************************************/
void *
list_hpop(cw_list_t * a_list);

/****************************************************************************
 *
 * Returns the item at the head of the list, without removing it.
 *
 ****************************************************************************/
void *
list_hpeek(cw_list_t * a_list);

/****************************************************************************
 *
 * Pushes an item onto the tail of the list.
 *
 ****************************************************************************/
cw_list_item_t *
list_tpush(cw_list_t * a_list, void * a_data);

/****************************************************************************
 *
 * Pops an item off the tail of the list.
 *
 ****************************************************************************/
void *
list_tpop(cw_list_t * a_list);

/****************************************************************************
 *
 * Returns the item at the tail of the list without removing it.
 *
 ****************************************************************************/
void *
list_tpeek(cw_list_t * a_list);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * a_in_list : Pointer to an item container in a_list, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to an item container, or NULL.
 *
 * <<< Description >>>
 *
 * Return a pointer to the next container after a_in_list in a_list (NULL if
 * a_in_list is the last container).  If a_in_list is NULL, return a pointer to
 * the first container in a_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_get_next(cw_list_t * a_list, cw_list_item_t * a_in_list);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * a_in_list : Pointer to an item container in a_list, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to an item container, or NULL.
 *
 * <<< Description >>>
 *
 * Return a pointer to the container before a_in_list in a_list (NULL if
 * a_in_list is the first container).  If a_in_list is NULL, return a pointer to
 * the last container in a_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_get_prev(cw_list_t * a_list, cw_list_item_t * a_in_list);

/****************************************************************************
 *
 * Inserts an item before the list node pointed to by a_in_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_insert_before(cw_list_t * a_list, cw_list_item_t * a_in_list,
		   void * a_data);

/****************************************************************************
 *
 * Inserts an item after the list node pointed to by a_in_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_insert_after(cw_list_t * a_list, cw_list_item_t * a_in_list,
		  void * a_data);

/****************************************************************************
 *
 * Given a pointer to an item, removes the item from the list and returns
 * the data pointer.
 *
 ****************************************************************************/
void *
list_remove(cw_list_t * a_list, cw_list_item_t * a_to_remove);

/****************************************************************************
 *
 * Free the space used by the free item list.
 *
 ****************************************************************************/
void
list_purge_spares(cw_list_t * a_list);

/****************************************************************************
 *
 * Print debugging spew.  Note that the 64 bit values don't print correctly 
 * when using long long for 64 bit variables.
 *
 ****************************************************************************/
void
list_dump(cw_list_t * a_list);
