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
typedef struct cw_kasioe_packedarray_s cw_kasioe_packedarray_t;
#endif

struct cw_kasioe_packedarray_s
{
  cw_kasioe_t kasioe;
};

cw_kasioe_packedarray_t *
kasioe_packedarray_new(cw_kasioe_packedarray_t * a_kasioe_packedarray);

void
kasioe_packedarray_delete(cw_kasioe_packedarray_t * a_kasioe_packedarray);
