/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Slate <Version = slate>
 *
 ******************************************************************************/

#include "../include/modslate.h"

/* Refers to a hook that holds a reference to the dynamically loaded module. */
static cw_nxo_t modslate_module_hook;

/* Code funnel for curses API calls. */
cw_nxo_t modslate_curses_funnel;

/* Reference iterator function used for modslate hooks created via
 * modslate_hooks_init().  This function makes sure that modslate_module_hook
 * and modslate_curses_funnal are not deleted until there are no more hooks. */
static cw_nxoe_t *
modslate_p_hook_ref_iter(void *a_data, cw_bool_t a_reset)
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
		retval = nxo_nxoe_get(&modslate_module_hook);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&modslate_curses_funnel);
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
modslate_hooks_init(cw_nxo_t *a_thread,
		    const struct cw_modslate_entry *a_entries,
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
	nxo_name_new(name, nx, a_entries[i].name,
		     strlen((char *) a_entries[i].name), FALSE);
	nxo_hook_new(value, nx, NULL, a_entries[i].eval_f,
		     modslate_p_hook_ref_iter, NULL);
	nxo_dup(nxo_hook_tag_get(value), name);
	nxo_attr_set(value, NXOA_EXECUTABLE);

	nxo_dict_def(currentdict, nx, name, value);
    }

    nxo_stack_npop(tstack, 2);
}

/* Verify that a_nxo is a =a_type=. */
cw_nxn_t
modslate_hook_type(cw_nxo_t *a_hook, const cw_uint8_t *a_type)
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
modslate_hook_p(void *a_data, cw_nxo_t *a_thread, const cw_uint8_t *a_type)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, a_type);

    nxo_boolean_new(nxo, error ? FALSE : TRUE);
}

void
modslate_init(void *a_arg, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack;
    cw_nxo_t *currentdict, *name, *value;
    cw_nx_t *nx;
    cw_nxmod_t *nxmod;

    /* The interpreter is currently executing a hook that holds a reference to
     * the dynamically loaded module.  Initialize modslate_module_hook to refer
     * to it, then create hooks such that they refer to modslate_module_hook.
     * This prevents the module from being closed until all hooks are gone. */
    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    nxo_no_new(&modslate_module_hook);
    nxo_dup(&modslate_module_hook, nxo_stack_get(estack));

    /* Set the GC iteration for module destruction. */
    nxmod = (cw_nxmod_t *) nxo_hook_data_get(&modslate_module_hook);
    nxmod->iter = MODSLATE_GC_ITER_MODULE;

    /* Initialize hooks. */
    modslate_buffer_init(a_thread);
    modslate_display_init(a_thread);
    modslate_frame_init(a_thread);
    modslate_window_init(a_thread);
    modslate_funnel_init(a_thread);

    /* Initialize curses_funnel such that it is usable both in Onyx code and C
     * code. */
    currentdict = nxo_stack_get(nxo_thread_dstack_get(a_thread));
    name = nxo_stack_push(ostack);
    nxo_name_new(name, nx, "curses_funnel", sizeof("curses_funnel") - 1, FALSE);
    modslate_funnel(NULL, a_thread);
    value = nxo_stack_get(ostack);
    nxo_dict_def(currentdict, nx, name, value);
    nxo_no_new(&modslate_curses_funnel);
    nxo_dup(&modslate_curses_funnel, value);
    nxo_stack_npop(ostack, 2);
}
