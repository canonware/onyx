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
 ****************************************************************************/

#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include "libstash/treen_p.h"

cw_treen_t *
treen_new(void)
{
  return treen_p_new(FALSE);
}

cw_treen_t *
treen_new_r(void)
{
  return treen_p_new(FALSE);
}

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
    if (0 < a_treen->num_children)
    {
      _cw_free(a_treen->children);
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
      if (NULL == a_treen->children)
      {
	retval = TRUE;
	goto RETURN;
      }
    }
    else
    {
      void * t_ptr;

      
      t_ptr = _cw_realloc(a_treen->children,
			  a_treen->num_children
			  * sizeof(cw_treen_t *));
      if (NULL == t_ptr)
      {
	retval = TRUE;
	goto RETURN;
      }
      else
      {
	a_treen->children = (cw_treen_t **) t_ptr;
      }
    }
    
    /* Shuffle things forward to make room. */
    for (i = (a_treen->num_children - 1); i > a_position; i--)
    {
      a_treen->children[i] = a_treen->children[i - 1];
    }

    /* Plop the new child pointer in place. */
    a_treen->children[a_position] = a_child;
  }

  RETURN:
#ifdef _CW_REENTRANT
  if (a_treen->is_thread_safe)
  {
    mtx_unlock(&a_treen->lock);
  }
#endif
  return retval;
}

cw_bool_t
treen_unlink_child(cw_treen_t * a_treen, cw_uint32_t a_position,
		   cw_treen_t ** r_child)
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
    
    *r_child = a_treen->children[a_position];

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
      /* Shrinking, so there should be no error. */
      _cw_check_ptr(a_treen->children);
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

cw_bool_t
treen_get_child_ptr(cw_treen_t * a_treen, cw_uint32_t a_position,
		    cw_treen_t ** r_child)
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
    *r_child = a_treen->children[a_position];
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

static cw_treen_t *
treen_p_new(cw_bool_t a_is_thread_safe)
{
  cw_treen_t * retval;

  retval = (cw_treen_t *) _cw_malloc(sizeof(cw_treen_t));
  if (NULL == retval)
  {
    goto RETURN;
  }
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

  RETURN:
  return retval;
}
