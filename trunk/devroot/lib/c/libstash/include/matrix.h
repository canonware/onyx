/* -*-mode:c-*-
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
 *
 *
 ****************************************************************************/

#ifndef _MATRIX_H_
#define _MATRIX_H_

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

#define matrix_new _CW_NS_ANY(matrix_new)
#define matrix_delete _CW_NS_ANY(matrix_delete)
#define matrix_init _CW_NS_ANY(matrix_init)
#define matrix_rebuild _CW_NS_ANY(matrix_rebuild)
#ifdef _STASH_DBG
#  define matrix_get_element _CW_NS_ANY(matrix_get_element)
#  define matrix_set_element _CW_NS_ANY(matrix_set_element)
#  define matrix_get_x_size _CW_NS_ANY(matrix_get_x_size)
#  define matrix_get_y_size _CW_NS_ANY(matrix_get_y_size)
#else
#  define matrix_get_element(a, b, c) (a)->grid[(a)->y_index[(c)] \
					       * (a)->grid_x_size \
					       + (a)->x_index[(b)]]
#  define matrix_set_element(a, b, c, d) (a)->grid[(a)->y_index[(c)] \
						  * (a)->grid_x_size \
						  + (a)->x_index[(b)]] = (d)
#  define matrix_get_x_size(a) (a)->x_size
#  define matrix_get_y_size(a) (a)->y_size
#endif
#define matrix_copy _CW_NS_ANY(matrix_copy)
#define matrix_dump _CW_NS_ANY(matrix_dump)
#define matrix_is_equal _CW_NS_ANY(matrix_is_equal)
#define matrix_remove_row _CW_NS_ANY(matrix_remove_row)
#define matrix_remove_column _CW_NS_ANY(matrix_remove_column)

cw_matrix_t * matrix_new(cw_matrix_t * a_matrix_o);
void matrix_delete(cw_matrix_t * a_matrix_o);
void matrix_init(cw_matrix_t * a_matrix_o, cw_uint32_t a_x_size,
		 cw_uint32_t a_y_size, cw_bool_t a_should_zero);
void matrix_rebuild(cw_matrix_t * a_matrix_o);
#ifdef _STASH_DBG
cw_sint32_t matrix_get_element(cw_matrix_t * a_matrix_o, cw_uint32_t a_x_pos,
			       cw_uint32_t a_y_pos);
void matrix_set_element(cw_matrix_t * a_matrix_o, cw_uint32_t a_x_pos,
			cw_uint32_t a_y_pos, cw_sint32_t a_val);
cw_uint32_t matrix_get_x_size(cw_matrix_t * a_matrix_o);
cw_uint32_t matrix_get_y_size(cw_matrix_t * a_matrix_o);
#endif
cw_matrix_t * matrix_copy(cw_matrix_t * a_matrix_o);
void matrix_dump(cw_matrix_t * a_matrix_o, cw_bool_t a_compact);
cw_bool_t matrix_is_equal(cw_matrix_t * a_a, cw_matrix_t * a_b);
void matrix_remove_row(cw_matrix_t * a_matrix_o, cw_uint32_t a_row);
void matrix_remove_column(cw_matrix_t * a_matrix_o, cw_uint32_t a_column);

#endif /* _MATRIX_H_ */
