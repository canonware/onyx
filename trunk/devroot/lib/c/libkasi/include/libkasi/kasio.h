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

typedef enum cw_kasio_type_s cw_kasio_type_t;

typedef struct cw_kasio_ext_s cw_kasio_ext_t;
typedef struct cw_arrayext_s cw_arrayext_t;
typedef struct cw_conditionext_s cw_conditionext_t;
typedef struct cw_dictext_s cw_dictext_t;
typedef struct cw_lockext_s cw_lockext_t;
typedef struct cw_mstateext_s cw_mstateext_t;
typedef struct cw_nameext_s cw_nameext_t;
typedef struct cw_numberext_s cw_numberext_t;
typedef struct cw_operatorext_s cw_operatorext_t;
typedef struct cw_packedarrayext_s cw_packedarrayext_t;
typedef struct cw_saveext_s cw_saveext_t;
typedef struct cw_stringext_s cw_stringext_t;

struct cw_kasio_ext_s
{
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;
  cw_uint32_t ref_count;
};

struct cw_arrayext_s
{
  cw_kasio_ext_t ext;
};

struct cw_conditionext_s
{
  cw_kasio_ext_t ext;
  cw_cnd_t condition;
};

/* Defined in kasid.h, in order to resolve a circular dependency. */
#if (0)
struct cw_dictext_s
{
  cw_kasio_ext_t ext;
  cw_kasid_t dict;
};
#endif

struct cw_lockext_s
{
  cw_kasio_ext_t ext;
  cw_mtx_t lock;
};

struct cw_mstateext_s
{
  cw_kasio_ext_t ext;
  cw_uint32_t accuracy;
  cw_uint32_t point;
  cw_uint32_t base;
};

struct cw_nameext_s
{
  cw_kasio_ext_t ext;
};

struct cw_numberext_s
{
  cw_kasio_ext_t ext;
  cw_uint32_t accuracy;
  cw_uint32_t point;
  cw_uint32_t base;
  cw_uint8_t * val;
};

struct cw_operatorext_s
{
  cw_kasio_ext_t ext;
  cw_kasio_t * operator;
};

struct cw_packedarrayext_s
{
  cw_kasio_ext_t ext;
};

struct cw_saveext_s
{
  cw_kasio_ext_t ext;
};

struct cw_stringext_s
{
  cw_kasio_ext_t ext;
};

/* Bit flags used in the flags field of cw_kasio_t. */
#define _CW_KASIO_EXT             0x00000001
#define _CW_KASIO_ARRAYTYPE       0x00000002
#define _CW_KASIO_BOOLEANTYPE     0x00000004
#define _CW_KASIO_CONDITIONTYPE   0x00000008
#define _CW_KASIO_DICTTYPE        0x00000010
#define _CW_KASIO_FILETYPE        0x00000020
#define _CW_KASIO_LOCKTYPE        0x00000040
#define _CW_KASIO_MARKTYPE        0x00000080
#define _CW_KASIO_MSTATETYPE      0x00000100
#define _CW_KASIO_NAMETYPE        0x00000200
#define _CW_KASIO_NULLTYPE        0x00000400
#define _CW_KASIO_NUMBERTYPE      0x00000800
#define _CW_KASIO_OPERATORTYPE    0x00001000
#define _CW_KASIO_PACKEDARRAYTYPE 0x00002000
#define _CW_KASIO_PASSTHRUTYPE    0x00004000
#define _CW_KASIO_SAVETYPE        0x00008000
#define _CW_KASIO_STRINGTYPE      0x00010000

/*
 * Main object structure.
 */
struct cw_kasio_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  cw_uint32_t flags;
  union
  {
    struct
    {
      cw_arrayext_t * ext;
    } array;
    struct
    {
      cw_bool_t val;
    } boolean;
    struct
    {
      cw_conditionext_t * ext;
    } condition;
    struct
    {
      cw_dictext_t * ext;
    } dict;
    struct
    {
      cw_sint32_t fd;
    } file;
    struct
    {
      cw_lockext_t * ext;
    } lock;
    struct
    {
      cw_uint32_t garbage;
    } mark;
    struct
    {
      /* If (flags & _CW_KASIO_EXT), the mstate is:
       *
       * accuracy : 32
       * point    : 0
       * base     : 10
       *
       * Otherwise, the mstate is in ext. */
      cw_mstateext_t * ext;
    } mstate;
    struct
    {
      cw_nameext_t * ext;
    } name;
    struct
    {
      cw_uint32_t garbage;
    } null;
    struct
    {
      /* If (flags & _CW_KASIO_EXT), this number is representable as a 32 bit
       * signed integer.  Otherwise the ext contains the value. */
      union
      {
	cw_numberext_t * ext;
	cw_sint32_t s32;
      } val;
    } number;
    struct
    {
      cw_operatorext_t * ext;
    } operator;
    /* Internal use only. */
    struct
    {
      cw_kasio_t * object;
    } passthru;
    struct
    {
      cw_packedarrayext_t * ext;
    } packedarray;
    struct
    {
      cw_saveext_t * ext;
    } save;
    struct
    {
      cw_stringext_t * ext;
    } string;
  } o;
};
