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

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxo_array_l.h"
#include "../include/libonyx/nxo_boolean_l.h"
#include "../include/libonyx/nxo_condition_l.h"
#include "../include/libonyx/nxo_dict_l.h"
#include "../include/libonyx/nxo_file_l.h"
#include "../include/libonyx/nxo_fino_l.h"
#include "../include/libonyx/nxo_hook_l.h"
#include "../include/libonyx/nxo_integer_l.h"
#include "../include/libonyx/nxo_mark_l.h"
#include "../include/libonyx/nxo_mutex_l.h"
#include "../include/libonyx/nxo_name_l.h"
#include "../include/libonyx/nxo_null_l.h"
#include "../include/libonyx/nxo_operator_l.h"
#include "../include/libonyx/nxo_pmark_l.h"
#include "../include/libonyx/nxo_stack_l.h"
#include "../include/libonyx/nxo_string_l.h"
#include "../include/libonyx/nxo_thread_l.h"

struct cw_sprintdict_entry {
	cw_nxn_t	nxn;
	cw_op_t		*op_f;
};

#define	ENTRY(name)	{NXN_##name##type, nxo_l_##name##_print}

/*
 * Array of operators in sprintdict.
 */
static const struct cw_sprintdict_entry sprintdict_ops[] = {
	ENTRY(array),
	ENTRY(boolean),
	ENTRY(condition),
	ENTRY(dict),
	ENTRY(file),
	ENTRY(fino),
	ENTRY(hook),
	ENTRY(integer),
	ENTRY(mark),
	ENTRY(mutex),
	ENTRY(name),
	ENTRY(null),
	ENTRY(operator),
	ENTRY(pmark),
	ENTRY(stack),
	ENTRY(string),
	ENTRY(thread)
};

void
sprintdict_l_populate(cw_nxo_t *a_dict, cw_nx_t *a_nx)
{
	cw_uint32_t	i;
	cw_nxo_t	name, value;

#define	NEXTRA	0
#define NENTRIES							\
	(sizeof(sprintdict_ops) / sizeof(struct cw_sprintdict_entry))

	nxo_dict_new(a_dict, a_nx, TRUE, NENTRIES + NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		nxo_name_new(&name, a_nx,
		    nxn_str(sprintdict_ops[i].nxn),
		    nxn_len(sprintdict_ops[i].nxn), TRUE);
		nxo_operator_new(&value, sprintdict_ops[i].op_f,
		    sprintdict_ops[i].nxn);
		nxo_attr_set(&value, NXOA_EXECUTABLE);

		nxo_dict_def(a_dict, a_nx, &name, &value);
	}

#ifdef _CW_DBG
	if (nxo_dict_count(a_dict) != NENTRIES + NEXTRA) {
		_cw_out_put_e("nxo_dict_count(a_dict) != NENTRIES"
		    " ([i] != [i])\n", nxo_dict_count(a_dict), NENTRIES +
		    NEXTRA);
		_cw_error("Adjust NENTRIES");
	}
#endif
#undef NENTRIES
}
