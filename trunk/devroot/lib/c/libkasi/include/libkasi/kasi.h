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

/* Size of buffer used for large extended objects. */
#define _CW_KASI_BUFC_SIZE 256

typedef struct cw_kasi_s cw_kasi_t;
typedef struct cw_kasi_bufc_s cw_kasi_bufc_t;

struct cw_kasi_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  cw_bool_t is_malloced;
  cw_pezz_t kasi_bufc_pezz;
};

/* Not opaque. */
struct cw_kasi_bufc_s
{
  cw_bufc_t bufc;
  cw_uint8_t buffer[_CW_KASI_BUFC_SIZE];
};

cw_kasi_t *
kasi_new(cw_kasi_t * a_kasi);

void
kasi_delete(cw_kasi_t * a_kasi);

cw_kasi_bufc_t *
kasi_get_kasi_bufc(cw_kasi_t * a_kasi);
