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

#ifdef _LIBKASI_DBG
#  define _CW_KASIO_MAGIC 0x398754ba
#endif

void
kasio_new(cw_kasio_t * a_kasio)
{
  _cw_check_ptr(a_kasio);

  bzero(a_kasio, sizeof(cw_kasio_t));
#ifdef _LIBKASI_DBG
  a_kasio->magic = _CW_KASIO_MAGIC;
#endif
}

void
kasio_delete(cw_kasio_t * a_kasio)
{
  _cw_check_ptr(a_kasio);
  _cw_assert(_CW_KASIO_MAGIC == a_kasio->magic);

  switch (a_kasio->type)
  {
    case _CW_KASIOT_NOTYPE:
    case _CW_KASIOT_BOOLEANTYPE:
    case _CW_KASIOT_FILETYPE:
    case _CW_KASIOT_MARKTYPE:
    case _CW_KASIOT_NULLTYPE:
    case _CW_KASIOT_OPERATORTYPE:
    {
      /* Simple type; do nothing. */
      break;
    }
    case _CW_KASIOT_ARRAYTYPE:
    {
      kasioe_array_unref(a_kasio->o.array.kasioe);
      break;
    }
    case _CW_KASIOT_CONDITIONTYPE:
    {
      kasioe_condition_unref(a_kasio->o.condition.kasioe);
      break;
    }
    case _CW_KASIOT_DICTTYPE:
    {
      break;
    }
    case _CW_KASIOT_LOCKTYPE:
    {
      break;
    }
    case _CW_KASIOT_MSTATETYPE:
    {
      if (FALSE != a_kasio->extended)
      {
	/* XXX */
      }
      break;
    }
    case _CW_KASIOT_NAMETYPE:
    {
      break;
    }
    case _CW_KASIOT_NUMBERTYPE:
    {
      if (FALSE != a_kasio->extended)
      {
	/* XXX */
      }
      break;
    }
    case _CW_KASIOT_PACKEDARRAYTYPE:
    {
      break;
    }
    case _CW_KASIOT_STRINGTYPE:
    {
      break;
    }
    default:
    {
      _cw_error("Programming error");
    }
  }

#ifdef _LIBKASI_DBG
  a_kasio->magic = _CW_KASIO_MAGIC;
#endif
}

cw_kasiot_t
kasio_type(cw_kasio_t * a_kasio)
{
  _cw_check_ptr(a_kasio);
  _cw_assert(_CW_KASIO_MAGIC == a_kasio->magic);

  return a_kasio->type;
}

void
kasio_copy(cw_kasio_t * a_to, cw_kasio_t * a_from)
{
  _cw_check_ptr(a_to);
  _cw_assert(_CW_KASIO_MAGIC == a_to->magic);
  _cw_check_ptr(a_from);
  _cw_assert(_CW_KASIO_MAGIC == a_from->magic);
}

cw_bool_t
kasio_cast(cw_kasio_t * a_kasio, cw_kasiot_t a_kasiot)
{
  _cw_check_ptr(a_kasio);
  _cw_assert(_CW_KASIO_MAGIC == a_kasio->magic);

  return TRUE; /* XXX */
}
