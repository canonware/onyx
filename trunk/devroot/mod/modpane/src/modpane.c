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

/* Refers to a hook that holds a reference to the dynamically loaded module. */
static cw_nxo_t modpane_module_hook;

/* Reference iterator function used for modpane hooks created via
 * modpane_hooks_init().  This function makes sure that modpane_module_hook is
 * not deleted until there are no more hooks. */
static cw_nxoe_t *
modpane_p_hook_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    static cw_uint32_t iter;

    if (a_reset)
    {
	iter = 0;
    }

    for (retval = NULL; retval == NULL; iter++)
    {
	switch (iter)
	{
	    case 0:
	    {
		retval = nxo_nxoe_get(&modpane_module_hook);
		break;
	    }
	    default:
	    {
		retval = NULL;
		goto RETURN;
	    }
	}
    }

    RETURN:
    return retval;
}

void
modpane_hooks_init(cw_nxo_t *a_thread,
		   const struct cw_modpane_entry *a_entries,
		   cw_uint32_t a_nentries)
{
    cw_nxo_t *tstack;
    cw_nxo_t *currentdict, *name, *value;
    cw_uint32_t i;

    tstack = nxo_thread_tstack_get(a_thread);
    currentdict = nxo_stack_get(nxo_thread_dstack_get(a_thread));

    name = nxo_stack_push(tstack);
    value = nxo_stack_push(tstack);

    for (i = 0; i < a_nentries; i++)
    {
	nxo_name_new(name, a_entries[i].name,
		     strlen((char *) a_entries[i].name), FALSE);
	nxo_hook_new(value, NULL, a_entries[i].eval_f, modpane_p_hook_ref_iter,
		     NULL);
	nxo_dup(nxo_hook_tag_get(value), name);
	nxo_attr_set(value, NXOA_EXECUTABLE);

	nxo_dict_def(currentdict, name, value);
    }

    nxo_stack_npop(tstack, 2);
}

/* Verify that a_nxo is a =a_type=. */
cw_nxn_t
modpane_hook_type(cw_nxo_t *a_hook, const cw_uint8_t *a_type)
{
    cw_nxn_t retval;
    cw_nxo_t *tag;
    cw_uint32_t name_len;
    const cw_uint8_t *name;

    if (nxo_type_get(a_hook) != NXOT_HOOK)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    tag = nxo_hook_tag_get(a_hook);
    if (nxo_type_get(tag) != NXOT_NAME)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    name_len = nxo_name_len_get(tag);
    name = nxo_name_str_get(tag);
    if ((name_len != strlen((char *) a_type))
	|| strncmp(a_type, name, name_len))
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}


/* #object a_type? #boolean */
void
modpane_hook_p(void *a_data, cw_nxo_t *a_thread, const cw_uint8_t *a_type)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modpane_hook_type(nxo, a_type);

    nxo_boolean_new(nxo, error ? FALSE : TRUE);
}

void
modpane_init(void *a_arg, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack;
    cw_nxmod_t *nxmod;

    /* The interpreter is currently executing a hook that holds a reference to
     * the dynamically loaded module.  Initialize modpane_module_hook to refer
     * to it, then create hooks such that they refer to modpane_module_hook.
     * This prevents the module from being closed until all hooks are gone. */
    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    nxo_no_new(&modpane_module_hook);
    nxo_dup(&modpane_module_hook, nxo_stack_get(estack));

    /* Set the GC iteration for module destruction. */
    nxmod = (cw_nxmod_t *) nxo_hook_data_get(&modpane_module_hook);
    nxmod->iter = MODPANE_GC_ITER_MODULE;

    /* Initialize hooks. */
    modpane_display_init(a_thread);
    modpane_pane_init(a_thread);
}
