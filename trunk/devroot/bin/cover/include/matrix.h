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

#define matrix_new _CW_NS_LIBSTASH(matrix_new)
cw_matrix_t *
matrix_new(cw_matrix_t * a_matrix);

#define matrix_delete _CW_NS_LIBSTASH(matrix_delete)
void
matrix_delete(cw_matrix_t * a_matrix);

#define matrix_init _CW_NS_LIBSTASH(matrix_init)
void
matrix_init(cw_matrix_t * a_matrix, cw_uint32_t a_x_size,
	    cw_uint32_t a_y_size, cw_bool_t a_should_zero);

#define matrix_rebuild _CW_NS_LIBSTASH(matrix_rebuild)
void
matrix_rebuild(cw_matrix_t * a_matrix);

#ifdef _LIBSTASH_DBG
#  define matrix_get_element _CW_NS_LIBSTASH(matrix_get_element)
cw_sint32_t
matrix_get_element(cw_matrix_t * a_matrix, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos);
#else
#  define matrix_get_element(a, b, c) (a)->grid[(a)->y_index[(c)] \
					       * (a)->grid_x_size \
					       + (a)->x_index[(b)]]
#endif

#ifdef _LIBSTASH_DBG
#  define matrix_set_element _CW_NS_LIBSTASH(matrix_set_element)
void
matrix_set_element(cw_matrix_t * a_matrix, cw_uint32_t a_x_pos,
		   cw_uint32_t a_y_pos, cw_sint32_t a_val);
#else
#  define matrix_set_element(a, b, c, d) (a)->grid[(a)->y_index[(c)] \
						  * (a)->grid_x_size \
						  + (a)->x_index[(b)]] = (d)
#endif

#ifdef _LIBSTASH_DBG
#  define matrix_get_x_size _CW_NS_LIBSTASH(matrix_get_x_size)
cw_uint32_t
matrix_get_x_size(cw_matrix_t * a_matrix);
#else
#  define matrix_get_x_size(a) (a)->x_size
#endif

#ifdef _LIBSTASH_DBG
#  define matrix_get_y_size _CW_NS_LIBSTASH(matrix_get_y_size)
cw_uint32_t
matrix_get_y_size(cw_matrix_t * a_matrix);
#else
#  define matrix_get_y_size(a) (a)->y_size
#endif

#define matrix_copy _CW_NS_LIBSTASH(matrix_copy)
cw_matrix_t *
matrix_copy(cw_matrix_t * a_matrix);

#define matrix_dump _CW_NS_LIBSTASH(matrix_dump)
void
matrix_dump(cw_matrix_t * a_matrix, cw_bool_t a_compact);

#define matrix_is_equal _CW_NS_LIBSTASH(matrix_is_equal)
cw_bool_t
matrix_is_equal(cw_matrix_t * a_a, cw_matrix_t * a_b);

#define matrix_remove_row _CW_NS_LIBSTASH(matrix_remove_row)
void
matrix_remove_row(cw_matrix_t * a_matrix, cw_uint32_t a_row);

#define matrix_remove_column _CW_NS_LIBSTASH(matrix_remove_column)
void
matrix_remove_column(cw_matrix_t * a_matrix, cw_uint32_t a_column);
