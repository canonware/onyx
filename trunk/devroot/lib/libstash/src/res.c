/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 224 $
 * Last modified: $Date: 1998-09-15 18:08:52 -0700 (Tue, 15 Sep 1998) $
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
 *             | <hash> <e>
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

#include <string.h>
#include <stdarg.h>

#define _INC_RES_H_
#define _INC_OH_H_
#include <libstash.h>
#include <res_priv.h>

/* Size of buffer to use for name/value parsing.  In practice, this is
 * probably plenty, but in theory, any arbitrary limitation is bad. */
#define _STASH_RES_BUFFSIZE 8192

/* Character types for state machine. */
#define _STASH_RES_CHAR_CAP 0
#define _STASH_RES_CHAR_LOWER 1
#define _STASH_RES_CHAR_NUMBER 2
#define _STASH_RES_CHAR_UNDER 3
#define _STASH_RES_CHAR_PERIOD 4
#define _STASH_RES_CHAR_HASH 5
#define _STASH_RES_CHAR_WHITESPACE 6
#define _STASH_RES_CHAR_COLON 7
#define _STASH_RES_CHAR_BACKSLASH 8
#define _STASH_RES_CHAR_NEWLINE 9
#define _STASH_RES_CHAR_NULL 10
#define _STASH_RES_CHAR_VALID_IN_VAL 11
#define _STASH_RES_CHAR_OTHER 12

/* State machine states. */
#define _STASH_RES_STATE_START 0
#define _STASH_RES_STATE_BEGIN_WHITESPACE 1
#define _STASH_RES_STATE_BEGIN_COMMENT 2
#define _STASH_RES_STATE_NAME 3
#define _STASH_RES_STATE_POST_NAME_WHITESPACE 4
#define _STASH_RES_STATE_POST_COLON_WHITESPACE 5
#define _STASH_RES_STATE_VALUE 6
#define _STASH_RES_STATE_VALUE_BACKSLASH 7
#define _STASH_RES_STATE_BACKSLASH_WHITESPACE 8
#define _STASH_RES_STATE_TRAILING_COMMENT 9
#define _STASH_RES_STATE_FINISH 10

cw_res_t *
res_new(cw_res_t * a_res_o)
{
  cw_res_t * retval;

  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_new()");
  }
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

  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_new()");
  }
  return retval;
}

void
res_delete(cw_res_t * a_res_o)
{
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_delete()");
  }
  _cw_check_ptr(a_res_o);

  /* Clean up internals. */
  rwl_delete(&a_res_o->rw_lock);
  oh_delete(&a_res_o->hash_o);
  
  if (a_res_o->is_malloced)
  {
    _cw_free(a_res_o);
  }
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_delete()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Clears out all resources.
 *
 ****************************************************************************/
void
res_clear(cw_res_t * a_res_o)
{
  char * key, * val;
  
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_clear()");
  }
  _cw_check_ptr(a_res_o);
  rwl_wlock(&a_res_o->rw_lock);

  while (FALSE == oh_item_delete_iterate(&a_res_o->hash_o, (void **) &key,
					 (void **) &val))
  {
    _cw_free(key);
    _cw_free(val);
  }
  
  rwl_wunlock(&a_res_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_clear()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
res_is_equal(cw_res_t * a_res_o, cw_res_t * a_other)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_is_equal()");
  }
  _cw_check_ptr(a_res_o);
  _cw_check_ptr(a_other);
  rwl_wlock(&a_res_o->rw_lock);
  rwl_rlock(&a_other->rw_lock);

  if (a_res_o == a_other)
  {
    /* Two pointers to the same instance. */
    retval = TRUE;
  }
  else if (oh_get_num_items(&a_res_o->hash_o)
	   != oh_get_num_items(&a_other->hash_o))
  {
    retval = FALSE;
  }
  else
  {
    cw_uint32_t i, num_resources;
    char * key, * val;

    num_resources = oh_get_num_items(&a_res_o->hash_o);
    
    for (i = 0, retval = FALSE; (i < num_resources) && (retval == FALSE); i++)
    {
      oh_item_delete_iterate(&a_res_o->hash_o, (void **) &key,
			     (void **) &val);

      if (NULL == res_get_res_val(a_other, key))
      {
	retval = TRUE;
      }

      oh_item_insert(&a_res_o->hash_o, key, val);
    }
  }
  
  rwl_runlock(&a_other->rw_lock);
  rwl_wunlock(&a_res_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_is_equal()");
  }
  return retval;
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
  
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_merge_file()");
  }
  _cw_check_ptr(a_res_o);
  rwl_wlock(&a_res_o->rw_lock);

  a_res_o->fd = fopen(a_filename, "r");
  if (a_res_o->fd == NULL)
  {
    retval = TRUE;
  }
  else
  {
    /* Run the state machine on the file. */
    state_mach_error = res_p_parse_res(a_res_o, TRUE);
    if (state_mach_error == TRUE)
    {
      retval = TRUE;
    }

    /* Close the file. */
    error = fclose(a_res_o->fd);
    if (error)
    {
      retval = TRUE;
    }
  }

  rwl_wunlock(&a_res_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_merge_file()");
  }
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
  
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_merge_list()");
  }
  _cw_check_ptr(a_res_o);
  rwl_wlock(&a_res_o->rw_lock);

  /* Run the strings through the insertion state machine. */
  va_start(ap, a_res_o);
  for (a_res_o->str = va_arg(ap, char *);
       ((a_res_o->str != NULL) && (retval != TRUE));
       a_res_o->str = va_arg(ap, char *))
  {
    state_mach_error = res_p_parse_res(a_res_o, FALSE);
    if (state_mach_error == TRUE)
    {
      retval = TRUE;
    }
  }
  va_end(ap);
  
  rwl_wunlock(&a_res_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_merge_list()");
  }
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
  
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_get_res_val()");
  }
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
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_get_res_val()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Find a resource name/value pair, remove it from the resource database,
 * and set *a_res_name and *a_res_val to point it.  If the resource isn't
 * found, return TRUE.
 *
 ****************************************************************************/
cw_bool_t
res_extract_res(cw_res_t * a_res_o, char * a_res_key,
		char ** a_res_name, char ** a_res_val)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_extract_res()");
  }
  _cw_check_ptr(a_res_o);
  rwl_wlock(&a_res_o->rw_lock);

  retval = oh_item_delete(&a_res_o->hash_o, a_res_key,
			  (void **) a_res_name, (void **) a_res_val);

  rwl_wunlock(&a_res_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_extract_res()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Dump the resource database.  If a_filename is non-NULL, attempt to open
 * the specified file and write to it.  Otherwise, use g_log_o.
 *
 ****************************************************************************/
cw_bool_t
res_dump(cw_res_t * a_res_o, char * a_filename)
{
  cw_bool_t retval;
  cw_log_t * t_log_o;
  
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_dump()");
  }
  _cw_check_ptr(a_res_o);
  rwl_wlock(&a_res_o->rw_lock);

  if (a_filename != NULL)
  {
    t_log_o = log_new(NULL);
    if (log_set_logfile(t_log_o, a_filename, TRUE) == TRUE)
    {
      log_leprintf(g_log_o, NULL, 0, "res_dump",
		   "Error opening file \"%s\"\n", a_filename);
      retval = TRUE;
      goto RETURN;
    }
  }
  else
  {
    t_log_o = g_log_o;
  }

  retval = FALSE;

  /* Now dump the resources to t_log_o. */
  {
    cw_uint64_t num_items, i;
    char * key, * val;
    cw_uint32_t j, curr_offset, val_len;
    
    num_items = oh_get_num_items(&a_res_o->hash_o);

    for (i = 0; i < num_items; i++)
    {
      oh_item_delete_iterate(&a_res_o->hash_o, (void *) &key, (void *) &val);

      log_printf(t_log_o, "%s:", key);
      
      for (j = 0, curr_offset = 0, val_len = strlen(val);
	   j < val_len + 1;
	   j++)
      {
	if (val[j] == '\n')
	{
	  val[j] = '\0';
	  if ((j < val_len) && (val[j + 1] != '\0'))
	  {
	    log_printf(t_log_o, "%s\\n\\\n", (char *) (val + curr_offset));
	  }
	  else
	  {
	    log_printf(t_log_o, "%s\\n", (char *) (val + curr_offset));
	  }
	  val[j] = '\n';
	  curr_offset = j + 1;
	}
	else if (val[j] == '\0')
	{
	  log_printf(t_log_o, "%s\n", (char *) (val + curr_offset));
	}
      }
      
      oh_item_insert(&a_res_o->hash_o, key, val);
    }
  }

 RETURN:  
  if (a_filename != NULL)
  {
    log_delete(t_log_o);
  }
  rwl_wunlock(&a_res_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_dump()");
  }
  return retval;
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
res_p_parse_res(cw_res_t * a_res_o, cw_bool_t a_is_file)
{
  cw_bool_t retval = FALSE;
  size_t i, name_pos = 0, val_pos = 0;
  cw_uint32_t state = _STASH_RES_STATE_START, col_num, line_num = 1;
  char c, name[_STASH_RES_BUFFSIZE], val[_STASH_RES_BUFFSIZE];

  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_p_parse_res()");
  }
  for (i = 0, col_num = 1;
       ((state != _STASH_RES_STATE_FINISH) && (retval != TRUE));
       i++, col_num++)
  {
    /* XXX Check whether we overflowed the buffers.  Perhaps we should move
     * to extensible buffers, once they're written for the socket code. */
    _cw_assert(name_pos < _STASH_RES_BUFFSIZE);
    _cw_assert(val_pos < _STASH_RES_BUFFSIZE);
    
    /* Read the next character in. */
    if (a_is_file)
    {
      c = (char) getc(a_res_o->fd);
      if (c == EOF)
      {
	/* Make sure it's an EOF, not an error. */
	if (ferror(a_res_o->fd))
	{
	  log_printf(g_log_o, "res_parse_res(): Error reading from file\n");
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
    
    if (_cw_fmatch(_STASH_DBG_R_RES_STATE))
    {
      log_printf(g_log_o, "res_parse_res(): State == %d, Input == \'%c\'\n",
		 state, c);
    }
    
    switch (state)
    {
      /* Starting state. */
      case _STASH_RES_STATE_START:
      {
	/* Initialize counters, buffers, etc. */
	name_pos = 0;
	val_pos = 0;
	col_num = 1;
	/* Truncate.  Not strictly necessary with static buffers. */
	/* 	name = '\0'; */
	/* 	val = '\0'; */
	
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  {
	    /* First character of the name. */
	    name[name_pos] = c;
	    name_pos++;
	    state = _STASH_RES_STATE_NAME;
	    break;
	  }
	  case _STASH_RES_CHAR_HASH:
	  {
	    /* Beginning of comment.  Throw the character away. */
	    state = _STASH_RES_STATE_BEGIN_COMMENT;
	    break;
	  }
	  case _STASH_RES_CHAR_WHITESPACE:
	  {
	    /* Leading whitespace.  Swallow it. */
	    break;
	  }
	  case _STASH_RES_CHAR_NEWLINE:
	  {
	    /* Leading whitespace.  Swallow it. */
	    line_num++;
	    col_num = 1;
	    break;
	  }
	  case _STASH_RES_CHAR_NULL:
	  {
	    /* Completely empty string or file. */
	    state = _STASH_RES_STATE_FINISH;
	    break;
	  }
	  case _STASH_RES_CHAR_COLON:
	  case _STASH_RES_CHAR_BACKSLASH:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_START, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Whitespace that precedes the resource name. */
      case _STASH_RES_STATE_BEGIN_WHITESPACE:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  {
	    /* First character of the name. */
	    name[name_pos] = c;
	    name_pos++;
	    state = _STASH_RES_STATE_NAME;
	    break;
	  }
	  case _STASH_RES_CHAR_HASH:
	  {
	    /* Beginning of a comment. */
	    state = _STASH_RES_STATE_BEGIN_COMMENT;
	    break;
	  }
	  case _STASH_RES_CHAR_WHITESPACE:
	  {
	    /* More whitespace.  Swallow it. */
	    break;
	  }
	  case _STASH_RES_CHAR_NEWLINE:
	  {
	    /* Blank line.  Jump back to the start state to make sure all
	     * counters are correctly reset. */
	    line_num++;
	    col_num = 1;
	    state = _STASH_RES_STATE_START;
	    break;
	  }
	  case _STASH_RES_CHAR_NULL:
	  {
	    /* String is legal, but contains no resource. */
	    state = _STASH_RES_STATE_START;
	    break;
	  }
	  case _STASH_RES_CHAR_COLON:
	  case _STASH_RES_CHAR_BACKSLASH:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_BEGIN_WHITESPACE, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Comment that was preceded only by whitespace characters. */
      case _STASH_RES_STATE_BEGIN_COMMENT:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  case _STASH_RES_CHAR_HASH:
	  case _STASH_RES_CHAR_WHITESPACE:
	  case _STASH_RES_CHAR_COLON:
	  case _STASH_RES_CHAR_BACKSLASH:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _STASH_RES_CHAR_NEWLINE:
	  {
	    /* Go back to the initial state. */
	    line_num++;
	    col_num = 1;
	    state = _STASH_RES_STATE_START;
	    break;
	  }
	  case _STASH_RES_CHAR_NULL:
	  {
	    /* String is legal, but contains no resource. */
	    state = _STASH_RES_STATE_FINISH;
	    break;
	  }
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_BEGIN_COMMENT, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Resource name state. */
      case _STASH_RES_STATE_NAME:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  {
	    /* Character of the resource name. */
	    name[name_pos] = c;
	    name_pos++;
	    break;
	  }
	  case _STASH_RES_CHAR_WHITESPACE:
	  {
	    /* End of name.  Jump to a state that can deal with additional
	     * whitespace. */
	    name[name_pos] = '\0';
	    state = _STASH_RES_STATE_POST_NAME_WHITESPACE;
	    break;
	  }
	  case _STASH_RES_CHAR_COLON:
	  {
	    /* Okay, here's the colon.  Terminate the name and jump to a
	     * state that can deal with whitespace leading the value. */
	    name[name_pos] = '\0';
	    state = _STASH_RES_STATE_POST_COLON_WHITESPACE;
	    break;
	  }
	  case _STASH_RES_CHAR_HASH:
	  case _STASH_RES_CHAR_BACKSLASH:
	  case _STASH_RES_CHAR_NEWLINE:
	  case _STASH_RES_CHAR_NULL:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_NAME, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Swallow whitespace following the resource name. */
      case _STASH_RES_STATE_POST_NAME_WHITESPACE:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_COLON:
	  {
	    /* Here's the colon.  Jump to a state that can deal with
	     * whitespace leading the value. */
	    state = _STASH_RES_STATE_POST_COLON_WHITESPACE;
	    break;
	  }
	  case _STASH_RES_CHAR_WHITESPACE:
	  {
	    /* Additional whitespace.  Swallow it. */
	    break;
	  }
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  case _STASH_RES_CHAR_HASH:
	  case _STASH_RES_CHAR_BACKSLASH:
	  case _STASH_RES_CHAR_NEWLINE:
	  case _STASH_RES_CHAR_NULL:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_POST_NAME_WHITESPACE, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Swallow whitespace following the colon. */
      case _STASH_RES_STATE_POST_COLON_WHITESPACE:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  case _STASH_RES_CHAR_COLON:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Beginning of value. */
	    val[val_pos] = c;
	    val_pos++;
	    state = _STASH_RES_STATE_VALUE;
	    break;
	  }
	  case _STASH_RES_CHAR_WHITESPACE:
	  {
	    /* More whitespace.  Swallow it. */
	    break;
	  }
	  case _STASH_RES_CHAR_HASH:
	  {
	    /* Empty value.  NULL-terminate the string and jump to the
	     * trailing comment state. */
	    val[val_pos] = '\0';
	    res_p_merge_res(a_res_o, name, val);
	    state = _STASH_RES_STATE_TRAILING_COMMENT;
	    break;
	  }
	  case _STASH_RES_CHAR_BACKSLASH:
	  {
	    /* Beginning of value, but it's a backslash, so jump to the
	     * backslash handling state. */
	    state = _STASH_RES_STATE_VALUE_BACKSLASH;
	    break;
	  }
	  case _STASH_RES_CHAR_NEWLINE:
	  {
	    /* Empty value.  Insert it though. */
	    line_num++;
	    col_num = 1;
	    val[val_pos] = '\0';
	    res_p_merge_res(a_res_o, name, val);
	    state = _STASH_RES_STATE_START;
	    break;
	  }
	  case _STASH_RES_CHAR_NULL:
	  {
	    /* Empty value, and end of input.  Insert the resource. */
	    val[val_pos] = '\0';
	    res_p_merge_res(a_res_o, name, val);
	    state = _STASH_RES_STATE_FINISH;
	    break;
	  }
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_POST_COLON_WHITESPACE, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Resource value. */
      case _STASH_RES_STATE_VALUE:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  case _STASH_RES_CHAR_COLON:
	  case _STASH_RES_CHAR_WHITESPACE: /* Allow whitespace in value. */
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  {
	    /* More of the value. */
	    val[val_pos] = c;
	    val_pos++;
	    state = _STASH_RES_STATE_VALUE;
	    break;
	  }
	  case _STASH_RES_CHAR_HASH:
	  {
	    /* Beginning of comment, and therefore the end of the value.
	     * NULL-terminate the string, insert the resource, and jump to
	     * the trailing comment state. */
	    val[val_pos] = '\0';
	    res_p_merge_res(a_res_o, name, val);
	    state = _STASH_RES_STATE_TRAILING_COMMENT;
	    break;
	  }
	  case _STASH_RES_CHAR_BACKSLASH:
	  {
	    /* Backslash.  Jump to the backslash state. */
	    state = _STASH_RES_STATE_VALUE_BACKSLASH;
	    break;
	  }
	  case _STASH_RES_CHAR_NEWLINE:
	  {
	    /* End of line, and therefore the end of the value.
	     * NULL-terminate the string, insert the resource in the hash
	     * table, and jump back to the starting state. */
	    line_num++;
	    col_num = 1;
	    val[val_pos] = '\0';
	    res_p_merge_res(a_res_o, name, val);
	    state = _STASH_RES_STATE_START;
	    break;
	  }
	  case _STASH_RES_CHAR_NULL:
	  {
	    /* Do the same thing as for a newline, except that we want the
	     * state machine to exit. */
	    val[val_pos] = '\0';
	    res_p_merge_res(a_res_o, name, val);
	    state = _STASH_RES_STATE_FINISH;
	    break;
	  }
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_VALUE, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Backslash within the resource value. */
      case _STASH_RES_STATE_VALUE_BACKSLASH:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_BACKSLASH:
	  {
	    /* Insert a backslash and jump back to the value state. */
	    val[val_pos] = '\\';
	    val_pos++;
	    state = _STASH_RES_STATE_VALUE;
	    break;
	  }
	  case _STASH_RES_CHAR_HASH:
	  {
	    /* Insert a hash and jump back to the value state. */
	    val[val_pos] = '#';
	    val_pos++;
	    state = _STASH_RES_STATE_VALUE;
	    break;
	  }
	  case _STASH_RES_CHAR_WHITESPACE:
	  {
	    /* We need to make sure that what follows is whitespace
	     * followed by a newline, but we can't do that if we stay in
	     * this state, since we woudn't notice '\\' and '#' later on in
	     * the stream. */
	    state = _STASH_RES_STATE_BACKSLASH_WHITESPACE;
	    break;
	  }
	  case _STASH_RES_CHAR_NEWLINE:
	  {
	    /* \ continuation.  Swallow this and jump back to the value
	     * state. */
	    line_num++;
	    col_num = 1;
	    state = _STASH_RES_STATE_VALUE;
	    break;
	  }
	  case _STASH_RES_CHAR_LOWER:
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
	      state = _STASH_RES_STATE_VALUE;
	      break;
	    }
	    /* Note that if it's not an 'n', we fall through to the error
	     * case. */
	  }
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  case _STASH_RES_CHAR_COLON:
	  case _STASH_RES_CHAR_NULL:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_VALUE_BACKSLASH, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      case _STASH_RES_STATE_BACKSLASH_WHITESPACE:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_WHITESPACE:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _STASH_RES_CHAR_NEWLINE:
	  {
	    /* \ continuation.  Swallow this and jump back to the value
	     * state. */
	    line_num++;
	    col_num = 1;
	    state = _STASH_RES_STATE_VALUE;
	    break;
	  }
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  case _STASH_RES_CHAR_HASH:
	  case _STASH_RES_CHAR_COLON:
	  case _STASH_RES_CHAR_BACKSLASH:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  case _STASH_RES_CHAR_NULL:
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_BACKSLASH_WHITESPACE, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Comment at end of resource. */
      case _STASH_RES_STATE_TRAILING_COMMENT:
      {
	switch (res_p_char_type(c))
	{
	  case _STASH_RES_CHAR_CAP:
	  case _STASH_RES_CHAR_LOWER:
	  case _STASH_RES_CHAR_NUMBER:
	  case _STASH_RES_CHAR_UNDER:
	  case _STASH_RES_CHAR_PERIOD:
	  case _STASH_RES_CHAR_HASH:
	  case _STASH_RES_CHAR_WHITESPACE:
	  case _STASH_RES_CHAR_COLON:
	  case _STASH_RES_CHAR_BACKSLASH:
	  case _STASH_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _STASH_RES_CHAR_NEWLINE:
	  {
	    /* Okay, end of comment.  Jump back to the starting state. */
	    line_num++;
	    col_num = 1;
	    state = _STASH_RES_STATE_START;
	    break;
	  }
	  case _STASH_RES_CHAR_NULL:
	  {
	    /* End of input.  Finish. */
	    state = _STASH_RES_STATE_FINISH;
	    break;
	  }
	  case _STASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(g_log_o, NULL, 0, "res_parse_res",
			"Illegal character while in _STASH_RES_STATE_TRAILING_COMMENT, line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      default:
      {
	log_eprintf(g_log_o, NULL, 0, "res_parse_res",
		    "Jumped to non-existant state, line %d, column %d\n",
		    line_num, col_num);
	retval = TRUE;
	break;
      }
    }
  }

  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_p_parse_res()");
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
res_p_char_type(char a_char)
{
  cw_uint32_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_p_char_type()");
  }
  switch (a_char)
  {
    /* Capital letters. */
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    {
      retval = _STASH_RES_CHAR_CAP;
      break;
    }
    
    /* Lower case letters. */
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
    {
      retval = _STASH_RES_CHAR_LOWER;
      break;
    }

    /* Numbers. */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
      retval = _STASH_RES_CHAR_NUMBER;
      break;
    }
    
    /* Underscore. */
    case '_':
    {
      retval = _STASH_RES_CHAR_UNDER;
      break;
    }
    
    /* Period. */
    case '.':
    {
      retval = _STASH_RES_CHAR_PERIOD;
      break;
    }
    
    /* Start comment. */
    case '#':
    {
      retval = _STASH_RES_CHAR_HASH;
      break;
    }
    
    /* Whitespace. */
    case '\t': case ' ':
    {
      retval = _STASH_RES_CHAR_WHITESPACE;
      break;
    }
    
    /* Colon. */
    case ':':
    {
      retval = _STASH_RES_CHAR_COLON;
      break;
    }
    
    /* Backslash. */
    case '\\':
    {
      retval = _STASH_RES_CHAR_BACKSLASH;
      break;
    }

    /* Carriage return. */
    case '\n':
    {
      retval = _STASH_RES_CHAR_NEWLINE;
      break;
    }
    
    /* Null terminator. */
    case '\0':
    {
      retval = _STASH_RES_CHAR_NULL;
      break;
    }

    /* Other valid characters within a resource values. */
    case '!': case '"': case '$': case '%': case '&': case '\'': case '(':
    case ')': case '*': case '+': case ',': case '-': case '/': case ';':
    case '<': case '=': case '>': case '?': case '@': case '[': case ']':
    case '^': case '`': case '{': case '|': case '}': case '~':
    {
      retval = _STASH_RES_CHAR_VALID_IN_VAL;
      break;
    }
    
    /* Something we're not expecting at all. */
    default:
    {
      retval = _STASH_RES_CHAR_OTHER;
      break;
    }
  }

  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_p_char_type()");
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
res_p_merge_res(cw_res_t * a_res_o, char * a_name, char * a_val)
{
  char * temp_name, * temp_val;
  cw_bool_t error;
	    
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Enter res_p_merge_res()");
  }
  /* Make copies to insert into the hash table. */
  temp_name = (char *) _cw_malloc(strlen(a_name) + 1);
  strcpy(temp_name, a_name);
  temp_val = (char *) _cw_malloc(strlen(a_val) + 1);
  strcpy(temp_val, a_val);

  if (_cw_pmatch(_STASH_DBG_R_RES_STATE))
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
  if (_cw_pmatch(_STASH_DBG_R_RES_FUNC))
  {
    _cw_marker("Exit res_p_merge_res()");
  }
}
