/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/modpane.h"

cw_ds_t *
ds_new(cw_ds_t *a_ds, int a_in, int a_out, const char *a_term)
{
    cw_ds_t *retval;

    fprintf(stderr, "%d %d %s\n", a_in, a_out, a_term);

    retval = NULL; /* XXX */

    return retval;
}

void
ds_delete(cw_ds_t *a_ds)
{
}
