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
typedef struct cw_arraytype_s cw_arraytype_t;
typedef struct cw_booleantype_s cw_booleantype_t;
typedef struct cw_conditiontype_s cw_conditiontype_t;
typedef struct cw_dicttype_s cw_dicttype_t;
typedef struct cw_filetype_s cw_filetype_t;
typedef struct cw_locktype_s cw_locktype_t;
typedef struct cw_marktype_s cw_marktype_t;
typedef struct cw_mstatetype_s cw_mstatetype_t;
typedef struct cw_nametype_s cw_nametype_t;
typedef struct cw_nulltype_s cw_nulltype_t;
typedef struct cw_numbertype_s cw_numbertype_t;
typedef struct cw_operatortype_s cw_operatortype_t;
typedef struct cw_passthrutype_s cw_passthrutype_t; /* Internal library use
						     * only. */
typedef struct cw_packedarraytype_s cw_packedarraytype_t;
typedef struct cw_savetype_s cw_savetype_t;
typedef struct cw_stringtype_s cw_stringtype_t;

typedef struct cw_kasio_ext_s cw_kasio_ext_t;
typedef struct cw_arrayext_s cw_arrayext_t;
typedef struct cw_conditionext_s cw_conditionext_t;
typedef struct cw_dictext_s cw_dictext_t;
typedef struct cw_lockext_s cw_lockext_t;
typedef struct cw_nameext_s cw_nameext_t;
typedef struct cw_numberext_s cw_numberext_t;
typedef struct cw_operatorext_s cw_operatorext_t;
typedef struct cw_packedarrayext_s cw_packedarrayext_t;
typedef struct cw_saveext_s cw_saveext_t;
typedef struct cw_stringext_s cw_stringext_t;

/*
 * type's.
 */
enum cw_kasio_type_s
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
  _CW_KASIO_NULLTYPE,
  _CW_KASIO_NUMBERTYPE,
  _CW_KASIO_OPERATORTYPE,
  _CW_KASIO_PACKEDARRAYTYPE,
  _CW_KASIO_PASSTHRUTYPE, /* Internal library use only. */
  _CW_KASIO_SAVETYPE,
  _CW_KASIO_STRINGTYPE
};

struct cw_arraytype_s
{
  cw_arrayext_t * ext;
};

struct cw_booleantype_s
{
  cw_bool_t boolean;
};

struct cw_conditiontype_s
{
  cw_conditionext_t * ext;
};

struct cw_dicttype_s
{
  cw_dictext_t * ext;
};

struct cw_filetype_s
{
  cw_sint32_t fd;
};

struct cw_locktype_s
{
  cw_lockext_t * ext;
};

struct cw_marktype_s
{
  int garbage;
};

struct cw_mstatetype_s
{
  cw_uint32_t accuracy;
  cw_uint32_t point;
  cw_uint32_t base;
};

struct cw_nametype_s
{
  cw_nameext_t * ext;
};

struct cw_nulltype_s
{
  int garbage;
};

struct cw_numbertype_s
{
  enum
  {
    _CW_KASIO_NUMBER_UINT32,
    _CW_KASIO_NUMBER_SINT32,
    _CW_KASIO_NUMBER_UINT64,
    _CW_KASIO_NUMBER_SINT64,
    _CW_KASIO_NUMBER_OTHER
  } type;
  union
  {
    cw_numberext_t * ext;
    cw_sint32_t u32;
    cw_sint32_t s32;
    cw_sint32_t u64;
    cw_sint32_t s64;
  } val;
};

struct cw_operatortype_s
{
  cw_operatorext_t * ext;
};

struct cw_packedarraytype_s
{
  cw_packedarrayext_t * ext;
};

struct cw_passthrutype_s
{
  cw_kasio_t * object;
};

struct cw_savetype_s
{
  cw_saveext_t * ext;
};

struct cw_stringtype_s
{
  cw_stringext_t * ext;
};

/*
 * ext's.
 */
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

struct cw_nameext_s
{
  cw_kasio_ext_t ext;
};

struct cw_numberext_s
{
  cw_kasio_ext_t ext;
  cw_mstatetype_t mstate;
  union
  {
    cw_sint32_t u32;
    cw_sint32_t s32;
    cw_sint32_t u64;
    cw_sint32_t s64;
    cw_uint8_t * space;
  } val;
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

/*
 * Main object structure.
 */
struct cw_kasio_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  cw_kasio_type_t type;
  union
  {
    cw_arraytype_t array;
    cw_booleantype_t boolean;
    cw_conditiontype_t condition;
    cw_dicttype_t dict;
    cw_filetype_t file;
    cw_locktype_t lock;
    cw_marktype_t mark;
    cw_mstatetype_t mstate;
    cw_nametype_t name;
    cw_nulltype_t null;
    cw_numbertype_t number;
    cw_operatortype_t operator;
    cw_passthrutype_t passthru;
    cw_packedarraytype_t packedarray;
    cw_savetype_t save;
    cw_stringtype_t string;
  } object;
};
