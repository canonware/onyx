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

static const cw_stiln_t errordict_ops[] = {
	STILN_dstackoverflow,
	STILN_dstackunderflow,
	STILN_estackoverflow,
	STILN_interrupt,
	STILN_invalidaccess,
	STILN_invalidcontext,
	STILN_invalidexit,
	STILN_invalidfileaccess,
	STILN_ioerror,
	STILN_limitcheck,
	STILN_rangecheck,
	STILN_stackoverflow,
	STILN_stackunderflow,
	STILN_syntaxerror,
	STILN_timeout,
	STILN_typecheck,
	STILN_undefined,
	STILN_undefinedfilename,
	STILN_undefinedresult,
	STILN_unmatchedmark,
	STILN_unregistered,
	STILN_vmerror
};

void
errordict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_uint32_t	i;
	cw_stilo_t	name, operator;

#define	NEXTRA	1
#define NENTRIES							\
	(sizeof(errordict_ops) / sizeof(cw_stiln_t))

	stilo_dict_new(a_dict, stilt_stil_get(a_stilt), NENTRIES);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, stilt_stil_get(a_stilt),
		    stiln_str(errordict_ops[i]),
		    stiln_len(errordict_ops[i]), TRUE);
		stilo_operator_new(&operator, errordict_generic,
		    errordict_ops[i]);
		stilo_attrs_set(&operator, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stilt, &name, &operator);
	}

	/*
	 * Initialize entries that aren't aliases for the generic error
	 * processor.
	 */
	stilo_name_new(&name, stilt_stil_get(a_stilt),
	    stiln_str(STILN_handleerror), stiln_len(STILN_handleerror), TRUE);
	stilo_operator_new(&operator, errordict_handleerror, STILN_handleerror);
	stilo_attrs_set(&operator, STILOA_EXECUTABLE);
	stilo_dict_def(a_dict, a_stilt, &name, &operator);

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

void
errordict_generic(cw_stilt_t *a_stilt)
{
	cw_stils_t	*tstack;
	cw_stilo_t	*currenterror, *tname, *tstilo;
	cw_stiln_t	stiln;

	tstack = stilt_tstack_get(a_stilt);

	/* Get currenterror. */
	currenterror = stils_push(tstack);
	tname = stils_push(tstack);
	stilo_name_new(tname, stilt_stil_get(a_stilt),
	    stiln_str(STILN_sym_currenterror),
	    stiln_len(STILN_sym_currenterror), TRUE);
	if (stilt_dict_stack_search(a_stilt, tname, currenterror)) {
		stils_npop(tstack, 2);
		xep_throw(_CW_STILX_CURRENTERROR);
	}

	tstilo = stils_push(tstack);

	/* Set newerror to TRUE. */
	stilo_boolean_new(tstilo, TRUE);
	stilo_name_new(tname, stilt_stil_get(a_stilt),
	    stiln_str(STILN_newerror), stiln_len(STILN_newerror), TRUE);
	stilo_dict_def(currenterror, a_stilt, tname, tstilo);

	/* Set errorname. */
	stilo_name_new(tname, stilt_stil_get(a_stilt),
	    stiln_str(STILN_errorname), stiln_len(STILN_errorname), TRUE);
	stiln = stilte_stiln(stilt_error_get(a_stilt));
	stilo_name_new(tstilo, stilt_stil_get(a_stilt), stiln_str(stiln),
	    stiln_len(stiln), TRUE);
	stilo_dict_def(currenterror, a_stilt, tname, tstilo);
	
	switch (stilt_error_get(a_stilt)) {
	case STILTE_INTERRUPT:
	case STILTE_TIMEOUT:
		/* Don't do anything. */
		break;
	case STILTE_DSTACKOVERFLOW:
	case STILTE_DSTACKUNDERFLOW:
	case STILTE_ESTACKOVERFLOW:
	case STILTE_INVALIDACCESS:
	case STILTE_INVALIDCONTEXT:
	case STILTE_INVALIDEXIT:
	case STILTE_INVALIDFILEACCESS:
	case STILTE_IOERROR:
	case STILTE_LIMITCHECK:
	case STILTE_RANGECHECK:
	case STILTE_STACKUNDERFLOW:
	case STILTE_SYNTAXERROR:
	case STILTE_TYPECHECK:
	case STILTE_UNDEFINED:
	case STILTE_UNDEFINEDFILENAME:
	case STILTE_UNDEFINEDRESULT:
	case STILTE_UNMATCHEDMARK:
	case STILTE_UNREGISTERED:
	case STILTE_VMERROR: {
		cw_stils_t	*ostack, *estack, *dstack;

		ostack = stilt_ostack_get(a_stilt);
		estack = stilt_estack_get(a_stilt);
		dstack = stilt_dstack_get(a_stilt);

		/* Set command to second element of estack. */
		stilo_name_new(tname, stilt_stil_get(a_stilt),
		    stiln_str(STILN_command), stiln_len(STILN_command), TRUE);
		stilo_dict_def(currenterror, a_stilt, tname, stils_nget(estack,
		    1));

		/*
		 * If recordstacks is TRUE, snapshot the stacks (unless
		 * vmerror).
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
		if (stilo_boolean_get(tstilo) && stilt_error_get(a_stilt) !=
		    STILTE_VMERROR) {
			cw_stilo_t	*arr, *stilo;
			cw_sint32_t	i, count;

			arr = stils_push(tstack);

			/* ostack. */
			stilo_name_new(tname, stilt_stil_get(a_stilt),
			    stiln_str(STILN_ostack), stiln_len(STILN_ostack),
			    TRUE);
			count = stils_count(ostack);
			stilo_array_new(arr, stilt_stil_get(a_stilt), count);
			for (i = count - 1, stilo = NULL; i >= 0; i--) {
				stilo = stils_down_get(ostack, stilo);
				stilo_dup(stilo_array_el_get(arr, i), stilo);
			}
			stilo_dict_def(currenterror, a_stilt, tname, arr);

			/*
			 * estack.  Don't include the top element, which is
			 * this currently executing operator.
			 */
			stilo_name_new(tname, stilt_stil_get(a_stilt),
			    stiln_str(STILN_estack), stiln_len(STILN_estack),
			    TRUE);
			count = stils_count(estack) - 1;
			stilo_array_new(arr, stilt_stil_get(a_stilt), count);
			for (i = count - 1, stilo = stils_get(estack);
			     i >= 0; i--) {
				stilo = stils_down_get(estack, stilo);
				stilo_dup(stilo_array_el_get(arr, i), stilo);
			}
			stilo_dict_def(currenterror, a_stilt, tname, arr);

			/* dstack. */
			stilo_name_new(tname, stilt_stil_get(a_stilt),
			    stiln_str(STILN_dstack), stiln_len(STILN_dstack),
			    TRUE);
			count = stils_count(dstack);
			stilo_array_new(arr, stilt_stil_get(a_stilt), count);
			for (i = count - 1, stilo = NULL; i >= 0; i--) {
				stilo = stils_down_get(dstack, stilo);
				stilo_dup(stilo_array_el_get(arr, i), stilo);
			}
			stilo_dict_def(currenterror, a_stilt, tname, arr);

			stils_pop(tstack);
		}
		break;
	}
	default:
		_cw_not_reached();
	}
	stils_npop(tstack, 3);

	_cw_stil_code(a_stilt, "handleerror stop");
}

void
errordict_handleerror(cw_stilt_t *a_stilt)
{
	_cw_stil_code(a_stilt, "
currenterror begin
(Error /) print errorname cvs print ( in ) print //command spop
recordstacks {
	(ostack: ) print
	ostack spop
	(estack: ) print
	estack spop
	(dstack: ) print
	dstack spop
} if
end
");
}
