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

typedef struct cw_board_s cw_board_t;

struct cw_board_s
{
#ifdef _XVX_DEBUG
  cw_uint32_t magic;
#endif
  cw_bool_t is_malloced;
  
  /* Each byte represents one vertex.  Numbering is in the first quadrant,
   * row-major; (x, y) --> (x + y * 19).
   *
   *   18 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *   17 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *   16 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *   15 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *   14 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *   13 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *   12 +--+--+--+--+--+--X--+--+--+--+--+--X--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *   11 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *   10 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   * y  9 +--+--+--+--+--+--+--+--+--X--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    8 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    7 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    6 +--+--+--+--+--+--X--+--+--+--+--+--X--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    5 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    4 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    3 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    2 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    1 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
   *    0 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
   *
   *                                 x
   */
  cw_stone_t vstate[361];

  /* Number of captured stones for each player.  Some variations allow six
   * players.  The array has seven elements to allow direct mapping from stone
   * values in cw_stone_t to offset into this array. */
  cw_uint32_t captured[7];
};

cw_board_t *
board_new(cw_board_t * a_board);

void
board_delete(cw_board_t * a_board);

void
board_clear(cw_board_t * a_board);

void
board_copy(cw_board_t * a_to, cw_board_t * a_from);

cw_stone_t
board_vertex_get(cw_board_t * a_board, cw_uint32_t a_index);

void
board_vertex_set(cw_board_t * a_board, cw_uint32_t a_index, cw_stone_t a_stone);

cw_uint32_t
board_get_captured(cw_board_t * a_board, cw_stone_t a_stone);

void
board_set_captured(cw_board_t * a_board, cw_stone_t a_stone,
		   cw_uint32_t a_count);

void
board_dump(cw_board_t * a_board);
