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

typedef struct cw_move_s cw_move_t;

struct cw_move_s
{
#ifdef _XVX_DEBUG
  cw_uint32_t magic;
#endif

  /* Automatic deallocation hooks. */
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;
  
  /* Used for linking moves together into move lists and/or game trees. */
  cw_treen_t * node; /* XXX Should be embedded in this struct. */

  /* Starts at 1.  Not strictly necessary, but it makes many algorithms easier,
   * since the tree depth doesn't have to be passed as a recursion argument, nor
   * does the tree have to be traversed to discover the depth. */
  cw_uint32_t depth.
  
  /* Stone placement, in the first quadrant, (x + y * 19). */
  cw_uint32_t add;

  /* A maximum of 24 stones can be captured, depending on the game variation.
   * Capturable stones are numbered as follows:
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
  cw_uint32_t sub[24];

  /* Used for game tree pruning. */
  enum {_XVX_MOVE_NONTERM = 0, _XVX_MOVE_LOSE = 1, _XVX_MOVE_WIN = 2} move_type;
};

cw_move_t *
move_new(cw_move_t * a_move,
	 void (*a_dealloc_func)(void * dealloc_arg, void * move),
	 void * a_dealloc_arg, cw_uint32_t a_depth);

void
move_delete(cw_move_t * a_move);

cw_uint32_t
move_depth(cw_move_t * a_move);

cw_uint32_t
move_get_x(cw_move_t * a_move);

void
move_set_x(cw_move_t * a_move, cw_uint32_t a_x);

cw_uint32_t
move_get_y(cw_move_t * a_move);

void
move_set_y(cw_move_t * a_move, cw_uint32_t a_y);

cw_bool_t
move_is_capture(cw_move_t * a_move);

cw_stone_t
move_get_capture(cw_move_t * a_move, cw_uint32_t a_index);

void
move_set_capture(cw_move_t * a_move, cw_uint32_t a_index, cw_stone_t a_stone);

/* Tree operations. */
cw_move_t *
move_get_parent(cw_move_t * a_move);

cw_uint32_t
move_get_nchildren(cw_move_t * a_move);

cw_move_t *
move_get_child(cw_move_t * a_move, cw_uint32_t a_index);

cw_sint32_t
move_out_metric(const char * a_format, cw_uint32_t a_len, const void * a_arg);

char *
buf_out_render(const char * a_format, cw_uint32_t a_len, const void * a_arg,
	       char * r_buf);
