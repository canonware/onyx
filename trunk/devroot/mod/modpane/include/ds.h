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
    /* I/O file descriptors. */
    int in;
    int out;

    /* Terminal name. */
    cw_uint8_t *term;

    /* Original terminal settings. */
    struct termios term_saved;
};

cw_ds_t *
ds_new(cw_ds_t *a_ds, int a_in, int a_out, const cw_uint8_t *a_term);

void
ds_delete(cw_ds_t *a_ds);
