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

static cw_list_t *
list_p_new(cw_list_t * a_list, cw_bool_t a_is_thread_safe);

/****************************************************************************
 *
 * Pop an item of the head of the list, without locking.
 *
 ****************************************************************************/
static void *
list_p_hpop(cw_list_t * a_list);

/****************************************************************************
 *
 * Pop an item off the tail of the list, without locking.
 *
 ****************************************************************************/
static void *
list_p_tpop(cw_list_t * a_list);

/* Remove a list_item container from the list and return the data pointer. */
static void *
list_p_remove_container(cw_list_t * a_list, cw_list_item_t * a_to_remove);
