/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstil/libstil.h"

struct cw_threaddict_entry {
	const cw_uint8_t	*name;
	cw_op_t			*op_f;
};

#define soft_operator(a_str) do {					\
	cw_stilts_t	stilts;						\
	cw_uint8_t	code[] = (a_str);				\
									\
	stilts_new(&stilts, a_stilt);					\
	stilt_interpret(a_stilt, &stilts, code, sizeof(code) - 1);	\
	stilt_flush(a_stilt, &stilts);					\
	stilts_delete(&stilts, a_stilt);				\
} while (0)

#define _THREADDICT_ENTRY(name)	{#name, threaddict_##name}

/*
 * Array of operators in threaddict.
 */
static struct cw_threaddict_entry threaddict_ops[] = {
	_THREADDICT_ENTRY(errordict),
	_THREADDICT_ENTRY(internaldict),
	_THREADDICT_ENTRY(localinstancedict),
	_THREADDICT_ENTRY(serverdict),
	_THREADDICT_ENTRY(statusdict),
	{"$error",	threaddict_sym_derror},
	_THREADDICT_ENTRY(userdict),
	_THREADDICT_ENTRY(userparams)
};
#undef _THREADDICT_ENTRY

void
threaddict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_uint32_t	i;
	cw_stilo_t	name, operator;	/* XXX GC-unsafe. */
#define NENTRIES							\
	((sizeof(threaddict_ops) / sizeof(struct cw_threaddict_entry)))

	stilo_dict_new(a_dict, a_stilt, NENTRIES);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stilt, threaddict_ops[i].name,
		    strlen(threaddict_ops[i].name), TRUE);
		stilo_operator_new(&operator, threaddict_ops[i].op_f);
		stilo_attrs_set(&operator, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stilt, &name, &operator);
	}

#undef NENTRIES
}

void
threaddict_errordict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
threaddict_internaldict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
threaddict_localinstancedict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
threaddict_serverdict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
threaddict_statusdict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
threaddict_sym_derror(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
threaddict_userdict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
threaddict_userparams(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}
