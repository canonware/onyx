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

#include <sys/time.h>	/* For realtime operator. */
#include <errno.h>

/* Initial size of dictionaries created with the dict operator. */
#define	_CW_SYSTEMDICT_DICT_SIZE	16

struct cw_systemdict_entry {
	cw_stiln_t	stiln;
	cw_op_t		*op_f;
};

#define ENTRY(name)	{STILN_##name, systemdict_##name}

/*
 * Array of operators in systemdict.
 */
static const struct cw_systemdict_entry systemdict_ops[] = {
	ENTRY(abs),
	ENTRY(add),
	ENTRY(aload),
	ENTRY(anchorsearch),
	ENTRY(and),
	ENTRY(array),
	ENTRY(astore),
	ENTRY(begin),
	ENTRY(bind),
	ENTRY(bytesavailable),
	ENTRY(clear),
	ENTRY(cleardictstack),
	ENTRY(cleartomark),
	ENTRY(closefile),
	ENTRY(condition),
	ENTRY(copy),
	ENTRY(count),
	ENTRY(countdictstack),
	ENTRY(countexecstack),
	ENTRY(counttomark),
	ENTRY(currentcontext),
	ENTRY(currentdict),
	ENTRY(currentfile),
	ENTRY(currentglobal),
	ENTRY(cvlit),
	ENTRY(cvn),
	ENTRY(cvrs),
	ENTRY(cvs),
	ENTRY(cvx),
	ENTRY(def),
	ENTRY(deletefile),
	ENTRY(detach),
	ENTRY(dict),
	ENTRY(dictstack),
	ENTRY(div),
	ENTRY(dup),
	ENTRY(end),
	ENTRY(eq),
	ENTRY(exch),
	ENTRY(exec),
	ENTRY(execstack),
	ENTRY(executeonly),
	ENTRY(exit),
	ENTRY(exp),
	ENTRY(file),
	ENTRY(filenameforall),
	ENTRY(fileposition),
	ENTRY(filter),
	ENTRY(flush),
	ENTRY(flushfile),
	ENTRY(for),
	ENTRY(forall),
	ENTRY(fork),
	ENTRY(gcheck),
	ENTRY(ge),
	ENTRY(get),
	ENTRY(getinterval),
	ENTRY(gt),
	ENTRY(handleerror),
	ENTRY(if),
	ENTRY(ifelse),
	ENTRY(index),
	ENTRY(join),
	ENTRY(known),
	ENTRY(le),
	ENTRY(length),
	ENTRY(load),
	ENTRY(lock),
	ENTRY(loop),
	ENTRY(lt),
	ENTRY(mark),
	ENTRY(mod),
	ENTRY(mul),
	ENTRY(ne),
	ENTRY(neg),
	ENTRY(noaccess),
	ENTRY(not),
	ENTRY(notify),
	ENTRY(or),
	ENTRY(pop),
	ENTRY(print),
	ENTRY(product),
	ENTRY(promptstring),
	ENTRY(pstack),
	ENTRY(put),
	ENTRY(putinterval),
	ENTRY(quit),
	ENTRY(rand),
	ENTRY(rcheck),
	ENTRY(read),
	ENTRY(readline),
	ENTRY(readonly),
	ENTRY(readstring),
	ENTRY(realtime),
	ENTRY(renamefile),
	ENTRY(repeat),
	ENTRY(roll),
	ENTRY(run),
	ENTRY(search),
	ENTRY(setfileposition),
	ENTRY(setglobal),
	ENTRY(shift),
	ENTRY(srand),
	ENTRY(stack),
	ENTRY(start),
	ENTRY(status),
	ENTRY(stdin),
	ENTRY(stderr),
	ENTRY(stdout),
	ENTRY(stop),
	ENTRY(stopped),
	ENTRY(store),
	ENTRY(string),
	ENTRY(sub),
	ENTRY(sym_eq),
	ENTRY(sym_eq_eq),
	ENTRY(sym_gt_gt),
	{STILN_sym_lb, systemdict_mark},
	{STILN_sym_lt_lt, systemdict_mark},
	ENTRY(sym_rb),
	ENTRY(timedwait),
	ENTRY(token),
	ENTRY(trylock),
	ENTRY(type),
	ENTRY(undef),
	ENTRY(unlock),
	ENTRY(version),
	ENTRY(wait),
	ENTRY(wcheck),
	ENTRY(where),
	ENTRY(write),
	ENTRY(writestring),
	ENTRY(xcheck),
	ENTRY(xor),
	ENTRY(yield)
};

void
systemdict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_uint32_t	i;
	cw_stilo_t	name, operator;

#define	NEXTRA	5
#define NENTRIES							\
	(sizeof(systemdict_ops) / sizeof(struct cw_systemdict_entry))

	stilo_dict_new(a_dict, a_stilt, NENTRIES + NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stilt,
		    stiln_str(systemdict_ops[i].stiln),
		    stiln_len(systemdict_ops[i].stiln), TRUE);
		stilo_operator_new(&operator, systemdict_ops[i].op_f,
		    systemdict_ops[i].stiln);
		stilo_attrs_set(&operator, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stilt, &name, &operator);
	}

	/* Initialize entries that are not operators. */

	/* globaldict. */
	stilo_name_new(&name, a_stilt, stiln_str(STILN_globaldict),
	    stiln_len(STILN_globaldict), TRUE);
	stilo_dup(&operator, stilt_globaldict_get(a_stilt));
	stilo_dict_def(a_dict, a_stilt, &name, &operator);

	/* systemdict. */
	stilo_name_new(&name, a_stilt, stiln_str(STILN_systemdict),
	    stiln_len(STILN_systemdict), TRUE);
	stilo_dup(&operator, stilt_systemdict_get(a_stilt));
	stilo_dict_def(a_dict, a_stilt, &name, &operator);

	/* true. */
	stilo_name_new(&name, a_stilt, stiln_str(STILN_true),
	    stiln_len(STILN_true), TRUE);
	stilo_boolean_new(&operator, TRUE);
	stilo_dict_def(a_dict, a_stilt, &name, &operator);

	/* false. */
	stilo_name_new(&name, a_stilt, stiln_str(STILN_false),
	    stiln_len(STILN_false), TRUE);
	stilo_boolean_new(&operator, FALSE);
	stilo_dict_def(a_dict, a_stilt, &name, &operator);

	/* null. */
	stilo_name_new(&name, a_stilt, stiln_str(STILN_null),
	    stiln_len(STILN_null), TRUE);
	stilo_null_new(&operator);
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
systemdict_abs(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(a, ostack, a_stilt);
	if (stilo_type_get(a) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	if (stilo_integer_get(a) < 0)
		stilo_integer_set(a, -stilo_integer_get(a));
}

void
systemdict_add(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(b, ostack, a_stilt);
	STILS_DOWN_GET(a, ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) + stilo_integer_get(b));
	stils_pop(ostack);
}

void
systemdict_aload(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*array, *stilo;
	cw_uint32_t	i, len;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(array, ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	for (i = 0, len = stilo_array_len_get(array); i < len; i++) {
		stilo = stils_under_push(ostack, array);
		stilo_dup(stilo, stilo_array_el_get(array, i));
	}
}

void
systemdict_anchorsearch(cw_stilt_t *a_stilt)
{
	_cw_stil_code(a_stilt, "
%/anchorsearch
%{
  exch dup 3 1 roll % Make a copy of the original string.
  search % -string- -seek- search -post- -match- -pre- true
         % -string- -seek- search -string- false
  {
    % Search string found.  Check if it was at the beginning.
    3 2 roll length 0 eq
    {
      % Success.
      3 2 roll pop % Get rid of original string.
      true
    }{
      % Nope.  Clean up
      pop pop
      false
    } ifelse
  }{
    % Search string not found at all.
    pop pop pop
    false
  } ifelse
%} bind def
");
}

void
systemdict_and(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	if (stilo_type_get(stilo_a) == STILOT_BOOLEAN && stilo_type_get(stilo_b)
	    == STILOT_BOOLEAN) {
		cw_bool_t	and;

		if (stilo_boolean_get(stilo_a) && stilo_boolean_get(stilo_b))
			and = TRUE;
		else
			and = FALSE;
		stilo_boolean_new(stilo_a, and);
	} else if (stilo_type_get(stilo_a) == STILOT_INTEGER &&
	    stilo_type_get(stilo_b) == STILOT_INTEGER) {
		stilo_integer_set(stilo_a, stilo_integer_get(stilo_a) &
		    stilo_integer_get(stilo_b));
	} else {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stils_pop(ostack);
}

void
systemdict_array(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	len = stilo_integer_get(stilo);
	if (len < 0) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	stilo_array_new(stilo, a_stilt, len);
}

void
systemdict_astore(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*array, *stilo;
	cw_sint32_t	i, len;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(array, ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	/* Make sure there will be enough objects to fill the array. */
	len = stilo_array_len_get(array);
	if (len > stils_count(ostack) - 1) {
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);
		return;
	}

	stilo = stils_push(tstack);
	stilo_dup(stilo, array);
	stils_pop(ostack);
	array = stilo;

	/* Move ostack objects to the array. */
	for (i = len - 1; i >= 0; i--) {
		stilo_array_el_set(array, stils_get(ostack), i);
		stils_pop(ostack);
	}

	/* Push the array back onto ostack. */
	stilo = stils_push(ostack);
	stilo_dup(stilo, array);
	stils_pop(tstack);
}

void
systemdict_begin(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo, *dict;

	dstack = stilt_dstack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(dict, ostack, a_stilt);
	if (stilo_type_get(dict) != STILOT_DICT) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	
	stilo = stils_push(dstack);
	stilo_dup(stilo, dict);
	stils_pop(ostack);
}

static void
systemdict_p_bind(cw_stilo_t *a_proc, cw_stilt_t *a_stilt)
{
	cw_stils_t	*tstack;
	cw_stilo_t	*el, *val;
	cw_uint32_t	i, count;
	/* Array of fastops.  stilt_loop() must have corresponding handlers. */
	static const struct cw_systemdict_entry fastops[] = {
		ENTRY(add),
		ENTRY(dup),
		ENTRY(exch),
		ENTRY(index),
		ENTRY(pop),
		ENTRY(roll)
	};

	tstack = stilt_tstack_get(a_stilt);

	val = stils_push(tstack);

	stilo_perms_set(a_proc, STILOP_READONLY);

	for (i = 0, count = stilo_array_len_get(a_proc); i < count; i++) {
		el = stilo_array_el_get(a_proc, i);
		if (stilo_attrs_get(el) != STILOA_EXECUTABLE)
			continue;

		switch (stilo_type_get(el)) {
		case STILOT_ARRAY:
			if (stilo_perms_get(el) == STILOP_UNLIMITED)
				systemdict_p_bind(el, a_stilt);
			break;
		case STILOT_NAME:
			if (stilt_dict_stack_search(a_stilt, el, val) ==
			    FALSE) {
				if (stilo_type_get(val) == STILOT_OPERATOR) {
					cw_uint32_t	j;

#define	NFASTOPS							\
	(sizeof(fastops) / sizeof(struct cw_systemdict_entry))

					/*
					 * If val can be converted to a fastop,
					 * do so.
					 */
					for (j = 0; j < NFASTOPS; j++) {
						if (stilo_operator_f(val) ==
						    fastops[j].op_f) {
							stilo_dup(el, val);
							/* XXX API abuse. */
							el->fast_op = TRUE;
							el->op_code =
							    fastops[j].stiln;
							break;
						}
					}
					/*
					 * If val isn't a fastop, still convert
					 * the name to an operator.
					 */
					if (j == NFASTOPS) {
						/* Replace el with val. */
						stilo_dup(el, val);
					}
#undef NFASTOPS
				} else if (stilo_attrs_get(val) !=
				    STILOA_EXECUTABLE) {
					/* Replace el with val. */
					stilo_dup(el, val);
				}
			}
		default:
		}
	}

	stils_pop(tstack);
}

void
systemdict_bind(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*array;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(array, ostack, a_stilt);

	systemdict_p_bind(array, a_stilt);
}

void
systemdict_bytesavailable(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file;
	cw_uint32_t	bytes;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(file, ostack, a_stilt);
	
	if (stilo_type_get(file) != STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	bytes = stilo_file_buffer_count(file);
	stilo_integer_new(file, bytes);
}

void
systemdict_clear(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_uint32_t	count;

	ostack = stilt_ostack_get(a_stilt);
	count = stils_count(ostack);
	if (count > 0)
		stils_npop(ostack, count);
}

void
systemdict_cleardictstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*dstack;
	cw_uint32_t	count;

	dstack = stilt_dstack_get(a_stilt);
	count = stils_count(dstack);
	if (count > 3)
		stils_npop(dstack, count - 3);
}

void
systemdict_cleartomark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_uint32_t	i, depth;

	ostack = stilt_ostack_get(a_stilt);

	for (i = 0, depth = stils_count(ostack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilt_error(a_stilt, STILTE_UNMATCHEDMARK);
		return;
	}

	stils_npop(ostack, i + 1);
}

void
systemdict_closefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	error = stilo_file_close(stilo, a_stilt);
	if (error) {
		stilt_error(a_stilt, STILTE_IOERROR);
		return;
	}

	stils_pop(ostack);
}

void
systemdict_condition(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_copy(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_INTEGER: {
		cw_stilo_t	*dup;
		cw_uint32_t	i;
		cw_sint64_t	count;

		/* Dup a range of the stack. */
		count = stilo_integer_get(stilo);
		if (count < 0) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		if (count > stils_count(ostack) - 1) {
			stilt_error(a_stilt, STILTE_STACKUNDERFLOW);
			return;
		}
		stils_pop(ostack);

		/*
		 * Iterate down the stack, creating dup's along the way.  Since
		 * we're going down, it's necessary to use stils_under_push() to
		 * preserve order.
		 */
		for (i = 0, stilo = NULL, dup = NULL; i < count; i++) {
			stilo = stils_down_get(ostack, stilo);
			dup = stils_under_push(ostack, dup);
			stilo_dup(dup, stilo);
		}
		break;
	}
	case STILOT_ARRAY: {
		cw_stilte_t	error;
		cw_stilo_t	*orig;

		STILS_DOWN_GET(orig, ostack, a_stilt, stilo);
		if (stilo_type_get(orig) != STILOT_ARRAY) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}

		error = stilo_array_copy(stilo, orig, a_stilt);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	case STILOT_DICT: {
		cw_stilte_t	error;
		cw_stilo_t	*orig;

		STILS_DOWN_GET(orig, ostack, a_stilt, stilo);
		if (stilo_type_get(orig) != STILOT_DICT) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}

		error = stilo_dict_copy(stilo, orig, a_stilt);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	case STILOT_HOOK: {
		cw_stilte_t	error;
		cw_stilo_t	*orig;

		STILS_DOWN_GET(orig, ostack, a_stilt, stilo);
		if (stilo_type_get(orig) != STILOT_HOOK) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}

		error = stilo_hook_copy(stilo, orig, a_stilt);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	case STILOT_STRING: {
		cw_stilte_t	error;
		cw_stilo_t	*orig;

		STILS_DOWN_GET(orig, ostack, a_stilt, stilo);
		if (stilo_type_get(orig) != STILOT_STRING) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}

		error = stilo_string_copy(stilo, orig, a_stilt);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
}

void
systemdict_count(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_integer_new(stilo, stils_count(ostack) - 1);
}

void
systemdict_countdictstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo;

	dstack = stilt_dstack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_integer_new(stilo, stils_count(dstack));
}

void
systemdict_countexecstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*stilo;

	estack = stilt_estack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_integer_new(stilo, stils_count(estack));
}

void
systemdict_counttomark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_uint32_t	i, depth;

	ostack = stilt_ostack_get(a_stilt);

	for (i = 0, depth = stils_count(ostack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilt_error(a_stilt, STILTE_UNMATCHEDMARK);
		return;
	}

	stilo = stils_push(ostack);
	stilo_integer_new(stilo, i);
}

void
systemdict_currentcontext(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_currentdict(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_dup(stilo, stils_get(dstack));
}

void
systemdict_currentfile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*file, *stilo;
	cw_uint32_t	i, depth;

	estack = stilt_estack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	file = stils_push(ostack);
	for (i = 0, depth = stils_count(estack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(estack, stilo);
		if (stilo_type_get(stilo) == STILOT_FILE) {
			stilo_dup(file, stilo);
			break;
		}
	}
	if (i == depth)
		stilo_file_new(file, a_stilt);
}

void
systemdict_currentglobal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack);
	stilo_boolean_new(stilo, stilt_currentglobal(a_stilt));
}

void
systemdict_cvlit(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	stilo_attrs_set(stilo, STILOA_LITERAL);
}

void
systemdict_cvn(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*stilo, *tstilo;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	if (stilo_type_get(stilo) != STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	tstilo = stils_push(tstack);
	stilo_dup(tstilo, stilo);

	stilo_name_new(stilo, a_stilt, stilo_string_get(tstilo),
	    stilo_string_len_get(tstilo), FALSE);
	stilo_attrs_set(stilo, stilo_attrs_get(tstilo));

	stils_pop(tstack);
}

void
systemdict_cvrs(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*num, *radix, *string, *tstring;
	cw_stilte_t	error;
	cw_uint64_t	val;
	cw_uint32_t	i, rlen, base;
	cw_uint8_t	*str;
	static const cw_uint8_t *syms = "0123456789abcdefghijklmnopqrstuvwxyz";
	cw_uint8_t	*result, s_result[65] =
	    "0000000000000000000000000000000000000000000000000000000000000000";

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	STILS_GET(string, ostack, a_stilt);
	STILS_DOWN_GET(radix, ostack, a_stilt, string);
	STILS_DOWN_GET(num, ostack, a_stilt, radix);
	if ((stilo_type_get(num) != STILOT_INTEGER) || (stilo_type_get(radix) !=
	    STILOT_INTEGER) || (stilo_type_get(string) != STILOT_STRING)) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	base = stilo_integer_get(radix);
	if (base < 2 || base > 36) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}
	val = (cw_uint64_t)stilo_integer_get(num);

	/*
	 * Handle radix 16 specially, since it can be done quickly, and is
	 * commonly used.
	 */
	result = s_result;
	if (base == 16) {
		for (i = 63; val != 0; i--) {
			result[i] = syms[val & 0xf];
			val >>= 4;
		}
	} else {
		for (i = 63; val != 0; i--) {
			result[i] = syms[val % base];
			val /= base;
		}
	}
	result += i + 1;
	rlen = 64 - (result - s_result);

	tstring = stils_push(tstack);
	stilo_dup(tstring, string);
	error = stilo_string_substring_new(string, tstring, a_stilt, 0, rlen);
	if (error) {
		stils_pop(tstack);
		stilt_error(a_stilt, error);
		return;
	}
	stils_pop(tstack);

	str = stilo_string_get(string);
	memcpy(str, result, rlen);

	stils_roll(ostack, 3, 1);
	stils_npop(ostack, 2);
}

void
systemdict_cvs(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	
	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_BOOLEAN:
	case STILOT_INTEGER:
		_cw_stil_code(a_stilt, "pop (XXX)");
		break;
	case STILOT_NAME: {
		cw_stils_t	*tstack;
		cw_stilo_t	*tstilo;

		tstack = stilt_tstack_get(a_stilt);
		tstilo = stils_push(tstack);
		stilo_dup(tstilo, stilo);

		stilo_string_new(stilo, a_stilt, stilo_name_len_get(tstilo));
		stilo_string_set(stilo, 0, stilo_name_str_get(tstilo),
		    stilo_name_len_get(tstilo));

		stils_pop(tstack);
		break;
	}
	case STILOT_OPERATOR:
		_cw_stil_code(a_stilt, "pop (XXX)");
		break;
	case STILOT_STRING:
		break;
	case STILOT_NO:
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_MARK:
	case STILOT_NULL:
	default:
		_cw_stil_code(a_stilt, "pop (--nostringval--)");
		break;
	}
}

void
systemdict_cvx(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	stilo_attrs_set(stilo, STILOA_EXECUTABLE);
}

void
systemdict_def(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*dict, *key, *val;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);

	dict = stils_get(dstack);
	STILS_GET(val, ostack, a_stilt);
	STILS_DOWN_GET(key, ostack, a_stilt, val);

	stilo_dict_def(dict, a_stilt, key, val);

	stils_npop(ostack, 2);
}

void
systemdict_deletefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*string;
	cw_uint8_t	str[PATH_MAX];
	cw_uint32_t	nbytes;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(string, ostack, a_stilt);

	if (stilo_type_get(string) != STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	if (stilo_string_len_get(string) < sizeof(str))
		nbytes = stilo_string_len_get(string);
	else
		nbytes = sizeof(str) - 1;
	memcpy(str, stilo_string_get(string), nbytes);
	str[nbytes] = '\0';

	stils_pop(ostack);

	if (unlink(str) == -1) {
		switch (errno) {
		case EACCES:
		case EPERM:
			stilt_error(a_stilt, STILTE_INVALIDFILEACCESS);
		case EIO:
		case EBUSY:
			stilt_error(a_stilt, STILTE_IOERROR);
		default:
			stilt_error(a_stilt, STILTE_UNDEFINEDFILENAME);
		}
		return;
	}
}

void
systemdict_detach(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_dict(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*dict;

	ostack = stilt_ostack_get(a_stilt);

	dict = stils_push(ostack);
	stilo_dict_new(dict, a_stilt, _CW_SYSTEMDICT_DICT_SIZE);
}

void
systemdict_dictstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack, *tstack;
	cw_stilo_t	*array, *subarray, *stilo;
	cw_stilte_t	error;
	cw_sint32_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(array, ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	count = stils_count(dstack);
	if (count > stilo_array_len_get(array)) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	/* Move array to tstack, create subarray, and pop array. */
	stilo = stils_push(tstack);
	stilo_dup(stilo, array);
	subarray = array;
	array = stilo;
	error = stilo_array_subarray_new(subarray, array, a_stilt, 0, count);
	if (error) {
		stils_pop(tstack);
		stilt_error(a_stilt, error);
		return;
	}
	stils_pop(tstack);

	/* Copy dstack to subarray. */
	for (i = count - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(dstack, stilo);
		stilo_dup(stilo_array_el_get(subarray, i), stilo);
	}
}

void
systemdict_div(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(b, ostack, a_stilt);
	STILS_DOWN_GET(a, ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(b) == 0) {
		stilt_error(a_stilt, STILTE_UNDEFINEDRESULT);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) / stilo_integer_get(b));
	stils_pop(ostack);
}

void
systemdict_dup(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*orig, *dup;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(orig, ostack, a_stilt);
	dup = stils_push(ostack);
	stilo_dup(dup, orig);
}

void
systemdict_end(cw_stilt_t *a_stilt)
{
	cw_stils_t	*dstack;

	dstack = stilt_dstack_get(a_stilt);

	/* threaddict, systemdict, globaldict, and userdict cannot be popped. */
	if (stils_count(dstack) <= 4) {
		stilt_error(a_stilt, STILTE_DICTSTACKUNDERFLOW);
		return;
	}

	stils_pop(dstack);
}

void
systemdict_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_sint32_t	result;
	cw_bool_t	eq;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	result = stilo_compare(stilo_a, stilo_b, a_stilt);
	if (result == 0)
		eq = TRUE;
	else
		eq = FALSE;

	stilo_boolean_new(stilo_a, eq);

	stils_pop(ostack);
}

void
systemdict_exch(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;

	ostack = stilt_ostack_get(a_stilt);

	if (stils_count(ostack) < 2) {
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);
		return;
	}

	stils_roll(ostack, 2, 1);
}

void
systemdict_exec(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*orig, *new;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);

	STILS_GET(orig, ostack, a_stilt);
	new = stils_push(estack);
	stilo_dup(new, orig);
	stils_pop(ostack);

	stilt_loop(a_stilt);
}

void
systemdict_execstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*array, *subarray, *stilo;
	cw_stilte_t	error;
	cw_sint32_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(array, ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	count = stils_count(estack);
	if (count > stilo_array_len_get(array)) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	/* Move array to tstack, create subarray, and pop array. */
	stilo = stils_push(tstack);
	stilo_dup(stilo, array);
	subarray = array;
	array = stilo;
	
	error = stilo_array_subarray_new(subarray, array, a_stilt, 0, count);
	if (error) {
		stils_pop(tstack);
		stilt_error(a_stilt, error);
		return;
	}
	stils_pop(tstack);

	/* Copy estack to subarray. */
	for (i = count - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(estack, stilo);
		stilo_dup(stilo_array_el_get(subarray, i), stilo);
	}
}

void
systemdict_executeonly(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_STRING: {
		cw_stilte_t	error;

		error = stilo_perms_set(stilo, STILOP_EXECUTEONLY);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
}

void
systemdict_exit(cw_stilt_t *a_stilt)
{
	xep_throw(_CW_STILX_EXIT);
}

void
systemdict_exp(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a, *b;
	cw_uint32_t	i;
	cw_sint64_t	r;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(b, ostack, a_stilt);
	STILS_DOWN_GET(a, ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	for (i = 0, r = stilo_integer_get(a); i < stilo_integer_get(b); i++)
		r *= stilo_integer_get(a);
	stilo_integer_set(a, r);
	stils_pop(ostack);
}

void
systemdict_file(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*name, *flags, *file;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	
	STILS_GET(flags, ostack, a_stilt);
	STILS_DOWN_GET(name, ostack, a_stilt, flags);
	if (stilo_type_get(name) != STILOT_STRING || stilo_type_get(flags) !=
	    STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	file = stils_push(tstack);
	stilo_file_new(file, a_stilt);
	error = stilo_file_open(file, stilo_string_get(name),
	    stilo_string_len_get(name), stilo_string_get(flags),
	    stilo_string_len_get(flags));
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	/* XXX Magic number. */
	stilo_file_buffer_size_set(file, 512);

	stils_pop(ostack);
	stilo_dup(name, file);
	stils_pop(tstack);
}

void
systemdict_filenameforall(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_fileposition(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file;
	cw_sint64_t	position;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(file, ostack, a_stilt);
	
	if (stilo_type_get(file) != STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	position = stilo_file_position_get(file);
	if (position == -1) {
		stilt_error(a_stilt, STILTE_IOERROR);
		return;
	}
	stilo_integer_new(file, position);
}

void
systemdict_filter(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_flush(cw_stilt_t *a_stilt)
{
	cw_stilte_t	error;

	error = stilo_file_buffer_flush(stilt_stdout_get(a_stilt), a_stilt);
	if (error)
		stilt_error(a_stilt, error);
}

void
systemdict_flushfile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(file, ostack, a_stilt);
	
	if (stilo_type_get(file) != STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	error = stilo_file_buffer_flush(file, a_stilt);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}
	
	stils_pop(ostack);
}

void
systemdict_for(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*exec, *ostilo, *estilo, *tstilo;
	cw_sint64_t	i, inc, limit, edepth;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(exec, ostack, a_stilt);

	STILS_DOWN_GET(ostilo, ostack, a_stilt, exec);
	if (stilo_type_get(ostilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	limit = stilo_integer_get(ostilo);

	STILS_DOWN_GET(ostilo, ostack, a_stilt, ostilo);
	if (stilo_type_get(ostilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	inc = stilo_integer_get(ostilo);

	STILS_DOWN_GET(ostilo, ostack, a_stilt, ostilo);
	if (stilo_type_get(ostilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	i = stilo_integer_get(ostilo);

	/* Move the object to be executed to tstack. */
	tstilo = stils_push(tstack);
	stilo_dup(tstilo, exec);
	stils_npop(ostack, 4);

	/* Record estack's depth so that we can clean up estack if necessary. */
	edepth = stils_count(estack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		if (limit >= 0) {
			for (; i <= limit; i += inc) {
				/*
				 * Dup the object to execute onto the execution
				 * stack.
				 */
				estilo = stils_push(estack);
				stilo_dup(estilo, tstilo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				ostilo = stils_push(ostack);
				stilo_integer_new(ostilo, i);

				stilt_loop(a_stilt);
			}
		} else {
			for (; i >= limit; i += inc) {
				/*
				 * Dup the object to execute onto the execution
				 * stack.
				 */
				estilo = stils_push(estack);
				stilo_dup(estilo, tstilo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				ostilo = stils_push(ostack);
				stilo_integer_new(ostilo, i);

				stilt_loop(a_stilt);
			}
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		for (i = stils_count(estack); i > edepth; i--)
			stils_pop(estack);
	}
	xep_end();

	stils_pop(tstack);
}

void
systemdict_forall(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*stilo, *what, *proc;
	cw_sint64_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(proc, ostack, a_stilt);
	STILS_DOWN_GET(what, ostack, a_stilt, proc);

	xep_begin();
	xep_try {
		switch (stilo_type_get(what)) {
		case STILOT_ARRAY: {
			cw_stilo_t	*el;

			/* Move proc and array to tstack. */
			stilo = stils_push(tstack);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stils_push(tstack);
			stilo_dup(stilo, what);
			what = stilo;

			stils_npop(ostack, 2);

			/*
			 * Iterate through the array, push each element onto
			 * ostack, and execute proc.
			 */
			for (i = 0, count = stilo_array_len_get(what); i <
				 count; i++) {
				el = stilo_array_el_get(what, i);
				stilo = stils_push(ostack);
				stilo_dup(stilo, el);

				stilo = stils_push(estack);
				stilo_dup(stilo, proc);
				stilt_loop(a_stilt);
			}
			break;
		}
		case STILOT_DICT: {
			cw_stilo_t	*key, *val;

			/* Move proc and array to tstack. */
			stilo = stils_push(tstack);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stils_push(tstack);
			stilo_dup(stilo, what);
			what = stilo;

			stils_npop(ostack, 2);

			for (i = 0, count = stilo_dict_count(what); i <
				 count; i++) {
				/* Push key and val onto ostack. */
				key = stils_push(ostack);
				val = stils_push(ostack);

				/* Get next key. */
				stilo_dict_iterate(what, a_stilt, key);

				/* Use key to get val. */
				stilo_dict_lookup(what, a_stilt, key, val);

				/* Push proc onto estack and execute it. */
				stilo = stils_push(estack);
				stilo_dup(stilo, proc);
				stilt_loop(a_stilt);
			}
			break;
		}
		case STILOT_STRING: {
			cw_uint8_t	el;

			/* Move proc and array to tstack. */
			stilo = stils_push(tstack);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stils_push(tstack);
			stilo_dup(stilo, what);
			what = stilo;

			stils_npop(ostack, 2);

			/*
			 * Iterate through the string, push each element onto
			 * ostack, and execute proc.
			 */
			for (i = 0, count = stilo_array_len_get(what); i <
				 count; i++) {
				el = *stilo_string_el_get(what, i);
				stilo = stils_push(ostack);
				stilo_integer_new(stilo, (cw_sint64_t)el);

				stilo = stils_push(estack);
				stilo_dup(stilo, proc);
				stilt_loop(a_stilt);
			}
			break;
		}

		default:
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up tstack. */
		stils_npop(tstack, 2);
	}
	xep_end();
}

void
systemdict_fork(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_gcheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_ge(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_stilot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	ge;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	type_a = stilo_type_get(stilo_a);
	type_b = stilo_type_get(stilo_b);
	if (type_a != type_b || (type_a != STILOT_INTEGER && type_a !=
	    STILOT_STRING) || (type_b != STILOT_INTEGER && type_b !=
	    STILOT_STRING)) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	result = stilo_compare(stilo_a, stilo_b, a_stilt);
	if (result >= 0)
		ge = TRUE;
	else
		ge = FALSE;

	stilo_boolean_new(stilo_a, ge);

	stils_pop(ostack);
}

void
systemdict_get(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*from, *with;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(with, ostack, a_stilt);
	STILS_DOWN_GET(from, ostack, a_stilt, with);

	switch (stilo_type_get(from)) {
	case STILOT_ARRAY: {
		cw_sint64_t	index;
		cw_stilo_t	*el;

		if (stilo_type_get(with) != STILOT_INTEGER) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);

		el = stilo_array_el_get(from, index);
		if (el == NULL) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		stilo_dup(with, el);

		stils_roll(ostack, 2, 1);
		stils_pop(ostack);
		break;
	}
	case STILOT_DICT: {
		cw_stilo_t	*val;

		val = stils_push(ostack);
		if (stilo_dict_lookup(from, a_stilt, with, val)) {
			stils_pop(ostack);
			stilt_error(a_stilt, STILTE_UNDEFINED);
			return;
		}
		stils_roll(ostack, 3, 1);
		stils_npop(ostack, 2);
		break;
	}
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING: {
		cw_sint64_t	index;
		cw_uint8_t	*el;

		if (stilo_type_get(with) != STILOT_INTEGER) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);

		el = stilo_string_el_get(from, index);
		if (el == NULL) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		stilo_integer_set(with, (cw_sint64_t)*el);

		stils_roll(ostack, 2, 1);
		stils_pop(ostack);
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
}

void
systemdict_getinterval(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*from, *with, *count;
	cw_stilte_t	error;
	cw_sint64_t	index, len;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(count, ostack, a_stilt);
	STILS_DOWN_GET(with, ostack, a_stilt, count);
	STILS_DOWN_GET(from, ostack, a_stilt, with);

	if ((stilo_type_get(with) != STILOT_INTEGER) || (stilo_type_get(count)
	    != STILOT_INTEGER)) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	index = stilo_integer_get(with);
	len = stilo_integer_get(count);

	switch (stilo_type_get(from)) {
	case STILOT_ARRAY:
		error = stilo_array_subarray_new(count, from, a_stilt, index,
		    len);
		break;
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING:
		error = stilo_string_substring_new(count, from, a_stilt, index,
		    len);
		break;
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_roll(ostack, 3, 1);
	stils_npop(ostack, 2);
}

void
systemdict_gt(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_stilot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	gt;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	type_a = stilo_type_get(stilo_a);
	type_b = stilo_type_get(stilo_b);
	if (type_a != type_b || (type_a != STILOT_INTEGER && type_a !=
	    STILOT_STRING) || (type_b != STILOT_INTEGER && type_b !=
	    STILOT_STRING)) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	result = stilo_compare(stilo_a, stilo_b, a_stilt);
	if (result == 1)
		gt = TRUE;
	else
		gt = FALSE;

	stilo_boolean_new(stilo_a, gt);

	stils_pop(ostack);
}

void
systemdict_handleerror(cw_stilt_t *a_stilt)
{
	cw_stils_t	*estack, *tstack;
	cw_stilo_t	*key, *errordict, *handleerror;

	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	/* Get errordict. */
	errordict = stils_push(tstack);
	key = stils_push(tstack);
	stilo_name_new(key, a_stilt, stiln_str(STILN_errordict),
	    stiln_len(STILN_errordict), TRUE);
	if (stilt_dict_stack_search(a_stilt, key, errordict)) {
		stils_npop(tstack, 2);
		xep_throw(_CW_STILX_ERRORDICT);
	}

	/* Get handleerror from errordict and push it onto estack. */
	handleerror = stils_push(estack);
	stilo_name_new(key, a_stilt, stiln_str(STILN_handleerror),
	    stiln_len(STILN_handleerror), TRUE);
	if (stilo_dict_lookup(errordict, a_stilt, key, handleerror)) {
		stils_pop(estack);
		stils_npop(tstack, 2);
		xep_throw(_CW_STILX_ERRORDICT);
	}
	stils_npop(tstack, 2);

	/* Execute handleerror. */
	stilt_loop(a_stilt);
}

void
systemdict_if(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*cond, *exec;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(exec, ostack, a_stilt);
	STILS_DOWN_GET(cond, ostack, a_stilt, exec);
	if (stilo_type_get(cond) != STILOT_BOOLEAN) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	if (stilo_boolean_get(cond)) {
		cw_stils_t	*estack;
		cw_stilo_t	*stilo;

		estack = stilt_estack_get(a_stilt);
		stilo = stils_push(estack);
		stilo_dup(stilo, exec);
		stils_npop(ostack, 2);
		stilt_loop(a_stilt);
	} else
		stils_npop(ostack, 2);
}

void
systemdict_ifelse(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*cond, *exec_if, *exec_else, *stilo;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);

	STILS_GET(exec_else, ostack, a_stilt);

	STILS_DOWN_GET(exec_if, ostack, a_stilt, exec_else);

	STILS_DOWN_GET(cond, ostack, a_stilt, exec_if);
	if (stilo_type_get(cond) != STILOT_BOOLEAN) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo = stils_push(estack);
	if (stilo_boolean_get(cond))
		stilo_dup(stilo, exec_if);
	else
		stilo_dup(stilo, exec_else);
	stils_npop(ostack, 3);
	stilt_loop(a_stilt);
}

void
systemdict_index(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *orig;
	cw_sint64_t	index;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	index = stilo_integer_get(stilo);
	if (index < 0) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	STILS_NGET(orig, ostack, a_stilt, index + 1);
	stilo_dup(stilo, orig);
}

void
systemdict_join(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_known(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*dict, *key;
	cw_bool_t	known;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(key, ostack, a_stilt);
	STILS_DOWN_GET(dict, ostack, a_stilt, key);
	if (stilo_type_get(dict) != STILOT_DICT) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	known = !stilo_dict_lookup(dict, a_stilt, key, NULL);
	stilo_boolean_new(dict, known);

	stils_pop(ostack);
}

void
systemdict_le(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_stilot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	le;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	type_a = stilo_type_get(stilo_a);
	type_b = stilo_type_get(stilo_b);
	if (type_a != type_b || (type_a != STILOT_INTEGER && type_a !=
	    STILOT_STRING) || (type_b != STILOT_INTEGER && type_b !=
	    STILOT_STRING)) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	result = stilo_compare(stilo_a, stilo_b, a_stilt);
	if (result <= 0)
		le = TRUE;
	else
		le = FALSE;

	stilo_boolean_new(stilo_a, le);

	stils_pop(ostack);
}

void
systemdict_length(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(stilo, ostack, a_stilt);
	switch (stilo_type_get(stilo)) {
	case STILOT_ARRAY:
		len = stilo_array_len_get(stilo);
		break;
	case STILOT_DICT:
		len = stilo_dict_count(stilo);
		break;
	case STILOT_NAME:
		len = stilo_name_len_get(stilo);
		break;
	case STILOT_STRING:
		len = stilo_string_len_get(stilo);
		break;
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_integer_new(stilo, len);
}

void
systemdict_load(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*key, *val;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(key, ostack, a_stilt);
	val = stils_push(tstack);
	
	if (stilt_dict_stack_search(a_stilt, key, val)) {
		stilt_error(a_stilt, STILTE_UNDEFINED);
		return;
	}
	stilo_dup(key, val);
	stils_pop(tstack);
}

void
systemdict_lock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_loop(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*exec, *stilo, *tstilo;
	cw_uint32_t	sdepth;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(exec, ostack, a_stilt);

	/* Move the object to be executed to tstack. */
	tstilo = stils_push(tstack);
	stilo_dup(tstilo, exec);
	stils_pop(ostack);

	sdepth = stils_count(estack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		for (;;) {
			stilo = stils_push(estack);
			stilo_dup(stilo, tstilo);
			stilt_loop(a_stilt);
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		cw_uint32_t	i;

		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		for (i = stils_count(estack); i > sdepth; i--)
			stils_pop(estack);
	}
	xep_end();

	stils_pop(estack);
	stils_pop(tstack);
}

void
systemdict_lt(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_stilot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	lt;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	type_a = stilo_type_get(stilo_a);
	type_b = stilo_type_get(stilo_b);
	if (type_a != type_b || (type_a != STILOT_INTEGER && type_a !=
	    STILOT_STRING) || (type_b != STILOT_INTEGER && type_b !=
	    STILOT_STRING)) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	result = stilo_compare(stilo_a, stilo_b, a_stilt);
	if (result == -1)
		lt = TRUE;
	else
		lt = FALSE;

	stilo_boolean_new(stilo_a, lt);

	stils_pop(ostack);
}

void
systemdict_mark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack);
	stilo_mark_new(stilo);
}

void
systemdict_mod(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(b, ostack, a_stilt);
	STILS_DOWN_GET(a, ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(b) == 0) {
		stilt_error(a_stilt, STILTE_UNDEFINEDRESULT);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) % stilo_integer_get(b));
	stils_pop(ostack);
}

void
systemdict_mul(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(b, ostack, a_stilt);
	STILS_DOWN_GET(a, ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) * stilo_integer_get(b));
	stils_pop(ostack);
}

void
systemdict_ne(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_sint32_t	result;
	cw_bool_t	ne;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	result = stilo_compare(stilo_a, stilo_b, a_stilt);
	if (result == 0)
		ne = FALSE;
	else
		ne = TRUE;

	stilo_boolean_new(stilo_a, ne);

	stils_pop(ostack);
}

void
systemdict_neg(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(a, ostack, a_stilt);
	if (stilo_type_get(a) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_integer_set(a, -stilo_integer_get(a));
}

void
systemdict_noaccess(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_STRING: {
		cw_stilte_t	error;

		error = stilo_perms_set(stilo, STILOP_NONE);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
}

void
systemdict_not(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo, ostack, a_stilt);

	if (stilo_type_get(stilo) == STILOT_BOOLEAN)
		stilo_boolean_set(stilo, !stilo_boolean_get(stilo));
	else if (stilo_type_get(stilo) == STILOT_INTEGER)
		stilo_integer_set(stilo, ~stilo_integer_get(stilo));
	else
		stilt_error(a_stilt, STILTE_TYPECHECK);
}

void
systemdict_notify(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_or(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	if (stilo_type_get(stilo_a) == STILOT_BOOLEAN && stilo_type_get(stilo_b)
	    == STILOT_BOOLEAN) {
		cw_bool_t	or;

		if (stilo_boolean_get(stilo_a) || stilo_boolean_get(stilo_b))
			or = TRUE;
		else
			or = FALSE;
		stilo_boolean_new(stilo_a, or);
	} else if (stilo_type_get(stilo_a) == STILOT_INTEGER &&
	    stilo_type_get(stilo_b) == STILOT_INTEGER) {
		stilo_integer_set(stilo_a, stilo_integer_get(stilo_a) |
		    stilo_integer_get(stilo_b));
	} else {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stils_pop(ostack);
}

void
systemdict_pop(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;

	ostack = stilt_ostack_get(a_stilt);

	STILS_POP(ostack, a_stilt);
}

void
systemdict_print(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *stdout_stilo;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	error = stilo_file_write(stdout_stilo, a_stilt, stilo_string_get(stilo),
	    stilo_string_len_get(stilo));
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_pop(ostack);
}

void
systemdict_product(cw_stilt_t *a_stilt)
{
	_cw_stil_code(a_stilt, "(Canonware stil)");
}

void
systemdict_promptstring(cw_stilt_t *a_stilt)
{
	_cw_stil_code(a_stilt, "(s> )");
}

void
systemdict_pstack(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_stils_t	*ostack;
	cw_uint32_t	i, count;
	cw_uint8_t	code[] = "==";

	stilts_new(&stilts, a_stilt);
	ostack = stilt_ostack_get(a_stilt);
	count = stils_count(ostack);

	for (i = 0; i < count; i++) {
		systemdict_dup(a_stilt);
		stilt_interpret(a_stilt, &stilts, code, sizeof(code) - 1);
		stilt_flush(a_stilt, &stilts);
		stils_roll(ostack, count, 1);
	}
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_put(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*into, *with, *what;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(what, ostack, a_stilt);
	STILS_DOWN_GET(with, ostack, a_stilt, what);
	STILS_DOWN_GET(into, ostack, a_stilt, with);

	switch (stilo_type_get(into)) {
	case STILOT_ARRAY: {
		cw_sint64_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);

		error = stilo_array_el_set(into, what, index);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	case STILOT_DICT: {
		stilo_dict_def(into, a_stilt, with, what);
		break;
	}
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING: {
		cw_sint64_t	index;
		cw_uint8_t	val;

		if ((stilo_type_get(with) != STILOT_INTEGER) ||
		    stilo_type_get(what) != STILOT_INTEGER) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);
		val = stilo_integer_get(what);
		
		error = stilo_string_el_set(into, val, index);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	stils_npop(ostack, 3);
}

void
systemdict_putinterval(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*into, *with, *what;
	cw_stilte_t	error;
	cw_sint64_t	index;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(what, ostack, a_stilt);
	STILS_DOWN_GET(with, ostack, a_stilt, what);
	STILS_DOWN_GET(into, ostack, a_stilt, with);

	if (stilo_type_get(with) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	index = stilo_integer_get(with);

	switch (stilo_type_get(into)) {
	case STILOT_ARRAY: {
		cw_stilo_t	*arr;
		cw_uint32_t	len;

		arr = stilo_array_get(what);
		len = stilo_array_len_get(what);
		error = stilo_array_set(into, index, arr, len);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING: {
		cw_uint8_t	*str;
		cw_uint32_t	len;

		str = stilo_string_get(what);
		len = stilo_string_len_get(what);
		error = stilo_string_set(into, index, str, len);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	stils_npop(ostack, 3);
}

void
systemdict_quit(cw_stilt_t *a_stilt)
{
	xep_throw(_CW_STILX_QUIT);
}

void
systemdict_rand(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*num;

	ostack = stilt_ostack_get(a_stilt);

	num = stils_push(ostack);
	stilo_integer_new(num, random());
}

void
systemdict_rcheck(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_STRING: {
		cw_stilop_t	perms;
		cw_bool_t	result;

		perms = stilo_perms_get(stilo);
		if (perms < STILOP_EXECUTEONLY)
			result = TRUE;
		else
			result = FALSE;
		stilo_boolean_new(stilo, result);

		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
}

void
systemdict_read(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *value, *code;
	cw_uint8_t	val;
	cw_sint32_t	nread;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(file, ostack, a_stilt);
	if (stilo_type_get(file) != STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	value = stils_push(ostack);
	code = stils_push(ostack);

	nread = stilo_file_read(file, a_stilt, 1, &val);
	if (nread == -1) {
		stilt_error(a_stilt, STILTE_IOERROR);
		return;
	}

	if (nread == 0) {
		stilo_integer_new(value, 0);
		stilo_boolean_new(code, FALSE);
	} else {
		stilo_integer_new(value, (cw_sint64_t)val);
		stilo_boolean_new(code, TRUE);
	}

	stils_roll(ostack, 3, 2);
	stils_pop(ostack);
}

void
systemdict_readline(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*stilo, *tfile;
	cw_stilte_t	error;
	cw_bool_t	eof;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	tfile = stils_push(tstack);
	stilo_dup(tfile, stilo);
	error = stilo_file_readline(tfile, a_stilt, stilo, &eof);
	if (error) {
		stils_pop(tstack);
		stilt_error(a_stilt, error);
		return;
	}
	stils_pop(tstack);

	stilo = stils_push(ostack);
	stilo_boolean_new(stilo, eof);
}

void
systemdict_readonly(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_STRING: {
		cw_stilte_t	error;

		error = stilo_perms_set(stilo, STILOP_READONLY);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
}

void
systemdict_readstring(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *string;
	cw_sint32_t	nread;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(string, ostack, a_stilt);
	STILS_DOWN_GET(file, ostack, a_stilt, string);

	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(string) !=
	    STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	nread = stilo_file_read(file, a_stilt, stilo_string_len_get(string),
	    stilo_string_get(string));
	if (nread == -1) {
		stilt_error(a_stilt, STILTE_IOERROR);
		return;
	}

	if (nread == 0) {
		/* EOF. */
		stilo_boolean_new(file, TRUE);
		stils_pop(ostack);
	} else if (nread < stilo_string_len_get(string)) {
		cw_stilo_t	*value, *code;

		/*
		 * We didn't fill the string, so we can't just use it as the
		 * result.  Create a copy.
		 */
		value = stils_under_push(ostack, file);
		stilo_string_substring_new(value, string, a_stilt, 0, nread);
		code = stils_under_push(ostack, file);
		stilo_boolean_new(code, FALSE);

		stils_npop(ostack, 2);
	} else {
		stilo_boolean_new(file, FALSE);
		stils_roll(ostack, 2, 1);
		stils_pop(ostack);
	}
}

void
systemdict_realtime(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	struct timeval	tv;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack);

	gettimeofday(&tv, NULL);
	stilo_integer_new(stilo, ((cw_sint64_t)tv.tv_sec *
	    (cw_sint64_t)1000000) + (cw_sint64_t)tv.tv_usec);
}

void
systemdict_renamefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*string_from, *string_to;
	cw_uint8_t	str_from[PATH_MAX], str_to[1024];
	cw_uint32_t	nbytes;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(string_to, ostack, a_stilt);
	STILS_DOWN_GET(string_from, ostack, a_stilt, string_to);

	if (stilo_type_get(string_from) != STILOT_STRING ||
	    stilo_type_get(string_to) != STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	if (stilo_string_len_get(string_from) >= sizeof(str_from)) {
		stilt_error(a_stilt, STILTE_LIMITCHECK);
		return;
	}
	nbytes = stilo_string_len_get(string_from);
	memcpy(str_from, stilo_string_get(string_from), nbytes);
	str_from[nbytes] = '\0';

	if (stilo_string_len_get(string_to) >= sizeof(str_to)) {
		stilt_error(a_stilt, STILTE_LIMITCHECK);
		return;
	}
	nbytes = stilo_string_len_get(string_to);
	memcpy(str_to, stilo_string_get(string_to), nbytes);
	str_to[nbytes] = '\0';

	if (rename(str_from, str_to) == -1) {
		switch (errno) {
		case EACCES:
		case EPERM:
		case EROFS:
		case EINVAL:
			stilt_error(a_stilt, STILTE_INVALIDFILEACCESS);
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			stilt_error(a_stilt, STILTE_UNDEFINEDFILENAME);
		default:
			stilt_error(a_stilt, STILTE_IOERROR);
		}
		return;
	}

	stils_npop(ostack, 2);
}

void
systemdict_repeat(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*count, *exec, *stilo, *tstilo;
	cw_sint64_t	i, cnt;
	cw_uint32_t	sdepth;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	STILS_GET(exec, ostack, a_stilt);
	STILS_DOWN_GET(count, ostack, a_stilt, exec);
	if (stilo_type_get(count) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	tstilo = stils_push(tstack);
	stilo_dup(tstilo, exec);

	cnt = stilo_integer_get(count);
	stils_npop(ostack, 2);

	sdepth = stils_count(estack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		for (i = 0; i < cnt; i++) {
			stilo = stils_push(estack);
			stilo_dup(stilo, tstilo);
			stilt_loop(a_stilt);
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		for (i = stils_count(estack); i > sdepth; i--)
			stils_pop(estack);
	}
	xep_end();

	stils_pop(tstack);
}

void
systemdict_roll(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_sint64_t	count, amount;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	amount = stilo_integer_get(stilo);
	STILS_DOWN_GET(stilo, ostack, a_stilt, stilo);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	count = stilo_integer_get(stilo);
	if (count < 1) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}
	if (count > stils_count(ostack) - 2) {
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);
		return;
	}

	stils_npop(ostack, 2);
	stils_roll(ostack, count, amount);
}

void
systemdict_run(cw_stilt_t *a_stilt)
{
	_cw_stil_code(a_stilt, "(r) file cvx exec");
}

void
systemdict_search(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_setfileposition(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *position;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(position, ostack, a_stilt);
	STILS_DOWN_GET(file, ostack, a_stilt, position);
	
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(position) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	error = stilo_file_position_set(file, a_stilt,
	    stilo_integer_get(position));
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_npop(ostack, 2);
}

void
systemdict_setglobal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	stilt_setglobal(a_stilt, stilo_boolean_get(stilo));
	stils_pop(ostack);
}

void
systemdict_shift(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*integer, *shift;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(shift, ostack, a_stilt);
	STILS_DOWN_GET(integer, ostack, a_stilt, shift);

	if (stilo_type_get(integer) != STILOT_INTEGER || stilo_type_get(shift)
	    != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	if (stilo_integer_get(shift) > 0) {
		stilo_integer_set(integer, stilo_integer_get(integer) <<
		    stilo_integer_get(shift));
	} else if (stilo_integer_get(shift) < 0) {
		stilo_integer_set(integer, stilo_integer_get(integer) >>
		    stilo_integer_get(shift));
	}

	stils_pop(ostack);
}

void
systemdict_srand(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*seed;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(seed, ostack, a_stilt);
	srandom(stilo_integer_get(seed));
	stils_pop(ostack);
}

void
systemdict_stack(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_stils_t	*ostack;
	cw_uint32_t	i, count;
	cw_uint8_t	code[] = "=";

	stilts_new(&stilts, a_stilt);
	ostack = stilt_ostack_get(a_stilt);
	count = stils_count(ostack);

	for (i = 0; i < count; i++) {
		systemdict_dup(a_stilt);
		stilt_interpret(a_stilt, &stilts, code, sizeof(code) - 1);
		stilt_flush(a_stilt, &stilts);
		stils_roll(ostack, count, 1);
	}
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_start(cw_stilt_t *a_stilt)
{
	cw_stils_t	*estack;
	cw_stilo_t	*file;

	estack = stilt_estack_get(a_stilt);

	file = stils_push(estack);
	stilo_dup(file, stilt_stdin_get(a_stilt));
	stilo_attrs_set(file, STILOA_EXECUTABLE);

	xep_begin();
	xep_try {
		stilt_loop(a_stilt);
	}
	xep_catch(_CW_STILX_EXIT)
	xep_mcatch(_CW_STILX_STOP) {
		xep_handled();
	}
	xep_catch(_CW_STILX_QUIT) {
		cw_stilo_t	*stilo;
		cw_uint32_t	i, depth;

		/*
		 * Pop objects off the exec stack, up to and including file.
		 */
		for (i = 0, depth = stils_count(estack), stilo = NULL; i <
		     depth; i++) {
			stilo = stils_down_get(estack, stilo);
			if (stilo == file)
				break;
		}
		_cw_assert(i < depth);
		stils_npop(estack, i + 1);

		xep_handled();
	}
	xep_end();
}

void
systemdict_status(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_stdin(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_stdin;

	ostack = stilt_ostack_get(a_stilt);
	stilo_stdin = stils_push(ostack);

	stilo_dup(stilo_stdin, stilt_stdin_get(a_stilt));
}

void
systemdict_stderr(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_stderr;

	ostack = stilt_ostack_get(a_stilt);
	stilo_stderr = stils_push(ostack);

	stilo_dup(stilo_stderr, stilt_stderr_get(a_stilt));
}

void
systemdict_stdout(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_stdout;

	ostack = stilt_ostack_get(a_stilt);
	stilo_stdout = stils_push(ostack);

	stilo_dup(stilo_stdout, stilt_stdout_get(a_stilt));
}

void
systemdict_stop(cw_stilt_t *a_stilt)
{
	xep_throw(_CW_STILX_STOP);
}

void
systemdict_stopped(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*exec, *stilo;
	cw_bool_t	result = FALSE;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	
	STILS_GET(exec, ostack, a_stilt);
	stilo = stils_push(estack);
	stilo_dup(stilo, exec);
	stils_pop(ostack);

	/*
	 * Point exec to the copy on the execution stack, so that it can be used
	 * as a marker for execution stack cleanup if the stop operator is
	 * called.
	 */
	exec = stilo;

	/* Catch a stop exception, if thrown. */
	xep_begin();
	xep_try {
		stilt_loop(a_stilt);
	}
	xep_catch(_CW_STILX_STOP) {
		xep_handled();
		result = TRUE;

		/* Clean up whatever mess was left on the execution stack. */
		do {
			stilo = stils_get(estack);
			stils_pop(estack);
		} while (stilo != exec);
	}
	xep_end();

	stilo = stils_push(ostack);
	stilo_boolean_new(stilo, result);
}

void
systemdict_store(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*dict, *key, *val;
	cw_uint32_t	i, depth;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);

	STILS_GET(val, ostack, a_stilt);
	STILS_DOWN_GET(key, ostack, a_stilt, val);

	/*
	 * Iteratively search the dictionaries on the dictionary stack for key,
	 * and replace its value with val.
	 */
	for (i = 0, depth = stils_count(dstack), dict = NULL; i < depth; i++) {
		dict = stils_down_get(dstack, dict);
		if (stilo_dict_lookup(dict, a_stilt, key, NULL) == FALSE) {
			/* Found. */
			stilo_dict_def(dict, a_stilt, key, val);
			return;
		}
	}
	/* Not found.  Create a new entry in currentdict. */
	dict = stils_get(dstack);
	stilo_dict_def(dict, a_stilt, key, val);

	stils_npop(ostack, 2);
}

void
systemdict_string(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	len = stilo_integer_get(stilo);
	if (len < 0) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	stilo_string_new(stilo, a_stilt, len);
}

void
systemdict_sub(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	STILS_GET(b, ostack, a_stilt);
	STILS_DOWN_GET(a, ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) - stilo_integer_get(b));
	stils_pop(ostack);
}

/* = */
void
systemdict_sym_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *stdout_stilo;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	STILS_GET(stilo, ostack, a_stilt);
	error = stilo_print(stilo, a_stilt, stdout_stilo, FALSE, TRUE);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}
	error = stilo_file_buffer_flush(stdout_stilo, a_stilt);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_pop(ostack);
}

/* == */
void
systemdict_sym_eq_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *stdout_stilo;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	STILS_GET(stilo, ostack, a_stilt);
	error = stilo_print(stilo, a_stilt, stdout_stilo, TRUE, TRUE);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}
	error = stilo_file_buffer_flush(stdout_stilo, a_stilt);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_pop(ostack);
}

/* >> */
void
systemdict_sym_gt_gt(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*stilo, *dict, *key, *val;
	cw_uint32_t	npairs, i, depth;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	/* Find the mark. */
	for (i = 0, depth = stils_count(ostack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilt_error(a_stilt, STILTE_UNMATCHEDMARK);
		return;
	}
	if ((i & 1) == 1) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set npairs
	 * accordingly.  When we pop the stilo's off the stack, we'll have to
	 * pop (npairs << 1 + 1) stilo's.
	 */
	npairs = i >> 1;

	dict = stils_push(tstack);
	stilo_dict_new(dict, a_stilt, npairs);

	/*
	 * Traverse down the stack, moving stilo's to the dict.
	 */
	for (i = 0, key = NULL; i < npairs; i++) {
		val = stils_down_get(ostack, key);
		key = stils_down_get(ostack, val);
		stilo_dict_def(dict, a_stilt, key, val);
	}

	/* Pop the stilo's off the stack now. */
	stils_npop(ostack, (npairs << 1) + 1);

	/* Push the dict onto the stack. */
	stilo = stils_push(ostack);
	stilo_dup(stilo, dict);

	stils_pop(tstack);
}

/* ] */
void
systemdict_sym_rb(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*tstilo, *stilo, *arr;
	cw_sint32_t	nelements, i, depth;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	/* Find the mark. */
	for (i = 0, depth = stils_count(ostack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilt_error(a_stilt, STILTE_UNMATCHEDMARK);
		return;
	}

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	tstilo = stils_push(tstack);
	stilo_array_new(tstilo, a_stilt, nelements);
	arr = stilo_array_get(tstilo);

	/*
	 * Traverse down the stack, moving stilo's to the array.
	 */
	for (i = nelements - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(ostack, stilo);
		stilo_dup(&arr[i], stilo);
	}

	/* Pop the stilo's off the stack now. */
	stils_npop(ostack, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(ostack);
	stilo_dup(stilo, tstilo);

	stils_pop(tstack);
}

void
systemdict_timedwait(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_token(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_trylock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_type(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stilot_t	type;
	/* Must be kept in sync with cw_stilot_t. */
	static const cw_stiln_t typenames[] = {
		0,
		STILN_arraytype,
		STILN_booleantype,
		STILN_conditiontype,
		STILN_dicttype,
		STILN_filetype,
		STILN_hooktype,
		STILN_integertype,
		STILN_locktype,
		STILN_marktype,
		STILN_nametype,
		STILN_nulltype,
		STILN_operatortype,
		STILN_stringtype
	};

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	type = stilo_type_get(stilo);
	_cw_assert(type != STILOT_NO && type <= STILOT_STRING);

	stilo_name_new(stilo, a_stilt, stiln_str(typenames[type]),
		    stiln_len(typenames[type]), TRUE);
	stilo_attrs_set(stilo, STILOA_EXECUTABLE);
}

void
systemdict_undef(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*dict, *key;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(key, ostack, a_stilt);
	STILS_DOWN_GET(dict, ostack, a_stilt, key);
	if (stilo_type_get(dict) != STILOT_DICT) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_dict_undef(dict, a_stilt, key);

	stils_npop(ostack, 2);
}

void
systemdict_unlock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_version(cw_stilt_t *a_stilt)
{
	_cw_stil_code(a_stilt, "(" _LIBSTIL_VERSION ")");
}

void
systemdict_wait(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_wcheck(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_STRING: {
		cw_stilop_t	perms;
		cw_bool_t	result;

		perms = stilo_perms_get(stilo);
		if (perms < STILOP_READONLY)
			result = TRUE;
		else
			result = FALSE;
		stilo_boolean_new(stilo, result);

		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
}

void
systemdict_where(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*dict, *key, *stilo;
	cw_uint32_t	i, depth;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);

	STILS_GET(key, ostack, a_stilt);

	/*
	 * Iteratively search the dictionaries on the dictionary stack for key.
	 */
	for (i = 0, depth = stils_count(dstack), dict = NULL; i < depth; i++) {
		dict = stils_down_get(dstack, dict);
		if (stilo_dict_lookup(dict, a_stilt, key, NULL) == FALSE) {
			/* Found. */
			stilo = stils_push(ostack);
			stilo_dup(key, dict);
			stilo_boolean_new(stilo, TRUE);
			return;
		}
	}
	/* Not found.  Create a new entry in currentdict. */
	stilo_boolean_new(key, FALSE);
}

void
systemdict_write(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *value;
	cw_stilte_t	error;
	cw_uint8_t	val;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(value, ostack, a_stilt);
	STILS_DOWN_GET(file, ostack, a_stilt, value);
	
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(value) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	val = (cw_uint8_t)stilo_integer_get(value);
	error = stilo_file_write(file, a_stilt, &val, 1);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_npop(ostack, 2);
}

void
systemdict_writestring(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *string;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(string, ostack, a_stilt);
	STILS_DOWN_GET(file, ostack, a_stilt, string);

	if (stilo_type_get(string) != STILOT_STRING || stilo_type_get(file) !=
	    STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	error = stilo_file_write(file, a_stilt, stilo_string_get(string),
	    stilo_string_len_get(string));
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_npop(ostack, 2);
}

void
systemdict_xcheck(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stiloa_t	attr;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	
	attr = stilo_attrs_get(stilo);

	if (attr == STILOA_EXECUTABLE)
		stilo_boolean_new(stilo, TRUE);
	else
		stilo_boolean_new(stilo, FALSE);
}

void
systemdict_xor(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo_b, ostack, a_stilt);
	STILS_DOWN_GET(stilo_a, ostack, a_stilt, stilo_b);

	if (stilo_type_get(stilo_a) == STILOT_BOOLEAN && stilo_type_get(stilo_b)
	    == STILOT_BOOLEAN) {
		cw_bool_t	xor;

		if (stilo_boolean_get(stilo_a) || stilo_boolean_get(stilo_b)) {
			if (stilo_boolean_get(stilo_a) ==
			    stilo_boolean_get(stilo_b))
				xor = FALSE;
			else
				xor = TRUE;
		} else
			xor = FALSE;
		stilo_boolean_new(stilo_a, xor);
	} else if (stilo_type_get(stilo_a) == STILOT_INTEGER &&
	    stilo_type_get(stilo_b) == STILOT_INTEGER) {
		stilo_integer_set(stilo_a, stilo_integer_get(stilo_a) ^
		    stilo_integer_get(stilo_b));
	} else {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stils_pop(ostack);
}

void
systemdict_yield(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}
