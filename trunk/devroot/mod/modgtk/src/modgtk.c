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

#include "../include/modgtk.h"

/* Refers to a hook that holds a reference to the dynamically loaded module. */
static cw_nxo_t hook_data;

static cw_nxoe_t *
modgtk_p_hook_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxo_t *hook = (cw_nxo_t *) a_data;

    if (a_reset)
    {
	retval = nxo_nxoe_get(hook);
    }
    else
    {
	retval = NULL;
    }

    return retval;
}

void
modgtk_hooks_init(cw_nxo_t *a_thread, const struct cw_modgtk_entry *a_entries,
		  cw_uint32_t a_nentries)
{
    cw_nxo_t *tstack;
    cw_nxo_t *currentdict, *name, *value;
    cw_nx_t *nx;
    cw_uint32_t i;

    tstack = nxo_thread_tstack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    currentdict = nxo_stack_get(nxo_thread_dstack_get(a_thread));

    name = nxo_stack_push(tstack);
    value = nxo_stack_push(tstack);

    for (i = 0; i < a_nentries; i++)
    {
	nxo_name_new(name, nx, a_entries[i].name, strlen(a_entries[i].name),
		     FALSE);
	nxo_hook_new(value, nx, (void *) &hook_data, a_entries[i].eval_f,
		     modgtk_p_hook_ref_iter, NULL);
	nxo_dup(nxo_hook_tag_get(value), name);
	nxo_attr_set(value, NXOA_EXECUTABLE);

	nxo_dict_def(currentdict, nx, name, value);
    }

    nxo_stack_npop(tstack, 2);
}

void
modgtk_init(void *a_arg, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack;

    /* The interpreter is currently executing a hook that holds a reference to
     * the dynamically loaded module.  Initialize hook_data to refer to it, then
     * create hooks such that they refer to hook_data.  This prevents the module
     * from being closed until all hooks are gone. */
    estack = nxo_thread_estack_get(a_thread);
    nxo_no_new(&hook_data);
    nxo_dup(&hook_data, nxo_stack_get(estack));

    /* Initialize hooks. */
}
