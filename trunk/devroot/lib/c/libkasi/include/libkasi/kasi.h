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

typedef struct cw_kasi_s cw_kasi_t;

struct cw_kasi_s
{
  int garbage;
};

cw_kasi_t *
kasi_new(cw_kasi_t * a_kasi);

void
kasi_delete(cw_kasi_t * a_kasi);
