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
typedef struct cw_kasioe_condition_s cw_kasioe_condition_t;
#endif

struct cw_kasioe_condition_s
{
  cw_kasioe_t kasioe;
  cw_cnd_t condition;
};

cw_kasioe_condition_t *
kasioe_condition_new(cw_kasioe_condition_t * a_kasioe_condition);

void
kasioe_condition_ref(cw_kasioe_condition_t * a_kasioe_condition);

void
kasioe_condition_unref(cw_kasioe_condition_t * a_kasioe_condition);

