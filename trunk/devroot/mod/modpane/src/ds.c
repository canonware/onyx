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
    a_ds->in = a_in;
    a_ds->out = a_out;

    a_ds->started = FALSE;

#ifdef CW_DBG
    a_ds->magic = CW_DS_MAGIC;
#endif
}

void
ds_delete(cw_ds_t *a_ds)
{
    if (a_ds->started)
    {
	/* XXX Check for error? */
	ds_stop(a_ds);
    }
#ifdef CW_DBG
    a_ds->magic = 0;
#endif
}

cw_bool_t
ds_size(cw_ds_t *a_ds, cw_uint32_t *r_x, cw_uint32_t *r_y)
{
    cw_error("XXX Not implemented");

    return TRUE; /* XXX */
}

cw_bool_t
ds_start(cw_ds_t *a_ds)
{
    cw_bool_t retval;
    struct termios t;

    if (a_ds->started)
    {
	retval = TRUE;
	goto RETURN;
    }

    if (tcgetattr(0, &a_ds->termio))
    {
	retval = TRUE;
	goto RETURN;
    }
    t = a_ds->termio;
    cfmakeraw(&t);
    if (tcsetattr(0, TCSANOW, &t))
    {
	retval = TRUE;
	goto RETURN;
    }

    a_ds->started = TRUE;

    retval = FALSE;
    RETURN:
    return retval;
}

cw_bool_t
ds_stop(cw_ds_t *a_ds)
{
    cw_bool_t retval;

    if (a_ds->started == FALSE)
    {
	retval = TRUE;
	goto RETURN;
    }

    if (tcsetattr(0, TCSANOW, &a_ds->termio))
    {
	retval = TRUE;
	goto RETURN;
    }

    a_ds->started = FALSE;

    retval = FALSE;
    RETURN:
    return retval;
}
