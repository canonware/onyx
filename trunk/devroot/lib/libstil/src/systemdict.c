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

#define	_SYSTEMDICT_C_

#include "../include/libstil/libstil.h"

#include <sys/time.h>	/* For realtime operator. */
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_array_l.h"
#include "../include/libstil/stilo_operator_l.h"

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
	ENTRY(catenate),
	ENTRY(cd),
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
	ENTRY(fino),
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
	ENTRY(ostack),
	ENTRY(pop),
	ENTRY(print),
	ENTRY(put),
	ENTRY(putinterval),
	ENTRY(pwd),
	ENTRY(quit),
	ENTRY(rand),
	ENTRY(read),
	ENTRY(readline),
	ENTRY(realtime),
	ENTRY(renamefile),
	ENTRY(repeat),
	ENTRY(roll),
	ENTRY(run),
	ENTRY(sclear),
	ENTRY(scleartomark),
	ENTRY(scount),
	ENTRY(scounttomark),
	ENTRY(sdup),
	ENTRY(seek),
	ENTRY(self),
	ENTRY(setlocking),
	ENTRY(sexch),
	ENTRY(shift),
	ENTRY(signal),
	ENTRY(sindex),
	ENTRY(spop),
	ENTRY(sprint),
	ENTRY(spush),
	ENTRY(srand),
	ENTRY(sroll),
	ENTRY(stack),
	ENTRY(start),
	ENTRY(stat),
	ENTRY(stop),
	ENTRY(stopped),
	ENTRY(store),
	ENTRY(string),
	ENTRY(sub),
	{STILN_sym_hash_bang, systemdict_mark},
	{STILN_sym_bang_hash, systemdict_cleartomark},
	{STILN_sym_lp, systemdict_fino},
	ENTRY(sym_rp),
	{STILN_sym_lt, systemdict_mark},
	ENTRY(sym_gt),
	{STILN_sym_lb, systemdict_mark},
	ENTRY(sym_rb),
	ENTRY(symlink),
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

#define	NEXTRA	12
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

	/* sprintdict. */
	stilo_name_new(&name, a_stil, stiln_str(STILN_sprintdict),
	    stiln_len(STILN_sprintdict), TRUE);
	stilo_dup(&value, stil_sprintdict_get(a_stil));
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
systemdict_abs(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*a;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(a, ostack, a_thread);
	if (stilo_type_get(a) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	if (stilo_integer_get(a) < 0)
		stilo_integer_set(a, -stilo_integer_get(a));
}

void
systemdict_add(cw_stilo_t *a_thread)
{
	systemdict_inline_add(a_thread);
}

void
systemdict_aload(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*array, *stilo;
	cw_uint32_t	i, len;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(array, ostack, a_thread);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	for (i = 0, len = stilo_array_len_get(array); i < len; i++) {
		stilo = stilo_stack_under_push(ostack, array);
		stilo_array_el_set(array, stilo, i);
	}
}

void
systemdict_and(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

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
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_stack_pop(ostack);
}

void
systemdict_array(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stiloi_t	len;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	len = stilo_integer_get(stilo);
	if (len < 0) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}

	stilo_array_new(stilo, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), len);
}

void
systemdict_astore(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack;
	cw_stilo_t	*array, *stilo;
	cw_sint32_t	i, len;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	STILO_STACK_GET(array, ostack, a_thread);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	/* Make sure there will be enough objects to fill the array. */
	len = stilo_array_len_get(array);
	if (len > stilo_stack_count(ostack) - 1) {
		stilo_thread_error(a_thread, STILO_THREADE_STACKUNDERFLOW);
		return;
	}

	stilo = stilo_stack_push(tstack);
	stilo_dup(stilo, array);
	stilo_stack_pop(ostack);
	array = stilo;

	/* Move ostack objects to the array. */
	for (i = len - 1; i >= 0; i--) {
		stilo_array_el_set(array, stilo_stack_get(ostack), i);
		stilo_stack_pop(ostack);
	}

	/* Push the array back onto ostack. */
	stilo = stilo_stack_push(ostack);
	stilo_dup(stilo, array);
	stilo_stack_pop(tstack);
}

void
systemdict_begin(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *dstack;
	cw_stilo_t	*stilo, *dict;

	dstack = stilo_thread_dstack_get(a_thread);
	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(dict, ostack, a_thread);
	if (stilo_type_get(dict) != STILOT_DICT) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	
	stilo = stilo_stack_push(dstack);
	stilo_dup(stilo, dict);
	stilo_stack_pop(ostack);
}

static void
systemdict_p_bind(cw_stilo_t *a_proc, cw_stilo_t *a_thread)
{
	cw_stilo_t	*tstack;
	cw_stilo_t	*el, *val;
	cw_uint32_t	i, count;
	/*
	 * Array of fastops.  stilo_thread_loop() must have corresponding
	 * handlers.
	 */
	static const struct cw_systemdict_entry fastops[] = {
		ENTRY(add),
		ENTRY(dup),
		ENTRY(exch),
		ENTRY(index),
		ENTRY(pop),
		ENTRY(roll)
	};

	tstack = stilo_thread_tstack_get(a_thread);

	val = stilo_stack_push(tstack);
	el = stilo_stack_push(tstack);

	stilo_l_array_bound_set(a_proc, TRUE);

	for (i = 0, count = stilo_array_len_get(a_proc); i < count; i++) {
		stilo_array_el_get(a_proc, i, el);
		if (stilo_attrs_get(el) != STILOA_EXECUTABLE)
			continue;

		switch (stilo_type_get(el)) {
		case STILOT_ARRAY:
			if (stilo_l_array_bound_get(el) == FALSE)
				systemdict_p_bind(el, a_thread);
			break;
		case STILOT_NAME:
			if (stilo_thread_dstack_search(a_thread, el, val) ==
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
							stilo_array_el_set(
							    a_proc, el, i);
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

	stilo_stack_npop(tstack, 2);
}

void
systemdict_bind(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*array;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(array, ostack, a_thread);

	systemdict_p_bind(array, a_thread);
}

void
systemdict_broadcast(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*condition;
	
	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(condition, ostack, a_thread);
	if (stilo_type_get(condition) != STILOT_CONDITION) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_condition_broadcast(condition);

	stilo_stack_pop(ostack);
}

void
systemdict_bytesavailable(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*file;
	cw_uint32_t	bytes;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(file, ostack, a_thread);
	
	if (stilo_type_get(file) != STILOT_FILE) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	bytes = stilo_file_buffer_count(file);
	stilo_integer_new(file, bytes);
}

void
systemdict_catenate(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *a, *b, *r;
	cw_uint32_t	i, len_a, len_b;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(b, ostack, a_thread);
	STILO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (stilo_type_get(a) != stilo_type_get(b) || (stilo_type_get(a) !=
	    STILOT_ARRAY && stilo_type_get(a) != STILOT_STACK &&
	    stilo_type_get(a) != STILOT_STRING)) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	r = stilo_stack_under_push(ostack, a);

	switch (stilo_type_get(a)) {
	case STILOT_ARRAY: {
		cw_stilo_t	*tstack, *tstilo;
		
		tstack = stilo_thread_tstack_get(a_thread);
		tstilo = stilo_stack_push(tstack);

		len_a = stilo_array_len_get(a);
		len_b = stilo_array_len_get(b);

		stilo_array_new(r, stilo_thread_stil_get(a_thread),
		    stilo_thread_currentlocking(a_thread), len_a + len_b);

		for (i = 0; i < len_a; i++) {
			stilo_array_el_get(a, i, tstilo);
			stilo_array_el_set(r, tstilo, i);
		}
		for (i = 0; i < len_b; i++) {
			stilo_array_el_get(b, i, tstilo);
			stilo_array_el_set(r, tstilo, i + len_a);
		}

		stilo_stack_pop(tstack);

		break;
	}
	case STILOT_STACK: {
		cw_stilo_t	*fr, *to;

		len_a = stilo_stack_count(a);
		len_b = stilo_stack_count(b);

		stilo_stack_new(r, stilo_thread_stil_get(a_thread),
		    stilo_thread_currentlocking(a_thread));

		for (i = 0, fr = to = NULL; i < len_b; i++) {
			fr = stilo_stack_down_get(b, fr);
			to = stilo_stack_under_push(r, to);
			stilo_dup(to, fr);
		}
		for (i = 0, fr = NULL; i < len_a; i++) {
			fr = stilo_stack_down_get(a, fr);
			to = stilo_stack_under_push(r, to);
			stilo_dup(to, fr);
		}

		break;
	}
	case STILOT_STRING:
		len_a = stilo_string_len_get(a);
		len_b = stilo_string_len_get(b);

		stilo_string_new(r, stilo_thread_stil_get(a_thread),
		    stilo_thread_currentlocking(a_thread), len_a + len_b);

		stilo_string_lock(r);
		stilo_string_lock(a);
		stilo_string_set(r, 0, stilo_string_get(a), len_a);
		stilo_string_unlock(a);

		stilo_string_lock(b);
		stilo_string_set(r, len_a, stilo_string_get(b), len_b);
		stilo_string_unlock(b);
		stilo_string_unlock(r);

		break;
	default:
		_cw_not_reached();
	}

	stilo_stack_npop(ostack, 2);
}

void
systemdict_cd(cw_stilo_t *a_thread)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_chmod(cw_stilo_t *a_thread)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_chown(cw_stilo_t *a_thread)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_clear(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_uint32_t	count;

	ostack = stilo_thread_ostack_get(a_thread);
	count = stilo_stack_count(ostack);
	if (count > 0)
		stilo_stack_npop(ostack, count);
}

void
systemdict_cleardstack(cw_stilo_t *a_thread)
{
	cw_stilo_t	*dstack;
	cw_uint32_t	count;

	dstack = stilo_thread_dstack_get(a_thread);
	count = stilo_stack_count(dstack);
	if (count > 3)
		stilo_stack_npop(dstack, count - 3);
}

void
systemdict_cleartomark(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	cw_uint32_t	i, depth;

	ostack = stilo_thread_ostack_get(a_thread);

	for (i = 0, depth = stilo_stack_count(ostack), stilo = NULL; i < depth;
	     i++) {
		stilo = stilo_stack_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilo_thread_error(a_thread, STILO_THREADE_UNMATCHEDMARK);
		return;
	}

	stilo_stack_npop(ostack, i + 1);
}

void
systemdict_closefile(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack;
	cw_stilo_t		*stilo;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_FILE) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	error = stilo_file_close(stilo);
	if (error) {
		stilo_thread_error(a_thread, STILO_THREADE_IOERROR);
		return;
	}

	stilo_stack_pop(ostack);
}

void
systemdict_condition(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*condition;

	ostack = stilo_thread_ostack_get(a_thread);
	condition = stilo_stack_push(ostack);
	stilo_condition_new(condition, stilo_thread_stil_get(a_thread));
}

void
systemdict_copy(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);

	switch (stilo_type_get(stilo)) {
	case STILOT_INTEGER: {
		cw_stilo_t	*dup;
		cw_uint32_t	i;
		cw_stiloi_t	count;

		/* Dup a range of the stack. */
		count = stilo_integer_get(stilo);
		if (count < 0) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}
		if (count > stilo_stack_count(ostack) - 1) {
			stilo_thread_error(a_thread,
			    STILO_THREADE_STACKUNDERFLOW);
			return;
		}
		stilo_stack_pop(ostack);

		/*
		 * Iterate down the stack, creating dup's along the way.  Since
		 * we're going down, it's necessary to use
		 * stilo_stack_under_push() to preserve order.
		 */
		for (i = 0, stilo = NULL, dup = NULL; i < count; i++) {
			stilo = stilo_stack_down_get(ostack, stilo);
			dup = stilo_stack_under_push(ostack, dup);
			stilo_dup(dup, stilo);
		}
		break;
	}
	case STILOT_ARRAY: {
		cw_stilo_t	*orig;

		STILO_STACK_DOWN_GET(orig, ostack, a_thread, stilo);
		if (stilo_type_get(orig) != STILOT_ARRAY) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}
		if (stilo_array_len_get(stilo) < stilo_array_len_get(orig)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}

		stilo_array_copy(stilo, orig);

		stilo_stack_roll(ostack, 2, 1);
		stilo_stack_pop(ostack);
		break;
	}
	case STILOT_DICT: {
		cw_stilo_t	*orig;

		STILO_STACK_DOWN_GET(orig, ostack, a_thread, stilo);
		if (stilo_type_get(orig) != STILOT_DICT) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}

		stilo_dict_copy(stilo, orig, stilo_thread_stil_get(a_thread),
		    stilo_thread_currentlocking(a_thread));
		break;
	}
	case STILOT_STACK: {
		cw_stilo_t	*orig;

		STILO_STACK_DOWN_GET(orig, ostack, a_thread, stilo);
		if (stilo_type_get(orig) != STILOT_STACK) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}

		stilo_stack_copy(stilo, orig);

		stilo_stack_roll(ostack, 2, 1);
		stilo_stack_pop(ostack);
		break;
	}
	case STILOT_STRING: {
		cw_stilo_t	*orig;

		STILO_STACK_DOWN_GET(orig, ostack, a_thread, stilo);
		if (stilo_type_get(orig) != STILOT_STRING) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}
		if (stilo_string_len_get(stilo) < stilo_string_len_get(orig)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}

		stilo_string_copy(stilo, orig);

		stilo_stack_roll(ostack, 2, 1);
		stilo_stack_pop(ostack);
		break;
	}
	default:
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
}

void
systemdict_count(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);

	stilo = stilo_stack_push(ostack);
	stilo_integer_new(stilo, stilo_stack_count(ostack) - 1);
}

void
systemdict_countdstack(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *dstack;
	cw_stilo_t	*stilo;

	dstack = stilo_thread_dstack_get(a_thread);
	ostack = stilo_thread_ostack_get(a_thread);

	stilo = stilo_stack_push(ostack);
	stilo_integer_new(stilo, stilo_stack_count(dstack));
}

void
systemdict_countestack(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack;
	cw_stilo_t	*stilo;

	estack = stilo_thread_estack_get(a_thread);
	ostack = stilo_thread_ostack_get(a_thread);

	stilo = stilo_stack_push(ostack);
	stilo_integer_new(stilo, stilo_stack_count(estack));
}

void
systemdict_counttomark(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	cw_uint32_t	i, depth;

	ostack = stilo_thread_ostack_get(a_thread);

	for (i = 0, depth = stilo_stack_count(ostack), stilo = NULL; i < depth;
	    i++) {
		stilo = stilo_stack_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilo_thread_error(a_thread, STILO_THREADE_UNMATCHEDMARK);
		return;
	}

	stilo = stilo_stack_push(ostack);
	stilo_integer_new(stilo, i);
}

void
systemdict_currentdict(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *dstack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	dstack = stilo_thread_dstack_get(a_thread);

	stilo = stilo_stack_push(ostack);
	stilo_dup(stilo, stilo_stack_get(dstack));
}

void
systemdict_currentfile(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack;
	cw_stilo_t	*file, *stilo;
	cw_uint32_t	i, depth;

	estack = stilo_thread_estack_get(a_thread);
	ostack = stilo_thread_ostack_get(a_thread);

	file = stilo_stack_push(ostack);
	for (i = 0, depth = stilo_stack_count(estack), stilo = NULL; i < depth;
	     i++) {
		stilo = stilo_stack_down_get(estack, stilo);
		if (stilo_type_get(stilo) == STILOT_FILE) {
			stilo_dup(file, stilo);
			break;
		}
	}
	if (i == depth) {
		stilo_file_new(file, stilo_thread_stil_get(a_thread),
		    stilo_thread_currentlocking(a_thread));
	}
}

void
systemdict_currentlocking(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	stilo = stilo_stack_push(ostack);
	stilo_boolean_new(stilo, stilo_thread_currentlocking(a_thread));
}

void
systemdict_cvlit(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	stilo_attrs_set(stilo, STILOA_LITERAL);
}

void
systemdict_cvn(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack;
	cw_stilo_t	*stilo, *tstilo;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);

	if (stilo_type_get(stilo) != STILOT_STRING) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	tstilo = stilo_stack_push(tstack);
	stilo_dup(tstilo, stilo);

	stilo_string_lock(tstilo);
	stilo_name_new(stilo, stilo_thread_stil_get(a_thread),
	    stilo_string_get(tstilo), stilo_string_len_get(tstilo), FALSE);
	stilo_string_unlock(tstilo);
	stilo_attrs_set(stilo, stilo_attrs_get(tstilo));

	stilo_stack_pop(tstack);
}

void
systemdict_cvrs(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*num, *radix;
	cw_uint64_t	val;
	cw_uint32_t	i, rlen, base;
	cw_uint8_t	*str;
	static const cw_uint8_t *syms = "0123456789abcdefghijklmnopqrstuvwxyz";
	cw_uint8_t	*result, s_result[65] =
	    "0000000000000000000000000000000000000000000000000000000000000000";

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(radix, ostack, a_thread);
	STILO_STACK_DOWN_GET(num, ostack, a_thread, radix);
	if ((stilo_type_get(num) != STILOT_INTEGER) || (stilo_type_get(radix) !=
	    STILOT_INTEGER)) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	base = stilo_integer_get(radix);
	if (base < 2 || base > 36) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
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

	stilo_string_new(num, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), rlen);

	str = stilo_string_get(num);
	stilo_string_lock(num);
	memcpy(str, result, rlen);
	stilo_string_unlock(num);

	stilo_stack_pop(ostack);
}

void
systemdict_cvs(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	
	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);

	switch (stilo_type_get(stilo)) {
	case STILOT_BOOLEAN:
		_cw_stil_code(a_thread, "{`true'} {`false'} ifelse");
		break;
	case STILOT_INTEGER: {
		cw_uint8_t	result[21];
		cw_sint32_t	len;

#if (_CW_STILOI_SIZEOF == 8)
		len = _cw_out_put_s(result, "[q|s:s]",
		    stilo_integer_get(stilo));
#elif (_CW_STILOI_SIZEOF == 4)
		len = _cw_out_put_s(result, "[i|s:s]",
		    stilo_integer_get(stilo));
#else
#error "Unsupported integer size"
#endif
		stilo_string_new(stilo, stilo_thread_stil_get(a_thread),
		    stilo_thread_currentlocking(a_thread), len);
		stilo_string_lock(stilo);
		stilo_string_set(stilo, 0, result, len);
		stilo_string_unlock(stilo);
		break;
	}
	case STILOT_NAME: {
		cw_stilo_t	*tstack;
		cw_stilo_t	*tstilo;

		tstack = stilo_thread_tstack_get(a_thread);
		tstilo = stilo_stack_push(tstack);
		stilo_dup(tstilo, stilo);

		stilo_string_new(stilo, stilo_thread_stil_get(a_thread),
		    stilo_thread_currentlocking(a_thread),
		    stilo_name_len_get(tstilo));
		stilo_string_lock(stilo);
		stilo_string_set(stilo, 0, stilo_name_str_get(tstilo),
		    stilo_name_len_get(tstilo));
		stilo_string_unlock(stilo);

		stilo_stack_pop(tstack);
		break;
	}
	case STILOT_OPERATOR: {
		cw_stiln_t	stiln;

		stiln = stilo_l_operator_fast_op_stiln(stilo);
		if (stiln > STILN_LAST)
			_cw_stil_code(a_thread, "pop `--operator--'");
		else {
			cw_stilo_t	*tstack;
			cw_stilo_t	*tstilo;

			tstack = stilo_thread_tstack_get(a_thread);
			tstilo = stilo_stack_push(tstack);
			stilo_dup(tstilo, stilo);

			stilo_string_new(stilo, stilo_thread_stil_get(a_thread),
			    stilo_thread_currentlocking(a_thread),
			    stiln_len(stiln));
			stilo_string_lock(stilo);
			stilo_string_set(stilo, 0, stiln_str(stiln),
			    stiln_len(stiln));
			stilo_string_unlock(stilo);

			stilo_stack_pop(tstack);
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
		_cw_stil_code(a_thread, "pop `--nostringval--'");
		break;
	}
}

void
systemdict_cvx(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	stilo_attrs_set(stilo, STILOA_EXECUTABLE);
}

void
systemdict_def(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *dstack;
	cw_stilo_t	*dict, *key, *val;

	ostack = stilo_thread_ostack_get(a_thread);
	dstack = stilo_thread_dstack_get(a_thread);

	dict = stilo_stack_get(dstack);
	STILO_STACK_GET(val, ostack, a_thread);
	STILO_STACK_DOWN_GET(key, ostack, a_thread, val);

	stilo_dict_def(dict, stilo_thread_stil_get(a_thread), key, val);

	stilo_stack_npop(ostack, 2);
}

void
systemdict_detach(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*thread;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(thread, ostack, a_thread);
	if (stilo_type_get(thread) != STILOT_THREAD) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_thread_detach(thread);

	stilo_stack_pop(ostack);
}

void
systemdict_dict(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*dict;

	ostack = stilo_thread_ostack_get(a_thread);

	dict = stilo_stack_push(ostack);
	stilo_dict_new(dict, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), _CW_SYSTEMDICT_DICT_SIZE);
}

void
systemdict_dirforeach(cw_stilo_t *a_thread)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_div(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(b, ostack, a_thread);
	STILO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(b) == 0) {
		stilo_thread_error(a_thread, STILO_THREADE_UNDEFINEDRESULT);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) / stilo_integer_get(b));
	stilo_stack_pop(ostack);
}

void
systemdict_dstack(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *dstack, *stack;

	ostack = stilo_thread_ostack_get(a_thread);
	dstack = stilo_thread_dstack_get(a_thread);

	stack = stilo_stack_push(ostack);
	stilo_stack_new(stack, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread));
	stilo_stack_copy(stack, dstack);
}

void
systemdict_dup(cw_stilo_t *a_thread)
{
	systemdict_inline_dup(a_thread);
}

void
systemdict_end(cw_stilo_t *a_thread)
{
	cw_stilo_t	*dstack;

	dstack = stilo_thread_dstack_get(a_thread);

	/* threaddict, systemdict, globaldict, and userdict cannot be popped. */
	if (stilo_stack_count(dstack) <= 4) {
		stilo_thread_error(a_thread, STILO_THREADE_DSTACKUNDERFLOW);
		return;
	}

	stilo_stack_pop(dstack);
}

void
systemdict_eq(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_sint32_t	result;
	cw_bool_t	eq;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

	result = stilo_compare(stilo_a, stilo_b);
	if (result == 0)
		eq = TRUE;
	else
		eq = FALSE;

	stilo_boolean_new(stilo_a, eq);

	stilo_stack_pop(ostack);
}

void
systemdict_estack(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack, *stack;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);

	stack = stilo_stack_push(ostack);
	stilo_stack_new(stack, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread));
	stilo_stack_copy(stack, estack);
}

void
systemdict_eval(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack;
	cw_stilo_t	*orig, *new;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);

	STILO_STACK_GET(orig, ostack, a_thread);
	new = stilo_stack_push(estack);
	stilo_dup(new, orig);
	stilo_stack_pop(ostack);

	stilo_thread_loop(a_thread);
}

void
systemdict_exch(cw_stilo_t *a_thread)
{
	systemdict_inline_exch(a_thread);
}

void
systemdict_exec(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack;
	cw_stilo_t	*array, *el;
	cw_uint32_t	i, slen, argc;
	char		*path, **argv, **envp;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	el = stilo_stack_push(tstack);
	
	STILO_STACK_GET(array, ostack, a_thread);
	if (stilo_type_get(array) != STILOT_ARRAY) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		goto VALIDATION_ERROR;
	}
	argc = stilo_array_len_get(array);
	for (i = 0; i < argc; i++) {
		stilo_array_el_get(array, i, el);
		if (stilo_type_get(el) != STILOT_STRING) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			goto VALIDATION_ERROR;
		}
	}

	/*
	 * Construct path.
	 */
	stilo_array_el_get(array, 0, el);
	if (stilo_type_get(el) != STILOT_STRING) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
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
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
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

		key = stilo_stack_push(tstack);
		val = stilo_stack_push(tstack);

		dcount = stilo_dict_count(stil_envdict_get(
		    stilo_thread_stil_get(a_thread)));
		envp = (char **)_cw_malloc(sizeof(char *) * (dcount + 1));
		for (i = 0; i < dcount; i++) {
			/* Get key and val. */
			stilo_dict_iterate(stil_envdict_get(
			    stilo_thread_stil_get(a_thread)), key);
			stilo_dict_lookup(stil_envdict_get(
			    stilo_thread_stil_get(a_thread)), key, val);
			if (stilo_type_get(key) != STILOT_NAME ||
			    stilo_type_get(val) != STILOT_STRING) {
				stilo_thread_error(a_thread,
				    STILO_THREADE_TYPECHECK);
				stilo_stack_npop(tstack, 2);
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

		stilo_stack_npop(tstack, 2);
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
	stilo_stack_pop(tstack);
}

void
systemdict_exit(cw_stilo_t *a_thread)
{
	xep_throw(_CW_STILX_EXIT);
}

void
systemdict_exp(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*a, *b;
	cw_uint32_t	i;
	cw_stiloi_t	r;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(b, ostack, a_thread);
	STILO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	if (stilo_integer_get(b) < 0) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	} else if (stilo_integer_get(b) > 0) {
		for (i = 1, r = stilo_integer_get(a); i < stilo_integer_get(b);
		     i++)
			r *= stilo_integer_get(a);
	} else
		r = 1;

	stilo_integer_set(a, r);
	stilo_stack_pop(ostack);
}

void
systemdict_fino(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	stilo = stilo_stack_push(ostack);
	stilo_fino_new(stilo);
}

void
systemdict_flush(cw_stilo_t *a_thread)
{
	cw_stilo_threade_t	error;

	error = stilo_file_buffer_flush(stil_stdout_get(stilo_thread_stil_get(
	    a_thread)));
	if (error)
		stilo_thread_error(a_thread, error);
}

void
systemdict_flushfile(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack;
	cw_stilo_t		*file;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(file, ostack, a_thread);
	
	if (stilo_type_get(file) != STILOT_FILE) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	error = stilo_file_buffer_flush(file);
	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}
	
	stilo_stack_pop(ostack);
}

void
systemdict_for(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack, *tstack;
	cw_stilo_t	*exec, *ostilo, *estilo, *tstilo;
	cw_stiloi_t	i, inc, limit, edepth, tdepth;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	STILO_STACK_GET(exec, ostack, a_thread);

	STILO_STACK_DOWN_GET(ostilo, ostack, a_thread, exec);
	if (stilo_type_get(ostilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	limit = stilo_integer_get(ostilo);

	STILO_STACK_DOWN_GET(ostilo, ostack, a_thread, ostilo);
	if (stilo_type_get(ostilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	inc = stilo_integer_get(ostilo);

	STILO_STACK_DOWN_GET(ostilo, ostack, a_thread, ostilo);
	if (stilo_type_get(ostilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	i = stilo_integer_get(ostilo);

	/* Move the object to be executed to tstack. */
	tstilo = stilo_stack_push(tstack);
	stilo_dup(tstilo, exec);
	stilo_stack_npop(ostack, 4);

	/*
	 * Record estack's and tstack's depth so that we can clean up if
	 * necessary.
	 */
	edepth = stilo_stack_count(estack);
	tdepth = stilo_stack_count(tstack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		if (inc >= 0) {
			for (; i <= limit; i += inc) {
				/*
				 * Dup the object to execute onto the execution
				 * stack.
				 */
				estilo = stilo_stack_push(estack);
				stilo_dup(estilo, tstilo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				ostilo = stilo_stack_push(ostack);
				stilo_integer_new(ostilo, i);

				stilo_thread_loop(a_thread);
			}
		} else {
			for (; i >= limit; i += inc) {
				/*
				 * Dup the object to execute onto the execution
				 * stack.
				 */
				estilo = stilo_stack_push(estack);
				stilo_dup(estilo, tstilo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				ostilo = stilo_stack_push(ostack);
				stilo_integer_new(ostilo, i);

				stilo_thread_loop(a_thread);
			}
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		for (i = stilo_stack_count(estack); i > edepth; i--)
			stilo_stack_pop(estack);

		/* Clean up tstack. */
		stilo_stack_npop(tstack, stilo_stack_count(tstack) - tdepth);
	}
	xep_end();

	stilo_stack_pop(tstack);
}

void
systemdict_foreach(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack, *tstack;
	cw_stilo_t	*stilo, *what, *proc;
	cw_uint32_t	tdepth;
	cw_stiloi_t	i, count;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	tdepth = stilo_stack_count(tstack);

	STILO_STACK_GET(proc, ostack, a_thread);
	STILO_STACK_DOWN_GET(what, ostack, a_thread, proc);

	xep_begin();
	xep_try {
		switch (stilo_type_get(what)) {
		case STILOT_ARRAY: {
			cw_stilo_t	*el;

			/* Move proc and array to tstack. */
			stilo = stilo_stack_push(tstack);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stilo_stack_push(tstack);
			stilo_dup(stilo, what);
			what = stilo;

			stilo_stack_npop(ostack, 2);

			/*
			 * Iterate through the array, push each element onto
			 * ostack, and execute proc.
			 */
			el = stilo_stack_push(tstack);
			for (i = 0, count = stilo_array_len_get(what); i <
				 count; i++) {
				stilo_array_el_get(what, i, el);
				stilo = stilo_stack_push(ostack);
				stilo_dup(stilo, el);

				stilo = stilo_stack_push(estack);
				stilo_dup(stilo, proc);
				stilo_thread_loop(a_thread);
			}
			break;
		}
		case STILOT_DICT: {
			cw_stilo_t	*key, *val;

			/* Move proc and dict to tstack. */
			stilo = stilo_stack_push(tstack);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stilo_stack_push(tstack);
			stilo_dup(stilo, what);
			what = stilo;

			stilo_stack_npop(ostack, 2);

			for (i = 0, count = stilo_dict_count(what); i <
				 count; i++) {
				/* Push key and val onto ostack. */
				key = stilo_stack_push(ostack);
				val = stilo_stack_push(ostack);

				/* Get next key. */
				stilo_dict_iterate(what, key);

				/* Use key to get val. */
				stilo_dict_lookup(what, key, val);

				/* Push proc onto estack and execute it. */
				stilo = stilo_stack_push(estack);
				stilo_dup(stilo, proc);
				stilo_thread_loop(a_thread);
			}
			break;
		}
		case STILOT_STACK: {
			cw_stilo_t	*el;

			/* Move proc and stack to tstack. */
			stilo = stilo_stack_push(tstack);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stilo_stack_push(tstack);
			stilo_dup(stilo, what);
			what = stilo;

			stilo_stack_npop(ostack, 2);

			/*
			 * Iterate through the stack, push each element onto
			 * ostack, and execute proc.
			 */
			for (i = 0, count = stilo_stack_count(what), el = NULL;
			    i < count; i++) {
				el = stilo_stack_down_get(what, el);

				stilo = stilo_stack_push(ostack);
				stilo_dup(stilo, el);

				stilo = stilo_stack_push(estack);
				stilo_dup(stilo, proc);
				stilo_thread_loop(a_thread);
			}
			break;
		}
		case STILOT_STRING: {
			cw_uint8_t	el;

			/* Move proc and array to tstack. */
			stilo = stilo_stack_push(tstack);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stilo_stack_push(tstack);
			stilo_dup(stilo, what);
			what = stilo;

			stilo_stack_npop(ostack, 2);

			/*
			 * Iterate through the string, push each element onto
			 * ostack, and execute proc.
			 */
			for (i = 0, count = stilo_array_len_get(what); i <
			    count; i++) {
				stilo_string_el_get(what, i, &el);
				stilo = stilo_stack_push(ostack);
				stilo_integer_new(stilo, (cw_stiloi_t)el);

				stilo = stilo_stack_push(estack);
				stilo_dup(stilo, proc);
				stilo_thread_loop(a_thread);
			}
			break;
		}
		default:
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up tstack. */
		stilo_stack_npop(tstack, stilo_stack_count(tstack) -
		    tdepth);
	}
	xep_end();

	stilo_stack_npop(tstack, stilo_stack_count(tstack) - tdepth);
}

void
systemdict_fork(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	pid_t		pid;

	pid = fork();
	if (pid == -1) {
		/* Error, related to some form of resource exhaustion. */
		stilo_thread_error(a_thread, STILO_THREADE_LIMITCHECK);
		return;
	}

	ostack = stilo_thread_ostack_get(a_thread);
	stilo = stilo_stack_push(ostack);
	stilo_integer_new(stilo, pid);
}

void
systemdict_ge(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_stilot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	ge;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

	type_a = stilo_type_get(stilo_a);
	type_b = stilo_type_get(stilo_b);
	if (type_a != type_b || (type_a != STILOT_INTEGER && type_a !=
	    STILOT_STRING) || (type_b != STILOT_INTEGER && type_b !=
	    STILOT_STRING)) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	result = stilo_compare(stilo_a, stilo_b);
	if (result >= 0)
		ge = TRUE;
	else
		ge = FALSE;

	stilo_boolean_new(stilo_a, ge);

	stilo_stack_pop(ostack);
}

void
systemdict_get(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*from, *with;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(with, ostack, a_thread);
	STILO_STACK_DOWN_GET(from, ostack, a_thread, with);

	switch (stilo_type_get(from)) {
	case STILOT_ARRAY: {
		cw_stiloi_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);

		if (index >= stilo_array_len_get(from)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}
		stilo_array_el_get(from, index, with);

		stilo_stack_roll(ostack, 2, 1);
		stilo_stack_pop(ostack);
		break;
	}
	case STILOT_DICT: {
		cw_stilo_t	*val;

		val = stilo_stack_push(ostack);
		if (stilo_dict_lookup(from, with, val)) {
			stilo_stack_pop(ostack);
			stilo_thread_error(a_thread, STILO_THREADE_UNDEFINED);
			return;
		}
		stilo_stack_roll(ostack, 3, 1);
		stilo_stack_npop(ostack, 2);
		break;
	}
	case STILOT_STRING: {
		cw_stiloi_t	index;
		cw_uint8_t	el;

		if (stilo_type_get(with) != STILOT_INTEGER) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);

		if (index < 0 || index >= stilo_string_len_get(from)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}
		stilo_string_el_get(from, index, &el);
		stilo_integer_set(with, (cw_stiloi_t)el);

		stilo_stack_roll(ostack, 2, 1);
		stilo_stack_pop(ostack);
		break;
	}
	default:
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
}

void
systemdict_getinterval(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*from, *with, *count;
	cw_stiloi_t	index, len;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(count, ostack, a_thread);
	STILO_STACK_DOWN_GET(with, ostack, a_thread, count);
	STILO_STACK_DOWN_GET(from, ostack, a_thread, with);

	if ((stilo_type_get(with) != STILOT_INTEGER) || (stilo_type_get(count)
	    != STILOT_INTEGER)) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	index = stilo_integer_get(with);
	len = stilo_integer_get(count);

	switch (stilo_type_get(from)) {
	case STILOT_ARRAY:
		if (index + len > stilo_array_len_get(from)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}
		stilo_array_subarray_new(count, from,
		    stilo_thread_stil_get(a_thread), index, len);
		break;
	case STILOT_STRING:
		if (index + len > stilo_string_len_get(from)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}
		stilo_string_substring_new(count, from,
		    stilo_thread_stil_get(a_thread), index, len);
		break;
	default:
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_stack_roll(ostack, 3, 1);
	stilo_stack_npop(ostack, 2);
}

void
systemdict_gt(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_stilot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	gt;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

	type_a = stilo_type_get(stilo_a);
	type_b = stilo_type_get(stilo_b);
	if (type_a != type_b || (type_a != STILOT_INTEGER && type_a !=
	    STILOT_STRING) || (type_b != STILOT_INTEGER && type_b !=
	    STILOT_STRING)) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	result = stilo_compare(stilo_a, stilo_b);
	if (result == 1)
		gt = TRUE;
	else
		gt = FALSE;

	stilo_boolean_new(stilo_a, gt);

	stilo_stack_pop(ostack);
}

void
systemdict_handleerror(cw_stilo_t *a_thread)
{
	cw_stilo_t	*estack, *tstack;
	cw_stilo_t	*key, *errordict, *handleerror;

	estack = stilo_thread_estack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	/* Get errordict. */
	errordict = stilo_stack_push(tstack);
	key = stilo_stack_push(tstack);
	stilo_name_new(key, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_errordict), stiln_len(STILN_errordict), TRUE);
	if (stilo_thread_dstack_search(a_thread, key, errordict)) {
		/*
		 * Fall back to the errordict defined during thread creation,
		 * since the alternative is to blow up (or potentially go
		 * infinitely recursive).
		 */
		stilo_dup(errordict, stilo_thread_errordict_get(a_thread));
	}

	/* Get handleerror from errordict and push it onto estack. */
	handleerror = stilo_stack_push(estack);
	stilo_name_new(key, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_handleerror), stiln_len(STILN_handleerror), TRUE);
	if (stilo_dict_lookup(errordict, key, handleerror)) {
		/*
		 * Do not execute an error handler, since the alternative is to
		 * blow up (or potentially go infinitely recursive).
		 */
		stilo_stack_pop(estack);
		stilo_stack_npop(tstack, 2);
		return;
	}
	stilo_stack_npop(tstack, 2);

	/* Execute handleerror. */
	stilo_thread_loop(a_thread);
}

void
systemdict_if(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*cond, *exec;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(exec, ostack, a_thread);
	STILO_STACK_DOWN_GET(cond, ostack, a_thread, exec);
	if (stilo_type_get(cond) != STILOT_BOOLEAN) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	if (stilo_boolean_get(cond)) {
		cw_stilo_t	*estack;
		cw_stilo_t	*stilo;

		estack = stilo_thread_estack_get(a_thread);
		stilo = stilo_stack_push(estack);
		stilo_dup(stilo, exec);
		stilo_stack_npop(ostack, 2);
		stilo_thread_loop(a_thread);
	} else
		stilo_stack_npop(ostack, 2);
}

void
systemdict_ifelse(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack;
	cw_stilo_t	*cond, *exec_if, *exec_else, *stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);

	STILO_STACK_GET(exec_else, ostack, a_thread);

	STILO_STACK_DOWN_GET(exec_if, ostack, a_thread, exec_else);

	STILO_STACK_DOWN_GET(cond, ostack, a_thread, exec_if);
	if (stilo_type_get(cond) != STILOT_BOOLEAN) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo = stilo_stack_push(estack);
	if (stilo_boolean_get(cond))
		stilo_dup(stilo, exec_if);
	else
		stilo_dup(stilo, exec_else);
	stilo_stack_npop(ostack, 3);
	stilo_thread_loop(a_thread);
}

void
systemdict_index(cw_stilo_t *a_thread)
{
	systemdict_inline_index(a_thread);
}

void
systemdict_join(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*thread;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(thread, ostack, a_thread);
	if (stilo_type_get(thread) != STILOT_THREAD) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_thread_join(thread);

	stilo_stack_pop(ostack);
}

void
systemdict_known(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*dict, *key;
	cw_bool_t	known;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(key, ostack, a_thread);
	STILO_STACK_DOWN_GET(dict, ostack, a_thread, key);
	if (stilo_type_get(dict) != STILOT_DICT) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	known = !stilo_dict_lookup(dict, key, NULL);
	stilo_boolean_new(dict, known);

	stilo_stack_pop(ostack);
}

void
systemdict_lcheck(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	cw_bool_t	locking;
	
	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);

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
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
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
systemdict_le(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_stilot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	le;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

	type_a = stilo_type_get(stilo_a);
	type_b = stilo_type_get(stilo_b);
	if (type_a != type_b || (type_a != STILOT_INTEGER && type_a !=
	    STILOT_STRING) || (type_b != STILOT_INTEGER && type_b !=
	    STILOT_STRING)) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	result = stilo_compare(stilo_a, stilo_b);
	if (result <= 0)
		le = TRUE;
	else
		le = FALSE;

	stilo_boolean_new(stilo_a, le);

	stilo_stack_pop(ostack);
}

void
systemdict_length(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stiloi_t	len;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(stilo, ostack, a_thread);
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
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_integer_new(stilo, len);
}

void
systemdict_link(cw_stilo_t *a_thread)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_load(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack;
	cw_stilo_t	*key, *val;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	STILO_STACK_GET(key, ostack, a_thread);
	val = stilo_stack_push(tstack);

	if (stilo_thread_dstack_search(a_thread, key, val)) {
		stilo_thread_error(a_thread, STILO_THREADE_UNDEFINED);
		return;
	}
	stilo_dup(key, val);
	stilo_stack_pop(tstack);
}

void
systemdict_lock(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*mutex;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(mutex, ostack, a_thread);
	if (stilo_type_get(mutex) != STILOT_MUTEX) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_mutex_lock(mutex);

	stilo_stack_pop(ostack);
}

void
systemdict_loop(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack, *tstack;
	cw_stilo_t	*exec, *stilo, *tstilo;
	cw_uint32_t	sdepth, tdepth;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	STILO_STACK_GET(exec, ostack, a_thread);

	/* Move the object to be executed to tstack. */
	tstilo = stilo_stack_push(tstack);
	stilo_dup(tstilo, exec);
	stilo_stack_pop(ostack);

	sdepth = stilo_stack_count(estack);
	tdepth = stilo_stack_count(tstack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		for (;;) {
			stilo = stilo_stack_push(estack);
			stilo_dup(stilo, tstilo);
			stilo_thread_loop(a_thread);
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		cw_uint32_t	i;

		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		for (i = stilo_stack_count(estack); i > sdepth + 1; i--)
			stilo_stack_pop(estack);

		/* Clean up tstack. */
		stilo_stack_npop(tstack, stilo_stack_count(tstack) - tdepth);
	}
	xep_end();

	stilo_stack_pop(estack);
	stilo_stack_pop(tstack);
}

void
systemdict_lt(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_stilot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	lt;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

	type_a = stilo_type_get(stilo_a);
	type_b = stilo_type_get(stilo_b);
	if (type_a != type_b || (type_a != STILOT_INTEGER && type_a !=
	    STILOT_STRING) || (type_b != STILOT_INTEGER && type_b !=
	    STILOT_STRING)) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	result = stilo_compare(stilo_a, stilo_b);
	if (result == -1)
		lt = TRUE;
	else
		lt = FALSE;

	stilo_boolean_new(stilo_a, lt);

	stilo_stack_pop(ostack);
}

void
systemdict_mark(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	stilo = stilo_stack_push(ostack);
	stilo_mark_new(stilo);
}

void
systemdict_mkdir(cw_stilo_t *a_thread)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_mod(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(b, ostack, a_thread);
	STILO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(b) == 0) {
		stilo_thread_error(a_thread, STILO_THREADE_UNDEFINEDRESULT);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) % stilo_integer_get(b));
	stilo_stack_pop(ostack);
}

void
systemdict_mul(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(b, ostack, a_thread);
	STILO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) * stilo_integer_get(b));
	stilo_stack_pop(ostack);
}

void
systemdict_mutex(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*mutex;

	ostack = stilo_thread_ostack_get(a_thread);
	mutex = stilo_stack_push(ostack);
	stilo_mutex_new(mutex, stilo_thread_stil_get(a_thread));
}

void
systemdict_ne(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;
	cw_sint32_t	result;
	cw_bool_t	ne;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

	result = stilo_compare(stilo_a, stilo_b);
	if (result == 0)
		ne = FALSE;
	else
		ne = TRUE;

	stilo_boolean_new(stilo_a, ne);

	stilo_stack_pop(ostack);
}

void
systemdict_neg(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*a;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(a, ostack, a_thread);
	if (stilo_type_get(a) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_integer_set(a, -stilo_integer_get(a));
}

void
systemdict_not(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo, ostack, a_thread);

	if (stilo_type_get(stilo) == STILOT_BOOLEAN)
		stilo_boolean_set(stilo, !stilo_boolean_get(stilo));
	else if (stilo_type_get(stilo) == STILOT_INTEGER)
		stilo_integer_set(stilo, ~stilo_integer_get(stilo));
	else
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
}

void
systemdict_nsleep(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	int		error;
	struct timespec sleeptime, remainder;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo, ostack, a_thread);

	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(stilo) <= 0) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}

	sleeptime.tv_sec = stilo_integer_get(stilo) / 1000000000LL;
	sleeptime.tv_nsec = stilo_integer_get(stilo) % 1000000000LL;

	for (;;) {
		error = nanosleep(&sleeptime, &remainder);
		if (error == 0) {
			/* We've slept the entire time. */
			break;
		}
		/* A signal interrupted us.  Sleep some more. */
		memcpy(&sleeptime, &remainder, sizeof(struct timespec));
	}
	stilo_stack_pop(ostack);
}

void
systemdict_open(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *tstack;
	cw_stilo_t		*name, *flags, *file;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	
	STILO_STACK_GET(flags, ostack, a_thread);
	STILO_STACK_DOWN_GET(name, ostack, a_thread, flags);
	if (stilo_type_get(name) != STILOT_STRING || stilo_type_get(flags) !=
	    STILOT_STRING) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	file = stilo_stack_push(tstack);
	stilo_file_new(file, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread));
	stilo_string_lock(name);
	error = stilo_file_open(file, stilo_string_get(name),
	    stilo_string_len_get(name), stilo_string_get(flags),
	    stilo_string_len_get(flags));
	stilo_string_unlock(name);
	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_file_buffer_size_set(file, _LIBSTIL_FILE_BUFFER_SIZE);

	stilo_stack_pop(ostack);
	stilo_dup(name, file);
	stilo_stack_pop(tstack);
}

void
systemdict_or(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

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
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_stack_pop(ostack);
}

void
systemdict_ostack(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *stack;

	ostack = stilo_thread_ostack_get(a_thread);
	stack = stilo_stack_push(ostack);
	stilo_stack_new(stack, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread));
	stilo_stack_copy(stack, ostack);

	/*
	 * Pop the top element off the stack, since it's a reference to the
	 * stack itself.
	 */
	stilo_stack_pop(stack);
}

void
systemdict_pop(cw_stilo_t *a_thread)
{
	systemdict_inline_pop(a_thread);
}

void
systemdict_print(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack;
	cw_stilo_t		*stilo, *stdout_stilo;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	stdout_stilo = stil_stdout_get(stilo_thread_stil_get(a_thread));

	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_STRING) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_string_lock(stilo);
	error = stilo_file_write(stdout_stilo, stilo_string_get(stilo),
	    stilo_string_len_get(stilo));
	stilo_string_unlock(stilo);
	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_pop(ostack);
}

void
systemdict_put(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*into, *with, *what;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(what, ostack, a_thread);
	STILO_STACK_DOWN_GET(with, ostack, a_thread, what);
	STILO_STACK_DOWN_GET(into, ostack, a_thread, with);

	switch (stilo_type_get(into)) {
	case STILOT_ARRAY: {
		cw_stiloi_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);

		if (index >= stilo_array_len_get(into)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}
		stilo_array_el_set(into, what, index);
		break;
	}
	case STILOT_DICT: {
		stilo_dict_def(into, stilo_thread_stil_get(a_thread), with,
		    what);
		break;
	}
	case STILOT_STRING: {
		cw_stiloi_t	index;
		cw_uint8_t	val;

		if ((stilo_type_get(with) != STILOT_INTEGER) ||
		    stilo_type_get(what) != STILOT_INTEGER) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}
		index = stilo_integer_get(with);
		val = stilo_integer_get(what);

		if (index >= stilo_string_len_get(into)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			return;
		}
		stilo_string_el_set(into, val, index);
		break;
	}
	default:
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stilo_stack_npop(ostack, 3);
}

void
systemdict_putinterval(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*into, *with, *what;
	cw_stiloi_t	index;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(what, ostack, a_thread);
	STILO_STACK_DOWN_GET(with, ostack, a_thread, what);
	STILO_STACK_DOWN_GET(into, ostack, a_thread, with);

	if (stilo_type_get(with) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	index = stilo_integer_get(with);

	switch (stilo_type_get(into)) {
	case STILOT_ARRAY: {
		cw_stilo_t	*tstack;
		cw_stilo_t	*el;
		cw_uint32_t	i, len;

		tstack = stilo_thread_tstack_get(a_thread);
		el = stilo_stack_push(tstack);
		len = stilo_array_len_get(what);
		if (index + len > stilo_array_len_get(into)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
			stilo_stack_pop(tstack);
			return;
		}
		for (i = 0; i < len; i++) {
			stilo_array_el_get(what, i, el);
			stilo_array_el_set(into, el, i + index);
		}
		stilo_stack_pop(tstack);
		break;
	}
	case STILOT_STRING: {
		cw_uint8_t	*str;
		cw_uint32_t	len;

		str = stilo_string_get(what);
		len = stilo_string_len_get(what);
		if (index + len > stilo_string_len_get(into)) {
			stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
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
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stilo_stack_npop(ostack, 3);
}

void
systemdict_pwd(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stilo;
	char		*str;

	str = getcwd(NULL, 0);
	if (str == NULL) {
		stilo_thread_error(a_thread, STILO_THREADE_INVALIDACCESS);
		return;
	}

	ostack = stilo_thread_ostack_get(a_thread);
	stilo = stilo_stack_push(ostack);

	stilo_string_new(stilo, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), strlen(str));
	stilo_string_lock(stilo);
	stilo_string_set(stilo, 0, str, stilo_string_len_get(stilo));
	stilo_string_unlock(stilo);

	free(str);
}

void
systemdict_quit(cw_stilo_t *a_thread)
{
	xep_throw(_CW_STILX_QUIT);
}

void
systemdict_rand(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*num;

	ostack = stilo_thread_ostack_get(a_thread);

	num = stilo_stack_push(ostack);
	stilo_integer_new(num, random());
}

void
systemdict_read(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*file;
	cw_uint8_t	val;
	cw_sint32_t	nread;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(file, ostack, a_thread);
	switch (stilo_type_get(file)) {
	case STILOT_FILE: {
		cw_stilo_t	*value, *code;

		/* Character read. */
		value = stilo_stack_push(ostack);
		code = stilo_stack_push(ostack);
		
		nread = stilo_file_read(file, 1, &val);
		if (nread == -1) {
			stilo_stack_npop(ostack, 2);
			stilo_thread_error(a_thread, STILO_THREADE_IOERROR);
			return;
		}

		if (nread == 0) {
			stilo_integer_new(value, 0);
			stilo_boolean_new(code, FALSE);
		} else {
			stilo_integer_new(value, (cw_stiloi_t)val);
			stilo_boolean_new(code, TRUE);
		}

		stilo_stack_roll(ostack, 3, 2);
		stilo_stack_pop(ostack);
		break;
	}
	case STILOT_STRING: {
		cw_stilo_t	*string;

		/* String read. */
		string = file;
		STILO_STACK_DOWN_GET(file, ostack, a_thread, string);
		if (stilo_type_get(file) != STILOT_FILE) {
			stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
			return;
		}

		stilo_string_lock(string);
		nread = stilo_file_read(file, stilo_string_len_get(string),
		    stilo_string_get(string));
		stilo_string_unlock(string);
		if (nread == -1) {
			stilo_thread_error(a_thread, STILO_THREADE_IOERROR);
			return;
		}

		if (nread == 0) {
			/* EOF. */
			stilo_boolean_new(file, TRUE);
			stilo_string_new(string,
			    stilo_thread_stil_get(a_thread),
			    stilo_thread_currentlocking(a_thread), 0);
			stilo_stack_roll(ostack, 2, 1);
		} else if (nread < stilo_string_len_get(string)) {
			cw_stilo_t	*value, *code;

			/*
			 * We didn't fill the string, so we can't just use it as
			 * the result.  Create a copy.
			 */
			value = stilo_stack_under_push(ostack, file);
			stilo_string_substring_new(value, string,
			    stilo_thread_stil_get(a_thread), 0, nread);
			code = stilo_stack_under_push(ostack, file);
			stilo_boolean_new(code, FALSE);

			stilo_stack_npop(ostack, 2);
		} else {
			/*
			 * The string is full, so doesn't need modified.
			 */
			stilo_boolean_new(file, FALSE);
			stilo_stack_roll(ostack, 2, 1);
		}
		break;
	}
	default:
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
}

void
systemdict_readline(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *tstack;
	cw_stilo_t		*stilo, *tfile;
	cw_stilo_threade_t	error;
	cw_bool_t		eof;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_FILE) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	tfile = stilo_stack_push(tstack);
	stilo_dup(tfile, stilo);
	error = stilo_file_readline(tfile, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), stilo, &eof);
	if (error) {
		stilo_stack_pop(tstack);
		stilo_thread_error(a_thread, error);
		return;
	}
	stilo_stack_pop(tstack);

	stilo = stilo_stack_push(ostack);
	stilo_boolean_new(stilo, eof);
}

void
systemdict_realtime(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	struct timeval	tv;

	ostack = stilo_thread_ostack_get(a_thread);
	stilo = stilo_stack_push(ostack);

	gettimeofday(&tv, NULL);
	stilo_integer_new(stilo, (((cw_stiloi_t)tv.tv_sec *
	    (cw_stiloi_t)1000000000) + (cw_stiloi_t)tv.tv_usec *
	    (cw_stiloi_t)1000));
}

void
systemdict_renamefile(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*string_from, *string_to;
	cw_uint8_t	str_from[PATH_MAX], str_to[1024];
	cw_uint32_t	nbytes;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(string_to, ostack, a_thread);
	STILO_STACK_DOWN_GET(string_from, ostack, a_thread, string_to);

	if (stilo_type_get(string_from) != STILOT_STRING ||
	    stilo_type_get(string_to) != STILOT_STRING) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	if (stilo_string_len_get(string_from) >= sizeof(str_from)) {
		stilo_thread_error(a_thread, STILO_THREADE_LIMITCHECK);
		return;
	}
	nbytes = stilo_string_len_get(string_from);
	stilo_string_lock(string_from);
	memcpy(str_from, stilo_string_get(string_from), nbytes);
	stilo_string_unlock(string_from);
	str_from[nbytes] = '\0';

	if (stilo_string_len_get(string_to) >= sizeof(str_to)) {
		stilo_thread_error(a_thread, STILO_THREADE_LIMITCHECK);
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
			stilo_thread_error(a_thread,
			    STILO_THREADE_INVALIDFILEACCESS);
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			stilo_thread_error(a_thread,
			    STILO_THREADE_UNDEFINEDFILENAME);
		default:
			stilo_thread_error(a_thread, STILO_THREADE_IOERROR);
		}
		return;
	}

	stilo_stack_npop(ostack, 2);
}

void
systemdict_repeat(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack, *tstack;
	cw_stilo_t	*count, *exec, *stilo, *tstilo;
	cw_stiloi_t	i, cnt;
	cw_uint32_t	sdepth, tdepth;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	STILO_STACK_GET(exec, ostack, a_thread);
	STILO_STACK_DOWN_GET(count, ostack, a_thread, exec);
	if (stilo_type_get(count) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	tstilo = stilo_stack_push(tstack);
	stilo_dup(tstilo, exec);

	cnt = stilo_integer_get(count);
	stilo_stack_npop(ostack, 2);

	sdepth = stilo_stack_count(estack);
	tdepth = stilo_stack_count(tstack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		for (i = 0; i < cnt; i++) {
			stilo = stilo_stack_push(estack);
			stilo_dup(stilo, tstilo);
			stilo_thread_loop(a_thread);
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		for (i = stilo_stack_count(estack); i > sdepth; i--)
			stilo_stack_pop(estack);

		/* Clean up tstack. */
		stilo_stack_npop(tstack, stilo_stack_count(tstack) - tdepth);
	}
	xep_end();

	stilo_stack_pop(tstack);
}

void
systemdict_roll(cw_stilo_t *a_thread)
{
	systemdict_inline_roll(a_thread);
}

void
systemdict_run(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *estack, *tstack;
	cw_stilo_t		*stilo, *tfile;
	cw_stilo_threade_t	error;
	cw_uint32_t		sdepth, tdepth;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_STRING) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	tfile = stilo_stack_push(tstack);
	stilo_file_new(tfile, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread));
	stilo_string_lock(stilo);
	error = stilo_file_open(tfile, stilo_string_get(stilo),
	    stilo_string_len_get(stilo), "r", 1);
	stilo_string_unlock(stilo);
	if (error) {
		stilo_stack_pop(tstack);
		stilo_thread_error(a_thread, error);
		return;
	}
	stilo_stack_pop(ostack);
	stilo_attrs_set(tfile, STILOA_EXECUTABLE);
	stilo = stilo_stack_push(estack);

	sdepth = stilo_stack_count(estack);
	tdepth = stilo_stack_count(tstack);

	stilo_dup(stilo, tfile);

	xep_begin();
	xep_try {
		stilo_thread_loop(a_thread);
	}
	xep_catch(_CW_STILX_EXIT) {
		stilo_thread_error(a_thread, STILO_THREADE_INVALIDEXIT);
		/*
		 * Pop the exit operator off estack to avoid an infinite loop.
		 */
		stilo_stack_pop(estack);
		xep_retry();
	}
	xep_catch(_CW_STILX_STOP)
	xep_mcatch(_CW_STILX_QUIT) {
		cw_uint32_t	i;

		/* Close the file, but don't handle the exception. */
		error = stilo_file_close(tfile);
		if (error)
			stilo_thread_error(a_thread, error);

		/* Clean up estack. */
		for (i = stilo_stack_count(estack); i > sdepth; i--)
			stilo_stack_pop(estack);

		/* Clean up tstack. */
		stilo_stack_npop(tstack, stilo_stack_count(tstack) - tdepth);
	}
	xep_end();

	stilo_stack_pop(tstack);
}

void
systemdict_sclear(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stack;
	cw_uint32_t	count;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stack, ostack, a_thread);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	count = stilo_stack_count(stack);
	if (count > 0)
		stilo_stack_npop(stack, count);
}

void
systemdict_scleartomark(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stack, *stilo;
	cw_uint32_t	i, depth;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stack, ostack, a_thread);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	for (i = 0, depth = stilo_stack_count(stack), stilo = NULL; i < depth;
	     i++) {
		stilo = stilo_stack_down_get(stack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilo_thread_error(a_thread, STILO_THREADE_UNMATCHEDMARK);
		return;
	}

	stilo_stack_npop(stack, i + 1);
}

void
systemdict_scount(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stack, *stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stack, ostack, a_thread);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo = stilo_stack_push(ostack);
	stilo_integer_new(stilo, stilo_stack_count(stack));
}

void
systemdict_scounttomark(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stack, *stilo;
	cw_uint32_t	i, depth;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stack, ostack, a_thread);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	for (i = 0, depth = stilo_stack_count(stack), stilo = NULL; i < depth;
	    i++) {
		stilo = stilo_stack_down_get(stack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilo_thread_error(a_thread, STILO_THREADE_UNMATCHEDMARK);
		return;
	}

	stilo = stilo_stack_push(ostack);
	stilo_integer_new(stilo, i);
}

void
systemdict_sdup(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stack, *orig, *dup;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stack, ostack, a_thread);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	STILO_STACK_GET(orig, stack, a_thread);
	dup = stilo_stack_push(stack);
	stilo_dup(dup, orig);
}

void
systemdict_self(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*thread;

	ostack = stilo_thread_ostack_get(a_thread);
	thread = stilo_stack_push(ostack);

	stilo_thread_self(a_thread, thread);
}

void
systemdict_seek(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack;
	cw_stilo_t		*file, *position;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(position, ostack, a_thread);
	STILO_STACK_DOWN_GET(file, ostack, a_thread, position);
	
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(position) !=
	    STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	error = stilo_file_position_set(file, stilo_integer_get(position));
	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_npop(ostack, 2);
}

void
systemdict_setlocking(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_BOOLEAN) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stilo_thread_setlocking(a_thread, stilo_boolean_get(stilo));
	stilo_stack_pop(ostack);
}

void
systemdict_sexch(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stack;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stack, ostack, a_thread);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	if (stilo_stack_count(stack) < 2) {
		stilo_thread_error(a_thread, STILO_THREADE_STACKUNDERFLOW);
		return;
	}

	stilo_stack_roll(stack, 2, 1);
}

void
systemdict_shift(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*integer, *shift;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(shift, ostack, a_thread);
	STILO_STACK_DOWN_GET(integer, ostack, a_thread, shift);

	if (stilo_type_get(integer) != STILOT_INTEGER || stilo_type_get(shift)
	    != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	if (stilo_integer_get(shift) > 0) {
		stilo_integer_set(integer, stilo_integer_get(integer) <<
		    stilo_integer_get(shift));
	} else if (stilo_integer_get(shift) < 0) {
		stilo_integer_set(integer, stilo_integer_get(integer) >>
		    -stilo_integer_get(shift));
	}

	stilo_stack_pop(ostack);
}

void
systemdict_signal(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*condition;
	
	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(condition, ostack, a_thread);
	if (stilo_type_get(condition) != STILOT_CONDITION) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_condition_signal(condition);

	stilo_stack_pop(ostack);
}

void
systemdict_sindex(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stilo, *stack, *orig;
	cw_stiloi_t	index;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	STILO_STACK_DOWN_GET(stack, ostack, a_thread, stilo);
	if (stilo_type_get(stilo) != STILOT_INTEGER || stilo_type_get(stack) !=
	    STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	index = stilo_integer_get(stilo);
	if (index < 0) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}

	STILO_STACK_NGET(orig, stack, a_thread, index);
	stilo = stilo_stack_push(stack);
	stilo_dup(stilo, orig);

	stilo_stack_pop(ostack);
}

void
systemdict_spop(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stack;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stack, ostack, a_thread);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	STILO_STACK_POP(stack, a_thread);
}

void
systemdict_sprint(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack;
	cw_stilo_t		*stilo, *depth, *stdout_stilo;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	stdout_stilo = stil_stdout_get(stilo_thread_stil_get(a_thread));

	/*
	 * depth and stilo aren't used, but accessing them causes a more
	 * intelligible error than the embedded stil code would.
	 */
	STILO_STACK_GET(depth, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo, ostack, a_thread, depth);
	if (stilo_type_get(depth) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	/* object depth sprint - */
	_cw_stil_code(a_thread, "1 index type sprintdict exch get eval");
	error = stilo_file_output(stdout_stilo, "\n");
	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}
	error = stilo_file_buffer_flush(stdout_stilo);
	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}
}

void
systemdict_spush(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stilo, *stack, *nstilo;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	STILO_STACK_DOWN_GET(stack, ostack, a_thread, stilo);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	nstilo = stilo_stack_push(stack);
	stilo_dup(nstilo, stilo);
	stilo_stack_pop(ostack);
}

void
systemdict_srand(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*seed;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(seed, ostack, a_thread);
	srandom(stilo_integer_get(seed));
	stilo_stack_pop(ostack);
}

void
systemdict_sroll(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *stack, *stilo;
	cw_stiloi_t	count, amount;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	amount = stilo_integer_get(stilo);
	STILO_STACK_DOWN_GET(stilo, ostack, a_thread, stilo);
	STILO_STACK_DOWN_GET(stack, ostack, a_thread, stilo);
	if (stilo_type_get(stilo) != STILOT_INTEGER || stilo_type_get(stack) !=
	    STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	count = stilo_integer_get(stilo);
	if (count < 1) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}
	if (count > stilo_stack_count(stack)) {
		stilo_thread_error(a_thread, STILO_THREADE_STACKUNDERFLOW);
		return;
	}

	stilo_stack_npop(ostack, 2);
	stilo_stack_roll(stack, count, amount);
}

void
systemdict_stack(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *nstack;

	ostack = stilo_thread_ostack_get(a_thread);
	nstack = stilo_stack_push(ostack);
	stilo_stack_new(nstack, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread));
}

void
systemdict_start(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack, *tstack;
	cw_stilo_t	*ostilo, *estilo;
	cw_uint32_t	edepth, tdepth;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	edepth = stilo_stack_count(estack);
	tdepth = stilo_stack_count(tstack);

	STILO_STACK_GET(ostilo, ostack, a_thread);
	estilo = stilo_stack_push(estack);
	stilo_dup(estilo, ostilo);
	stilo_stack_pop(ostack);

	xep_begin();
	xep_try {
		stilo_thread_loop(a_thread);
	}
	xep_catch(_CW_STILX_EXIT)
	xep_mcatch(_CW_STILX_QUIT)
	xep_mcatch(_CW_STILX_STOP) {
		xep_handled();
	}
	xep_end();

	/*
	 * Pop all objects off estack and tstack that weren't there
	 * before entering this function.
	 */
	stilo_stack_npop(estack, stilo_stack_count(estack) - edepth);
	stilo_stack_npop(tstack, stilo_stack_count(tstack) - tdepth);
}

void
systemdict_stat(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_FILE) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stilo_boolean_new(stilo, stilo_file_status(stilo));
}

void
systemdict_stop(cw_stilo_t *a_thread)
{
	xep_throw(_CW_STILX_STOP);
}

void
systemdict_stopped(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *estack, *tstack;
	cw_stilo_t	*exec, *stilo;
	cw_bool_t	result = FALSE;
	cw_uint32_t	tdepth;

	ostack = stilo_thread_ostack_get(a_thread);
	estack = stilo_thread_estack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	
	STILO_STACK_GET(exec, ostack, a_thread);
	stilo = stilo_stack_push(estack);
	stilo_dup(stilo, exec);
	stilo_stack_pop(ostack);

	tdepth = stilo_stack_count(tstack);

	/*
	 * Point exec to the copy on the execution stack, so that it can be used
	 * as a marker for execution stack cleanup if the stop operator is
	 * called.
	 */
	exec = stilo;

	/* Catch a stop exception, if thrown. */
	xep_begin();
	xep_try {
		stilo_thread_loop(a_thread);
	}
	xep_catch(_CW_STILX_STOP) {
		xep_handled();
		result = TRUE;

		/* Clean up whatever mess was left on the execution stack. */
		do {
			stilo = stilo_stack_get(estack);
			stilo_stack_pop(estack);
		} while (stilo != exec);

		/* Clean up tstack. */
		stilo_stack_npop(tstack, stilo_stack_count(tstack) - tdepth);
	}
	xep_end();

	stilo = stilo_stack_push(ostack);
	stilo_boolean_new(stilo, result);
}

void
systemdict_store(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *dstack;
	cw_stilo_t	*dict, *key, *val;
	cw_uint32_t	i, depth;

	ostack = stilo_thread_ostack_get(a_thread);
	dstack = stilo_thread_dstack_get(a_thread);

	STILO_STACK_GET(val, ostack, a_thread);
	STILO_STACK_DOWN_GET(key, ostack, a_thread, val);

	/*
	 * Iteratively search the dictionaries on the dictionary stack for key,
	 * and replace its value with val.
	 */
	for (i = 0, depth = stilo_stack_count(dstack), dict = NULL; i < depth;
	     i++) {
		dict = stilo_stack_down_get(dstack, dict);
		if (stilo_dict_lookup(dict, key, NULL) == FALSE) {
			/* Found. */
			stilo_dict_def(dict, stilo_thread_stil_get(a_thread),
			    key, val);
			return;
		}
	}
	/* Not found.  Create a new entry in currentdict. */
	dict = stilo_stack_get(dstack);
	stilo_dict_def(dict, stilo_thread_stil_get(a_thread), key, val);

	stilo_stack_npop(ostack, 2);
}

void
systemdict_string(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stiloi_t	len;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	len = stilo_integer_get(stilo);
	if (len < 0) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}

	stilo_string_new(stilo, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), len);
}

void
systemdict_sub(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilo_thread_ostack_get(a_thread);
	
	STILO_STACK_GET(b, ostack, a_thread);
	STILO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_integer_set(a, stilo_integer_get(a) - stilo_integer_get(b));
	stilo_stack_pop(ostack);
}

/* ) */
void
systemdict_sym_rp(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack;
	cw_stilo_t	*nstack, *tstilo, *stilo;
	cw_sint32_t	nelements, i, depth;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	/* Find the fino. */
	for (i = 0, depth = stilo_stack_count(ostack), stilo = NULL; i < depth;
	     i++) {
		stilo = stilo_stack_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_FINO)
			break;
	}
	if (i == depth) {
		stilo_thread_error(a_thread, STILO_THREADE_UNMATCHEDFINO);
		return;
	}

	/*
	 * i is the index of the fino, and stilo points to the fino.  Set
	 * nelements accordingly.
	 */
	nelements = i;

	nstack = stilo_stack_push(tstack);
	stilo_stack_new(nstack, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread));

	/*
	 * Push objects onto tstack and pop them off ostack.
	 */
	for (i = 0; i < nelements; i++) {
		stilo = stilo_stack_get(ostack);
		tstilo = stilo_stack_push(tstack);
		stilo_dup(tstilo, stilo);
		stilo_stack_pop(ostack);
	}

	/* Pop the fino off ostack. */
	stilo_stack_pop(ostack);

	/*
	 * Push objects onto nstack and pop them off tstack.
	 */
	for (i = 0; i < nelements; i++) {
		stilo = stilo_stack_get(tstack);
		tstilo = stilo_stack_push(nstack);
		stilo_dup(tstilo, stilo);
		stilo_stack_pop(tstack);
	}

	/* Push nstack onto ostack and pop it off of tstack. */
	stilo = stilo_stack_push(ostack);
	stilo_dup(stilo, nstack);
	stilo_stack_pop(tstack);
}

/* > */
void
systemdict_sym_gt(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack;
	cw_stilo_t	*stilo, *dict, *key, *val;
	cw_uint32_t	npairs, i, depth;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	/* Find the mark. */
	for (i = 0, depth = stilo_stack_count(ostack), stilo = NULL; i < depth;
	     i++) {
		stilo = stilo_stack_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilo_thread_error(a_thread, STILO_THREADE_UNMATCHEDMARK);
		return;
	}
	if ((i & 1) == 1) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set npairs
	 * accordingly.  When we pop the stilo's off the stack, we'll have to
	 * pop (npairs << 1 + 1) stilo's.
	 */
	npairs = i >> 1;

	dict = stilo_stack_push(tstack);
	stilo_dict_new(dict, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), npairs);

	/*
	 * Traverse down the stack, moving stilo's to the dict.
	 */
	for (i = 0, key = NULL; i < npairs; i++) {
		val = stilo_stack_down_get(ostack, key);
		key = stilo_stack_down_get(ostack, val);
		stilo_dict_def(dict, stilo_thread_stil_get(a_thread), key, val);
	}

	/* Pop the stilo's off the stack now. */
	stilo_stack_npop(ostack, (npairs << 1) + 1);

	/* Push the dict onto the stack. */
	stilo = stilo_stack_push(ostack);
	stilo_dup(stilo, dict);

	stilo_stack_pop(tstack);
}

/* ] */
void
systemdict_sym_rb(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack;
	cw_stilo_t	*tstilo, *stilo;
	cw_sint32_t	nelements, i, depth;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	/* Find the mark. */
	for (i = 0, depth = stilo_stack_count(ostack), stilo = NULL; i < depth;
	     i++) {
		stilo = stilo_stack_down_get(ostack, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth) {
		stilo_thread_error(a_thread, STILO_THREADE_UNMATCHEDMARK);
		return;
	}

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	tstilo = stilo_stack_push(tstack);
	stilo_array_new(tstilo, stilo_thread_stil_get(a_thread),
	    stilo_thread_currentlocking(a_thread), nelements);

	/*
	 * Traverse down the stack, moving stilo's to the array.
	 */
	for (i = nelements - 1, stilo = NULL; i >= 0; i--) {
		stilo = stilo_stack_down_get(ostack, stilo);
		stilo_array_el_set(tstilo, stilo, i);
	}

	/* Pop the stilo's off the stack now. */
	stilo_stack_npop(ostack, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stilo_stack_push(ostack);
	stilo_dup(stilo, tstilo);

	stilo_stack_pop(tstack);
}

void
systemdict_symlink(cw_stilo_t *a_thread)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_tell(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*file;
	cw_stiloi_t	position;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(file, ostack, a_thread);
	
	if (stilo_type_get(file) != STILOT_FILE) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	position = stilo_file_position_get(file);
	if (position == -1) {
		stilo_thread_error(a_thread, STILO_THREADE_IOERROR);
		return;
	}
	stilo_integer_new(file, position);
}

void
systemdict_test(cw_stilo_t *a_thread)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_thread(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack, *stack;
	cw_stilo_t	*entry, *thread, *stilo;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);

	STILO_STACK_GET(entry, ostack, a_thread);
	STILO_STACK_DOWN_GET(stack, ostack, a_thread, entry);
	if (stilo_type_get(stack) != STILOT_STACK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	/* Create the new thread. */
	thread = stilo_stack_under_push(ostack, stack);
	stilo_thread_new(thread, stilo_thread_stil_get(a_thread));

	/* Set up the new thread's ostack. */
	stilo_stack_copy(stilo_thread_ostack_get(thread), stack);
	stilo = stilo_stack_push(stilo_thread_ostack_get(thread));
	stilo_dup(stilo, entry);

	/* Clean up. */
	stilo_stack_npop(ostack, 2);

	/* Start the thread. */
	stilo_thread_thread(thread);
}

void
systemdict_timedwait(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*condition, *mutex, *nsecs;
	struct timespec	timeout;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(nsecs, ostack, a_thread);
	STILO_STACK_DOWN_GET(mutex, ostack, a_thread, nsecs);
	STILO_STACK_DOWN_GET(condition, ostack, a_thread, mutex);
	if (stilo_type_get(condition) != STILOT_CONDITION ||
	    stilo_type_get(mutex) != STILOT_MUTEX || stilo_type_get(nsecs) !=
	    STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(nsecs) < 0) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}

	/* Convert integer to timespec. */
	timeout.tv_nsec = stilo_integer_get(nsecs) % 1000000000;
	timeout.tv_sec = stilo_integer_get(nsecs) / 1000000000;

	stilo_condition_timedwait(condition, mutex, &timeout);

	stilo_stack_npop(ostack, 3);
}

void
systemdict_token(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *tstack;
	cw_stilo_t	*stilo, *tstilo;

	ostack = stilo_thread_ostack_get(a_thread);
	tstack = stilo_thread_tstack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	switch (stilo_type_get(stilo)) {
	case STILOT_STRING: {
		cw_stilo_threadp_t	threadp;
		cw_uint32_t		nscanned, scount;

		scount = stilo_stack_count(ostack);
		tstilo = stilo_stack_push(tstack);
		stilo_dup(tstilo, stilo);
		stilo_threadp_new(&threadp);

		xep_begin();
		xep_try {
			stilo_string_lock(tstilo);
			nscanned = stilo_thread_token(a_thread, &threadp,
			    stilo_string_get(tstilo),
			    stilo_string_len_get(tstilo));
		}
		xep_acatch {
			stilo_string_unlock(tstilo);
			stilo_stack_pop(tstack);
			stilo_threadp_delete(&threadp, a_thread);
		}
		xep_end();
		stilo_string_unlock(tstilo);

		xep_begin();
		xep_try {
			stilo_thread_flush(a_thread, &threadp);
		}
		xep_acatch {
			stilo_stack_pop(tstack);
			stilo_threadp_delete(&threadp, a_thread);
		}
		xep_end();

		if (stilo_thread_state(a_thread) == THREADTS_START &&
		    stilo_thread_deferred(a_thread) == FALSE &&
		    stilo_stack_count(ostack) == scount + 1) {
			/* Success. */
			stilo_string_substring_new(stilo, tstilo,
			    stilo_thread_stil_get(a_thread), nscanned,
			    stilo_string_len_get(tstilo) - nscanned);
			stilo = stilo_stack_push(ostack);
			stilo_boolean_new(stilo, TRUE);
		} else {
			cw_uint32_t	i;

			/*
			 * We failed to scan a token.  Reset a_thread and clean
			 * up ostack.
			 */
			stilo_thread_reset(a_thread);
			for (i = stilo_stack_count(ostack); i > scount; i--)
				stilo_stack_pop(ostack);

			stilo_boolean_new(stilo, FALSE);
		}
		stilo_threadp_delete(&threadp, a_thread);
		stilo_stack_pop(tstack);
		break;
	}
	case STILOT_FILE: {
		cw_stilo_threadp_t	threadp;
		cw_sint32_t		nread;
		cw_uint32_t		scount;
		cw_uint8_t		c;

		scount = stilo_stack_count(ostack);
		tstilo = stilo_stack_push(tstack);
		stilo_dup(tstilo, stilo);
		stilo_threadp_new(&threadp);

		/*
		 * Feed the scanner one byte at a time, checking after every
		 * character whether a token was accepted.  If we run out of
		 * data, flush the scanner in the hope of causing token
		 * acceptance.
		 */
		for (nread = stilo_file_read(tstilo, 1, &c); nread > 0; nread =
		    stilo_file_read(tstilo, 1, &c)) {
			xep_begin();
			xep_try {
				stilo_thread_token(a_thread, &threadp, &c, 1);
			}
			xep_acatch {
				stilo_stack_pop(tstack);
				stilo_threadp_delete(&threadp, a_thread);
			}
			xep_end();

			if (stilo_thread_state(a_thread) == THREADTS_START &&
			    stilo_thread_deferred(a_thread) == FALSE &&
			    stilo_stack_count(ostack) == scount + 1)
				goto SUCCESS;
		}
		xep_begin();
		xep_try {
			stilo_thread_flush(a_thread, &threadp);
		}
		xep_acatch {
			stilo_stack_pop(tstack);
			stilo_threadp_delete(&threadp, a_thread);
		}
		xep_end();

		if (stilo_thread_state(a_thread) == THREADTS_START &&
		    stilo_thread_deferred(a_thread) == FALSE &&
		    stilo_stack_count(ostack) == scount + 1) {
			/* Success. */
			SUCCESS:
			stilo_boolean_new(stilo, TRUE);
			stilo_stack_roll(ostack, 2, 1);
		} else {
			cw_uint32_t	i;

			/*
			 * We failed to scan a token.  Reset a_thread and clean
			 * up ostack.
			 */
			stilo_thread_reset(a_thread);
			for (i = stilo_stack_count(ostack); i > scount; i--)
				stilo_stack_pop(ostack);

			stilo_boolean_new(stilo, FALSE);
		}
		stilo_threadp_delete(&threadp, a_thread);
		stilo_stack_pop(tstack);
		break;
	}
	default:
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
}

void
systemdict_truncate(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack;
	cw_stilo_t		*file, *length;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(length, ostack, a_thread);
	STILO_STACK_DOWN_GET(file, ostack, a_thread, length);
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(length) !=
	    STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	if (stilo_integer_get(length) < 0) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}

	error = stilo_file_truncate(file, stilo_integer_get(length));
	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_npop(ostack, 2);
}

void
systemdict_trylock(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*mutex;
	cw_bool_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(mutex, ostack, a_thread);
	if (stilo_type_get(mutex) != STILOT_MUTEX) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	error = stilo_mutex_trylock(mutex);

	stilo_boolean_new(mutex, error);
}

void
systemdict_type(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
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
		STILN_finotype,
		STILN_hooktype,
		STILN_integertype,
		STILN_marktype,
		STILN_mutextype,
		STILN_nametype,
		STILN_nulltype,
		STILN_operatortype,
		STILN_stacktype,
		STILN_stringtype,
		STILN_threadtype
	};
	_cw_assert(sizeof(typenames) / sizeof(cw_stiln_t) == STILOT_LAST + 1);

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);

	type = stilo_type_get(stilo);
	_cw_assert(type > STILOT_NO && type <= STILOT_LAST);

	stilo_name_new(stilo, stilo_thread_stil_get(a_thread),
	    stiln_str(typenames[type]), stiln_len(typenames[type]), TRUE);
	stilo_attrs_set(stilo, STILOA_EXECUTABLE);
}

void
systemdict_undef(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*dict, *key;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(key, ostack, a_thread);
	STILO_STACK_DOWN_GET(dict, ostack, a_thread, key);
	if (stilo_type_get(dict) != STILOT_DICT) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_dict_undef(dict, stilo_thread_stil_get(a_thread), key);

	stilo_stack_npop(ostack, 2);
}

void
systemdict_unlink(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*string;
	cw_uint8_t	str[PATH_MAX];
	cw_uint32_t	nbytes;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(string, ostack, a_thread);

	if (stilo_type_get(string) != STILOT_STRING) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
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

	stilo_stack_pop(ostack);

	if (unlink(str) == -1) {
		switch (errno) {
		case EACCES:
		case EPERM:
			stilo_thread_error(a_thread,
			    STILO_THREADE_INVALIDFILEACCESS);
		case EIO:
		case EBUSY:
			stilo_thread_error(a_thread, STILO_THREADE_IOERROR);
		default:
			stilo_thread_error(a_thread,
			    STILO_THREADE_UNDEFINEDFILENAME);
		}
		return;
	}
}

void
systemdict_unlock(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*mutex;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(mutex, ostack, a_thread);
	if (stilo_type_get(mutex) != STILOT_MUTEX) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_mutex_unlock(mutex);

	stilo_stack_pop(ostack);
}

void
systemdict_wait(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*condition, *mutex;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(mutex, ostack, a_thread);
	STILO_STACK_DOWN_GET(condition, ostack, a_thread, mutex);
	if (stilo_type_get(condition) != STILOT_CONDITION ||
	    stilo_type_get(mutex) != STILOT_MUTEX) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_condition_wait(condition, mutex);

	stilo_stack_npop(ostack, 2);
}

void
systemdict_waitpid(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	pid_t		pid;
	int		status;
	cw_stiloi_t	result;
	
	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
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
systemdict_where(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *dstack;
	cw_stilo_t	*dict, *key, *stilo;
	cw_uint32_t	i, depth;

	ostack = stilo_thread_ostack_get(a_thread);
	dstack = stilo_thread_dstack_get(a_thread);

	STILO_STACK_GET(key, ostack, a_thread);

	/*
	 * Iteratively search the dictionaries on the dictionary stack for key.
	 */
	for (i = 0, depth = stilo_stack_count(dstack), dict = NULL; i < depth;
	     i++) {
		dict = stilo_stack_down_get(dstack, dict);
		if (stilo_dict_lookup(dict, key, NULL) == FALSE) {
			/* Found. */
			stilo = stilo_stack_push(ostack);
			stilo_dup(key, dict);
			stilo_boolean_new(stilo, TRUE);
			return;
		}
	}
	/* Not found.  Create a new entry in currentdict. */
	stilo_boolean_new(key, FALSE);
}

void
systemdict_write(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack;
	cw_stilo_t		*file, *value;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(value, ostack, a_thread);
	STILO_STACK_DOWN_GET(file, ostack, a_thread, value);
	
	if (stilo_type_get(file) != STILOT_FILE) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	switch (stilo_type_get(value)) {
	case STILOT_INTEGER: {
		cw_uint8_t	val;

		val = (cw_uint8_t)stilo_integer_get(value);
		error = stilo_file_write(file, &val, 1);
		if (error) {
			stilo_thread_error(a_thread, error);
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
			stilo_thread_error(a_thread, error);
			return;
		}
		break;
	default:
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_stack_npop(ostack, 2);
}

void
systemdict_xcheck(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stiloa_t	attr;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	
	attr = stilo_attrs_get(stilo);

	if (attr == STILOA_EXECUTABLE)
		stilo_boolean_new(stilo, TRUE);
	else
		stilo_boolean_new(stilo, FALSE);
}

void
systemdict_xor(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo_a, *stilo_b;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo_b, ostack, a_thread);
	STILO_STACK_DOWN_GET(stilo_a, ostack, a_thread, stilo_b);

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
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}

	stilo_stack_pop(ostack);
}

void
systemdict_yield(cw_stilo_t *a_thread)
{
	thd_yield();
}
