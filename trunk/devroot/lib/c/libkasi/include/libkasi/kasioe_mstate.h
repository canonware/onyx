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

/* Defined in kasio.h to resolve a circular dependency. */
#if (0)
typedef struct cw_kasioe_mstate_s cw_kasioe_mstate_t;
#endif

struct cw_kasioe_mstate_s
{
  cw_kasioe_t kasioe;
  cw_uint32_t accuracy;
  cw_uint32_t point;
  cw_uint32_t base;
};

cw_kasioe_mstate_t *
kasioe_mstate_new(cw_kasioe_mstate_t * a_kasioe_mstate);

void
kasioe_mstate_delete(cw_kasioe_mstate_t * a_kasioe_mstate);
