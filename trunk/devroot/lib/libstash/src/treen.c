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
 ****************************************************************************/

#define _STASH_USE_TREEN
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

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
 * treen destructor.  Also deletes all subtrees.
 *
 ****************************************************************************/
void
treen_delete(cw_treen_t * a_treen)
{
  if (a_treen == NULL)
  {
    /* Non-existent node.  Do nothing. */
  }
  else
  {
    cw_uint32_t i;

    /* Recursively delete all subtrees. */
    for (i = 0;
	 i < a_treen->num_children;
	 i++)
    {
      treen_delete(a_treen->children[i]);
    }

    /* Delete self. */
#ifdef _CW_REENTRANT
    if (a_treen->is_thread_safe == TRUE)
    {
      mtx_delete(&a_treen->lock);
    }
#endif

    _cw_free(a_treen);
  }
}

/****************************************************************************
 *
 * Returns the number of children of a_treen.
 *
 ****************************************************************************/
cw_uint32_t
treen_get_num_children(cw_treen_t * a_treen)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_treen);
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_lock(&a_treen->lock);
  }
#endif

  retval = a_treen->num_children;
  
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_unlock(&a_treen->lock);
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
treen_link_child(cw_treen_t * a_treen, cw_treen_t * a_child,
		 cw_uint32_t a_position)
{
  cw_bool_t retval;

  _cw_check_ptr(a_treen);
  _cw_check_ptr(a_child);
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_lock(&a_treen->lock);
  }
#endif

  if (a_position > a_treen->num_children)
  {
    /* More than one position past the end of the child array. */
    retval = TRUE;
  }
  else
  {
    cw_uint32_t i;
    
    retval = FALSE;
    
    a_treen->num_children++;

    /* Extend the array. */
    if (a_treen->children == NULL)
    {
      a_treen->children = _cw_malloc(a_treen->num_children
				       * sizeof(cw_treen_t *));
    }
    else
    {
      a_treen->children = _cw_realloc(a_treen->children,
					a_treen->num_children
					* sizeof(cw_treen_t *));
    }
    
    /* Shuffle things forward to make room. */
    for (i = (a_treen->num_children - 1); i > a_position; i--)
    {
      a_treen->children[i] = a_treen->children[i - 1];
    }

    /* Plop the new child pointer in place. */
    a_treen->children[a_position] = a_child;
  }
  
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_unlock(&a_treen->lock);
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
treen_unlink_child(cw_treen_t * a_treen, cw_uint32_t a_position,
		   cw_treen_t ** a_child)
{
  cw_bool_t retval;

  if (a_position >= a_treen->num_children)
  {
    /* More than one position past the end of the child array. */
    retval = TRUE;
  }
  else
  {
    cw_uint32_t i;

#ifdef _CW_REENTRANT
    if (a_treen->is_thread_safe)
    {
      mtx_lock(&a_treen->lock);
    }
#endif

    retval = FALSE;
    
    *a_child = a_treen->children[a_position];

    /* Shuffle things backward to fill the gap. */
    for (i = a_position + 1; i < a_treen->num_children; i++)
    {
      a_treen->children[i - 1] = a_treen->children[i];
    }

    a_treen->num_children--;

    /* Truncate the array. */
    if (a_treen->num_children == 0)
    {
      _cw_free(a_treen->children);
      a_treen->children = NULL;
    }
    else
    {
      a_treen->children = _cw_realloc(a_treen->children,
					a_treen->num_children
					* sizeof(cw_treen_t *));
    }

#ifdef _CW_REENTRANT
    if (a_treen->is_thread_safe)
    {
      mtx_unlock(&a_treen->lock);
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
treen_get_child_ptr(cw_treen_t * a_treen, cw_uint32_t a_position,
		    cw_treen_t ** a_child)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_treen);
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_lock(&a_treen->lock);
  }
#endif

  if (a_position < a_treen->num_children)
  {
    retval = FALSE;
    *a_child = a_treen->children[a_position];
  }
  else
  {
    /* Past end of child pointer array. */
    retval = TRUE;
  }
  
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_unlock(&a_treen->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Returns a pointer to the data for a_treen.
 *
 ****************************************************************************/
void *
treen_get_data_ptr(cw_treen_t * a_treen)
{
  void * retval;
  
  _cw_check_ptr(a_treen);
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_lock(&a_treen->lock);
  }
#endif

  retval = a_treen->data;
  
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_unlock(&a_treen->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Sets the pointer for data for a_treen.
 *
 ****************************************************************************/
void *
treen_set_data_ptr(cw_treen_t * a_treen, void * a_data)
{
  void * retval;
  
  _cw_check_ptr(a_treen);
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_lock(&a_treen->lock);
  }
#endif

  retval = a_treen->data;
  a_treen->data = a_data;
  
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_unlock(&a_treen->lock);
  }
#endif
  return retval;
}
