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

typedef struct cw_ds_s cw_ds_t;

struct cw_ds_s
{
    /* XXX Think about having function pointers to abstract files away. */
    /* I/O file descriptors. */
    int in;
    int out;

};

cw_ds_t *
ds_new(cw_ds_t *a_ds, int a_in, int a_out, const char *a_term);

void
ds_delete(cw_ds_t *a_ds);
