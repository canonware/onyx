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
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a list_item, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_list_item_t *
list_item_new(void);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cont : Pointer to a list_item.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
list_item_delete(cw_list_item_t * a_cont);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cont : Pointer to a list_item.
 *
 * <<< Output(s) >>>
 *
 * retval : Data pointer.
 *
 * <<< Description >>>
 *
 * Get the value of the data pointer.
 *
 ****************************************************************************/
void *
list_item_get(cw_list_item_t * a_cont);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_cont : Pointer to a list_item.
 *
 * a_data : Data pointer.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set the value of a_cont's data pointer to a_data.
 *
 ****************************************************************************/
void
list_item_set(cw_list_item_t * a_cont, void * a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to space for a list, or NULL.
 *
 * a_is_thread_safe : TRUE if list is to be threadsafe, FALSE otherwise.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a list, or NULL.
 *          NULL : Memory allocation error.  Can only occur if (NULL == a_list).
 *
 * <<< Description >>>
 *
 * Constructor.
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
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
list_delete(cw_list_t * a_list);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of items in a_list.
 *
 * <<< Description >>>
 *
 * Return the number of items in a_list.
 *
 ****************************************************************************/
cw_uint64_t
list_count(cw_list_t * a_list);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a list.
 *
 * a_b : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Append the contents of a_b to a_a.
 *
 ****************************************************************************/
void
list_catenate_list(cw_list_t * a_a, cw_list_t * a_b);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * a_data : Data pointer.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to the list_item that contains a_data, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Push an item onto the head of a_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_hpush(cw_list_t * a_list, void * a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * retval : Data pointer, or NULL.
 *          NULL : a_list is empty.
 *
 * <<< Description >>>
 *
 * Pop an item of the head of a_list.
 *
 ****************************************************************************/
void *
list_hpop(cw_list_t * a_list);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * retval : Data pointer, or NULL.
 *          NULL : a_list is empty.
 *
 * <<< Description >>>
 *
 * Return the item at the head of the list, without removing it.
 *
 ****************************************************************************/
void *
list_hpeek(cw_list_t * a_list);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * a_data : Data pointer.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to the list_item that contains a_data, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Push an item onto the tail of a_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_tpush(cw_list_t * a_list, void * a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * retval : Data pointer, or NULL.
 *          NULL : a_list is empty.
 *
 * <<< Description >>>
 *
 * Pop an item of the tail of a_list.
 *
 ****************************************************************************/
void *
list_tpop(cw_list_t * a_list);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * retval : Data pointer, or NULL.
 *          NULL : a_list is empty.
 *
 * <<< Description >>>
 *
 * Return the item at the tail of the list, without removing it.
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
 *          NULL : a_in_list is the tail of a_list.
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
 *          NULL : a_in_list is the head of a_list.
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
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * a_in_list : Pointer to a list_item in a_list.
 *
 * a_data : Data pointer.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to the list_item that contains a_data, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Insert an item before the list node pointed to by a_in_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_insert_before(cw_list_t * a_list, cw_list_item_t * a_in_list,
		   void * a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * a_in_list : Pointer to a list_item in a_list.
 *
 * a_data : Data pointer.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to the list_item that contains a_data, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Insert an item after the list node pointed to by a_in_list.
 *
 ****************************************************************************/
cw_list_item_t *
list_insert_after(cw_list_t * a_list, cw_list_item_t * a_in_list,
		  void * a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * a_data : Pointer to data.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a_data, or NULL.
 *
 * <<< Description >>>
 *
 * Find a_data in a_list and remove it from the list.
 *
 ****************************************************************************/
void *
list_remove_item(cw_list_t * a_list, void * a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * a_to_remove : Pointer to a list_item.
 *
 * <<< Output(s) >>>
 *
 * retval : Data pointer.
 *
 * <<< Description >>>
 *
 * Given a pointer to an item, remove the item from the list and return the
 * data pointer.
 *
 ****************************************************************************/
void *
list_remove_container(cw_list_t * a_list, cw_list_item_t * a_to_remove);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Free the space used by the internal free item list.
 *
 ****************************************************************************/
void
list_purge_spares(cw_list_t * a_list);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_list : Pointer to a list.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Print debugging spew to cw_g_out.
 *
 ****************************************************************************/
void
list_dump(cw_list_t * a_list);
