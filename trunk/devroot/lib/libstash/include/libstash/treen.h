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
 * Implementation of arbitrary trees.  Each treen (tree node) can have an
 * arbitrary number of children.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_treen_s cw_treen_t;

struct cw_treen_s
{
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
#endif
  void * data;
  cw_uint32_t num_children;
  cw_treen_t ** children;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a treen, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Non-thread-safe and thread-safe constructors.
 *
 ****************************************************************************/
cw_treen_t *
treen_new(void);
#ifdef _CW_REENTRANT
cw_treen_t *
treen_new_r(void);
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_treen : Pointer to a treen.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.  Recursively destroys all subtrees.
 *
 ****************************************************************************/
void
treen_delete(cw_treen_t * a_treen);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_treen : Pointer to a treen.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of children of a_treen.
 *
 * <<< Description >>>
 *
 * Return the number of children of a_treen.
 *
 ****************************************************************************/
cw_uint32_t
treen_get_num_children(cw_treen_t * a_treen);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_treen : Pointer to a treen.
 *
 * a_child : Pointer to a treen.
 *
 * a_position : Which child of a_treen to insert a_child as (0..n).
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : a_position > number of children.
 *               : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Insert a child pointer at position a_position.  If a_position is greater than
 * the number of children, return TRUE and do not insert the child pointer.
 *
 ****************************************************************************/
cw_bool_t
treen_link_child(cw_treen_t * a_treen, cw_treen_t * a_child,
		 cw_uint32_t a_position);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_treen : Pointer to a treen.
 *
 * a_position : Which child of a_treen to unlink (0..n).
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : a_position > number of children.
 *
 * *r_child : Pointer to a treen.
 *
 * <<< Description >>>
 *
 * Delete child pointer at a_position and shuffle following children down to
 * fill the space.  If a_position is greater than the index of the last child,
 * return TRUE.
 *
 ****************************************************************************/
cw_bool_t
treen_unlink_child(cw_treen_t * a_treen, cw_uint32_t a_position,
		   cw_treen_t ** r_child);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_treen : Pointer to a treen.
 *
 * a_position : Whild child of a_treen to return a pointer to (0..n).
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : a_position > number of children.
 *
 * *r_child : Pointer to a treen.
 *
 * <<< Description >>>
 *
 * Return a pointer to the child pointer at a_position in *r_child, unless
 * a_position is greater than the index of the last child, in which case, return
 * TRUE.
 *
 ****************************************************************************/
cw_bool_t
treen_get_child_ptr(cw_treen_t * a_treen, cw_uint32_t a_position,
		    cw_treen_t ** r_child);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_treen : Pointer to a treen.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to data.
 *
 * <<< Description >>>
 *
 * Return a pointer to the data for a_treen.
 *
 ****************************************************************************/
void *
treen_get_data_ptr(cw_treen_t * a_treen);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_treen : Pointer to a treen.
 *
 * a_data : Pointer to data.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to old data.
 *
 * <<< Description >>>
 *
 * Set the pointer for data for a_treen.
 *
 ****************************************************************************/
void *
treen_set_data_ptr(cw_treen_t * a_treen, void * a_data);
