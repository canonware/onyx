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

typedef struct cw_kasioe_s cw_kasioe_t;
typedef struct cw_kasioer_s cw_kasioer_t;

typedef enum
{
  _CW_KASIOEA_NONE        = 0,
  _CW_KASIOEA_UNLIMITED   = 1,
  _CW_KASIOEA_READONLY    = 2,
  _CW_KASIOEA_EXECUTEONLY = 3
} cw_kasioea_t;

/* All extended type objects contain a kasioe.  This provides a poor man's
 * inheritance.  Since kasi's type system is static, this idiom is adequate. */
struct cw_kasioe_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  cw_kasiot_t type : 4;
  cw_kasioea_t access : 2;
  /* If TRUE, there is a watchpoint set on this object.  In general, this field
   * is not looked at unless the interpreter has been put into debugging mode.
   * Note that setting a watchpoint on an extended type causes modification via
   * *any* reference to be watched. */
  cw_bool_t watchpoint : 1;
  /* If TRUE, this object is black (or gray).  Otherwise it is white. */
  cw_bool_t black : 1;
  /* If TRUE, this object cannot be modified, which means it need not be
   * locked, even if global. */
  cw_bool_t immutable : 1;
};

void
kasioe_new(cw_kasioe_t * a_kasioe);

void
kasioe_delete(cw_kasioe_t * a_kasioe);
