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

  cw_bool_t rule_constrained_open;
  cw_bool_t rule_perfect_five;
  cw_bool_t rule_capture_three;
  cw_bool_t rule_teams;
  cw_bool_t rule_no_double_three;

  cw_move_t * root;
  cw_move_t * last;

  cw_board_t board;
};
