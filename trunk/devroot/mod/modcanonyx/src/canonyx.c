/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "modcanonyx.h"

#define	ENTRY(name)	{#name, canonyx_##name}

struct cw_canonyx_entry {
	const cw_uint8_t	*op_n;
	cw_op_t			*op_f;
};

static const struct cw_canonyx_entry canonyx_ops[] = {
	ENTRY(buffer)
};

void
canonyx_init(void *a_arg, cw_nxo_t *a_thread)
{
	cw_nxo_t	*tstack;
	cw_nxo_t	*globaldict, *name, *value;
	cw_nx_t		*nx;
	cw_uint32_t	i;

#define	NENTRIES (sizeof(canonyx_ops) / sizeof(struct cw_canonyx_entry))

	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);
	globaldict = nx_globaldict_get(nx);

	name = nxo_stack_push(tstack);
	value = nxo_stack_push(tstack);

	for (i = 0; i < NENTRIES; i++) {
		nxo_name_new(name, nx, canonyx_ops[i].op_n,
		    strlen(canonyx_ops[i].op_n), FALSE);
		nxo_operator_new(value, canonyx_ops[i].op_f, NXN_ZERO);
		nxo_attr_set(value, NXOA_EXECUTABLE);

		nxo_dict_def(globaldict, nx, name, value);
	}

	nxo_stack_npop(tstack, 2);
}
