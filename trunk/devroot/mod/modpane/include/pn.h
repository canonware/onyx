/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

typedef struct cw_pn_s cw_pn_t;

struct cw_pn_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_PN_MAGIC 0x28393254
#endif
};

void
pn_new(cw_pn_t *a_pn, cw_ds_t *a_ds);

void
pn_delete(cw_pn_t *a_pn);

cw_ds_t *
pn_ds_get(cw_pn_t *a_pn);

cw_bool_t
pn_size(cw_pn_t *a_pn, cw_uint32_t *r_x, cw_uint32_t *r_y);
