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

#include <errno.h>

struct cw_errordict_entry {
	cw_nxn_t	nxn;
	cw_op_t		*op_f;
};

#define ENTRY(name)	{NXN_##name, errordict_##name}

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
errordict_l_populate(cw_nxo_t *a_dict, cw_nxo_t *a_thread)
{
	cw_nxo_t	*tstack;
	cw_nxo_t	*name, *value;
	cw_uint32_t	i;

#define	NEXTRA	0
#define NENTRIES							\
	(sizeof(errordict_ops) / sizeof(struct cw_errordict_entry))

	nxo_dict_new(a_dict, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), NENTRIES + NEXTRA);

	tstack = nxo_thread_tstack_get(a_thread);
	name = nxo_stack_push(tstack);
	value = nxo_stack_push(tstack);

	for (i = 0; i < NENTRIES; i++) {
		nxo_name_new(name, nxo_thread_nx_get(a_thread),
		    nxn_str(errordict_ops[i].nxn),
		    nxn_len(errordict_ops[i].nxn), TRUE);
		nxo_operator_new(value, errordict_ops[i].op_f,
		    errordict_ops[i].nxn);
		nxo_attrs_set(value, NXOA_EXECUTABLE);

		nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, value);
	}

	nxo_stack_npop(tstack, 2);

#ifdef _LIBONYX_DBG
	if (nxo_dict_count(a_dict) != NENTRIES + NEXTRA) {
		_cw_out_put_e("nxo_dict_count(a_dict) != NENTRIES + NEXTRA"
		    " ([i] != [i])\n", nxo_dict_count(a_dict), NENTRIES +
		    NEXTRA);
		_cw_error("Adjust NEXTRA");
	}
#endif
#undef NENTRIES
#undef NEXTRA
}

static void
errordict_p_generic(cw_nxo_t *a_thread, cw_nxo_threade_t a_threade,
    cw_bool_t a_record)
{
	cw_nxo_t	*tstack;
	cw_nxo_t	*currenterror, *tname, *tnxo;
	cw_nxn_t	nxn;

	tstack = nxo_thread_tstack_get(a_thread);

	/* Get currenterror. */
	currenterror = nxo_stack_push(tstack);
	tname = nxo_stack_push(tstack);
	nxo_name_new(tname, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_currenterror),
	    nxn_len(NXN_currenterror), TRUE);
	if (nxo_thread_dstack_search(a_thread, tname, currenterror)) {
		/*
		 * Fall back to the currenterror defined during thread creation.
		 */
		nxo_dup(currenterror,
		    nxo_thread_currenterror_get(a_thread));
	}

	tnxo = nxo_stack_push(tstack);

	/* Set newerror to TRUE. */
	nxo_boolean_new(tnxo, TRUE);
	nxo_name_new(tname, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_newerror), nxn_len(NXN_newerror), TRUE);
	nxo_dict_def(currenterror, nxo_thread_nx_get(a_thread), tname,
	    tnxo);

	/* Set errorname. */
	nxo_name_new(tname, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_errorname), nxn_len(NXN_errorname), TRUE);
	nxn = nxo_threade_nxn(a_threade);
	nxo_name_new(tnxo, nxo_thread_nx_get(a_thread),
	    nxn_str(nxn), nxn_len(nxn), TRUE);
	nxo_dict_def(currenterror, nxo_thread_nx_get(a_thread), tname,
	    tnxo);
	
	if (a_record) {
		cw_nxo_t	*ostack, *estack, *dstack;

		ostack = nxo_thread_ostack_get(a_thread);
		estack = nxo_thread_estack_get(a_thread);
		dstack = nxo_thread_dstack_get(a_thread);

		/* Set command to second element of estack. */
		nxo_name_new(tname, nxo_thread_nx_get(a_thread),
		    nxn_str(NXN_command), nxn_len(NXN_command), TRUE);
		nxo_dict_def(currenterror, nxo_thread_nx_get(a_thread),
		    tname, nxo_stack_nget(estack, 1));

		/*
		 * If recordstacks is TRUE, snapshot the stacks.
		 */
		nxo_name_new(tname, nxo_thread_nx_get(a_thread),
		    nxn_str(NXN_recordstacks),
		    nxn_len(NXN_recordstacks), TRUE);
		if (nxo_dict_lookup(currenterror, tname, tnxo)) {
			/*
			 * Give up on snapshotting the stacks, since the
			 * alternative is to blow up (or potentially go
			 * infinitely recursive).
			 */
			goto ERROR;
		}
		if (nxo_type_get(tnxo) != NXOT_BOOLEAN)
			goto ERROR;
		if (nxo_boolean_get(tnxo) && a_threade) {
			cw_nxo_t	*stack;

			stack = nxo_stack_push(tstack);

			/* ostack. */
			nxo_name_new(tname, nxo_thread_nx_get(a_thread),
			    nxn_str(NXN_ostack), nxn_len(NXN_ostack),
			    TRUE);
			nxo_stack_new(stack, nxo_thread_nx_get(a_thread),
			    FALSE);
			nxo_stack_copy(stack, ostack);
			nxo_dict_def(currenterror,
			    nxo_thread_nx_get(a_thread), tname, stack);

			/*
			 * estack.  Don't include the top element, which is
			 * this currently executing operator.
			 */
			nxo_name_new(tname, nxo_thread_nx_get(a_thread),
			    nxn_str(NXN_estack), nxn_len(NXN_estack),
			    TRUE);
			nxo_stack_new(stack, nxo_thread_nx_get(a_thread),
			    FALSE);
			nxo_stack_copy(stack, estack);
			nxo_stack_pop(stack);
			nxo_dict_def(currenterror,
			    nxo_thread_nx_get(a_thread), tname, stack);

			/* dstack. */
			nxo_name_new(tname, nxo_thread_nx_get(a_thread),
			    nxn_str(NXN_dstack), nxn_len(NXN_dstack),
			    TRUE);
			nxo_stack_new(stack, nxo_thread_nx_get(a_thread),
			    FALSE);
			nxo_stack_copy(stack, dstack);
			nxo_dict_def(currenterror,
			    nxo_thread_nx_get(a_thread), tname, stack);

			nxo_stack_pop(tstack);
		}
	}
	ERROR:
	nxo_stack_npop(tstack, 3);

	_cw_onyx_code(a_thread, "handleerror currenterror /stop get eval");
}

void
errordict_dstackunderflow(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_DSTACKUNDERFLOW, TRUE);
}

void
errordict_estackoverflow(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_ESTACKOVERFLOW, TRUE);
}

void
errordict_handleerror(cw_nxo_t *a_thread)
{
	_cw_onyx_code(a_thread, "
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
errordict_interrupt(cw_nxo_t *a_thread)
{
	/* Do nothing. */
}

void
errordict_invalidaccess(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_INVALIDACCESS, TRUE);
}

void
errordict_invalidcontext(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_INVALIDCONTEXT, TRUE);
}

void
errordict_invalidexit(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_INVALIDEXIT, TRUE);
}

void
errordict_invalidfileaccess(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_INVALIDFILEACCESS, TRUE);
}

void
errordict_ioerror(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_IOERROR, TRUE);
}

void
errordict_limitcheck(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_LIMITCHECK, TRUE);
}

void
errordict_rangecheck(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_RANGECHECK, TRUE);
}

void
errordict_stackunderflow(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_STACKUNDERFLOW, TRUE);
}

void
errordict_syntaxerror(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_SYNTAXERROR, TRUE);
}

void
errordict_timeout(cw_nxo_t *a_thread)
{
	/* Do nothing. */
}

void
errordict_typecheck(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_TYPECHECK, TRUE);
}

void
errordict_undefined(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_UNDEFINED, TRUE);
}

void
errordict_undefinedfilename(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_UNDEFINEDFILENAME, TRUE);
}

void
errordict_undefinedresult(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_UNDEFINEDRESULT, TRUE);
}

void
errordict_unmatchedfino(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_UNMATCHEDFINO, TRUE);
}

void
errordict_unmatchedmark(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_UNMATCHEDMARK, TRUE);
}

void
errordict_unregistered(cw_nxo_t *a_thread)
{
	errordict_p_generic(a_thread, NXO_THREADE_UNREGISTERED, TRUE);
}
