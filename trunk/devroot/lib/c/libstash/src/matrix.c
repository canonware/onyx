/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 167 $
 * $Date: 1998-08-12 15:02:36 -0700 (Wed, 12 Aug 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_MATRIX_H_
#include <libstash.h>

#include <string.h>
#include <limits.h>

cw_matrix_t *
matrix_new(cw_matrix_t * a_matrix_o)
{
  cw_matrix_t * retval;

  if (a_matrix_o != NULL)
  {
    retval = a_matrix_o;
    bzero(retval, sizeof(cw_matrix_t));
    retval->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_matrix_t *) _cw_malloc(sizeof(cw_matrix_t));
    bzero(retval, sizeof(cw_matrix_t));
    retval->is_malloced = TRUE;
  }
  
  return retval;
}

void
matrix_delete(cw_matrix_t * a_matrix_o)
{
  _cw_check_ptr(a_matrix_o);

  if (a_matrix_o->grid != NULL)
  {
    _cw_free(a_matrix_o->grid);
    _cw_free(a_matrix_o->x_index);
    _cw_free(a_matrix_o->y_index);
  }

  if (a_matrix_o->is_malloced == TRUE)
  {
    _cw_free(a_matrix_o);
  }
}

void
matrix_init(cw_matrix_t * a_matrix_o, cw_uint32_t a_x_size,
	    cw_uint32_t a_y_size, cw_bool_t a_should_zero)
{
  _cw_check_ptr(a_matrix_o);
  _cw_assert(a_x_size != 0);
  _cw_assert(a_y_size != 0);

  if (a_matrix_o->grid == NULL)
  {
    /* Fresh start, create a matrix. */
    a_matrix_o->grid = (cw_sint32_t *) _cw_malloc(sizeof(cw_sint32_t)
						  * a_x_size * a_y_size);
    a_matrix_o->x_index = (cw_uint32_t *) _cw_malloc(sizeof(cw_uint32_t)
						     * a_x_size);
    a_matrix_o->y_index = (cw_uint32_t *) _cw_malloc(sizeof(cw_uint32_t)
						     * a_y_size);
  }
  else if ((a_x_size == a_matrix_o->grid_x_size)
	   && (a_y_size == a_matrix_o->grid_y_size))
  {
    /* Same size as existing matrix, so just bzero() it. */
  }
  else
  {
    /* realloc(). */
    a_matrix_o->grid = (cw_sint32_t *) _cw_realloc(a_matrix_o->grid,
						   sizeof(cw_bool_t)
						   * a_x_size * a_y_size);
    a_matrix_o->x_index = (cw_uint32_t *) _cw_realloc(a_matrix_o->x_index,
						      sizeof(cw_uint32_t)
						      * a_x_size);
    a_matrix_o->y_index = (cw_uint32_t *) _cw_realloc(a_matrix_o->y_index,
						      sizeof(cw_uint32_t)
						      * a_y_size);
  }
  
  a_matrix_o->x_size
    = a_matrix_o->grid_x_size
    = a_x_size;
  a_matrix_o->y_size
    = a_matrix_o->grid_y_size
    = a_y_size;

  /* bzero() the matrix and initialize the indices. */
  if (a_should_zero == TRUE)
  {
    bzero(a_matrix_o->grid, sizeof(cw_sint32_t) * a_x_size * a_y_size);
  }
  
  {
    cw_uint32_t i;
    
    for (i = 0; i < a_x_size; i++)
    {
      a_matrix_o->x_index[i] = i;
    }
    for (i = 0; i < a_y_size; i++)
    {
      a_matrix_o->y_index[i] = i;
    }
  }
}

void
matrix_rebuild(cw_matrix_t * a_matrix_o)
{
  _cw_check_ptr(a_matrix_o);

  if ((a_matrix_o->x_size != a_matrix_o->grid_x_size)
      || (a_matrix_o->y_size != a_matrix_o->grid_y_size))
  {
    cw_sint32_t * old_grid, i, j;

    /* Create new grid and indices. */
    old_grid = a_matrix_o->grid;
    a_matrix_o->grid = (cw_sint32_t *) _cw_malloc(sizeof(cw_sint32_t)
						  * a_matrix_o->x_size
						  * a_matrix_o->y_size);

    /* Copy valid parts of old grid to new grid. */
    for (j = 0; j < a_matrix_o->y_size; j++)
    {
      for (i = 0; i < a_matrix_o->x_size; i++)
      {
	a_matrix_o->grid[a_matrix_o->x_size * j + i]
	  = old_grid[(a_matrix_o->x_index[i]
		      * a_matrix_o->grid_x_size)
		    + a_matrix_o->y_index[j]];
      }
    }

    /* Free memory pointed to by old_grid. */
    _cw_free(old_grid);

    a_matrix_o->x_index
      = (cw_uint32_t *) _cw_realloc(a_matrix_o->x_index,
				    sizeof(cw_uint32_t) * a_matrix_o->x_size);
    for (i = 0; i < a_matrix_o->x_size; i++)
    {
      a_matrix_o->x_index[i] = i;
    }

    a_matrix_o->y_index
      = (cw_uint32_t *) _cw_realloc(a_matrix_o->y_index,
				    sizeof(cw_uint32_t) * a_matrix_o->y_size);
    for (i = 0; i < a_matrix_o->y_size; i++)
    {
      a_matrix_o->y_index[i] = i;
    }
    
    /* Finish fixing things up. */
    a_matrix_o->grid_x_size = a_matrix_o->x_size;
    a_matrix_o->grid_y_size = a_matrix_o->y_size;
  }
}

#ifdef _CW_DEBUG
cw_sint32_t
matrix_get_element(cw_matrix_t * a_matrix_o, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos)
{
  cw_sint32_t retval;
  
  _cw_check_ptr(a_matrix_o);
  _cw_assert(a_x_pos < a_matrix_o->x_size);
  _cw_assert(a_y_pos < a_matrix_o->y_size);

  retval = a_matrix_o->grid[a_matrix_o->y_index[a_y_pos]
			   * a_matrix_o->grid_x_size
			   + a_matrix_o->x_index[a_x_pos]];

  return retval;
}

void
matrix_set_element(cw_matrix_t * a_matrix_o, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos, cw_sint32_t a_val)
{
  _cw_check_ptr(a_matrix_o);
  _cw_assert(a_x_pos < a_matrix_o->x_size);
  _cw_assert(a_y_pos < a_matrix_o->y_size);

  a_matrix_o->grid[a_matrix_o->y_index[a_y_pos]
		  * a_matrix_o->grid_x_size
		  + a_matrix_o->x_index[a_x_pos]]
    = a_val;
}

cw_uint32_t
matrix_get_x_size(cw_matrix_t * a_matrix_o)
{
  _cw_check_ptr(a_matrix_o);

  return a_matrix_o->x_size;
}
cw_uint32_t
matrix_get_y_size(cw_matrix_t * a_matrix_o)
{
  _cw_check_ptr(a_matrix_o);

  return a_matrix_o->y_size;
}
#endif

cw_matrix_t *
matrix_copy(cw_matrix_t * a_matrix_o)
{
  cw_matrix_t * retval;
  cw_uint32_t i, j;
  
  _cw_check_ptr(a_matrix_o);

  retval = matrix_new(NULL);

  if (a_matrix_o->grid != NULL)
  {
    matrix_init(retval, matrix_get_x_size(a_matrix_o),
		matrix_get_y_size(a_matrix_o), FALSE);

    /* Copy the grid. */
    for (j = 0; j < retval->grid_y_size; j++)
    {
      for (i = 0; i < retval->grid_x_size; i++)
      {
	matrix_set_element(retval, i, j,
			   matrix_get_element(a_matrix_o, i, j));
      }
    }
  }

  return retval;
}

void
matrix_dump(cw_matrix_t * a_matrix_o, cw_bool_t a_compact)
{
  _cw_check_ptr(a_matrix_o);

  if (a_matrix_o->grid != NULL)
  {
    cw_uint32_t i, j;
    
    if (a_compact == TRUE)
    {
      for (j = 0; j < a_matrix_o->y_size; j++)
      {
	for (i = 0; i < a_matrix_o->x_size; i++)
	{
	  log_printf(g_log_o, "%s",
		     matrix_get_element(a_matrix_o, i, j) ? "X" : ".");
	}
	log_printf(g_log_o, "\n");
      }
    }
    else
    {
      cw_uint32_t k, x_digits, y_digits, t_len, x, y, greatest;
      char t_str[20];

      /* Figure out the maximum length of the labels. */
      y_digits = sprintf(t_str, "%u", a_matrix_o->y_size -1);
	
      /* Figure out the maximum length of any field. */
      for (y = 0, greatest = 0; y < a_matrix_o->y_size; y++)
      {
	for (x = 0; x < a_matrix_o->x_size; x++)
	{
	  if (matrix_get_element(a_matrix_o, x, y) > greatest)
	  {
	    greatest = matrix_get_element(a_matrix_o, x, y);
	  }
	}
      }
      if (greatest > (a_matrix_o->x_size - 1))
      {
	x_digits = sprintf(t_str, "%u", greatest);
      }
      else
      {
	x_digits = sprintf(t_str, "%u", a_matrix_o->x_size - 1);
      }

      /* Top labels. */
      for (k = 0; k < y_digits; k++)
      {
	log_printf(g_log_o, " ");
      }
      log_printf(g_log_o, "|");
      for (i = 0; i < a_matrix_o->x_size; i++)
      {
	sprintf(t_str, "%u", i);
	t_len = strlen(t_str);
	for (k = 0; k < ((x_digits + 1) - t_len); k++)
	{
	  log_printf(g_log_o, " ");
	}
	log_printf(g_log_o, "%u", i);
      }
      log_printf(g_log_o, "\n");

      /* Top horizontal line. */
      for (k = 0; k < y_digits; k++)
      {
	log_printf(g_log_o, "-");
      }
      log_printf(g_log_o, "+");
      for (i = 0; i < (a_matrix_o->x_size * (x_digits + 1)); i++)
      {
	log_printf(g_log_o, "-");
      }
      log_printf(g_log_o, "\n");

      for (j = 0; j < a_matrix_o->y_size; j++)
      {
	/* Side label. */
	sprintf(t_str, "%u", j);
	t_len = strlen(t_str);
	for (k = 0; k < ((y_digits) - t_len); k++)
	{
	  log_printf(g_log_o, " ");
	}
	log_printf(g_log_o, "%u|", j);

	/* Matrix elements. */
	for (i = 0; i < a_matrix_o->x_size; i++)
	{
	  for (k = 0;
	       k < (x_digits + 1 - sprintf(t_str, "%u",
					   matrix_get_element(a_matrix_o,
							      i, j)));
	       k++)
	  {
	    log_printf(g_log_o, " ");
	  }
	  log_printf(g_log_o, "%d",
		     matrix_get_element(a_matrix_o, i, j));
	}
	
	log_printf(g_log_o, "\n");
      }
    }
  }
  else
  {
    log_printf(g_log_o, "Invalid matrix\n");
  }
}

cw_bool_t
matrix_is_equal(cw_matrix_t * a_a, cw_matrix_t * a_b)
{
  cw_uint32_t i, j;
  
  _cw_check_ptr(a_a);
  _cw_check_ptr(a_b);

  if (a_a == a_b)
  {
    return TRUE;
  }

  if ((a_a->grid == NULL)
      || (a_b->grid == NULL)
      || (a_a->x_size != a_b->x_size)
      || (a_a->y_size != a_b->y_size))
  {
    return FALSE;
  }

  for (j = 0; j < a_a->y_size; j++)
  {
    for (i = 0; i < a_a->x_size; i++)
    {
      if (matrix_get_element(a_a, i, j) != matrix_get_element(a_b, i, j))
      {
	return FALSE;
      }
    }
  }

  return TRUE;
}

void
matrix_remove_row(cw_matrix_t * a_matrix_o, cw_uint32_t a_row)
{
  cw_uint32_t j;

  _cw_check_ptr(a_matrix_o);
  _cw_assert(a_row < matrix_get_x_size(a_matrix_o));

  for (j = a_row; j < (a_matrix_o->y_size - 1); j++)
  {
    a_matrix_o->y_index[j] = a_matrix_o->y_index[j + 1];
  }
  a_matrix_o->y_size--;
}

void
matrix_remove_column(cw_matrix_t * a_matrix_o, cw_uint32_t a_column)
{
  cw_uint32_t i;

  _cw_check_ptr(a_matrix_o);
  _cw_assert(a_column < matrix_get_x_size(a_matrix_o));

  for (i = a_column; i < (a_matrix_o->x_size - 1); i++)
  {
    a_matrix_o->x_index[i] = a_matrix_o->x_index[i + 1];
  }
  a_matrix_o->x_size--;
}
