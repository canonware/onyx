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
#include "../include/libstil/stilo_l.h"

#include <sys/time.h>	/* For realtime operator. */
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

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
	ENTRY(and),
	ENTRY(array),
	ENTRY(astore),
	ENTRY(begin),
	ENTRY(bind),
	ENTRY(broadcast),
	ENTRY(bytesavailable),
	ENTRY(chmod),
	ENTRY(chown),
	ENTRY(clear),
	ENTRY(cleardstack),
	ENTRY(cleartomark),
	ENTRY(closefile),
	ENTRY(condition),
	ENTRY(copy),
	ENTRY(count),
	ENTRY(countdstack),
	ENTRY(countestack),
	ENTRY(counttomark),
	ENTRY(currentdict),
	ENTRY(currentfile),
	ENTRY(currentlocking),
	ENTRY(cvlit),
	ENTRY(cvn),
	ENTRY(cvrs),
	ENTRY(cvs),
	ENTRY(cvx),
	ENTRY(def),
	ENTRY(detach),
	ENTRY(dict),
	ENTRY(dirforeach),
	ENTRY(div),
	ENTRY(dstack),
	ENTRY(dup),
	ENTRY(end),
	ENTRY(eq),
	ENTRY(estack),
	ENTRY(eval),
	ENTRY(exch),
	ENTRY(exec),
	ENTRY(exit),
	ENTRY(exp),
	ENTRY(flush),
	ENTRY(flushfile),
	ENTRY(for),
	ENTRY(foreach),
	ENTRY(fork),
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
	ENTRY(link),
	ENTRY(load),
	ENTRY(lock),
	ENTRY(loop),
	ENTRY(lt),
	ENTRY(mark),
	ENTRY(mkdir),
	ENTRY(mod),
	ENTRY(mul),
	ENTRY(mutex),
	ENTRY(ne),
	ENTRY(neg),
	ENTRY(not),
	ENTRY(nsleep),
	ENTRY(open),
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
	ENTRY(read),
	ENTRY(readline),
	ENTRY(realtime),
	ENTRY(renamefile),
	ENTRY(repeat),
	ENTRY(roll),
	ENTRY(run),
	ENTRY(seek),
	ENTRY(self),
	ENTRY(setlocking),
	ENTRY(shift),
	ENTRY(signal),
	ENTRY(spop),
	ENTRY(srand),
	ENTRY(start),
	ENTRY(stat),
	ENTRY(stop),
	ENTRY(stopped),
	ENTRY(store),
	ENTRY(string),
	ENTRY(sub),
	ENTRY(sym_gt_gt),
	{STILN_sym_lb, systemdict_mark},
	{STILN_sym_lt_lt, systemdict_mark},
	ENTRY(sym_rb),
	ENTRY(symlink),
	ENTRY(system),
	ENTRY(tell),
	ENTRY(test),
	ENTRY(thread),
	ENTRY(timedwait),
	ENTRY(token),
	ENTRY(truncate),
	ENTRY(trylock),
	ENTRY(type),
	ENTRY(undef),
	ENTRY(unlink),
	ENTRY(unlock),
	ENTRY(version),
	ENTRY(wait),
	ENTRY(waitpid),
	ENTRY(where),
	ENTRY(write),
	ENTRY(xcheck),
	ENTRY(xor),
	ENTRY(yield)
};

void
systemdict_l_populate(cw_stilo_t *a_dict, cw_stil_t *a_stil, int a_argc, char
    **a_argv)
{
	cw_uint32_t	i;
	cw_stilo_t	name, value;

#define	NEXTRA	11
#define NENTRIES							\
	(sizeof(systemdict_ops) / sizeof(struct cw_systemdict_entry))

	stilo_dict_new(a_dict, a_stil, TRUE, NENTRIES + NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stil,
		    stiln_str(systemdict_ops[i].stiln),
		    stiln_len(systemdict_ops[i].stiln), TRUE);
		stilo_operator_new(&value, systemdict_ops[i].op_f,
		    systemdict_ops[i].stiln);
		stilo_attrs_set(&value, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stil, &name, &value);
	}

	/*
	 * Initialize entries that are not operators.
	 */

	/* globaldict. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_globaldict),
	    stiln_len(STILN_globaldict), TRUE);
	stilo_dup(&value, stil_globaldict_get(a_stil));
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* systemdict. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_systemdict),
	    stiln_len(STILN_systemdict), TRUE);
	stilo_dup(&value, stil_systemdict_get(a_stil));
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* gcdict. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_gcdict),
	    stiln_len(STILN_gcdict), TRUE);
	stilo_dup(&value, stila_gcdict_get(stil_stila_get(a_stil)));
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* envdict. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_envdict),
	    stiln_len(STILN_envdict), TRUE);
	stilo_dup(&value, stil_envdict_get(a_stil));
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* argv. */
	{
		int		i;
		cw_sint32_t	len;
		cw_stilo_t	argv_stilo, str_stilo;
		cw_uint8_t	*t_str;

		/* Create the argv array and populate it. */
		stilo_array_new(&argv_stilo, a_stil, TRUE, a_argc);
		for (i = 0; i < a_argc; i++) {
			len = strlen(a_argv[i]);
			stilo_string_new(&str_stilo, a_stil, TRUE, len);
			t_str = stilo_string_get(&str_stilo);
			stilo_string_lock(&str_stilo);
			memcpy(t_str, a_argv[i], len);
			stilo_string_unlock(&str_stilo);

			stilo_array_el_set(&argv_stilo, &str_stilo, i);
		}

		/* Insert argv into systemdict. */
		stilo_name_new(&name, a_stil, stiln_str(STILN_argv),
		    stiln_len(STILN_argv), TRUE);
		stilo_dict_def(a_dict, a_stil, &name, &argv_stilo);
	}

	/* stdin. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_stdin),
	    stiln_len(STILN_stdin), TRUE);
	stilo_dup(&value, stil_stdin_get(a_stil));
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* stdout. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_stdout),
	    stiln_len(STILN_stdout), TRUE);
	stilo_dup(&value, stil_stdout_get(a_stil));
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* stderr. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_stderr),
	    stiln_len(STILN_stderr), TRUE);
	stilo_dup(&value, stil_stderr_get(a_stil));
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* true. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_true),
	    stiln_len(STILN_true), TRUE);
	stilo_boolean_new(&value, TRUE);
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* false. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_false),
	    stiln_len(STILN_false), TRUE);
	stilo_boolean_new(&value, FALSE);
	stilo_dict_def(a_dict, a_stil, &name, &value);

	/* null. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_null),
	    stiln_len(STILN_null), TRUE);
	stilo_null_new(&value);
	stilo_dict_def(a_dict, a_stil, &name, &value);

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
		stilo_array_el_set(array, stilo, i);
	}
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

	stilo_array_new(stilo, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), len);
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
	el = stils_push(tstack);

	stilo_l_array_bound_set(a_proc, TRUE);

	for (i = 0, count = stilo_array_len_get(a_proc); i < count; i++) {
		stilo_array_el_get(a_proc, i, el);
		if (stilo_attrs_get(el) != STILOA_EXECUTABLE)
			continue;

		switch (stilo_type_get(el)) {
		case STILOT_ARRAY:
			if (stilo_l_array_bound_get(el) == FALSE)
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
							stilo_l_operator_fast_op_set(el,
							    fastops[j].stiln);
							stilo_array_el_set(a_proc,
							    el, i);
							break;
						}
					}
					/*
					 * If val isn't a fastop, still convert
					 * the name to an operator.
					 */
					if (j == NFASTOPS) {
						/* Replace el with val. */
						stilo_array_el_set(a_proc, val,
						    i);
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

	stils_npop(tstack, 2);
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
systemdict_broadcast(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*condition;
	
	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(condition, ostack, a_stilt);
	if (stilo_type_get(condition) != STILOT_CONDITION) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_condition_broadcast(condition);

	stils_pop(ostack);
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
systemdict_chmod(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_chown(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
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
systemdict_cleardstack(cw_stilt_t *a_stilt)
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

	error = stilo_file_close(stilo);
	if (error) {
		stilt_error(a_stilt, STILTE_IOERROR);
		return;
	}

	stils_pop(ostack);
}

void
systemdict_condition(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*condition;

	ostack = stilt_ostack_get(a_stilt);
	condition = stils_push(ostack);
	stilo_condition_new(condition, stilt_stil_get(a_stilt));
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
		cw_stilo_t	*orig;

		STILS_DOWN_GET(orig, ostack, a_stilt, stilo);
		if (stilo_type_get(orig) != STILOT_ARRAY) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
		if (stilo_array_len_get(stilo) < stilo_array_len_get(orig)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}

		stilo_array_copy(stilo, orig);

		stils_roll(ostack, 2, 1);
		stils_pop(ostack);
		break;
	}
	case STILOT_DICT: {
		cw_stilo_t	*orig;

		STILS_DOWN_GET(orig, ostack, a_stilt, stilo);
		if (stilo_type_get(orig) != STILOT_DICT) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}

		stilo_dict_copy(stilo, orig, stilt_stil_get(a_stilt),
		    stilt_currentlocking(a_stilt));
		break;
	}
	case STILOT_STRING: {
		cw_stilo_t	*orig;

		STILS_DOWN_GET(orig, ostack, a_stilt, stilo);
		if (stilo_type_get(orig) != STILOT_STRING) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
		if (stilo_string_len_get(stilo) < stilo_string_len_get(orig)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}

		stilo_string_copy(stilo, orig);

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
systemdict_count(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_integer_new(stilo, stils_count(ostack) - 1);
}

void
systemdict_countdstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo;

	dstack = stilt_dstack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_integer_new(stilo, stils_count(dstack));
}

void
systemdict_countestack(cw_stilt_t *a_stilt)
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
	if (i == depth) {
		stilo_file_new(file, stilt_stil_get(a_stilt),
		    stilt_currentlocking(a_stilt));
	}
}

void
systemdict_currentlocking(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack);
	stilo_boolean_new(stilo, stilt_currentlocking(a_stilt));
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

	stilo_string_lock(tstilo);
	stilo_name_new(stilo, stilt_stil_get(a_stilt), stilo_string_get(tstilo),
	    stilo_string_len_get(tstilo), FALSE);
	stilo_string_unlock(tstilo);
	stilo_attrs_set(stilo, stilo_attrs_get(tstilo));

	stils_pop(tstack);
}

void
systemdict_cvrs(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*num, *radix;
	cw_uint64_t	val;
	cw_uint32_t	i, rlen, base;
	cw_uint8_t	*str;
	static const cw_uint8_t *syms = "0123456789abcdefghijklmnopqrstuvwxyz";
	cw_uint8_t	*result, s_result[65] =
	    "0000000000000000000000000000000000000000000000000000000000000000";

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(radix, ostack, a_stilt);
	STILS_DOWN_GET(num, ostack, a_stilt, radix);
	if ((stilo_type_get(num) != STILOT_INTEGER) || (stilo_type_get(radix) !=
	    STILOT_INTEGER)) {
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
	/* Print a 0 if the number is 0. */
	if (rlen == 0) {
		rlen++;
		result--;
	}

	stilo_string_new(num, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), rlen);

	str = stilo_string_get(num);
	stilo_string_lock(num);
	memcpy(str, result, rlen);
	stilo_string_unlock(num);

	stils_pop(ostack);
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
		_cw_stil_code(a_stilt, "{(true)} {(false)} ifelse");
		break;
	case STILOT_INTEGER: {
		cw_uint8_t	result[21];
		cw_sint32_t	len;

		len = _cw_out_put_s(result, "[q|s:s]",
		    stilo_integer_get(stilo));
		stilo_string_new(stilo, stilt_stil_get(a_stilt),
		    stilt_currentlocking(a_stilt), len);
		stilo_string_lock(stilo);
		stilo_string_set(stilo, 0, result, len);
		stilo_string_unlock(stilo);
		break;
	}
	case STILOT_NAME: {
		cw_stils_t	*tstack;
		cw_stilo_t	*tstilo;

		tstack = stilt_tstack_get(a_stilt);
		tstilo = stils_push(tstack);
		stilo_dup(tstilo, stilo);

		stilo_string_new(stilo, stilt_stil_get(a_stilt),
		    stilt_currentlocking(a_stilt), stilo_name_len_get(tstilo));
		stilo_string_lock(stilo);
		stilo_string_set(stilo, 0, stilo_name_str_get(tstilo),
		    stilo_name_len_get(tstilo));
		stilo_string_unlock(stilo);

		stils_pop(tstack);
		break;
	}
	case STILOT_OPERATOR: {
		cw_stiln_t	stiln;

		stiln = stilo_l_operator_fast_op_stiln(stilo);
		if (stiln > STILN_LAST)
			_cw_stil_code(a_stilt, "pop (--operator--)");
		else {
			cw_stils_t	*tstack;
			cw_stilo_t	*tstilo;

			tstack = stilt_tstack_get(a_stilt);
			tstilo = stils_push(tstack);
			stilo_dup(tstilo, stilo);

			stilo_string_new(stilo, stilt_stil_get(a_stilt),
			    stilt_currentlocking(a_stilt), stiln_len(stiln));
			stilo_string_lock(stilo);
			stilo_string_set(stilo, 0, stiln_str(stiln),
			    stiln_len(stiln));
			stilo_string_unlock(stilo);

			stils_pop(tstack);
		}
		break;
	}
	case STILOT_STRING:
		break;
	case STILOT_NO:
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_MUTEX:
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

	stilo_dict_def(dict, stilt_stil_get(a_stilt), key, val);

	stils_npop(ostack, 2);
}

void
systemdict_detach(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*thread;
	union {
		cw_sint64_t	i;
		cw_stilt_t	*stilt;
	} u;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(thread, ostack, a_stilt);
	if (stilo_type_get(thread) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	u.i = stilo_integer_get(thread);
	stilt_detach(u.stilt);

	stils_pop(ostack);
}

void
systemdict_dict(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*dict;

	ostack = stilt_ostack_get(a_stilt);

	dict = stils_push(ostack);
	stilo_dict_new(dict, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), _CW_SYSTEMDICT_DICT_SIZE);
}

void
systemdict_dirforeach(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
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
systemdict_dstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*array, *stilo;
	cw_sint32_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);

	/* Create array. */
	count = stils_count(dstack);
	array = stils_push(ostack);
	stilo_array_new(array, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), count);

	/* Copy dstack to array. */
	for (i = count - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(dstack, stilo);
		stilo_array_el_set(array, stilo, i);
	}
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
		stilt_error(a_stilt, STILTE_DSTACKUNDERFLOW);
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

	result = stilo_compare(stilo_a, stilo_b);
	if (result == 0)
		eq = TRUE;
	else
		eq = FALSE;

	stilo_boolean_new(stilo_a, eq);

	stils_pop(ostack);
}

void
systemdict_estack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*array, *stilo;
	cw_sint32_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);

	/* Create array. */
	count = stils_count(estack);
	array = stils_push(ostack);
	stilo_array_new(array, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), count);

	/* Copy estack to array. */
	for (i = count - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(estack, stilo);
		stilo_array_el_set(array, stilo, i);
	}
}

void
systemdict_eval(cw_stilt_t *a_stilt)
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
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*array, *el;
	cw_uint32_t	i, slen, argc;
	char		*path, **argv, **envp;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	el = stils_push(tstack);
	
	STILS_GET(array, ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		goto VALIDATION_ERROR;
	}
	argc = stilo_array_len_get(array);
	for (i = 0; i < argc; i++) {
		stilo_array_el_get(array, i, el);
		if (stilo_type_get(el) != STILOT_STRING) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			goto VALIDATION_ERROR;
		}
	}

	/*
	 * Construct path.
	 */
	stilo_array_el_get(array, 0, el);
	if (stilo_type_get(el) != STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		goto PATH_ERROR;
	}
	slen = stilo_string_len_get(el);
	path = (char *)_cw_malloc(slen + 1);
	stilo_string_lock(el);
	memcpy(path, stilo_string_get(el), slen);
	stilo_string_unlock(el);
	path[slen] = '\0';

	/*
	 * Construct argv.
	 */
	argv = (char **)_cw_malloc(sizeof(char *) * (argc + 1));
	for (i = 0; i < argc; i++) {
		stilo_array_el_get(array, i, el);
		if (stilo_type_get(el) != STILOT_STRING) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			goto ARGV_ERROR;
		}
		slen = stilo_string_len_get(el);
		argv[i] = (char *)_cw_malloc(slen + 1);
		stilo_string_lock(el);
		memcpy(argv[i], stilo_string_get(el), slen);
		stilo_string_unlock(el);
		argv[i][slen] = '\0';
	}
	argv[i] = NULL;

	/*
	 * Construct envp.
	 */
	{
		cw_uint32_t	dcount, key_len, val_len;
		cw_stilo_t	*key, *val;
		char		*entry;

		key = stils_push(tstack);
		val = stils_push(tstack);

		dcount = stilo_dict_count(stilt_envdict_get(a_stilt));
		envp = (char **)_cw_malloc(sizeof(char *) * (dcount + 1));
		for (i = 0; i < dcount; i++) {
			/* Get key and val. */
			stilo_dict_iterate(stilt_envdict_get(a_stilt), key);
			stilo_dict_lookup(stilt_envdict_get(a_stilt), key, val);
			if (stilo_type_get(key) != STILOT_NAME ||
			    stilo_type_get(val) != STILOT_STRING) {
				stilt_error(a_stilt, STILTE_TYPECHECK);
				stils_npop(tstack, 2);
				goto ENVP_ERROR;
			}

			/* Create string that looks like "<key>=<val>\0". */
			key_len = stilo_name_len_get(key);
			val_len = stilo_string_len_get(val);
			entry = (char *)_cw_malloc(key_len + val_len + 2);

			memcpy(entry, stilo_name_str_get(key), key_len);
			entry[key_len] = '=';
			stilo_string_lock(val);
			memcpy(&entry[key_len + 1], stilo_string_get(val),
			    val_len);
			stilo_string_unlock(val);
			entry[key_len + 1 + val_len] = '\0';

			envp[i] = entry;
		}
		envp[i] = NULL;

		stils_npop(tstack, 2);
	}

	execve(path, argv, envp);
	/*
	 * If we get here, then the execve() call failed.  Get an error
	 * back to the parent.
	 */
	exit(1);

	ENVP_ERROR:
	for (i = 0; envp[i] != NULL; i++)
		_cw_free(envp[i]);
	_cw_free(envp);
	ARGV_ERROR:
	for (i = 0; argv[i] != NULL; i++)
		_cw_free(argv[i]);
	_cw_free(argv);
	_cw_free(path);
	PATH_ERROR:
	VALIDATION_ERROR:
	stils_pop(tstack);
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

	if (stilo_integer_get(b) < 0) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	} else if (stilo_integer_get(b) > 0) {
		for (i = 1, r = stilo_integer_get(a); i < stilo_integer_get(b);
		     i++)
			r *= stilo_integer_get(a);
	} else
		r = 1;

	stilo_integer_set(a, r);
	stils_pop(ostack);
}

void
systemdict_flush(cw_stilt_t *a_stilt)
{
	cw_stilte_t	error;

	error = stilo_file_buffer_flush(stilt_stdout_get(a_stilt));
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

	error = stilo_file_buffer_flush(file);
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
systemdict_foreach(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*stilo, *what, *proc;
	cw_uint32_t	tstack_depth;
	cw_sint64_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	tstack_depth = stils_count(tstack);

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
			el = stils_push(tstack);
			for (i = 0, count = stilo_array_len_get(what); i <
				 count; i++) {
				stilo_array_el_get(what, i, el);
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
				stilo_dict_iterate(what, key);

				/* Use key to get val. */
				stilo_dict_lookup(what, key, val);

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
				stilo_string_el_get(what, i, &el);
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
		stils_npop(tstack, stils_count(tstack) - tstack_depth);
	}
	xep_end();

	stils_npop(tstack, stils_count(tstack) - tstack_depth);
}

void
systemdict_fork(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	pid_t		pid;

	pid = fork();
	if (pid == -1) {
		/* Error, related to some form of resource exhaustion. */
		stilt_error(a_stilt, STILTE_LIMITCHECK);
		return;
	}

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack);
	stilo_integer_new(stilo, pid);
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

	result = stilo_compare(stilo_a, stilo_b);
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

		if (stilo_type_get(with) != STILOT_INTEGER) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);

		if (index >= stilo_array_len_get(from)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		stilo_array_el_get(from, index, with);

		stils_roll(ostack, 2, 1);
		stils_pop(ostack);
		break;
	}
	case STILOT_DICT: {
		cw_stilo_t	*val;

		val = stils_push(ostack);
		if (stilo_dict_lookup(from, with, val)) {
			stils_pop(ostack);
			stilt_error(a_stilt, STILTE_UNDEFINED);
			return;
		}
		stils_roll(ostack, 3, 1);
		stils_npop(ostack, 2);
		break;
	}
	case STILOT_STRING: {
		cw_sint64_t	index;
		cw_uint8_t	el;

		if (stilo_type_get(with) != STILOT_INTEGER) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);

		if (index < 0 || index >= stilo_string_len_get(from)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		stilo_string_el_get(from, index, &el);
		stilo_integer_set(with, (cw_sint64_t)el);

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
		if (index + len > stilo_array_len_get(from)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		stilo_array_subarray_new(count, from, stilt_stil_get(a_stilt),
		    index, len);
		break;
	case STILOT_STRING:
		if (index + len > stilo_string_len_get(from)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		stilo_string_substring_new(count, from, stilt_stil_get(a_stilt),
		    index, len);
		break;
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
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

	result = stilo_compare(stilo_a, stilo_b);
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
	stilo_name_new(key, stilt_stil_get(a_stilt), stiln_str(STILN_errordict),
	    stiln_len(STILN_errordict), TRUE);
	if (stilt_dict_stack_search(a_stilt, key, errordict)) {
		stils_npop(tstack, 2);
		xep_throw(_CW_STILX_ERRORDICT);
	}

	/* Get handleerror from errordict and push it onto estack. */
	handleerror = stils_push(estack);
	stilo_name_new(key, stilt_stil_get(a_stilt),
	    stiln_str(STILN_handleerror), stiln_len(STILN_handleerror), TRUE);
	if (stilo_dict_lookup(errordict, key, handleerror)) {
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
	cw_stils_t	*ostack;
	cw_stilo_t	*thread;
	union {
		cw_sint64_t	i;
		cw_stilt_t	*stilt;
	} u;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(thread, ostack, a_stilt);
	if (stilo_type_get(thread) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	u.i = stilo_integer_get(thread);
	stilt_join(u.stilt);

	stils_pop(ostack);
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

	known = !stilo_dict_lookup(dict, key, NULL);
	stilo_boolean_new(dict, known);

	stils_pop(ostack);
}

void
systemdict_lcheck(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_bool_t	locking;
	
	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_BOOLEAN:
	case STILOT_CONDITION:
	case STILOT_HOOK:
	case STILOT_INTEGER:
	case STILOT_MARK:
	case STILOT_MUTEX:
	case STILOT_NAME:
	case STILOT_NULL:
	case STILOT_OPERATOR:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	case STILOT_ARRAY:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_STRING:
		locking = stilo_lcheck(stilo);
		break;
	case STILOT_NO:
	default:
		_cw_not_reached();
	}
	stilo_boolean_new(stilo, locking);
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

	result = stilo_compare(stilo_a, stilo_b);
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
systemdict_link(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
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
	cw_stils_t	*ostack;
	cw_stilo_t	*mutex;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(mutex, ostack, a_stilt);
	if (stilo_type_get(mutex) != STILOT_MUTEX) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_mutex_lock(mutex);

	stils_pop(ostack);
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
		for (i = stils_count(estack); i > sdepth + 1; i--)
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

	result = stilo_compare(stilo_a, stilo_b);
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
systemdict_mkdir(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
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
systemdict_mutex(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*mutex;

	ostack = stilt_ostack_get(a_stilt);
	mutex = stils_push(ostack);
	stilo_mutex_new(mutex, stilt_stil_get(a_stilt));
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

	result = stilo_compare(stilo_a, stilo_b);
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
systemdict_nsleep(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	struct timespec sleeptime;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(stilo, ostack, a_stilt);

	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(stilo) <= 0) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	sleeptime.tv_sec = stilo_integer_get(stilo) / 1000000000;
	sleeptime.tv_nsec = stilo_integer_get(stilo) % 1000000000;

	nanosleep(&sleeptime, NULL);
	stils_pop(ostack);
}

void
systemdict_open(cw_stilt_t *a_stilt)
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
	stilo_file_new(file, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt));
	stilo_string_lock(name);
	error = stilo_file_open(file, stilo_string_get(name),
	    stilo_string_len_get(name), stilo_string_get(flags),
	    stilo_string_len_get(flags));
	stilo_string_unlock(name);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stilo_file_buffer_size_set(file, _LIBSTIL_FILE_BUFFER_SIZE);

	stils_pop(ostack);
	stilo_dup(name, file);
	stils_pop(tstack);
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

	stilo_string_lock(stilo);
	error = stilo_file_write(stdout_stilo, stilo_string_get(stilo),
	    stilo_string_len_get(stilo));
	stilo_string_unlock(stilo);
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
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *stdout_stilo;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	for (stilo = stils_down_get(ostack, NULL); stilo != NULL; stilo =
	     stils_down_get(ostack, stilo)) {
		error = stilo_print(stilo, stdout_stilo, 1, TRUE);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
	}
	error = stilo_file_buffer_flush(stdout_stilo);
	if (error)
		stilt_error(a_stilt, error);
}

void
systemdict_put(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*into, *with, *what;

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

		if (index >= stilo_array_len_get(into)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		stilo_array_el_set(into, what, index);
		break;
	}
	case STILOT_DICT: {
		stilo_dict_def(into, stilt_stil_get(a_stilt), with, what);
		break;
	}
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

		if (index >= stilo_string_len_get(into)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}
		stilo_string_el_set(into, val, index);
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
		cw_stils_t	*tstack;
		cw_stilo_t	*el;
		cw_uint32_t	i, len;

		tstack = stilt_tstack_get(a_stilt);
		el = stils_push(tstack);
		len = stilo_array_len_get(what);
		if (index + len > stilo_array_len_get(into)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			stils_pop(tstack);
			return;
		}
		for (i = 0; i < len; i++) {
			stilo_array_el_get(what, i, el);
			stilo_array_el_set(into, el, i + index);
		}
		stils_pop(tstack);
		break;
	}
	case STILOT_STRING: {
		cw_uint8_t	*str;
		cw_uint32_t	len;

		str = stilo_string_get(what);
		len = stilo_string_len_get(what);
		if (index + len > stilo_string_len_get(into)) {
			stilt_error(a_stilt, STILTE_RANGECHECK);
			return;
		}

		stilo_string_lock(what);
		stilo_string_lock(into);
		stilo_string_set(into, index, str, len);
		stilo_string_unlock(into);
		stilo_string_unlock(what);
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
systemdict_read(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file;
	cw_uint8_t	val;
	cw_sint32_t	nread;

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(file, ostack, a_stilt);
	switch (stilo_type_get(file)) {
	case STILOT_FILE: {
		cw_stilo_t	*value, *code;

		/* Character read. */
		value = stils_push(ostack);
		code = stils_push(ostack);
		
		nread = stilo_file_read(file, 1, &val);
		if (nread == -1) {
			stils_npop(ostack, 2);
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
		break;
	}
	case STILOT_STRING: {
		cw_stilo_t	*string;

		/* String read. */
		string = file;
		STILS_DOWN_GET(file, ostack, a_stilt, string);
		if (stilo_type_get(file) != STILOT_FILE) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			return;
		}

		stilo_string_lock(string);
		nread = stilo_file_read(file, stilo_string_len_get(string),
		    stilo_string_get(string));
		stilo_string_unlock(string);
		if (nread == -1) {
			stilt_error(a_stilt, STILTE_IOERROR);
			return;
		}

		if (nread == 0) {
			/* EOF. */
			stilo_boolean_new(file, TRUE);
			stilo_string_new(string, stilt_stil_get(a_stilt),
			    stilt_currentlocking(a_stilt), 0);
			stils_roll(ostack, 2, 1);
		} else if (nread < stilo_string_len_get(string)) {
			cw_stilo_t	*value, *code;

			/*
			 * We didn't fill the string, so we can't just use it as
			 * the result.  Create a copy.
			 */
			value = stils_under_push(ostack, file);
			stilo_string_substring_new(value, string,
			    stilt_stil_get(a_stilt), 0, nread);
			code = stils_under_push(ostack, file);
			stilo_boolean_new(code, FALSE);

			stils_npop(ostack, 2);
		} else {
			/*
			 * The string is full, so doesn't need modified.
			 */
			stilo_boolean_new(file, FALSE);
			stils_roll(ostack, 2, 1);
		}
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
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
	error = stilo_file_readline(tfile, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), stilo, &eof);
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
	stilo_string_lock(string_from);
	memcpy(str_from, stilo_string_get(string_from), nbytes);
	stilo_string_unlock(string_from);
	str_from[nbytes] = '\0';

	if (stilo_string_len_get(string_to) >= sizeof(str_to)) {
		stilt_error(a_stilt, STILTE_LIMITCHECK);
		return;
	}
	nbytes = stilo_string_len_get(string_to);
	stilo_string_lock(string_to);
	memcpy(str_to, stilo_string_get(string_to), nbytes);
	stilo_string_unlock(string_to);
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
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*stilo, *tfile;
	cw_stilte_t	error;
	cw_uint32_t	sdepth;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	tfile = stils_push(tstack);
	stilo_file_new(tfile, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt));
	stilo_string_lock(stilo);
	error = stilo_file_open(tfile, stilo_string_get(stilo),
	    stilo_string_len_get(stilo), "r", 1);
	stilo_string_unlock(stilo);
	if (error) {
		stils_pop(tstack);
		stilt_error(a_stilt, error);
		return;
	}
	stils_pop(ostack);
	stilo_attrs_set(tfile, STILOA_EXECUTABLE);
	stilo = stils_push(estack);

	sdepth = stils_count(estack);
	stilo_dup(stilo, tfile);

	xep_begin();
	xep_try {
		stilt_loop(a_stilt);
	}
	xep_catch(_CW_STILX_EXIT) {
		stilt_error(a_stilt, STILTE_INVALIDEXIT);
		/*
		 * Pop the exit operator off estack to avoid an infinite loop.
		 */
		stils_pop(estack);
		xep_retry();
	}
	xep_catch(_CW_STILX_STOP)
	xep_mcatch(_CW_STILX_QUIT) {
		cw_uint32_t	i;

		/* Close the file, but don't handle the exception. */
		error = stilo_file_close(tfile);
		stils_pop(tstack);
		if (error)
			stilt_error(a_stilt, error);

		/* Clean up estack. */
		for (i = stils_count(estack); i > sdepth; i--)
			stils_pop(estack);
	}
	xep_end();

	stils_pop(tstack);
}

void
systemdict_self(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*context;
	union {
		cw_sint64_t	i;
		cw_stilt_t	*stilt;
	} u;
	_cw_assert(sizeof(cw_sint64_t) >= sizeof(void *));

	ostack = stilt_ostack_get(a_stilt);
	context = stils_push(ostack);

	u.i = 0;
	u.stilt = a_stilt;
	stilo_integer_new(context, u.i);
}

void
systemdict_seek(cw_stilt_t *a_stilt)
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

	error = stilo_file_position_set(file, stilo_integer_get(position));
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_npop(ostack, 2);
}

void
systemdict_setlocking(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_BOOLEAN) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	stilt_setlocking(a_stilt, stilo_boolean_get(stilo));
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
systemdict_signal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*condition;
	
	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(condition, ostack, a_stilt);
	if (stilo_type_get(condition) != STILOT_CONDITION) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_condition_signal(condition);

	stils_pop(ostack);
}

void
systemdict_spop(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *depth, *stdout_stilo;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	STILS_GET(depth, ostack, a_stilt);
	STILS_DOWN_GET(stilo, ostack, a_stilt, depth);
	if (stilo_type_get(depth) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	error = stilo_print(stilo, stdout_stilo, stilo_integer_get(depth),
	    TRUE);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}
	error = stilo_file_buffer_flush(stdout_stilo);
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_npop(ostack, 2);
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
systemdict_start(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*ostilo, *estilo;
	cw_uint32_t	depth;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);

	STILS_GET(ostilo, ostack, a_stilt);
	estilo = stils_push(estack);
	stilo_dup(estilo, ostilo);
	stils_pop(ostack);

	depth = stils_count(estack);

	xep_begin();
	xep_try {
		stilt_loop(a_stilt);
	}
	xep_catch(_CW_STILX_EXIT)
	xep_mcatch(_CW_STILX_STOP) {
		xep_handled();
	}
	xep_catch(_CW_STILX_QUIT) {
		/*
		 * Pop all objects off the exec stack that weren't there before
		 * entering this function.
		 */
		stils_npop(estack, stils_count(estack) - depth);

		xep_handled();
	}
	xep_end();
}

void
systemdict_stat(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	stilo_boolean_new(stilo, stilo_file_status(stilo));
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
		if (stilo_dict_lookup(dict, key, NULL) == FALSE) {
			/* Found. */
			stilo_dict_def(dict, stilt_stil_get(a_stilt), key, val);
			return;
		}
	}
	/* Not found.  Create a new entry in currentdict. */
	dict = stils_get(dstack);
	stilo_dict_def(dict, stilt_stil_get(a_stilt), key, val);

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

	stilo_string_new(stilo, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), len);
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
	stilo_dict_new(dict, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), npairs);

	/*
	 * Traverse down the stack, moving stilo's to the dict.
	 */
	for (i = 0, key = NULL; i < npairs; i++) {
		val = stils_down_get(ostack, key);
		key = stils_down_get(ostack, val);
		stilo_dict_def(dict, stilt_stil_get(a_stilt), key, val);
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
	cw_stilo_t	*tstilo, *stilo;
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
	stilo_array_new(tstilo, stilt_stil_get(a_stilt),
	    stilt_currentlocking(a_stilt), nelements);

	/*
	 * Traverse down the stack, moving stilo's to the array.
	 */
	for (i = nelements - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(ostack, stilo);
		stilo_array_el_set(tstilo, stilo, i);
	}

	/* Pop the stilo's off the stack now. */
	stils_npop(ostack, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(ostack);
	stilo_dup(stilo, tstilo);

	stils_pop(tstack);
}

void
systemdict_symlink(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_system(cw_stilt_t *a_stilt)
{
#if (0)
	_cw_stil_code(a_stilt, "
fork
dup 0 eq {
	errordict begin
	/handleerror {quit} def
	end
	pop exec
}{
	exch pop waitpid
} ifelse
");
#else
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*array, *el;
	cw_uint32_t	i, slen, argc;
	char		*path, **argv, **envp;
	pid_t		pid;
	int		status;
	cw_sint64_t	result;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	el = stils_push(tstack);
	
	STILS_GET(array, ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		goto VALIDATION_ERROR;
	}
	argc = stilo_array_len_get(array);
	for (i = 0; i < argc; i++) {
		stilo_array_el_get(array, i, el);
		if (stilo_type_get(el) != STILOT_STRING) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			goto VALIDATION_ERROR;
		}
	}

	/*
	 * Construct path.
	 */
	stilo_array_el_get(array, 0, el);
	if (stilo_type_get(el) != STILOT_STRING) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		goto PATH_ERROR;
	}
	slen = stilo_string_len_get(el);
	path = (char *)_cw_malloc(slen + 1);
	stilo_string_lock(el);
	memcpy(path, stilo_string_get(el), slen);
	stilo_string_unlock(el);
	path[slen] = '\0';

	/*
	 * Construct argv.
	 */
	argv = (char **)_cw_malloc(sizeof(char *) * (argc + 1));
	for (i = 0; i < argc; i++) {
		stilo_array_el_get(array, i, el);
		if (stilo_type_get(el) != STILOT_STRING) {
			stilt_error(a_stilt, STILTE_TYPECHECK);
			goto ARGV_ERROR;
		}
		slen = stilo_string_len_get(el);
		argv[i] = (char *)_cw_malloc(slen + 1);
		stilo_string_lock(el);
		memcpy(argv[i], stilo_string_get(el), slen);
		stilo_string_unlock(el);
		argv[i][slen] = '\0';
	}
	argv[i] = NULL;

	/*
	 * Construct envp.
	 */
	{
		cw_uint32_t	dcount, key_len, val_len;
		cw_stilo_t	*key, *val;
		char		*entry;

		key = stils_push(tstack);
		val = stils_push(tstack);

		dcount = stilo_dict_count(stilt_envdict_get(a_stilt));
		envp = (char **)_cw_malloc(sizeof(char *) * (dcount + 1));
		for (i = 0; i < dcount; i++) {
			/* Get key and val. */
			stilo_dict_iterate(stilt_envdict_get(a_stilt), key);
			stilo_dict_lookup(stilt_envdict_get(a_stilt), key, val);
			if (stilo_type_get(key) != STILOT_NAME ||
			    stilo_type_get(val) != STILOT_STRING) {
				stilt_error(a_stilt, STILTE_TYPECHECK);
				stils_npop(tstack, 2);
				goto ENVP_ERROR;
			}

			/* Create string that looks like "<key>=<val>\0". */
			key_len = stilo_name_len_get(key);
			val_len = stilo_string_len_get(val);
			entry = (char *)_cw_malloc(key_len + val_len + 2);

			memcpy(entry, stilo_name_str_get(key), key_len);
			entry[key_len] = '=';
			stilo_string_lock(val);
			memcpy(&entry[key_len + 1], stilo_string_get(val),
			    val_len);
			stilo_string_unlock(val);
			entry[key_len + 1 + val_len] = '\0';

			envp[i] = entry;
		}
		envp[i] = NULL;

		stils_npop(tstack, 2);
	}

	/*
	 * Call fork()/execve()/wait().
	 */
	pid = fork();
	switch (pid) {
	case -1:
		/* Error, related to some form of resource exhaustion. */
		stilt_error(a_stilt, STILTE_LIMITCHECK);
		goto FORK_ERROR;
	case 0:
		/* Child. */
		execve(path, argv, envp);
		/*
		 * If we get here, then the execve() call failed.  Get an error
		 * back to the parent.
		 */
		exit(1);
	}

	/*
	 * Only the parent gets to here.
	 */
	waitpid(pid, &status, 0);
	if (WIFEXITED(status)) {
		/* Normal program exit. */
		result = WEXITSTATUS(status);
	} else {
		/*
		 * Program termination due to a signal.  Set a negative
		 * return value.
		 */
		result = -WTERMSIG(status);
	}

	/* Replace the top stack element (array) with the return value. */
	stilo_integer_new(array, result);

	/*
	 * Clean up.
	 */
	FORK_ERROR:
	ENVP_ERROR:
	for (i = 0; envp[i] != NULL; i++)
		_cw_free(envp[i]);
	_cw_free(envp);
	ARGV_ERROR:
	for (i = 0; argv[i] != NULL; i++)
		_cw_free(argv[i]);
	_cw_free(argv);
	_cw_free(path);
	PATH_ERROR:
	VALIDATION_ERROR:
	stils_pop(tstack);
#endif
}

void
systemdict_tell(cw_stilt_t *a_stilt)
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
systemdict_test(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_thread(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*mark, *stilo, *new;
	union {
		cw_sint64_t	i;
		cw_stilt_t	*stilt;
	} u;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	/*
	 * Find the first mark on ostack, which demarks the objects to be moved
	 * to the new stilt's stacks.
	 */
	STILS_GET(mark, ostack, a_stilt);
	stilo = mark;
	while (stilo_type_get(mark) != STILOT_MARK)
		STILS_DOWN_GET(mark, ostack, a_stilt, mark);

	/*
	 * Create the new stilt.
	 */
	u.i = 0;
	u.stilt = stilt_new(NULL, stilt_stil_get(a_stilt));

	/*
	 * Move objects over to the new stilt.
	 */
	if (stilo != mark) {
		new = stils_push(stilt_estack_get(u.stilt));
		stilo_dup(new, stilo);
		stils_pop(ostack);
	}
	for (stilo = stils_get(ostack); stilo != mark; stilo =
	     stils_get(ostack)) {
		new = stils_push(stilt_ostack_get(u.stilt));
		stilo_dup(new, stilo);
		stils_pop(ostack);
	}

	/*
	 * Put the context on ostack.
	 */
	stilo_integer_new(mark, u.i);

	/*
	 * Start the thread.
	 */
	stilt_thread(u.stilt);
}

void
systemdict_timedwait(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*condition, *mutex, *nsecs;
	struct timespec	timeout;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(nsecs, ostack, a_stilt);
	STILS_DOWN_GET(mutex, ostack, a_stilt, nsecs);
	STILS_DOWN_GET(condition, ostack, a_stilt, mutex);
	if (stilo_type_get(condition) != STILOT_CONDITION ||
	    stilo_type_get(mutex) != STILOT_MUTEX || stilo_type_get(nsecs) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(nsecs) < 0) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	/* Convert integer to timespec. */
	timeout.tv_nsec = stilo_integer_get(nsecs) % 1000000000;
	timeout.tv_sec = stilo_integer_get(nsecs) / 1000000000;

	stilo_condition_timedwait(condition, mutex, &timeout);

	stils_npop(ostack, 3);
}

void
systemdict_token(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*stilo, *tstilo;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	switch (stilo_type_get(stilo)) {
	case STILOT_STRING: {
		cw_stilts_t	stilts;
		cw_uint32_t	nscanned, scount;

		scount = stils_count(ostack);
		tstilo = stils_push(tstack);
		stilo_dup(tstilo, stilo);
		stilts_new(&stilts);

		/* Reset the error value. */
		stilt_error_set(a_stilt, STILTE_NONE);

		stilo_string_lock(tstilo);
		nscanned = stilt_token(a_stilt, &stilts,
		    stilo_string_get(tstilo), stilo_string_len_get(tstilo));
		stilo_string_unlock(tstilo);
		if (stilt_error_get(a_stilt)) {
			stils_pop(tstack);
			stilts_delete(&stilts, a_stilt);
			return;
		}

		stilt_flush(a_stilt, &stilts);
		if (stilt_error_get(a_stilt)) {
			stils_pop(tstack);
			stilts_delete(&stilts, a_stilt);
			return;
		}

		if (stilt_state(a_stilt) == STILTTS_START &&
		    stilt_deferred(a_stilt) == FALSE && stils_count(ostack) ==
		    scount + 1) {
			/* Success. */
			stilo_string_substring_new(stilo, tstilo,
			    stilt_stil_get(a_stilt), nscanned,
			    stilo_string_len_get(tstilo) - nscanned);
			stilo = stils_push(ostack);
			stilo_boolean_new(stilo, TRUE);
		} else {
			cw_uint32_t	i;

			/*
			 * We failed to scan a token.  Reset a_stilt and clean
			 * up ostack.
			 */
			stilt_reset(a_stilt);
			for (i = stils_count(ostack); i > scount; i--)
				stils_pop(ostack);

			stilo_boolean_new(stilo, FALSE);
		}
		stilts_delete(&stilts, a_stilt);
		stils_pop(tstack);
		break;
	}
	case STILOT_FILE: {
		cw_stilts_t	stilts;
		cw_sint32_t	nread;
		cw_uint32_t	scount;
		cw_uint8_t	c;

		scount = stils_count(ostack);
		tstilo = stils_push(tstack);
		stilo_dup(tstilo, stilo);
		stilts_new(&stilts);

		/* Reset the error value. */
		stilt_error_set(a_stilt, STILTE_NONE);

		/*
		 * Feed the scanner one byte at a time, checking after every
		 * character whether a token was accepted.  If we run out of
		 * data, flush the scanner in the hope of causing token
		 * acceptance.
		 */
		for (nread = stilo_file_read(tstilo, 1, &c); nread > 0; nread =
		    stilo_file_read(tstilo, 1, &c)) {
			stilt_token(a_stilt, &stilts, &c, 1);
			if (stilt_error_get(a_stilt)) {
				stils_pop(tstack);
				stilts_delete(&stilts, a_stilt);
				return;
			}
			if (stilt_state(a_stilt) == STILTTS_START &&
			    stilt_deferred(a_stilt) == FALSE &&
			    stils_count(ostack) == scount + 1)
				goto SUCCESS;
		}
		stilt_flush(a_stilt, &stilts);
		if (stilt_error_get(a_stilt)) {
			stils_pop(tstack);
			stilts_delete(&stilts, a_stilt);
			return;
		}

		if (stilt_state(a_stilt) == STILTTS_START &&
		    stilt_deferred(a_stilt) == FALSE && stils_count(ostack) ==
		    scount + 1) {
			/* Success. */
			SUCCESS:
			stilo_boolean_new(stilo, TRUE);
			stils_roll(ostack, 2, 1);
		} else {
			cw_uint32_t	i;

			/*
			 * We failed to scan a token.  Reset a_stilt and clean
			 * up ostack.
			 */
			stilt_reset(a_stilt);
			for (i = stils_count(ostack); i > scount; i--)
				stils_pop(ostack);

			stilo_boolean_new(stilo, FALSE);
		}
		stilts_delete(&stilts, a_stilt);
		stils_pop(tstack);
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
}

void
systemdict_truncate(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *length;
	cw_stilte_t	error;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(length, ostack, a_stilt);
	STILS_DOWN_GET(file, ostack, a_stilt, length);
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(length) !=
	    STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(length) < 0) {
		stilt_error(a_stilt, STILTE_RANGECHECK);
		return;
	}

	error = stilo_file_truncate(file, stilo_integer_get(length));
	if (error) {
		stilt_error(a_stilt, error);
		return;
	}

	stils_npop(ostack, 2);
}

void
systemdict_trylock(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*mutex;
	cw_bool_t	error;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(mutex, ostack, a_stilt);
	if (stilo_type_get(mutex) != STILOT_MUTEX) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	error = stilo_mutex_trylock(mutex);

	stilo_boolean_new(mutex, error);
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

	stilo_name_new(stilo, stilt_stil_get(a_stilt),
	    stiln_str(typenames[type]), stiln_len(typenames[type]), TRUE);
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

	stilo_dict_undef(dict, stilt_stil_get(a_stilt), key);

	stils_npop(ostack, 2);
}

void
systemdict_unlink(cw_stilt_t *a_stilt)
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
	stilo_string_lock(string);
	memcpy(str, stilo_string_get(string), nbytes);
	stilo_string_unlock(string);
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
systemdict_unlock(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*mutex;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(mutex, ostack, a_stilt);
	if (stilo_type_get(mutex) != STILOT_MUTEX) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_mutex_unlock(mutex);

	stils_pop(ostack);
}

void
systemdict_version(cw_stilt_t *a_stilt)
{
	_cw_stil_code(a_stilt, "(" _LIBSTIL_VERSION ")");
}

void
systemdict_wait(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*condition, *mutex;

	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(mutex, ostack, a_stilt);
	STILS_DOWN_GET(condition, ostack, a_stilt, mutex);
	if (stilo_type_get(condition) != STILOT_CONDITION ||
	    stilo_type_get(mutex) != STILOT_MUTEX) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	stilo_condition_wait(condition, mutex);

	stils_npop(ostack, 2);
}

void
systemdict_waitpid(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	pid_t		pid;
	int		status;
	cw_sint64_t	result;
	
	ostack = stilt_ostack_get(a_stilt);
	STILS_GET(stilo, ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}
	pid = stilo_integer_get(stilo);

	waitpid(pid, &status, 0);
	if (WIFEXITED(status)) {
		/* Normal program exit. */
		result = WEXITSTATUS(status);
	} else {
		/*
		 * Program termination due to a signal.  Set a negative
		 * return value.
		 */
		result = -WTERMSIG(status);
	}

	stilo_integer_new(stilo, result);
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
		if (stilo_dict_lookup(dict, key, NULL) == FALSE) {
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

	ostack = stilt_ostack_get(a_stilt);

	STILS_GET(value, ostack, a_stilt);
	STILS_DOWN_GET(file, ostack, a_stilt, value);
	
	if (stilo_type_get(file) != STILOT_FILE) {
		stilt_error(a_stilt, STILTE_TYPECHECK);
		return;
	}

	switch (stilo_type_get(value)) {
	case STILOT_INTEGER: {
		cw_uint8_t	val;

		val = (cw_uint8_t)stilo_integer_get(value);
		error = stilo_file_write(file, &val, 1);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	}
	case STILOT_STRING:
		stilo_string_lock(value);
		error = stilo_file_write(file, stilo_string_get(value),
		    stilo_string_len_get(value));
		stilo_string_unlock(value);
		if (error) {
			stilt_error(a_stilt, error);
			return;
		}
		break;
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
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
	thd_yield();
}
