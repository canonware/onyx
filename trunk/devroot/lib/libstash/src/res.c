/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 108 $
 * Last modified: $Date: 1998-06-30 00:07:07 -0700 (Tue, 30 Jun 1998) $
 *
 * <<< Description >>>
 *
 * Very simple resources class.  A resource looks like:
 *
 * <resource> ::= <^> <whitespaces> <name> <whitespaces> <linebreaks> <colon>
 *                <whitespaces> <linebreaks> <value> <comment>
 *              | <^> <whitespaces> <comment>
 * <name> ::= { <caps> | <lower> | <numbers> | <under> | <period> }+
 * <value> ::= { <caps> | <lower> | <numbers> | <under> | <period>
 *               | <backslash> <hash> | <whitespace> | <colon>
 *               | <backslash> <backslash> | <backslash> <n>
 *               | <legal_in_name> | <linebreak> }+
 *           | <e>
 * <comment> ::= <hash> { <caps> | <lower> | <numbers> | <under> | <period>
 *                        | <hash> | <whitespace> | <colon>
 *                        | <backslash> | <legal_in_name> }+
 *             | <e>
 * <linebreak> ::= <backslash> <whitespaces> <newline>
 * <linebreaks> ::= <linebreak> <linebreaks>
 *                | <e>
 * <^> ::= Bound to beginning of line.
 * <e> ::= Epsilon.
 * <n> ::= [n]
 * <caps> ::= [A-Z]
 * <lower> ::= [a-z]
 * <numbers> ::= [0-9]
 * <under> ::= [_]
 * <period> ::= [.]
 * <hash> ::= [#]
 * <whitespace> ::= [ \t] | <whitespace> [ \t]
 * <whitespaces> ::= <whitespace> <whitespaces>
 *                 | <e>
 * <colon> ::= [:]
 * <backslash> ::= [\\]
 * <legal_in_name> ::= [!"$%&'()*+,-/;<=>?@[]^`{|}~]
 * <newline> ::= [\n]
 * <null> ::= [\0]
 *
 * \ is a special character within <name>.  \ protects [#\\\n] and [ ]+[\n],
 * but a \ followed by anything else is an error.
 *
 *****************************************************************************/

#define _INC_STRING_H_
#define _INC_STDARG_H_
#define _INC_RES_H_
#define _INC_OH_H_
#include <config.h>
#include <res_priv.h>

/* Size of buffer to use for name/value parsing.  In practice, this is
 * probably plenty, but in theory, any arbitrary limitation is bad. */
#define _CW_RES_BUFFSIZE 8192

/* Character types for state machine. */
#define _CW_RES_CHAR_CAP 0
#define _CW_RES_CHAR_LOWER 1
#define _CW_RES_CHAR_NUMBER 2
#define _CW_RES_CHAR_UNDER 3
#define _CW_RES_CHAR_PERIOD 4
#define _CW_RES_CHAR_HASH 5
#define _CW_RES_CHAR_WHITESPACE 6
#define _CW_RES_CHAR_COLON 7
#define _CW_RES_CHAR_BACKSLASH 8
#define _CW_RES_CHAR_NEWLINE 9
#define _CW_RES_CHAR_NULL 10
#define _CW_RES_CHAR_VALID_IN_VAL 11
#define _CW_RES_CHAR_OTHER 12

/* State machine states. */
#define _CW_RES_STATE_START 0
#define _CW_RES_STATE_BEGIN_WHITESPACE 1
#define _CW_RES_STATE_BEGIN_COMMENT 2
#define _CW_RES_STATE_NAME 3
#define _CW_RES_STATE_POST_NAME_WHITESPACE 4
#define _CW_RES_STATE_POST_COLON_WHITESPACE 5
#define _CW_RES_STATE_VALUE 6
#define _CW_RES_STATE_VALUE_BACKSLASH 7
#define _CW_RES_STATE_BACKSLASH_WHITESPACE 8
#define _CW_RES_STATE_TRAILING_COMMENT 9
#define _CW_RES_STATE_FINISH 10

cw_res_t *
res_new(cw_res_t * a_res_o)
{
  cw_res_t * retval;

  if (a_res_o == NULL)
  {
    retval = (cw_res_t *) _cw_malloc(sizeof(cw_res_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_res_o;
    retval->is_malloced = FALSE;
  }

  /* Initialize internals. */
  rwl_new(&retval->rw_lock);
  /* Non-thread-safe hash table, since we're already taking care of the
   * locking. */
  oh_new(&retval->hash_o, FALSE);

  return retval;
}

void
res_delete(cw_res_t * a_res_o)
{
  _cw_check_ptr(a_res_o);

  /* Clean up internals. */
  rwl_delete(&a_res_o->rw_lock);
  oh_delete(&a_res_o->hash_o);
  
  if (a_res_o->is_malloced)
  {
    _cw_free(a_res_o);
  }
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == error 
 *
 * <<< Description >>>
 *
 * Merges the resources contained in a_filename into the resource database.
 *
 ****************************************************************************/
cw_bool_t
res_merge_file(cw_res_t * a_res_o, char * a_filename)
{
  cw_bool_t retval = FALSE, state_mach_error;
  int error;
  FILE * fd;
    
  _cw_check_ptr(a_res_o);
  rwl_wlock(&a_res_o->rw_lock);

  fd = fopen(a_filename, "r");
  if (fd == NULL)
  {
    retval = TRUE;
  }
  else
  {
    /* Run the state machine on the file. */
    state_mach_error = res_parse_res(a_res_o, TRUE);
    if (state_mach_error == TRUE)
    {
      retval = TRUE;
    }

    /* Close the file. */
    error = fclose(fd);
    if (error)
    {
      retval = TRUE;
    }
  }

  rwl_wunlock(&a_res_o->rw_lock);
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 * ... : NULL-terminated list of resource name/value pair strings.
 *
 * <<< Return Value >>>
 *
 * TRUE == error
 *
 * <<< Description >>>
 *
 * Merges the resources into the resource database.
 *
 ****************************************************************************/
cw_bool_t
res_merge_list(cw_res_t * a_res_o, ...)
{
  va_list ap;
  cw_bool_t retval = FALSE, state_mach_error;
  
  _cw_check_ptr(a_res_o);
  rwl_wlock(&a_res_o->rw_lock);

  /* Run the strings through the insertion state machine. */
  va_start(ap, a_res_o);
  for (a_res_o->str = va_arg(ap, char *);
       ((a_res_o->str != NULL) && (retval != TRUE));
       a_res_o->str = va_arg(ap, char *))
  {
    state_mach_error = res_parse_res(a_res_o, FALSE);
    if (state_mach_error == TRUE)
    {
      retval = TRUE;
    }
  }
  va_end(ap);
  
  rwl_wunlock(&a_res_o->rw_lock);
  return retval;
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * NULL == error
 *
 * <<< Description >>>
 *
 * Returns the value associated with a_res_name if it exists.  Note that it
 * returns a pointer to an internal buffer, so if any modifications need
 * to be made to the string, make a copy and modify it instead.
 *
 ****************************************************************************/
char *
res_get_res_val(cw_res_t * a_res_o, char * a_res_name)
{
  char * retval;
  cw_bool_t error;
  
  _cw_check_ptr(a_res_o);
  _cw_check_ptr(a_res_name);
  rwl_rlock(&a_res_o->rw_lock);

  error = oh_item_search(&a_res_o->hash_o, (void *) a_res_name,
			 (void **) &retval);
  if (error == TRUE)
  {
    retval = NULL;
  }
  
  rwl_runlock(&a_res_o->rw_lock);
  return retval;
}
		
/****************************************************************************
 * <<< Description >>>
 *
 * Dumps the resource database.
 *
 * XXX Since the database is stored in a hash table, the only reliable way
 * to achieve this is to iteratively delete the items in the table and
 * build a new one.  This is rather ugly, and I'm not so sure I want to do
 * it.  Therefore, this function is unimplemented for the time being.
 *
 ****************************************************************************/
void
res_dump(cw_res_t * a_res_o)
{
  _cw_check_ptr(a_res_o);

  _cw_error("Not implemented.");
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == error
 *
 * <<< Description >>>
 *
 * Private method.  Parses the resources contained either in a string or in 
 * a file and inserts the results into the hash table.
 *
 ****************************************************************************/
cw_bool_t
res_parse_res(cw_res_t * a_res_o, cw_bool_t a_is_file)
{
  cw_bool_t retval = FALSE;
  size_t i, name_pos = 0, val_pos = 0;
  cw_uint32_t state = _CW_RES_STATE_START, col_num, line_num = 1;
  char c, name[_CW_RES_BUFFSIZE], val[_CW_RES_BUFFSIZE];

  /* These switch statements look awful, but they should be fast, since the
   * compiler supposedly builds perfect hashes for them.  I'm not sure why
   * I'm so worried about performance.  Oh well, it's neat at least. */
  for (i = 0;
       ((state != _CW_RES_STATE_FINISH) && (retval != TRUE));
       i++, col_num++)
  {
    /* XXX Check whether we overflowed the buffers.  Perhaps we should move
     * to extensible buffers, once they're written for the socket code. */
    _cw_assert(name_pos < _CW_RES_BUFFSIZE);
    _cw_assert(val_pos < _CW_RES_BUFFSIZE);
    
    /* Read the next character in. */
    if (a_is_file)
    {
      c = (char) getc(a_res_o->fd);
      if (c == EOF)
      {
	/* Make sure it's an EOF, not an error. */
	if (ferror(a_res_o->fd))
	{
	  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	  {
	    log_printf(g_log_o, "res_parse_res(): Error reading from file\n");
	  }
	  retval = TRUE;
	  break;
	}
	else
	{
	  /* Just in case EOF != NULL (is this possible?). */
	  c = '\0';
	}
      }
    }
    else
    {
      c = a_res_o->str[i];
    }
    
    if (dbg_fmatch(g_dbg_o, _CW_DBG_R_RES_STATE))
    {
      log_printf(g_log_o, "res_parse_res(): State == %d, Input == \'%c\'\n",
		 state, c);
    }
    
    switch (state)
    {
      /* Starting state. */
      case _CW_RES_STATE_START:
      {
	/* Initialize counters, buffers, etc. */
	name_pos = 0;
	val_pos = 0;
	col_num = 1;
	/* Truncate.  Not strictly necessary with static buffers. */
	/* 	name = '\0'; */
	/* 	val = '\0'; */
	
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  {
	    /* First character of the name. */
	    name[name_pos] = c;
	    name_pos++;
	    state = _CW_RES_STATE_NAME;
	    break;
	  }
	  case _CW_RES_CHAR_HASH:
	  {
	    /* Beginning of comment.  Throw the character away. */
	    state = _CW_RES_STATE_BEGIN_COMMENT;
	    break;
	  }
	  case _CW_RES_CHAR_WHITESPACE:
	  {
	    /* Leading whitespace.  Swallow it. */
	    break;
	  }
	  case _CW_RES_CHAR_NEWLINE:
	  {
	    /* Leading whitespace.  Swallow it. */
	    line_num++;
	    col_num = 1;
	    break;
	  }
	  case _CW_RES_CHAR_NULL:
	  {
	    /* Completely empty string or file. */
	    state = _CW_RES_STATE_FINISH;
	    break;
	  }
	  case _CW_RES_CHAR_COLON:
	  case _CW_RES_CHAR_BACKSLASH:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_START, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Whitespace that precedes the resource name. */
      case _CW_RES_STATE_BEGIN_WHITESPACE:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  {
	    /* First character of the name. */
	    name[name_pos] = c;
	    name_pos++;
	    state = _CW_RES_STATE_NAME;
	    break;
	  }
	  case _CW_RES_CHAR_HASH:
	  {
	    /* Beginning of a comment. */
	    state = _CW_RES_STATE_BEGIN_COMMENT;
	    break;
	  }
	  case _CW_RES_CHAR_WHITESPACE:
	  {
	    /* More whitespace.  Swallow it. */
	    break;
	  }
	  case _CW_RES_CHAR_NEWLINE:
	  {
	    /* Blank line.  Jump back to the start state to make sure all
	     * counters are correctly reset. */
	    line_num++;
	    col_num = 1;
	    state = _CW_RES_STATE_START;
	    break;
	  }
	  case _CW_RES_CHAR_NULL:
	  {
	    /* String is legal, but contains no resource. */
	    state = _CW_RES_STATE_START;
	    break;
	  }
	  case _CW_RES_CHAR_COLON:
	  case _CW_RES_CHAR_BACKSLASH:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_BEGIN_WHITESPACE, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Comment that was preceded only by whitespace characters. */
      case _CW_RES_STATE_BEGIN_COMMENT:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  case _CW_RES_CHAR_HASH:
	  case _CW_RES_CHAR_WHITESPACE:
	  case _CW_RES_CHAR_COLON:
	  case _CW_RES_CHAR_BACKSLASH:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _CW_RES_CHAR_NEWLINE:
	  {
	    /* Go back to the initial state. */
	    line_num++;
	    col_num = 1;
	    state = _CW_RES_STATE_START;
	    break;
	  }
	  case _CW_RES_CHAR_NULL:
	  {
	    /* String is legal, but contains no resource. */
	    state = _CW_RES_STATE_FINISH;
	    break;
	  }
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_BEGIN_COMMENT, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Resource name state. */
      case _CW_RES_STATE_NAME:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  {
	    /* Character of the resource name. */
	    name[name_pos] = c;
	    name_pos++;
	    break;
	  }
	  case _CW_RES_CHAR_WHITESPACE:
	  {
	    /* End of name.  Jump to a state that can deal with additional
	     * whitespace. */
	    name[name_pos] = '\0';
	    state = _CW_RES_STATE_POST_NAME_WHITESPACE;
	    break;
	  }
	  case _CW_RES_CHAR_COLON:
	  {
	    /* Okay, here's the colon.  Terminate the name and jump to a
	     * state that can deal with whitespace leading the value. */
	    name[name_pos] = '\0';
	    state = _CW_RES_STATE_POST_COLON_WHITESPACE;
	    break;
	  }
	  case _CW_RES_CHAR_HASH:
	  case _CW_RES_CHAR_BACKSLASH:
	  case _CW_RES_CHAR_NEWLINE:
	  case _CW_RES_CHAR_NULL:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_NAME, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Swallow whitespace following the resource name. */
      case _CW_RES_STATE_POST_NAME_WHITESPACE:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_COLON:
	  {
	    /* Here's the colon.  Jump to a state that can deal with
	     * whitespace leading the value. */
	    state = _CW_RES_STATE_POST_COLON_WHITESPACE;
	    break;
	  }
	  case _CW_RES_CHAR_WHITESPACE:
	  {
	    /* Additional whitespace.  Swallow it. */
	    break;
	  }
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  case _CW_RES_CHAR_HASH:
	  case _CW_RES_CHAR_BACKSLASH:
	  case _CW_RES_CHAR_NEWLINE:
	  case _CW_RES_CHAR_NULL:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_POST_NAME_WHITESPACE, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Swallow whitespace following the colon. */
      case _CW_RES_STATE_POST_COLON_WHITESPACE:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  case _CW_RES_CHAR_COLON:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Beginning of value. */
	    val[val_pos] = c;
	    val_pos++;
	    state = _CW_RES_STATE_VALUE;
	    break;
	  }
	  case _CW_RES_CHAR_WHITESPACE:
	  {
	    /* More whitespace.  Swallow it. */
	    break;
	  }
	  case _CW_RES_CHAR_HASH:
	  {
	    /* Empty value.  NULL-terminate the string and jump to the
	     * trailing comment state. */
	    val[val_pos] = '\0';
	    res_merge_res(a_res_o, name, val);
	    state = _CW_RES_STATE_TRAILING_COMMENT;
	    break;
	  }
	  case _CW_RES_CHAR_BACKSLASH:
	  {
	    /* Beginning of value, but it's a backslash, so jump to the
	     * backslash handling state. */
	    state = _CW_RES_STATE_VALUE_BACKSLASH;
	    break;
	  }
	  case _CW_RES_CHAR_NEWLINE:
	  {
	    /* Empty value.  Insert it though. */
	    line_num++;
	    col_num = 1;
	    val[val_pos] = '\0';
	    res_merge_res(a_res_o, name, val);
	    state = _CW_RES_STATE_START;
	    break;
	  }
	  case _CW_RES_CHAR_NULL:
	  {
	    /* Empty value, and end of input.  Insert the resource. */
	    val[val_pos] = '\0';
	    res_merge_res(a_res_o, name, val);
	    state = _CW_RES_STATE_FINISH;
	    break;
	  }
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_POST_COLON_WHITESPACE, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Resource value. */
      case _CW_RES_STATE_VALUE:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  case _CW_RES_CHAR_COLON:
	  case _CW_RES_CHAR_WHITESPACE: /* Allow whitespace in value. */
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  {
	    /* More of the value. */
	    val[val_pos] = c;
	    val_pos++;
	    state = _CW_RES_STATE_VALUE;
	    break;
	  }
	  case _CW_RES_CHAR_HASH:
	  {
	    /* Beginning of comment, and therefore the end of the value.
	     * NULL-terminate the string, insert the resource, and jump to
	     * the trailing comment state. */
	    val[val_pos] = '\0';
	    res_merge_res(a_res_o, name, val);
	    state = _CW_RES_STATE_TRAILING_COMMENT;
	    break;
	  }
	  case _CW_RES_CHAR_BACKSLASH:
	  {
	    /* Backslash.  Jump to the backslash state. */
	    state = _CW_RES_STATE_VALUE_BACKSLASH;
	    break;
	  }
	  case _CW_RES_CHAR_NEWLINE:
	  {
	    /* End of line, and therefore the end of the value.
	     * NULL-terminate the string, insert the resource in the hash
	     * table, and jump back to the starting state. */
	    line_num++;
	    col_num = 1;
	    val[val_pos] = '\0';
	    res_merge_res(a_res_o, name, val);
	    state = _CW_RES_STATE_START;
	    break;
	  }
	  case _CW_RES_CHAR_NULL:
	  {
	    /* Do the same thing as for a newline, except that we want the
	     * state machine to exit. */
	    val[val_pos] = '\0';
	    res_merge_res(a_res_o, name, val);
	    state = _CW_RES_STATE_FINISH;
	    break;
	  }
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_VALUE, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Backslash within the resource value. */
      case _CW_RES_STATE_VALUE_BACKSLASH:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_BACKSLASH:
	  {
	    /* Insert a backslash and jump back to the value state. */
	    val[val_pos] = '\\';
	    val_pos++;
	    state = _CW_RES_STATE_VALUE;
	    break;
	  }
	  case _CW_RES_CHAR_HASH:
	  {
	    /* Insert a hash and jump back to the value state. */
	    val[val_pos] = '#';
	    val_pos++;
	    state = _CW_RES_STATE_VALUE;
	    break;
	  }
	  case _CW_RES_CHAR_WHITESPACE:
	  {
	    /* We need to make sure that what follows is whitespace
	     * followed by a newline, but we can't do that if we stay in
	     * this state, since we woudn't notice '\\' and '#' later on in
	     * the stream. */
	    state = _CW_RES_STATE_BACKSLASH_WHITESPACE;
	    break;
	  }
	  case _CW_RES_CHAR_NEWLINE:
	  {
	    /* \ continuation.  Swallow this and jump back to the value
	     * state. */
	    line_num++;
	    col_num = 1;
	    state = _CW_RES_STATE_VALUE;
	    break;
	  }
	  case _CW_RES_CHAR_LOWER:
	  {
	    if (c == 'n')
	    {
	      /* Insert a carriage return and jump back to the value
	       * state.  Yes, this is a gross hack, and violation of the
	       * otherwise purity of the state machine and character
	       * classes, but to have made a special character class just
	       * for this would probably have made the code less
	       * understandable. */
	      val[val_pos] = '\n';
	      val_pos++;
	      state = _CW_RES_STATE_VALUE;
	      break;
	    }
	    /* Note that if it's not an 'n', we fall through to the error
	     * case. */
	  }
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  case _CW_RES_CHAR_COLON:
	  case _CW_RES_CHAR_NULL:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_VALUE_BACKSLASH, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      case _CW_RES_STATE_BACKSLASH_WHITESPACE:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_WHITESPACE:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _CW_RES_CHAR_NEWLINE:
	  {
	    /* \ continuation.  Swallow this and jump back to the value
	     * state. */
	    line_num++;
	    col_num = 1;
	    state = _CW_RES_STATE_VALUE;
	    break;
	  }
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  case _CW_RES_CHAR_HASH:
	  case _CW_RES_CHAR_COLON:
	  case _CW_RES_CHAR_BACKSLASH:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  case _CW_RES_CHAR_NULL:
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_BACKSLASH_WHITESPACE, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Comment at end of resource. */
      case _CW_RES_STATE_TRAILING_COMMENT:
      {
	switch (res_char_type(c))
	{
	  case _CW_RES_CHAR_CAP:
	  case _CW_RES_CHAR_LOWER:
	  case _CW_RES_CHAR_NUMBER:
	  case _CW_RES_CHAR_UNDER:
	  case _CW_RES_CHAR_PERIOD:
	  case _CW_RES_CHAR_HASH:
	  case _CW_RES_CHAR_WHITESPACE:
	  case _CW_RES_CHAR_COLON:
	  case _CW_RES_CHAR_BACKSLASH:
	  case _CW_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _CW_RES_CHAR_NEWLINE:
	  {
	    /* Okay, end of comment.  Jump back to the starting state. */
	    line_num++;
	    col_num = 1;
	    state = _CW_RES_STATE_START;
	    break;
	  }
	  case _CW_RES_CHAR_NULL:
	  {
	    /* End of input.  Finish. */
	    state = _CW_RES_STATE_FINISH;
	    break;
	  }
	  case _CW_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	    {
	      log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			  "Illegal character while in _CW_STATE_TRAILING_COMMENT, line %d, column %d\n",
			  line_num, col_num);
	    }
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      default:
      {
	if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_ERROR))
	{
	  log_eprintf(g_log_o, NULL, 0, "res_parse_res",
		      "Jumped to non-existant state, line %d, column %d\n",
		      line_num, col_num);
	}
	retval = TRUE;
	break;
      }
    }
  }

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the "type" of a_char.
 *
 ****************************************************************************/
cw_uint32_t
res_char_type(char a_char)
{
  cw_uint32_t retval;
  
  switch (a_char)
  {
    /* Capital letters. */
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    {
      retval = _CW_RES_CHAR_CAP;
      break;
    }
    
    /* Lower case letters. */
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
    {
      retval = _CW_RES_CHAR_LOWER;
      break;
    }

    /* Numbers. */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
      retval = _CW_RES_CHAR_NUMBER;
      break;
    }
    
    /* Underscore. */
    case '_':
    {
      retval = _CW_RES_CHAR_UNDER;
      break;
    }
    
    /* Period. */
    case '.':
    {
      retval = _CW_RES_CHAR_PERIOD;
      break;
    }
    
    /* Start comment. */
    case '#':
    {
      retval = _CW_RES_CHAR_HASH;
      break;
    }
    
    /* Whitespace. */
    case '\t': case ' ':
    {
      retval = _CW_RES_CHAR_WHITESPACE;
      break;
    }
    
    /* Colon. */
    case ':':
    {
      retval = _CW_RES_CHAR_COLON;
      break;
    }
    
    /* Backslash. */
    case '\\':
    {
      retval = _CW_RES_CHAR_BACKSLASH;
      break;
    }

    /* Carriage return. */
    case '\n':
    {
      retval = _CW_RES_CHAR_NEWLINE;
      break;
    }
    
    /* Null terminator. */
    case '\0':
    {
      retval = _CW_RES_CHAR_NULL;
      break;
    }

    /* Other valid characters within a resource values. */
    case '!': case '"': case '$': case '%': case '&': case '\'': case '(':
    case ')': case '*': case '+': case ',': case '-': case '/': case ';':
    case '<': case '=': case '>': case '?': case '@': case '[': case ']':
    case '^': case '`': case '{': case '|': case '}': case '~':
    {
      retval = _CW_RES_CHAR_VALID_IN_VAL;
      break;
    }
    
    /* Something we're not expecting at all. */
    default:
    {
      retval = _CW_RES_CHAR_OTHER;
      break;
    }
  }

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Merges a resource into the hash table, taking care to clean up any
 * entry it replaces.  This is private, because we don't want to trust any
 * methods external to res to give us valid resources.
 *
 ****************************************************************************/
void
res_merge_res(cw_res_t * a_res_o, char * a_name, char * a_val)
{
  char * temp_name, * temp_val;
  cw_bool_t error;
	    
  /* Make copies to insert into the hash table. */
  temp_name = (char *) _cw_malloc(strlen(a_name) + 1);
  strcpy(temp_name, a_name);
  temp_val = (char *) _cw_malloc(strlen(a_val) + 1);
  strcpy(temp_val, a_val);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_RES_STATE))
  {
    log_printf(g_log_o,
	       "res_merge_res(): Merging name == :%s:, value == :%s:\n",
	       a_name, a_val);
  }

  /* Insert the resource into the hash table. */
  error = oh_item_insert(&a_res_o->hash_o, (void *) temp_name,
			 (void *) temp_val);
  if (error == TRUE)
  {
    char * old_name, * old_val;
	      
    /* The resource already exists.  That means we need to delete the
     * existing one, free the resources that are taken up by it, and redo
     * the insertion. */
    oh_item_delete(&a_res_o->hash_o, (void *) temp_name,
		   (void **) &old_name,
		   (void **) &old_val);
    _cw_free(old_name);
    _cw_free(old_val);

    oh_item_insert(&a_res_o->hash_o, (void *) temp_name,
		   (void *) temp_val);
  }
}
