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
 * Implementation of arbitrary trees.  Each treen (tree node) can have an
 * arbitrary number of children.
 *
 * Don't do silly things like creating a cycle in the graph, or bad things
 * (infinite recursion) could happen.
 *
 ****************************************************************************/

#define _INC_TREE_H_
#ifdef _CW_REENTRANT
#  include "libstash_r.h"
#else
#  include "libstash.h"
#endif

/****************************************************************************
 *
 * tree constructor.
 *
 ****************************************************************************/
cw_tree_t *
#ifdef _CW_REENTRANT
tree_new(cw_tree_t * a_tree_o, cw_bool_t a_is_thread_safe)
#else
tree_new(cw_tree_t * a_tree_o)
#endif
{
  cw_tree_t * retval;
  
  if (a_tree_o == NULL)
  {
    retval = (cw_tree_t *) _cw_malloc(sizeof(cw_tree_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_tree_o;
    retval->is_malloced = FALSE;
  }
  
#ifdef _CW_REENTRANT
  if (a_is_thread_safe == TRUE)
  {
    mtx_new(&retval->lock);
    retval->is_thread_safe = TRUE;
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
#endif

  retval->root = NULL;
  
  return retval;
}

/****************************************************************************
 *
 * tree destructor.  Deletes the tree nodes as well, but leaves the data
 * pointers alone.
 *
 ****************************************************************************/
void
tree_delete(cw_tree_t * a_tree_o)
{
  _cw_check_ptr(a_tree_o);

  treen_delete(a_tree_o->root);

#ifdef _CW_REENTRANT
  if (a_tree_o->is_thread_safe == TRUE)
  {
    mtx_delete(&a_tree_o->lock);
  }
#endif

  if (a_tree_o->is_malloced == TRUE)
  {
    _cw_free(a_tree_o);
  }
}

/****************************************************************************
 *
 * Returns a pointer to the root treen.
 *
 ****************************************************************************/
cw_treen_t *
tree_get_root_ptr(cw_tree_t * a_tree_o)
{
  cw_treen_t * retval;
  
  _cw_marker("Got here");
  _cw_check_ptr(a_tree_o);
  _cw_marker("Got here");
#ifdef _CW_REENTRANT
  if (a_tree_o->is_thread_safe)
  {
  _cw_marker("Got here");
    mtx_lock(&a_tree_o->lock);
  _cw_marker("Got here");
  }
#endif
  _cw_marker("Got here");

  retval = a_tree_o->root;
  
#ifdef _CW_REENTRANT
  if (a_tree_o->is_thread_safe)
  {
    mtx_unlock(&a_tree_o->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Sets the pointer to the root treen.
 *
 ****************************************************************************/
void
tree_set_root_ptr(cw_tree_t * a_tree_o, cw_treen_t * a_treen_o)
{
  _cw_check_ptr(a_tree_o);
#ifdef _CW_REENTRANT
  if (a_tree_o->is_thread_safe)
  {
    mtx_lock(&a_tree_o->lock);
  }
#endif

  a_tree_o->root = a_treen_o;

#ifdef _CW_REENTRANT
  if (a_tree_o->is_thread_safe)
  {
    mtx_unlock(&a_tree_o->lock);
  }
#endif
}

/****************************************************************************
 *
 * treen constructor.
 *
 ****************************************************************************/
cw_treen_t *
treen_new(cw_bool_t a_is_thread_safe)
{
  cw_treen_t * retval;

  retval = (cw_treen_t *) _cw_malloc(sizeof(cw_treen_t));
  bzero(retval, sizeof(cw_treen_t));
  
#ifdef _CW_REENTRANT
  if (a_is_thread_safe == TRUE)
  {
    mtx_new(&retval->lock);
    retval->is_thread_safe = TRUE;
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
#endif
  
  return retval;
}

/****************************************************************************
 *
 * treen destructor.
 *
 ****************************************************************************/
void
treen_delete(cw_treen_t * a_treen_o)
{
  if (a_treen_o == NULL)
  {
    /* Non-existent node.  Do nothing. */
  }
  else
  {
    cw_uint32_t i;

    /* Recursively delete all subtrees. */
    for (i = 0;
	 i < a_treen_o->num_children;
	 i++)
    {
      treen_delete(a_treen_o->children[i]);
    }

    /* Delete self. */
#ifdef _CW_REENTRANT
    if (a_treen_o->is_thread_safe == TRUE)
    {
      mtx_delete(&a_treen_o->lock);
    }
#endif

    _cw_free(a_treen_o);
  }
}

/****************************************************************************
 *
 * Returns a pointer to the parent of a_treen_o.
 *
 ****************************************************************************/
cw_treen_t *
treen_get_parent_ptr(cw_treen_t * a_treen_o)
{
  cw_treen_t * retval;
  
  _cw_check_ptr(a_treen_o);
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_lock(&a_treen_o->lock);
  }
#endif

  retval = a_treen_o->parent;
  
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_unlock(&a_treen_o->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Sets the pointer to the parent of a_treen_o.
 *
 ****************************************************************************/
void
treen_set_parent_ptr(cw_treen_t * a_treen_o, cw_treen_t * a_parent)
{
  _cw_check_ptr(a_treen_o);
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_lock(&a_treen_o->lock);
  }
#endif

  a_treen_o->parent = a_parent;

#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_unlock(&a_treen_o->lock);
  }
#endif
}

/****************************************************************************
 *
 * Returns a pointer to the data for a_treen_o.
 *
 ****************************************************************************/
void *
treen_get_data_ptr(cw_treen_t * a_treen_o)
{
  void * retval;
  
  _cw_check_ptr(a_treen_o);
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_lock(&a_treen_o->lock);
  }
#endif

  retval = a_treen_o->data;
  
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_unlock(&a_treen_o->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Sets the pointer for data for a_treen_o.
 *
 ****************************************************************************/
void
treen_set_data_ptr(cw_treen_t * a_treen_o, void * a_data)
{
  _cw_check_ptr(a_treen_o);
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_lock(&a_treen_o->lock);
  }
#endif

  a_treen_o->data = a_data;
  
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_unlock(&a_treen_o->lock);
  }
#endif
}

/****************************************************************************
 *
 * Returns the number of children of a_treen_o.
 *
 ****************************************************************************/
cw_uint32_t
treen_get_num_children(cw_treen_t * a_treen_o)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_treen_o);
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_lock(&a_treen_o->lock);
  }
#endif

  retval = a_treen_o->num_children;
  
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_unlock(&a_treen_o->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Inserts a child pointer at position a_position.  If a_position is greater
 * than the number of children, returns TRUE and does not insert the child
 * pointer.
 *
 ****************************************************************************/
cw_bool_t
treen_ins_child(cw_treen_t * a_treen_o, cw_treen_t * a_child,
		cw_uint32_t a_position)
{
  cw_bool_t retval;

  _cw_check_ptr(a_treen_o);
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_lock(&a_treen_o->lock);
  }
#endif

  if (a_position > a_treen_o->num_children)
  {
    /* More than one position past the end of the child array. */
    retval = TRUE;
  }
  else
  {
    cw_uint32_t i;
    
    retval = FALSE;
    
    a_treen_o->num_children++;

    /* Extend the array. */
    a_treen_o->children = _cw_realloc(a_treen_o->children,
				      a_treen_o->num_children
				      * sizeof(cw_treen_t *));

    /* Shuffle things forward to make room. */
    for (i = (a_treen_o->num_children - 1); i > a_position; i--)
    {
      a_treen_o->children[i] = a_treen_o->children[i - 1];
    }

    /* Plop the new child pointer in place. */
    a_treen_o->children[a_position] = a_child;
  }
  
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_unlock(&a_treen_o->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Deletes child pointer at a_position and shuffles following children down to
 * fill the space.  If a_position is greater than the index of the last child,
 * return TRUE.
 *
 ****************************************************************************/
cw_bool_t
treen_del_child(cw_treen_t * a_treen_o, cw_uint32_t a_position,
		cw_treen_t ** a_child)
{
  cw_bool_t retval;

  if (a_position >= a_treen_o->num_children)
  {
    /* More than one position past the end of the child array. */
    retval = TRUE;
  }
  else
  {
    cw_uint32_t i;
    
#ifdef _CW_REENTRANT
    if (a_treen_o->is_thread_safe)
    {
      mtx_lock(&a_treen_o->lock);
    }
#endif
    retval = FALSE;
    
    *a_child = a_treen_o->children[a_position];

    /* Shuffle things backward to fill the gap. */
    for (i = a_position + 1; i < a_treen_o->num_children; i++)
    {
      a_treen_o->children[i - 1] = a_treen_o->children[i];
    }

    a_treen_o->num_children--;

    /* Truncate the array. */
    a_treen_o->children = _cw_realloc(a_treen_o->children,
				      a_treen_o->num_children
				      * sizeof(cw_treen_t *));

#ifdef _CW_REENTRANT
    if (a_treen_o->is_thread_safe)
    {
      mtx_unlock(&a_treen_o->lock);
    }
#endif
  }

  return retval;
}

/****************************************************************************
 *
 * Returns a pointer to the child pointer at a_position in *a_child, unless
 * a_position is greater than the index of the last child, in which case,
 * returns TRUE.
 *
 ****************************************************************************/
cw_bool_t
treen_get_child_ptr(cw_treen_t * a_treen_o, cw_uint32_t a_position,
		    cw_treen_t ** a_child)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_treen_o);
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_lock(&a_treen_o->lock);
  }
#endif

  if (a_position < a_treen_o->num_children)
  {
    retval = FALSE;
    *a_child = a_treen_o->children[a_position];
  }
  else
  {
    /* Past end of child pointer array. */
    retval = TRUE;
  }
  
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_unlock(&a_treen_o->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Sets the child pointer at a_position to a_child.  If a_position is past the
 * last child pointer, returns TRUE.
 *
 ****************************************************************************/
cw_bool_t
treen_set_child_ptr(cw_treen_t * a_treen_o, cw_uint32_t a_position,
		    cw_treen_t * a_child)
{
  cw_bool_t retval;

  _cw_check_ptr(a_treen_o);
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_lock(&a_treen_o->lock);
  }
#endif

  if (a_position < a_treen_o->num_children)
  {
    retval = FALSE;
    a_treen_o->children[a_position] = a_child;
  }
  else
  {
    /* Past end of child pointer array. */
    retval = TRUE;
  }
  
#ifdef _CW_REENTRANT
  if (a_treen_o->is_thread_safe)
  {
    mtx_unlock(&a_treen_o->lock);
  }
#endif
  return retval;
}
