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
#  define _CW_KASIS_MAGIC 0x0ea67890
#endif

cw_kasis_t *
kasis_new(cw_kasis_t * a_kasis, cw_pezz_t * a_chunk_pezz)
{
  cw_kasis_t * retval;

  _cw_check_ptr(a_kasis);
  _cw_check_ptr(a_chunk_pezz);

  

  return retval;
}

void
kasis_delete(cw_kasis_t * a_kasis)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
  
}

void
kasis_collect(cw_kasis_t * a_kasis,
	      void (*a_add_root_func)(void * add_root_arg, cw_kasioe_t * root),
	      void * a_add_root_arg)
{
}

cw_kasio_t *
kasis_push(cw_kasis_t * a_kasis)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}

cw_bool_t
kasis_pop(cw_kasis_t * a_kasis, cw_uint32_t a_count)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}

cw_bool_t
kasis_roll(cw_kasis_t * a_kasis, cw_sint32_t a_count)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}

cw_bool_t
kasis_dup(cw_kasis_t * a_kasis, cw_uint32_t a_count, cw_uint32_t a_index)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}

cw_uint32_t
kasis_count(cw_kasis_t * a_kasis)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}

cw_kasio_t *
kasis_get(cw_kasis_t * a_kasis, cw_uint32_t a_index)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}

cw_kasio_t *
kasis_get_down(cw_kasis_t * a_kasis, cw_kasio_t * a_kasio)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}

cw_kasio_t *
kasis_get_up(cw_kasis_t * a_kasis, cw_kasio_t * a_kasio)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}

cw_bool_t
kasis_get_index(cw_kasis_t * a_kasis, cw_kasio_t * a_kasio)
{
  _cw_check_ptr(a_kasis);
  _cw_assert(_CW_KASIS_MAGIC == a_kasis->magic);
}
