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

/*
 * Array of keys in errordict.
 */
static const cw_uint8_t *errordict_keys[] = {
	"dictstackoverflow",
	"dictstackunderflow",
	"execstackoverflow",
	"handleerror",
	"interrupt",
	"invalidaccess",
	"invalidcontext",
	"invalidexit",
	"invalidfileaccess",
	"ioerror",
	"limitcheck",
	"rangecheck",
	"stackoverflow",
	"stackunderflow",
	"syntaxerror",
	"timeout",
	"typecheck",
	"undefined",
	"undefinedfilename",
	"undefinedresource",
	"undefinedresult",
	"unmatchedmark",
	"unregistered",
	"vmerror"
};

void
errordict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_uint32_t	i;
	cw_stilo_t	name, operator;	/* XXX GC-unsafe. */
#define NENTRIES	(sizeof(errordict_keys) / sizeof(cw_uint8_t *))

	stilo_dict_new(a_dict, a_stilt, NENTRIES);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stilt, errordict_keys[i],
		    strlen(errordict_keys[i]), TRUE);
		stilo_operator_new(&operator, errordict_generic);
		stilo_attrs_set(&operator, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stilt, &name, &operator);
	}
#undef NENTRIES
}

void
errordict_generic(cw_stilt_t *a_stilt)
{
	cw_stils_t	*tstack;
	cw_stilo_t	*derror, *tname, *tstilo;

	tstack = stilt_tstack_get(a_stilt);

	/* Get $error. */
	derror = stils_push(tstack, a_stilt);
	tname = stils_push(tstack, a_stilt);
	{
		static const cw_uint8_t	keystr[] = "$error";

		stilo_name_new(tname, a_stilt, keystr, sizeof(keystr) - 1,
		    TRUE);
	}
	if (stilt_dict_stack_search(a_stilt, tname, derror)) {
		stils_npop(tstack, a_stilt, 2);
		xep_throw(_CW_STILX_DERROR);
	}

	tstilo = stils_push(tstack, a_stilt);

	/* Set newerror to TRUE. */
	stilo_boolean_new(tstilo, TRUE);
	{
		static const cw_uint8_t	keystr[] = "newerror";

		stilo_name_new(tname, a_stilt, keystr, sizeof(keystr) - 1,
		    TRUE);
	}
	stilo_dict_def(derror, a_stilt, tname, tstilo);

	/* Set errorname. */
	{
		static const cw_uint8_t	keystr[] = "errorname";

		stilo_name_new(tname, a_stilt, keystr, sizeof(keystr) - 1,
		    TRUE);
	}
	switch (stilt_error_get(a_stilt)) {
	case STILTE_DICTSTACKOVERFLOW: {
		static const cw_uint8_t	str[] = "dictstackoverflow";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_DICTSTACKUNDERFLOW: {
		static const cw_uint8_t	str[] = "dictstackunderflow";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_EXECSTACKOVERFLOW: {
		static const cw_uint8_t	str[] = "execstackoverflow";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_INTERRUPT: {
		static const cw_uint8_t	str[] = "interrupt";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_INVALIDACCESS: {
		static const cw_uint8_t	str[] = "invalidaccess";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_INVALIDCONTEXT: {
		static const cw_uint8_t	str[] = "invalidcontext";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_INVALIDEXIT: {
		static const cw_uint8_t	str[] = "invalidexit";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_INVALIDFILEACCESS: {
		static const cw_uint8_t	str[] = "invalidfileaccess";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_IOERROR: {
		static const cw_uint8_t	str[] = "ioerror";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_LIMITCHECK: {
		static const cw_uint8_t	str[] = "limitcheck";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_RANGECHECK: {
		static const cw_uint8_t	str[] = "rangecheck";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_STACKOVERFLOW: {
		static const cw_uint8_t	str[] = "stackoverflow";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_STACKUNDERFLOW: {
		static const cw_uint8_t	str[] = "stackunderflow";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_SYNTAXERROR: {
		static const cw_uint8_t	str[] = "syntaxerror";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_TIMEOUT: {
		static const cw_uint8_t	str[] = "timeout";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_TYPECHECK: {
		static const cw_uint8_t	str[] = "typecheck";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_UNDEFINED: {
		static const cw_uint8_t	str[] = "undefined";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_UNDEFINEDFILENAME: {
		static const cw_uint8_t	str[] = "undefinedfilename";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_UNDEFINEDRESOURCE: {
		static const cw_uint8_t	str[] = "undefinedresource";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_UNDEFINEDRESULT: {
		static const cw_uint8_t	str[] = "undefinedresult";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_UNMATCHEDMARK: {
		static const cw_uint8_t	str[] = "unmatchedmark";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_UNREGISTERED: {
		static const cw_uint8_t	str[] = "unregistered";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	case STILTE_VMERROR: {
		static const cw_uint8_t	str[] = "vmerror";

		stilo_string_new(tstilo, a_stilt, sizeof(str) - 1);
		stilo_string_set(tstilo, a_stilt, 0, str, sizeof(str) - 1);
		break;
	}
	default:
		_cw_not_reached();
	}
	stilo_dict_def(derror, a_stilt, tname, tstilo);
	
	switch (stilt_error_get(a_stilt)) {
	case STILTE_INTERRUPT:
	case STILTE_TIMEOUT:
		/* Don't do anything. */
		break;
	case STILTE_DICTSTACKOVERFLOW:
	case STILTE_DICTSTACKUNDERFLOW:
	case STILTE_EXECSTACKOVERFLOW:
	case STILTE_INVALIDACCESS:
	case STILTE_INVALIDCONTEXT:
	case STILTE_INVALIDEXIT:
	case STILTE_INVALIDFILEACCESS:
	case STILTE_IOERROR:
	case STILTE_LIMITCHECK:
	case STILTE_RANGECHECK:
	case STILTE_STACKOVERFLOW:
	case STILTE_STACKUNDERFLOW:
	case STILTE_SYNTAXERROR:
	case STILTE_TYPECHECK:
	case STILTE_UNDEFINED:
	case STILTE_UNDEFINEDFILENAME:
	case STILTE_UNDEFINEDRESOURCE:
	case STILTE_UNDEFINEDRESULT:
	case STILTE_UNMATCHEDMARK:
	case STILTE_UNREGISTERED:
	case STILTE_VMERROR: {
		cw_stils_t	*ostack, *estack, *dstack;

		ostack = stilt_ostack_get(a_stilt);
		estack = stilt_estack_get(a_stilt);
		dstack = stilt_dstack_get(a_stilt);

		/* Set command to top of estack. */
		{
			static const cw_uint8_t	keystr[] = "command";

			stilo_name_new(tname, a_stilt, keystr, sizeof(keystr) -
			    1, TRUE);
		}
		/* XXX */
		stilo_dict_def(derror, a_stilt, tname, stils_get(estack,
		    a_stilt));

		/*
		 * If recordstacks is TRUE, snapshot the stacks (unless
		 * vmerror).
		 */
		{
			static const cw_uint8_t	keystr[] = "recordstacks";

			stilo_name_new(tname, a_stilt, keystr, sizeof(keystr) -
			    1, TRUE);
		}
		if (stilo_dict_lookup(derror, a_stilt, tname, tstilo)) {
			stils_npop(tstack, a_stilt, 3);
			xep_throw(_CW_STILX_DERROR);
		}
		if (stilo_type_get(tstilo) != STILOT_BOOLEAN) {
			stils_npop(tstack, a_stilt, 3);
			xep_throw(_CW_STILX_DERROR);
		}
		if (stilo_boolean_get(tstilo) && stilt_error_get(a_stilt) !=
		    STILTE_VMERROR) {
			cw_stilo_t	*arr, *stilo;
			cw_sint32_t	i, count;

			arr = stils_push(tstack, a_stilt);

			/* ostack. */
			{
				static const cw_uint8_t	keystr[] = "ostack";

				stilo_name_new(tname, a_stilt, keystr,
				    sizeof(keystr) - 1, TRUE);
			}
			count = stils_count(ostack);
			stilo_array_new(arr, a_stilt, count);
			for (i = count - 1, stilo = NULL; i >= 0; i--) {
				stilo = stils_down_get(ostack, a_stilt, stilo);
				stilo_dup(stilo_array_el_get(arr, a_stilt, i),
				    stilo);
			}
			stilo_dict_def(derror, a_stilt, tname, arr);
			
			/* estack. */
			{
				static const cw_uint8_t	keystr[] = "estack";

				stilo_name_new(tname, a_stilt, keystr,
				    sizeof(keystr) - 1, TRUE);
			}
			count = stils_count(estack);
			stilo_array_new(arr, a_stilt, count);
			for (i = count - 1, stilo = NULL; i >= 0; i--) {
				stilo = stils_down_get(estack, a_stilt, stilo);
				stilo_dup(stilo_array_el_get(arr, a_stilt, i),
				    stilo);
			}
			stilo_dict_def(derror, a_stilt, tname, arr);
			
			/* dstack. */
			{
				static const cw_uint8_t	keystr[] = "dstack";

				stilo_name_new(tname, a_stilt, keystr,
				    sizeof(keystr) - 1, TRUE);
			}
			count = stils_count(dstack);
			stilo_array_new(arr, a_stilt, count);
			for (i = count - 1, stilo = NULL; i >= 0; i--) {
				stilo = stils_down_get(dstack, a_stilt, stilo);
				stilo_dup(stilo_array_el_get(arr, a_stilt, i),
				    stilo);
			}
			stilo_dict_def(derror, a_stilt, tname, arr);
			
			stils_pop(tstack, a_stilt);
		}
		break;
	}
	default:
		_cw_not_reached();
	}
	stils_npop(tstack, a_stilt, 3);
	systemdict_stop(a_stilt);
}
