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

/* XXX Temporary hack. */
#include <termios.h>

/* Refers to a hook that holds a reference to the dynamically loaded module. */
static cw_nxo_t hook_data;

static cw_nxoe_t *
modslate_p_hook_ref_iter(void *a_data, cw_bool_t a_reset)
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
	nxo_name_new(name, nx, a_entries[i].name, strlen(a_entries[i].name),
		     FALSE);
	nxo_hook_new(value, nx, (void *) &hook_data, a_entries[i].eval_f,
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
    if ((name_len != strlen(a_type)) || strncmp(a_type, name, name_len))
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
    cw_nxo_t *estack;
    cw_nxmod_t *nxmod;
    struct termios t; /* XXX Temporary hack. */

    /* The interpreter is currently executing a hook that holds a reference to
     * the dynamically loaded module.  Initialize hook_data to refer to it, then
     * create hooks such that they refer to hook_data.  This prevents the module
     * from being closed until all hooks are gone. */
    estack = nxo_thread_estack_get(a_thread);
    nxo_no_new(&hook_data);
    nxo_dup(&hook_data, nxo_stack_get(estack));

    /* Set the GC iteration for module destruction. */
    nxmod = (cw_nxmod_t *) nxo_hook_data_get(&hook_data);
    nxmod->iter = 2;

    /* Initialize hooks. */
    modslate_buffer_init(a_thread);
    modslate_display_init(a_thread);
    modslate_frame_init(a_thread);

    /* XXX Temporary hack. */
    if (tcgetattr(0, &t))
    {
	fprintf(stderr, "tcgetattr() error\n");
    }
    cfmakeraw(&t);
    if (tcsetattr(0, TCSANOW, &t))
    {
	fprintf(stderr, "tcsetattr() error\n");
    }
}
