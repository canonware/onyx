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

#include "../include/libonyx/libonyx.h"

#define NAME_anon_hook "anon_hook"
#define NAME_mark_hook "mark_hook"

const cw_uint8_t *data = "hook data";

void
hook_eval(void *a_data, cw_nxo_t *a_thread)
{
    cw_assert(a_data == data);

    cw_onyx_code(a_thread, "`Evaluated hook\n' print");
}

cw_nxoe_t *
hook_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_assert(a_data == data);

    return NULL;
}

cw_bool_t
hook_delete(void *a_data, cw_uint32_t a_iter)
{
    cw_assert(a_data == data);

    return FALSE;
}

void
anon_hook(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *hook;

    ostack = nxo_thread_ostack_get(a_thread);
    hook = nxo_stack_push(ostack);
    nxo_hook_new(hook, (void *) data, hook_eval, hook_ref_iter, hook_delete);
    nxo_attr_set(hook, NXOA_EXECUTABLE);
}

void
mark_hook(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *hook, *tag;

    anon_hook(a_thread);

    ostack = nxo_thread_ostack_get(a_thread);
    hook = nxo_stack_get(ostack);
    tag = nxo_hook_tag_get(hook);
    nxo_mark_new(tag);
}

int
main(int argc, char **argv, char **envp)
{
    cw_nx_t nx;
    cw_nxo_t thread, *ostack, *name, *operator;

    libonyx_init(argc, argv, envp);
    fprintf(stderr, "Test begin\n");

    cw_assert(nx_new(&nx, NULL) == &nx);
    nxo_thread_new(&thread, &nx);

    /* Define anon_hook and mark_hook. */
    ostack = nxo_thread_ostack_get(&thread);

    name = nxo_stack_push(ostack);
    operator = nxo_stack_push(ostack);
    nxo_name_new(name, NAME_anon_hook, strlen(NAME_anon_hook), FALSE);
    nxo_operator_new(operator, anon_hook, NXN_ZERO);
    nxo_attr_set(operator, NXOA_EXECUTABLE);
    cw_onyx_code(&thread, "def");

    name = nxo_stack_push(ostack);
    operator = nxo_stack_push(ostack);
    nxo_name_new(name, NAME_mark_hook, strlen(NAME_mark_hook), FALSE);
    nxo_operator_new(operator, mark_hook, NXN_ZERO);
    nxo_attr_set(operator, NXOA_EXECUTABLE);
    cw_onyx_code(&thread, "def");

    cw_onyx_code(&thread, "\n\
anon_hook 1 sprint\n\
anon_hook eval\n\
mark_hook 1 sprint\n\
mark_hook eval\n\
");
    nx_delete(&nx);

    libonyx_shutdown();
    fprintf(stderr, "Test end\n");
    return 0;
}
