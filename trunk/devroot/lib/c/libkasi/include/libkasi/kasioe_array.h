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
typedef struct cw_kasioe_array_s cw_kasioe_array_t;
#endif

struct cw_kasioe_array_s
{
  cw_kasioe_t kasioe;
};

cw_kasioe_array_t *
kasioe_array_new(cw_kasioe_array_t * a_kasioe_array);

void
kasioe_array_delete(cw_kasioe_array_t * a_kasioe_array);
