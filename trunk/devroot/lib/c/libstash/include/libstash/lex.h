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
cw_lex_t *
lex_new();

#define lex_delete _CW_NS_STASH(lex_delete)
void
lex_delete(cw_lex_t * a_lex);

#define lex_insert_regex _CW_NS_STASH(lex_insert_regex)
cw_bool_t
lex_insert_regex(cw_lex_t * a_lex, char * a_regex, cw_uint32_t a_tok_val);

#define lex_delete_regex _CW_NS_STASH(lex_delete_regex)
char *
lex_delete_regex(cw_lex_t * a_lex, char * a_regex);

#define lex_build_table _CW_NS_STASH(lex_build_table)
cw_bool_t
lex_build_table(cw_lex_t * a_lex);

#define lexer_new _CW_NS_STASH(lexer_new)
cw_lexer_t *
lexer_new(cw_lexer_t * a_lexer);

#define lexer_delete _CW_NS_STASH(lexer_delete)
void
lexer_delete(cw_lexer_t * a_lexer);

#define lexer_set_lex _CW_NS_STASH(lexer_set_lex)
void
lexer_set_lex(cw_lexer_t * a_lexer, cw_lex_t * a_lex);

#define lexer_connect_buf _CW_NS_STASH(lexer_connect_buf)
cw_bool_t
lexer_connect_buf(cw_lexer_t * a_lexer, cw_buf_t * a_buf);

#define lexer_get_token _CW_NS_STASH(lexer_get_token)
cw_lexertok_t *
lexer_get_token(cw_lexer_t * a_lexer);

#define lexer_get_line_num _CW_NS_STASH(lexer_get_line_num)
cw_uint32_t
lexer_get_line_num(cw_lexer_t * a_lexer, cw_bool_t a_last_or_next_tok);

#define lexer_set_line_num _CW_NS_STASH(lexer_set_line_num)
cw_bool_t
lexer_set_line_num(cw_lexer_t * a_lexer, cw_uint32_t a_line_num);

#define lexer_get_col_num _CW_NS_STASH(lexer_get_col_num)
cw_uint32_t
lexer_get_col_num(cw_lexer_t * a_lexer, cw_bool_t a_last_or_next_tok);

#define lexer_reject _CW_NS_STASH(lexer_reject)
cw_bool_t
lexer_reject(cw_lexer_t * a_lexer);

#define lexer_push_text _CW_NS_STASH(lexer_push_text)
cw_bool_t
lexer_push_text(cw_lexer_t * a_lexer, cw_uint32_t a_strlen, char * a_text);

#define lexer_less _CW_NS_STASH(lexer_less)
cw_bool_t
lexer_less(cw_lexer_t * a_lexer, cw_uint32_t a_num_chars);

#define lexer_more _CW_NS_STASH(lexer_more)
cw_bool_t
lexer_more(cw_lexer_t * a_lexer);

#define lexertok_new _CW_NS_STASH(lexertok_new)
cw_lexertok_t *
lexertok_new(cw_lexertok_t * a_lexertok);

#define lexertok_delete _CW_NS_STASH(lexertok_delete)
void
lexertok_delete(cw_lexertok_t * a_lexertok);

#define lexertok_get_tok_val _CW_NS_STASH(lexertok_get_tok_val)
cw_uint32_t
lexertok_get_tok_val(cw_lexertok_t * a_lexertok);

#define lexertok_get_str _CW_NS_STASH(lexertok_get_str)
char *
lexertok_get_str(cw_lexertok_t * a_lexertok);
