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
 * treen constructor.
 *
 ****************************************************************************/
cw_treen_t *
treen_new(cw_bool_t a_is_thread_safe);

/****************************************************************************
 *
 * treen destructor.  Also deletes all subtrees.
 *
 ****************************************************************************/
void
treen_delete(cw_treen_t * a_treen);

/****************************************************************************
 *
 * Returns the number of children of a_treen.
 *
 ****************************************************************************/
cw_uint32_t
treen_get_num_children(cw_treen_t * a_treen);

/****************************************************************************
 *
 * Inserts a child pointer at position a_position.  If a_position is greater
 * than the number of children, returns TRUE and does not insert the child
 * pointer.
 *
 ****************************************************************************/
cw_bool_t
treen_link_child(cw_treen_t * a_treen, cw_treen_t * a_child,
		 cw_uint32_t a_position);

/****************************************************************************
 *
 * Deletes child pointer at a_position and shuffles following children down to
 * fill the space.  If a_position is greater than the index of the last child,
 * return TRUE.
 *
 ****************************************************************************/
cw_bool_t
treen_unlink_child(cw_treen_t * a_treen, cw_uint32_t a_position,
		   cw_treen_t ** a_child);

/****************************************************************************
 *
 * Returns a pointer to the child pointer at a_position in *a_child, unless
 * a_position is greater than the index of the last child, in which case,
 * returns TRUE.
 *
 ****************************************************************************/
cw_bool_t
treen_get_child_ptr(cw_treen_t * a_treen, cw_uint32_t a_position,
		    cw_treen_t ** a_child);

/****************************************************************************
 *
 * Returns a pointer to the data for a_treen.
 *
 ****************************************************************************/
void *
treen_get_data_ptr(cw_treen_t * a_treen);

/****************************************************************************
 *
 * Sets the pointer for data for a_treen.
 *
 ****************************************************************************/
void *
treen_set_data_ptr(cw_treen_t * a_treen, void * a_data);
