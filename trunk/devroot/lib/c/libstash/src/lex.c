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

#include "libstash/lex_priv.h"

/****************************************************************************
 *
 * Constructor.
 *
 ****************************************************************************/
cw_lex_t *
lex_new()
{
  return NULL; /* XXX */
}
     
/****************************************************************************
 *
 * Destructor.
 *
 ****************************************************************************/
void
lex_delete(cw_lex_t * a_lex_o)
{
}

/****************************************************************************
 *
 * Inserts a regex.  Note that the regex is not compiled into the state machine
 * until lex_build_table() is called.  a_regex is a string that conforms to the
 * regular expression syntax outlined below.  a_tok_val is a 32 bit number that
 * lexers return when the rule is matched by a token.
 *
 *
 *
 * Regex syntax and meaning:
 *
 *  Metacharacters |
 * ----------------+---------------------------------------------------------
 *  .              | Matches a single character, except for '\n' (0xd).
 * ----------------+---------------------------------------------------------
 *  \              | '\' is used as an escape character to include various
 *                 | unprintable (and printable) characters.  The following are
 *                 | legal:
 *                 |
 *                 |
 *                 |
 *                 | \a    - alert (bell).
 *                 | \b    - backspace.
 *                 | \f    - form feed.
 *                 | \n    - newline.
 *                 | \r    - carriage return.
 *                 | \t    - horizontal tab.
 *                 | \v    - vertical tab.
 *                 | \0    - ASCII zero (0x0)
 *                 | \d123 - character with decimal value 123.  The value must 
 *                 |         be (0 <= value <= 255).
 *                 | \o123 - character with octal value 123.  The value must be
 *                 |         (0 <= value <= 377).
 *                 | \xf3  - character with hex value f3.  The value must be
 *                 |         (0 <= value <= ff).
 *                 |
 *                 | '\' followed by any character other than 'a', 'b', 'd',
 *                 | 'f', 'n', 'o', 'r', 't', 'v', 'x', and '0' simply quotes
 *                 | the following character.  So, to include '\', use "\\".
 * ----------------+---------------------------------------------------------
 * ""              | Quotes the characters inside literally, other than '\',
 *                 | which acts as an escape character, just as outside quotes.
 *                 | When typed in code, this of course looks rather gross:
 *                 | "abc\"a literal [string]\"trail".  With '\' thrown in,
 *                 | things get especially ugly in code: 
 *                 | "abc\"a quoted string with \\\"quotes\\\"\"trail".
 * ----------------+---------------------------------------------------------
 *  []             | Matches any one of the characters within the brackets.
 *                 |
 *                 | If the first character within the brackets is '^', then
 *                 | match any character _except_ the ones within the brackets.
 *                 | To include a '^', put it anywhere other than the first
 *                 | position.
 *                 |
 *                 | In order to include a ']' in the set of characters to
 *                 | match, put it first inside the brackets: "[]ABC]".
 *                 |
 *                 | Ranges of characters can be specified by using the '-' 
 *                 | character.  For example, to specify all capital letters,
 *                 | use "[A-Z]", or all letters would be "[A-Za-z].  Any range
 *                 | can be used, as long as the beginning and end characters
 *                 | are printable (other than ']'), and the beginning character
 *                 | comes before the end character in the ASCII character set.
 *                 | In order to include it first inside the brackets: "[-A-Z]".
 *                 |
 *                 | In order to include both ']' and '-' in the set, the 
 *                 | characters must come first, as follows: "[]-0-9]".  The
 *                 | opposite order ("[-]A-Z]") will not have the same result.
 *                 |
 *                 | The '\' character works the same as outside brackets.
 * ----------------+---------------------------------------------------------
 *  ?              | Matches zero or one of the preceding expression.  For
 *                 | example, "X?Y?" matches zero or one 'X', followed by zero
 *                 | or one 'Y'.
 * ----------------+---------------------------------------------------------
 *  *              | Matches zero or more of the preceding expression.  For
 *                 | example, "a*" matches zero or more 'a' characters.  
 *                 | "[a-z]*" matches zero or more lower case letters.
 * ----------------+---------------------------------------------------------
 *  +              | Matches one or more of the prededing expression.  For
 *                 | example, "a+b" matches one or more 'a' characters, 
 *                 | followed by one 'b'.
 * ----------------+---------------------------------------------------------
 *  {}             | Specifies how many times to match the preceding expression.
 *                 | There are three ways these can be used:
 *                 |
 *                 | {3}    - 3 times.
 *                 | {2,10} - at least 2 times, but not more than 10 times.
 *                 | {3,}   - 3 or more times.
 *                 |
 *                 | For example, "[a-z]{3,5}" matches 3, 4, or 5 lower case
 *                 | letters.
 * ----------------+---------------------------------------------------------
 *  |              | Used to match either the preceding expression, or the next
 *                 | expression.  For example, "foo|bar" matches either "foo" or
 *                 | "bar".  "foo|ba[rR]" matches "foo", "bar", or "baR".
 *                 | Multiple '|' metacharacters can be chained: 
 *                 | "foo|bar|biz|baz".
 * ----------------+---------------------------------------------------------
 *  ()             | Groups a sequence of expressions as an expression.  Parens
 *                 | are useful with the '?', '*', and '+' metacharacters in the
 *                 | following way: "(abc)?".  Parens are useful with the '|'
 *                 | metacharacter in the following way: "(abc|def)?".
 * ----------------+---------------------------------------------------------
 *  /              | Matches the preceding expression iff followed by the 
 *                 | trailing expression.  For example, "abc/ABC" matches "abc"
 *                 | iff immediately followed by "ABC".  Each regex can only
 *                 | have one '/' metacharacter, and is incompatible with the
 *                 | '$'. metacharacter.
 * ----------------+---------------------------------------------------------
 *  ^              | If the first character in the expression, it anchors the
 *                 | expression to the beginning of a line.  Other than at the
 *                 | beginning of the line and as the first character inside
 *                 | square brackets, '^' is uninterpreted.  An expression
 *                 | anchoring example: "^abc".
 * ----------------+---------------------------------------------------------
 *  $              | If the last character of an expression, it anchors the 
 *                 | expression to the end of a line.  Other than at the end
 *                 | of the line, '$' in uninterpreted.  "abc$" has the same
 *                 | effect as "abc/\n".
 * ----------------+---------------------------------------------------------
 *
 ****************************************************************************/
cw_bool_t
lex_insert_regex(cw_lex_t * a_lex_o,
		 char * a_regex,
		 cw_uint32_t a_tok_val)
{
  /* Parse a_regex and generate a graph for it. */

  
  return TRUE; /* XXX */
}

/****************************************************************************
 *
 * Removes a regex.  Note that the regex is not removed from the state machine
 * until lex_build_table() is called.
 *
 ****************************************************************************/
char *
lex_delete_regex(cw_lex_t * a_lex_o, char * a_regex)
{
  return NULL; /* XXX */
}

/****************************************************************************
 *
 * Compiles the regexes into a useable state table.
 *
 ****************************************************************************/
cw_bool_t
lex_build_table(cw_lex_t * a_lex_o)
{
  return TRUE; /* XXX */
}

/****************************************************************************
 *
 * Constructor.
 *
 ****************************************************************************/
cw_lexer_t *
lexer_new(cw_lexer_t * a_lexer_o)
{
  return NULL; /* XXX */
}

/****************************************************************************
 *
 * Destructor.
 *
 ****************************************************************************/
void
lexer_delete(cw_lexer_t * a_lexer_o)
{
}

/****************************************************************************
 *
 * Set the lex state machine to be used.
 *
 ****************************************************************************/
void
lexer_set_lex(cw_lexer_t * a_lexer_o, cw_lex_t * a_lex_o)
{
}

/****************************************************************************
 *
 * Connect a buf to this lexer.  New data is read from the buf whenever
 * internally buffered data runs out.  Passing in a_buf_o == NULL disconnects
 * the current buf.
 *
 ****************************************************************************/
cw_bool_t
lexer_connect_buf(cw_lexer_t * a_lexer_o, cw_buf_t * a_buf_o)
{
  return TRUE; /* XXX */
}

/****************************************************************************
 *
 * Gets a token, as recognized by the internal buf instance.
 *
 ****************************************************************************/
cw_lexertok_t *
lexer_get_token(cw_lexer_t * a_lexer_o)
{
  return NULL; /* XXX */
}

/****************************************************************************
 *
 * Returns the line number (numbering starts at 1 by default).  If
 * a_last_or_next_tok is FALSE, returns the line number at the beginning of the
 * previous token match.  If a_last_or_next_tok is TRUE, returns the line number
 * immediately following the previous token match.
 *
 ****************************************************************************/
cw_uint32_t
lexer_get_line_num(cw_lexer_t * a_lexer_o,
		   cw_bool_t a_last_or_next_tok)
{
  return 0; /* XXX */
}

/****************************************************************************
 *
 * Sets the line number for the current point (immediately following the last
 * token match).
 *
 ****************************************************************************/
cw_bool_t
lexer_set_line_num(cw_lexer_t * a_lexer_o,
		   cw_uint32_t a_line_num)
{
  return TRUE; /* XXX */
}

/****************************************************************************
 *
 * Returns the column number (numbering starts at 1).  If a_last_or_next_tok is
 * FALSE, returns the column number at the beginning of the previous token
 * match.  If a_last_or_next_tok is TRUE, returns the column number immediately
 * following the previous token match.
 *
 ****************************************************************************/
cw_uint32_t
lexer_get_col_num(cw_lexer_t * a_lexer_o,
		  cw_bool_t a_last_or_next_tok)
{
  return 0; /* XXX */
}

/****************************************************************************
 *
 * Tells the lexer to find the next best (i.e. shorter) match.  Internally, this
 * pushes the token back on the input stream and goes into a much slower mode
 * where every state transition is kept track of in order to allow backtracking.
 * This means that the first time lexer_reject() is called for a token, things
 * are slow, but then there is enough cached information to make multiple
 * successive calls to lexer_reject() fast.
 *
 ****************************************************************************/
cw_bool_t
lexer_reject(cw_lexer_t * a_lexer_o)
{
  return TRUE; /* XXX */
}

/****************************************************************************
 *
 * Pushes arbitrary text onto the input stream.  This can be useful for programs
 * such as preprocessors that do recursive string substitutions.  Note that if
 * simply pushing unmodified input back onto the input stream, lexer_less() is
 * much more efficient.
 *
 ****************************************************************************/
cw_bool_t
lexer_push_text(cw_lexer_t * a_lexer_o, cw_uint32_t a_strlen,
		char * a_text)
{
  return TRUE; /* XXX */
}

/****************************************************************************
 *
 * Backs up the input by a_num_chars characters.
 *
 ****************************************************************************/
cw_bool_t
lexer_less(cw_lexer_t * a_lexer_o, cw_uint32_t a_num_chars)
{
  return TRUE; /* XXX */
}

/****************************************************************************
 *
 * Tells the lexer to prepend the previous match to the next match.
 *
 ****************************************************************************/
cw_bool_t
lexer_more(cw_lexer_t * a_lexer_o)
{
  return TRUE; /* XXX */
}

/****************************************************************************
 *
 * Constructor.
 *
 ****************************************************************************/
cw_lexertok_t *
lexertok_new(cw_lexertok_t * a_lexertok_o)
{
  return NULL; /* XXX */
}

/****************************************************************************
 *
 * Destructor.
 *
 ****************************************************************************/
void
lexertok_delete(cw_lexertok_t * a_lexertok_o)
{
}

/****************************************************************************
 *
 * Returns the token value.
 *
 ****************************************************************************/
cw_uint32_t
lexertok_get_tok_val(cw_lexertok_t * a_lexertok_o)
{
  return 0; /* XXX */
}

/****************************************************************************
 *
 * Returns the token string.  Note that this must be called before calling
 * lexer_get_token() again, since the string is not constructed unless asked
 * for.  After lexer_get_token() is called again, internal lexer buffers may be
 * discarded, resulting in undefined behavior for this function.
 *
 ****************************************************************************/
char *
lexertok_get_str(cw_lexertok_t * a_lexertok_o)
{
  return NULL; /* XXX */
}
