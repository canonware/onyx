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
typedef struct cw_kasioe_string_s cw_kasioe_string_t;
#endif

struct cw_kasioe_string_s
{
  cw_kasioe_t kasioe;
};

cw_kasioe_string_t *
kasioe_string_new(cw_kasioe_string_t * a_kasioe_string);

void
kasioe_string_delete(cw_kasioe_string_t * a_kasioe_string);
