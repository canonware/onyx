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

struct cw_systemdict_entry {
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

#define _SYSTEMDICT_ENTRY(name)	{#name, systemdict_##name}

/*
 * Array of operators in systemdict.
 */
static struct cw_systemdict_entry systemdict_ops[] = {
	_SYSTEMDICT_ENTRY(abort),
	_SYSTEMDICT_ENTRY(abs),
	_SYSTEMDICT_ENTRY(add),
	_SYSTEMDICT_ENTRY(aload),
	_SYSTEMDICT_ENTRY(anchorsearch),
	_SYSTEMDICT_ENTRY(and),
	_SYSTEMDICT_ENTRY(array),
	_SYSTEMDICT_ENTRY(astore),
	_SYSTEMDICT_ENTRY(begin),
	_SYSTEMDICT_ENTRY(bind),
	_SYSTEMDICT_ENTRY(bytesavailable),
	_SYSTEMDICT_ENTRY(clear),
	_SYSTEMDICT_ENTRY(cleardictstack),
	_SYSTEMDICT_ENTRY(cleartomark),
	_SYSTEMDICT_ENTRY(closefile),
	_SYSTEMDICT_ENTRY(condition),
	_SYSTEMDICT_ENTRY(copy),
	_SYSTEMDICT_ENTRY(count),
	_SYSTEMDICT_ENTRY(countdictstack),
	_SYSTEMDICT_ENTRY(countexecstack),
	_SYSTEMDICT_ENTRY(counttomark),
	_SYSTEMDICT_ENTRY(currentcontext),
	_SYSTEMDICT_ENTRY(currentdict),
	_SYSTEMDICT_ENTRY(currentfile),
	_SYSTEMDICT_ENTRY(currentglobal),
	_SYSTEMDICT_ENTRY(cvlit),
	_SYSTEMDICT_ENTRY(cvm),
	_SYSTEMDICT_ENTRY(cvn),
	_SYSTEMDICT_ENTRY(cvrs),
	_SYSTEMDICT_ENTRY(cvs),
	_SYSTEMDICT_ENTRY(cvx),
	_SYSTEMDICT_ENTRY(def),
	_SYSTEMDICT_ENTRY(defineresource),
	_SYSTEMDICT_ENTRY(deletefile),
	_SYSTEMDICT_ENTRY(detach),
	_SYSTEMDICT_ENTRY(dict),
	_SYSTEMDICT_ENTRY(dictstack),
	_SYSTEMDICT_ENTRY(div),
	_SYSTEMDICT_ENTRY(dup),
	_SYSTEMDICT_ENTRY(end),
	_SYSTEMDICT_ENTRY(eq),
	_SYSTEMDICT_ENTRY(errordict),
	_SYSTEMDICT_ENTRY(exch),
	_SYSTEMDICT_ENTRY(exec),
	_SYSTEMDICT_ENTRY(execstack),
	_SYSTEMDICT_ENTRY(executeonly),
	_SYSTEMDICT_ENTRY(exit),
	_SYSTEMDICT_ENTRY(exp),
	_SYSTEMDICT_ENTRY(false),
	_SYSTEMDICT_ENTRY(file),
	_SYSTEMDICT_ENTRY(filenameforall),
	_SYSTEMDICT_ENTRY(fileposition),
	_SYSTEMDICT_ENTRY(filter),
	_SYSTEMDICT_ENTRY(findresource),
	_SYSTEMDICT_ENTRY(flush),
	_SYSTEMDICT_ENTRY(flushfile),
	_SYSTEMDICT_ENTRY(for),
	_SYSTEMDICT_ENTRY(forall),
	_SYSTEMDICT_ENTRY(fork),
	_SYSTEMDICT_ENTRY(gcheck),
	_SYSTEMDICT_ENTRY(ge),
	_SYSTEMDICT_ENTRY(get),
	_SYSTEMDICT_ENTRY(getinterval),
	_SYSTEMDICT_ENTRY(globaldict),
	_SYSTEMDICT_ENTRY(gt),
	_SYSTEMDICT_ENTRY(if),
	_SYSTEMDICT_ENTRY(ifelse),
	_SYSTEMDICT_ENTRY(index),
	_SYSTEMDICT_ENTRY(internaldict),
	_SYSTEMDICT_ENTRY(join),
	_SYSTEMDICT_ENTRY(known),
	_SYSTEMDICT_ENTRY(le),
	_SYSTEMDICT_ENTRY(length),
	_SYSTEMDICT_ENTRY(load),
	_SYSTEMDICT_ENTRY(localinstancedict),
	_SYSTEMDICT_ENTRY(lock),
	_SYSTEMDICT_ENTRY(loop),
	_SYSTEMDICT_ENTRY(lt),
	_SYSTEMDICT_ENTRY(mark),
	_SYSTEMDICT_ENTRY(maxlength),
	_SYSTEMDICT_ENTRY(mod),
	_SYSTEMDICT_ENTRY(mul),
	_SYSTEMDICT_ENTRY(mutex),
	_SYSTEMDICT_ENTRY(ne),
	_SYSTEMDICT_ENTRY(neg),
	_SYSTEMDICT_ENTRY(noaccess),
	_SYSTEMDICT_ENTRY(not),
	_SYSTEMDICT_ENTRY(notify),
	_SYSTEMDICT_ENTRY(null),
	_SYSTEMDICT_ENTRY(or),
	_SYSTEMDICT_ENTRY(pop),
	_SYSTEMDICT_ENTRY(print),
	_SYSTEMDICT_ENTRY(printobject),
	_SYSTEMDICT_ENTRY(product),
	_SYSTEMDICT_ENTRY(prompt),
	_SYSTEMDICT_ENTRY(pstack),
	_SYSTEMDICT_ENTRY(put),
	_SYSTEMDICT_ENTRY(putinterval),
	_SYSTEMDICT_ENTRY(quit),
	_SYSTEMDICT_ENTRY(rand),
	_SYSTEMDICT_ENTRY(rcheck),
	_SYSTEMDICT_ENTRY(read),
	_SYSTEMDICT_ENTRY(readonly),
	_SYSTEMDICT_ENTRY(readstring),
	_SYSTEMDICT_ENTRY(realtime),
	_SYSTEMDICT_ENTRY(renamefile),
	_SYSTEMDICT_ENTRY(repeat),
	_SYSTEMDICT_ENTRY(resourceforall),
	_SYSTEMDICT_ENTRY(resourcestatus),
	_SYSTEMDICT_ENTRY(roll),
	_SYSTEMDICT_ENTRY(run),
	_SYSTEMDICT_ENTRY(search),
	_SYSTEMDICT_ENTRY(serverdict),
	_SYSTEMDICT_ENTRY(setfileposition),
	_SYSTEMDICT_ENTRY(setglobal),
	_SYSTEMDICT_ENTRY(shift),
	_SYSTEMDICT_ENTRY(srand),
	_SYSTEMDICT_ENTRY(stack),
	_SYSTEMDICT_ENTRY(start),
	_SYSTEMDICT_ENTRY(status),
	_SYSTEMDICT_ENTRY(statusdict),
	_SYSTEMDICT_ENTRY(stdin),
	_SYSTEMDICT_ENTRY(stderr),
	_SYSTEMDICT_ENTRY(stdout),
	_SYSTEMDICT_ENTRY(stop),
	_SYSTEMDICT_ENTRY(stopped),
	_SYSTEMDICT_ENTRY(store),
	_SYSTEMDICT_ENTRY(string),
	_SYSTEMDICT_ENTRY(sub),
	{"$error",	systemdict_sym_derror},
	{"=",	systemdict_sym_eq},
	{"==",	systemdict_sym_eq_eq},
	{">>",	systemdict_sym_gt_gt},
	{"[",	systemdict_mark},
	{"<<",	systemdict_mark},
	{"]",	systemdict_sym_rb},
	_SYSTEMDICT_ENTRY(systemdict),
	_SYSTEMDICT_ENTRY(timedwait),
	_SYSTEMDICT_ENTRY(token),
	_SYSTEMDICT_ENTRY(true),
	_SYSTEMDICT_ENTRY(trylock),
	_SYSTEMDICT_ENTRY(type),
	_SYSTEMDICT_ENTRY(undef),
	_SYSTEMDICT_ENTRY(undefineresource),
	_SYSTEMDICT_ENTRY(unlock),
	_SYSTEMDICT_ENTRY(userdict),
	_SYSTEMDICT_ENTRY(userparams),
	_SYSTEMDICT_ENTRY(usertime),
	_SYSTEMDICT_ENTRY(version),
	_SYSTEMDICT_ENTRY(wait),
	_SYSTEMDICT_ENTRY(wcheck),
	_SYSTEMDICT_ENTRY(where),
	_SYSTEMDICT_ENTRY(write),
	_SYSTEMDICT_ENTRY(writestring),
	_SYSTEMDICT_ENTRY(xcheck),
	_SYSTEMDICT_ENTRY(xor),
	_SYSTEMDICT_ENTRY(yield)
};
#undef _SYSTEMDICT_ENTRY

void
systemdict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_uint32_t	i;
	cw_stilo_t	name, operator;	/* XXX GC-unsafe. */
#define NENTRIES							\
	((sizeof(systemdict_ops) / sizeof(struct cw_systemdict_entry)))

	stilo_dict_new(a_dict, a_stilt, NENTRIES);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stilt, systemdict_ops[i].name,
		    strlen(systemdict_ops[i].name), TRUE);
		stilo_operator_new(&operator, systemdict_ops[i].op_f);
		stilo_attrs_set(&operator, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stilt, &name, &operator);
	}

#undef NENTRIES
}

void
systemdict_abort(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_abs(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*a;

	stack = stilt_data_stack_get(a_stilt);
	
	a = stils_get(stack);
	if (stilo_type_get(a) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_integer_abs(a, a);
}

void
systemdict_add(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*a, *b;

	stack = stilt_data_stack_get(a_stilt);
	
	b = stils_get(stack);
	a = stils_down_get(stack, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_integer_add(a, b, a);
	stils_pop(stack);
}

void
systemdict_aload(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_anchorsearch(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_and(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_array(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	stack = stilt_data_stack_get(a_stilt);
	
	stilo = stils_get(stack);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);
	len = stilo_integer_get(stilo);
	if (len < 0)
		xep_throw(_CW_STILX_RANGECHECK);

	stilo_array_new(stilo, a_stilt, len);
}

void
systemdict_astore(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_begin(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo, *dict;

	dstack = stilt_dict_stack_get(a_stilt);
	ostack = stilt_data_stack_get(a_stilt);

	dict = stils_get(ostack);
	if (stilo_type_get(dict) != STILOT_DICT)
		xep_throw(_CW_STILX_TYPECHECK);
	
	stilo = stils_push(dstack);
	stilo_dup(stilo, dict);
	stils_pop(ostack);
}

void
systemdict_bind(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_bytesavailable(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*file;
	cw_uint32_t	bytes;

	stack = stilt_data_stack_get(a_stilt);

	file = stils_get(stack);
	
	if (stilo_type_get(file) != STILOT_FILE)
		xep_throw(_CW_STILX_TYPECHECK);

	bytes = stilo_file_buffer_count(file);
	stilo_integer_new(file, bytes);
}

void
systemdict_clear(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_uint32_t	count;

	stack = stilt_data_stack_get(a_stilt);
	count = stils_count(stack);
	if (count > 0)
		stils_npop(stack, count);
}

void
systemdict_cleardictstack(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cleartomark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_uint32_t	i, depth;

	stack = stilt_data_stack_get(a_stilt);

	i = 0;
	depth = stils_count(stack);
	if (depth > 0) {
		stilo = stils_get(stack);
		if (stilo_type_get(stilo) == STILOT_MARK)
			goto OUT;

		for (i = 1; i < depth; i++) {
			stilo = stils_down_get(stack, stilo);
			if (stilo_type_get(stilo) == STILOT_MARK)
				break;
		}
	}
	OUT:
	if (i == depth)
		xep_throw(_CW_STILX_UNMATCHEDMARK);

	stils_npop(stack, i + 1);
}

void
systemdict_closefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);

	stilo = stils_get(stack);
	if (stilo_type_get(stilo) != STILOT_FILE)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_file_close(stilo);

	stils_pop(stack);
}

void
systemdict_condition(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_copy(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_get(stack);

	switch (stilo_type_get(stilo)) {
	case STILOT_INTEGER: {
		cw_stilo_t	*dup;
		cw_sint64_t	count;

		/* Dup a range of the stack. */
		count = stilo_integer_get(stilo);
		if (count < 0)
			xep_throw(_CW_STILX_RANGECHECK);
		if (count > stils_count(stack) - 1)
			xep_throw(_CW_STILX_STACKUNDERFLOW);
		stils_pop(stack);

		if (count > 0) {
			cw_uint32_t	i;

			/*
			 * Iterate down the stack, creating dup's along the way.
			 * Since we're going down, it's necessary to use
			 * stils_under_push() to preserve order.
			 */

			stilo = stils_get(stack);
			i = 0;
			dup = stils_push(stack);
			stilo_dup(dup, stilo);
			if (count > 1) {
				
				for (i = 1; i < count; i++) {
					stilo = stils_down_get(stack, stilo);
					dup = stils_under_push(stack, dup);
					stilo_dup(dup, stilo);
				}
			}
		}
		break;
	}
	case STILOT_ARRAY:
	case STILOT_DICT:
	case STILOT_HOOK:
	case STILOT_STRING: {
		cw_stilo_t	*orig;

		orig = stils_down_get(stack, stilo);

		stilo_copy(stilo, orig, a_stilt);
		break;
	}
	default:
		xep_throw(_CW_STILX_TYPECHECK);
	}
}

void
systemdict_count(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);

	stilo = stils_push(stack);
	stilo_integer_new(stilo, stils_count(stack) - 1);
}

void
systemdict_countdictstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo;

	dstack = stilt_dict_stack_get(a_stilt);
	ostack = stilt_data_stack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_integer_new(stilo, stils_count(dstack));
}

void
systemdict_countexecstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*stilo;

	estack = stilt_exec_stack_get(a_stilt);
	ostack = stilt_data_stack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_integer_new(stilo, stils_count(estack));
}

void
systemdict_counttomark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_uint32_t	i, depth;

	stack = stilt_data_stack_get(a_stilt);

	i = 0;
	depth = stils_count(stack);
	if (depth > 0) {
		stilo = stils_get(stack);
		if (stilo_type_get(stilo) == STILOT_MARK)
			goto OUT;
		
		for (i = 1; i < depth; i++) {
			stilo = stils_down_get(stack, stilo);
			if (stilo_type_get(stilo) == STILOT_MARK)
				break;
		}
	}
	OUT:
	if (i == depth)
		xep_throw(_CW_STILX_UNMATCHEDMARK);

	stilo = stils_push(stack);
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

	ostack = stilt_data_stack_get(a_stilt);
	dstack = stilt_dict_stack_get(a_stilt);

	stilo = stils_push(ostack);
	stilo_dup(stilo, stils_get(dstack));
}

void
systemdict_currentfile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*file, *stilo;
	cw_uint32_t	i, depth;

	estack = stilt_exec_stack_get(a_stilt);
	ostack = stilt_data_stack_get(a_stilt);

	file = stils_push(ostack);
	i = 0;
	depth = stils_count(estack);
	if (depth > 0) {
		stilo = stils_get(estack);
		if (stilo_type_get(stilo) == STILOT_FILE) {
			stilo_dup(file, stilo);
			goto OUT;
		}

		for (i = 1; i < depth; i++) {
			stilo = stils_down_get(estack, stilo);
			if (stilo_type_get(stilo) == STILOT_FILE) {
				stilo_dup(file, stilo);
				break;
			}
		}
	}
	OUT:
	if (i == depth)
		stilo_file_new(file, a_stilt);
}

void
systemdict_currentglobal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_boolean_new(stilo, stilt_currentglobal(a_stilt));
}

void
systemdict_cvlit(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_get(stack);
	stilo_attrs_set(stilo, STILOA_LITERAL);
}

void
systemdict_cvm(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cvn(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cvrs(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cvs(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cvx(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_get(stack);
	stilo_attrs_set(stilo, STILOA_EXECUTABLE);
}

void
systemdict_def(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_defineresource(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_deletefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*string;
	cw_uint8_t	str[PATH_MAX];
	cw_uint32_t	nbytes;

	stack = stilt_data_stack_get(a_stilt);
	string = stils_get(stack);

	if (stilo_type_get(string) != STILOT_STRING)
		xep_throw(_CW_STILX_TYPECHECK);

	if (stilo_string_len_get(string) < sizeof(str))
		nbytes = stilo_string_len_get(string);
	else
		nbytes = sizeof(str) - 1;
	memcpy(str, stilo_string_get(string), nbytes);
	str[nbytes] = '\0';

	stils_pop(stack);

	if (unlink(str) == -1) {
		switch (errno) {
		case EACCES:
		case EPERM:
			xep_throw(_CW_STILX_INVALIDFILEACCESS);
		case EIO:
		case EBUSY:
			xep_throw(_CW_STILX_IOERROR);
		default:
			xep_throw(_CW_STILX_UNDEFINEDFILENAME);
		}
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
	_cw_error("XXX Not implemented");
}

void
systemdict_dictstack(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_div(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *a, *b;

	stack = stilt_data_stack_get(a_stilt);
	
	b = stils_get(stack);
	a = stils_down_get(stack, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_div(&t_stilo, b, a);
	stils_pop(stack);
}

void
systemdict_dup(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*orig, *dup;

	stack = stilt_data_stack_get(a_stilt);

	orig = stils_get(stack);
	dup = stils_push(stack);
	stilo_dup(dup, orig);
}

void
systemdict_end(cw_stilt_t *a_stilt)
{
	cw_stils_t	*dstack;

	dstack = stilt_dict_stack_get(a_stilt);

	if (stils_count(dstack) < 4)
		xep_throw(_CW_STILX_DICTSTACKUNDERFLOW);

	stils_pop(dstack);
}

void
systemdict_eq(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_errordict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_exch(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;

	stack = stilt_data_stack_get(a_stilt);

	stils_roll(stack, 2, 1);
}

void
systemdict_exec(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*orig, *new;

	ostack = stilt_data_stack_get(a_stilt);
	estack = stilt_exec_stack_get(a_stilt);

	orig = stils_get(ostack);
	new = stils_push(estack);
	stilo_dup(new, orig);
	stils_pop(ostack);

	stilt_loop(a_stilt);
}

void
systemdict_execstack(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_executeonly(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_exit(cw_stilt_t *a_stilt)
{
	xep_throw(_CW_STILX_EXIT);
}

void
systemdict_exp(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *a, *b;

	stack = stilt_data_stack_get(a_stilt);
	
	b = stils_get(stack);
	a = stils_down_get(stack, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_exp(&t_stilo, b, a);
	stils_pop(stack);
}

void
systemdict_false(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_boolean_new(stilo, FALSE);
}

void
systemdict_file(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*name, *flags, *file;

	stack = stilt_data_stack_get(a_stilt);
	
	flags = stils_get(stack);
	name = stils_down_get(stack, flags);
	if (stilo_type_get(name) != STILOT_STRING || stilo_type_get(flags) !=
	    STILOT_STRING)
		xep_throw(_CW_STILX_TYPECHECK);

	file = stils_push(stack);
	stilo_file_new(file, a_stilt);
	xep_begin();
	xep_try {
		stilo_file_open(file, stilo_string_get(name),
		    stilo_string_len_get(name), stilo_string_get(flags),
		    stilo_string_len_get(flags));
	}
	xep_catch(_CW_STILX_INVALIDFILEACCESS)
	xep_mcatch(_CW_STILX_IOERROR) {
		stils_npop(stack, 3);
	}
	xep_end();

	/* XXX Magic number. */
	stilo_file_buffer_size_set(file, 512);

	stils_roll(stack, 3, 1);
	stils_npop(stack, 2);
}

void
systemdict_filenameforall(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_fileposition(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*file;
	cw_sint64_t	position;

	stack = stilt_data_stack_get(a_stilt);

	file = stils_get(stack);
	
	if (stilo_type_get(file) != STILOT_FILE)
		xep_throw(_CW_STILX_TYPECHECK);

	position = stilo_file_position_get(file);
	stilo_integer_new(file, position);
}

void
systemdict_filter(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_findresource(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_flush(cw_stilt_t *a_stilt)
{
	stilo_file_buffer_flush(stilt_stdout_get(a_stilt));
}

void
systemdict_flushfile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*file;

	stack = stilt_data_stack_get(a_stilt);

	file = stils_get(stack);
	
	if (stilo_type_get(file) != STILOT_FILE)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_file_buffer_flush(file);
	
	stils_pop(stack);
}

void
systemdict_for(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*exec, *stilo, *tstilo;
	cw_sint64_t	i, inc, limit;

	ostack = stilt_data_stack_get(a_stilt);
	estack = stilt_exec_stack_get(a_stilt);

	exec = stils_get(ostack);

	stilo = stils_down_get(ostack, exec);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);
	limit = stilo_integer_get(stilo);

	stilo = stils_down_get(ostack, stilo);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);
	inc = stilo_integer_get(stilo);

	stilo = stils_down_get(ostack, stilo);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);
	i = stilo_integer_get(stilo);

	/*
	 * Use the execution stack for temporary storage of the stilo to be
	 * executed, in order to be GC-safe.
	 */
	tstilo = stils_push(estack);
	stilo_dup(tstilo, exec);

	stils_npop(ostack, 4);

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
				stilo = stils_push(estack);
				stilo_dup(stilo, tstilo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				stilo = stils_push(ostack);
				stilo_integer_new(stilo, i);

				stilt_loop(a_stilt);
			}
		} else {
			for (; i >= limit; i += inc) {
				/*
				 * Dup the object to execute onto the execution
				 * stack.
				 */
				stilo = stils_push(estack);
				stilo_dup(stilo, tstilo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				stilo = stils_push(ostack);
				stilo_integer_new(stilo, i);

				stilt_loop(a_stilt);
			}
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		while (stils_get(estack) != tstilo)
			stils_pop(estack);
	}
	xep_end();

	stils_pop(estack);
}

void
systemdict_forall(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
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
	_cw_error("XXX Not implemented");
}

void
systemdict_get(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*from, *with;

	stack = stilt_data_stack_get(a_stilt);
	with = stils_get(stack);
	from = stils_down_get(stack, with);

	switch (stilo_type_get(from)) {
	case STILOT_ARRAY: {
		cw_sint64_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER)
			xep_throw(_CW_STILX_TYPECHECK);
		index = stilo_integer_get(with);

		stilo_dup(with, stilo_array_el_get(from, index));

		stils_roll(stack, 2, 1);
		stils_pop(stack);
		break;
	}
	case STILOT_DICT: {
		cw_stilo_t	*val;

		val = stils_push(stack);
		if (stilo_dict_lookup(from, a_stilt, with, val)) {
			stils_pop(stack);
			xep_throw(_CW_STILX_UNDEFINED);
		}
		stils_roll(stack, 3, 1);
		stils_npop(stack, 2);
		break;
	}
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING: {
		cw_sint64_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER)
			xep_throw(_CW_STILX_TYPECHECK);
		index = stilo_integer_get(with);

		stilo_integer_set(with, (cw_sint64_t)*stilo_string_el_get(from,
		    index));

		stils_roll(stack, 2, 1);
		stils_pop(stack);
		break;
	}
	default:
		xep_throw(_CW_STILX_TYPECHECK);
	}
}

void
systemdict_getinterval(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*from, *with, *count;
	cw_sint64_t	index, len;

	stack = stilt_data_stack_get(a_stilt);
	count = stils_get(stack);
	with = stils_down_get(stack, count);
	from = stils_down_get(stack, with);

	if ((stilo_type_get(with) != STILOT_INTEGER) || (stilo_type_get(count)
	    != STILOT_INTEGER))
		xep_throw(_CW_STILX_TYPECHECK);
	index = stilo_integer_get(with);
	len = stilo_integer_get(count);

	switch (stilo_type_get(from)) {
	case STILOT_ARRAY:
		stilo_array_subarray_new(count, from, a_stilt, index, len);
		break;
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING:
		stilo_string_substring_new(count, from, a_stilt, index, len);
		break;
	default:
		xep_throw(_CW_STILX_TYPECHECK);
	}
	stils_roll(stack, 3, 1);
	stils_npop(stack, 2);
}

void
systemdict_globaldict(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);

	stilo = stils_push(stack);
	stilo_dup(stilo, stilt_globaldict_get(a_stilt));
}

void
systemdict_gt(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_if(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*cond, *exec;

	ostack = stilt_data_stack_get(a_stilt);
	exec = stils_get(ostack);
	cond = stils_down_get(ostack, exec);
	if (stilo_type_get(cond) != STILOT_BOOLEAN)
		xep_throw(_CW_STILX_TYPECHECK);

	if (stilo_boolean_get(cond)) {
		cw_stils_t	*estack;
		cw_stilo_t	*stilo;

		estack = stilt_exec_stack_get(a_stilt);
		stilo = stils_push(estack);
		stilo_move(stilo, exec);
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

	ostack = stilt_data_stack_get(a_stilt);
	estack = stilt_exec_stack_get(a_stilt);

	exec_else = stils_get(ostack);

	exec_if = stils_down_get(ostack, exec_else);

	cond = stils_down_get(ostack, exec_if);
	if (stilo_type_get(cond) != STILOT_BOOLEAN)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo = stils_push(estack);
	if (stilo_boolean_get(cond))
		stilo_move(stilo, exec_if);
	else
		stilo_move(stilo, exec_else);
	stils_npop(ostack, 3);
	stilt_loop(a_stilt);
}

void
systemdict_index(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo, *orig;
	cw_sint64_t	index;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_get(stack);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);
	index = stilo_integer_get(stilo);
	if (index < 0)
		xep_throw(_CW_STILX_RANGECHECK);

	orig = stils_nget(stack, index + 1);
	stilo_no_new(stilo);
	stilo_dup(stilo, orig);
}

void
systemdict_internaldict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_join(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_known(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_le(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_length(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	stack = stilt_data_stack_get(a_stilt);
	
	stilo = stils_get(stack);
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
		xep_throw(_CW_STILX_TYPECHECK);
	}

	stilo_integer_new(stilo, len);
}

void
systemdict_load(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_localinstancedict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_lock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_loop(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*exec, *stilo, *tstilo;

	ostack = stilt_data_stack_get(a_stilt);
	estack = stilt_exec_stack_get(a_stilt);

	exec = stils_get(ostack);

	/*
	 * Use the execution stack for temporary storage of the stilo to be
	 * executed, in order to be GC-safe.
	 */
	tstilo = stils_push(estack);
	stilo_dup(tstilo, exec);

	stils_pop(ostack);

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
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		while (stils_get(estack) != tstilo)
			stils_pop(estack);
	}
	xep_end();

	stils_pop(estack);
}

void
systemdict_lt(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_mark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_mark_new(stilo);
}

void
systemdict_maxlength(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_mod(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *a, *b;

	stack = stilt_data_stack_get(a_stilt);
	
	b = stils_get(stack);
	a = stils_down_get(stack, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_mod(&t_stilo, b, a);
	stils_pop(stack);
}

void
systemdict_mul(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *a, *b;

	stack = stilt_data_stack_get(a_stilt);
	
	b = stils_get(stack);
	a = stils_down_get(stack, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_mul(&t_stilo, b, a);
	stils_pop(stack);
}

void
systemdict_mutex(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_ne(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_neg(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*a;

	stack = stilt_data_stack_get(a_stilt);
	
	a = stils_get(stack);
	if (stilo_type_get(a) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_integer_neg(a, a);
}

void
systemdict_noaccess(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_not(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_notify(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_null(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_null_new(stilo);
}

void
systemdict_or(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_output(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_pop(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;

	stack = stilt_data_stack_get(a_stilt);

	stils_pop(stack);
}

void
systemdict_print(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo, *stdout_stilo;

	stack = stilt_data_stack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	stilo = stils_get(stack);
	stilo_print(stilo, stdout_stilo, FALSE, FALSE);
	stils_pop(stack);
}

void
systemdict_printobject(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_product(cw_stilt_t *a_stilt)
{
	soft_operator("`Canonware stil'");
}

void
systemdict_prompt(cw_stilt_t *a_stilt)
{
	soft_operator("`s> '");
}

void
systemdict_pstack(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_stils_t	*stack;
	cw_uint32_t	i, count;
	cw_uint8_t	code[] = "==";

	stilts_new(&stilts, a_stilt);
	stack = stilt_data_stack_get(a_stilt);
	count = stils_count(stack);

	for (i = 0; i < count; i++) {
		systemdict_dup(a_stilt);
		stilt_interpret(a_stilt, &stilts, code, sizeof(code) - 1);
		stilt_flush(a_stilt, &stilts);
		stils_roll(stack, count, 1);
	}
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_put(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*into, *with, *what;

	stack = stilt_data_stack_get(a_stilt);
	what = stils_get(stack);
	with = stils_down_get(stack, what);
	into = stils_down_get(stack, with);

	switch (stilo_type_get(into)) {
	case STILOT_ARRAY: {
		cw_sint64_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER)
			xep_throw(_CW_STILX_TYPECHECK);
		index = stilo_integer_get(with);

		stilo_array_el_set(into, what, index);
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
		    stilo_type_get(what) != STILOT_INTEGER)
			xep_throw(_CW_STILX_TYPECHECK);
		index = stilo_integer_get(with);
		val = stilo_integer_get(what);
		
		stilo_string_el_set(into, val, index);
		break;
	}
	default:
		xep_throw(_CW_STILX_TYPECHECK);
	}
	stils_npop(stack, 3);
}

void
systemdict_putinterval(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*into, *with, *what;
	cw_sint64_t	index;

	stack = stilt_data_stack_get(a_stilt);
	what = stils_get(stack);
	with = stils_down_get(stack, what);
	into = stils_down_get(stack, with);

	if (stilo_type_get(with) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);
	index = stilo_integer_get(with);

	switch (stilo_type_get(into)) {
	case STILOT_ARRAY: {
		cw_stilo_t	*arr;
		cw_uint32_t	len;

		arr = stilo_array_get(what);
		len = stilo_array_len_get(what);
		stilo_array_set(into, index, arr, len);
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
		stilo_string_set(into, index, str, len);
		break;
	}
	default:
		xep_throw(_CW_STILX_TYPECHECK);
	}
	stils_npop(stack, 3);
}

void
systemdict_quit(cw_stilt_t *a_stilt)
{
	xep_throw(_CW_STILX_QUIT);
}

void
systemdict_rand(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*num;

	stack = stilt_data_stack_get(a_stilt);

	num = stils_push(stack);
	stilo_cast(num, STILOT_INTEGER);
	stilo_integer_rand(num);
}

void
systemdict_rcheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_read(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*file, *value, *code;
	cw_uint8_t	val;
	cw_sint32_t	nread;

	stack = stilt_data_stack_get(a_stilt);

	file = stils_get(stack);
	if (stilo_type_get(file) != STILOT_FILE)
		xep_throw(_CW_STILX_TYPECHECK);

	value = stils_push(stack);
	code = stils_push(stack);

	nread = stilo_file_read(file, 1, &val);

	if (nread == 0) {
		stilo_integer_new(value, 0);
		stilo_boolean_new(code, FALSE);
	} else {
		stilo_integer_new(value, (cw_sint64_t)val);
		stilo_boolean_new(code, TRUE);
	}

	stils_roll(stack, 3, 2);
	stils_pop(stack);
}

void
systemdict_readonly(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_readstring(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*file, *string;
	cw_sint32_t	nread;

	stack = stilt_data_stack_get(a_stilt);

	string = stils_get(stack);
	file = stils_down_get(stack, string);

	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(string) !=
	    STILOT_STRING)
		xep_throw(_CW_STILX_TYPECHECK);

	nread = stilo_file_read(file, stilo_string_len_get(string),
	    stilo_string_get(string));

	if (nread == 0) {
		/* EOF. */
		stilo_boolean_new(file, TRUE);
		stils_pop(stack);
	} else if (nread < stilo_string_len_get(string)) {
		cw_stilo_t	*value, *code;

		/*
		 * We didn't fill the string, so we can't just use it as the
		 * result.  Create a copy.
		 */
		value = stils_under_push(stack, file);
		stilo_string_substring_new(value, string, a_stilt, 0, nread);

		code = stils_under_push(stack, file);
		stilo_boolean_new(code, FALSE);

		stils_npop(stack, 2);
	} else {
		stilo_boolean_new(file, FALSE);
		stils_roll(stack, 2, 1);
		stils_pop(stack);
	}
}

void
systemdict_realtime(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_renamefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*string_from, *string_to;
	cw_uint8_t	str_from[PATH_MAX], str_to[1024];
	cw_uint32_t	nbytes;

	stack = stilt_data_stack_get(a_stilt);
	string_to = stils_get(stack);
	string_from = stils_down_get(stack, string_to);

	if (stilo_type_get(string_from) != STILOT_STRING ||
	    stilo_type_get(string_to) != STILOT_STRING)
		xep_throw(_CW_STILX_TYPECHECK);

	if (stilo_string_len_get(string_from) >= sizeof(str_from))
		xep_throw(_CW_STILX_LIMITCHECK);
	nbytes = stilo_string_len_get(string_from);
	memcpy(str_from, stilo_string_get(string_from), nbytes);
	str_from[nbytes] = '\0';

	if (stilo_string_len_get(string_to) >= sizeof(str_to))
		xep_throw(_CW_STILX_LIMITCHECK);
	nbytes = stilo_string_len_get(string_to);
	memcpy(str_to, stilo_string_get(string_to), nbytes);
	str_to[nbytes] = '\0';

	stils_npop(stack, 2);

	if (rename(str_from, str_to) == -1) {
		switch (errno) {
		case EACCES:
		case EPERM:
		case EROFS:
		case EINVAL:
			xep_throw(_CW_STILX_INVALIDFILEACCESS);
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			xep_throw(_CW_STILX_UNDEFINEDFILENAME);
		default:
			xep_throw(_CW_STILX_IOERROR);
		}
	}
}

void
systemdict_repeat(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*count, *exec, *stilo, *tstilo;
	cw_sint64_t	i, cnt;

	ostack = stilt_data_stack_get(a_stilt);
	estack = stilt_exec_stack_get(a_stilt);

	exec = stils_get(ostack);
	count = stils_down_get(ostack, exec);
	if (stilo_type_get(count) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	/*
	 * Use the execution stack for temporary storage of the stilo to be
	 * executed, in order to be GC-safe.
	 */
	tstilo = stils_push(estack);
	stilo_dup(tstilo, exec);

	cnt = stilo_integer_get(count);
	stils_npop(ostack, 2);

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
		while (stils_get(estack) != tstilo)
			stils_pop(estack);
	}
	xep_end();

	stils_pop(estack);
}

void
systemdict_resourceforall(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_resourcestatus(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_roll(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint64_t	count, amount;

	stack = stilt_data_stack_get(a_stilt);

	stilo = stils_get(stack);
	amount = stilo_integer_get(stilo);
	stils_pop(stack);

	stilo = stils_get(stack);
	count = stilo_integer_get(stilo);
	stils_pop(stack);

	stils_roll(stack, count, amount);
}

void
systemdict_run(cw_stilt_t *a_stilt)
{
	soft_operator("`r' file cvx exec");
}

void
systemdict_search(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_serverdict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_setfileposition(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*file, *position;

	stack = stilt_data_stack_get(a_stilt);

	position = stils_get(stack);
	file = stils_down_get(stack, position);
	
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(position) !=
	    STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_file_position_set(file, stilo_integer_get(position));

	stils_npop(stack, 2);
}

void
systemdict_setglobal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_get(stack);
	stilt_setglobal(a_stilt, stilo_boolean_get(stilo));
	stils_pop(stack);
}

void
systemdict_shift(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_srand(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*seed;

	stack = stilt_data_stack_get(a_stilt);

	seed = stils_get(stack);
	stilo_integer_srand(seed);
	stils_pop(stack);
}

void
systemdict_stack(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_stils_t	*stack;
	cw_uint32_t	i, count;
	cw_uint8_t	code[] = "=";

	stilts_new(&stilts, a_stilt);
	stack = stilt_data_stack_get(a_stilt);
	count = stils_count(stack);

	for (i = 0; i < count; i++) {
		systemdict_dup(a_stilt);
		stilt_interpret(a_stilt, &stilts, code, sizeof(code) - 1);
		stilt_flush(a_stilt, &stilts);
		stils_roll(stack, count, 1);
	}
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_start(cw_stilt_t *a_stilt)
{
	cw_stilo_t	*file;

	file = stils_push(&a_stilt->exec_stils);
	stilo_dup(file, stil_stdin_get(a_stilt->stil));
	stilo_attrs_set(file, STILOA_EXECUTABLE);

	xep_begin();
	xep_try {
		stilt_loop(a_stilt);
	}
	xep_catch(_CW_STILX_QUIT) {
		cw_stils_t	*estack;
		cw_stilo_t	*stilo;
		cw_uint32_t	i, depth;

		/*
		 * Pop objects off the exec stack, up to and including file.
		 */
		estack = stilt_exec_stack_get(a_stilt);

		i = 0;
		depth = stils_count(estack);
		if (depth > 0) {
			stilo = stils_get(estack);
			if (stilo == file)
				goto OUT;

			for (i = 1; i < depth; i++) {
				stilo = stils_down_get(estack, stilo);
				if (stilo == file)
					break;
			}
		}
		OUT:
		_cw_assert(i < depth);
		stils_npop(estack, i + 1);

		xep_handled();
	}
	/* XXX Set up exception handling. */
	xep_end();
}

void
systemdict_status(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_statusdict(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_stdin(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo_stdin;

	stack = stilt_data_stack_get(a_stilt);
	stilo_stdin = stils_push(stack);

	stilo_dup(stilo_stdin, stilt_stdin_get(a_stilt));
}

void
systemdict_stderr(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo_stderr;

	stack = stilt_data_stack_get(a_stilt);
	stilo_stderr = stils_push(stack);

	stilo_dup(stilo_stderr, stilt_stderr_get(a_stilt));
}

void
systemdict_stdout(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo_stdout;

	stack = stilt_data_stack_get(a_stilt);
	stilo_stdout = stils_push(stack);

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

	ostack = stilt_data_stack_get(a_stilt);
	estack = stilt_exec_stack_get(a_stilt);
	
	exec = stils_get(ostack);
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
	_cw_error("XXX Not implemented");
}

void
systemdict_string(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	stack = stilt_data_stack_get(a_stilt);
	
	stilo = stils_get(stack);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);
	len = stilo_integer_get(stilo);
	if (len < 0)
		xep_throw(_CW_STILX_RANGECHECK);

	stilo_string_new(stilo, a_stilt, len);
}

void
systemdict_sub(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *a, *b;

	stack = stilt_data_stack_get(a_stilt);
	
	b = stils_get(stack);
	a = stils_down_get(stack, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_sub(&t_stilo, b, a);
	stils_pop(stack);
}

/* $error */
void
systemdict_sym_derror(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

/* = */
void
systemdict_sym_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo, *stdout_stilo;

	stack = stilt_data_stack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	stilo = stils_get(stack);
	stilo_print(stilo, stdout_stilo, FALSE, TRUE);
	stilo_file_buffer_flush(stdout_stilo);
	stils_pop(stack);
}

/* == */
void
systemdict_sym_eq_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo, *stdout_stilo;

	stack = stilt_data_stack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	stilo = stils_get(stack);
	stilo_print(stilo, stdout_stilo, TRUE, TRUE);
	stilo_file_buffer_flush(stdout_stilo);
	stils_pop(stack);
}

/* >> */
void
systemdict_sym_gt_gt(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

/* ] */
void
systemdict_sym_rb(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *stilo, *arr;	/* XXX GC-unsafe.*/
	cw_uint32_t	nelements, i, depth;

	stack = stilt_data_stack_get(a_stilt);
	/* Find the mark. */
	i = 0;
	depth = stils_count(stack);
	if (depth > 0) {
		stilo = stils_get(stack);
		if (stilo_type_get(stilo) == STILOT_MARK)
			goto OUT;

		for (i = 1; i < depth; i++) {
			stilo = stils_down_get(stack, stilo);
			if (stilo_type_get(stilo) == STILOT_MARK)
				break;
		}
	}
	OUT:
	if (i == depth)
		xep_throw(_CW_STILX_UNMATCHEDMARK);

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	stilo_array_new(&t_stilo, a_stilt, nelements);
	arr = stilo_array_get(&t_stilo);

	/*
	 * Traverse down the stack, moving stilo's to the array.
	 */
	i = nelements;
	if (i > 0) {
		stilo = stils_get(stack);
		stilo_move(&arr[i - 1], stilo);

		for (i--; i > 0; i--) {
			stilo = stils_down_get(stack, stilo);
			stilo_move(&arr[i - 1], stilo);
		}
	}
	
	/* Pop the stilo's off the stack now. */
	stils_npop(stack, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(stack);
	stilo_move(stilo, &t_stilo);
}

void
systemdict_systemdict(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);

	stilo = stils_push(stack);
	stilo_dup(stilo, stilt_systemdict_get(a_stilt));
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
systemdict_true(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_boolean_new(stilo, TRUE);
}

void
systemdict_trylock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_type(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_undef(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_undefineresource(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_unlock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_userdict(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);

	stilo = stils_push(stack);
	stilo_dup(stilo, stilt_userdict_get(a_stilt));
}

void
systemdict_userparams(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_usertime(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_version(cw_stilt_t *a_stilt)
{
	soft_operator("`" _LIBSTIL_VERSION "'");
}

void
systemdict_wait(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_wcheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_where(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_write(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*file, *value;
	cw_uint8_t	val;

	stack = stilt_data_stack_get(a_stilt);

	value = stils_get(stack);
	file = stils_down_get(stack, value);
	
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(value) !=
	    STILOT_INTEGER)
		xep_throw(_CW_STILX_TYPECHECK);

	val = (cw_uint8_t)stilo_integer_get(value);
	stilo_file_write(file, &val, 1);

	stils_npop(stack, 2);
}

void
systemdict_writestring(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*file, *string;

	stack = stilt_data_stack_get(a_stilt);
	string = stils_get(stack);
	file = stils_down_get(stack, string);

	if (stilo_type_get(string) != STILOT_STRING || stilo_type_get(file) !=
	    STILOT_FILE)
		xep_throw(_CW_STILX_TYPECHECK);

	stilo_file_write(file, stilo_string_get(string),
	    stilo_string_len_get(string));

	stils_npop(stack, 2);
}

void
systemdict_xcheck(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_stiloa_t	attr;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_get(stack);
	
	attr = stilo_attrs_get(stilo);
	stilo_clobber(stilo);

	if (attr == STILOA_EXECUTABLE)
		stilo_boolean_new(stilo, TRUE);
	else
		stilo_boolean_new(stilo, FALSE);
}

void
systemdict_xor(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_yield(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}
