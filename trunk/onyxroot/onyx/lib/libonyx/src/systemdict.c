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

#include "../include/libonyx/libonyx.h"

#include <sys/time.h>	/* For realtime operator. */
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <dirent.h>	/* For dirforeach operator. */

#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_array_l.h"
#include "../include/libonyx/nxo_operator_l.h"

/* Initial size of dictionaries created with the dict operator. */
#define	_CW_SYSTEMDICT_DICT_SIZE	16

struct cw_systemdict_entry {
	cw_nxn_t	nxn;
	cw_op_t		*op_f;
};

#define ENTRY(name)	{NXN_##name, systemdict_##name}

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
	ENTRY(rmdir),
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
	ENTRY(sload),
	ENTRY(spop),
	ENTRY(sprint),
	ENTRY(spush),
	ENTRY(srand),
	ENTRY(sroll),
	ENTRY(sstore),
	ENTRY(stack),
	ENTRY(start),
	ENTRY(status),
	ENTRY(stop),
	ENTRY(stopped),
	ENTRY(store),
	ENTRY(string),
	ENTRY(sub),
	{NXN_sym_hash_bang, systemdict_mark},
	{NXN_sym_bang_hash, systemdict_cleartomark},
	{NXN_sym_lp, systemdict_fino},
	ENTRY(sym_rp),
	{NXN_sym_lt, systemdict_mark},
	ENTRY(sym_gt),
	{NXN_sym_lb, systemdict_mark},
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
systemdict_l_populate(cw_nxo_t *a_dict, cw_nx_t *a_nx, int a_argc, char
    **a_argv)
{
	cw_uint32_t	i;
	cw_nxo_t	name, value;

#define	NEXTRA	12
#define NENTRIES							\
	(sizeof(systemdict_ops) / sizeof(struct cw_systemdict_entry))

	nxo_dict_new(a_dict, a_nx, TRUE, NENTRIES + NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		nxo_name_new(&name, a_nx,
		    nxn_str(systemdict_ops[i].nxn),
		    nxn_len(systemdict_ops[i].nxn), TRUE);
		nxo_operator_new(&value, systemdict_ops[i].op_f,
		    systemdict_ops[i].nxn);
		nxo_attrs_set(&value, NXOA_EXECUTABLE);

		nxo_dict_def(a_dict, a_nx, &name, &value);
	}

	/*
	 * Initialize entries that are not operators.
	 */

	/* globaldict. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_globaldict),
	    nxn_len(NXN_globaldict), TRUE);
	nxo_dup(&value, nx_globaldict_get(a_nx));
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* systemdict. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_systemdict),
	    nxn_len(NXN_systemdict), TRUE);
	nxo_dup(&value, nx_systemdict_get(a_nx));
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* gcdict. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_gcdict), nxn_len(NXN_gcdict),
	    TRUE);
	nxo_dup(&value, nxa_gcdict_get(nx_nxa_get(a_nx)));
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* envdict. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_envdict), nxn_len(NXN_envdict),
	    TRUE);
	nxo_dup(&value, nx_envdict_get(a_nx));
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* sprintdict. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_sprintdict),
	    nxn_len(NXN_sprintdict), TRUE);
	nxo_dup(&value, nx_sprintdict_get(a_nx));
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* argv. */
	{
		int		i;
		cw_sint32_t	len;
		cw_nxo_t	argv_nxo, str_nxo;
		cw_uint8_t	*t_str;

		/* Create the argv array and populate it. */
		nxo_array_new(&argv_nxo, a_nx, TRUE, a_argc);
		for (i = 0; i < a_argc; i++) {
			len = strlen(a_argv[i]);
			nxo_string_new(&str_nxo, a_nx, TRUE, len);
			t_str = nxo_string_get(&str_nxo);
			nxo_string_lock(&str_nxo);
			memcpy(t_str, a_argv[i], len);
			nxo_string_unlock(&str_nxo);

			nxo_array_el_set(&argv_nxo, &str_nxo, i);
		}

		/* Insert argv into systemdict. */
		nxo_name_new(&name, a_nx, nxn_str(NXN_argv),
		    nxn_len(NXN_argv), TRUE);
		nxo_dict_def(a_dict, a_nx, &name, &argv_nxo);
	}

	/* stdin. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_stdin), nxn_len(NXN_stdin), TRUE);
	nxo_dup(&value, nx_stdin_get(a_nx));
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* stdout. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_stdout), nxn_len(NXN_stdout),
	    TRUE);
	nxo_dup(&value, nx_stdout_get(a_nx));
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* stderr. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_stderr), nxn_len(NXN_stderr),
	    TRUE);
	nxo_dup(&value, nx_stderr_get(a_nx));
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* true. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_true), nxn_len(NXN_true), TRUE);
	nxo_boolean_new(&value, TRUE);
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* false. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_false), nxn_len(NXN_false), TRUE);
	nxo_boolean_new(&value, FALSE);
	nxo_dict_def(a_dict, a_nx, &name, &value);

	/* null. */
	nxo_name_new(&name, a_nx, nxn_str(NXN_null), nxn_len(NXN_null), TRUE);
	nxo_null_new(&value);
	nxo_dict_def(a_dict, a_nx, &name, &value);

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

void
systemdict_abs(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*a;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(a, ostack, a_thread);
	if (nxo_type_get(a) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	if (nxo_integer_get(a) < 0)
		nxo_integer_set(a, -nxo_integer_get(a));
}

void
systemdict_add(cw_nxo_t *a_thread)
{
	systemdict_inline_add(a_thread);
}

void
systemdict_aload(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*array, *nxo;
	cw_uint32_t	i, len;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(array, ostack, a_thread);
	if (nxo_type_get(array) != NXOT_ARRAY) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	for (i = 0, len = nxo_array_len_get(array); i < len; i++) {
		nxo = nxo_stack_under_push(ostack, array);
		nxo_array_el_set(array, nxo, i);
	}
}

void
systemdict_and(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	if (nxo_type_get(nxo_a) == NXOT_BOOLEAN && nxo_type_get(nxo_b) ==
	    NXOT_BOOLEAN) {
		cw_bool_t	and;

		if (nxo_boolean_get(nxo_a) && nxo_boolean_get(nxo_b))
			and = TRUE;
		else
			and = FALSE;
		nxo_boolean_new(nxo_a, and);
	} else if (nxo_type_get(nxo_a) == NXOT_INTEGER && nxo_type_get(nxo_b) ==
	    NXOT_INTEGER) {
		nxo_integer_set(nxo_a, nxo_integer_get(nxo_a) &
		    nxo_integer_get(nxo_b));
	} else {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_stack_pop(ostack);
}

void
systemdict_array(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_nxoi_t	len;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	len = nxo_integer_get(nxo);
	if (len < 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	nxo_array_new(nxo, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), len);
}

void
systemdict_astore(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*array, *nxo;
	cw_sint32_t	i, len;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	NXO_STACK_GET(array, ostack, a_thread);
	if (nxo_type_get(array) != NXOT_ARRAY) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	/* Make sure there will be enough objects to fill the array. */
	len = nxo_array_len_get(array);
	if (len > nxo_stack_count(ostack) - 1) {
		nxo_thread_error(a_thread, NXO_THREADE_STACKUNDERFLOW);
		return;
	}

	nxo = nxo_stack_push(tstack);
	nxo_dup(nxo, array);
	nxo_stack_pop(ostack);
	array = nxo;

	/* Move ostack objects to the array. */
	for (i = len - 1; i >= 0; i--) {
		nxo_array_el_set(array, nxo_stack_get(ostack), i);
		nxo_stack_pop(ostack);
	}

	/* Push the array back onto ostack. */
	nxo = nxo_stack_push(ostack);
	nxo_dup(nxo, array);
	nxo_stack_pop(tstack);
}

void
systemdict_begin(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *dstack;
	cw_nxo_t	*nxo, *dict;

	dstack = nxo_thread_dstack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(dict, ostack, a_thread);
	if (nxo_type_get(dict) != NXOT_DICT) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	
	nxo = nxo_stack_push(dstack);
	nxo_dup(nxo, dict);
	nxo_stack_pop(ostack);
}

static void
systemdict_p_bind(cw_nxo_t *a_proc, cw_nxo_t *a_thread)
{
	cw_nxo_t	*tstack;
	cw_nxo_t	*el, *val;
	cw_uint32_t	i, count;
	/*
	 * Array of fastops.  nxo_thread_loop() must have corresponding
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

	tstack = nxo_thread_tstack_get(a_thread);

	val = nxo_stack_push(tstack);
	el = nxo_stack_push(tstack);

	nxo_l_array_bound_set(a_proc, TRUE);

	for (i = 0, count = nxo_array_len_get(a_proc); i < count; i++) {
		nxo_array_el_get(a_proc, i, el);
		if (nxo_attrs_get(el) != NXOA_EXECUTABLE)
			continue;

		switch (nxo_type_get(el)) {
		case NXOT_ARRAY:
			if (nxo_l_array_bound_get(el) == FALSE)
				systemdict_p_bind(el, a_thread);
			break;
		case NXOT_NAME:
			if (nxo_thread_dstack_search(a_thread, el, val) ==
			    FALSE) {
				if (nxo_type_get(val) == NXOT_OPERATOR) {
					cw_uint32_t	j;

#define	NFASTOPS							\
	(sizeof(fastops) / sizeof(struct cw_systemdict_entry))

					/*
					 * If val can be converted to a fastop,
					 * do so.
					 */
					for (j = 0; j < NFASTOPS; j++) {
						if (nxo_operator_f(val) ==
						    fastops[j].op_f) {
							nxo_dup(el, val);
							nxo_l_operator_fast_op_set(el,
							    fastops[j].nxn);
							nxo_array_el_set(
							    a_proc, el, i);
							break;
						}
					}
					/*
					 * If val isn't a fastop, nxl convert
					 * the name to an operator.
					 */
					if (j == NFASTOPS) {
						/* Replace el with val. */
						nxo_array_el_set(a_proc, val,
						    i);
					}
#undef NFASTOPS
				} else if (nxo_attrs_get(val) !=
				    NXOA_EXECUTABLE) {
					/* Replace el with val. */
					nxo_dup(el, val);
				}
			}
		default:
		}
	}

	nxo_stack_npop(tstack, 2);
}

void
systemdict_bind(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*array;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(array, ostack, a_thread);

	systemdict_p_bind(array, a_thread);
}

void
systemdict_broadcast(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*condition;
	
	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(condition, ostack, a_thread);
	if (nxo_type_get(condition) != NXOT_CONDITION) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_condition_broadcast(condition);

	nxo_stack_pop(ostack);
}

void
systemdict_bytesavailable(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*file;
	cw_uint32_t	bytes;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(file, ostack, a_thread);
	
	if (nxo_type_get(file) != NXOT_FILE) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	bytes = nxo_file_buffer_count(file);
	nxo_integer_new(file, bytes);
}

void
systemdict_catenate(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *a, *b, *r;
	cw_uint32_t	i, len_a, len_b;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(b, ostack, a_thread);
	NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (nxo_type_get(a) != nxo_type_get(b) || (nxo_type_get(a) !=
	    NXOT_ARRAY && nxo_type_get(a) != NXOT_STACK &&
	    nxo_type_get(a) != NXOT_STRING)) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	r = nxo_stack_under_push(ostack, a);

	switch (nxo_type_get(a)) {
	case NXOT_ARRAY: {
		cw_nxo_t	*tstack, *tnxo;
		
		tstack = nxo_thread_tstack_get(a_thread);
		tnxo = nxo_stack_push(tstack);

		len_a = nxo_array_len_get(a);
		len_b = nxo_array_len_get(b);

		nxo_array_new(r, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread), len_a + len_b);

		for (i = 0; i < len_a; i++) {
			nxo_array_el_get(a, i, tnxo);
			nxo_array_el_set(r, tnxo, i);
		}
		for (i = 0; i < len_b; i++) {
			nxo_array_el_get(b, i, tnxo);
			nxo_array_el_set(r, tnxo, i + len_a);
		}

		nxo_stack_pop(tstack);

		break;
	}
	case NXOT_STACK: {
		cw_nxo_t	*fr, *to;

		len_a = nxo_stack_count(a);
		len_b = nxo_stack_count(b);

		nxo_stack_new(r, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread));

		for (i = 0, fr = to = NULL; i < len_b; i++) {
			fr = nxo_stack_down_get(b, fr);
			to = nxo_stack_under_push(r, to);
			nxo_dup(to, fr);
		}
		for (i = 0, fr = NULL; i < len_a; i++) {
			fr = nxo_stack_down_get(a, fr);
			to = nxo_stack_under_push(r, to);
			nxo_dup(to, fr);
		}

		break;
	}
	case NXOT_STRING:
		len_a = nxo_string_len_get(a);
		len_b = nxo_string_len_get(b);

		nxo_string_new(r, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread), len_a + len_b);

		nxo_string_lock(r);
		nxo_string_lock(a);
		nxo_string_set(r, 0, nxo_string_get(a), len_a);
		nxo_string_unlock(a);

		nxo_string_lock(b);
		nxo_string_set(r, len_a, nxo_string_get(b), len_b);
		nxo_string_unlock(b);
		nxo_string_unlock(r);

		break;
	default:
		_cw_not_reached();
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_cd(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack, *path, *tpath;
	int		error;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(path, ostack, a_thread);
	if (nxo_type_get(path) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/*
	 * Create a copy of the path with an extra byte to store a '\0'
	 * terminator.
	 */
	tpath = nxo_stack_push(tstack);
	nxo_string_new(tpath, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(path) + 1);
	nxo_string_lock(path);
	nxo_string_lock(tpath);
	nxo_string_set(tpath, 0, nxo_string_get(path),
	    nxo_string_len_get(path));
	nxo_string_el_set(tpath, '\0', nxo_string_len_get(tpath) - 1);
	nxo_string_unlock(path);

	error = chdir(nxo_string_get(tpath));
	if (error == -1) {
		nxo_string_unlock(tpath);
		switch (errno) {
		case EIO:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		default:
			nxo_thread_error(a_thread, NXO_THREADE_INVALIDACCESS);
		}
		goto ERROR;
	}

	nxo_string_unlock(tpath);

	nxo_stack_pop(ostack);

	ERROR:
	nxo_stack_pop(tstack);
}

void
systemdict_chmod(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *file, *mode;
	int		error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(mode, ostack, a_thread);
	NXO_STACK_DOWN_GET(file, ostack, a_thread, mode);
	if ((nxo_type_get(mode) != NXOT_INTEGER) || (nxo_type_get(file) !=
	    NXOT_FILE && nxo_type_get(file) != NXOT_STRING)) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(mode) < 0 || nxo_integer_get(mode) > 0xfff) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	if (nxo_type_get(file) == NXOT_FILE) {
		int	fd;

		fd = nxo_file_fd_get(file);
		if (fd < 0) {
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			return;
		}

		error = fchmod(fd, nxo_integer_get(mode));
	} else {
		cw_nxo_t	*tstack, *tfile;

		tstack = nxo_thread_tstack_get(a_thread);

		/*
		 * Create a copy of file with an extra byte to store a '\0'
		 * terminator.
		 */
		tfile = nxo_stack_push(tstack);
		nxo_string_new(tfile, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread),
		    nxo_string_len_get(file) + 1);
		nxo_string_lock(file);
		nxo_string_lock(tfile);
		nxo_string_set(tfile, 0, nxo_string_get(file),
		    nxo_string_len_get(file));
		nxo_string_el_set(tfile, '\0', nxo_string_len_get(tfile) - 1);
		nxo_string_unlock(file);

		error = chmod(nxo_string_get(tfile), nxo_integer_get(mode));

		nxo_string_unlock(tfile);

		nxo_stack_pop(tstack);
	}

	if (error == -1) {
		switch (errno) {
		case EIO:
		case EROFS:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		case EACCES:
		case EFTYPE:
		case EINVAL:
		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
		case EPERM:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			break;
		case EBADF:
		case EFAULT:
		default:
			nxo_thread_error(a_thread, NXO_THREADE_UNREGISTERED);
		}
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_chown(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *file, *uid, *gid;
	int		error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(gid, ostack, a_thread);
	NXO_STACK_DOWN_GET(uid, ostack, a_thread, gid);
	NXO_STACK_DOWN_GET(file, ostack, a_thread, uid);
	if ((nxo_type_get(file) != NXOT_FILE && nxo_type_get(file) !=
	    NXOT_STRING) || nxo_type_get(gid) != NXOT_INTEGER ||
	    nxo_type_get(uid) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(uid) < 0 || nxo_integer_get(gid) < 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	if (nxo_type_get(file) == NXOT_FILE) {
		int	fd;

		fd = nxo_file_fd_get(file);
		if (fd < 0) {
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			return;
		}

		error = fchown(fd, nxo_integer_get(uid), nxo_integer_get(gid));
	} else {
		cw_nxo_t	*tstack, *tfile;

		tstack = nxo_thread_tstack_get(a_thread);

		/*
		 * Create a copy of file with an extra byte to store a '\0'
		 * terminator.
		 */
		tfile = nxo_stack_push(tstack);
		nxo_string_new(tfile, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread),
		    nxo_string_len_get(file) + 1);
		nxo_string_lock(file);
		nxo_string_lock(tfile);
		nxo_string_set(tfile, 0, nxo_string_get(file),
		    nxo_string_len_get(file));
		nxo_string_el_set(tfile, '\0', nxo_string_len_get(tfile) - 1);
		nxo_string_unlock(file);

		error = chown(nxo_string_get(tfile), nxo_integer_get(uid),
		    nxo_integer_get(gid));

		nxo_string_unlock(tfile);

		nxo_stack_pop(tstack);
	}

	if (error == -1) {
		switch (errno) {
		case EIO:
		case EROFS:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		case EACCES:
		case EINVAL:
		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
		case EPERM:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			break;
		case EBADF:
		case EFAULT:
		default:
			nxo_thread_error(a_thread,
			    NXO_THREADE_UNREGISTERED);
		}
		return;
	}

	nxo_stack_npop(ostack, 3);
}

void
systemdict_clear(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_uint32_t	count;

	ostack = nxo_thread_ostack_get(a_thread);
	count = nxo_stack_count(ostack);
	if (count > 0)
		nxo_stack_npop(ostack, count);
}

void
systemdict_cleardstack(cw_nxo_t *a_thread)
{
	cw_nxo_t	*dstack;
	cw_uint32_t	count;

	dstack = nxo_thread_dstack_get(a_thread);
	count = nxo_stack_count(dstack);
	if (count > 3)
		nxo_stack_npop(dstack, count - 3);
}

void
systemdict_cleartomark(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_uint32_t	i, depth;

	ostack = nxo_thread_ostack_get(a_thread);

	for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth;
	    i++) {
		nxo = nxo_stack_down_get(ostack, nxo);
		if (nxo_type_get(nxo) == NXOT_MARK)
			break;
	}
	if (i == depth) {
		nxo_thread_error(a_thread, NXO_THREADE_UNMATCHEDMARK);
		return;
	}

	nxo_stack_npop(ostack, i + 1);
}

void
systemdict_closefile(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack;
	cw_nxo_t		*nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_FILE) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	error = nxo_file_close(nxo);
	if (error) {
		nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
		return;
	}

	nxo_stack_pop(ostack);
}

void
systemdict_condition(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*condition;

	ostack = nxo_thread_ostack_get(a_thread);
	condition = nxo_stack_push(ostack);
	nxo_condition_new(condition, nxo_thread_nx_get(a_thread));
}

void
systemdict_copy(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);

	switch (nxo_type_get(nxo)) {
	case NXOT_INTEGER: {
		cw_nxo_t	*dup;
		cw_uint32_t	i;
		cw_nxoi_t	count;

		/* Dup a range of the stack. */
		count = nxo_integer_get(nxo);
		if (count < 0) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}
		if (count > nxo_stack_count(ostack) - 1) {
			nxo_thread_error(a_thread, NXO_THREADE_STACKUNDERFLOW);
			return;
		}
		nxo_stack_pop(ostack);

		/*
		 * Iterate down the stack, creating dup's along the way.  Since
		 * we're going down, it's necessary to use
		 * nxo_stack_under_push() to preserve order.
		 */
		for (i = 0, nxo = NULL, dup = NULL; i < count; i++) {
			nxo = nxo_stack_down_get(ostack, nxo);
			dup = nxo_stack_under_push(ostack, dup);
			nxo_dup(dup, nxo);
		}
		break;
	}
	case NXOT_ARRAY: {
		cw_nxo_t	*orig;

		NXO_STACK_DOWN_GET(orig, ostack, a_thread, nxo);
		if (nxo_type_get(orig) != NXOT_ARRAY) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}
		if (nxo_array_len_get(nxo) < nxo_array_len_get(orig)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}

		nxo_array_copy(nxo, orig);

		nxo_stack_roll(ostack, 2, 1);
		nxo_stack_pop(ostack);
		break;
	}
	case NXOT_DICT: {
		cw_nxo_t	*orig;

		NXO_STACK_DOWN_GET(orig, ostack, a_thread, nxo);
		if (nxo_type_get(orig) != NXOT_DICT) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}

		nxo_dict_copy(nxo, orig, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread));
		break;
	}
	case NXOT_STACK: {
		cw_nxo_t	*orig;

		NXO_STACK_DOWN_GET(orig, ostack, a_thread, nxo);
		if (nxo_type_get(orig) != NXOT_STACK) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}

		nxo_stack_copy(nxo, orig);

		nxo_stack_roll(ostack, 2, 1);
		nxo_stack_pop(ostack);
		break;
	}
	case NXOT_STRING: {
		cw_nxo_t	*orig;

		NXO_STACK_DOWN_GET(orig, ostack, a_thread, nxo);
		if (nxo_type_get(orig) != NXOT_STRING) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}
		if (nxo_string_len_get(nxo) < nxo_string_len_get(orig)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}

		nxo_string_copy(nxo, orig);

		nxo_stack_roll(ostack, 2, 1);
		nxo_stack_pop(ostack);
		break;
	}
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
}

void
systemdict_count(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);

	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, nxo_stack_count(ostack) - 1);
}

void
systemdict_countdstack(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *dstack;
	cw_nxo_t	*nxo;

	dstack = nxo_thread_dstack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);

	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, nxo_stack_count(dstack));
}

void
systemdict_countestack(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack;
	cw_nxo_t	*nxo;

	estack = nxo_thread_estack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);

	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, nxo_stack_count(estack));
}

void
systemdict_counttomark(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_uint32_t	i, depth;

	ostack = nxo_thread_ostack_get(a_thread);

	for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth;
	    i++) {
		nxo = nxo_stack_down_get(ostack, nxo);
		if (nxo_type_get(nxo) == NXOT_MARK)
			break;
	}
	if (i == depth) {
		nxo_thread_error(a_thread, NXO_THREADE_UNMATCHEDMARK);
		return;
	}

	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, i);
}

void
systemdict_currentdict(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *dstack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	dstack = nxo_thread_dstack_get(a_thread);

	nxo = nxo_stack_push(ostack);
	nxo_dup(nxo, nxo_stack_get(dstack));
}

void
systemdict_currentfile(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack;
	cw_nxo_t	*file, *nxo;
	cw_uint32_t	i, depth;

	estack = nxo_thread_estack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);

	file = nxo_stack_push(ostack);
	for (i = 0, depth = nxo_stack_count(estack), nxo = NULL; i < depth;
	    i++) {
		nxo = nxo_stack_down_get(estack, nxo);
		if (nxo_type_get(nxo) == NXOT_FILE) {
			nxo_dup(file, nxo);
			break;
		}
	}
	if (i == depth) {
		nxo_file_new(file, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread));
	}
}

void
systemdict_currentlocking(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	nxo = nxo_stack_push(ostack);
	nxo_boolean_new(nxo, nxo_thread_currentlocking(a_thread));
}

void
systemdict_cvlit(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	nxo_attrs_set(nxo, NXOA_LITERAL);
}

void
systemdict_cvn(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*nxo, *tnxo;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);

	if (nxo_type_get(nxo) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	tnxo = nxo_stack_push(tstack);
	nxo_dup(tnxo, nxo);

	nxo_string_lock(tnxo);
	nxo_name_new(nxo, nxo_thread_nx_get(a_thread),
	    nxo_string_get(tnxo), nxo_string_len_get(tnxo), FALSE);
	nxo_string_unlock(tnxo);
	nxo_attrs_set(nxo, nxo_attrs_get(tnxo));

	nxo_stack_pop(tstack);
}

void
systemdict_cvrs(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*num, *radix;
	cw_uint64_t	val;
	cw_uint32_t	i, rlen, base;
	cw_uint8_t	*str;
	static const cw_uint8_t *syms = "0123456789abcdefghijklmnopqrstuvwxyz";
	cw_uint8_t	*result, s_result[65] =
	    "0000000000000000000000000000000000000000000000000000000000000000";

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(radix, ostack, a_thread);
	NXO_STACK_DOWN_GET(num, ostack, a_thread, radix);
	if ((nxo_type_get(num) != NXOT_INTEGER) || (nxo_type_get(radix) !=
	    NXOT_INTEGER)) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	base = nxo_integer_get(radix);
	if (base < 2 || base > 36) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}
	val = (cw_uint64_t)nxo_integer_get(num);

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

	nxo_string_new(num, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), rlen);

	str = nxo_string_get(num);
	nxo_string_lock(num);
	memcpy(str, result, rlen);
	nxo_string_unlock(num);

	nxo_stack_pop(ostack);
}

void
systemdict_cvs(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	
	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);

	switch (nxo_type_get(nxo)) {
	case NXOT_BOOLEAN:
		_cw_onyx_code(a_thread, "{`true'} {`false'} ifelse");
		break;
	case NXOT_INTEGER: {
		cw_uint8_t	result[21];
		cw_sint32_t	len;

#if (_CW_NXOI_SIZEOF == 8)
		len = _cw_out_put_s(result, "[q|s:s]", nxo_integer_get(nxo));
#elif (_CW_NXOI_SIZEOF == 4)
		len = _cw_out_put_s(result, "[i|s:s]", nxo_integer_get(nxo));
#else
#error "Unsupported integer size"
#endif
		nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread), len);
		nxo_string_lock(nxo);
		nxo_string_set(nxo, 0, result, len);
		nxo_string_unlock(nxo);
		break;
	}
	case NXOT_NAME: {
		cw_nxo_t	*tstack;
		cw_nxo_t	*tnxo;

		tstack = nxo_thread_tstack_get(a_thread);
		tnxo = nxo_stack_push(tstack);
		nxo_dup(tnxo, nxo);

		nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread),
		    nxo_name_len_get(tnxo));
		nxo_string_lock(nxo);
		nxo_string_set(nxo, 0, nxo_name_str_get(tnxo),
		    nxo_name_len_get(tnxo));
		nxo_string_unlock(nxo);

		nxo_stack_pop(tstack);
		break;
	}
	case NXOT_OPERATOR: {
		cw_nxn_t	nxn;

		nxn = nxo_l_operator_fast_op_nxn(nxo);
		if (nxn > NXN_LAST)
			_cw_onyx_code(a_thread, "pop `--operator--'");
		else {
			cw_nxo_t	*tstack;
			cw_nxo_t	*tnxo;

			tstack = nxo_thread_tstack_get(a_thread);
			tnxo = nxo_stack_push(tstack);
			nxo_dup(tnxo, nxo);

			nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
			    nxo_thread_currentlocking(a_thread),
			    nxn_len(nxn));
			nxo_string_lock(nxo);
			nxo_string_set(nxo, 0, nxn_str(nxn), nxn_len(nxn));
			nxo_string_unlock(nxo);

			nxo_stack_pop(tstack);
		}
		break;
	}
	case NXOT_STRING:
		break;
	case NXOT_NO:
	case NXOT_ARRAY:
	case NXOT_CONDITION:
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_HOOK:
	case NXOT_MUTEX:
	case NXOT_MARK:
	case NXOT_NULL:
	default:
		_cw_onyx_code(a_thread, "pop `--nostringval--'");
		break;
	}
}

void
systemdict_cvx(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	nxo_attrs_set(nxo, NXOA_EXECUTABLE);
}

void
systemdict_def(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *dstack;
	cw_nxo_t	*dict, *key, *val;

	ostack = nxo_thread_ostack_get(a_thread);
	dstack = nxo_thread_dstack_get(a_thread);

	dict = nxo_stack_get(dstack);
	NXO_STACK_GET(val, ostack, a_thread);
	NXO_STACK_DOWN_GET(key, ostack, a_thread, val);

	nxo_dict_def(dict, nxo_thread_nx_get(a_thread), key, val);

	nxo_stack_npop(ostack, 2);
}

void
systemdict_detach(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*thread;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(thread, ostack, a_thread);
	if (nxo_type_get(thread) != NXOT_THREAD) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_thread_detach(thread);

	nxo_stack_pop(ostack);
}

void
systemdict_dict(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*dict;

	ostack = nxo_thread_ostack_get(a_thread);

	dict = nxo_stack_push(ostack);
	nxo_dict_new(dict, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), _CW_SYSTEMDICT_DICT_SIZE);
}

void
systemdict_dirforeach(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *tstack;
	cw_nxo_t	*nxo, *tnxo, *path, *proc, *scratch, *entry;
	DIR		*dir;
	struct dirent	ent, *entp;
	int		error;
	cw_uint32_t	edepth, tdepth;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	edepth = nxo_stack_count(estack);
	tdepth = nxo_stack_count(tstack);

	NXO_STACK_GET(tnxo, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, tnxo);
	if (nxo_type_get(nxo) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/*
	 * Make a copy of the procedure to execute.
	 */
	proc = nxo_stack_push(tstack);
	nxo_dup(proc, tnxo);

	/*
	 * Create a copy of the path with an extra byte to store a '\0'
	 * terminator.
	 */
	path = nxo_stack_push(tstack);
	nxo_string_new(path, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(nxo) + 1);
	nxo_string_lock(path);
	nxo_string_lock(nxo);
	nxo_string_set(path, 0, nxo_string_get(nxo), nxo_string_len_get(nxo));
	nxo_string_el_set(path, '\0', nxo_string_len_get(path) - 1);
	nxo_string_unlock(nxo);
	nxo_string_unlock(path);

	/*
	 * Open the directory.
	 */
	dir = opendir(nxo_string_get(path));
	if (dir == NULL) {
		nxo_stack_npop(tstack, 2);
		nxo_thread_error(a_thread, NXO_THREADE_INVALIDACCESS);
		return;
	}

	/* Pop the path and proc off ostack before going into the loop. */
	nxo_stack_npop(ostack, 2);

	/*
	 * Create a scratch string for storing directory entries.  The string
	 * pushed onto ostack in the loop below is a substring of this.
	 */
	scratch = nxo_stack_push(tstack);
	nxo_string_new(scratch, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), PATH_MAX);

	xep_begin();
	xep_try {
		/*
		 * Iterate through the directory.
		 */
		while ((error = readdir_r(dir, &ent, &entp) == 0) && entp ==
		    &ent) {
			_cw_assert(ent.d_namlen < PATH_MAX);

			/*
			 * Push a string onto ostack that represents the
			 * directory entry.
			 */
			entry = nxo_stack_push(ostack);
			nxo_string_set(scratch, 0, ent.d_name, ent.d_namlen);
			nxo_string_substring_new(entry, scratch,
			    nxo_thread_nx_get(a_thread), 0, ent.d_namlen);

			/*
			 * Evaluate proc.
			 */
			nxo = nxo_stack_push(estack);
			nxo_dup(nxo, proc);
			nxo_thread_loop(a_thread);
		}
		if (error && entp != NULL) {
			/* The loop terminated due to an error. */
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
		}
	}
	xep_catch(_CW_ONYXX_EXIT) {
		xep_handled();

		/* Clean up estack. */
		nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	}
	xep_acatch {
		/* Close the directory. */
		closedir(dir);
	}
	xep_end();

	/* Close the directory. */
	closedir(dir);

	/* Clean up tstack. */
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
}

void
systemdict_div(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*a, *b;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(b, ostack, a_thread);
	NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (nxo_type_get(a) != NXOT_INTEGER || nxo_type_get(b) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(b) == 0) {
		nxo_thread_error(a_thread, NXO_THREADE_UNDEFINEDRESULT);
		return;
	}

	nxo_integer_set(a, nxo_integer_get(a) / nxo_integer_get(b));
	nxo_stack_pop(ostack);
}

void
systemdict_dstack(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *dstack, *stack;

	ostack = nxo_thread_ostack_get(a_thread);
	dstack = nxo_thread_dstack_get(a_thread);

	stack = nxo_stack_push(ostack);
	nxo_stack_new(stack, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread));
	nxo_stack_copy(stack, dstack);
}

void
systemdict_dup(cw_nxo_t *a_thread)
{
	systemdict_inline_dup(a_thread);
}

void
systemdict_end(cw_nxo_t *a_thread)
{
	cw_nxo_t	*dstack;

	dstack = nxo_thread_dstack_get(a_thread);

	/* threaddict, systemdict, globaldict, and userdict cannot be popped. */
	if (nxo_stack_count(dstack) <= 4) {
		nxo_thread_error(a_thread, NXO_THREADE_DSTACKUNDERFLOW);
		return;
	}

	nxo_stack_pop(dstack);
}

void
systemdict_eq(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;
	cw_sint32_t	result;
	cw_bool_t	eq;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	result = nxo_compare(nxo_a, nxo_b);
	if (result == 0)
		eq = TRUE;
	else
		eq = FALSE;

	nxo_boolean_new(nxo_a, eq);

	nxo_stack_pop(ostack);
}

void
systemdict_estack(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *stack;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);

	stack = nxo_stack_push(ostack);
	nxo_stack_new(stack, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread));
	nxo_stack_copy(stack, estack);
}

void
systemdict_eval(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack;
	cw_nxo_t	*orig, *new;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);

	NXO_STACK_GET(orig, ostack, a_thread);
	new = nxo_stack_push(estack);
	nxo_dup(new, orig);
	nxo_stack_pop(ostack);

	nxo_thread_loop(a_thread);
}

void
systemdict_exch(cw_nxo_t *a_thread)
{
	systemdict_inline_exch(a_thread);
}

void
systemdict_exec(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*array, *el;
	cw_uint32_t	i, slen, argc;
	char		*path, **argv, **envp;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	el = nxo_stack_push(tstack);
	
	NXO_STACK_GET(array, ostack, a_thread);
	if (nxo_type_get(array) != NXOT_ARRAY) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		goto VALIDATION_ERROR;
	}
	argc = nxo_array_len_get(array);
	for (i = 0; i < argc; i++) {
		nxo_array_el_get(array, i, el);
		if (nxo_type_get(el) != NXOT_STRING) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			goto VALIDATION_ERROR;
		}
	}

	/*
	 * Construct path.
	 */
	nxo_array_el_get(array, 0, el);
	if (nxo_type_get(el) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		goto PATH_ERROR;
	}
	slen = nxo_string_len_get(el);
	path = (char *)_cw_malloc(slen + 1);
	nxo_string_lock(el);
	memcpy(path, nxo_string_get(el), slen);
	nxo_string_unlock(el);
	path[slen] = '\0';

	/*
	 * Construct argv.
	 */
	argv = (char **)_cw_malloc(sizeof(char *) * (argc + 1));
	for (i = 0; i < argc; i++) {
		nxo_array_el_get(array, i, el);
		if (nxo_type_get(el) != NXOT_STRING) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			goto ARGV_ERROR;
		}
		slen = nxo_string_len_get(el);
		argv[i] = (char *)_cw_malloc(slen + 1);
		nxo_string_lock(el);
		memcpy(argv[i], nxo_string_get(el), slen);
		nxo_string_unlock(el);
		argv[i][slen] = '\0';
	}
	argv[i] = NULL;

	/*
	 * Construct envp.
	 */
	{
		cw_uint32_t	dcount, key_len, val_len;
		cw_nxo_t	*key, *val;
		char		*entry;

		key = nxo_stack_push(tstack);
		val = nxo_stack_push(tstack);

		dcount = nxo_dict_count(nx_envdict_get(
		    nxo_thread_nx_get(a_thread)));
		envp = (char **)_cw_malloc(sizeof(char *) * (dcount + 1));
		for (i = 0; i < dcount; i++) {
			/* Get key and val. */
			nxo_dict_iterate(nx_envdict_get(
			    nxo_thread_nx_get(a_thread)), key);
			nxo_dict_lookup(nx_envdict_get(
			    nxo_thread_nx_get(a_thread)), key, val);
			if (nxo_type_get(key) != NXOT_NAME || nxo_type_get(val)
			    != NXOT_STRING) {
				nxo_thread_error(a_thread,
				    NXO_THREADE_TYPECHECK);
				nxo_stack_npop(tstack, 2);
				goto ENVP_ERROR;
			}

			/* Create string that looks like "<key>=<val>\0". */
			key_len = nxo_name_len_get(key);
			val_len = nxo_string_len_get(val);
			entry = (char *)_cw_malloc(key_len + val_len + 2);

			memcpy(entry, nxo_name_str_get(key), key_len);
			entry[key_len] = '=';
			nxo_string_lock(val);
			memcpy(&entry[key_len + 1], nxo_string_get(val),
			    val_len);
			nxo_string_unlock(val);
			entry[key_len + 1 + val_len] = '\0';

			envp[i] = entry;
		}
		envp[i] = NULL;

		nxo_stack_npop(tstack, 2);
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
	nxo_stack_pop(tstack);
}

void
systemdict_exit(cw_nxo_t *a_thread)
{
	xep_throw(_CW_ONYXX_EXIT);
}

void
systemdict_exp(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*a, *b;
	cw_uint32_t	i;
	cw_nxoi_t	r;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(b, ostack, a_thread);
	NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (nxo_type_get(a) != NXOT_INTEGER || nxo_type_get(b) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	if (nxo_integer_get(b) < 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	} else if (nxo_integer_get(b) > 0) {
		for (i = 1, r = nxo_integer_get(a); i < nxo_integer_get(b); i++)
			r *= nxo_integer_get(a);
	} else
		r = 1;

	nxo_integer_set(a, r);
	nxo_stack_pop(ostack);
}

void
systemdict_fino(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	nxo = nxo_stack_push(ostack);
	nxo_fino_new(nxo);
}

void
systemdict_flush(cw_nxo_t *a_thread)
{
	cw_nxo_threade_t	error;

	error =
	    nxo_file_buffer_flush(nx_stdout_get(nxo_thread_nx_get(a_thread)));
	if (error)
		nxo_thread_error(a_thread, error);
}

void
systemdict_flushfile(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack;
	cw_nxo_t		*file;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(file, ostack, a_thread);
	
	if (nxo_type_get(file) != NXOT_FILE) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	error = nxo_file_buffer_flush(file);
	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}
	
	nxo_stack_pop(ostack);
}

void
systemdict_for(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *tstack;
	cw_nxo_t	*exec, *onxo, *enxo, *tnxo;
	cw_nxoi_t	i, inc, limit, edepth, tdepth;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	NXO_STACK_GET(exec, ostack, a_thread);

	NXO_STACK_DOWN_GET(onxo, ostack, a_thread, exec);
	if (nxo_type_get(onxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	limit = nxo_integer_get(onxo);

	NXO_STACK_DOWN_GET(onxo, ostack, a_thread, onxo);
	if (nxo_type_get(onxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	inc = nxo_integer_get(onxo);

	NXO_STACK_DOWN_GET(onxo, ostack, a_thread, onxo);
	if (nxo_type_get(onxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	i = nxo_integer_get(onxo);

	/* Move the object to be executed to tstack. */
	tnxo = nxo_stack_push(tstack);
	nxo_dup(tnxo, exec);
	nxo_stack_npop(ostack, 4);

	/* Record stack depths so that we can clean up if necessary. */
	edepth = nxo_stack_count(estack);
	tdepth = nxo_stack_count(tstack);

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
				enxo = nxo_stack_push(estack);
				nxo_dup(enxo, tnxo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				onxo = nxo_stack_push(ostack);
				nxo_integer_new(onxo, i);

				nxo_thread_loop(a_thread);
			}
		} else {
			for (; i >= limit; i += inc) {
				/*
				 * Dup the object to execute onto the execution
				 * stack.
				 */
				enxo = nxo_stack_push(estack);
				nxo_dup(enxo, tnxo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				onxo = nxo_stack_push(ostack);
				nxo_integer_new(onxo, i);

				nxo_thread_loop(a_thread);
			}
		}
	}
	xep_catch(_CW_ONYXX_EXIT) {
		xep_handled();

		/* Clean up stacks. */
		nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
		nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
	}
	xep_end();

	/*
	 * An object is pushed before tdepth is stored, so we can
	 * unconditionally pop it here.
	 */
	nxo_stack_pop(tstack);
}

void
systemdict_foreach(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *tstack;
	cw_nxo_t	*nxo, *what, *proc;
	cw_uint32_t	edepth, tdepth;
	cw_nxoi_t	i, count;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	NXO_STACK_GET(proc, ostack, a_thread);
	NXO_STACK_DOWN_GET(what, ostack, a_thread, proc);
	switch (nxo_type_get(what)) {
	case NXOT_ARRAY:
	case NXOT_DICT:
	case NXOT_STACK:
	case NXOT_STRING:
		break;
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/* Record stack depths so that we can clean up if necessary. */
	edepth = nxo_stack_count(estack);
	tdepth = nxo_stack_count(tstack);

	xep_begin();
	xep_try {
		switch (nxo_type_get(what)) {
		case NXOT_ARRAY: {
			cw_nxo_t	*el;

			/* Move proc and array to tstack. */
			nxo = nxo_stack_push(tstack);
			nxo_dup(nxo, proc);
			proc = nxo;

			nxo = nxo_stack_push(tstack);
			nxo_dup(nxo, what);
			what = nxo;

			nxo_stack_npop(ostack, 2);

			/*
			 * Iterate through the array, push each element onto
			 * ostack, and execute proc.
			 */
			el = nxo_stack_push(tstack);
			for (i = 0, count = nxo_array_len_get(what); i <
				 count; i++) {
				nxo_array_el_get(what, i, el);
				nxo = nxo_stack_push(ostack);
				nxo_dup(nxo, el);

				nxo = nxo_stack_push(estack);
				nxo_dup(nxo, proc);
				nxo_thread_loop(a_thread);
			}
			break;
		}
		case NXOT_DICT: {
			cw_nxo_t	*key, *val;

			/* Move proc and dict to tstack. */
			nxo = nxo_stack_push(tstack);
			nxo_dup(nxo, proc);
			proc = nxo;

			nxo = nxo_stack_push(tstack);
			nxo_dup(nxo, what);
			what = nxo;

			nxo_stack_npop(ostack, 2);

			for (i = 0, count = nxo_dict_count(what); i <
				 count; i++) {
				/* Push key and val onto ostack. */
				key = nxo_stack_push(ostack);
				val = nxo_stack_push(ostack);

				/* Get next key. */
				nxo_dict_iterate(what, key);

				/* Use key to get val. */
				nxo_dict_lookup(what, key, val);

				/* Push proc onto estack and execute it. */
				nxo = nxo_stack_push(estack);
				nxo_dup(nxo, proc);
				nxo_thread_loop(a_thread);
			}
			break;
		}
		case NXOT_STACK: {
			cw_nxo_t	*el;

			/* Move proc and stack to tstack. */
			nxo = nxo_stack_push(tstack);
			nxo_dup(nxo, proc);
			proc = nxo;

			nxo = nxo_stack_push(tstack);
			nxo_dup(nxo, what);
			what = nxo;

			nxo_stack_npop(ostack, 2);

			/*
			 * Iterate through the stack, push each element onto
			 * ostack, and execute proc.
			 */
			for (i = 0, count = nxo_stack_count(what), el = NULL; i
			    < count; i++) {
				el = nxo_stack_down_get(what, el);

				nxo = nxo_stack_push(ostack);
				nxo_dup(nxo, el);

				nxo = nxo_stack_push(estack);
				nxo_dup(nxo, proc);
				nxo_thread_loop(a_thread);
			}
			break;
		}
		case NXOT_STRING: {
			cw_uint8_t	el;

			/* Move proc and array to tstack. */
			nxo = nxo_stack_push(tstack);
			nxo_dup(nxo, proc);
			proc = nxo;

			nxo = nxo_stack_push(tstack);
			nxo_dup(nxo, what);
			what = nxo;

			nxo_stack_npop(ostack, 2);

			/*
			 * Iterate through the string, push each element onto
			 * ostack, and execute proc.
			 */
			for (i = 0, count = nxo_array_len_get(what); i < count;
			    i++) {
				nxo_string_el_get(what, i, &el);
				nxo = nxo_stack_push(ostack);
				nxo_integer_new(nxo, (cw_nxoi_t)el);

				nxo = nxo_stack_push(estack);
				nxo_dup(nxo, proc);
				nxo_thread_loop(a_thread);
			}
			break;
		}
		default:
			_cw_not_reached();
		}
	}
	xep_catch(_CW_ONYXX_EXIT) {
		xep_handled();

		/*
		 * Clean up estack.  tstack is handled later, so don't bother
		 * cleaning it up here.
		 */
		nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	}
	xep_end();

	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
}

void
systemdict_fork(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	pid_t		pid;

	pid = fork();
	if (pid == -1) {
		/* Error, related to some form of resource exhaustion. */
		nxo_thread_error(a_thread, NXO_THREADE_LIMITCHECK);
		return;
	}

	ostack = nxo_thread_ostack_get(a_thread);
	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, pid);
}

void
systemdict_ge(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;
	cw_nxot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	ge;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	type_a = nxo_type_get(nxo_a);
	type_b = nxo_type_get(nxo_b);
	if (type_a != type_b || (type_a != NXOT_INTEGER && type_a !=
	    NXOT_STRING) || (type_b != NXOT_INTEGER && type_b != NXOT_STRING)) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	result = nxo_compare(nxo_a, nxo_b);
	if (result >= 0)
		ge = TRUE;
	else
		ge = FALSE;

	nxo_boolean_new(nxo_a, ge);

	nxo_stack_pop(ostack);
}

void
systemdict_get(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*from, *with;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(with, ostack, a_thread);
	NXO_STACK_DOWN_GET(from, ostack, a_thread, with);

	switch (nxo_type_get(from)) {
	case NXOT_ARRAY: {
		cw_nxoi_t	index;

		if (nxo_type_get(with) != NXOT_INTEGER) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}
		index = nxo_integer_get(with);

		if (index >= nxo_array_len_get(from)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}
		nxo_array_el_get(from, index, with);

		nxo_stack_roll(ostack, 2, 1);
		nxo_stack_pop(ostack);
		break;
	}
	case NXOT_DICT: {
		cw_nxo_t	*val;

		val = nxo_stack_push(ostack);
		if (nxo_dict_lookup(from, with, val)) {
			nxo_stack_pop(ostack);
			nxo_thread_error(a_thread, NXO_THREADE_UNDEFINED);
			return;
		}
		nxo_stack_roll(ostack, 3, 1);
		nxo_stack_npop(ostack, 2);
		break;
	}
	case NXOT_STRING: {
		cw_nxoi_t	index;
		cw_uint8_t	el;

		if (nxo_type_get(with) != NXOT_INTEGER) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}
		index = nxo_integer_get(with);

		if (index < 0 || index >= nxo_string_len_get(from)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}
		nxo_string_el_get(from, index, &el);
		nxo_integer_set(with, (cw_nxoi_t)el);

		nxo_stack_roll(ostack, 2, 1);
		nxo_stack_pop(ostack);
		break;
	}
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
}

void
systemdict_getinterval(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*from, *with, *count;
	cw_nxoi_t	index, len;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(count, ostack, a_thread);
	NXO_STACK_DOWN_GET(with, ostack, a_thread, count);
	NXO_STACK_DOWN_GET(from, ostack, a_thread, with);

	if ((nxo_type_get(with) != NXOT_INTEGER) || (nxo_type_get(count) !=
	    NXOT_INTEGER)) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	index = nxo_integer_get(with);
	len = nxo_integer_get(count);

	switch (nxo_type_get(from)) {
	case NXOT_ARRAY:
		if (index + len > nxo_array_len_get(from)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}
		nxo_array_subarray_new(count, from,
		    nxo_thread_nx_get(a_thread), index, len);
		break;
	case NXOT_STRING:
		if (index + len > nxo_string_len_get(from)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}
		nxo_string_substring_new(count, from,
		    nxo_thread_nx_get(a_thread), index, len);
		break;
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_stack_roll(ostack, 3, 1);
	nxo_stack_npop(ostack, 2);
}

void
systemdict_gt(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;
	cw_nxot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	gt;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	type_a = nxo_type_get(nxo_a);
	type_b = nxo_type_get(nxo_b);
	if (type_a != type_b || (type_a != NXOT_INTEGER && type_a !=
	    NXOT_STRING) || (type_b != NXOT_INTEGER && type_b !=
	    NXOT_STRING)) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	result = nxo_compare(nxo_a, nxo_b);
	if (result == 1)
		gt = TRUE;
	else
		gt = FALSE;

	nxo_boolean_new(nxo_a, gt);

	nxo_stack_pop(ostack);
}

void
systemdict_handleerror(cw_nxo_t *a_thread)
{
	cw_nxo_t	*estack, *tstack;
	cw_nxo_t	*key, *errordict, *handleerror;

	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	/* Get errordict. */
	errordict = nxo_stack_push(tstack);
	key = nxo_stack_push(tstack);
	nxo_name_new(key, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_errordict), nxn_len(NXN_errordict), TRUE);
	if (nxo_thread_dstack_search(a_thread, key, errordict)) {
		/*
		 * Fall back to the errordict defined during thread creation,
		 * since the alternative is to blow up (or potentially go
		 * infinitely recursive).
		 */
		nxo_dup(errordict, nxo_thread_errordict_get(a_thread));
	}

	/* Get handleerror from errordict and push it onto estack. */
	handleerror = nxo_stack_push(estack);
	nxo_name_new(key, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_handleerror), nxn_len(NXN_handleerror), TRUE);
	if (nxo_dict_lookup(errordict, key, handleerror)) {
		/*
		 * Do not execute an error handler, since the alternative is to
		 * blow up (or potentially go infinitely recursive).
		 */
		nxo_stack_pop(estack);
		nxo_stack_npop(tstack, 2);
		return;
	}
	nxo_stack_npop(tstack, 2);

	/* Execute handleerror. */
	nxo_thread_loop(a_thread);
}

void
systemdict_if(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*cond, *exec;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(exec, ostack, a_thread);
	NXO_STACK_DOWN_GET(cond, ostack, a_thread, exec);
	if (nxo_type_get(cond) != NXOT_BOOLEAN) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	if (nxo_boolean_get(cond)) {
		cw_nxo_t	*estack;
		cw_nxo_t	*nxo;

		estack = nxo_thread_estack_get(a_thread);
		nxo = nxo_stack_push(estack);
		nxo_dup(nxo, exec);
		nxo_stack_npop(ostack, 2);
		nxo_thread_loop(a_thread);
	} else
		nxo_stack_npop(ostack, 2);
}

void
systemdict_ifelse(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack;
	cw_nxo_t	*cond, *exec_if, *exec_else, *nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);

	NXO_STACK_GET(exec_else, ostack, a_thread);

	NXO_STACK_DOWN_GET(exec_if, ostack, a_thread, exec_else);

	NXO_STACK_DOWN_GET(cond, ostack, a_thread, exec_if);
	if (nxo_type_get(cond) != NXOT_BOOLEAN) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo = nxo_stack_push(estack);
	if (nxo_boolean_get(cond))
		nxo_dup(nxo, exec_if);
	else
		nxo_dup(nxo, exec_else);
	nxo_stack_npop(ostack, 3);
	nxo_thread_loop(a_thread);
}

void
systemdict_index(cw_nxo_t *a_thread)
{
	systemdict_inline_index(a_thread);
}

void
systemdict_join(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*thread;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(thread, ostack, a_thread);
	if (nxo_type_get(thread) != NXOT_THREAD) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_thread_join(thread);

	nxo_stack_pop(ostack);
}

void
systemdict_known(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*dict, *key;
	cw_bool_t	known;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(key, ostack, a_thread);
	NXO_STACK_DOWN_GET(dict, ostack, a_thread, key);
	if (nxo_type_get(dict) != NXOT_DICT) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	known = !nxo_dict_lookup(dict, key, NULL);
	nxo_boolean_new(dict, known);

	nxo_stack_pop(ostack);
}

void
systemdict_lcheck(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_bool_t	locking;
	
	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);

	switch (nxo_type_get(nxo)) {
	case NXOT_BOOLEAN:
	case NXOT_CONDITION:
	case NXOT_HOOK:
	case NXOT_INTEGER:
	case NXOT_MARK:
	case NXOT_MUTEX:
	case NXOT_NAME:
	case NXOT_NULL:
	case NXOT_OPERATOR:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	case NXOT_ARRAY:
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_STRING:
		locking = nxo_lcheck(nxo);
		break;
	case NXOT_NO:
	default:
		_cw_not_reached();
	}
	nxo_boolean_new(nxo, locking);
}

void
systemdict_le(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;
	cw_nxot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	le;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	type_a = nxo_type_get(nxo_a);
	type_b = nxo_type_get(nxo_b);
	if (type_a != type_b || (type_a != NXOT_INTEGER && type_a !=
	    NXOT_STRING) || (type_b != NXOT_INTEGER && type_b != NXOT_STRING)) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	result = nxo_compare(nxo_a, nxo_b);
	if (result <= 0)
		le = TRUE;
	else
		le = FALSE;

	nxo_boolean_new(nxo_a, le);

	nxo_stack_pop(ostack);
}

void
systemdict_length(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_nxoi_t	len;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(nxo, ostack, a_thread);
	switch (nxo_type_get(nxo)) {
	case NXOT_ARRAY:
		len = nxo_array_len_get(nxo);
		break;
	case NXOT_DICT:
		len = nxo_dict_count(nxo);
		break;
	case NXOT_NAME:
		len = nxo_name_len_get(nxo);
		break;
	case NXOT_STRING:
		len = nxo_string_len_get(nxo);
		break;
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_integer_new(nxo, len);
}

void
systemdict_link(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*filename, *linkname, *tfilename, *tlinkname;
	int		error;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(linkname, ostack, a_thread);
	NXO_STACK_DOWN_GET(filename, ostack, a_thread, linkname);
	if (nxo_type_get(filename) != NXOT_STRING ||
	    nxo_type_get(linkname) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/*
	 * Create a copy of filename with an extra byte to store a '\0'
	 * terminator.
	 */
	tfilename = nxo_stack_push(tstack);
	nxo_string_new(tfilename, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(filename) +
	    1);
	
	nxo_string_lock(filename);
	nxo_string_lock(tfilename);
	nxo_string_set(tfilename, 0, nxo_string_get(filename),
	    nxo_string_len_get(filename));
	nxo_string_el_set(tfilename, '\0', nxo_string_len_get(tfilename) - 1);
	nxo_string_unlock(filename);

	/*
	 * Create a copy of linkname with an extra byte to store a '\0'
	 * terminator.
	 */
	tlinkname = nxo_stack_push(tstack);
	nxo_string_new(tlinkname, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(linkname) +
	    1);
	nxo_string_lock(linkname);
	nxo_string_lock(tlinkname);
	nxo_string_set(tlinkname, 0, nxo_string_get(linkname),
	    nxo_string_len_get(linkname));
	nxo_string_el_set(tlinkname, '\0', nxo_string_len_get(tlinkname) - 1);
	nxo_string_unlock(linkname);

	error = link(nxo_string_get(tfilename), nxo_string_get(tlinkname));
	nxo_stack_npop(tstack, 2);
	if (error == -1) {
		switch (errno) {
		case EIO:
		case EDQUOT:
		case EMLINK:
		case ENOSPC:
		case EROFS:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		case EEXIST:
		case ENOENT:
		case ENOTDIR:
		case EOPNOTSUPP:
			nxo_thread_error(a_thread,
			    NXO_THREADE_UNDEFINEDFILENAME);
			break;
		case EACCES:
		case ENAMETOOLONG:
		case ELOOP:
		case EPERM:
		case EXDEV:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			break;
		case EFAULT:
		default:
			nxo_thread_error(a_thread, NXO_THREADE_UNREGISTERED);
		}
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_load(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*key, *val;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	NXO_STACK_GET(key, ostack, a_thread);
	val = nxo_stack_push(tstack);

	if (nxo_thread_dstack_search(a_thread, key, val)) {
		nxo_thread_error(a_thread, NXO_THREADE_UNDEFINED);
		return;
	}
	nxo_dup(key, val);
	nxo_stack_pop(tstack);
}

void
systemdict_lock(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*mutex;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(mutex, ostack, a_thread);
	if (nxo_type_get(mutex) != NXOT_MUTEX) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_mutex_lock(mutex);

	nxo_stack_pop(ostack);
}

void
systemdict_loop(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *tstack;
	cw_nxo_t	*exec, *nxo, *tnxo;
	cw_uint32_t	edepth, tdepth;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	NXO_STACK_GET(exec, ostack, a_thread);

	/* Move the object to be executed to tstack. */
	tnxo = nxo_stack_push(tstack);
	nxo_dup(tnxo, exec);
	nxo_stack_pop(ostack);

	/* Record stack depths so that we can clean up later. */
	edepth = nxo_stack_count(estack);
	tdepth = nxo_stack_count(tstack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		for (;;) {
			nxo = nxo_stack_push(estack);
			nxo_dup(nxo, tnxo);
			nxo_thread_loop(a_thread);
		}
	}
	xep_catch(_CW_ONYXX_EXIT) {
		xep_handled();
	}
	xep_end();

	/* Clean up stacks. */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
}

void
systemdict_lt(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;
	cw_nxot_t	type_a, type_b;
	cw_sint32_t	result;
	cw_bool_t	lt;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	type_a = nxo_type_get(nxo_a);
	type_b = nxo_type_get(nxo_b);
	if (type_a != type_b || (type_a != NXOT_INTEGER && type_a !=
	    NXOT_STRING) || (type_b != NXOT_INTEGER && type_b != NXOT_STRING)) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	result = nxo_compare(nxo_a, nxo_b);
	if (result == -1)
		lt = TRUE;
	else
		lt = FALSE;

	nxo_boolean_new(nxo_a, lt);

	nxo_stack_pop(ostack);
}

void
systemdict_mark(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	nxo = nxo_stack_push(ostack);
	nxo_mark_new(nxo);
}

void
systemdict_mkdir(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack, *path, *mode, *tpath;
	int		error;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(mode, ostack, a_thread);
	NXO_STACK_DOWN_GET(path, ostack, a_thread, mode);
	if ((nxo_type_get(mode) != NXOT_INTEGER) || nxo_type_get(path) !=
	    NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(mode) < 0 || nxo_integer_get(mode) > 0xfff) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	/*
	 * Create a copy of path with an extra byte to store a '\0' terminator.
	 */
	tpath = nxo_stack_push(tstack);
	nxo_string_new(tpath, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(path) + 1);
	nxo_string_lock(path);
	nxo_string_lock(tpath);
	nxo_string_set(tpath, 0, nxo_string_get(path),
	    nxo_string_len_get(path));
	nxo_string_el_set(tpath, '\0', nxo_string_len_get(tpath) - 1);
	nxo_string_unlock(path);

	error = mkdir(nxo_string_get(tpath), nxo_integer_get(mode));

	nxo_string_unlock(tpath);
	nxo_stack_pop(tstack);

	if (error == -1) {
		switch (errno) {
		case EIO:
		case EDQUOT:
		case ENOSPC:
		case EROFS:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		case EACCES:
		case EEXIST:
		case ELOOP:
		case ENOENT:
		case ENOTDIR:
		case ENAMETOOLONG:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			break;
		case EFAULT:
		default:
			nxo_thread_error(a_thread, NXO_THREADE_UNREGISTERED);
		}
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_mod(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*a, *b;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(b, ostack, a_thread);
	NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (nxo_type_get(a) != NXOT_INTEGER || nxo_type_get(b) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(b) == 0) {
		nxo_thread_error(a_thread, NXO_THREADE_UNDEFINEDRESULT);
		return;
	}

	nxo_integer_set(a, nxo_integer_get(a) % nxo_integer_get(b));
	nxo_stack_pop(ostack);
}

void
systemdict_mul(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*a, *b;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(b, ostack, a_thread);
	NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (nxo_type_get(a) != NXOT_INTEGER || nxo_type_get(b) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_integer_set(a, nxo_integer_get(a) * nxo_integer_get(b));
	nxo_stack_pop(ostack);
}

void
systemdict_mutex(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*mutex;

	ostack = nxo_thread_ostack_get(a_thread);
	mutex = nxo_stack_push(ostack);
	nxo_mutex_new(mutex, nxo_thread_nx_get(a_thread));
}

void
systemdict_ne(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;
	cw_sint32_t	result;
	cw_bool_t	ne;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	result = nxo_compare(nxo_a, nxo_b);
	if (result == 0)
		ne = FALSE;
	else
		ne = TRUE;

	nxo_boolean_new(nxo_a, ne);

	nxo_stack_pop(ostack);
}

void
systemdict_neg(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*a;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(a, ostack, a_thread);
	if (nxo_type_get(a) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_integer_set(a, -nxo_integer_get(a));
}

void
systemdict_not(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo, ostack, a_thread);

	if (nxo_type_get(nxo) == NXOT_BOOLEAN)
		nxo_boolean_set(nxo, !nxo_boolean_get(nxo));
	else if (nxo_type_get(nxo) == NXOT_INTEGER)
		nxo_integer_set(nxo, ~nxo_integer_get(nxo));
	else
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
}

void
systemdict_nsleep(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	int		error;
	struct timespec sleeptime, remainder;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo, ostack, a_thread);

	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(nxo) <= 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	sleeptime.tv_sec = nxo_integer_get(nxo) / 1000000000LL;
	sleeptime.tv_nsec = nxo_integer_get(nxo) % 1000000000LL;

	for (;;) {
		error = nanosleep(&sleeptime, &remainder);
		if (error == 0) {
			/* We've slept the entire time. */
			break;
		}
		/* A signal interrupted us.  Sleep some more. */
		memcpy(&sleeptime, &remainder, sizeof(struct timespec));
	}
	nxo_stack_pop(ostack);
}

void
systemdict_open(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *tstack;
	cw_nxo_t		*name, *flags, *file;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	
	NXO_STACK_GET(flags, ostack, a_thread);
	NXO_STACK_DOWN_GET(name, ostack, a_thread, flags);
	if (nxo_type_get(name) != NXOT_STRING || nxo_type_get(flags) !=
	    NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	file = nxo_stack_push(tstack);
	nxo_file_new(file, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread));
	nxo_string_lock(name);
	error = nxo_file_open(file, nxo_string_get(name),
	    nxo_string_len_get(name), nxo_string_get(flags),
	    nxo_string_len_get(flags));
	nxo_string_unlock(name);
	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_file_buffer_size_set(file, _LIBONYX_FILE_BUFFER_SIZE);

	nxo_stack_pop(ostack);
	nxo_dup(name, file);
	nxo_stack_pop(tstack);
}

void
systemdict_or(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	if (nxo_type_get(nxo_a) == NXOT_BOOLEAN && nxo_type_get(nxo_b) ==
	    NXOT_BOOLEAN) {
		cw_bool_t	or;

		if (nxo_boolean_get(nxo_a) || nxo_boolean_get(nxo_b))
			or = TRUE;
		else
			or = FALSE;
		nxo_boolean_new(nxo_a, or);
	} else if (nxo_type_get(nxo_a) == NXOT_INTEGER &&
	    nxo_type_get(nxo_b) == NXOT_INTEGER) {
		nxo_integer_set(nxo_a, nxo_integer_get(nxo_a) |
		    nxo_integer_get(nxo_b));
	} else {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_stack_pop(ostack);
}

void
systemdict_ostack(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack;

	ostack = nxo_thread_ostack_get(a_thread);
	stack = nxo_stack_push(ostack);
	nxo_stack_new(stack, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread));
	nxo_stack_copy(stack, ostack);

	/*
	 * Pop the top element off the stack, since it's a reference to the
	 * stack itself.
	 */
	nxo_stack_pop(stack);
}

void
systemdict_pop(cw_nxo_t *a_thread)
{
	systemdict_inline_pop(a_thread);
}

void
systemdict_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack;
	cw_nxo_t		*nxo, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	stdout_nxo = nx_stdout_get(nxo_thread_nx_get(a_thread));

	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_string_lock(nxo);
	error = nxo_file_write(stdout_nxo, nxo_string_get(nxo),
	    nxo_string_len_get(nxo));
	nxo_string_unlock(nxo);
	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_pop(ostack);
}

void
systemdict_put(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*into, *with, *what;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(what, ostack, a_thread);
	NXO_STACK_DOWN_GET(with, ostack, a_thread, what);
	NXO_STACK_DOWN_GET(into, ostack, a_thread, with);

	switch (nxo_type_get(into)) {
	case NXOT_ARRAY: {
		cw_nxoi_t	index;

		if (nxo_type_get(with) != NXOT_INTEGER) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}
		index = nxo_integer_get(with);

		if (index >= nxo_array_len_get(into)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}
		nxo_array_el_set(into, what, index);
		break;
	}
	case NXOT_DICT:
		nxo_dict_def(into, nxo_thread_nx_get(a_thread), with, what);
		break;
	case NXOT_STRING: {
		cw_nxoi_t	index;
		cw_uint8_t	val;

		if ((nxo_type_get(with) != NXOT_INTEGER) ||
		    nxo_type_get(what) != NXOT_INTEGER) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}
		index = nxo_integer_get(with);
		val = nxo_integer_get(what);

		if (index >= nxo_string_len_get(into)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}
		nxo_string_el_set(into, val, index);
		break;
	}
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	nxo_stack_npop(ostack, 3);
}

void
systemdict_putinterval(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*into, *with, *what;
	cw_nxoi_t	index;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(what, ostack, a_thread);
	NXO_STACK_DOWN_GET(with, ostack, a_thread, what);
	NXO_STACK_DOWN_GET(into, ostack, a_thread, with);

	if (nxo_type_get(with) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	index = nxo_integer_get(with);

	switch (nxo_type_get(into)) {
	case NXOT_ARRAY: {
		cw_nxo_t	*tstack;
		cw_nxo_t	*el;
		cw_uint32_t	i, len;

		tstack = nxo_thread_tstack_get(a_thread);
		el = nxo_stack_push(tstack);
		len = nxo_array_len_get(what);
		if (index + len > nxo_array_len_get(into)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			nxo_stack_pop(tstack);
			return;
		}
		for (i = 0; i < len; i++) {
			nxo_array_el_get(what, i, el);
			nxo_array_el_set(into, el, i + index);
		}
		nxo_stack_pop(tstack);
		break;
	}
	case NXOT_STRING: {
		cw_uint8_t	*str;
		cw_uint32_t	len;

		str = nxo_string_get(what);
		len = nxo_string_len_get(what);
		if (index + len > nxo_string_len_get(into)) {
			nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
			return;
		}

		nxo_string_lock(what);
		nxo_string_lock(into);
		nxo_string_set(into, index, str, len);
		nxo_string_unlock(into);
		nxo_string_unlock(what);
		break;
	}
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	nxo_stack_npop(ostack, 3);
}

void
systemdict_pwd(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *nxo;
	char		*str;

	str = getcwd(NULL, 0);
	if (str == NULL) {
		nxo_thread_error(a_thread, NXO_THREADE_INVALIDACCESS);
		return;
	}

	ostack = nxo_thread_ostack_get(a_thread);
	nxo = nxo_stack_push(ostack);

	nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), strlen(str));
	nxo_string_lock(nxo);
	nxo_string_set(nxo, 0, str, nxo_string_len_get(nxo));
	nxo_string_unlock(nxo);

	free(str);
}

void
systemdict_quit(cw_nxo_t *a_thread)
{
	xep_throw(_CW_ONYXX_QUIT);
}

void
systemdict_rand(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*num;

	ostack = nxo_thread_ostack_get(a_thread);

	num = nxo_stack_push(ostack);
	nxo_integer_new(num, random());
}

void
systemdict_read(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*file;
	cw_uint8_t	val;
	cw_sint32_t	nread;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(file, ostack, a_thread);
	switch (nxo_type_get(file)) {
	case NXOT_FILE: {
		cw_nxo_t	*value, *code;

		/* Character read. */
		value = nxo_stack_push(ostack);
		code = nxo_stack_push(ostack);
		
		nread = nxo_file_read(file, 1, &val);
		if (nread == -1) {
			nxo_stack_npop(ostack, 2);
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			return;
		}

		if (nread == 0) {
			nxo_integer_new(value, 0);
			nxo_boolean_new(code, FALSE);
		} else {
			nxo_integer_new(value, (cw_nxoi_t)val);
			nxo_boolean_new(code, TRUE);
		}

		nxo_stack_roll(ostack, 3, 2);
		nxo_stack_pop(ostack);
		break;
	}
	case NXOT_STRING: {
		cw_nxo_t	*string;

		/* String read. */
		string = file;
		NXO_STACK_DOWN_GET(file, ostack, a_thread, string);
		if (nxo_type_get(file) != NXOT_FILE) {
			nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
			return;
		}

		nxo_string_lock(string);
		nread = nxo_file_read(file, nxo_string_len_get(string),
		    nxo_string_get(string));
		nxo_string_unlock(string);
		if (nread == -1) {
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			return;
		}

		if (nread == 0) {
			/* EOF. */
			nxo_boolean_new(file, TRUE);
			nxo_string_new(string, nxo_thread_nx_get(a_thread),
			    nxo_thread_currentlocking(a_thread), 0);
			nxo_stack_roll(ostack, 2, 1);
		} else if (nread < nxo_string_len_get(string)) {
			cw_nxo_t	*value, *code;

			/*
			 * We didn't fill the string, so we can't just use it as
			 * the result.  Create a copy.
			 */
			value = nxo_stack_under_push(ostack, file);
			nxo_string_substring_new(value, string,
			    nxo_thread_nx_get(a_thread), 0, nread);
			code = nxo_stack_under_push(ostack, file);
			nxo_boolean_new(code, FALSE);

			nxo_stack_npop(ostack, 2);
		} else {
			/*
			 * The string is full, so doesn't need modified.
			 */
			nxo_boolean_new(file, FALSE);
			nxo_stack_roll(ostack, 2, 1);
		}
		break;
	}
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
}

void
systemdict_readline(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *tstack;
	cw_nxo_t		*nxo, *tfile;
	cw_nxo_threade_t	error;
	cw_bool_t		eof;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_FILE) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	tfile = nxo_stack_push(tstack);
	nxo_dup(tfile, nxo);
	error = nxo_file_readline(tfile, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo, &eof);
	if (error) {
		nxo_stack_pop(tstack);
		nxo_thread_error(a_thread, error);
		return;
	}
	nxo_stack_pop(tstack);

	nxo = nxo_stack_push(ostack);
	nxo_boolean_new(nxo, eof);
}

void
systemdict_realtime(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	struct timeval	tv;

	ostack = nxo_thread_ostack_get(a_thread);
	nxo = nxo_stack_push(ostack);

	gettimeofday(&tv, NULL);
	nxo_integer_new(nxo, (((cw_nxoi_t)tv.tv_sec * (cw_nxoi_t)1000000000) +
	    (cw_nxoi_t)tv.tv_usec * (cw_nxoi_t)1000));
}

void
systemdict_renamefile(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*string_from, *string_to;
	cw_uint8_t	str_from[PATH_MAX], str_to[1024];
	cw_uint32_t	nbytes;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(string_to, ostack, a_thread);
	NXO_STACK_DOWN_GET(string_from, ostack, a_thread, string_to);

	if (nxo_type_get(string_from) != NXOT_STRING ||
	    nxo_type_get(string_to) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	if (nxo_string_len_get(string_from) >= sizeof(str_from)) {
		nxo_thread_error(a_thread, NXO_THREADE_LIMITCHECK);
		return;
	}
	nbytes = nxo_string_len_get(string_from);
	nxo_string_lock(string_from);
	memcpy(str_from, nxo_string_get(string_from), nbytes);
	nxo_string_unlock(string_from);
	str_from[nbytes] = '\0';

	if (nxo_string_len_get(string_to) >= sizeof(str_to)) {
		nxo_thread_error(a_thread, NXO_THREADE_LIMITCHECK);
		return;
	}
	nbytes = nxo_string_len_get(string_to);
	nxo_string_lock(string_to);
	memcpy(str_to, nxo_string_get(string_to), nbytes);
	nxo_string_unlock(string_to);
	str_to[nbytes] = '\0';

	if (rename(str_from, str_to) == -1) {
		switch (errno) {
		case EACCES:
		case EPERM:
		case EROFS:
		case EINVAL:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			nxo_thread_error(a_thread,
			    NXO_THREADE_UNDEFINEDFILENAME);
		default:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
		}
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_repeat(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *tstack;
	cw_nxo_t	*count, *exec, *nxo, *tnxo;
	cw_nxoi_t	i, cnt;
	cw_uint32_t	edepth, tdepth;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	NXO_STACK_GET(exec, ostack, a_thread);
	NXO_STACK_DOWN_GET(count, ostack, a_thread, exec);
	if (nxo_type_get(count) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	tnxo = nxo_stack_push(tstack);
	nxo_dup(tnxo, exec);

	cnt = nxo_integer_get(count);
	nxo_stack_npop(ostack, 2);

	/* Record stack depths so that we can clean up if necessary. */
	edepth = nxo_stack_count(estack);
	tdepth = nxo_stack_count(tstack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		for (i = 0; i < cnt; i++) {
			nxo = nxo_stack_push(estack);
			nxo_dup(nxo, tnxo);
			nxo_thread_loop(a_thread);
		}
	}
	xep_catch(_CW_ONYXX_EXIT) {
		xep_handled();

		/* Clean up stacks. */
		nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
		nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
	}
	xep_end();

	/*
	 * An object is pushed before tdepth is stored, so we can
	 * unconditionally pop it here.
	 */
	nxo_stack_pop(tstack);
}

void
systemdict_rmdir(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack, *path, *tpath;
	int		error;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(path, ostack, a_thread);
	if (nxo_type_get(path) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/*
	 * Create a copy of path with an extra byte to store a '\0' terminator.
	 */
	tpath = nxo_stack_push(tstack);
	nxo_string_new(tpath, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(path) + 1);
	nxo_string_lock(path);
	nxo_string_lock(tpath);
	nxo_string_set(tpath, 0, nxo_string_get(path),
	    nxo_string_len_get(path));
	nxo_string_el_set(tpath, '\0', nxo_string_len_get(tpath) - 1);
	nxo_string_unlock(path);

	error = rmdir(nxo_string_get(tpath));

	nxo_string_unlock(tpath);
	nxo_stack_pop(tstack);

	if (error == -1) {
		switch (errno) {
		case EBUSY:
		case EIO:
		case ENOTEMPTY:
		case EROFS:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		case EACCES:
		case ELOOP:
		case ENOENT:
		case ENOTDIR:
		case ENAMETOOLONG:
		case EPERM:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			break;
		case EFAULT:
		default:
			nxo_thread_error(a_thread, NXO_THREADE_UNREGISTERED);
		}
		return;
	}

	nxo_stack_pop(ostack);
}

void
systemdict_roll(cw_nxo_t *a_thread)
{
	systemdict_inline_roll(a_thread);
}

void
systemdict_run(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *estack, *tstack;
	cw_nxo_t		*nxo, *tfile;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	tfile = nxo_stack_push(tstack);
	nxo_file_new(tfile, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread));
	nxo_string_lock(nxo);
	error = nxo_file_open(tfile, nxo_string_get(nxo),
	    nxo_string_len_get(nxo), "r", 1);
	nxo_string_unlock(nxo);
	if (error) {
		nxo_stack_pop(tstack);
		nxo_thread_error(a_thread, error);
		return;
	}
	nxo_stack_pop(ostack);
	nxo_attrs_set(tfile, NXOA_EXECUTABLE);
	nxo = nxo_stack_push(estack);

	nxo_dup(nxo, tfile);

	xep_begin();
	xep_try {
		nxo_thread_loop(a_thread);
	}
	xep_catch(_CW_ONYXX_EXIT) {
		nxo_thread_error(a_thread, NXO_THREADE_INVALIDEXIT);

		xep_retry();
	}
	xep_catch(_CW_ONYXX_STOP)
	xep_mcatch(_CW_ONYXX_QUIT) {
		/* Close the file, but don't handle the exception. */
		error = nxo_file_close(tfile);
		if (error)
			nxo_thread_error(a_thread, error);

		/*
		 * We don't need to clean up the stack, since the exception
		 * catcher will do it.
		 */
	}
	xep_end();

	nxo_stack_pop(tstack);
}

void
systemdict_sclear(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack;
	cw_uint32_t	count;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	count = nxo_stack_count(stack);
	if (count > 0)
		nxo_stack_npop(stack, count);
}

void
systemdict_scleartomark(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack, *nxo;
	cw_uint32_t	i, depth;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	for (i = 0, depth = nxo_stack_count(stack), nxo = NULL; i < depth;
	    i++) {
		nxo = nxo_stack_down_get(stack, nxo);
		if (nxo_type_get(nxo) == NXOT_MARK)
			break;
	}
	if (i == depth) {
		nxo_thread_error(a_thread, NXO_THREADE_UNMATCHEDMARK);
		return;
	}

	nxo_stack_npop(stack, i + 1);
}

void
systemdict_scount(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack, *nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, nxo_stack_count(stack));
}

void
systemdict_scounttomark(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack, *nxo;
	cw_uint32_t	i, depth;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	for (i = 0, depth = nxo_stack_count(stack), nxo = NULL; i < depth;
	    i++) {
		nxo = nxo_stack_down_get(stack, nxo);
		if (nxo_type_get(nxo) == NXOT_MARK)
			break;
	}
	if (i == depth) {
		nxo_thread_error(a_thread, NXO_THREADE_UNMATCHEDMARK);
		return;
	}

	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, i);
}

void
systemdict_sdup(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack, *orig, *dup;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	NXO_STACK_GET(orig, stack, a_thread);
	dup = nxo_stack_push(stack);
	nxo_dup(dup, orig);
}

void
systemdict_self(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*thread;

	ostack = nxo_thread_ostack_get(a_thread);
	thread = nxo_stack_push(ostack);

	nxo_thread_self(a_thread, thread);
}

void
systemdict_seek(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack;
	cw_nxo_t		*file, *position;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(position, ostack, a_thread);
	NXO_STACK_DOWN_GET(file, ostack, a_thread, position);
	
	if (nxo_type_get(file) != NXOT_FILE || nxo_type_get(position) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	error = nxo_file_position_set(file, nxo_integer_get(position));
	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_setlocking(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_BOOLEAN) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	nxo_thread_setlocking(a_thread, nxo_boolean_get(nxo));
	nxo_stack_pop(ostack);
}

void
systemdict_sexch(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_stack_count(stack) < 2) {
		nxo_thread_error(a_thread, NXO_THREADE_STACKUNDERFLOW);
		return;
	}

	nxo_stack_roll(stack, 2, 1);
}

void
systemdict_shift(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*integer, *shift;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(shift, ostack, a_thread);
	NXO_STACK_DOWN_GET(integer, ostack, a_thread, shift);

	if (nxo_type_get(integer) != NXOT_INTEGER || nxo_type_get(shift) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	if (nxo_integer_get(shift) > 0) {
		nxo_integer_set(integer, nxo_integer_get(integer) <<
		    nxo_integer_get(shift));
	} else if (nxo_integer_get(shift) < 0) {
		nxo_integer_set(integer, nxo_integer_get(integer) >>
		    -nxo_integer_get(shift));
	}

	nxo_stack_pop(ostack);
}

void
systemdict_signal(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*condition;
	
	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(condition, ostack, a_thread);
	if (nxo_type_get(condition) != NXOT_CONDITION) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_condition_signal(condition);

	nxo_stack_pop(ostack);
}

void
systemdict_sindex(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *nxo, *stack, *orig;
	cw_nxoi_t	index;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
	if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) !=
	    NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	index = nxo_integer_get(nxo);
	if (index < 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	NXO_STACK_NGET(orig, stack, a_thread, index);
	nxo = nxo_stack_push(stack);
	nxo_dup(nxo, orig);

	nxo_stack_pop(ostack);
}

void
systemdict_sload(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack, *stack, *ttstack;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	ttstack = nxo_stack_push(tstack);
	nxo_dup(ttstack, stack);
	nxo_stack_pop(ostack);

	nxo_stack_copy(ostack, ttstack);

	stack = nxo_stack_push(ostack);
	nxo_dup(stack, ttstack);
	nxo_stack_pop(tstack);
}

void
systemdict_spop(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack, *snxo, *onxo;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	NXO_STACK_GET(snxo, stack, a_thread);
	onxo = nxo_stack_push(ostack);
	nxo_dup(onxo, snxo);

	NXO_STACK_POP(stack, a_thread);
}

void
systemdict_sprint(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack;
	cw_nxo_t		*nxo, *depth, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	stdout_nxo = nx_stdout_get(nxo_thread_nx_get(a_thread));

	/*
	 * depth and nxo aren't used, but accessing them causes a more
	 * intelligible error than the embedded nx code would.
	 */
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/* object depth sprint - */
	_cw_onyx_code(a_thread, "1 index type sprintdict exch get eval");
	error = nxo_file_output(stdout_nxo, "\n");
	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}
	error = nxo_file_buffer_flush(stdout_nxo);
	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}
}

void
systemdict_spush(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *nxo, *stack, *nnxo;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nnxo = nxo_stack_push(stack);
	nxo_dup(nnxo, nxo);
	nxo_stack_pop(ostack);
}

void
systemdict_srand(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*seed;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(seed, ostack, a_thread);
	srandom(nxo_integer_get(seed));
	nxo_stack_pop(ostack);
}

void
systemdict_sroll(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *stack, *nxo;
	cw_nxoi_t	count, amount;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	amount = nxo_integer_get(nxo);
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
	if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) !=
	    NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	count = nxo_integer_get(nxo);
	if (count < 1) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}
	if (count > nxo_stack_count(stack)) {
		nxo_thread_error(a_thread, NXO_THREADE_STACKUNDERFLOW);
		return;
	}

	nxo_stack_npop(ostack, 2);
	nxo_stack_roll(stack, count, amount);
}

void
systemdict_sstore(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack, *stack, *ttstack;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(stack, ostack, a_thread);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	ttstack = nxo_stack_push(tstack);
	nxo_dup(ttstack, stack);
	nxo_stack_pop(ostack);

	nxo_stack_copy(ttstack, ostack);

	stack = nxo_stack_push(ostack);
	nxo_dup(stack, ttstack);
	nxo_stack_pop(tstack);
}

void
systemdict_stack(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *nstack;

	ostack = nxo_thread_ostack_get(a_thread);
	nstack = nxo_stack_push(ostack);
	nxo_stack_new(nstack, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread));
}

void
systemdict_start(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *tstack;
	cw_nxo_t	*onxo, *enxo;
	cw_uint32_t	edepth, tdepth;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	/* Record stack depths so that we can clean up later. */
	edepth = nxo_stack_count(estack);
	tdepth = nxo_stack_count(tstack);

	NXO_STACK_GET(onxo, ostack, a_thread);
	enxo = nxo_stack_push(estack);
	nxo_dup(enxo, onxo);
	nxo_stack_pop(ostack);

	xep_begin();
	xep_try {
		nxo_thread_loop(a_thread);
	}
	xep_catch(_CW_ONYXX_EXIT)
	xep_mcatch(_CW_ONYXX_QUIT)
	xep_mcatch(_CW_ONYXX_STOP) {
		xep_handled();
	}
	xep_end();

	/*
	 * Pop all objects off estack and tstack that weren't there
	 * before entering this function.
	 */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
}

void
systemdict_status(cw_nxo_t *a_thread)
{
	cw_nx_t		*nx;
	cw_nxo_t	*ostack, *tstack, *file;
	cw_nxo_t	*dict, *name, *value;
	int		error;
	struct stat	sb;

	nx = nxo_thread_nx_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(file, ostack, a_thread);
	if (nxo_type_get(file) != NXOT_FILE && nxo_type_get(file) !=
	    NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	if (nxo_type_get(file) == NXOT_FILE) {
		int	fd;

		fd = nxo_file_fd_get(file);
		if (fd < 0) {
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			return;
		}

		error = fstat(fd, &sb);
	} else {
		cw_nxo_t	*tfile;

		/*
		 * Create a copy of file with an extra byte to store a '\0'
		 * terminator.
		 */
		tfile = nxo_stack_push(tstack);
		nxo_string_new(tfile, nx, nxo_thread_currentlocking(a_thread),
		    nxo_string_len_get(file) + 1);
		nxo_string_lock(file);
		nxo_string_lock(tfile);
		nxo_string_set(tfile, 0, nxo_string_get(file),
		    nxo_string_len_get(file));
		nxo_string_el_set(tfile, '\0', nxo_string_len_get(tfile) - 1);
		nxo_string_unlock(file);

		error = stat(nxo_string_get(tfile), &sb);

		nxo_string_unlock(tfile);

		nxo_stack_pop(tstack);
	}

	if (error == -1) {
		switch (errno) {
		case EIO:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		case EACCES:
		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			break;
		case EBADF:
		case EFAULT:
		default:
			nxo_thread_error(a_thread, NXO_THREADE_UNREGISTERED);
		}
		return;
	}

	nxo_stack_pop(ostack);

	/*
	 * We now have a valid stat buffer.  Create a dictionary that represents
	 * it.
	 */
	dict = nxo_stack_push(ostack);
	nxo_dict_new(dict, nx, nxo_thread_currentlocking(a_thread), 13);

	name = nxo_stack_push(tstack);
	value = nxo_stack_push(tstack);

	/* dev. */
	nxo_name_new(name, nx, nxn_str(NXN_dev), nxn_len(NXN_dev), TRUE);
	nxo_integer_new(value, sb.st_dev);
	nxo_dict_def(dict, nx, name, value);

	/* ino. */
	nxo_name_new(name, nx, nxn_str(NXN_ino), nxn_len(NXN_ino), TRUE);
	nxo_integer_new(value, sb.st_ino);
	nxo_dict_def(dict, nx, name, value);

	/* mode. */
	nxo_name_new(name, nx, nxn_str(NXN_mode), nxn_len(NXN_mode), TRUE);
	nxo_integer_new(value, sb.st_mode);
	nxo_dict_def(dict, nx, name, value);

	/* nlink. */
	nxo_name_new(name, nx, nxn_str(NXN_nlink), nxn_len(NXN_nlink), TRUE);
	nxo_integer_new(value, sb.st_nlink);
	nxo_dict_def(dict, nx, name, value);

	/* uid. */
	nxo_name_new(name, nx, nxn_str(NXN_uid), nxn_len(NXN_uid), TRUE);
	nxo_integer_new(value, sb.st_uid);
	nxo_dict_def(dict, nx, name, value);

	/* gid. */
	nxo_name_new(name, nx, nxn_str(NXN_gid), nxn_len(NXN_gid), TRUE);
	nxo_integer_new(value, sb.st_gid);
	nxo_dict_def(dict, nx, name, value);

	/* rdev. */
	nxo_name_new(name, nx, nxn_str(NXN_rdev), nxn_len(NXN_rdev), TRUE);
	nxo_integer_new(value, sb.st_rdev);
	nxo_dict_def(dict, nx, name, value);

	/* size. */
	nxo_name_new(name, nx, nxn_str(NXN_size), nxn_len(NXN_size), TRUE);
	nxo_integer_new(value, sb.st_size);
	nxo_dict_def(dict, nx, name, value);

	/* atime. */
	nxo_name_new(name, nx, nxn_str(NXN_atime), nxn_len(NXN_atime), TRUE);
#ifdef _CW_OS_FREEBSD
	nxo_integer_new(value, ((cw_nxoi_t)sb.st_atimespec.tv_sec *
	    (cw_nxoi_t)1000000000) + (cw_nxoi_t)sb.st_atimespec.tv_nsec);
#else
	nxo_integer_new(value, ((cw_nxoi_t)sb.st_atime *
	    (cw_nxoi_t)1000000000));
#endif
	nxo_dict_def(dict, nx, name, value);

	/* mtime. */
	nxo_name_new(name, nx, nxn_str(NXN_mtime), nxn_len(NXN_mtime), TRUE);
#ifdef _CW_OS_FREEBSD
	nxo_integer_new(value, ((cw_nxoi_t)sb.st_mtimespec.tv_sec *
	    (cw_nxoi_t)1000000000) + (cw_nxoi_t)sb.st_mtimespec.tv_nsec);
#else
	nxo_integer_new(value, ((cw_nxoi_t)sb.st_mtime *
	    (cw_nxoi_t)1000000000));
#endif
	nxo_dict_def(dict, nx, name, value);

	/* ctime. */
	nxo_name_new(name, nx, nxn_str(NXN_ctime), nxn_len(NXN_ctime), TRUE);
#ifdef _CW_OS_FREEBSD
	nxo_integer_new(value, ((cw_nxoi_t)sb.st_ctimespec.tv_sec *
	    (cw_nxoi_t)1000000000) + (cw_nxoi_t)sb.st_ctimespec.tv_nsec);
#else
	nxo_integer_new(value, ((cw_nxoi_t)sb.st_ctime *
	    (cw_nxoi_t)1000000000));
#endif
	nxo_dict_def(dict, nx, name, value);

	/* blksize. */
	nxo_name_new(name, nx, nxn_str(NXN_blksize), nxn_len(NXN_blksize),
	    TRUE);
	nxo_integer_new(value, sb.st_blksize);
	nxo_dict_def(dict, nx, name, value);

	/* blocks. */
	nxo_name_new(name, nx, nxn_str(NXN_blocks), nxn_len(NXN_blocks), TRUE);
	nxo_integer_new(value, sb.st_blocks);
	nxo_dict_def(dict, nx, name, value);

	nxo_stack_npop(tstack, 2);
}

void
systemdict_stop(cw_nxo_t *a_thread)
{
	xep_throw(_CW_ONYXX_STOP);
}

void
systemdict_stopped(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *tstack;
	cw_nxo_t	*exec, *nxo;
	cw_bool_t	result = FALSE;
	cw_uint32_t	edepth, tdepth;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	
	NXO_STACK_GET(exec, ostack, a_thread);
	nxo = nxo_stack_push(estack);
	nxo_dup(nxo, exec);
	nxo_stack_pop(ostack);

	/* Record stack depths so that we can clean up if necessary. */
	edepth = nxo_stack_count(estack);
	tdepth = nxo_stack_count(tstack);

	/* Catch a stop exception, if thrown. */
	xep_begin();
	xep_try {
		nxo_thread_loop(a_thread);
	}
	xep_catch(_CW_ONYXX_STOP) {
		xep_handled();
		result = TRUE;

		/* Clean up stacks. */
		nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
		nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
	}
	xep_end();

	nxo = nxo_stack_push(ostack);
	nxo_boolean_new(nxo, result);
}

void
systemdict_store(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *dstack;
	cw_nxo_t	*dict, *key, *val;
	cw_uint32_t	i, depth;

	ostack = nxo_thread_ostack_get(a_thread);
	dstack = nxo_thread_dstack_get(a_thread);

	NXO_STACK_GET(val, ostack, a_thread);
	NXO_STACK_DOWN_GET(key, ostack, a_thread, val);

	/*
	 * Iteratively search the dictionaries on the dictionary stack for key,
	 * and replace its value with val.
	 */
	for (i = 0, depth = nxo_stack_count(dstack), dict = NULL; i < depth;
	    i++) {
		dict = nxo_stack_down_get(dstack, dict);
		if (nxo_dict_lookup(dict, key, NULL) == FALSE) {
			/* Found. */
			nxo_dict_def(dict, nxo_thread_nx_get(a_thread), key,
			    val);
			return;
		}
	}
	/* Not found.  Create a new entry in currentdict. */
	dict = nxo_stack_get(dstack);
	nxo_dict_def(dict, nxo_thread_nx_get(a_thread), key, val);

	nxo_stack_npop(ostack, 2);
}

void
systemdict_string(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_nxoi_t	len;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	len = nxo_integer_get(nxo);
	if (len < 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), len);
}

void
systemdict_sub(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*a, *b;

	ostack = nxo_thread_ostack_get(a_thread);
	
	NXO_STACK_GET(b, ostack, a_thread);
	NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
	if (nxo_type_get(a) != NXOT_INTEGER || nxo_type_get(b) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_integer_set(a, nxo_integer_get(a) - nxo_integer_get(b));
	nxo_stack_pop(ostack);
}

/* ) */
void
systemdict_sym_rp(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*nstack, *tnxo, *nxo;
	cw_sint32_t	nelements, i, depth;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	/* Find the fino. */
	for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth;
	    i++) {
		nxo = nxo_stack_down_get(ostack, nxo);
		if (nxo_type_get(nxo) == NXOT_FINO)
			break;
	}
	if (i == depth) {
		nxo_thread_error(a_thread, NXO_THREADE_UNMATCHEDFINO);
		return;
	}

	/*
	 * i is the index of the fino, and nxo points to the fino.  Set
	 * nelements accordingly.
	 */
	nelements = i;

	nstack = nxo_stack_push(tstack);
	nxo_stack_new(nstack, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread));

	/*
	 * Push objects onto tstack and pop them off ostack.
	 */
	for (i = 0; i < nelements; i++) {
		nxo = nxo_stack_get(ostack);
		tnxo = nxo_stack_push(tstack);
		nxo_dup(tnxo, nxo);
		nxo_stack_pop(ostack);
	}

	/* Pop the fino off ostack. */
	nxo_stack_pop(ostack);

	/*
	 * Push objects onto nstack and pop them off tstack.
	 */
	for (i = 0; i < nelements; i++) {
		nxo = nxo_stack_get(tstack);
		tnxo = nxo_stack_push(nstack);
		nxo_dup(tnxo, nxo);
		nxo_stack_pop(tstack);
	}

	/* Push nstack onto ostack and pop it off of tstack. */
	nxo = nxo_stack_push(ostack);
	nxo_dup(nxo, nstack);
	nxo_stack_pop(tstack);
}

/* > */
void
systemdict_sym_gt(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*nxo, *dict, *key, *val;
	cw_uint32_t	npairs, i, depth;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	/* Find the mark. */
	for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth;
	    i++) {
		nxo = nxo_stack_down_get(ostack, nxo);
		if (nxo_type_get(nxo) == NXOT_MARK)
			break;
	}
	if (i == depth) {
		nxo_thread_error(a_thread, NXO_THREADE_UNMATCHEDMARK);
		return;
	}
	if ((i & 1) == 1) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	/*
	 * i is the index of the mark, and nxo points to the mark.  Set npairs
	 * accordingly.  When we pop the nxo's off the stack, we'll have to
	 * pop (npairs << 1 + 1) nxo's.
	 */
	npairs = i >> 1;

	dict = nxo_stack_push(tstack);
	nxo_dict_new(dict, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), npairs);

	/*
	 * Traverse down the stack, moving nxo's to the dict.
	 */
	for (i = 0, key = NULL; i < npairs; i++) {
		val = nxo_stack_down_get(ostack, key);
		key = nxo_stack_down_get(ostack, val);
		nxo_dict_def(dict, nxo_thread_nx_get(a_thread), key, val);
	}

	/* Pop the nxo's off the stack now. */
	nxo_stack_npop(ostack, (npairs << 1) + 1);

	/* Push the dict onto the stack. */
	nxo = nxo_stack_push(ostack);
	nxo_dup(nxo, dict);

	nxo_stack_pop(tstack);
}

/* ] */
void
systemdict_sym_rb(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*tnxo, *nxo;
	cw_sint32_t	nelements, i, depth;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	/* Find the mark. */
	for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth;
	    i++) {
		nxo = nxo_stack_down_get(ostack, nxo);
		if (nxo_type_get(nxo) == NXOT_MARK)
			break;
	}
	if (i == depth) {
		nxo_thread_error(a_thread, NXO_THREADE_UNMATCHEDMARK);
		return;
	}

	/*
	 * i is the index of the mark, and nxo points to the mark.  Set
	 * nelements accordingly.  When we pop the nxo's off the stack, we'll
	 * have to pop (nelements + 1) nxo's.
	 */
	nelements = i;

	tnxo = nxo_stack_push(tstack);
	nxo_array_new(tnxo, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nelements);

	/*
	 * Traverse down the stack, moving nxo's to the array.
	 */
	for (i = nelements - 1, nxo = NULL; i >= 0; i--) {
		nxo = nxo_stack_down_get(ostack, nxo);
		nxo_array_el_set(tnxo, nxo, i);
	}

	/* Pop the nxo's off the stack now. */
	nxo_stack_npop(ostack, nelements + 1);

	/* Push the array onto the stack. */
	nxo = nxo_stack_push(ostack);
	nxo_dup(nxo, tnxo);

	nxo_stack_pop(tstack);
}

void
systemdict_symlink(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*filename, *linkname, *tfilename, *tlinkname;
	int		error;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(linkname, ostack, a_thread);
	NXO_STACK_DOWN_GET(filename, ostack, a_thread, linkname);
	if (nxo_type_get(filename) != NXOT_STRING ||
	    nxo_type_get(linkname) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/*
	 * Create a copy of filename with an extra byte to store a '\0'
	 * terminator.
	 */
	tfilename = nxo_stack_push(tstack);
	nxo_string_new(tfilename, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(filename) +
	    1);
	nxo_string_lock(filename);
	nxo_string_lock(tfilename);
	nxo_string_set(tfilename, 0, nxo_string_get(filename),
	    nxo_string_len_get(filename));
	nxo_string_el_set(tfilename, '\0', nxo_string_len_get(tfilename) - 1);
	nxo_string_unlock(filename);

	/*
	 * Create a copy of linkname with an extra byte to store a '\0'
	 * terminator.
	 */
	tlinkname = nxo_stack_push(tstack);
	nxo_string_new(tlinkname, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(linkname) +
	    1);
	nxo_string_lock(linkname);
	nxo_string_lock(tlinkname);
	nxo_string_set(tlinkname, 0, nxo_string_get(linkname),
	    nxo_string_len_get(linkname));
	nxo_string_el_set(tlinkname, '\0', nxo_string_len_get(tlinkname) - 1);
	nxo_string_unlock(linkname);

	error = symlink(nxo_string_get(tfilename), nxo_string_get(tlinkname));
	nxo_stack_npop(tstack, 2);
	if (error == -1) {
		switch (errno) {
		case EDQUOT:
		case EIO:
		case EMLINK:
		case ENOSPC:
		case EROFS:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		case EEXIST:
		case ENOENT:
		case ENOTDIR:
			nxo_thread_error(a_thread,
			    NXO_THREADE_UNDEFINEDFILENAME);
			break;
		case EACCES:
		case ELOOP:
		case ENAMETOOLONG:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			break;
		case EFAULT:
		default:
			nxo_thread_error(a_thread, NXO_THREADE_UNREGISTERED);
		}
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_tell(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*file;
	cw_nxoi_t	position;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(file, ostack, a_thread);
	
	if (nxo_type_get(file) != NXOT_FILE) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	position = nxo_file_position_get(file);
	if (position == -1) {
		nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
		return;
	}
	nxo_integer_new(file, position);
}

void
systemdict_test(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *file, *test;
	cw_uint8_t	c;
	cw_bool_t	result, fd = -1;
	int		error;
	struct stat	sb;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(test, ostack, a_thread);
	NXO_STACK_DOWN_GET(file, ostack, a_thread, test);
	if ((nxo_type_get(file) != NXOT_FILE && nxo_type_get(file) !=
	    NXOT_STRING) || nxo_type_get(test) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_string_len_get(test) != 1) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}
	nxo_string_el_get(test, 0, &c);
	switch (c) {
	case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'k':
	case 'p': case 'r': case 's': case 't': case 'u': case 'w': case 'x':
	case 'L': case 'O': case 'G': case 'S':
		break;
	default:
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	/*
	 * We haven't determined which test to perform yet, but in any case, we
	 * will need the results of a stat()/fstat() call.  Do this first, then
	 * use the data as appropriate later.
	 */
	if (nxo_type_get(file) == NXOT_FILE) {
		fd = nxo_file_fd_get(file);
		if (fd < 0) {
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			return;
		}

		error = fstat(fd, &sb);
	} else {
		cw_nxo_t	*tstack, *tfile;

		tstack = nxo_thread_tstack_get(a_thread);

		/*
		 * Create a copy of file with an extra byte to store a '\0'
		 * terminator.
		 */
		tfile = nxo_stack_push(tstack);
		nxo_string_new(tfile, nxo_thread_nx_get(a_thread),
		    nxo_thread_currentlocking(a_thread),
		    nxo_string_len_get(file) + 1);
		nxo_string_lock(file);
		nxo_string_lock(tfile);
		nxo_string_set(tfile, 0, nxo_string_get(file),
		    nxo_string_len_get(file));
		nxo_string_el_set(tfile, '\0', nxo_string_len_get(tfile) - 1);
		nxo_string_unlock(file);

		error = stat(nxo_string_get(tfile), &sb);

		nxo_string_unlock(tfile);

		nxo_stack_pop(tstack);
	}

	if (error == -1) {
		/*
		 * There was a stat error.  If this is because the file doesn't
		 * exist, set result to FALSE.  Otherwise, throw an error.
		 */
		switch (errno) {
		case EACCES:
		case ENOENT:
		case ENOTDIR:
			result = FALSE;
			break;
		case EIO:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			return;
		case ELOOP:
		case ENAMETOOLONG:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			return;
		case EBADF:
		case EFAULT:
			nxo_thread_error(a_thread, NXO_THREADE_UNREGISTERED);
			return;
		}
	} else {
		switch (c) {
		case 'b':
			/* Block special device? */
			result = (sb.st_mode & S_IFBLK) ? TRUE : FALSE;
			break;
		case 'c':
			/* Character special device? */
			result = (sb.st_mode & S_IFCHR) ? TRUE : FALSE;
			break;
		case 'd':
			/* Directory? */
			result = (sb.st_mode & S_IFDIR) ? TRUE : FALSE;
			break;
		case 'e':
			/* Exists? */
			/* There was no stat error, so the file must exist. */
			result = TRUE;
			break;
		case 'f':
			/* Regular file? */
			result = (sb.st_mode & S_IFREG) ? TRUE : FALSE;
			break;
		case 'g':
			/* Setgid? */
			result = (sb.st_mode & S_ISGID) ? TRUE : FALSE;
			break;
		case 'k':
			/* Sticky? */
			result = (sb.st_mode & S_ISVTX) ? TRUE : FALSE;
			break;
		case 'p':
			/* Named pipe? */
			result = (sb.st_mode & S_IFIFO) ? TRUE : FALSE;
			break;
		case 'r':
			/* Readable? */
			result = (sb.st_mode & S_IRUSR) ? TRUE : FALSE;
			break;
		case 's':
			/* Size greater than 0? */
			result = (sb.st_size > 0) ? TRUE : FALSE;
			break;
		case 't':
			/* tty? */
			/* fd only.  If a string was passed in, return false. */
			if (fd == -1)
				result = FALSE;
			else
				result = (isatty(fd)) ? TRUE : FALSE;
			break;
		case 'u':
			/* Setuid? */
			result = (sb.st_mode & S_ISUID) ? TRUE : FALSE;
			break;
		case 'w':
			/* Write bit set? */
			result = (sb.st_mode & S_IWUSR) ? TRUE : FALSE;
			break;
		case 'x':
			/* Executable bit set? */
			result = (sb.st_mode & S_IXUSR) ? TRUE : FALSE;
			break;
		case 'L':
			/* Symbolic link? */
			result = (sb.st_mode & S_IFLNK) ? TRUE : FALSE;
			break;
		case 'O':
			/* Owner matches effective uid? */
			result = (geteuid() == sb.st_uid) ? TRUE : FALSE;
			break;
		case 'G':
			/* Group matches effective gid? */
			result = (getegid() == sb.st_gid) ? TRUE : FALSE;
			break;
		case 'S':
			/* Socket? */
			result = (sb.st_mode & S_IFSOCK) ? TRUE : FALSE;
			break;
		default:
			_cw_not_reached();
		}
	}

	nxo_stack_pop(ostack);
	nxo_boolean_new(file, result);
}

void
systemdict_thread(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack, *stack;
	cw_nxo_t	*entry, *thread, *nxo;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);

	NXO_STACK_GET(entry, ostack, a_thread);
	NXO_STACK_DOWN_GET(stack, ostack, a_thread, entry);
	if (nxo_type_get(stack) != NXOT_STACK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/* Create the new thread. */
	thread = nxo_stack_under_push(ostack, stack);
	nxo_thread_new(thread, nxo_thread_nx_get(a_thread));

	/* Set up the new thread's ostack. */
	nxo_stack_copy(nxo_thread_ostack_get(thread), stack);
	nxo = nxo_stack_push(nxo_thread_ostack_get(thread));
	nxo_dup(nxo, entry);

	/* Clean up. */
	nxo_stack_npop(ostack, 2);

	/* Start the thread. */
	nxo_thread_thread(thread);
}

void
systemdict_timedwait(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*condition, *mutex, *nsecs;
	struct timespec	timeout;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nsecs, ostack, a_thread);
	NXO_STACK_DOWN_GET(mutex, ostack, a_thread, nsecs);
	NXO_STACK_DOWN_GET(condition, ostack, a_thread, mutex);
	if (nxo_type_get(condition) != NXOT_CONDITION ||
	    nxo_type_get(mutex) != NXOT_MUTEX || nxo_type_get(nsecs) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(nsecs) < 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	/* Convert integer to timespec. */
	timeout.tv_nsec = nxo_integer_get(nsecs) % 1000000000;
	timeout.tv_sec = nxo_integer_get(nsecs) / 1000000000;

	nxo_condition_timedwait(condition, mutex, &timeout);

	nxo_stack_npop(ostack, 3);
}

void
systemdict_token(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*nxo, *tnxo;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	switch (nxo_type_get(nxo)) {
	case NXOT_STRING: {
		cw_nxo_threadp_t	threadp;
		cw_uint32_t		nscanned, scount;

		scount = nxo_stack_count(ostack);
		tnxo = nxo_stack_push(tstack);
		nxo_dup(tnxo, nxo);
		nxo_threadp_new(&threadp);

		xep_begin();
		xep_try {
			nxo_string_lock(tnxo);
			nscanned = nxo_thread_token(a_thread, &threadp,
			    nxo_string_get(tnxo), nxo_string_len_get(tnxo));
		}
		xep_acatch {
			nxo_string_unlock(tnxo);
			nxo_stack_pop(tstack);
			nxo_threadp_delete(&threadp, a_thread);
		}
		xep_end();
		nxo_string_unlock(tnxo);

		xep_begin();
		xep_try {
			nxo_thread_flush(a_thread, &threadp);
		}
		xep_acatch {
			nxo_stack_pop(tstack);
			nxo_threadp_delete(&threadp, a_thread);
		}
		xep_end();

		if (nxo_thread_state(a_thread) == THREADTS_START &&
		    nxo_thread_deferred(a_thread) == FALSE &&
		    nxo_stack_count(ostack) == scount + 1) {
			/* Success. */
			nxo_string_substring_new(nxo, tnxo,
			    nxo_thread_nx_get(a_thread), nscanned,
			    nxo_string_len_get(tnxo) - nscanned);
			nxo = nxo_stack_push(ostack);
			nxo_boolean_new(nxo, TRUE);
		} else {
			cw_uint32_t	i;

			/*
			 * We failed to scan a token.  Reset a_thread and clean
			 * up ostack.
			 */
			nxo_thread_reset(a_thread);
			for (i = nxo_stack_count(ostack); i > scount; i--)
				nxo_stack_pop(ostack);

			nxo_boolean_new(nxo, FALSE);
		}
		nxo_threadp_delete(&threadp, a_thread);
		nxo_stack_pop(tstack);
		break;
	}
	case NXOT_FILE: {
		cw_nxo_threadp_t	threadp;
		cw_sint32_t		nread;
		cw_uint32_t		scount;
		cw_uint8_t		c;

		scount = nxo_stack_count(ostack);
		tnxo = nxo_stack_push(tstack);
		nxo_dup(tnxo, nxo);
		nxo_threadp_new(&threadp);

		/*
		 * Feed the scanner one byte at a time, checking after every
		 * character whether a token was accepted.  If we run out of
		 * data, flush the scanner in the hope of causing token
		 * acceptance.
		 */
		for (nread = nxo_file_read(tnxo, 1, &c); nread > 0; nread =
		    nxo_file_read(tnxo, 1, &c)) {
			xep_begin();
			xep_try {
				nxo_thread_token(a_thread, &threadp, &c, 1);
			}
			xep_acatch {
				nxo_stack_pop(tstack);
				nxo_threadp_delete(&threadp, a_thread);
			}
			xep_end();

			if (nxo_thread_state(a_thread) == THREADTS_START &&
			    nxo_thread_deferred(a_thread) == FALSE &&
			    nxo_stack_count(ostack) == scount + 1)
				goto SUCCESS;
		}
		xep_begin();
		xep_try {
			nxo_thread_flush(a_thread, &threadp);
		}
		xep_acatch {
			nxo_stack_pop(tstack);
			nxo_threadp_delete(&threadp, a_thread);
		}
		xep_end();

		if (nxo_thread_state(a_thread) == THREADTS_START &&
		    nxo_thread_deferred(a_thread) == FALSE &&
		    nxo_stack_count(ostack) == scount + 1) {
			/* Success. */
			SUCCESS:
			nxo_boolean_new(nxo, TRUE);
			nxo_stack_roll(ostack, 2, 1);
		} else {
			cw_uint32_t	i;

			/*
			 * We failed to scan a token.  Reset a_thread and clean
			 * up ostack.
			 */
			nxo_thread_reset(a_thread);
			for (i = nxo_stack_count(ostack); i > scount; i--)
				nxo_stack_pop(ostack);

			nxo_boolean_new(nxo, FALSE);
		}
		nxo_threadp_delete(&threadp, a_thread);
		nxo_stack_pop(tstack);
		break;
	}
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
}

void
systemdict_truncate(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack;
	cw_nxo_t		*file, *length;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(length, ostack, a_thread);
	NXO_STACK_DOWN_GET(file, ostack, a_thread, length);
	if (nxo_type_get(file) != NXOT_FILE || nxo_type_get(length) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	if (nxo_integer_get(length) < 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	error = nxo_file_truncate(file, nxo_integer_get(length));
	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_trylock(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*mutex;
	cw_bool_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(mutex, ostack, a_thread);
	if (nxo_type_get(mutex) != NXOT_MUTEX) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	error = nxo_mutex_trylock(mutex);

	nxo_boolean_new(mutex, error);
}

void
systemdict_type(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_nxot_t	type;
	/* Must be kept in sync with cw_nxot_t. */
	static const cw_nxn_t typenames[] = {
		0,
		NXN_arraytype,
		NXN_booleantype,
		NXN_conditiontype,
		NXN_dicttype,
		NXN_filetype,
		NXN_finotype,
		NXN_hooktype,
		NXN_integertype,
		NXN_marktype,
		NXN_mutextype,
		NXN_nametype,
		NXN_nulltype,
		NXN_operatortype,
		NXN_stacktype,
		NXN_stringtype,
		NXN_threadtype
	};
	_cw_assert(sizeof(typenames) / sizeof(cw_nxn_t) == NXOT_LAST + 1);

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);

	type = nxo_type_get(nxo);
	_cw_assert(type > NXOT_NO && type <= NXOT_LAST);

	nxo_name_new(nxo, nxo_thread_nx_get(a_thread),
	    nxn_str(typenames[type]), nxn_len(typenames[type]), TRUE);
	nxo_attrs_set(nxo, NXOA_EXECUTABLE);
}

void
systemdict_undef(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*dict, *key;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(key, ostack, a_thread);
	NXO_STACK_DOWN_GET(dict, ostack, a_thread, key);
	if (nxo_type_get(dict) != NXOT_DICT) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_dict_undef(dict, nxo_thread_nx_get(a_thread), key);

	nxo_stack_npop(ostack, 2);
}

void
systemdict_unlink(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack;
	cw_nxo_t	*filename, *tfilename;
	int		error;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(filename, ostack, a_thread);

	if (nxo_type_get(filename) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/*
	 * Create a copy of filename with an extra byte to store a '\0'
	 * terminator.
	 */
	tfilename = nxo_stack_push(tstack);
	nxo_string_new(tfilename, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(filename) +
	    1);
	nxo_string_lock(filename);
	nxo_string_lock(tfilename);
	nxo_string_set(tfilename, 0, nxo_string_get(filename),
	    nxo_string_len_get(filename));
	nxo_string_el_set(tfilename, '\0', nxo_string_len_get(tfilename) - 1);
	nxo_string_unlock(filename);

	error = unlink(nxo_string_get(tfilename));

	nxo_string_unlock(tfilename);
	nxo_stack_pop(tstack);

	if (error == -1) {
		switch (errno) {
		case EIO:
		case EBUSY:
		case ELOOP:
		case EROFS:
			nxo_thread_error(a_thread, NXO_THREADE_IOERROR);
			break;
		case ENOENT:
		case ENOTDIR:
		case ENAMETOOLONG:
			nxo_thread_error(a_thread,
			    NXO_THREADE_UNDEFINEDFILENAME);
			break;
		case EACCES:
		case EPERM:
			nxo_thread_error(a_thread,
			    NXO_THREADE_INVALIDFILEACCESS);
			break;
		case EFAULT:
		default:
			nxo_thread_error(a_thread, NXO_THREADE_UNREGISTERED);
		}
		return;
	}

	nxo_stack_pop(ostack);
}

void
systemdict_unlock(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*mutex;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(mutex, ostack, a_thread);
	if (nxo_type_get(mutex) != NXOT_MUTEX) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_mutex_unlock(mutex);

	nxo_stack_pop(ostack);
}

void
systemdict_wait(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*condition, *mutex;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(mutex, ostack, a_thread);
	NXO_STACK_DOWN_GET(condition, ostack, a_thread, mutex);
	if (nxo_type_get(condition) != NXOT_CONDITION ||
	    nxo_type_get(mutex) != NXOT_MUTEX) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_condition_wait(condition, mutex);

	nxo_stack_npop(ostack, 2);
}

void
systemdict_waitpid(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	pid_t		pid;
	int		status;
	cw_nxoi_t	result;
	
	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	pid = nxo_integer_get(nxo);

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

	nxo_integer_new(nxo, result);
}

void
systemdict_where(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *dstack;
	cw_nxo_t	*dict, *key, *nxo;
	cw_uint32_t	i, depth;

	ostack = nxo_thread_ostack_get(a_thread);
	dstack = nxo_thread_dstack_get(a_thread);

	NXO_STACK_GET(key, ostack, a_thread);

	/*
	 * Iteratively search the dictionaries on the dictionary stack for key.
	 */
	for (i = 0, depth = nxo_stack_count(dstack), dict = NULL; i < depth;
	    i++) {
		dict = nxo_stack_down_get(dstack, dict);
		if (nxo_dict_lookup(dict, key, NULL) == FALSE) {
			/* Found. */
			nxo = nxo_stack_push(ostack);
			nxo_dup(key, dict);
			nxo_boolean_new(nxo, TRUE);
			return;
		}
	}
	/* Not found.  Create a new entry in currentdict. */
	nxo_boolean_new(key, FALSE);
}

void
systemdict_write(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack;
	cw_nxo_t		*file, *value;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(value, ostack, a_thread);
	NXO_STACK_DOWN_GET(file, ostack, a_thread, value);
	
	if (nxo_type_get(file) != NXOT_FILE) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	switch (nxo_type_get(value)) {
	case NXOT_INTEGER: {
		cw_uint8_t	val;

		val = (cw_uint8_t)nxo_integer_get(value);
		error = nxo_file_write(file, &val, 1);
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}
		break;
	}
	case NXOT_STRING:
		nxo_string_lock(value);
		error = nxo_file_write(file, nxo_string_get(value),
		    nxo_string_len_get(value));
		nxo_string_unlock(value);
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}
		break;
	default:
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
systemdict_xcheck(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_nxoa_t	attr;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	
	attr = nxo_attrs_get(nxo);

	if (attr == NXOA_EXECUTABLE)
		nxo_boolean_new(nxo, TRUE);
	else
		nxo_boolean_new(nxo, FALSE);
}

void
systemdict_xor(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo_a, *nxo_b;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo_b, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

	if (nxo_type_get(nxo_a) == NXOT_BOOLEAN && nxo_type_get(nxo_b) ==
	    NXOT_BOOLEAN) {
		cw_bool_t	xor;

		if (nxo_boolean_get(nxo_a) || nxo_boolean_get(nxo_b)) {
			if (nxo_boolean_get(nxo_a) ==
			    nxo_boolean_get(nxo_b))
				xor = FALSE;
			else
				xor = TRUE;
		} else
			xor = FALSE;
		nxo_boolean_new(nxo_a, xor);
	} else if (nxo_type_get(nxo_a) == NXOT_INTEGER &&
	    nxo_type_get(nxo_b) == NXOT_INTEGER) {
		nxo_integer_set(nxo_a, nxo_integer_get(nxo_a) ^
		    nxo_integer_get(nxo_b));
	} else {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	nxo_stack_pop(ostack);
}

void
systemdict_yield(cw_nxo_t *a_thread)
{
	thd_yield();
}
