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
 * <<< Description >>>
 *
 * This is a lexer implementation that dynamically creates lexer state machines,
 * which means that specifications can be compiled and replaced on the fly.
 *
 ****************************************************************************/

#define _STASH_USE_LEX
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include "libstash/lex_p.h"

cw_lex_t *
lex_new()
{
  return NULL; /* XXX */
}
     
void
lex_delete(cw_lex_t * a_lex)
{
}

cw_bool_t
lex_insert_regex(cw_lex_t * a_lex,
		 const char * a_regex,
		 cw_uint32_t a_tok_val)
{
  /* Parse a_regex and generate a graph for it. */

  
  return TRUE; /* XXX */
}

const char *
lex_delete_regex(cw_lex_t * a_lex, const char * a_regex)
{
  return NULL; /* XXX */
}

cw_bool_t
lex_build_table(cw_lex_t * a_lex)
{
  return TRUE; /* XXX */
}

cw_lexer_t *
lexer_new(cw_lexer_t * a_lexer)
{
  return NULL; /* XXX */
}

void
lexer_delete(cw_lexer_t * a_lexer)
{
}

void
lexer_set_lex(cw_lexer_t * a_lexer, cw_lex_t * a_lex)
{
}

cw_bool_t
lexer_connect_buf(cw_lexer_t * a_lexer, cw_buf_t * a_buf)
{
  return TRUE; /* XXX */
}

cw_lexertok_t *
lexer_get_token(cw_lexer_t * a_lexer)
{
  return NULL; /* XXX */
}

cw_uint32_t
lexer_get_line_num(cw_lexer_t * a_lexer,
		   cw_bool_t a_last_or_next_tok)
{
  return 0; /* XXX */
}

cw_bool_t
lexer_set_line_num(cw_lexer_t * a_lexer,
		   cw_uint32_t a_line_num)
{
  return TRUE; /* XXX */
}

cw_uint32_t
lexer_get_col_num(cw_lexer_t * a_lexer,
		  cw_bool_t a_last_or_next_tok)
{
  return 0; /* XXX */
}

cw_bool_t
lexer_reject(cw_lexer_t * a_lexer)
{
  return TRUE; /* XXX */
}

cw_bool_t
lexer_push_text(cw_lexer_t * a_lexer, cw_uint32_t a_strlen,
		const char * a_text)
{
  return TRUE; /* XXX */
}

cw_bool_t
lexer_less(cw_lexer_t * a_lexer, cw_uint32_t a_num_chars)
{
  return TRUE; /* XXX */
}

cw_bool_t
lexer_more(cw_lexer_t * a_lexer)
{
  return TRUE; /* XXX */
}

cw_lexertok_t *
lexertok_new(cw_lexertok_t * a_lexertok)
{
  return NULL; /* XXX */
}

void
lexertok_delete(cw_lexertok_t * a_lexertok)
{
}

cw_uint32_t
lexertok_get_tok_val(cw_lexertok_t * a_lexertok)
{
  return 0; /* XXX */
}

const char *
lexertok_get_str(cw_lexertok_t * a_lexertok)
{
  return NULL; /* XXX */
}
