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

/* Refers to a handle that holds a reference to the dynamically loaded
 * module. */
static cw_nxo_t modpane_module_handle;

/* Reference iterator function used for modpane classes/handles created via
 * modpane_class_init().  This function makes sure that modpane_module_handle
 * is not deleted until there are no more classes/handles. */
static cw_nxoe_t *
modpane_p_ref_iter(void *a_data, cw_bool_t a_reset)
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
		retval = nxo_nxoe_get(&modpane_module_handle);
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
modpane_class_init(cw_nxo_t *a_thread, const cw_uint8_t *a_name,
		   const struct cw_modpane_method *a_methods,
		   cw_uint32_t a_nmethods, void *a_opaque, cw_nxo_t *r_class)
{
    cw_nxo_t *tstack, *classname, *class_;
    cw_nxo_t *data, *methods;
    cw_nxo_t *name, *value;
    cw_bool_t currentlocking;
    cw_uint32_t i;

    tstack = nxo_thread_tstack_get(a_thread);
    currentlocking = nxo_thread_currentlocking(a_thread);

    /* Create the class name. */
    classname = nxo_stack_push(tstack);
    nxo_name_new(classname, a_name, strlen(a_name), FALSE);

    /* Create a class. */
    class_ = nxo_stack_push(tstack);
    nxo_class_new(class_, a_opaque, modpane_p_ref_iter, NULL);

    /* Set the class's name. */
    nxo_dup(nxo_class_name_get(class_), classname);

    /* Create data dict. */
    data = nxo_class_data_get(class_);
    nxo_dict_new(data, currentlocking, 0);

    /* Create methods dict. */
    methods = nxo_class_methods_get(class_);
    nxo_dict_new(methods, currentlocking, a_nmethods);

    /* Populate methods dict. */
    name = nxo_stack_push(tstack);
    value = nxo_stack_push(tstack);
    for (i = 0; i < a_nmethods; i++)
    {
	nxo_name_new(name, a_methods[i].name,
		     strlen((char *) a_methods[i].name), FALSE);
	nxo_handle_new(value, NULL, a_methods[i].eval_f,
		       modpane_p_ref_iter, NULL);
	nxo_dup(nxo_handle_tag_get(value), name);
	nxo_attr_set(value, NXOA_EXECUTABLE);

	nxo_dict_def(methods, name, value);
    }

    /* Define class in currentdict. */
    nxo_dict_def(nxo_stack_get(nxo_thread_dstack_get(a_thread)),
		 classname, class_);

    nxo_dup(r_class, class_);

    /* Clean up. */
    nxo_stack_npop(tstack, 4);
}

/* Verify that a_nxo has a_class in its inheritance hierarchy. */
cw_nxn_t
modpane_instance_kind(cw_nxo_t *a_instance, cw_nxo_t *a_class)
{
    cw_nxn_t retval;
    cw_nxo_t *tclass;

    cw_assert(nxo_type_get(a_instance) == NXOT_INSTANCE);

    /* Iterate up the inheritance chain until class_ is found, or the baseclass
     * is reached. */
    for (tclass = nxo_instance_isa_get(a_instance);
	 nxo_type_get(tclass) == NXOT_CLASS;
	 tclass = nxo_class_super_get(tclass))
    {
	if (nxo_compare(a_class, tclass) == 0)
	{
	    /* Found. */
	    retval = NXN_ZERO;
	    goto RETURN;
	}
    }

    /* Not found. */
    retval = NXN_typecheck;
    RETURN:
    return retval;
}

void
modpane_init(void *a_arg, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack;
    cw_nxmod_t *nxmod;

    /* The interpreter is currently executing a handle that holds a reference to
     * the dynamically loaded module.  Initialize modpane_module_handle to refer
     * to it, then create handles such that they refer to modpane_module_handle.
     * This prevents the module from being closed until all handles are gone. */
    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    nxo_no_new(&modpane_module_handle);
    nxo_dup(&modpane_module_handle, nxo_stack_get(estack));

    /* Set the GC iteration for module destruction. */
    nxmod = (cw_nxmod_t *) nxo_handle_opaque_get(&modpane_module_handle);
    nxmod->iter = MODPANE_GC_ITER_MODULE;

    /* Initialize handles. */
    modpane_display_init(a_thread);
    modpane_pane_init(a_thread);
}
