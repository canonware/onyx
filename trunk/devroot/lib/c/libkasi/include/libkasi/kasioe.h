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

typedef enum
{
  _CW_KASIOEA_NONE        = 0,
  _CW_KASIOEA_UNLIMITED   = 1,
  _CW_KASIOEA_READONLY    = 2,
  _CW_KASIOEA_EXECUTEONLY = 3
} kasioea;

/* All extended type objects contain a kasioe.  This provides a poor man's
 * inheritance.  Since kasi's type system is static, this idiom is adequate. */
struct cw_kasioe_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  struct
  {
    /* Not an enumerated type, since that would make it a full machine word.
     * Instead, cw_kasiot_t is cast to a cw_uint8_t before being stored here. */
    cw_uint8_t type;
    /* Not an enumerated type, since that would make it a full machine word.
     * Instead, cw_kasioea_t is cast to a cw_uint8_t before being stored
     * here. */
    cw_uint8_t access;
    /* If non-zero, then this object must be locked during access. */
    cw_uint8_t global;
    /* If non-zero, there is a watchpoint set on this object.  In general, this
     * field is not looked at unless the interpreter has been put into debugging
     * mode.  Note that setting a watchpoint on an extended type causes
     * modification via *any* reference to be watched. */
    cw_uint8_t watchpoint;
  } flags;
  cw_mtx_t lock;

  /* Number of references to this object. */
  cw_uint32_t ref_count;

  /* Deallocation hooks. */
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;
};

void
kasioe_new(cw_kasioe_t * a_kasioe);

void
kasioe_delete(cw_kasioe_t * a_kasioe);
