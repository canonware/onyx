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
 * Public interface for the lex (lexer state machine) and lexer (lexer
 * instance that uses a lex instance) classes.
 *
 ****************************************************************************/

/* Pseudo-opaque types. */
typedef struct cw_lexer_s cw_lexer_t;
typedef struct cw_lexertok_s cw_lexertok_t;

/* Opaque types. */
typedef struct cw_lex_s cw_lex_t;
/* XXX Does this need to be visible at all? */
typedef struct cw_lexs_s cw_lexs_t;

struct cw_lexer_s
{
  cw_uint32_t line_num;
  cw_uint32_t col_num;
};

struct cw_lexertok_s
{
  cw_uint32_t length;
  cw_uint32_t num_lexs_structs;
  cw_lexs_t * strings;
  cw_uint32_t tok_val;
};

#define lex_new _CW_NS_STASH(lex_new)
#define lex_delete _CW_NS_STASH(lex_delete)
#define lex_insert_regex _CW_NS_STASH(lex_insert_regex)
#define lex_delete_regex _CW_NS_STASH(lex_delete_regex)
#define lex_build_table _CW_NS_STASH(lex_build_table)

#define lexer_new _CW_NS_STASH(lexer_new)
#define lexer_delete _CW_NS_STASH(lexer_delete)
#define lexer_set_lex _CW_NS_STASH(lexer_set_lex)
#define lexer_connect_buf _CW_NS_STASH(lexer_connect_buf)
#define lexer_get_token _CW_NS_STASH(lexer_get_token)
#define lexer_get_line_num _CW_NS_STASH(lexer_get_line_num)
#define lexer_set_line_num _CW_NS_STASH(lexer_set_line_num)
#define lexer_get_col_num _CW_NS_STASH(lexer_get_col_num)
#define lexer_reject _CW_NS_STASH(lexer_reject)
#define lexer_push_text _CW_NS_STASH(lexer_push_text)
#define lexer_less _CW_NS_STASH(lexer_less)
#define lexer_more _CW_NS_STASH(lexer_more)

#define lexertok_new _CW_NS_STASH(lexertok_new)
#define lexertok_delete _CW_NS_STASH(lexertok_delete)
#define lexertok_get_tok_val _CW_NS_STASH(lexertok_get_tok_val)
#define lexertok_get_str _CW_NS_STASH(lexertok_get_str)

/* Public lex interface. */
cw_lex_t * lex_new();
void lex_delete(cw_lex_t * a_lex_o);
cw_bool_t lex_insert_regex(cw_lex_t * a_lex_o, char * a_regex,
			   cw_uint32_t a_tok_val);

char * lex_delete_regex(cw_lex_t * a_lex_o, char * a_regex);
cw_bool_t lex_build_table(cw_lex_t * a_lex_o);

/* Public lexer interface. */
cw_lexer_t * lexer_new(cw_lexer_t * a_lexer_o);
void lexer_delete(cw_lexer_t * a_lexer_o);

void lexer_set_lex(cw_lexer_t * a_lexer_o, cw_lex_t * a_lex_o);
cw_bool_t lexer_connect_buf(cw_lexer_t * a_lexer_o, cw_buf_t * a_buf_o);

cw_lexertok_t * lexer_get_token(cw_lexer_t * a_lexer_o);

cw_uint32_t lexer_get_line_num(cw_lexer_t * a_lexer_o,
			       cw_bool_t a_last_or_next_tok);

cw_bool_t lexer_set_line_num(cw_lexer_t * a_lexer_o, cw_uint32_t a_line_num);
cw_uint32_t lexer_get_col_num(cw_lexer_t * a_lexer_o,
			      cw_bool_t a_last_or_next_tok);
cw_bool_t lexer_reject(cw_lexer_t * a_lexer_o);
cw_bool_t lexer_push_text(cw_lexer_t * a_lexer_o, cw_uint32_t a_strlen,
			  char * a_text);
cw_bool_t lexer_less(cw_lexer_t * a_lexer_o, cw_uint32_t a_num_chars);

cw_bool_t lexer_more(cw_lexer_t * a_lexer_o);

/* Public lexertok interface. */
cw_lexertok_t * lexertok_new(cw_lexertok_t * a_lexertok_o);
void lexertok_delete(cw_lexertok_t * a_lexertok_o);

cw_uint32_t lexertok_get_tok_val(cw_lexertok_t * a_lexertok_o);
char * lexertok_get_str(cw_lexertok_t * a_lexertok_o);
