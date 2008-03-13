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

#define NAME_anon_handle "anon_handle"
#define NAME_mark_handle "mark_handle"

const char *data = "handle data";

void
handle_eval(void *a_data, cw_nxo_t *a_thread)
{
    cw_assert(a_data == data);

    cw_onyx_code(a_thread, "`Evaluated handle\n' print");
}

cw_nxoe_t *
handle_ref_iter(void *a_data, bool a_reset)
{
    cw_assert(a_data == data);

    return NULL;
}

bool
handle_delete(void *a_data, uint32_t a_iter)
{
    cw_assert(a_data == data);

    return false;
}

void
anon_handle(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *handle;

    ostack = nxo_thread_ostack_get(a_thread);
    handle = nxo_stack_push(ostack);
    nxo_handle_new(handle, (void *) data, handle_eval, handle_ref_iter,
		   handle_delete);
    nxo_attr_set(handle, NXOA_EXECUTABLE);
}

void
mark_handle(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *handle, *tag;

    anon_handle(a_thread);

    ostack = nxo_thread_ostack_get(a_thread);
    handle = nxo_stack_get(ostack);
    tag = nxo_handle_tag_get(handle);
    nxo_mark_new(tag);
}

int
main(int argc, char **argv, char **envp)
{
    cw_nx_t nx;
    cw_nxo_t thread, *ostack, *name, *operator;

    libonyx_init(argc, argv, envp);
    fprintf(stderr, "Test begin\n");

    cw_assert(nx_new(&nx, NULL, NULL) == &nx);
    nxo_thread_new(&thread, &nx);

    /* Define anon_handle and mark_handle. */
    ostack = nxo_thread_ostack_get(&thread);

    name = nxo_stack_push(ostack);
    operator = nxo_stack_push(ostack);
    nxo_name_new(name, NAME_anon_handle, strlen(NAME_anon_handle), false);
    nxo_operator_new(operator, anon_handle, NXN_ZERO);
    nxo_attr_set(operator, NXOA_EXECUTABLE);
    cw_onyx_code(&thread, "def");

    name = nxo_stack_push(ostack);
    operator = nxo_stack_push(ostack);
    nxo_name_new(name, NAME_mark_handle, strlen(NAME_mark_handle), false);
    nxo_operator_new(operator, mark_handle, NXN_ZERO);
    nxo_attr_set(operator, NXOA_EXECUTABLE);
    cw_onyx_code(&thread, "def");

    cw_onyx_code(&thread, "\n\
anon_handle 1 sprint\n\
anon_handle eval\n\
mark_handle 1 sprint\n\
mark_handle eval\n\
");
    nx_delete(&nx);

    libonyx_shutdown();
    fprintf(stderr, "Test end\n");
    return 0;
}
