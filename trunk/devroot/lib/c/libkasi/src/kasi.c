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
#  define _CW_KASI_MAGIC 0xae9678fd
#endif

/* Number of kasi_bufc structures to allocate at a time via the pezz code. */
#define _CW_KASI_BUFC_CHUNK_COUNT 16

cw_kasi_t *
kasi_new(cw_kasi_t * a_kasi)
{
  cw_kasi_t * retval;

  if (NULL != a_kasi)
  {
    retval = a_kasi;
    bzero(retval, sizeof(cw_kasi_t));
    retval->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_kasi_t *) _cw_malloc(sizeof(cw_kasi_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_kasi_t));
    retval->is_malloced = TRUE;
  }

  if (NULL == pezz_new(&retval->kasi_bufc_pezz, sizeof(cw_kasi_bufc_t),
		       _CW_KASI_BUFC_CHUNK_COUNT))
  {
    if (TRUE == retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }

#ifdef _LIBKASI_DBG
  retval->magic = _CW_KASI_MAGIC;
#endif

  RETURN:
  return retval;
}

void
kasi_delete(cw_kasi_t * a_kasi)
{
  _cw_check_ptr(a_kasi);
  _cw_assert(_CW_KASI_MAGIC == a_kasi->magic);

  pezz_delete(&a_kasi->kasi_bufc_pezz);

  if (TRUE == a_kasi->is_malloced)
  {
    _cw_free(a_kasi);
  }
#ifdef _LIBKASI_DBG
  else
  {
    memset(a_kasi, 0x5a, sizeof(cw_kasi_t));
  }
#endif
}

cw_kasi_bufc_t *
kasi_get_kasi_bufc(cw_kasi_t * a_kasi)
{
  cw_kasi_bufc_t * retval;

  _cw_check_ptr(a_kasi);
  _cw_assert(_CW_KASI_MAGIC == a_kasi->magic);
  
  retval = (cw_kasi_bufc_t *) _cw_pezz_get(&a_kasi->kasi_bufc_pezz);
  if (NULL == retval)
  {
    goto RETURN;
  }
  bufc_new(&retval->bufc, pezz_put, &a_kasi->kasi_bufc_pezz);
  bzero(retval->buffer, sizeof(retval->buffer));
  bufc_set_buffer(&retval->bufc, retval->buffer, _CW_KASI_BUFC_SIZE, TRUE,
		  NULL, NULL);

  RETURN:
  return retval;
}

