/****************************************************************************
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

struct cw_list_s {
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t magic_a;
#endif
	cw_bool_t is_malloced;
	cw_bool_t is_thread_safe;
	cw_mtx_t lock;
	cw_list_item_t *head;
	cw_list_item_t *tail;
	cw_uint64_t count;
	cw_list_item_t *spares_head;
	cw_uint64_t spares_count;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t size_of;
	cw_uint32_t magic_b;
#endif
};

cw_list_item_t *list_item_new(void);

void    list_item_delete(cw_list_item_t *a_list_item);

void   *list_item_get(cw_list_item_t *a_list_item);

void    list_item_set(cw_list_item_t *a_list_item, void *a_data);

cw_list_t *list_new(cw_list_t *a_list);
cw_list_t *list_new_r(cw_list_t *a_list);

void    list_delete(cw_list_t *a_list);

cw_uint64_t list_count(cw_list_t *a_list);

void    list_catenate_list(cw_list_t *a_a, cw_list_t *a_b);

cw_list_item_t *list_hpush(cw_list_t *a_list, void *a_data);

void   *list_hpop(cw_list_t *a_list);

void   *list_hpeek(cw_list_t *a_list);

cw_list_item_t *list_tpush(cw_list_t *a_list, void *a_data);

void   *list_tpop(cw_list_t *a_list);

void   *list_tpeek(cw_list_t *a_list);

cw_list_item_t *list_get_next(cw_list_t *a_list, cw_list_item_t *a_in_list);

cw_list_item_t *list_get_prev(cw_list_t *a_list, cw_list_item_t *a_in_list);

cw_list_item_t *
list_insert_before(cw_list_t *a_list, cw_list_item_t *a_in_list,
    void *a_data);

cw_list_item_t *list_insert_after(cw_list_t *a_list, cw_list_item_t *a_in_list,
    void *a_data);

void   *list_remove_item(cw_list_t *a_list, void *a_data);

void   *list_remove_container(cw_list_t *a_list, cw_list_item_t *a_to_remove);

void    list_purge_spares(cw_list_t *a_list);

void    list_dump(cw_list_t *a_list);
