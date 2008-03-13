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

typedef struct cw_pn_s cw_pn_t;

struct cw_pn_s
{
    /* Top level display object. */
    cw_ds_t *ds;

    /* Parent pane.  If NULL, this is the top level pane for disp. */
    cw_pn_t *parent;

    /* Ordered (bottom to top) list of children. */
    ql_head(cw_pn_t) children;

    /* Sibling linkage. */
    ql_elm(cw_pn_t) link;

    /* If a top level pane, the actual data array, in row-major form. */
    cw_cl_t *cells;

    /* Position of top left corner, relative to the parent's top left corner. */
    cw_nxoi_t xpos;
    cw_nxoi_t ypos;

    /* Virtual size.  The actual size is constrained by the borders of the
     * parent. */
    cw_nxoi_t xsize;
    cw_nxoi_t ysize;
};

cw_pn_t *
pn_new(cw_pn_t *a_pn, cw_ds_t *a_ds, cw_pn_t *a_parent);

void
pn_delete(cw_pn_t *a_pn);

cw_pn_t *
pn_parent(cw_pn_t *a_pn);
