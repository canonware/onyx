/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Implementation of matrices.  Each cell is a signed 8 bit integer.
 *
 ****************************************************************************/

#include <limits.h>

#include "cover.h"

cw_matrix_t *
matrix_new(cw_matrix_t *a_matrix)
{
	cw_matrix_t	*retval;

	if (a_matrix != NULL) {
		retval = a_matrix;
		bzero(retval, sizeof(cw_matrix_t));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_matrix_t *)_cw_malloc(sizeof(cw_matrix_t));
		if (retval == NULL)
			goto RETURN;
		bzero(retval, sizeof(cw_matrix_t));
		retval->is_malloced = TRUE;
	}

	RETURN:
	return retval;
}

void
matrix_delete(cw_matrix_t *a_matrix)
{
	_cw_check_ptr(a_matrix);

	if (a_matrix->grid != NULL) {
		_cw_free(a_matrix->grid);
		_cw_free(a_matrix->x_index);
		_cw_free(a_matrix->y_index);
	}

	if (a_matrix->is_malloced)
		_cw_free(a_matrix);
}

cw_bool_t
matrix_init(cw_matrix_t *a_matrix, cw_uint32_t a_x_size, cw_uint32_t a_y_size,
    cw_bool_t a_should_zero)
{
	cw_bool_t	retval;
	cw_uint32_t	i;
	void		*t_ptr;
  
	_cw_check_ptr(a_matrix);
	_cw_assert(a_x_size != 0);
	_cw_assert(a_y_size != 0);

	if (a_matrix->grid == NULL) {
		/* Fresh start, create a matrix. */
		t_ptr = _cw_malloc(sizeof(cw_sint8_t) * a_x_size * a_y_size);
		if (t_ptr == NULL) {
			retval = TRUE;
			goto RETURN;
		} else
			a_matrix->grid = (cw_sint8_t *)t_ptr;

		t_ptr = _cw_malloc(sizeof(cw_uint32_t) * a_x_size);
		if (t_ptr == NULL) {
			_cw_free(a_matrix->grid);
			a_matrix->grid = NULL;
			retval = TRUE;
			goto RETURN;
		} else
			a_matrix->x_index = (cw_uint32_t *)t_ptr;

		t_ptr = _cw_malloc(sizeof(cw_uint32_t) * a_y_size);
		if (t_ptr == NULL) {
			_cw_free(a_matrix->grid);
			a_matrix->grid = NULL;
			_cw_free(a_matrix->x_index);
			a_matrix->x_index = NULL; /* Probably not necessary. */
			retval = TRUE;
			goto RETURN;
		} else
			a_matrix->y_index = (cw_uint32_t *)t_ptr;
	}
	else if ((a_matrix->grid_x_size == a_x_size) && (a_matrix->grid_y_size
	    == a_y_size)) {
		/* Same size as existing matrix, so just bzero() it. */
	} else {
		/* realloc(). */
		t_ptr = _cw_realloc(a_matrix->grid, sizeof(cw_bool_t) * a_x_size
		    * a_y_size);
		if (t_ptr == NULL) {
			retval = TRUE;
			goto RETURN;
		} else
			a_matrix->grid = (cw_sint8_t *)t_ptr;

		t_ptr = _cw_realloc(a_matrix->x_index, sizeof(cw_uint32_t) *
		    a_x_size);
		if (t_ptr == NULL) {
			retval = TRUE;
			goto RETURN;
		} else
			a_matrix->x_index = (cw_uint32_t *)t_ptr;

		t_ptr = _cw_realloc(a_matrix->y_index, sizeof(cw_uint32_t) *
		    a_y_size);
		if (t_ptr == NULL) {
			retval = TRUE;
			goto RETURN;
		} else
			a_matrix->y_index = (cw_uint32_t *)t_ptr;
	}
  
	a_matrix->x_size
	    = a_matrix->grid_x_size
	    = a_x_size;
	a_matrix->y_size
	    = a_matrix->grid_y_size
	    = a_y_size;

	/* bzero() the matrix and initialize the indices. */
	if (a_should_zero) {
		bzero(a_matrix->grid, sizeof(cw_sint8_t) * a_x_size *
		    a_y_size);
	}
  
	for (i = 0; i < a_x_size; i++)
		a_matrix->x_index[i] = i;
	for (i = 0; i < a_y_size; i++)
		a_matrix->y_index[i] = i;
  
	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
matrix_rebuild(cw_matrix_t *a_matrix)
{
	cw_bool_t	retval;
	void		*t_ptr;
  
	_cw_check_ptr(a_matrix);

	if ((a_matrix->x_size != a_matrix->grid_x_size)
	    || (a_matrix->y_size != a_matrix->grid_y_size)) {
		cw_sint8_t	*old_grid;
		cw_sint32_t	i, j;

		/* Create new grid and indices. */
		old_grid = a_matrix->grid;
		t_ptr = _cw_malloc(sizeof(cw_sint8_t)
		    * a_matrix->x_size
		    * a_matrix->y_size);
		if (t_ptr == NULL) {
			retval = TRUE;
			goto RETURN;
		} else
			a_matrix->grid = (cw_sint8_t *)t_ptr;

		/* Copy valid parts of old grid to new grid. */
		for (j = 0; j < a_matrix->y_size; j++) {
			for (i = 0; i < a_matrix->x_size; i++) {
				a_matrix->grid[a_matrix->x_size * j + i] =
				    old_grid[(a_matrix->grid_x_size *
				    a_matrix->y_index[j]) +
				    a_matrix->x_index[i]];
			}
		}

		/* Free memory pointed to by old_grid. */
		_cw_free(old_grid);

		a_matrix->x_index
		    = (cw_uint32_t *)_cw_realloc(a_matrix->x_index,
		    sizeof(cw_uint32_t) * a_matrix->x_size);
		/*
		 * We're never growing the allocation, so we should never
		 * fail.
		 */
		_cw_check_ptr(a_matrix->x_index);
    
		for (i = 0; i < a_matrix->x_size; i++)
			a_matrix->x_index[i] = i;

		a_matrix->y_index
		    = (cw_uint32_t *)_cw_realloc(a_matrix->y_index,
		    sizeof(cw_uint32_t) * a_matrix->y_size);
		/*
		 * We're never growing the allocation, so we should never
		 * fail.
		 */
		_cw_check_ptr(a_matrix->y_index);
    
		for (i = 0; i < a_matrix->y_size; i++)
			a_matrix->y_index[i] = i;
    
		/* Finish fixing things up. */
		a_matrix->grid_x_size = a_matrix->x_size;
		a_matrix->grid_y_size = a_matrix->y_size;
	}
  
	retval = FALSE;
	RETURN:
	return retval;
}

#ifdef _COVER_DBG
cw_sint8_t
matrix_get_element(cw_matrix_t *a_matrix, cw_uint32_t a_x_pos, cw_uint32_t
    a_y_pos)
{
	cw_sint8_t	retval;
  
	_cw_check_ptr(a_matrix);
	_cw_assert(a_x_pos < a_matrix->x_size);
	_cw_assert(a_y_pos < a_matrix->y_size);

	retval = a_matrix->grid[a_matrix->y_index[a_y_pos]
	    * a_matrix->grid_x_size
	    + a_matrix->x_index[a_x_pos]];

	return retval;
}

void
matrix_set_element(cw_matrix_t *a_matrix, cw_uint32_t a_x_pos, cw_uint32_t
    a_y_pos, cw_sint8_t a_val)
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
matrix_get_x_size(cw_matrix_t *a_matrix)
{
	_cw_check_ptr(a_matrix);
	return a_matrix->x_size;
}
cw_uint32_t
matrix_get_y_size(cw_matrix_t *a_matrix)
{
	_cw_check_ptr(a_matrix);
	return a_matrix->y_size;
}
#endif

cw_matrix_t *
matrix_copy(cw_matrix_t *a_matrix)
{
	cw_matrix_t	*retval;
	cw_uint32_t	i, j;
  
	_cw_check_ptr(a_matrix);

	retval = matrix_new(NULL);
	if (retval == NULL)
		goto RETURN;

	if (a_matrix->grid != NULL) {
		if (matrix_init(retval, matrix_get_x_size(a_matrix),
		    matrix_get_y_size(a_matrix), FALSE)) {
			matrix_delete(retval);
			retval = NULL;
			goto RETURN;
		}

		/* Copy the grid. */
		for (j = 0; j < retval->grid_y_size; j++) {
			for (i = 0; i < retval->grid_x_size; i++) {
				matrix_set_element(retval, i, j,
				    matrix_get_element(a_matrix, i, j));
			}
		}
	}

	RETURN:
	return retval;
}

void
matrix_dump(cw_matrix_t *a_matrix, const char *a_prefix, cw_bool_t a_compact)
{
	_cw_check_ptr(a_matrix);

	if (a_matrix->grid != NULL) {
		cw_uint32_t	i, j;
    
		if (a_compact) {
			for (j = 0; j < a_matrix->y_size; j++) {
				out_put(out_err, "[s]", a_prefix);
				for (i = 0; i < a_matrix->x_size; i++) {
					out_put(out_err, "[s]",
					    matrix_get_element(a_matrix, i, j) ?
					    "X" : ".");
				}
				out_put(out_err, "\n");
			}
		} else {
			cw_uint32_t	k, x_digits, y_digits, t_len, x, y;
			cw_uint32_t	greatest;
			char		t_str[20];

			/* Figure out the maximum length of the labels. */
			y_digits = _cw_out_put_s(t_str, "[i]",
			    a_matrix->y_size -1);
	
			/* Figure out the maximum length of any field. */
			for (y = 0, greatest = 0; y < a_matrix->y_size; y++) {
				for (x = 0; x < a_matrix->x_size; x++) {
					if (matrix_get_element(a_matrix, x, y) >
					    greatest) {
						greatest =
						    matrix_get_element(a_matrix,
						    x, y);
					}
				}
			}
			if (greatest > (a_matrix->x_size - 1)) {
				x_digits = _cw_out_put_s(t_str, "[i]",
				    greatest);
			} else {
				x_digits = _cw_out_put_s(t_str, "[i]",
				    a_matrix->x_size - 1);
			}

			/* Top labels. */
			out_put(out_err, "[s]", a_prefix);
			for (k = 0; k < y_digits; k++)
				out_put(out_err, " ");
			out_put(out_err, "|");
			for (i = 0; i < a_matrix->x_size; i++) {
				_cw_out_put_s(t_str, "[i]", i);
				t_len = strlen(t_str);
				for (k = 0; k < ((x_digits + 1) - t_len); k++)
					out_put(out_err, " ");
				out_put(out_err, "[i]", i);
			}
			out_put(out_err, "\n");

			/* Top horizontal line. */
			out_put(out_err, "[s]", a_prefix);
			for (k = 0; k < y_digits; k++)
				out_put(out_err, "-");
			out_put(out_err, "+");
			for (i = 0; i < (a_matrix->x_size * (x_digits + 1));
			     i++)
				out_put(out_err, "-");
			out_put(out_err, "\n");

			for (j = 0; j < a_matrix->y_size; j++) {
				out_put(out_err, "[s]", a_prefix);
				/* Side label. */
				_cw_out_put_s(t_str, "[i]", j);
				t_len = strlen(t_str);
				for (k = 0; k < ((y_digits) - t_len); k++)
					out_put(out_err, " ");
				out_put(out_err, "[i]|", j);

				/* Matrix elements. */
				for (i = 0; i < a_matrix->x_size; i++) {
					for (k = 0; k < (x_digits + 1 -
					    _cw_out_put_s(t_str, "[i]",
					    matrix_get_element(a_matrix, i,
					    j))); k++)
						out_put(out_err, " ");
					out_put(out_err, "[i|s:s]",
					    matrix_get_element(a_matrix, i, j));
				}
	
				out_put(out_err, "\n");
			}
		}
	}
	else
		out_put(out_err, "Invalid matrix\n");
}

cw_bool_t
matrix_is_equal(cw_matrix_t *a_a, cw_matrix_t *a_b)
{
	cw_bool_t	retval;
	cw_uint32_t	i, j;
  
	_cw_check_ptr(a_a);
	_cw_check_ptr(a_b);

	if (a_a != a_b) {
		if ((a_a->grid == NULL) || (a_b->grid == NULL) || (a_a->x_size
		    != a_b->x_size) || (a_a->y_size != a_b->y_size)) {
			retval = FALSE;
			goto RETURN;
		}

		for (j = 0; j < a_a->y_size; j++) {
			for (i = 0; i < a_a->x_size; i++) {
				if (matrix_get_element(a_a, i, j) !=
				    matrix_get_element(a_b, i, j)) {
					retval = FALSE;
					goto RETURN;
				}
			}
		}
	}

	retval = TRUE;
	RETURN:
	return retval;
}

void
matrix_remove_row(cw_matrix_t *a_matrix, cw_uint32_t a_row)
{
	cw_uint32_t	j;

	_cw_check_ptr(a_matrix);
	_cw_assert(a_row < matrix_get_y_size(a_matrix));

	for (j = a_row; j < (a_matrix->y_size - 1); j++)
		a_matrix->y_index[j] = a_matrix->y_index[j + 1];
	a_matrix->y_size--;
}

void
matrix_remove_column(cw_matrix_t *a_matrix, cw_uint32_t a_column)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_matrix);
	_cw_assert(a_column < matrix_get_x_size(a_matrix));

	for (i = a_column; i < (a_matrix->x_size - 1); i++)
		a_matrix->x_index[i] = a_matrix->x_index[i + 1];
	a_matrix->x_size--;
}
