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

typedef struct cw_ds_s cw_ds_t;

struct cw_ds_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_DS_MAGIC 0x9832d754
#endif
};

void
ds_new(cw_ds_t *a_ds, cw_opaque_alloc_t *a_alloc,
       cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc,
       void *a_arg, int a_in, int a_out);

void
ds_delete(cw_ds_t *a_ds);

cw_bool_t
ds_size(cw_ds_t *a_ds, cw_uint32_t *r_x, cw_uint32_t *r_y);
