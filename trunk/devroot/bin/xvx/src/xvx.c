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

cw_bool_t
oom_handler(const void * a_data, cw_uint32_t a_size);

int
main(int argc, char ** argv)
{
  cw_game_t game;

  libstash_init();
  mem_set_oom_handler(cw_g_mem, oom_handler, NULL);
  
  out_register(cw_g_out, "move", sizeof(cw_move_t *), move_out_metric,
	       move_out_render);
  
  game_new(&game, 2, FALSE, 10, TRUE, FALSE, FALSE, FALSE);
  game_dump(&game);

  _cw_assert(0 == game_move(&game, 9, 9));
  game_dump(&game);

  _cw_assert(0 == game_move(&game, 13, 10));
  game_dump(&game);

  _cw_assert(0 == game_move(&game, 9, 13));
  game_dump(&game);

  _cw_assert(0 == game_move(&game, 9, 14));
  game_dump(&game);

  _cw_assert(0 == game_move(&game, 9, 15));
  game_dump(&game);

  game_delete(&game);
  
  libstash_shutdown();
  return 0;
}

cw_bool_t
oom_handler(const void * a_data, cw_uint32_t a_size)
{
  if (dbg_is_registered(cw_g_dbg, "ncat_error"))
  {
    out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
	      "Memory allocation error for size [i]\n", a_size);
  }
  exit(1);
  
  return FALSE;
}
