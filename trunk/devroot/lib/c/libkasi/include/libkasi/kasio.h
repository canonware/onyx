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

typedef struct cw_kasioe_s cw_kasioe_t;
typedef struct cw_kasioe_array_s cw_kasioe_array_t;
typedef struct cw_kasioe_condition_s cw_kasioe_condition_t;
typedef struct cw_kasioe_dict_s cw_kasioe_dict_t;
typedef struct cw_kasioe_lock_s cw_kasioe_lock_t;
typedef struct cw_kasioe_mstate_s cw_kasioe_mstate_t;
typedef struct cw_kasioe_name_s cw_kasioe_name_t;
typedef struct cw_kasioe_number_s cw_kasioe_number_t;
typedef struct cw_kasioe_operator_s cw_kasioe_operator_t;
typedef struct cw_kasioe_packedarray_s cw_kasioe_packedarray_t;
typedef struct cw_kasioe_string_s cw_kasioe_string_t;

struct cw_kasioe_s
{
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;

  /* If TRUE, then this object must be locked during access. */
  cw_bool_t is_global;
  cw_mtx_t lock;
  
  cw_uint32_t ref_count;
  cw_dch_t * keyed_refs;
  enum
  {
    _CW_KASIOE_UNLIMITED,
    _CW_KASIOE_READONLY,
    _CW_KASIOE_EXECUTEONLY,
    _CW_KASIOE_NONE
  } access;
  cw_bool_t is_watchpoint;
};

struct cw_kasioe_array_s
{
  cw_kasioe_t kasioe;
};

struct cw_kasioe_condition_s
{
  cw_kasioe_t kasioe;
  cw_cnd_t condition;
};

/* Defined in kasid.h, in order to resolve a circular dependency. */
#if (0)
struct cw_kasioe_dict_s
{
  cw_kasioe_t kasioe;
  /* Must come last, since its size varies. */
  cw_kasid_t dict;
};
#endif

struct cw_kasioe_lock_s
{
  cw_kasioe_t kasioe;
  cw_mtx_t lock;
};

struct cw_kasioe_mstate_s
{
  cw_kasioe_t kasioe;
  cw_uint32_t accuracy;
  cw_uint32_t point;
  cw_uint32_t base;
};

struct cw_kasioe_name_s
{
  cw_kasioe_t kasioe;
  cw_uint32_t len;
  cw_uint8_t * name;
};

struct cw_kasioe_number_s
{
  cw_kasioe_t kasioe;
  /* Offset in val that the "decimal point" precedes. */
  cw_uint32_t point;
  /* Base.  Can be from 2 to 36, inclusive. */
  cw_uint32_t base;
  /* Number of bytes that val points to. */
  cw_uint32_t val_len;
  /* Offset of most significant non-zero digit. */
  cw_uint32_t val_msd;
  /* The least significant digit is at val[0].  Each byte can range in value
   * from 0 to 35, depending on the base.  This representation is not compact,
   * but it is easy to work with. */
  cw_uint8_t * val;
};

/* Defined here (instead of in kasit.h) to resolve a circular dependency. */
typedef struct cw_kasit_s cw_kasit_t;

struct cw_kasioe_operator_s
{
  cw_kasioe_t kasioe;
  void (*operator)(cw_kasit_t *);
};

struct cw_kasioe_packedarray_s
{
  cw_kasioe_t kasioe;
};

struct cw_kasioe_string_s
{
  cw_kasioe_t kasioe;
};

/*
 * Main object structure.
 */
struct cw_kasio_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  struct
  {
    /* Not an enumerated type, since that would make it a full machine word. */
#define _CW_KASIO_ARRAYTYPE        1
#define _CW_KASIO_BOOLEANTYPE      2
#define _CW_KASIO_CONDITIONTYPE    3
#define _CW_KASIO_DICTTYPE         4
#define _CW_KASIO_FILETYPE         5
#define _CW_KASIO_LOCKTYPE         6
#define _CW_KASIO_MARKTYPE         7
#define _CW_KASIO_MSTATETYPE       8
#define _CW_KASIO_NAMETYPE         9
#define _CW_KASIO_NULLTYPE        10
#define _CW_KASIO_NUMBERTYPE      11
#define _CW_KASIO_OPERATORTYPE    12
#define _CW_KASIO_PACKEDARRAYTYPE 13
#define _CW_KASIO_STRINGTYPE      14
    cw_uint8_t type;
    /* If non-zero, this is an extended type.  This field is only used for
     * number and mstate objects, since no other types can switch between simple
     * and extended. */
    cw_uint8_t extended;
    /* If non-zero, there is a breakpoint set on this object.  In general, this
     * field is not looked at unless the interpreter has been put into debugging
     * mode. */
    cw_uint8_t breakpoint;
    /* If non-zero, there is a watchpoint set on this object.  In general, this
     * field is not looked at unless the interpreter has been put into debugging
     * mode.  Note that setting a watchpoint on an extended type only detects
     * changes that are made via that particular reference to the extension. */
    cw_uint8_t watchpoint;
  } flags;

  union
  {
    struct
    {
      cw_kasioe_array_t * kasioe;
    } array;
    struct
    {
      cw_bool_t val;
    } boolean;
    struct
    {
      cw_kasioe_condition_t * kasioe;
    } condition;
    struct
    {
      cw_kasioe_dict_t * kasioe;
    } dict;
    struct
    {
      cw_sint32_t fd;
    } file;
    struct
    {
      cw_kasioe_lock_t * kasioe;
    } lock;
    struct
    {
      cw_uint32_t garbage;
    } mark;
    struct
    {
      /* If (flags.extended), the mstate is:
       *
       * accuracy : 32
       * point    : 0
       * base     : 10
       *
       * Otherwise, the mstate is in ext. */
      cw_kasioe_mstate_t * kasioe;
    } mstate;
    struct
    {
      cw_kasioe_name_t * kasioe;
    } name;
    struct
    {
      cw_uint32_t garbage;
    } null;
    struct
    {
      /* If (flags.extended), this number is representable as a 32 bit signed
       * integer.  Otherwise the ext contains the value. */
      union
      {
	cw_kasioe_number_t * kasioe;
	cw_sint32_t s32;
      } val;
    } number;
    struct
    {
      cw_kasioe_operator_t * kasioe;
    } operator;
    struct
    {
      cw_kasioe_packedarray_t * kasioe;
    } packedarray;
    struct
    {
      cw_kasioe_string_t * kasioe;
    } string;
  } o;
};
