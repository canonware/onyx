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

#include <errno.h>

struct cw_errordict_entry {
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

#define _ERRORDICT_ENTRY(name)	{#name, errordict_##name}

/*
 * Array of operators in errordict.
 */
static struct cw_errordict_entry errordict_ops[] = {
	_ERRORDICT_ENTRY(dictstackoverflow),
	_ERRORDICT_ENTRY(dictstackunderflow),
	_ERRORDICT_ENTRY(execstackoverflow),
	_ERRORDICT_ENTRY(interrupt),
	_ERRORDICT_ENTRY(invalidaccess),
	_ERRORDICT_ENTRY(invalidcontext),
	_ERRORDICT_ENTRY(invalidexit),
	_ERRORDICT_ENTRY(invalidfileaccess),
	_ERRORDICT_ENTRY(ioerror),
	_ERRORDICT_ENTRY(limitcheck),
	_ERRORDICT_ENTRY(rangecheck),
	_ERRORDICT_ENTRY(stackoverflow),
	_ERRORDICT_ENTRY(stackunderflow),
	_ERRORDICT_ENTRY(syntaxerror),
	_ERRORDICT_ENTRY(timeout),
	_ERRORDICT_ENTRY(typecheck),
	_ERRORDICT_ENTRY(undefined),
	_ERRORDICT_ENTRY(undefinedfilename),
	_ERRORDICT_ENTRY(undefinedresource),
	_ERRORDICT_ENTRY(undefinedresult),
	_ERRORDICT_ENTRY(unmatchedmark),
	_ERRORDICT_ENTRY(unregistered),
	_ERRORDICT_ENTRY(vmerror)
};
#undef _ERRORDICT_ENTRY

void
errordict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_uint32_t	i;
	cw_stilo_t	name, operator;	/* XXX GC-unsafe. */
#define NENTRIES							\
	((sizeof(errordict_ops) / sizeof(struct cw_errordict_entry)))

	stilo_dict_new(a_dict, a_stilt, NENTRIES);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stilt, errordict_ops[i].name,
		    strlen(errordict_ops[i].name), TRUE);
		stilo_operator_new(&operator, errordict_ops[i].op_f);
		stilo_attrs_set(&operator, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stilt, &name, &operator);
	}

#undef NENTRIES
}

void
errordict_dictstackoverflow(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_dictstackunderflow(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_execstackoverflow(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_interrupt(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_invalidaccess(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_invalidcontext(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_invalidexit(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_invalidfileaccess(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_ioerror(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_limitcheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_rangecheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_stackoverflow(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_stackunderflow(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_syntaxerror(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_timeout(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_typecheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_undefined(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_undefinedfilename(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_undefinedresource(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_undefinedresult(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_unmatchedmark(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_unregistered(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
errordict_vmerror(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}
