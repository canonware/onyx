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
 * Implementation of matrices.  Each cell is a signed 32 bit integer.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_MATRIX
#include "libstash/libstash.h"

#include <limits.h>

cw_matrix_t *
matrix_new(cw_matrix_t * a_matrix)
{
  cw_matrix_t * retval;

  if (a_matrix != NULL)
  {
    retval = a_matrix;
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
matrix_delete(cw_matrix_t * a_matrix)
{
  _cw_check_ptr(a_matrix);

  if (a_matrix->grid != NULL)
  {
    _cw_free(a_matrix->grid);
    _cw_free(a_matrix->x_index);
    _cw_free(a_matrix->y_index);
  }

  if (a_matrix->is_malloced == TRUE)
  {
    _cw_free(a_matrix);
  }
}

void
matrix_init(cw_matrix_t * a_matrix, cw_uint32_t a_x_size,
	    cw_uint32_t a_y_size, cw_bool_t a_should_zero)
{
  _cw_check_ptr(a_matrix);
  _cw_assert(a_x_size != 0);
  _cw_assert(a_y_size != 0);

  if (a_matrix->grid == NULL)
  {
    /* Fresh start, create a matrix. */
    a_matrix->grid = (cw_sint32_t *) _cw_malloc(sizeof(cw_sint32_t)
						  * a_x_size * a_y_size);
    a_matrix->x_index = (cw_uint32_t *) _cw_malloc(sizeof(cw_uint32_t)
						     * a_x_size);
    a_matrix->y_index = (cw_uint32_t *) _cw_malloc(sizeof(cw_uint32_t)
						     * a_y_size);
  }
  else if ((a_x_size == a_matrix->grid_x_size)
	   && (a_y_size == a_matrix->grid_y_size))
  {
    /* Same size as existing matrix, so just bzero() it. */
  }
  else
  {
    /* realloc(). */
    a_matrix->grid = (cw_sint32_t *) _cw_realloc(a_matrix->grid,
						   sizeof(cw_bool_t)
						   * a_x_size * a_y_size);
    a_matrix->x_index = (cw_uint32_t *) _cw_realloc(a_matrix->x_index,
						      sizeof(cw_uint32_t)
						      * a_x_size);
    a_matrix->y_index = (cw_uint32_t *) _cw_realloc(a_matrix->y_index,
						      sizeof(cw_uint32_t)
						      * a_y_size);
  }
  
  a_matrix->x_size
    = a_matrix->grid_x_size
    = a_x_size;
  a_matrix->y_size
    = a_matrix->grid_y_size
    = a_y_size;

  /* bzero() the matrix and initialize the indices. */
  if (a_should_zero == TRUE)
  {
    bzero(a_matrix->grid, sizeof(cw_sint32_t) * a_x_size * a_y_size);
  }
  
  {
    cw_uint32_t i;
    
    for (i = 0; i < a_x_size; i++)
    {
      a_matrix->x_index[i] = i;
    }
    for (i = 0; i < a_y_size; i++)
    {
      a_matrix->y_index[i] = i;
    }
  }
}

void
matrix_rebuild(cw_matrix_t * a_matrix)
{
  _cw_check_ptr(a_matrix);

  if ((a_matrix->x_size != a_matrix->grid_x_size)
      || (a_matrix->y_size != a_matrix->grid_y_size))
  {
    cw_sint32_t * old_grid, i, j;

    /* Create new grid and indices. */
    old_grid = a_matrix->grid;
    a_matrix->grid = (cw_sint32_t *) _cw_malloc(sizeof(cw_sint32_t)
						  * a_matrix->x_size
						  * a_matrix->y_size);

    /* Copy valid parts of old grid to new grid. */
    for (j = 0; j < a_matrix->y_size; j++)
    {
      for (i = 0; i < a_matrix->x_size; i++)
      {
	a_matrix->grid[a_matrix->x_size * j + i]
	  = old_grid[(a_matrix->grid_x_size * a_matrix->y_index[j])
		    + a_matrix->x_index[i]];
      }
    }

    /* Free memory pointed to by old_grid. */
    _cw_free(old_grid);

    a_matrix->x_index
      = (cw_uint32_t *) _cw_realloc(a_matrix->x_index,
				    sizeof(cw_uint32_t) * a_matrix->x_size);
    for (i = 0; i < a_matrix->x_size; i++)
    {
      a_matrix->x_index[i] = i;
    }

    a_matrix->y_index
      = (cw_uint32_t *) _cw_realloc(a_matrix->y_index,
				    sizeof(cw_uint32_t) * a_matrix->y_size);
    for (i = 0; i < a_matrix->y_size; i++)
    {
      a_matrix->y_index[i] = i;
    }
    
    /* Finish fixing things up. */
    a_matrix->grid_x_size = a_matrix->x_size;
    a_matrix->grid_y_size = a_matrix->y_size;
  }
}

#ifdef _LIBSTASH_DBG
cw_sint32_t
matrix_get_element(cw_matrix_t * a_matrix, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos)
{
  cw_sint32_t retval;
  
  _cw_check_ptr(a_matrix);
  _cw_assert(a_x_pos < a_matrix->x_size);
  _cw_assert(a_y_pos < a_matrix->y_size);

  retval = a_matrix->grid[a_matrix->y_index[a_y_pos]
			   * a_matrix->grid_x_size
			   + a_matrix->x_index[a_x_pos]];

  return retval;
}

void
matrix_set_element(cw_matrix_t * a_matrix, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos, cw_sint32_t a_val)
{
  _cw_check_ptr(a_matrix);
  _cw_assert(a_x_pos < a_matrix->x_size);
  _cw_assert(a_y_pos < a_matrix->y_size);

  a_matrix->grid[a_matrix->y_index[a_y_pos]
		  * a_matrix->grid_x_size
		  + a_matrix->x_index[a_x_pos]]
    = a_val;
}

cw_uint32_t
matrix_get_x_size(cw_matrix_t * a_matrix)
{
  _cw_check_ptr(a_matrix);
  return a_matrix->x_size;
}
cw_uint32_t
matrix_get_y_size(cw_matrix_t * a_matrix)
{
  _cw_check_ptr(a_matrix);
  return a_matrix->y_size;
}
#endif

cw_matrix_t *
matrix_copy(cw_matrix_t * a_matrix)
{
  cw_matrix_t * retval;
  cw_uint32_t i, j;
  
  _cw_check_ptr(a_matrix);

  retval = matrix_new(NULL);

  if (a_matrix->grid != NULL)
  {
    matrix_init(retval, matrix_get_x_size(a_matrix),
		matrix_get_y_size(a_matrix), FALSE);

    /* Copy the grid. */
    for (j = 0; j < retval->grid_y_size; j++)
    {
      for (i = 0; i < retval->grid_x_size; i++)
      {
	matrix_set_element(retval, i, j,
			   matrix_get_element(a_matrix, i, j));
      }
    }
  }

  return retval;
}

void
matrix_dump(cw_matrix_t * a_matrix, cw_bool_t a_compact)
{
  _cw_check_ptr(a_matrix);

  if (a_matrix->grid != NULL)
  {
    cw_uint32_t i, j;
    
    if (a_compact == TRUE)
    {
      for (j = 0; j < a_matrix->y_size; j++)
      {
	for (i = 0; i < a_matrix->x_size; i++)
	{
	  log_printf(cw_g_log, "%s",
		     matrix_get_element(a_matrix, i, j) ? "X" : ".");
	}
	log_printf(cw_g_log, "\n");
      }
    }
    else
    {
      cw_uint32_t k, x_digits, y_digits, t_len, x, y, greatest;
      char t_str[20];

      /* Figure out the maximum length of the labels. */
      y_digits = sprintf(t_str, "%u", a_matrix->y_size -1);
	
      /* Figure out the maximum length of any field. */
      for (y = 0, greatest = 0; y < a_matrix->y_size; y++)
      {
	for (x = 0; x < a_matrix->x_size; x++)
	{
	  if (matrix_get_element(a_matrix, x, y) > greatest)
	  {
	    greatest = matrix_get_element(a_matrix, x, y);
	  }
	}
      }
      if (greatest > (a_matrix->x_size - 1))
      {
	x_digits = sprintf(t_str, "%u", greatest);
      }
      else
      {
	x_digits = sprintf(t_str, "%u", a_matrix->x_size - 1);
      }

      /* Top labels. */
      for (k = 0; k < y_digits; k++)
      {
	log_printf(cw_g_log, " ");
      }
      log_printf(cw_g_log, "|");
      for (i = 0; i < a_matrix->x_size; i++)
      {
	sprintf(t_str, "%u", i);
	t_len = strlen(t_str);
	for (k = 0; k < ((x_digits + 1) - t_len); k++)
	{
	  log_printf(cw_g_log, " ");
	}
	log_printf(cw_g_log, "%u", i);
      }
      log_printf(cw_g_log, "\n");

      /* Top horizontal line. */
      for (k = 0; k < y_digits; k++)
      {
	log_printf(cw_g_log, "-");
      }
      log_printf(cw_g_log, "+");
      for (i = 0; i < (a_matrix->x_size * (x_digits + 1)); i++)
      {
	log_printf(cw_g_log, "-");
      }
      log_printf(cw_g_log, "\n");

      for (j = 0; j < a_matrix->y_size; j++)
      {
	/* Side label. */
	sprintf(t_str, "%u", j);
	t_len = strlen(t_str);
	for (k = 0; k < ((y_digits) - t_len); k++)
	{
	  log_printf(cw_g_log, " ");
	}
	log_printf(cw_g_log, "%u|", j);

	/* Matrix elements. */
	for (i = 0; i < a_matrix->x_size; i++)
	{
	  for (k = 0;
	       k < (x_digits + 1 - sprintf(t_str, "%u",
					   matrix_get_element(a_matrix,
							      i, j)));
	       k++)
	  {
	    log_printf(cw_g_log, " ");
	  }
	  log_printf(cw_g_log, "%d",
		     matrix_get_element(a_matrix, i, j));
	}
	
	log_printf(cw_g_log, "\n");
      }
    }
  }
  else
  {
    log_printf(cw_g_log, "Invalid matrix\n");
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
matrix_remove_row(cw_matrix_t * a_matrix, cw_uint32_t a_row)
{
  cw_uint32_t j;

  _cw_check_ptr(a_matrix);
  _cw_assert(a_row < matrix_get_y_size(a_matrix));

  for (j = a_row; j < (a_matrix->y_size - 1); j++)
  {
    a_matrix->y_index[j] = a_matrix->y_index[j + 1];
  }
  a_matrix->y_size--;
}

void
matrix_remove_column(cw_matrix_t * a_matrix, cw_uint32_t a_column)
{
  cw_uint32_t i;

  _cw_check_ptr(a_matrix);
  _cw_assert(a_column < matrix_get_x_size(a_matrix));

  for (i = a_column; i < (a_matrix->x_size - 1); i++)
  {
    a_matrix->x_index[i] = a_matrix->x_index[i + 1];
  }
  a_matrix->x_size--;
}
