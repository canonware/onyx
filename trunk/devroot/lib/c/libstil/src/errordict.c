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

#include <errno.h>

struct cw_errordict_entry {
	cw_stiln_t	stiln;
	cw_op_t		*op_f;
};

#define ENTRY(name)	{STILN_##name, errordict_##name}

/*
 * Array of operators in errordict.
 */
static const struct cw_errordict_entry errordict_ops[] = {
	ENTRY(dstackunderflow),
	ENTRY(estackoverflow),
	ENTRY(handleerror),
	ENTRY(interrupt),
	ENTRY(invalidaccess),
	ENTRY(invalidcontext),
	ENTRY(invalidexit),
	ENTRY(invalidfileaccess),
	ENTRY(ioerror),
	ENTRY(limitcheck),
	ENTRY(rangecheck),
	ENTRY(stackunderflow),
	ENTRY(syntaxerror),
	ENTRY(timeout),
	ENTRY(typecheck),
	ENTRY(undefined),
	ENTRY(undefinedfilename),
	ENTRY(undefinedresult),
	ENTRY(unmatchedmark),
	ENTRY(unregistered)
};

void
errordict_l_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_stils_t	*tstack;
	cw_stilo_t	*name, *value;
	cw_uint32_t	i;

#define	NEXTRA	0
#define NENTRIES							\
	(sizeof(errordict_ops) / sizeof(struct cw_errordict_entry))

	stilo_dict_new(a_dict, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), NENTRIES + NEXTRA);

	tstack = stilt_tstack_get(a_stilt);
	name = stils_push(tstack);
	value = stils_push(tstack);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(name, stilt_stil_get(a_stilt),
		    stiln_str(errordict_ops[i].stiln),
		    stiln_len(errordict_ops[i].stiln), TRUE);
		stilo_operator_new(value, errordict_ops[i].op_f,
		    errordict_ops[i].stiln);
		stilo_attrs_set(value, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, stilt_stil_get(a_stilt), name, value);
	}

	stils_npop(tstack, 2);

#ifdef _LIBSTIL_DBG
	if (stilo_dict_count(a_dict) != NENTRIES + NEXTRA) {
		_cw_out_put_e("stilo_dict_count(a_dict) != NENTRIES + NEXTRA"
		    " ([i] != [i])\n", stilo_dict_count(a_dict), NENTRIES +
		    NEXTRA);
		_cw_error("Adjust NEXTRA");
	}
#endif
#undef NENTRIES
#undef NEXTRA
}

static void
errordict_p_generic(cw_stilt_t *a_stilt, cw_stilte_t a_stilte, cw_bool_t
    a_record)
{
	cw_stils_t	*tstack;
	cw_stilo_t	*currenterror, *tname, *tstilo;
	cw_stiln_t	stiln;

	tstack = stilt_tstack_get(a_stilt);

	/* Get currenterror. */
	currenterror = stils_push(tstack);
	tname = stils_push(tstack);
	stilo_name_new(tname, stilt_stil_get(a_stilt),
	    stiln_str(STILN_currenterror),
	    stiln_len(STILN_currenterror), TRUE);
	if (stilt_dict_stack_search(a_stilt, tname, currenterror)) {
		stils_npop(tstack, 2);
		xep_throw(_CW_STILX_CURRENTERROR);
	}

	tstilo = stils_push(tstack);

	/* Set newerror to TRUE. */
	stilo_boolean_new(tstilo, TRUE);
	stilo_name_new(tname, stilt_stil_get(a_stilt),
	    stiln_str(STILN_newerror), stiln_len(STILN_newerror), TRUE);
	stilo_dict_def(currenterror, stilt_stil_get(a_stilt), tname, tstilo);

	/* Set errorname. */
	stilo_name_new(tname, stilt_stil_get(a_stilt),
	    stiln_str(STILN_errorname), stiln_len(STILN_errorname), TRUE);
	stiln = stilte_stiln(a_stilte);
	stilo_name_new(tstilo, stilt_stil_get(a_stilt), stiln_str(stiln),
	    stiln_len(stiln), TRUE);
	stilo_dict_def(currenterror, stilt_stil_get(a_stilt), tname, tstilo);
	
	if (a_record) {
		cw_stils_t	*ostack, *estack, *dstack;

		ostack = stilt_ostack_get(a_stilt);
		estack = stilt_estack_get(a_stilt);
		dstack = stilt_dstack_get(a_stilt);

		/* Set command to second element of estack. */
		stilo_name_new(tname, stilt_stil_get(a_stilt),
		    stiln_str(STILN_command), stiln_len(STILN_command), TRUE);
		stilo_dict_def(currenterror, stilt_stil_get(a_stilt), tname,
		    stils_nget(estack, 1));

		/*
		 * If recordstacks is TRUE, snapshot the stacks.
		 */
		stilo_name_new(tname, stilt_stil_get(a_stilt),
		    stiln_str(STILN_recordstacks),
		    stiln_len(STILN_recordstacks), TRUE);
		if (stilo_dict_lookup(currenterror, tname, tstilo)) {
			stils_npop(tstack, 3);
			xep_throw(_CW_STILX_CURRENTERROR);
		}
		if (stilo_type_get(tstilo) != STILOT_BOOLEAN) {
			stils_npop(tstack, 3);
			xep_throw(_CW_STILX_CURRENTERROR);
		}
		if (stilo_boolean_get(tstilo) && a_stilte) {
			cw_stilo_t	*arr, *stilo;
			cw_sint32_t	i, count;

			arr = stils_push(tstack);

			/* ostack. */
			stilo_name_new(tname, stilt_stil_get(a_stilt),
			    stiln_str(STILN_ostack), stiln_len(STILN_ostack),
			    TRUE);
			count = stils_count(ostack);
			stilo_array_new(arr, stilt_stil_get(a_stilt),
			    stilt_currentlocking(a_stilt), count);
			for (i = count - 1, stilo = NULL; i >= 0; i--) {
				stilo = stils_down_get(ostack, stilo);
				stilo_array_el_set(arr, stilo, i);
			}
			stilo_dict_def(currenterror, stilt_stil_get(a_stilt),
			    tname, arr);

			/*
			 * estack.  Don't include the top element, which is
			 * this currently executing operator.
			 */
			stilo_name_new(tname, stilt_stil_get(a_stilt),
			    stiln_str(STILN_estack), stiln_len(STILN_estack),
			    TRUE);
			count = stils_count(estack) - 1;
			stilo_array_new(arr, stilt_stil_get(a_stilt),
			    stilt_currentlocking(a_stilt), count);
			for (i = count - 1, stilo = stils_get(estack);
			     i >= 0; i--) {
				stilo = stils_down_get(estack, stilo);
				stilo_array_el_set(arr, stilo, i);
			}
			stilo_dict_def(currenterror, stilt_stil_get(a_stilt),
			    tname, arr);

			/* dstack. */
			stilo_name_new(tname, stilt_stil_get(a_stilt),
			    stiln_str(STILN_dstack), stiln_len(STILN_dstack),
			    TRUE);
			count = stils_count(dstack);
			stilo_array_new(arr, stilt_stil_get(a_stilt),
			    stilt_currentlocking(a_stilt), count);
			for (i = count - 1, stilo = NULL; i >= 0; i--) {
				stilo = stils_down_get(dstack, stilo);
				stilo_array_el_set(arr, stilo, i);
			}
			stilo_dict_def(currenterror, stilt_stil_get(a_stilt),
			    tname, arr);

			stils_pop(tstack);
		}
	}
	stils_npop(tstack, 3);

	/*
	 * XXX The stop operator is called in two places (here and in
	 * handleerror).  Which is correct?  Under normal conditions, this copy
	 * of the call never gets reached, but there is a bug here.
	 */
	_cw_stil_code(a_stilt, "handleerror currenterror /stop get eval");
}

void
errordict_dstackunderflow(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_DSTACKUNDERFLOW, TRUE);
}

void
errordict_estackoverflow(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_ESTACKOVERFLOW, TRUE);
}

void
errordict_handleerror(cw_stilt_t *a_stilt)
{
	_cw_stil_code(a_stilt, "
currenterror begin
(Error /) print errorname cvs print ( in ) print /command load 1 spop
recordstacks {
	(ostack: ) print
	ostack 1 spop
	(estack: ) print
	estack 1 spop
	(dstack: ) print
	dstack 1 spop
} if
end
currenterror /stop get eval
");
}

void
errordict_interrupt(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_INTERRUPT, FALSE);
}

void
errordict_invalidaccess(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_INVALIDACCESS, TRUE);
}

void
errordict_invalidcontext(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_INVALIDCONTEXT, TRUE);
}

void
errordict_invalidexit(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_INVALIDEXIT, TRUE);
}

void
errordict_invalidfileaccess(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_INVALIDFILEACCESS, TRUE);
}

void
errordict_ioerror(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_IOERROR, TRUE);
}

void
errordict_limitcheck(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_LIMITCHECK, TRUE);
}

void
errordict_rangecheck(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_RANGECHECK, TRUE);
}

void
errordict_stackunderflow(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_STACKUNDERFLOW, TRUE);
}

void
errordict_syntaxerror(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_SYNTAXERROR, TRUE);
}

void
errordict_timeout(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_TIMEOUT, FALSE);
}

void
errordict_typecheck(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_TYPECHECK, TRUE);
}

void
errordict_undefined(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_UNDEFINED, TRUE);
}

void
errordict_undefinedfilename(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_UNDEFINEDFILENAME, TRUE);
}

void
errordict_undefinedresult(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_UNDEFINEDRESULT, TRUE);
}

void
errordict_unmatchedmark(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_UNMATCHEDMARK, TRUE);
}

void
errordict_unregistered(cw_stilt_t *a_stilt)
{
	errordict_p_generic(a_stilt, STILTE_UNREGISTERED, TRUE);
}
