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

typedef struct cw_move_s cw_move_t;

struct cw_move_s
{
#ifdef _XVX_DBG
	cw_uint32_t	magic;
#endif

	/* Automatic deallocation hooks. */
	cw_opaque_dealloc_t *dealloc_func;
	void		*dealloc_arg;
  
	/* Used for linking moves together into move lists and/or game trees. */
	cw_treen_t	node;

	/* Stone placement, in the first quadrant, (x + y * 19). */
	cw_uint32_t	add;

	/* A maximum of 24 stones can be captured, depending on the game
	 * variation.  Capturable stones are numbered as follows:
	 *
	 * 21--+---+---22--+---+---23
	 * | \ |   |   |   |   |  /|
	 * |  \|   |   |   |   | / |
	 * +---18--+---19--+---20--+
	 * |   | \ |   |   |  /|   |
	 * |   |  \|   |   | / |   |
	 * +---+---15--16--17--+---+
	 * |   |   | \ |  /|   |   |
	 * |   |   |  \| / |   |   |
	 * 09--10--11--<>--12--13--14
	 * |   |   |  /| \ |   |   |
	 * |   |   | / |  \|   |   |
	 * +---+---06--07--08--+---+
	 * |   |  /|   |   | \ |   |
	 * |   | / |   |   |  \|   |
	 * +---03--+---04--+---05--+
	 * |  /|   |   |   |   | \ |
	 * | / |   |   |   |   |  \|
	 * 00--+---+---01--+---+---02
	 *
	 */
	cw_stone_t	sub[24];

	/* Used for game tree pruning. */
	enum {
		_XVX_MOVE_NONTERM	= 0,
		_XVX_MOVE_LOSE		= 1,
		_XVX_MOVE_WIN		= 2
	}		move_type;
};

cw_move_t	*move_new(cw_move_t *a_move, cw_opaque_dealloc_t
    *a_dealloc_func, void *a_dealloc_arg);
void		move_delete(cw_move_t *a_move);
cw_uint32_t	move_depth(cw_move_t *a_move);
cw_uint32_t	move_get_x(cw_move_t *a_move);
void		move_set_x(cw_move_t *a_move, cw_uint32_t a_x);
cw_uint32_t	move_get_y(cw_move_t *a_move);
void		move_set_y(cw_move_t *a_move, cw_uint32_t a_y);
cw_bool_t	move_is_capture(cw_move_t *a_move);
cw_stone_t	move_get_capture(cw_move_t *a_move, cw_uint32_t a_index);
void		move_set_capture(cw_move_t *a_move, cw_uint32_t a_index,
    cw_stone_t a_stone);

/* Tree operations. */
void		move_link(cw_move_t *a_move, cw_move_t *a_parent);
cw_move_t	*move_get_parent(cw_move_t *a_move);
cw_move_t	*move_get_child(cw_move_t *a_move);
cw_move_t	*move_get_sibling(cw_move_t *a_move);

/* Printing. */
cw_uint32_t	move_out_put(const char *a_format, cw_uint32_t a_len, const
    void *a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_buf);
