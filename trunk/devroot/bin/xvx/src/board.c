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

#define _CW_BOARD_MAGIC 0xb0c0d0e0

cw_board_t *
board_new(cw_board_t *a_board)
{
	cw_board_t	*retval;
  
	if (a_board != NULL) {
		retval = a_board;
		bzero(retval, sizeof(cw_board_t));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_board_t *)_cw_malloc(sizeof(cw_board_t));
		bzero(retval, sizeof(cw_board_t));
		retval->is_malloced = TRUE;
	}

#ifdef _XVX_DBG
	retval->magic = _CW_BOARD_MAGIC;
#endif

	return retval;
}

void
board_delete(cw_board_t *a_board)
{
	_cw_check_ptr(a_board);
	_cw_assert(a_board->magic == _CW_BOARD_MAGIC);

	if (a_board->is_malloced)
		_cw_free(a_board);
#ifdef _XVX_DBG
	else
		memset(a_board, 0x5a, sizeof(cw_board_t));
#endif
}

void
board_clear(cw_board_t *a_board)
{
	_cw_check_ptr(a_board);
	_cw_assert(a_board->magic == _CW_BOARD_MAGIC);

	bzero(a_board->vstate, sizeof(a_board->vstate));
	bzero(a_board->captured, sizeof(a_board->captured));
}

void
board_copy(cw_board_t *a_to, cw_board_t *a_from)
{
	_cw_check_ptr(a_to);
	_cw_assert(_CW_BOARD_MAGIC == a_to->magic);
	_cw_check_ptr(a_from);
	_cw_assert(_CW_BOARD_MAGIC == a_from->magic);

	memcpy(a_to->vstate, a_from->vstate, sizeof(a_to->vstate));
	memcpy(a_to->captured, a_from->captured, sizeof(a_to->captured));
}

cw_stone_t
board_vertex_get(cw_board_t *a_board, cw_uint32_t a_index)
{
	_cw_check_ptr(a_board);
	_cw_assert(a_board->magic == _CW_BOARD_MAGIC);
	_cw_assert(360 >= a_index);

	return a_board->vstate[a_index];
}

void
board_vertex_set(cw_board_t *a_board, cw_uint32_t a_index, cw_stone_t a_stone)
{
	_cw_check_ptr(a_board);
	_cw_assert(a_board->magic == _CW_BOARD_MAGIC);
	_cw_assert(360 >= a_index);

	a_board->vstate[a_index] = a_stone;
}

cw_uint32_t
board_get_captured(cw_board_t *a_board, cw_stone_t a_stone)
{
	_cw_check_ptr(a_board);
	_cw_assert(a_board->magic == _CW_BOARD_MAGIC);

	return a_board->captured[a_stone];
}

void
board_set_captured(cw_board_t *a_board, cw_stone_t a_stone,
    cw_uint32_t a_count)
{
	_cw_check_ptr(a_board);
	_cw_assert(a_board->magic == _CW_BOARD_MAGIC);

	a_board->captured[a_stone] = a_count;
}

void
board_dump(cw_board_t *a_board)
{
	cw_sint32_t	i, j;
	cw_sint8_t	c;
  
	_cw_check_ptr(a_board);
	_cw_assert(a_board->magic == _CW_BOARD_MAGIC);

	for (j = 18; j >= 0; j--) {
		_cw_out_put("[i|w:2]", j);
		for (i = 0; i < 19; i++) {
			switch (a_board->vstate[_XVX_XY2I(i, j)]) {
			case _XVX_EMPTY:
				switch (_XVX_XY2I(i, j)) {
				case _XVX_XY2I(9, 9):
				case _XVX_XY2I(12, 12):
				case _XVX_XY2I(6, 12):
				case _XVX_XY2I(12, 6):
				case _XVX_XY2I(6, 6):
					c = 'o';
					break;
				default:
					c = '.';
					break;
				}
				break;
			case _XVX_P1:
				c = '1';
				break;
			case _XVX_P2:
				c = '2';
				break;
			case _XVX_P3:
				c = '3';
				break;
			case _XVX_P4:
				c = '4';
				break;
			case _XVX_P5:
				c = '5';
				break;
			case _XVX_P6:
				c = '6';
				break;
			default:
				_cw_not_reached();
			}
			_cw_out_put("  [c]", c);
		}
		_cw_out_put("\n");
	}

	_cw_out_put("\n  ");
	for (i = 0; i < 19; i++)
		_cw_out_put(" [i|w:2]", i);
	_cw_out_put("\n\n");

	_cw_out_put("Captured: P1: [i|w:2], P2: [i|w:2], P3: [i|w:2],"
	    " P5: [i|w:2], P5: [i|w:2], P6: [i|w:2]\n",
	    a_board->captured[_XVX_P1],
	    a_board->captured[_XVX_P2],
	    a_board->captured[_XVX_P3],
	    a_board->captured[_XVX_P4],
	    a_board->captured[_XVX_P5],
	    a_board->captured[_XVX_P6]);
}
