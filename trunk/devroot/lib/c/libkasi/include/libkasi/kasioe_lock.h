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
typedef struct cw_kasioe_lock_s cw_kasioe_lock_t;
#endif

struct cw_kasioe_lock_s
{
  cw_kasioe_t kasioe;
  cw_mtx_t lock;
};

cw_kasioe_lock_t *
kasioe_lock_new(cw_kasioe_lock_t * a_kasioe_lock);

void
kasioe_lock_delete(cw_kasioe_lock_t * a_kasioe_lock);
