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
  /* Stone placement, in the first quadrant, (x + y * 19). */
  cw_uint32_t add;

  /* A maximum of 24 stones can be captured, depending on the game variation.
   * Capturable stones are numbered as follows:
   *
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
};

cw_move_t *
move_new(cw_move_t * a_move);

void
move_delete(cw_move_t * a_move);

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
