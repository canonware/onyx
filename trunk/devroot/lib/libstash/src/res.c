/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 *****************************************************************************/

#define _LIBSTASH_USE_RES
#define _LIBSTASH_USE_OH
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include <stdarg.h>

#include "libstash/res_p.h"

/* Initcial size of buffer to use for name/value parsing. */
#define _LIBSTASH_RES_BUFFSIZE 8192

/* Character types for state machine. */
#define _LIBSTASH_RES_CHAR_CAP 0
#define _LIBSTASH_RES_CHAR_LOWER 1
#define _LIBSTASH_RES_CHAR_NUMBER 2
#define _LIBSTASH_RES_CHAR_UNDER 3
#define _LIBSTASH_RES_CHAR_PERIOD 4
#define _LIBSTASH_RES_CHAR_HASH 5
#define _LIBSTASH_RES_CHAR_WHITESPACE 6
#define _LIBSTASH_RES_CHAR_COLON 7
#define _LIBSTASH_RES_CHAR_BACKSLASH 8
#define _LIBSTASH_RES_CHAR_NEWLINE 9
#define _LIBSTASH_RES_CHAR_NULL 10
#define _LIBSTASH_RES_CHAR_VALID_IN_VAL 11
#define _LIBSTASH_RES_CHAR_OTHER 12

/* State machine states. */
#define _LIBSTASH_RES_STATE_START 0
#define _LIBSTASH_RES_STATE_BEGIN_WHITESPACE 1
#define _LIBSTASH_RES_STATE_BEGIN_COMMENT 2
#define _LIBSTASH_RES_STATE_NAME 3
#define _LIBSTASH_RES_STATE_POST_NAME_WHITESPACE 4
#define _LIBSTASH_RES_STATE_POST_COLON_WHITESPACE 5
#define _LIBSTASH_RES_STATE_VALUE 6
#define _LIBSTASH_RES_STATE_VALUE_BACKSLASH 7
#define _LIBSTASH_RES_STATE_BACKSLASH_WHITESPACE 8
#define _LIBSTASH_RES_STATE_TRAILING_COMMENT 9
#define _LIBSTASH_RES_STATE_FINISH 10

cw_res_t *
res_new(cw_res_t * a_res)
{
  cw_res_t * retval;

  if (a_res == NULL)
  {
    retval = (cw_res_t *) _cw_malloc(sizeof(cw_res_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_res;
    retval->is_malloced = FALSE;
  }

  /* Initialize internals. */
#ifdef _CW_REENTRANT
  rwl_new(&retval->rw_lock);
#endif
  /* Non-thread-safe hash table, since we're already taking care of the
   * locking. */
#ifdef _CW_REENTRANT
  oh_new(&retval->hash, FALSE);
#else
  oh_new(&retval->hash);
#endif

  RETURN:
  return retval;
}

void
res_delete(cw_res_t * a_res)
{
  cw_uint64_t i, num_resources;
  void * name, * val;
  
  _cw_check_ptr(a_res);

  /* Clean up internals. */
#ifdef _CW_REENTRANT
  rwl_delete(&a_res->rw_lock);
#endif

  for (i = 0, num_resources = oh_get_num_items(&a_res->hash);
       i < num_resources;
       i++)
  {
    oh_item_delete_iterate(&a_res->hash, &name, &val);
    _cw_free(name);
    _cw_free(val);
  }
  oh_delete(&a_res->hash);
  
  if (a_res->is_malloced)
  {
    _cw_free(a_res);
  }
}

void
res_clear(cw_res_t * a_res)
{
  char * key, * val;
  
  _cw_check_ptr(a_res);
#ifdef _CW_REENTRANT
  rwl_wlock(&a_res->rw_lock);
#endif

  while (FALSE == oh_item_delete_iterate(&a_res->hash, (void **) &key,
					 (void **) &val))
  {
    _cw_free(key);
    _cw_free(val);
  }
  
#ifdef _CW_REENTRANT
  rwl_wunlock(&a_res->rw_lock);
#endif
}

cw_bool_t
res_is_equal(cw_res_t * a_res, cw_res_t * a_other)
{
  cw_bool_t retval;

  _cw_check_ptr(a_res);
  _cw_check_ptr(a_other);
#ifdef _CW_REENTRANT
  rwl_wlock(&a_res->rw_lock);
  rwl_rlock(&a_other->rw_lock);
#endif

  if (a_res == a_other)
  {
    /* Two pointers to the same instance. */
    retval = TRUE;
  }
  else if (oh_get_num_items(&a_res->hash)
	   != oh_get_num_items(&a_other->hash))
  {
    retval = FALSE;
  }
  else
  {
    cw_uint32_t i, num_resources;
    char * key, * val;

    num_resources = oh_get_num_items(&a_res->hash);
    
    for (i = 0, retval = FALSE; (i < num_resources) && (retval == FALSE); i++)
    {
      oh_item_delete_iterate(&a_res->hash, (void **) &key,
			     (void **) &val);

      if (NULL == res_get_res_val(a_other, key))
      {
	retval = TRUE;
      }

      if (0 != oh_item_insert(&a_res->hash, key, val))
      {
	retval = TRUE;
	goto RETURN;
      }
    }
  }

  RETURN:
#ifdef _CW_REENTRANT
  rwl_runlock(&a_other->rw_lock);
  rwl_wunlock(&a_res->rw_lock);
#endif
  return retval;
}

cw_bool_t
res_merge_file(cw_res_t * a_res, const char * a_filename)
{
  cw_bool_t retval = FALSE, state_mach_error;
  int error;
  
  _cw_check_ptr(a_res);
#ifdef _CW_REENTRANT
  rwl_wlock(&a_res->rw_lock);
#endif

  a_res->fd = fopen(a_filename, "r");
  if (a_res->fd == NULL)
  {
    retval = TRUE;
  }
  else
  {
    /* Run the state machine on the file. */
    state_mach_error = res_p_parse_res(a_res, TRUE);
    if (state_mach_error == TRUE)
    {
      retval = TRUE;
    }

    /* Close the file. */
    error = fclose(a_res->fd);
    if (error)
    {
      retval = TRUE;
    }
  }

#ifdef _CW_REENTRANT
  rwl_wunlock(&a_res->rw_lock);
#endif
  return retval;
}

cw_bool_t
res_merge_list(cw_res_t * a_res, ...)
{
  va_list ap;
  cw_bool_t retval = FALSE, state_mach_error;
  
  _cw_check_ptr(a_res);
#ifdef _CW_REENTRANT
  rwl_wlock(&a_res->rw_lock);
#endif

  /* Run the strings through the insertion state machine. */
  va_start(ap, a_res);
  for (a_res->str = va_arg(ap, char *);
       ((a_res->str != NULL) && (retval != TRUE));
       a_res->str = va_arg(ap, char *))
  {
    state_mach_error = res_p_parse_res(a_res, FALSE);
    if (state_mach_error == TRUE)
    {
      retval = TRUE;
    }
  }
  va_end(ap);
  
#ifdef _CW_REENTRANT
  rwl_wunlock(&a_res->rw_lock);
#endif
  return retval;
}

const char *
res_get_res_val(cw_res_t * a_res, const char * a_res_name)
{
  char * retval;
  cw_bool_t error;
  
  _cw_check_ptr(a_res);
  _cw_check_ptr(a_res_name);
#ifdef _CW_REENTRANT
  rwl_rlock(&a_res->rw_lock);
#endif

  error = oh_item_search(&a_res->hash, (void *) a_res_name,
			 (void **) &retval);
  if (error == TRUE)
  {
    retval = NULL;
  }
  
#ifdef _CW_REENTRANT
  rwl_runlock(&a_res->rw_lock);
#endif
  return retval;
}

cw_bool_t
res_extract_res(cw_res_t * a_res, char * a_res_key,
		char ** r_res_name, char ** r_res_val)
{
  cw_bool_t retval;

  _cw_check_ptr(a_res);
#ifdef _CW_REENTRANT
  rwl_wlock(&a_res->rw_lock);
#endif

  retval = oh_item_delete(&a_res->hash, a_res_key,
			  (void **) r_res_name, (void **) r_res_val);

#ifdef _CW_REENTRANT
  rwl_wunlock(&a_res->rw_lock);
#endif
  return retval;
}

cw_bool_t
res_dump(cw_res_t * a_res, char * a_filename)
{
  cw_bool_t retval;
  cw_log_t * t_log;
  
  _cw_check_ptr(a_res);
#ifdef _CW_REENTRANT
  rwl_wlock(&a_res->rw_lock);
#endif

  if (a_filename != NULL)
  {
    t_log = log_new();
    if (log_set_logfile(t_log, a_filename, TRUE) == TRUE)
    {
      log_leprintf(cw_g_log, NULL, 0, __FUNCTION__,
		   "Error opening file \"%s\"\n", a_filename);
      retval = TRUE;
      goto RETURN;
    }
  }
  else
  {
    t_log = cw_g_log;
  }

  retval = FALSE;

  /* Now dump the resources to t_log. */
  {
    cw_uint64_t num_items, i;
    char * key, * val;
    cw_uint32_t j, curr_offset, val_len;
    
    num_items = oh_get_num_items(&a_res->hash);

    for (i = 0; i < num_items; i++)
    {
      oh_item_get_iterate(&a_res->hash, (void *) &key, (void *) &val);

      log_printf(t_log, "%s:", key);
      
      for (j = 0, curr_offset = 0, val_len = strlen(val);
	   j < val_len + 1;
	   j++)
      {
	if (val[j] == '\n')
	{
	  val[j] = '\0';
	  if ((j < val_len) && (val[j + 1] != '\0'))
	  {
	    log_printf(t_log, "%s\\n\\\n", (char *) (val + curr_offset));
	  }
	  else
	  {
	    log_printf(t_log, "%s\\n", (char *) (val + curr_offset));
	  }
	  val[j] = '\n';
	  curr_offset = j + 1;
	}
	else if (val[j] == '\0')
	{
	  log_printf(t_log, "%s\n", (char *) (val + curr_offset));
	}
      }
    }
  }

 RETURN:  
  if (a_filename != NULL)
  {
    log_delete(t_log);
  }
#ifdef _CW_REENTRANT
  rwl_wunlock(&a_res->rw_lock);
#endif
  return retval;
}

static cw_bool_t
res_p_parse_res(cw_res_t * a_res, cw_bool_t a_is_file)
{
  cw_bool_t retval = FALSE;
  size_t i, name_pos = 0, val_pos = 0;
  cw_uint32_t state = _LIBSTASH_RES_STATE_START, col_num, line_num = 1;
  char c, * name = NULL, * val = NULL;
  cw_uint32_t name_bufsize, val_bufsize;

  name_bufsize = _LIBSTASH_RES_BUFFSIZE;
  val_bufsize = _LIBSTASH_RES_BUFFSIZE;

  name = (char *) _cw_malloc(name_bufsize);
  if (NULL == name)
  {
    goto RETURN;
  }
  
  val = (char *) _cw_malloc(val_bufsize);
  if (NULL == val)
  {
    goto RETURN;
  }

  for (i = 0, col_num = 1;
       ((state != _LIBSTASH_RES_STATE_FINISH) && (retval != TRUE));
       i++, col_num++)
  {
    /* Check whether we overflowed the buffers, and expand them, if
     * necessary. */
    if (name_pos >= name_bufsize)
    {
      name_bufsize <<= 1;
      name = (char *) _cw_realloc(name, name_bufsize);
      if (NULL == name)
      {
	goto RETURN;
      }
    }
    if (val_pos >= val_bufsize)
    {
      val_bufsize <<= 1;
      val = (char *) _cw_realloc(val, val_bufsize);
      if (NULL == val)
      {
	goto RETURN;
      }
    }
    
    /* Read the next character in. */
    if (a_is_file)
    {
      c = (char) getc(a_res->fd);
      if (c == EOF)
      {
	/* Make sure it's an EOF, not an error. */
	if (ferror(a_res->fd))
	{
	  log_printf(cw_g_log, "res_parse_res(): Error reading from file\n");
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
      c = a_res->str[i];
    }
    
    if (dbg_is_registered(cw_g_dbg, "res_state"))
    {
      log_printf(cw_g_log, "res_parse_res(): State == %d, Input == \'%c\'\n",
		 state, c);
    }
    
    switch (state)
    {
      /* Starting state. */
      case _LIBSTASH_RES_STATE_START:
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
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  {
	    /* First character of the name. */
	    name[name_pos] = c;
	    name_pos++;
	    state = _LIBSTASH_RES_STATE_NAME;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_HASH:
	  {
	    /* Beginning of comment.  Throw the character away. */
	    state = _LIBSTASH_RES_STATE_BEGIN_COMMENT;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  {
	    /* Leading whitespace.  Swallow it. */
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  {
	    /* Leading whitespace.  Swallow it. */
	    line_num++;
	    col_num = 1;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NULL:
	  {
	    /* Completely empty string or file. */
	    state = _LIBSTASH_RES_STATE_FINISH;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_COLON:
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in _LIBSTASH_RES_STATE_START,"
			" line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Whitespace that precedes the resource name. */
      case _LIBSTASH_RES_STATE_BEGIN_WHITESPACE:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  {
	    /* First character of the name. */
	    name[name_pos] = c;
	    name_pos++;
	    state = _LIBSTASH_RES_STATE_NAME;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_HASH:
	  {
	    /* Beginning of a comment. */
	    state = _LIBSTASH_RES_STATE_BEGIN_COMMENT;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  {
	    /* More whitespace.  Swallow it. */
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  {
	    /* Blank line.  Jump back to the start state to make sure all
	     * counters are correctly reset. */
	    line_num++;
	    col_num = 1;
	    state = _LIBSTASH_RES_STATE_START;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NULL:
	  {
	    /* String is legal, but contains no resource. */
	    state = _LIBSTASH_RES_STATE_START;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_COLON:
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in "
			"_LIBSTASH_RES_STATE_BEGIN_WHITESPACE,"
			" line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Comment that was preceded only by whitespace characters. */
      case _LIBSTASH_RES_STATE_BEGIN_COMMENT:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  case _LIBSTASH_RES_CHAR_HASH:
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  case _LIBSTASH_RES_CHAR_COLON:
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  {
	    /* Go back to the initial state. */
	    line_num++;
	    col_num = 1;
	    state = _LIBSTASH_RES_STATE_START;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NULL:
	  {
	    /* String is legal, but contains no resource. */
	    state = _LIBSTASH_RES_STATE_FINISH;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in "
			"_LIBSTASH_RES_STATE_BEGIN_COMMENT, "
			"line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Resource name state. */
      case _LIBSTASH_RES_STATE_NAME:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  {
	    /* Character of the resource name. */
	    name[name_pos] = c;
	    name_pos++;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  {
	    /* End of name.  Jump to a state that can deal with additional
	     * whitespace. */
	    name[name_pos] = '\0';
	    state = _LIBSTASH_RES_STATE_POST_NAME_WHITESPACE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_COLON:
	  {
	    /* Okay, here's the colon.  Terminate the name and jump to a
	     * state that can deal with whitespace leading the value. */
	    name[name_pos] = '\0';
	    state = _LIBSTASH_RES_STATE_POST_COLON_WHITESPACE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_HASH:
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  case _LIBSTASH_RES_CHAR_NULL:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in _LIBSTASH_RES_STATE_NAME, "
			"line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Swallow whitespace following the resource name. */
      case _LIBSTASH_RES_STATE_POST_NAME_WHITESPACE:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_COLON:
	  {
	    /* Here's the colon.  Jump to a state that can deal with
	     * whitespace leading the value. */
	    state = _LIBSTASH_RES_STATE_POST_COLON_WHITESPACE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  {
	    /* Additional whitespace.  Swallow it. */
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  case _LIBSTASH_RES_CHAR_HASH:
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  case _LIBSTASH_RES_CHAR_NULL:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in "
			"_LIBSTASH_RES_STATE_POST_NAME_WHITESPACE, "
			"line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Swallow whitespace following the colon. */
      case _LIBSTASH_RES_STATE_POST_COLON_WHITESPACE:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  case _LIBSTASH_RES_CHAR_COLON:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Beginning of value. */
	    val[val_pos] = c;
	    val_pos++;
	    state = _LIBSTASH_RES_STATE_VALUE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  {
	    /* More whitespace.  Swallow it. */
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_HASH:
	  {
	    /* Empty value.  NULL-terminate the string and jump to the
	     * trailing comment state. */
	    val[val_pos] = '\0';
	    if (res_p_merge_res(a_res, name, val))
	    {
	      retval = TRUE;
	      goto RETURN;
	    }
	    state = _LIBSTASH_RES_STATE_TRAILING_COMMENT;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  {
	    /* Beginning of value, but it's a backslash, so jump to the
	     * backslash handling state. */
	    state = _LIBSTASH_RES_STATE_VALUE_BACKSLASH;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  {
	    /* Empty value.  Insert it though. */
	    line_num++;
	    col_num = 1;
	    val[val_pos] = '\0';
	    if (res_p_merge_res(a_res, name, val))
	    {
	      retval = TRUE;
	      goto RETURN;
	    }
	    state = _LIBSTASH_RES_STATE_START;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NULL:
	  {
	    /* Empty value, and end of input.  Insert the resource. */
	    val[val_pos] = '\0';
	    if (res_p_merge_res(a_res, name, val))
	    {
	      retval = TRUE;
	      goto RETURN;
	    }
	    state = _LIBSTASH_RES_STATE_FINISH;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in "
			"_LIBSTASH_RES_STATE_POST_COLON_WHITESPACE, "
			"line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Resource value. */
      case _LIBSTASH_RES_STATE_VALUE:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  case _LIBSTASH_RES_CHAR_COLON:
	  case _LIBSTASH_RES_CHAR_WHITESPACE: /* Allow whitespace in value. */
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  {
	    /* More of the value. */
	    val[val_pos] = c;
	    val_pos++;
	    state = _LIBSTASH_RES_STATE_VALUE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_HASH:
	  {
	    /* Beginning of comment, and therefore the end of the value.
	     * NULL-terminate the string, insert the resource, and jump to
	     * the trailing comment state. */
	    val[val_pos] = '\0';
	    if (res_p_merge_res(a_res, name, val))
	    {
	      retval = TRUE;
	      goto RETURN;
	    }
	    state = _LIBSTASH_RES_STATE_TRAILING_COMMENT;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  {
	    /* Backslash.  Jump to the backslash state. */
	    state = _LIBSTASH_RES_STATE_VALUE_BACKSLASH;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  {
	    /* End of line, and therefore the end of the value.
	     * NULL-terminate the string, insert the resource in the hash
	     * table, and jump back to the starting state. */
	    line_num++;
	    col_num = 1;
	    val[val_pos] = '\0';
	    if (res_p_merge_res(a_res, name, val))
	    {
	      retval = TRUE;
	      goto RETURN;
	    }
	    
	    state = _LIBSTASH_RES_STATE_START;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NULL:
	  {
	    /* Do the same thing as for a newline, except that we want the
	     * state machine to exit. */
	    val[val_pos] = '\0';
	    if (res_p_merge_res(a_res, name, val))
	    {
	      retval = TRUE;
	      goto RETURN;
	    }
	    state = _LIBSTASH_RES_STATE_FINISH;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in _LIBSTASH_RES_STATE_VALUE, "
			"line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Backslash within the resource value. */
      case _LIBSTASH_RES_STATE_VALUE_BACKSLASH:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  {
	    /* Insert a backslash and jump back to the value state. */
	    val[val_pos] = '\\';
	    val_pos++;
	    state = _LIBSTASH_RES_STATE_VALUE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_HASH:
	  {
	    /* Insert a hash and jump back to the value state. */
	    val[val_pos] = '#';
	    val_pos++;
	    state = _LIBSTASH_RES_STATE_VALUE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  {
	    /* We need to make sure that what follows is whitespace
	     * followed by a newline, but we can't do that if we stay in
	     * this state, since we woudn't notice '\\' and '#' later on in
	     * the stream. */
	    state = _LIBSTASH_RES_STATE_BACKSLASH_WHITESPACE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  {
	    /* \ continuation.  Swallow this and jump back to the value
	     * state. */
	    line_num++;
	    col_num = 1;
	    state = _LIBSTASH_RES_STATE_VALUE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_LOWER:
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
	      state = _LIBSTASH_RES_STATE_VALUE;
	      break;
	    }
	    /* Note that if it's not an 'n', we fall through to the error
	     * case. */
	  }
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  case _LIBSTASH_RES_CHAR_COLON:
	  case _LIBSTASH_RES_CHAR_NULL:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in "
			"_LIBSTASH_RES_STATE_VALUE_BACKSLASH, "
			"line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      case _LIBSTASH_RES_STATE_BACKSLASH_WHITESPACE:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  {
	    /* \ continuation.  Swallow this and jump back to the value
	     * state. */
	    line_num++;
	    col_num = 1;
	    state = _LIBSTASH_RES_STATE_VALUE;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  case _LIBSTASH_RES_CHAR_HASH:
	  case _LIBSTASH_RES_CHAR_COLON:
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  case _LIBSTASH_RES_CHAR_NULL:
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in "
			"_LIBSTASH_RES_STATE_BACKSLASH_WHITESPACE, "
			"line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      /* Comment at end of resource. */
      case _LIBSTASH_RES_STATE_TRAILING_COMMENT:
      {
	switch (res_p_char_type(c))
	{
	  case _LIBSTASH_RES_CHAR_CAP:
	  case _LIBSTASH_RES_CHAR_LOWER:
	  case _LIBSTASH_RES_CHAR_NUMBER:
	  case _LIBSTASH_RES_CHAR_UNDER:
	  case _LIBSTASH_RES_CHAR_PERIOD:
	  case _LIBSTASH_RES_CHAR_HASH:
	  case _LIBSTASH_RES_CHAR_WHITESPACE:
	  case _LIBSTASH_RES_CHAR_COLON:
	  case _LIBSTASH_RES_CHAR_BACKSLASH:
	  case _LIBSTASH_RES_CHAR_VALID_IN_VAL:
	  {
	    /* Swallow the character. */
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NEWLINE:
	  {
	    /* Okay, end of comment.  Jump back to the starting state. */
	    line_num++;
	    col_num = 1;
	    state = _LIBSTASH_RES_STATE_START;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_NULL:
	  {
	    /* End of input.  Finish. */
	    state = _LIBSTASH_RES_STATE_FINISH;
	    break;
	  }
	  case _LIBSTASH_RES_CHAR_OTHER:
	  default:
	  {
	    /* Error. */
	    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
			"Illegal character while in "
			"_LIBSTASH_RES_STATE_TRAILING_COMMENT, "
			"line %d, column %d\n",
			line_num, col_num);
	    retval = TRUE;
	    break;
	  }
	}
	break;
      }
      default:
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "Jumped to non-existant state, line %d, column %d\n",
		    line_num, col_num);
	retval = TRUE;
	break;
      }
    }
  }

  RETURN:
  if (NULL != name)
  {
    _cw_free(name);
  }
  if (NULL != val)
  {
    _cw_free(val);
  }
  return retval;
}

static cw_uint32_t
res_p_char_type(char a_char)
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
      retval = _LIBSTASH_RES_CHAR_CAP;
      break;
    }
    
    /* Lower case letters. */
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
    {
      retval = _LIBSTASH_RES_CHAR_LOWER;
      break;
    }

    /* Numbers. */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
      retval = _LIBSTASH_RES_CHAR_NUMBER;
      break;
    }
    
    /* Underscore. */
    case '_':
    {
      retval = _LIBSTASH_RES_CHAR_UNDER;
      break;
    }
    
    /* Period. */
    case '.':
    {
      retval = _LIBSTASH_RES_CHAR_PERIOD;
      break;
    }
    
    /* Start comment. */
    case '#':
    {
      retval = _LIBSTASH_RES_CHAR_HASH;
      break;
    }
    
    /* Whitespace. */
    case '\t': case ' ':
    {
      retval = _LIBSTASH_RES_CHAR_WHITESPACE;
      break;
    }
    
    /* Colon. */
    case ':':
    {
      retval = _LIBSTASH_RES_CHAR_COLON;
      break;
    }
    
    /* Backslash. */
    case '\\':
    {
      retval = _LIBSTASH_RES_CHAR_BACKSLASH;
      break;
    }

    /* Carriage return. */
    case '\n':
    {
      retval = _LIBSTASH_RES_CHAR_NEWLINE;
      break;
    }
    
    /* Null terminator. */
    case '\0':
    {
      retval = _LIBSTASH_RES_CHAR_NULL;
      break;
    }

    /* Other valid characters within a resource values. */
    case '!': case '"': case '$': case '%': case '&': case '\'': case '(':
    case ')': case '*': case '+': case ',': case '-': case '/': case ';':
    case '<': case '=': case '>': case '?': case '@': case '[': case ']':
    case '^': case '`': case '{': case '|': case '}': case '~':
    {
      retval = _LIBSTASH_RES_CHAR_VALID_IN_VAL;
      break;
    }
    
    /* Something we're not expecting at all. */
    default:
    {
      retval = _LIBSTASH_RES_CHAR_OTHER;
      break;
    }
  }

  return retval;
}

static cw_bool_t
res_p_merge_res(cw_res_t * a_res, const char * a_name, const char * a_val)
{
  cw_bool_t retval;
  char * temp_name, * temp_val;
  cw_sint32_t error;
	    
  /* Make copies to insert into the hash table. */
  temp_name = (char *) _cw_malloc(strlen(a_name) + 1);
  if (NULL == temp_name)
  {
    retval = TRUE;
    goto RETURN;
  }
  strcpy(temp_name, a_name);
  
  temp_val = (char *) _cw_malloc(strlen(a_val) + 1);
  if (NULL == temp_val)
  {
    _cw_free(temp_name);
    retval = TRUE;
    goto RETURN;
  }
  strcpy(temp_val, a_val);

  if (dbg_is_registered(cw_g_dbg, "res_state"))
  {
    log_printf(cw_g_log,
	       "res_merge_res(): Merging name == :%s:, value == :%s:\n",
	       a_name, a_val);
  }

  /* Insert the resource into the hash table. */
  error = oh_item_insert(&a_res->hash, (void *) temp_name,
			 (void *) temp_val);
  if (error == 1)
  {
    char * old_name, * old_val;
	      
    /* The resource already exists.  That means we need to delete the
     * existing one, free the resources that are taken up by it, and redo
     * the insertion. */
    oh_item_delete(&a_res->hash, (void *) temp_name,
		   (void **) &old_name,
		   (void **) &old_val);
    _cw_free(old_name);
    _cw_free(old_val);

    error = oh_item_insert(&a_res->hash, (void *) temp_name,
			   (void *) temp_val);
  }

  if (error == -1)
  {
    _cw_free(temp_name);
    _cw_free(temp_val);
  }

  retval = FALSE;

  RETURN:
  return retval;
}
