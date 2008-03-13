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

cw_pn_t *
pn_new(cw_pn_t *a_pn, cw_ds_t *a_ds, cw_pn_t *a_parent)
{
    cw_pn_t *retval;

    retval = NULL; /* XXX */

    return retval;
}

void
pn_delete(cw_pn_t *a_pn)
{
}

cw_pn_t *
pn_parent(cw_pn_t *a_pn)
{
    return a_pn->parent;
}
