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

typedef struct cw_game_s cw_game_t;

struct cw_game_s
{
#ifdef _XVX_DEBUG
  cw_uint32_t magic;
#endif
  cw_bool_t is_malloced;

  cw_uint32_t nplayers;
  cw_bool_t rule_teams;
  cw_uint32_t ncaptures_to_win;
  cw_bool_t rule_constrained_open;
  cw_bool_t rule_perfect_five;
  cw_bool_t rule_capture_three;
  cw_bool_t rule_no_double_three;

  cw_move_t * root;
  cw_move_t * last;

  cw_board_t board;
};

cw_game_t *
game_new(cw_game_t * a_game, cw_uint32_t a_nplayers, cw_bool_t a_teams,
	 cw_uint32_t a_ncaptures_to_win, cw_bool_t a_constrained_open,
	 cw_bool_t a_perfect_five, cw_bool_t a_capture_three,
	 cw_bool_t a_no_double_three);

void
game_delete(cw_game_t * a_game);

void
game_reset(cw_game_t * a_game);

cw_bool_t
game_is_move_legal(cw_game_t * a_game, cw_uint32_t a_x, cw_uint32_t a_y);

cw_sint32_t
game_move(cw_game_t * a_game, cw_uint32_t a_x, cw_uint32_t a_y);

cw_bool_t
game_undo(cw_game_t * a_game);

const cw_board_t *
game_get_board(cw_game_t * a_game);

const cw_move_t *
game_get_move(cw_game_t * a_game, cw_uint32_t a_count);

void
game_dump(cw_game_t * a_game);
