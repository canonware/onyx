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

#include "../include/slate.h"

void
slate_ops(cw_nxo_t *a_thread, const struct cw_slate_entry *a_entries,
	  cw_uint32_t a_nentries)
{
    cw_nxo_t *tstack;
    cw_nxo_t *globaldict, *name, *value;
    cw_nx_t *nx;
    cw_uint32_t i;

    tstack = nxo_thread_tstack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    globaldict = nx_globaldict_get(nx);

    name = nxo_stack_push(tstack);
    value = nxo_stack_push(tstack);

    for (i = 0; i < a_nentries; i++)
    {
	nxo_name_new(name, nx, a_entries[i].name, strlen(a_entries[i].name),
		     FALSE);
	nxo_operator_new(value, a_entries[i].op_f, NXN_ZERO);

	nxo_dict_def(globaldict, nx, name, value);
    }

    nxo_stack_npop(tstack, 2);
}

void
slate_ops_init(cw_nxo_t *a_thread)
{
    slate_buffer_init(a_thread);
}
