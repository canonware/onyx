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

int
main(int argc, char ** argv)
{
  cw_board_t board;

  libstash_init();

  board_new(&board);
  board_vertex_set(&board, _XVX_XY2I(9, 9), _XVX_EMPTY);
  board_vertex_set(&board, _XVX_XY2I(10, 10), _XVX_P1);
  board_vertex_set(&board, _XVX_XY2I(11, 11), _XVX_P2);
  board_vertex_set(&board, _XVX_XY2I(12, 12), _XVX_P3);
  board_vertex_set(&board, _XVX_XY2I(13, 13), _XVX_P4);
  board_vertex_set(&board, _XVX_XY2I(14, 14), _XVX_P5);
  board_vertex_set(&board, _XVX_XY2I(15, 15), _XVX_P6);
  
  board_dump(&board);
  board_delete(&board);

  libstash_shutdown();
  return 0;
}
