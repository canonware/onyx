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
 * Public interface for the bhp (binomial heap) class, and its helper, the bhpi
 * class.
 *
 ****************************************************************************/

/* Typedef to allow easy function pointer passing. */
typedef cw_sint32_t bhp_prio_comp_t(const void *, const void *);

/* Pseudo-opaque types. */
typedef struct cw_bhp_s cw_bhp_t;
typedef struct cw_bhpi_s cw_bhpi_t;

struct cw_bhp_s
{
  cw_bool_t is_malloced;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
#endif
  cw_bhpi_t * head;
  cw_uint64_t num_nodes;
  bhp_prio_comp_t * priority_compare;
};

struct cw_bhpi_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;
  
  const void * priority;
  const void * data;
  struct cw_bhpi_s * parent;
  struct cw_bhpi_s * child;
  struct cw_bhpi_s * sibling;
  cw_uint32_t degree;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhpi : Pointer to space for a bhpi, or NULL.
 *
 * a_priority : Pointer to a priority object.
 *
 * a_data : Pointer to a data object.
 *
 * a_dealloc_func : Pointer to a memory deallocation function, or NULL.  Ignored
 *                  if (NULL == a_bhpi).
 *
 * a_dealloc_arg : Pointer to first argument to a_dealloc_func, or NULL.
 *                 Ignored if (NULL == a_dealloc_func), or (NULL == a_bhpi).
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a bhpi, or NULL.
 *          NULL : Memory allocation error.  Can only occur if (NULL == a_bhpi).
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_bhpi_t *
bhpi_new(cw_bhpi_t * a_bhpi, const void * a_priority, const void * a_data,
	 void (*a_dealloc_func)(void * dealloc_arg, void * bhpi),
	 void * a_dealloc_arg);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhpi : Pointer to a bhpi.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.  If the bhpi has been inserted into a bhp, do not call this
 * method, since the bhp code will eventually do it.
 *
 * Note: No attempt is made to free data pointed to by the priority or data
 * pointers.
 *
 ****************************************************************************/
void
bhpi_delete(cw_bhpi_t * a_bhpi);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhp : Pointer to space for a bhp, or NULL.
 *
 * a_prio_comp : Pointer to a priority comparison function.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a bhp, or NULL.
 *          NULL : Memory allocation error.  Can only occur if (NULL == a_bhp).
 *
 * <<< Description >>>
 *
 * Non-thread-safe and thread-safe constructors.
 *
 ****************************************************************************/
cw_bhp_t *
bhp_new(cw_bhp_t * a_bhp, bhp_prio_comp_t * a_prio_comp);
cw_bhp_t *
bhp_new_r(cw_bhp_t * a_bhp, bhp_prio_comp_t * a_prio_comp);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhp : Pointer to a bhp.
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
bhp_delete(cw_bhp_t * a_bhp);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhp : Pointer to a bhp.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Print the internal tree to cw_g_out.
 *
 ****************************************************************************/
void
bhp_dump(cw_bhp_t * a_bhp);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhp : Pointer to a bhp.
 *
 * a_bhpi : Pointer to a bhpi (heap node).
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Insert an item into a_bhp.
 *
 ****************************************************************************/
void
bhp_insert(cw_bhp_t * a_bhp, cw_bhpi_t * a_bhpi);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhp : Pointer to a bhp.
 *
 * r_priority : Pointer to a pointer, or NULL.
 *
 * r_data : Pointer to a pointer, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Empty heap.
 *
 * *r_priority : Pointer to the priority object.
 *
 * *r_data : Pointer to the data object.
 *
 * <<< Description >>>
 *
 * Set *r_priority and *r_data to point to a minimum node in a_bhp.
 *
 ****************************************************************************/
cw_bool_t
bhp_find_min(cw_bhp_t * a_bhp, void ** r_priority, void ** r_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhp : Pointer to a bhp.
 *
 * r_priority : Pointer to a pointer, or NULL.
 *
 * r_data : Pointer to a pointer, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Empty heap.
 *
 * *r_priority : Pointer to the priority object.
 *
 * *r_data : Pointer to the data object.
 *
 * <<< Description >>>
 *
 * Set *r_priority and *r_data to point to a minimum node in a_bhp and remove
 * the item from a_bhp.
 *
 ****************************************************************************/
cw_bool_t
bhp_del_min(cw_bhp_t * a_bhp, void ** r_priority, void ** r_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bhp : Pointer to a bhp.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of items in a_bhp.
 *
 * <<< Description >>>
 *
 * Return the number of items in a_bhp.
 *
 ****************************************************************************/
cw_uint64_t
bhp_get_size(cw_bhp_t * a_bhp);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a bhp.
 *
 * a_b : Pointer to a bhp.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Merge a_b into a_a.  a_b is invalid after this call, and does not
 * need to be deleted.
 *
 ****************************************************************************/
void
bhp_union(cw_bhp_t * a_a, cw_bhp_t * a_b);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a cw_uint32_t.
 *
 * a_b : Pointer to a cw_uint32_t.
 *
 * <<< Output(s) >>>
 *
 * retval : -1 : *a_a <  *a_b.
 *           0 : *a_a == *a_b.
 *           1 : *a_a >  *a_b.
 *
 * <<< Description >>>
 *
 * Compare *a_a and *a_b as unsigned 32 bit integers.
 *
 ****************************************************************************/
cw_sint32_t
bhp_priority_compare_uint32(const void * a_a, const void * a_b);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a cw_sint32_t.
 *
 * a_b : Pointer to a cw_sint32_t.
 *
 * <<< Output(s) >>>
 *
 * retval : -1 : *a_a <  *a_b.
 *           0 : *a_a == *a_b.
 *           1 : *a_a >  *a_b.
 *
 * <<< Description >>>
 *
 * Compare *a_a and *a_b as signed 32 bit integers.
 *
 ****************************************************************************/
cw_sint32_t
bhp_priority_compare_sint32(const void * a_a, const void * a_b);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a cw_uint64_t.
 *
 * a_b : Pointer to a cw_uint64_t.
 *
 * <<< Output(s) >>>
 *
 * retval : -1 : *a_a <  *a_b.
 *           0 : *a_a == *a_b.
 *           1 : *a_a >  *a_b.
 *
 * <<< Description >>>
 *
 * Compare *a_a and *a_b as unsigned 64 bit integers.
 *
 ****************************************************************************/
cw_sint32_t
bhp_priority_compare_uint64(const void * a_a, const void * a_b);
