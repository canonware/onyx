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

typedef struct cw_kasio_s cw_kasio_t;

typedef enum
{
  _CW_KASIO_ARRAYTYPE,
  _CW_KASIO_BOOLEANTYPE,
  _CW_KASIO_CONDITIONTYPE,
  _CW_KASIO_DICTTYPE,
  _CW_KASIO_FILETYPE,
  _CW_KASIO_LOCKTYPE,
  _CW_KASIO_MARKTYPE,
  _CW_KASIO_MSTATETYPE,
  _CW_KASIO_NAMETYPE,
  _CW_KASIO_NUMBERTYPE,
  _CW_KASIO_NULLTYPE,
  _CW_KASIO_OPERATORTYPE,
  _CW_KASIO_PACKEDARRAYTYPE,
  _CW_KASIO_SAVETYPE,
  _CW_KASIO_STRINGTYPE
} cw_kasio_type_t;

struct cw_kasio_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_a;
#endif
  
  /* Used for linking together kasio structures when one isn't big enough for an
   * operand. */
  cw_ring_t link;
  
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;
  cw_uint32_t ref_count;

  cw_kasio_type_t type;

  cw_uint8_t op[32];
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_b;
#endif
};

void
kasio_extend(cw_kasio_t * a_kasio, cw_kasio_t * a_tail);
