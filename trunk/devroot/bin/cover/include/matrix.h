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
 *
 *
 ****************************************************************************/

typedef struct cw_matrix_s cw_matrix_t;

struct cw_matrix_s
{
  cw_bool_t is_malloced;
  cw_sint32_t * grid;
  cw_uint32_t * x_index;
  cw_uint32_t * y_index;
  cw_uint32_t grid_x_size;
  cw_uint32_t grid_y_size;
  cw_uint32_t x_size;
  cw_uint32_t y_size;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to space for a matrix, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a matrix, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_matrix_t *
matrix_new(cw_matrix_t * a_matrix);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
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
matrix_delete(cw_matrix_t * a_matrix);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * a_x_size : Number of columns.
 *
 * a_y_size : Number of rows.
 *
 * a_should_zero : FALSE == do not initialize, TRUE == initialize all table
 *                 elements to zero.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Initialize a_matrix.
 *
 ****************************************************************************/
cw_bool_t
matrix_init(cw_matrix_t * a_matrix, cw_uint32_t a_x_size,
	    cw_uint32_t a_y_size, cw_bool_t a_should_zero);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Rebuild (compact) the matrix.
 *
 ****************************************************************************/
cw_bool_t
matrix_rebuild(cw_matrix_t * a_matrix);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * a_x_pos : Column number (0..n).
 *
 * a_y_pos : Row number (0..n).
 *
 * <<< Output(s) >>>
 *
 * retval : Element value.
 *
 * <<< Description >>>
 *
 * Get the value of the element at (a_x_pos, a_y_pos).
 *
 ****************************************************************************/
#ifdef _LIBSTASH_DBG
cw_sint32_t
matrix_get_element(cw_matrix_t * a_matrix, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos);
#else
#  define matrix_get_element(a, b, c) (a)->grid[(a)->y_index[(c)] \
					       * (a)->grid_x_size \
					       + (a)->x_index[(b)]]
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * a_x_pos : Column number (0..n).
 *
 * a_y_pos : Row number (0..n).
 *
 * a_val : Element value.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set the element at (a_x_pos, a_y_pos) to a_val.
 *
 ****************************************************************************/
#ifdef _LIBSTASH_DBG
void
matrix_set_element(cw_matrix_t * a_matrix, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos, cw_sint32_t a_val);
#else
#  define matrix_set_element(a, b, c, d) (a)->grid[(a)->y_index[(c)] \
						  * (a)->grid_x_size \
						  + (a)->x_index[(b)]] = (d)
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of columns.
 *
 * <<< Description >>>
 *
 * Get number of columns.
 *
 ****************************************************************************/
#ifdef _LIBSTASH_DBG
cw_uint32_t
matrix_get_x_size(cw_matrix_t * a_matrix);
#else
#  define matrix_get_x_size(a) (a)->x_size
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of rows.
 *
 * <<< Description >>>
 *
 * Get number of rows.
 *
 ****************************************************************************/
#ifdef _LIBSTASH_DBG
cw_uint32_t
matrix_get_y_size(cw_matrix_t * a_matrix);
#else
#  define matrix_get_y_size(a) (a)->y_size
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a matrix, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Make a copy of a_matrix and return a pointer to it.
 *
 ****************************************************************************/
cw_matrix_t *
matrix_copy(cw_matrix_t * a_matrix);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * a_compact : If TRUE, print a compact output format.  If FALSE, print a less
 *             compact (but guaranteed to have the columns aligned) format.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Print a_matrix to cw_g_log.
 *
 ****************************************************************************/
void
matrix_dump(cw_matrix_t * a_matrix, cw_bool_t a_compact);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a matrix.
 *
 * a_b : Pointer to a matrix.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == not equal, TRUE == equal.
 *
 * <<< Description >>>
 *
 * Compare a_a and a_b for equality.
 *
 ****************************************************************************/
cw_bool_t
matrix_is_equal(cw_matrix_t * a_a, cw_matrix_t * a_b);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * a_row : Row number (0..n).
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Remove row a_row from a_matrix.
 *
 ****************************************************************************/
void
matrix_remove_row(cw_matrix_t * a_matrix, cw_uint32_t a_row);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_matrix : Pointer to a matrix.
 *
 * a_column : Column number (0..n).
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Remove column a_column from a_matrix.
 *
 ****************************************************************************/
void
matrix_remove_column(cw_matrix_t * a_matrix, cw_uint32_t a_column);
