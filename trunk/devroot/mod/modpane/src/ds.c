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

#include "../include/modpane.h"

void
ds_new(cw_ds_t *a_ds, cw_opaque_alloc_t *a_alloc,
       cw_opaque_realloc_t *a_realloc, cw_opaque_dealloc_t *a_dealloc,
       void *a_arg, int a_in, int a_out)
{
    cw_error("XXX Not implemented");
}

void
ds_delete(cw_ds_t *a_ds)
{
    cw_error("XXX Not implemented");
}


cw_bool_t
ds_size(cw_ds_t *a_ds, cw_uint32_t *r_x, cw_uint32_t *r_y)
{
    cw_error("XXX Not implemented");

    return TRUE; /* XXX */
}
