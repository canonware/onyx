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

#include "../include/libstil/libstil.h"
#include "../include/libstil/stilo_array_l.h"
#include "../include/libstil/stilo_boolean_l.h"
#include "../include/libstil/stilo_condition_l.h"
#include "../include/libstil/stilo_dict_l.h"
#include "../include/libstil/stilo_file_l.h"
#include "../include/libstil/stilo_fino_l.h"
#include "../include/libstil/stilo_hook_l.h"
#include "../include/libstil/stilo_integer_l.h"
#include "../include/libstil/stilo_mark_l.h"
#include "../include/libstil/stilo_mutex_l.h"
#include "../include/libstil/stilo_name_l.h"
#include "../include/libstil/stilo_null_l.h"
#include "../include/libstil/stilo_operator_l.h"
#include "../include/libstil/stilo_stack_l.h"
#include "../include/libstil/stilo_string_l.h"
#include "../include/libstil/stilo_thread_l.h"

struct cw_sprintdict_entry {
	cw_stiln_t	stiln;
	cw_op_t		*op_f;
};

#define	ENTRY(name)	{STILN_##name##type, stilo_l_##name##_print}

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
	ENTRY(stack),
	ENTRY(string),
	ENTRY(thread)
};

void
sprintdict_l_populate(cw_stilo_t *a_dict, cw_stil_t *a_stil)
{
	cw_uint32_t	i;
	cw_stilo_t	name, value;

#define	NEXTRA	0
#define NENTRIES							\
	(sizeof(sprintdict_ops) / sizeof(struct cw_sprintdict_entry))

	stilo_dict_new(a_dict, a_stil, TRUE, NENTRIES + NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stil,
		    stiln_str(sprintdict_ops[i].stiln),
		    stiln_len(sprintdict_ops[i].stiln), TRUE);
		stilo_operator_new(&value, sprintdict_ops[i].op_f,
		    sprintdict_ops[i].stiln);
		stilo_attrs_set(&value, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stil, &name, &value);
	}

#ifdef _LIBSTIL_DBG
	if (stilo_dict_count(a_dict) != NENTRIES + NEXTRA) {
		_cw_out_put_e("stilo_dict_count(a_dict) != NENTRIES"
		    " ([i] != [i])\n", stilo_dict_count(a_dict), NENTRIES +
		    NEXTRA);
		_cw_error("Adjust NENTRIES");
	}
#endif
#undef NENTRIES
}
