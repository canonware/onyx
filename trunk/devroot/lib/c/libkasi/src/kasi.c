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

cw_kasi_t *
kasi_new(cw_kasi_t * a_kasi)
{
  return NULL; /* XXX */
}

void
kasi_delete(cw_kasi_t * a_kasi)
{
}

cw_kasi_bufc_t *
kasi_get_kasi_bufc(cw_kasi_t * a_kasi)
{
  cw_kasi_bufc_t * retval;

  /* XXX Check return. */
  retval = (cw_kasi_bufc_t *) _cw_pezz_get(&a_kasi->kasi_bufc_pezz);
  bufc_new(&retval->bufc, pezz_put, &a_kasi->kasi_bufc_pezz);
  bzero(retval->buffer, sizeof(retval->buffer));
  bufc_set_buffer(&retval->bufc, retval->buffer, _CW_KASI_BUFC_SIZE, TRUE,
		  NULL, NULL);

  return retval;
}

