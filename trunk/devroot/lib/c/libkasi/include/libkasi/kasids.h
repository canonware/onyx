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

typedef struct cw_kasids_s cw_kasids_t;

struct cw_kasids_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  cw_kasis_t stack;

  cw_ch_t hash;
};

struct cw_kasidsi_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  cw_kasio_t name;

  /* Linkage into a ring of kasido's that all have the same name. */
  cw_ring_t link;
};
