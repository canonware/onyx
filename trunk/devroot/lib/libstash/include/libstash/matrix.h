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

cw_matrix_t *
matrix_new(cw_matrix_t * a_matrix);

void
matrix_delete(cw_matrix_t * a_matrix);

cw_bool_t
matrix_init(cw_matrix_t * a_matrix, cw_uint32_t a_x_size,
	    cw_uint32_t a_y_size, cw_bool_t a_should_zero);

cw_bool_t
matrix_rebuild(cw_matrix_t * a_matrix);

#ifdef _LIBSTASH_DBG
cw_sint32_t
matrix_get_element(cw_matrix_t * a_matrix, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos);
#else
#  define matrix_get_element(a, b, c) (a)->grid[(a)->y_index[(c)] \
					       * (a)->grid_x_size \
					       + (a)->x_index[(b)]]
#endif

#ifdef _LIBSTASH_DBG
void
matrix_set_element(cw_matrix_t * a_matrix, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos, cw_sint32_t a_val);
#else
#  define matrix_set_element(a, b, c, d) (a)->grid[(a)->y_index[(c)] \
						  * (a)->grid_x_size \
						  + (a)->x_index[(b)]] = (d)
#endif

#ifdef _LIBSTASH_DBG
cw_uint32_t
matrix_get_x_size(cw_matrix_t * a_matrix);
#else
#  define matrix_get_x_size(a) (a)->x_size
#endif

#ifdef _LIBSTASH_DBG
cw_uint32_t
matrix_get_y_size(cw_matrix_t * a_matrix);
#else
#  define matrix_get_y_size(a) (a)->y_size
#endif

cw_matrix_t *
matrix_copy(cw_matrix_t * a_matrix);

void
matrix_dump(cw_matrix_t * a_matrix, cw_bool_t a_compact);

cw_bool_t
matrix_is_equal(cw_matrix_t * a_a, cw_matrix_t * a_b);

void
matrix_remove_row(cw_matrix_t * a_matrix, cw_uint32_t a_row);

void
matrix_remove_column(cw_matrix_t * a_matrix, cw_uint32_t a_column);
