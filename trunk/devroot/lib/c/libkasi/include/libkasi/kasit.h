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

typedef struct cw_kasit_s cw_kasit_t;

struct cw_kasit_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  /* kasi this kasit is part of. */
  cw_kasi_t * kasi;

/*    cw_mtx_t lock; */
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;

  /* Tokenizer state.  If a token is broken across two or more input strings,
   * data are copied to an internal buffer, and state machine state is preserved
   * so that the buffered data need not be processed again. */

  /* If greater than _CW_KASI_BUFC_SIZE, tok_buffer is a buf.  Otherwise, it
   * is a character array. */
  cw_uint32_t index;

  union
  {
    cw_buf_t buf;
    cw_uint8_t str[_CW_KASI_BUFC_SIZE];
  } tok_buffer;
  enum
  {
    _CW_KASIT_STATE_START,
    _CW_KASIT_STATE_LT_CONT,
    _CW_KASIT_STATE_GT_CONT,
    _CW_KASIT_STATE_SLASH_CONT,
    _CW_KASIT_STATE_COMMENT,
    _CW_KASIT_STATE_NUMBER,
    _CW_KASIT_STATE_ASCII_STRING,
    _CW_KASIT_STATE_ASCII_STRING_PROT_CONT,
    _CW_KASIT_STATE_ASCII_STRING_CRLF_CONT,
    _CW_KASIT_STATE_ASCII_STRING_HEX_CONT,
    _CW_KASIT_STATE_ASCII_STRING_HEX_FINISH,
    _CW_KASIT_STATE_HEX_STRING,
    _CW_KASIT_STATE_BASE85_STRING,
    _CW_KASIT_STATE_BASE85_STRING_CONT,
    _CW_KASIT_STATE_NAME
  } state;

  union
  {
    struct
    {
      enum
      {
	_CW_KASIS_NUMBER_POS,
	_CW_KASIS_NUMBER_NEG
      } sign;
      cw_uint32_t base;
      cw_sint32_t point_offset;
      cw_uint32_t begin_offset;
    } number;
    struct
    {
      cw_uint32_t paren_depth;
      cw_uint8_t hex_val;
    } string;
    struct
    {
      cw_bool_t is_literal;
      cw_bool_t is_immediate;
    } name;
  } meta;
  cw_bool_t procedure_depth;
};

cw_kasit_t *
kasit_new(cw_kasit_t * a_kasit,
	  void (*a_dealloc_func)(void * dealloc_arg, void * kasit),
	  void * a_dealloc_arg,
	  cw_kasi_t * a_kasi);

void
kasit_delete(cw_kasit_t * a_kasit);

cw_bool_t
kasit_interp_str(cw_kasit_t * a_kasit, const char * a_str, cw_uint32_t a_len);

cw_bool_t
kasit_interp_buf(cw_kasit_t * a_kasit, cw_buf_t * a_buf);

void
kasit_detach_str(cw_kasit_t * a_kasit, const char * a_str, cw_uint32_t a_len);

void
kasit_detach_buf(cw_kasit_t * a_kasit, cw_buf_t * a_buf);
