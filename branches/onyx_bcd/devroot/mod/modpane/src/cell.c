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

struct cw_cell
{
    /* For GC iteration. */
    cw_uint32_t iter;

    /* Cell. */
    cw_cl_t cl;

    /* Reference to =cell=, prevents module unloading. */
    cw_nxo_t hook;
};

static const struct cw_modpane_entry modpane_cell_hooks[] = {
    MODPANE_ENTRY(cell)
};

void
modpane_cell_init(cw_nxo_t *a_thread)
{
    modpane_hooks_init(a_thread, modpane_cell_hooks,
		       (sizeof(modpane_cell_hooks)
			/ sizeof(struct cw_modpane_entry)));
}

static cw_nxoe_t *
cell_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_cell *cell = (struct cw_cell *) a_data;

    if (a_reset)
    {
	cell->iter = 0;
    }

    switch (cell->iter)
    {
	case 0:
	{
	    retval = nxo_nxoe_get(&cell->hook);
	    break;
	}
	default:
	{
	    retval = NULL;
	}
    }

    cell->iter++;

    return retval;
}

static cw_bool_t
cell_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
    struct cw_cell *cell = (struct cw_cell *) a_data;

    cl_delete(&cell->cl);

    return FALSE;
}

cw_nxn_t
modpane_cell_type(cw_nxo_t *a_nxo)
{
    cw_nxn_t retval;
    cw_nxo_t *tag;
    cw_uint32_t name_len;
    const cw_uint8_t *name;

    if (nxo_type_get(a_nxo) != NXOT_HOOK)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    tag = nxo_hook_tag_get(a_nxo);
    if (nxo_type_get(tag) != NXOT_NAME)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    name_len = nxo_name_len_get(tag);
    name = nxo_name_str_get(tag);
    if ((name_len != sizeof("cell") - 1) || strncmp("cell", name, name_len))
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
modpane_cell(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *nxo, *tag;
    cw_nx_t *nx;
    struct cw_cell *cell;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);

    cell = (struct cw_cell *) nxa_malloc(nx_nxa_get(nx),
					 sizeof(struct cw_cell));

    /* Create a reference to this operator in order to prevent the module from
     * being prematurely unloaded. */
    nxo_no_new(&cell->hook);
    nxo_dup(&cell->hook, nxo_stack_get(estack));

    /* Initialize the cl. */
    cl_new(&cell->cl);

    /* Create a reference to the cell. */
    nxo = nxo_stack_push(ostack);
    nxo_hook_new(nxo, nx, cell, NULL, cell_p_ref_iter, cell_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(nxo);
    nxo_name_new(tag, nx, "cell", sizeof("cell") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);
}
