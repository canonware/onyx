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

int
main(int argc, char **argv)
{
	cw_game_t game;

	libstash_init();
  
	out_register(cw_g_out, "move", sizeof(cw_move_t *), move_out_put);
  
	game_new(&game, 2, FALSE, 10, TRUE, FALSE, FALSE, FALSE);
	game_dump(&game);

	_cw_assert(game_move(&game, 9, 9) == 0);
	game_dump(&game);

	_cw_assert(game_move(&game, 13, 10) == 0);
	game_dump(&game);

	_cw_assert(game_move(&game, 9, 13) == 0);
	game_dump(&game);

	_cw_assert(game_move(&game, 9, 14) == 0);
	game_dump(&game);

	_cw_assert(game_move(&game, 9, 15) == 0);
	game_dump(&game);

	game_delete(&game);
  
	libstash_shutdown();
	return 0;
}
