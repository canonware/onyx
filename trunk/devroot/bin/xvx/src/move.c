/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "xvx.h"

#define _CW_MOVE_MAGIC 0x99887654

static void	move_p_delete(cw_move_t *a_move);
static void	move_p_treen_delete(void *a_move, void *a_treen, const char *
    a_filename, cw_uint32_t a_line);

cw_move_t *
move_new(cw_move_t *a_move, cw_opaque_dealloc_t *a_dealloc_func, void
    *a_dealloc_arg)
{
	cw_move_t	*retval;

	if (a_move != NULL) {
		retval = a_move;
		bzero(retval, sizeof(cw_move_t));
		retval->dealloc_func = a_dealloc_func;
		retval->dealloc_arg = a_dealloc_arg;
	} else {
		retval = (cw_move_t *)_cw_malloc(sizeof(cw_move_t));
		bzero(retval, sizeof(cw_move_t));
		retval->dealloc_func = (cw_opaque_dealloc_t *)mem_free_e;
		retval->dealloc_arg = cw_g_mem;
	}

	treen_new(&retval->node, cw_g_mem, (cw_opaque_dealloc_t
	    *)move_p_treen_delete, retval);
	treen_data_ptr_set(&retval->node, (void *) retval);

#ifdef _XVX_DBG
	retval->magic = _CW_MOVE_MAGIC;
#endif

	return retval;
}

void
move_delete(cw_move_t *a_move)
{
	_cw_check_ptr(a_move);
	_cw_assert(_CW_MOVE_MAGIC == a_move->magic);

	/*
	 * move_p_delete() doesn't clean up the treen in order to avoid infinite
	 * recursion when called from move_p_treen_delete(), so do it here.
	 * Note that treen_delete() recursively deletes the tree, and as a
	 * byproduct, deletes the associated moves, so by the time this call
	 * returns, this move has no children.
	 */
	treen_delete(&a_move->node);
}

cw_uint32_t
move_depth(cw_move_t *a_move)
{
	cw_uint32_t	retval;
	cw_move_t	*t_move;
  
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	for (t_move = a_move, retval = 1;
	     NULL != (t_move = move_get_parent(t_move));
	     retval++)
		;

	return retval;
}

cw_uint32_t
move_get_x(cw_move_t *a_move)
{
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	return _XVX_I2X(a_move->add);
}

void
move_set_x(cw_move_t *a_move, cw_uint32_t a_x)
{
	cw_uint32_t	y;
  
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	y = _XVX_I2Y(a_move->add);
	a_move->add = _XVX_XY2I(a_x, y);
}

cw_uint32_t
move_get_y(cw_move_t *a_move)
{
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	return _XVX_I2Y(a_move->add);
}

void
move_set_y(cw_move_t *a_move, cw_uint32_t a_y)
{
	cw_uint32_t x;
  
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	x = _XVX_I2X(a_move->add);
	a_move->add = _XVX_XY2I(x, a_y);
}

cw_bool_t
move_is_capture(cw_move_t *a_move)
{
	cw_bool_t	retval;
	cw_uint32_t	i;
  
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	for (i = 0; i < 24; i++) {
		if (a_move->sub[i] != _XVX_EMPTY) {
			retval = TRUE;
			goto RETURN;
		}
	}

	retval = FALSE;
	RETURN:
	return retval;
}

cw_stone_t
move_get_capture(cw_move_t *a_move, cw_uint32_t a_index)
{
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);
	_cw_assert(a_index < 24);

	return a_move->sub[a_index];
}

void
move_set_capture(cw_move_t *a_move, cw_uint32_t a_index, cw_stone_t a_stone)
{
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);
	_cw_assert(a_index < 24);

	a_move->sub[a_index] = a_stone;
}

void
move_link(cw_move_t *a_move, cw_move_t *a_parent)
{
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	if (a_parent == NULL)
		treen_link(&a_move->node, NULL);
	else {
		_cw_check_ptr(a_parent);
		_cw_assert(_CW_MOVE_MAGIC == a_parent->magic);

		treen_link(&a_move->node, &a_parent->node);
	}
}

cw_move_t *
move_get_parent(cw_move_t *a_move)
{
	cw_move_t	*retval;
	cw_treen_t	*t_treen;
  
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	t_treen = treen_parent_get(&a_move->node);
	if (t_treen != NULL)
		retval = (cw_move_t *)treen_data_ptr_get(t_treen);
	else
		retval = NULL;
  
	return retval;
}

cw_move_t *
move_get_child(cw_move_t *a_move)
{
	cw_move_t	*retval;
	cw_treen_t	*t_treen;
  
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	t_treen = treen_child_get(&a_move->node);
	if (t_treen != NULL)
		retval = (cw_move_t *)treen_data_ptr_get(t_treen);
	else
		retval = NULL;

	return retval;
}

cw_move_t *
move_get_sibling(cw_move_t *a_move)
{
	cw_move_t	*retval;
	cw_treen_t	*t_treen;
  
	_cw_check_ptr(a_move);
	_cw_assert(a_move->magic == _CW_MOVE_MAGIC);

	t_treen = treen_sibling_get(&a_move->node);
	retval = (cw_move_t *)treen_data_ptr_get(t_treen);

	return retval;
}

/* XXX Doesn't honor the a_max_len argument. */
cw_uint32_t
move_out_put(const char *a_format, cw_uint32_t a_len, const void *a_arg,
    cw_uint32_t a_max_len, cw_uint8_t *r_buf)
{
	cw_uint32_t	retval = a_max_len; /* XXX */
	cw_sint32_t	val_len;
	cw_uint32_t	i, ncaptures = 0, width = 0, min_width;
	const cw_uint8_t *val;
	cw_move_t	*move;
  
	_cw_check_ptr(a_format);
	_cw_assert(a_len > 0);
	_cw_check_ptr(a_arg);
	_cw_check_ptr(r_buf);
  
	move = *(cw_move_t **)a_arg;
	_cw_check_ptr(move);
	_cw_assert(move->magic == _CW_MOVE_MAGIC);

	if ((_XVX_I2X(move->add) == 9) && (_XVX_I2Y(move->add) == 9)) {
		/* 0 */
		width++;
	} else {
		if (_XVX_I2X(move->add) != 9) {
			/* L9, R3, etc. */
			width += 2;
		}
  
		if (_XVX_I2Y(move->add) != 9) {
			/* U3, D8, etc. */
			width += 2;
		}
	}

	for (i = 0; i < 24; i++) {
		if (move->sub[i] != _XVX_EMPTY)
			ncaptures++;
	}

	if (ncaptures > 9) {
		/* +10, +13, etc. */
		width += 3;
	} else if (ncaptures > 0) {
		/* +3, +2, etc. */
		width += 2;
	}
  
	if ((val_len = spec_val_get(a_format, a_len, "w", 1, &val)) != -1) {
		/* Width specified. */
		
		/*
		 * The next character after val is either `|' or `]', so we
		 * don't have to worry about terminating the string that val
		 * points to.
		 */
		min_width = strtoul(val, NULL, 10);
	} else
		min_width = 0;

	if (min_width > width) {
		char	pad, justify;
    
		/*
		 * Padding needed.  memset() the output string to the padding
		 * character, then determine where to render, based on
		 * justification.
		 */
		if ((val_len = spec_val_get(a_format, a_len, "p", 1,
		    &val)) != -1)
			pad = val[0];
		else
			pad = ' ';
		memset(r_buf, pad, min_width);

		if ((val_len = spec_val_get(a_format, a_len, "j", 1, &val))
		    != -1)
			justify = val[0];
		else
			justify = 'l';

		switch (justify) {
		case 'r':
			r_buf = &r_buf[min_width - width];
			break;
		case 'l':
			break;
		case 'c':
			r_buf = &r_buf[(min_width - width) / 2];
			break;
		default:
			_cw_error("Unknown justification");
		}
	}

	/* Render the move to r_buf. */
	if ((_XVX_I2X(move->add) == 9) && (_XVX_I2Y(move->add) == 9)) {
		r_buf[0] = '0';
		r_buf++;
	} else {
		if (_XVX_I2X(move->add) < 9) {
			/* L9, L3, etc. */
			r_buf[0] = 'L';
			r_buf++;
      
			r_buf[0] = '0' + (9 - _XVX_I2X(move->add));
			r_buf++;
		} else if (_XVX_I2X(move->add) > 9) {
			/* R2, R4, etc. */
			r_buf[0] = 'R';
			r_buf++;

			r_buf[0] = '0' + (_XVX_I2X(move->add) - 9);
			r_buf++;
		}
    
		if (_XVX_I2Y(move->add) < 9) {
			/* D9, D3, etc. */
			r_buf[0] = 'D';
			r_buf++;
      
			r_buf[0] = '0' + (9 - _XVX_I2Y(move->add));
			r_buf++;
		} else if (_XVX_I2Y(move->add) > 9) {
			/* U2, U4, etc. */
			r_buf[0] = 'U';
			r_buf++;

			r_buf[0] = '0' + (_XVX_I2Y(move->add) - 9);
			r_buf++;
		}
	}

	if (ncaptures > 0) {
		r_buf[0] = '+';
		r_buf++;

		if (ncaptures / 10 > 0) {
			r_buf[0] = '0' + ncaptures / 10;
			r_buf++;
		}

		r_buf[0] = '0' + ncaptures % 10;
		r_buf++;
	}
    
	return retval;
}

static void
move_p_delete(cw_move_t *a_move)
{
	if (a_move->dealloc_func != NULL)
		a_move->dealloc_func(a_move->dealloc_arg, a_move, __FILE__,
		    __LINE__);
#ifdef _XVX_DBG
	else
		memset(a_move, 0x5a, sizeof(cw_move_t));
#endif
}

static void
move_p_treen_delete(void *a_move, void *a_treen, const char *a_filename,
    cw_uint32_t a_line)
{
	move_p_delete((cw_move_t *)a_move);
}
