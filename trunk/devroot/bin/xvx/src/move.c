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
 ****************************************************************************/

#include "xvx.h"

cw_move_t *
move_new(cw_move_t * a_move,
	 void (*a_dealloc_func)(void * dealloc_arg, void * move),
	 void * a_dealloc_arg, cw_uint32_t a_depth)
{
  cw_move_t * retval;

  if (NULL != a_move)
  {
    retval = a_move;
    bzero(retval, sizeof(cw_move_t));
    retval->dealloc_func = a_dealloc_func;
    retval->dealloc_arg = a_dealloc_arg;
  }
  else
  {
    retval = (cw_move_t *) _cw_malloc(sizeof(cw_move_t));
    bzero(retval, sizeof(cw_move_t));
    retval->dealloc_func = mem_dealloc;
    retval->dealloc_arg = cw_g_mem;
  }

  retval->node = treen_new(); /* XXX */
  treen_set_data_ptr(retval->node, (void *) retval);

  return retval;
}

void
move_delete(cw_move_t * a_move)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
  
}

cw_uint32_t
move_depth(cw_move_t * a_move)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

cw_uint32_t
move_get_x(cw_move_t * a_move)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

void
move_set_x(cw_move_t * a_move, cw_uint32_t a_x)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

cw_uint32_t
move_get_y(cw_move_t * a_move)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

void
move_set_y(cw_move_t * a_move, cw_uint32_t a_y)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

cw_bool_t
move_is_capture(cw_move_t * a_move)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

cw_stone_t
move_get_capture(cw_move_t * a_move, cw_uint32_t a_index)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

void
move_set_capture(cw_move_t * a_move, cw_uint32_t a_index, cw_stone_t a_stone)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

cw_move_t *
move_get_parent(cw_move_t * a_move)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

cw_uint32_t
move_get_nchildren(cw_move_t * a_move)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

cw_move_t *
move_get_child(cw_move_t * a_move, cw_uint32_t a_index)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

cw_sint32_t
move_out_metric(const char * a_format, cw_uint32_t a_len, const void * a_arg)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}

char *
buf_out_render(const char * a_format, cw_uint32_t a_len, const void * a_arg,
	       char * r_buf)
{
  _cw_check_ptr(a_move);
  _cw_assert(_CW_MOVE_MAGIC == a_move->magic);
}
