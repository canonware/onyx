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

/* Defined here to resolve circular dependencies. */
typedef struct cw_kasioe_array_s cw_kasioe_array_t;
typedef struct cw_kasioe_condition_s cw_kasioe_condition_t;
typedef struct cw_kasioe_dict_s cw_kasioe_dict_t;
typedef struct cw_kasioe_lock_s cw_kasioe_lock_t;
typedef struct cw_kasioe_mstate_s cw_kasioe_mstate_t;
typedef struct cw_kasioe_number_s cw_kasioe_number_t;
typedef struct cw_kasioe_packedarray_s cw_kasioe_packedarray_t;
typedef struct cw_kasioe_string_s cw_kasioe_string_t;
typedef struct cw_kasin_s cw_kasin_t;
typedef struct cw_kasit_s cw_kasit_t;

typedef enum
{
  _CW_KASIOT_NOTYPE          =  0,
  _CW_KASIOT_ARRAYTYPE       =  1,
  _CW_KASIOT_BOOLEANTYPE     =  2,
  _CW_KASIOT_CONDITIONTYPE   =  3,
  _CW_KASIOT_DICTTYPE        =  4,
  _CW_KASIOT_FILETYPE        =  5,
  _CW_KASIOT_LOCKTYPE        =  6,
  _CW_KASIOT_MARKTYPE        =  7,
  _CW_KASIOT_MSTATETYPE      =  8,
  _CW_KASIOT_NAMETYPE        =  9,
  _CW_KASIOT_NULLTYPE        = 10,
  _CW_KASIOT_NUMBERTYPE      = 11,
  _CW_KASIOT_OPERATORTYPE    = 12,
  _CW_KASIOT_PACKEDARRAYTYPE = 13,
  _CW_KASIOT_STRINGTYPE      = 14
} cw_kasiot_t;

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
    /* Not an enumerated type, since that would make it a full machine word.
     * Instead, cw_kasiot_t is cast to a cw_uint8_t before being stored here. */
    cw_uint8_t type;
    /* If non-zero, this is an extended type.  This field is used for number and
     * mstate objects, which can both switch between simple and extended
     * representations.
     *
     * In addition, name objects use it to indicate if a name is an indirect
     * reference.  Each kasit maintains a cache of kasin pointers, each holding
     * a single reference to the names hash in kasi.  If this is an indirect
     * reference (1 == flags.extended), the unreferencing operation should
     * actually be done with the kasit's kasin cache.  Note that this is the
     * (very) common case.  Use the kasin pointer to get the actual name "value"
     * for the unreferencing operation.  This is safe, because this kasit is
     * guaranteed to be holding a reference to the kasin. */
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
      /* If not (flags.extended), the mstate is:
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
      cw_kasin_t * kasin;
    } name;
    struct
    {
      cw_uint32_t garbage;
    } null;
    struct
    {
      /* If not (flags.extended), this number is representable as a 32 bit
       * signed integer.  Otherwise the ext contains the value. */
      union
      {
	cw_kasioe_number_t * kasioe;
	cw_sint32_t s32;
      } val;
    } number;
    struct
    {
      void (*f)(cw_kasit_t *);
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

void
kasio_new(cw_kasio_t * a_kasio);

void
kasio_delete(cw_kasio_t * a_kasio);

cw_kasiot_t
kasio_type(cw_kasio_t * a_kasio);

void
kasio_copy(cw_kasio_t * a_to, cw_kasio_t * a_from);

cw_bool_t
kasio_cast(cw_kasio_t * a_kasio, cw_kasiot_t a_kasiot);
