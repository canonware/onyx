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

#include "../include/libkasi/libkasi.h"

/* If defined, inline oft-used functions for improved performance (and slightly
 * increased code size). */
#define _CW_KASIT_INLINE

#ifdef _CW_KASIT_INLINE
#  define _CW_KASIT_GETC(a_i)                                               \
  ((_CW_KASI_BUFC_SIZE >= a_kasit->index)                                   \
   ? a_kasit->tok_buffer.str[(a_i)]                                         \
   : buf_get_uint8(&a_kasit->tok_buffer.buf, (a_i)))
#else
#  define _CW_KASIT_GETC(a_i)                                               \
  kasit_p_getc(a_kasit, a_i)
#endif

#ifdef _CW_KASIT_INLINE
#  define _CW_KASIT_PUTC(a_c)                                               \
  do                                                                        \
  {                                                                         \
    if (_CW_KASI_BUFC_SIZE > a_kasit->index)                                \
    {                                                                       \
      a_kasit->tok_buffer.str[a_kasit->index] = (a_c);                      \
      a_kasit->index++;                                                     \
    }                                                                       \
    else                                                                    \
    {                                                                       \
      if (-1 == kasit_p_putc(a_kasit, a_c))                                 \
      {                                                                     \
        retval = -1;                                                        \
        goto RETURN;                                                        \
      }                                                                     \
    }                                                                       \
  } while (0)
#else
#  define _CW_KASIT_PUTC(a_c)                                               \
  do                                                                        \
  {                                                                         \
    if (-1 == kasit_p_putc(a_kasit, a_c))                                   \
    {                                                                       \
      retval = -1;                                                          \
      goto RETURN;                                                          \
    }                                                                       \
  } while (0)
#endif

struct cw_kasit_entry_s
{
  cw_thd_t thd;
  cw_kasit_t * kasit;
  cw_buf_t buf;
};

static cw_sint32_t
kasit_p_feed(cw_kasit_t * a_kasit, const char * a_str, cw_uint32_t a_len);
static void
kasit_p_reset_tok_buffer(cw_kasit_t * a_kasit);
#ifndef _CW_KASIT_INLINE
static cw_uint8_t
kasit_p_getc(cw_kasit_t * a_kasit, cw_uint32_t a_index);
#endif
static cw_sint32_t
kasit_p_putc(cw_kasit_t * a_kasit, cw_uint32_t a_c);
static void
kasit_p_print_token(cw_kasit_t * a_kasit, cw_uint32_t a_length,
		    const char * a_note);
static void
kasit_p_print_syntax_error(cw_kasit_t * a_kasit, cw_uint8_t a_c);
static void *
kasit_p_entry(void * a_arg);

#ifdef _LIBKASI_DBG
#  define _CW_KASIT_MAGIC 0x978fdbe0
#endif

/* Size and fullness control of initial name cache hash table.  This starts out
 * empty, but should be expected to grow rather quickly to start with. */
#define _CW_KASIT_KASIN_BASE_TABLE  256
#define _CW_KASIT_KASIN_BASE_GROW   200
#define _CW_KASIT_KASIN_BASE_SHRINK  64

/* Size and fullness control of initial root set for local VM.  Local VM is
 * empty to begin with. */
#define _CW_KASIT_ROOTS_BASE_TABLE  32
#define _CW_KASIT_ROOTS_BASE_GROW   24
#define _CW_KASIT_ROOTS_BASE_SHRINK  8

cw_kasit_t *
kasit_new(cw_kasit_t * a_kasit,
	  cw_kasi_t * a_kasi)
{
  cw_kasit_t * retval;
  
  if (NULL != a_kasit)
  {
    retval = a_kasit;
    bzero(a_kasit, sizeof(cw_kasit_t));
    retval->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_kasit_t *) _cw_malloc(sizeof(cw_kasit_t));
    if (NULL == retval)
    {
      goto OOM_1;
    }
    retval->is_malloced = TRUE;
  }
  
  if (NULL == dch_new(&retval->kasin_dch, _CW_KASIT_KASIN_BASE_TABLE,
		      _CW_KASIT_KASIN_BASE_GROW, _CW_KASIT_KASIN_BASE_SHRINK,
		      kasit_get_chi_pezz(a_kasi),
		      ch_hash_direct, ch_key_comp_direct))
  {
    goto OOM_2;
  }

  if (NULL == dch_new(&retval->roots_dch, _CW_KASIT_ROOTS_BASE_TABLE,
		      _CW_KASIT_ROOTS_BASE_GROW, _CW_KASIT_ROOTS_BASE_SHRINK,
		      kasit_get_chi_pezz(a_kasi),
		      ch_hash_direct, ch_key_comp_direct))
  {
    goto OOM_3;
  }

  retval->kasi = a_kasi;
  retval->state = _CW_KASIT_STATE_START;
#ifdef _LIBKASI_DBG
  retval->magic = _CW_KASIT_MAGIC;
#endif
  
  return retval;

  OOM_3:
  dch_delete(&retval->kasin_dch);
  OOM_2:
  if (TRUE == retval->is_malloced)
  {
    _cw_free(retval);
  }
  OOM_1:
  return NULL;
}

void
kasit_delete(cw_kasit_t * a_kasit)
{
  _cw_check_ptr(a_kasit);
  _cw_assert(_CW_KASIT_MAGIC == a_kasit->magic);
  
  dch_delete(&a_kasit->roots_dch);
  dch_delete(&a_kasit->kasin_dch);
  if (TRUE == a_kasit->is_malloced)
  {
    _cw_free(a_kasit);
  }
}

cw_bool_t
kasit_interp_str(cw_kasit_t * a_kasit, const char * a_str, cw_uint32_t a_len)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_kasit);
  _cw_assert(_CW_KASIT_MAGIC == a_kasit->magic);

  retval = kasit_p_feed(a_kasit, a_str, a_len);
  
  return retval;
}

cw_bool_t
kasit_interp_buf(cw_kasit_t * a_kasit, cw_buf_t * a_buf)
{
  cw_bool_t retval;
  int iov_cnt, i;
  const struct iovec * iov;

  _cw_check_ptr(a_kasit);
  _cw_assert(_CW_KASIT_MAGIC == a_kasit->magic);
  _cw_check_ptr(a_buf);

  iov = buf_get_iovec(a_buf, UINT_MAX, FALSE, &iov_cnt);

  for (i = 0; i < iov_cnt; i++)
  {
    if (TRUE == kasit_p_feed(a_kasit, iov[i].iov_base,
			     (cw_uint32_t) iov[i].iov_len))
    {
      retval = TRUE;
      goto RETURN;
    }
  }

  retval = FALSE;

  RETURN:
  return retval;
}

cw_bool_t
kasit_detach_str(cw_kasit_t * a_kasit, const char * a_str, cw_uint32_t a_len)
{
  cw_bool_t retval;
  cw_buf_t buf;
  
  _cw_check_ptr(a_kasit);
  _cw_assert(_CW_KASIT_MAGIC == a_kasit->magic);
  _cw_check_ptr(a_str);

  buf_new(&buf);
  if (TRUE == buf_set_range(&buf, 0, a_len, (cw_uint8_t *) a_str, FALSE))
  {
    retval = TRUE;
    goto RETURN;
  }

  retval = kasit_detach_buf(a_kasit, &buf);

  RETURN:
  buf_delete(&buf);
  return retval;
}

cw_bool_t
kasit_detach_buf(cw_kasit_t * a_kasit, cw_buf_t * a_buf)
{
  struct cw_kasit_entry_s * entry_arg;

  _cw_check_ptr(a_kasit);
  _cw_assert(_CW_KASIT_MAGIC == a_kasit->magic);
  _cw_check_ptr(a_buf);

  entry_arg
    = (struct cw_kasit_entry_s *) _cw_malloc(sizeof(struct cw_kasit_entry_s));
  if (NULL == entry_arg)
  {
    goto OOM_1;
  }

  buf_new(&entry_arg->buf);
  if (TRUE == buf_catenate_buf(&entry_arg->buf, a_buf, TRUE))
  {
    goto OOM_2;
  }
  
  kasit_p_entry((void *) entry_arg);

  return FALSE;

  OOM_2:
  buf_delete(&entry_arg->buf);
  _cw_free(entry_arg);
  OOM_1:
  return TRUE;
}

static cw_sint32_t
kasit_p_feed(cw_kasit_t * a_kasit, const char * a_str, cw_uint32_t a_len)
{
  cw_sint32_t retval;
  cw_uint32_t i;
  cw_uint8_t c;
  
  for (i = 0; i < a_len; i++)
  {
    c = a_str[i];

#if (0)
#define _CW_KASIS_PSTATE(a)           \
  do                                  \
  {                                   \
    if (a_kasit->state == (a))        \
    {                                 \
      out_put(cw_g_out, "[s]\n", #a); \
    }                                 \
  } while (0)
    
    out_put(cw_g_out, "c: '[c]' ([i]), index: [i] ", c, c, a_kasit->index);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_START);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_LT_CONT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_GT_CONT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_SLASH_CONT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_COMMENT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_NUMBER);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_ASCII_STRING);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_ASCII_STRING_NEWLINE_CONT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_ASCII_STRING_CRLF_CONT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_ASCII_STRING_PROT_CONT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_ASCII_STRING_HEX_CONT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_ASCII_STRING_HEX_FINISH);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_HEX_STRING);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_BASE85_STRING);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_BASE85_STRING_CONT);
    _CW_KASIS_PSTATE(_CW_KASIT_STATE_NAME);
#undef _CW_KASIS_PSTATE
#endif

    /* If a special character causes the end of the previous token, the state
     * machine builds the object, then restarts the state machine without
     * incrementing the input character index.  This is done in order to avoid
     * having to duplicate the _CW_KASIT_STATE_START code. */
    RESTART:
    
    switch (a_kasit->state)
    {
      case _CW_KASIT_STATE_START:
      {
	_cw_assert(0 == a_kasit->index);
	
	switch (c)
	{
	  case '(':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    a_kasit->meta.string.paren_depth = 1;
	    break;
	  }
	  case ')':
	  {
	    kasit_p_print_syntax_error(a_kasit, c);
	    break;
	  }
	  case '<':
	  {
	    a_kasit->state = _CW_KASIT_STATE_LT_CONT;
	    break;
	  }
	  case '>':
	  {
	    a_kasit->state = _CW_KASIT_STATE_GT_CONT;
	    break;
	  }
	  case '[':
	  {
	    kasit_p_print_token(a_kasit, 0, "[");
	    break;
	  }
	  case ']':
	  {
	    kasit_p_print_token(a_kasit, 0, "]");
	    break;
	  }
	  case '{':
	  {
	    kasit_p_print_token(a_kasit, 0, "{");
	    break;
	  }
	  case '}':
	  {
	    kasit_p_print_token(a_kasit, 0, "}");
	    break;
	  }
	  case '/':
	  {
	    a_kasit->state = _CW_KASIT_STATE_SLASH_CONT;
	    break;
	  }
	  case '%':
	  {
	    a_kasit->state = _CW_KASIT_STATE_COMMENT;
	    break;
	  }
	  case '\0': case '\t': case '\n': case '\f': case '\r': case ' ':
	  {
	    /* Swallow. */
	    break;
	  }
	  case '+':
	  {
	    a_kasit->state = _CW_KASIT_STATE_NUMBER;
	    a_kasit->meta.number.sign = _CW_KASIS_NUMBER_POS;
	    a_kasit->meta.number.base = 10;
	    a_kasit->meta.number.point_offset = -1;
	    a_kasit->meta.number.begin_offset = 1;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case '-':
	  {
	    a_kasit->state = _CW_KASIT_STATE_NUMBER;
	    a_kasit->meta.number.sign = _CW_KASIS_NUMBER_NEG;
	    a_kasit->meta.number.base = 10;
	    a_kasit->meta.number.point_offset = -1;
	    a_kasit->meta.number.begin_offset = 1;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case '.':
	  {
	    a_kasit->state = _CW_KASIT_STATE_NUMBER;
	    a_kasit->meta.number.sign = _CW_KASIS_NUMBER_POS;
	    a_kasit->meta.number.base = 10;
	    a_kasit->meta.number.point_offset = 0;
	    a_kasit->meta.number.begin_offset = 0;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case '0': case '1': case '2': case '3': case '4': case '5': case '6': 
	  case '7': case '8': case '9':
	  {
	    a_kasit->state = _CW_KASIT_STATE_NUMBER;
	    a_kasit->meta.number.sign = _CW_KASIS_NUMBER_POS;
	    a_kasit->meta.number.base = 10;
	    a_kasit->meta.number.point_offset = -1;
	    a_kasit->meta.number.begin_offset = 0;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  default:
	  {
	    a_kasit->state = _CW_KASIT_STATE_NAME;
	    a_kasit->meta.name.is_literal = FALSE;
	    a_kasit->meta.name.is_immediate = FALSE;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_LT_CONT:
      {
	_cw_assert(0 == a_kasit->index);
	
	switch (c)
	{
	  case '<':
	  {
	    a_kasit->state = _CW_KASIT_STATE_START;
	    kasit_p_print_token(a_kasit, 0, "<<");
	    break;
	  }
	  case '~':
	  {
	    a_kasit->state = _CW_KASIT_STATE_BASE85_STRING;
	    break;
	  }
	  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	  {
	    /* To lower case. */
	    c += 32;
	    /* Fall through. */
	  }
	  case '0': case '1': case '2': case '3': case '4': case '5':
	  case '6': case '7': case '8': case '9': case 'a': case 'b':
	  case 'c': case 'd': case 'e': case 'f':
	  {
	    a_kasit->state = _CW_KASIT_STATE_HEX_STRING;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case '\0': case '\t': case '\n': case '\f': case '\r': case ' ':
	  {
	    /* Whitespace within a hex string. */
	    a_kasit->state = _CW_KASIT_STATE_HEX_STRING;
	    break;
	  }
	  default:
	  {
	    kasit_p_print_syntax_error(a_kasit, c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_GT_CONT:
      {
	_cw_assert(0 == a_kasit->index);

	switch (c)
	{
	  case '>':
	  {
	    a_kasit->state = _CW_KASIT_STATE_START;
	    kasit_p_print_token(a_kasit, 0, ">>");
	    break;
	  }
	  default:
	  {
	    kasit_p_print_syntax_error(a_kasit, c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_SLASH_CONT:
      {
	_cw_assert(0 == a_kasit->index);

	switch (c)
	{
	  case '/':
	  {
	    a_kasit->state = _CW_KASIT_STATE_NAME;
	    a_kasit->meta.name.is_literal = FALSE;
	    a_kasit->meta.name.is_immediate = TRUE;
	    break;
	  }
	  case '\0': case '\t': case '\n': case '\f': case '\r': case ' ':
	  case '(': case ')': case '<': case '>': case '[': case ']':
	  case '{': case '}': case '%':
	  {
	    kasit_p_print_syntax_error(a_kasit, c);
	    break;
	  }
	  default:
	  {
	    a_kasit->state = _CW_KASIT_STATE_NAME;
	    a_kasit->meta.name.is_literal = TRUE;
	    a_kasit->meta.name.is_immediate = FALSE;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_COMMENT:
      {
	_cw_assert(0 == a_kasit->index);
	
	switch (c)
	{
	  case '\n': case '\r':
	  {
	    a_kasit->state = _CW_KASIT_STATE_START;
	    break;
	  }
	  default:
	  {
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_NUMBER:
      {
	switch (c)
	{
	  case '.':
	  {
	    if (a_kasit->meta.number.point_offset == -1)
	    {
	      a_kasit->meta.number.point_offset = a_kasit->index;
	    }
	    else
	    {
	      a_kasit->state = _CW_KASIT_STATE_NAME;
	      a_kasit->meta.name.is_literal = FALSE;
	      a_kasit->meta.name.is_immediate = FALSE;
	    }
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	  case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	  case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	  case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	  case 'Y': case 'Z':
	  {
	    /* To lower case. */
	    c += 32;
	    /* Fall through. */
	  }
	  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	  case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	  case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	  case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	  case 'y': case 'z':
	  {
	    if ((10 + ((cw_uint32_t) (c - 'a')))
		>= a_kasit->meta.number.base)
	    {
	      /* Too big for this base. */
	      a_kasit->state = _CW_KASIT_STATE_NAME;
	      a_kasit->meta.name.is_literal = FALSE;
	      a_kasit->meta.name.is_immediate = FALSE;
	    }
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case '0': case '1': case '2': case '3': case '4': case '5':
	  case '6': case '7': case '8': case '9':
	  {
	    if (((cw_uint32_t) (c - '0'))
		>= a_kasit->meta.number.base)
	    {
	      /* Too big for this base. */
	      a_kasit->state = _CW_KASIT_STATE_NAME;
	      a_kasit->meta.name.is_literal = FALSE;
	      a_kasit->meta.name.is_immediate = FALSE;
	    }
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case '#':
	  {
	    cw_uint32_t ndigits;
	    
	    ndigits = a_kasit->index - a_kasit->meta.number.begin_offset;
	    
	    if ((a_kasit->meta.number.point_offset != -1)
		|| (a_kasit->meta.number.begin_offset == a_kasit->index))
	    {
	      /* Decimal point already seen, or no base specified. */
	      a_kasit->state = _CW_KASIT_STATE_NAME;
	      a_kasit->meta.name.is_literal = FALSE;
	      a_kasit->meta.name.is_immediate = FALSE;
	    }
	    else
	    {
	      cw_uint32_t i, digit;
	      
	      /* Convert the string to a base (interpreted as base 10). */
	      a_kasit->meta.number.base = 0;
	      
	      for (i = 0; i < ndigits; i++)
	      {
		digit = _CW_KASIT_GETC(a_kasit->meta.number.begin_offset + i)
			 - '0';

		if (a_kasit->index - a_kasit->meta.number.begin_offset - i == 2)
		{
		  digit *= 10;
		}
		a_kasit->meta.number.base += digit;
		
		if (((0 != digit)
		     && ((a_kasit->index
			  - a_kasit->meta.number.begin_offset - i) > 2))
		    || (a_kasit->meta.number.base > 36))
		{
		  /* Base too large.  Set base to 0 so that the check for too
		   * small a base catches this. */
		  a_kasit->meta.number.base = 0;
		  break;
		}
	      }
	    
	      if (2 > a_kasit->meta.number.base)
	      {
		/* Base too small (or too large, as detected in the for
		 * loop above). */
		a_kasit->state = _CW_KASIT_STATE_NAME;
		a_kasit->meta.name.is_literal = FALSE;
		a_kasit->meta.name.is_immediate = FALSE;
	      }
	      else
	      {
		a_kasit->meta.number.begin_offset = a_kasit->index + 1;
	      }
	    }
	    
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case '(': case ')': case '<': case '>': case '[': case ']':
	  case '{': case '}': case '/': case '%':
	  {
	    /* New token. */
	    a_kasit->state = _CW_KASIT_STATE_START;
	    if ((1 < (a_kasit->index - a_kasit->meta.number.begin_offset))
		|| ((0 < (a_kasit->index - a_kasit->meta.number.begin_offset))
		    && (-1 == a_kasit->meta.number.point_offset)))
	    {
	      kasit_p_print_token(a_kasit, a_kasit->index, "number");
	    }
	    else
	    {
	      /* No number specified, so a name. */
	      kasit_p_print_token(a_kasit, a_kasit->index, "name");
	    }
	    kasit_p_reset_tok_buffer(a_kasit);
	    goto RESTART;
	  }
	  case '\0': case '\t': case '\n': case '\f': case '\r': case ' ':
	  {
	    a_kasit->state = _CW_KASIT_STATE_START;
	    if ((1 < (a_kasit->index - a_kasit->meta.number.begin_offset))
		|| ((0 < (a_kasit->index - a_kasit->meta.number.begin_offset))
		    && (-1 == a_kasit->meta.number.point_offset)))
	    {
	      kasit_p_print_token(a_kasit, a_kasit->index, "number");
	    }
	    else
	    {
	      /* No number specified, so a name. */
	      kasit_p_print_token(a_kasit, a_kasit->index, "name");
	    }
	    kasit_p_reset_tok_buffer(a_kasit);
	    break;
	  }
	  default:
	  {
	    /* Not a number character. */
	    a_kasit->state = _CW_KASIT_STATE_NAME;
	    a_kasit->meta.name.is_literal = FALSE;
	    a_kasit->meta.name.is_immediate = FALSE;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_ASCII_STRING:
      {
	/* The CRLF code jumps here if there was no LF. */
	ASCII_STRING_CONTINUE:
	
	switch (c)
	{
	  case '\\':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING_PROT_CONT;
	    break;
	  }
	  case '(':
	  {
	    a_kasit->meta.string.paren_depth++;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case ')':
	  {
	    a_kasit->meta.string.paren_depth--;
	    if (0 == a_kasit->meta.string.paren_depth)
	    {
	      /* Matched opening paren; not part of the string. */
	      a_kasit->state = _CW_KASIT_STATE_START;
	      kasit_p_print_token(a_kasit, a_kasit->index, "string");
	      kasit_p_reset_tok_buffer(a_kasit);
	    }
	    else
	    {
	      _CW_KASIT_PUTC(c);
	    }
	    break;
	  }
	  case '\r':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING_NEWLINE_CONT;
	    break;
	  }
	  default:
	  {
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_ASCII_STRING_NEWLINE_CONT:
      {
	/* All cases in the switch statement do this. */
	_CW_KASIT_PUTC('\n');
	a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	switch (c)
	{
	  case '\n':
	  {
	    break;
	  }
	  default:
	  {
	    /* '\r' was not followed by a '\n'.  Translate the '\r' to a '\n'
	     * and jump back up to the string scanning state to scan c again. */
	    goto ASCII_STRING_CONTINUE;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_ASCII_STRING_PROT_CONT:
      {
	switch (c)
	{
	  case 'n':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC('\n');
	    break;
	  }
	  case 'r':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC('\r');
	    break;
	  }
	  case 't':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC('\t');
	    break;
	  }
	  case 'b':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC('\b');
	    break;
	  }
	  case 'f':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC('\f');
	    break;
	  }
	  case '\\':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC('\\');
	    break;
	  }
	  case '(':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC('(');
	    break;
	  }
	  case ')':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC(')');
	    break;
	  }
	  case 'x':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING_HEX_CONT;
	    break;
	  }
	  case '\r':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING_CRLF_CONT;
	    break;
	  }
	  case '\n':
	  {
	    /* Ignore. */
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    break;
	  }
	  default:
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_ASCII_STRING_CRLF_CONT:
      {
	switch (c)
	{
	  case '\n':
	  {
	    /* Ignore. */
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    break;
	  }
	  default:
	  {
	    goto ASCII_STRING_CONTINUE;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_ASCII_STRING_HEX_CONT:
      {
	switch (c)
	{
	  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	  {
	    /* To lower case. */
	    c += 32;
	    /* Fall through. */
	  }
	  case '0': case '1': case '2': case '3': case '4': case '5':
	  case '6': case '7': case '8': case '9': case 'a': case 'b':
	  case 'c': case 'd': case 'e': case 'f':
	  {
	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING_HEX_FINISH;
	    a_kasit->meta.string.hex_val = c;
	    break;
	  }
	  default:
	  {
	    kasit_p_print_syntax_error(a_kasit, c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_ASCII_STRING_HEX_FINISH:
      {
	switch (c)
	{
	  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	  {
	    /* To lower case. */
	    c += 32;
	    /* Fall through. */
	  }
	  case '0': case '1': case '2': case '3': case '4': case '5':
	  case '6': case '7': case '8': case '9': case 'a': case 'b':
	  case 'c': case 'd': case 'e': case 'f':
	  {
	    cw_uint8_t val;

	    a_kasit->state = _CW_KASIT_STATE_ASCII_STRING;
	    switch (a_kasit->meta.string.hex_val)
	    {
	      case '0': case '1': case '2': case '3': case '4': case '5':
	      case '6': case '7': case '8': case '9':
	      {
		val = (a_kasit->meta.string.hex_val - '0') << 4;
		break;
	      }
	      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	      {
		val = (a_kasit->meta.string.hex_val - 'a') << 4;
		break;
	      }
	      default:
	      {
		_cw_error("Programming error");
	      }
	    }
	    switch (c)
	    {
	      case '0': case '1': case '2': case '3': case '4': case '5':
	      case '6': case '7': case '8': case '9':
	      {
		val |= (c - '0');
		break;
	      }
	      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	      {
		val |= (c - 'a');
		break;
	      }
	      default:
	      {
		_cw_error("Programming error");
	      }
	    }
	    _CW_KASIT_PUTC(val);
	    break;
	  }
	  default:
	  {
	    kasit_p_print_syntax_error(a_kasit, c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_HEX_STRING:
      {
	switch (c)
	{
	  case '>':
	  {
	    a_kasit->state = _CW_KASIT_STATE_START;
	    kasit_p_print_token(a_kasit, a_kasit->index, "hex string");
	    kasit_p_reset_tok_buffer(a_kasit);
	    break;
	  }
	  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	  {
	    /* To lower case. */
	    c += 32;
	    /* Fall through. */
	  }
	  case '0': case '1': case '2': case '3': case '4': case '5':
	  case '6': case '7': case '8': case '9': case 'a': case 'b':
	  case 'c': case 'd': case 'e': case 'f':
	  {
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	  case '\0': case '\t': case '\n': case '\f': case '\r': case ' ':
	  {
	    /* Ignore. */
	    break;
	  }
	  default:
	  {
	    kasit_p_print_syntax_error(a_kasit, c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_BASE85_STRING:
      {
	switch (c)
	{
	  case '~':
	  {
	    a_kasit->state = _CW_KASIT_STATE_BASE85_STRING_CONT;
	    break;
	  }
	  case '\0': case '\t': case '\n': case '\f': case '\r': case ' ':
	  {
	    /* Ignore. */
	    break;
	  }
	  default:
	  {
	    if ((('!' <= c) && ('u' >= c))
		|| 'z' == c)
	    {
	      _CW_KASIT_PUTC(c);
	    }
	    else
	    {
	      kasit_p_print_syntax_error(a_kasit, c);
	    }
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_BASE85_STRING_CONT:
      {
	switch (c)
	{
	  case '>':
	  {
	    a_kasit->state = _CW_KASIT_STATE_START;
	    kasit_p_print_token(a_kasit, a_kasit->index, "base 85 string");
	    kasit_p_reset_tok_buffer(a_kasit);
	    break;
	  }
	  default:
	  {
	    kasit_p_print_syntax_error(a_kasit, c);
	    break;
	  }
	}
	break;
      }
      case _CW_KASIT_STATE_NAME:
      {
	switch (c)
	{
	  case '\0': case '\t': case '\n': case '\f': case '\r': case ' ':
	  {
	    /* End of name. */
	    a_kasit->state = _CW_KASIT_STATE_START;
	    if (a_kasit->index > 0)
	    {
	      kasit_p_print_token(a_kasit, a_kasit->index, "name");
	    }
	    else
	    {
	      kasit_p_print_syntax_error(a_kasit, c);
	    }
	    kasit_p_reset_tok_buffer(a_kasit);
	    break;
	  }
	  case '(': case ')': case '<': case '>': case '[': case ']':
	  case '{': case '}': case '/': case '%':
	  {
	    /* New token. */
	    a_kasit->state = _CW_KASIT_STATE_START;
	    if (a_kasit->index > 0)
	    {
	      kasit_p_print_token(a_kasit, a_kasit->index, "name");
	    }
	    else
	    {
	      kasit_p_print_syntax_error(a_kasit, c);
	    }
	    kasit_p_reset_tok_buffer(a_kasit);
	    goto RESTART;
	  }
	  default:
	  {
	    _CW_KASIT_PUTC(c);
	    break;
	  }
	}
	break;
      }
      default:
      {
	_cw_error("Programming error");
	break;
      }
    }
  }

  retval = 0;
  RETURN:
  return retval;
}

static void
kasit_p_reset_tok_buffer(cw_kasit_t * a_kasit)
{
  if (_CW_KASI_BUFC_SIZE < a_kasit->index)
  {
    buf_delete(&a_kasit->tok_buffer.buf);
  }
  a_kasit->index = 0;
}

#ifndef _CW_KASIT_INLINE
static cw_uint8_t
kasit_p_getc(cw_kasit_t * a_kasit, cw_uint32_t a_index)
{
  cw_uint8_t retval;
  
  if (_CW_KASI_BUFC_SIZE > a_kasit->index)
  {
    retval = a_kasit->tok_buffer.str[a_index];
  }
  else
  {
    retval = buf_get_uint8(&a_kasit->tok_buffer.buf, a_index);
  }
  
  return retval;
}
#endif

static cw_sint32_t
kasit_p_putc(cw_kasit_t * a_kasit, cw_uint32_t a_c)
{
  cw_sint32_t retval;

#ifndef _CW_KASIT_INLINE
  if (_CW_KASI_BUFC_SIZE > a_kasit->index)
  {
    a_kasit->tok_buffer.str[a_kasit->index] = a_c;
  }
  else
#endif
  {
    if (_CW_KASI_BUFC_SIZE == a_kasit->index)
    {
      cw_kasi_bufc_t * kbufc;
      kbufc = kasi_get_kasi_bufc(a_kasit->kasi);
      memcpy(kbufc->buffer, a_kasit->tok_buffer.str, _CW_KASI_BUFC_SIZE);
      buf_new(&a_kasit->tok_buffer.buf);
      if (TRUE == buf_append_bufc(&a_kasit->tok_buffer.buf, &kbufc->bufc,
				  0, _CW_KASI_BUFC_SIZE))
      {
	bufc_delete(&kbufc->bufc);
	retval = -1;
	goto RETURN;
      }
      bufc_delete(&kbufc->bufc);
    }
    if (buf_get_size(&a_kasit->tok_buffer.buf) == a_kasit->index)
    {
      cw_kasi_bufc_t * kbufc;
      kbufc = kasi_get_kasi_bufc(a_kasit->kasi);
      if (TRUE == buf_append_bufc(&a_kasit->tok_buffer.buf, &kbufc->bufc,
				  0, _CW_KASI_BUFC_SIZE))
      {
	bufc_delete(&kbufc->bufc);
	retval = -1;
	goto RETURN;
      }
      bufc_delete(&kbufc->bufc);
    }
    buf_set_uint8(&a_kasit->tok_buffer.buf, a_kasit->index, a_c);
  }
  a_kasit->index++;

  retval = 0;
  RETURN:
  return retval;
}

static void
kasit_p_print_token(cw_kasit_t * a_kasit, cw_uint32_t a_length,
		    const char * a_note)
{
#ifdef _LIBKASI_DBG
  out_put(cw_g_out, "-->");
  if (_CW_KASI_BUFC_SIZE >= a_kasit->index)
  {
    out_put_n(cw_g_out, a_length, "[s]", a_kasit->tok_buffer.str);
  }
  else
  {
    out_put_n(cw_g_out, a_length, "[b]", &a_kasit->tok_buffer.buf);
  }
  out_put(cw_g_out, "<-- [s]\n", a_note);
#endif
}

static void
kasit_p_print_syntax_error(cw_kasit_t * a_kasit, cw_uint8_t a_c)
{
  _cw_out_put_e("Syntax error for '[c]' (0x[i|b:16]), following -->", a_c, a_c);
  if (_CW_KASI_BUFC_SIZE >= a_kasit->index)
  {
    out_put_n(cw_g_out, a_kasit->index, "[s]", a_kasit->tok_buffer.str);
  }
  else
  {
    out_put_n(cw_g_out, a_kasit->index, "[b]", &a_kasit->tok_buffer.buf);
  }
  out_put(cw_g_out, "<--\n");
  a_kasit->state = _CW_KASIT_STATE_START;
  kasit_p_reset_tok_buffer(a_kasit);
}

static void *
kasit_p_entry(void * a_arg)
{
  struct cw_kasit_entry_s * arg = (struct cw_kasit_entry_s *) a_arg;

  if (TRUE == kasit_interp_buf(arg->kasit, &arg->buf))
  {
    /* XXX OOM error needs delivered in interpreter. */
  }

  buf_delete(&arg->buf);
  kasit_delete(arg->kasit);
  /* XXX Deal with detach/join inside interpreter. */
  thd_delete(&arg->thd);
  _cw_free(arg);
  return NULL;
}
