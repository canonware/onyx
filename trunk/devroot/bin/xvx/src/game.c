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

#define _CW_GAME_MAGIC 0x5e07e111

cw_game_t *
game_new(cw_game_t * a_game, cw_uint32_t a_nplayers, cw_bool_t a_teams,
	 cw_uint32_t a_ncaptures_to_win, cw_bool_t a_constrained_open,
	 cw_bool_t a_perfect_five, cw_bool_t a_capture_three,
	 cw_bool_t a_no_double_three)
{
  cw_game_t * retval;
  
  if (NULL != a_game)
  {
    retval = a_game;
    bzero(retval, sizeof(cw_game_t));
    retval->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_game_t *) _cw_malloc(sizeof(cw_game_t));
    bzero(retval, sizeof(cw_game_t));
    retval->is_malloced = TRUE;
  }

  retval->nplayers = a_nplayers;
  retval->rule_teams = a_teams;
  retval->ncaptures_to_win = a_ncaptures_to_win;
  retval->rule_constrained_open = a_constrained_open;
  retval->rule_perfect_five = a_perfect_five;
  retval->rule_capture_three = a_capture_three;
  retval->rule_no_double_three = a_no_double_three;

  board_new(&retval->board);

#ifdef _XVX_DEBUG
  retval->magic = _CW_GAME_MAGIC;
#endif

  return retval;
}

void
game_delete(cw_game_t * a_game)
{
  _cw_check_ptr(a_game);
  _cw_assert(_CW_GAME_MAGIC == a_game->magic);

  if (NULL != a_game->root)
  {
    move_delete(a_game->root);
  }
  board_delete(&a_game->board);

  if (TRUE == a_game->is_malloced)
  {
    _cw_free(a_game);
  }
#ifdef _XVX_DEBUG
  else
  {
    memset(a_game, 0x5a, sizeof(cw_game_t));
  }
#endif
}

void
game_reset(cw_game_t * a_game)
{
  _cw_check_ptr(a_game);
  _cw_assert(_CW_GAME_MAGIC == a_game->magic);

  if (NULL != a_game->root)
  {
    move_delete(a_game->root);
  }
  a_game->root = NULL;
  a_game->last = NULL;
  board_clear(&a_game->board);
}

cw_bool_t
game_is_move_legal(cw_game_t * a_game, cw_uint32_t a_x, cw_uint32_t a_y)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_game);
  _cw_assert(_CW_GAME_MAGIC == a_game->magic);

  /* Vertex occupied? */
  if (_XVX_EMPTY != board_vertex_get(&a_game->board, _XVX_XY2I(a_x, a_y)))
  {
    retval = FALSE;
    goto RETURN;
  }

  /* Illegal opening move? */
  if (NULL == a_game->root)
  {
    /* Not on center? */
    if (9 != a_x || 9 != a_y)
    {
      retval = FALSE;
      goto RETURN;
    }
  }
  
  if (TRUE == a_game->rule_constrained_open)
  {
    /* First move for player other than the first? */
    if ((NULL != a_game->root)
	&& (a_game->nplayers > move_depth(a_game->last)))
    {
      /* Inside the center region? */
      if (6 < a_x && 12 > a_x && 6 < a_y && 12 > a_y)
      {
	retval = FALSE;
	goto RETURN;
      }
    }
  }
  
  /* Illegal double three? */
  /* XXX */

  retval = TRUE;

  RETURN:
  return retval;
}

cw_sint32_t
game_move(cw_game_t * a_game, cw_uint32_t a_x, cw_uint32_t a_y)
{
  cw_sint32_t retval;
  cw_move_t * move;
  
  _cw_check_ptr(a_game);
  _cw_assert(_CW_GAME_MAGIC == a_game->magic);

  if (TRUE == game_is_move_legal(a_game, a_x, a_y))
  {
    _cw_marker("Got here");
    move = move_new(NULL, NULL, NULL);
    move_set_x(move, a_x);
    move_set_y(move, a_y);

    /* Calculate captures. */
    /* XXX */

    /* Update game tree. */
    if (NULL == a_game->root)
    {
      a_game->root = move;
      a_game->last = move;
    }
    else
    {
      move_link(move, a_game->last);
      a_game->last = move;
    }

    /* Apply move to board. */
    board_vertex_set(&a_game->board, _XVX_XY2I(a_x, a_y),
		     ((move_depth(a_game->last) - 1) % a_game->nplayers)
		     + _XVX_P1);
    /* XXX Apply captures. */

    /* Winning move? */
    if (0) /* XXX */
    {
      retval = 1;
    }
    else
    {
      retval = 0;
    }
  }
  else
  {
    retval = -1;
  }

  return retval;
}

cw_bool_t
game_undo(cw_game_t * a_game)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_game);
  _cw_assert(_CW_GAME_MAGIC == a_game->magic);

  retval = TRUE; /* XXX */

  return retval;
}

const cw_board_t *
game_get_board(cw_game_t * a_game)
{
  _cw_check_ptr(a_game);
  _cw_assert(_CW_GAME_MAGIC == a_game->magic);

  return &a_game->board;
}

const cw_move_t *
game_get_move(cw_game_t * a_game, cw_uint32_t a_count)
{
  const cw_move_t * retval;
  cw_uint32_t i;
  
  _cw_check_ptr(a_game);
  _cw_assert(_CW_GAME_MAGIC == a_game->magic);
  _cw_assert(0 < a_count);

  for (i = 1, retval = a_game->root;
       a_count > i && NULL != retval;
       i++, retval = move_get_child((cw_move_t *) retval))
  {
  }

  return retval;
}

void
game_dump(cw_game_t * a_game)
{
  cw_move_t * t_move;
  cw_uint32_t i;
  
  _cw_check_ptr(a_game);
  _cw_assert(_CW_GAME_MAGIC == a_game->magic);

  out_put(cw_g_out, "Players: [i]\n", a_game->nplayers);
  out_put(cw_g_out,
	  "Rules:            teams: [s]\n"
	  "       constrained_open: [s]\n"
	  "           perfect_five: [s]\n"
	  "          capture_three: [s]\n"
	  "        no_double_three: [s]\n",
	  a_game->rule_teams ? "yes" : "no",
	  a_game->rule_constrained_open ? "yes" : "no",
	  a_game->rule_perfect_five ? "yes" : "no",
	  a_game->rule_capture_three ? "yes" : "no",
	  a_game->rule_no_double_three ? "yes" : "no");

  board_dump(&a_game->board);

  for (i = 0, t_move = a_game->root;
       NULL != t_move;
       i++, t_move = move_get_child(t_move))
  {
    if (0 == i % a_game->nplayers)
    {
      /* Print turn number. */
      out_put(cw_g_out, "[i|w:3]. ", i / a_game->nplayers + 1);
    }

    out_put(cw_g_out, "[move|w:8|j:r]", t_move);

    if (a_game->nplayers - 1 == i % a_game->nplayers)
    {
      /* Newline. */
      out_put(cw_g_out, "\n");
    }
  }
  out_put(cw_g_out, "\n");
}
