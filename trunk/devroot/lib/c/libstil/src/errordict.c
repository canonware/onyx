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
	ENTRY(unmatchedfino),
	ENTRY(unregistered)
};

void
errordict_l_populate(cw_stilo_t *a_dict, cw_stilo_t *a_thread)
{
	cw_stilo_t	*tstack;
	cw_stilo_t	*name, *value;
	cw_uint32_t	i;

#define	NEXTRA	0
#define NENTRIES							\
	(sizeof(errordict_ops) / sizeof(struct cw_errordict_entry))

	stilo_dict_new(a_dict, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), NENTRIES + NEXTRA);

	tstack = stilo_thread_tstack_get(a_thread);
	name = stilo_stack_push(tstack);
	value = stilo_stack_push(tstack);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(name, stilo_thread_stil_get(a_thread),
		    stiln_str(errordict_ops[i].stiln),
		    stiln_len(errordict_ops[i].stiln), TRUE);
		stilo_operator_new(value, errordict_ops[i].op_f,
		    errordict_ops[i].stiln);
		stilo_attrs_set(value, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, stilo_thread_stil_get(a_thread), name, value);
	}

	stilo_stack_npop(tstack, 2);

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
errordict_p_generic(cw_stilo_t *a_thread, cw_stilo_threade_t a_threade,
    cw_bool_t a_record)
{
	cw_stilo_t	*tstack;
	cw_stilo_t	*currenterror, *tname, *tstilo;
	cw_stiln_t	stiln;

	tstack = stilo_thread_tstack_get(a_thread);

	/* Get currenterror. */
	currenterror = stilo_stack_push(tstack);
	tname = stilo_stack_push(tstack);
	stilo_name_new(tname, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_currenterror),
	    stiln_len(STILN_currenterror), TRUE);
	if (stilo_thread_dstack_search(a_thread, tname, currenterror)) {
		stilo_stack_npop(tstack, 2);
		xep_throw(_CW_STILX_CURRENTERROR);
	}

	tstilo = stilo_stack_push(tstack);

	/* Set newerror to TRUE. */
	stilo_boolean_new(tstilo, TRUE);
	stilo_name_new(tname, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_newerror), stiln_len(STILN_newerror), TRUE);
	stilo_dict_def(currenterror, stilo_thread_stil_get(a_thread), tname,
	    tstilo);

	/* Set errorname. */
	stilo_name_new(tname, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_errorname), stiln_len(STILN_errorname), TRUE);
	stiln = stilo_threade_stiln(a_threade);
	stilo_name_new(tstilo, stilo_thread_stil_get(a_thread),
	    stiln_str(stiln), stiln_len(stiln), TRUE);
	stilo_dict_def(currenterror, stilo_thread_stil_get(a_thread), tname,
	    tstilo);
	
	if (a_record) {
		cw_stilo_t	*ostack, *estack, *dstack;

		ostack = stilo_thread_ostack_get(a_thread);
		estack = stilo_thread_estack_get(a_thread);
		dstack = stilo_thread_dstack_get(a_thread);

		/* Set command to second element of estack. */
		stilo_name_new(tname, stilo_thread_stil_get(a_thread),
		    stiln_str(STILN_command), stiln_len(STILN_command), TRUE);
		stilo_dict_def(currenterror, stilo_thread_stil_get(a_thread),
		    tname, stilo_stack_nget(estack, 1));

		/*
		 * If recordstacks is TRUE, snapshot the stacks.
		 */
		stilo_name_new(tname, stilo_thread_stil_get(a_thread),
		    stiln_str(STILN_recordstacks),
		    stiln_len(STILN_recordstacks), TRUE);
		if (stilo_dict_lookup(currenterror, tname, tstilo)) {
			stilo_stack_npop(tstack, 3);
			xep_throw(_CW_STILX_CURRENTERROR);
		}
		if (stilo_type_get(tstilo) != STILOT_BOOLEAN) {
			stilo_stack_npop(tstack, 3);
			xep_throw(_CW_STILX_CURRENTERROR);
		}
		if (stilo_boolean_get(tstilo) && a_threade) {
			cw_stilo_t	*stack, *stilo, *nstilo;
			cw_sint32_t	i, count;

			stack = stilo_stack_push(tstack);

			/* ostack. */
			stilo_name_new(tname, stilo_thread_stil_get(a_thread),
			    stiln_str(STILN_ostack), stiln_len(STILN_ostack),
			    TRUE);
			count = stilo_stack_count(ostack);
			for (i = 0, stilo = NULL; i < count; i++) {
				stilo = stilo_stack_down_get(ostack, stilo);
				nstilo = stilo_stack_push(tstack);
				stilo_dup(nstilo, stilo);
			}
			stilo_stack_new(stack, stilo_thread_stil_get(a_thread),
			    FALSE);
			for (i = 0; i < count; i++) {
				stilo = stilo_stack_get(tstack);
				nstilo = stilo_stack_push(stack);
				stilo_dup(nstilo, stilo);
				stilo_stack_pop(tstack);
			}
			stilo_dict_def(currenterror,
			    stilo_thread_stil_get(a_thread), tname, stack);

			/*
			 * estack.  Don't include the top element, which is
			 * this currently executing operator.
			 */
			stilo_name_new(tname, stilo_thread_stil_get(a_thread),
			    stiln_str(STILN_estack), stiln_len(STILN_estack),
			    TRUE);
			count = stilo_stack_count(estack) - 1;
			for (i = 0, stilo = stilo_stack_get(estack); i < count;
			    i++) {
				stilo = stilo_stack_down_get(estack, stilo);
				nstilo = stilo_stack_push(tstack);
				stilo_dup(nstilo, stilo);
			}
			stilo_stack_new(stack, stilo_thread_stil_get(a_thread),
			    FALSE);
			for (i = 0; i < count; i++) {
				stilo = stilo_stack_get(tstack);
				nstilo = stilo_stack_push(stack);
				stilo_dup(nstilo, stilo);
				stilo_stack_pop(tstack);
			}
			stilo_dict_def(currenterror,
			    stilo_thread_stil_get(a_thread), tname, stack);

			/* dstack. */
			stilo_name_new(tname, stilo_thread_stil_get(a_thread),
			    stiln_str(STILN_dstack), stiln_len(STILN_dstack),
			    TRUE);
			count = stilo_stack_count(dstack);
			for (i = 0, stilo = NULL; i < count; i++) {
				stilo = stilo_stack_down_get(dstack, stilo);
				nstilo = stilo_stack_push(tstack);
				stilo_dup(nstilo, stilo);
			}
			stilo_stack_new(stack, stilo_thread_stil_get(a_thread),
			    FALSE);
			for (i = 0; i < count; i++) {
				stilo = stilo_stack_get(tstack);
				nstilo = stilo_stack_push(stack);
				stilo_dup(nstilo, stilo);
				stilo_stack_pop(tstack);
			}
			stilo_dict_def(currenterror,
			    stilo_thread_stil_get(a_thread), tname, stack);

			stilo_stack_pop(tstack);
		}
	}
	stilo_stack_npop(tstack, 3);

	_cw_stil_code(a_thread, "handleerror currenterror /stop get eval");
}

void
errordict_dstackunderflow(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_DSTACKUNDERFLOW, TRUE);
}

void
errordict_estackoverflow(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_ESTACKOVERFLOW, TRUE);
}

void
errordict_handleerror(cw_stilo_t *a_thread)
{
	_cw_stil_code(a_thread, "
currenterror begin
errorname /syntaxerror eq {
	`At line ' print
	line cvs print
	`, column ' print
	column cvs print
	`: ' print
} if
`Error /' print errorname cvs print ` in ' print /command load 1 sprint
recordstacks {
	`ostack: ' print
	ostack 1 sprint
	`estack: ' print
	estack 1 sprint
	`dstack: ' print
	dstack 1 sprint
} if
end
");
}

void
errordict_interrupt(cw_stilo_t *a_thread)
{
	/* Do nothing. */
}

void
errordict_invalidaccess(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_INVALIDACCESS, TRUE);
}

void
errordict_invalidcontext(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_INVALIDCONTEXT, TRUE);
}

void
errordict_invalidexit(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_INVALIDEXIT, TRUE);
}

void
errordict_invalidfileaccess(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_INVALIDFILEACCESS, TRUE);
}

void
errordict_ioerror(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_IOERROR, TRUE);
}

void
errordict_limitcheck(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_LIMITCHECK, TRUE);
}

void
errordict_rangecheck(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_RANGECHECK, TRUE);
}

void
errordict_stackunderflow(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_STACKUNDERFLOW, TRUE);
}

void
errordict_syntaxerror(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_SYNTAXERROR, TRUE);
}

void
errordict_timeout(cw_stilo_t *a_thread)
{
	/* Do nothing. */
}

void
errordict_typecheck(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_TYPECHECK, TRUE);
}

void
errordict_undefined(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_UNDEFINED, TRUE);
}

void
errordict_undefinedfilename(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_UNDEFINEDFILENAME, TRUE);
}

void
errordict_undefinedresult(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_UNDEFINEDRESULT, TRUE);
}

void
errordict_unmatchedfino(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_UNMATCHEDFINO, TRUE);
}

void
errordict_unmatchedmark(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_UNMATCHEDMARK, TRUE);
}

void
errordict_unregistered(cw_stilo_t *a_thread)
{
	errordict_p_generic(a_thread, STILO_THREADE_UNREGISTERED, TRUE);
}
