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

struct cw_systemdict_entry {
	const cw_uint8_t	*name;
	cw_op_t			*op_f;
};

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
	_SYSTEMDICT_ENTRY(ceiling),
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
	_SYSTEMDICT_ENTRY(currentaccuracy),
	_SYSTEMDICT_ENTRY(currentbase),
	_SYSTEMDICT_ENTRY(currentcontext),
	_SYSTEMDICT_ENTRY(currentdict),
	_SYSTEMDICT_ENTRY(currentfile),
	_SYSTEMDICT_ENTRY(currentglobal),
	_SYSTEMDICT_ENTRY(currentmstate),
	_SYSTEMDICT_ENTRY(currentobjectformat),
	_SYSTEMDICT_ENTRY(currentpoint),
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
	_SYSTEMDICT_ENTRY(echo),
	_SYSTEMDICT_ENTRY(end),
	_SYSTEMDICT_ENTRY(eq),
	_SYSTEMDICT_ENTRY(exch),
	_SYSTEMDICT_ENTRY(exec),
	_SYSTEMDICT_ENTRY(execstack),
	_SYSTEMDICT_ENTRY(executeonly),
	_SYSTEMDICT_ENTRY(executive),
	_SYSTEMDICT_ENTRY(exit),
	_SYSTEMDICT_ENTRY(exp),
	_SYSTEMDICT_ENTRY(false),
	_SYSTEMDICT_ENTRY(file),
	_SYSTEMDICT_ENTRY(filenameforall),
	_SYSTEMDICT_ENTRY(fileposition),
	_SYSTEMDICT_ENTRY(filter),
	_SYSTEMDICT_ENTRY(findresource),
	_SYSTEMDICT_ENTRY(floor),
	_SYSTEMDICT_ENTRY(flush),
	_SYSTEMDICT_ENTRY(flushfile),
	_SYSTEMDICT_ENTRY(for),
	_SYSTEMDICT_ENTRY(forall),
	_SYSTEMDICT_ENTRY(fork),
	_SYSTEMDICT_ENTRY(gcheck),
	_SYSTEMDICT_ENTRY(ge),
	_SYSTEMDICT_ENTRY(get),
	_SYSTEMDICT_ENTRY(getinterval),
	_SYSTEMDICT_ENTRY(gt),
	_SYSTEMDICT_ENTRY(if),
	_SYSTEMDICT_ENTRY(ifelse),
	_SYSTEMDICT_ENTRY(index),
	_SYSTEMDICT_ENTRY(initmath),
	_SYSTEMDICT_ENTRY(join),
	_SYSTEMDICT_ENTRY(known),
	_SYSTEMDICT_ENTRY(le),
	_SYSTEMDICT_ENTRY(length),
	_SYSTEMDICT_ENTRY(load),
	_SYSTEMDICT_ENTRY(lock),
	_SYSTEMDICT_ENTRY(loop),
	_SYSTEMDICT_ENTRY(lt),
	_SYSTEMDICT_ENTRY(mark),
	_SYSTEMDICT_ENTRY(maxlength),
	_SYSTEMDICT_ENTRY(mod),
	_SYSTEMDICT_ENTRY(mrestore),
	_SYSTEMDICT_ENTRY(mrestoreall),
	_SYSTEMDICT_ENTRY(msave),
	_SYSTEMDICT_ENTRY(mstate),
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
	_SYSTEMDICT_ENTRY(readhexstring),
	_SYSTEMDICT_ENTRY(readline),
	_SYSTEMDICT_ENTRY(readonly),
	_SYSTEMDICT_ENTRY(readstring),
	_SYSTEMDICT_ENTRY(realtime),
	_SYSTEMDICT_ENTRY(renamefile),
	_SYSTEMDICT_ENTRY(repeat),
	_SYSTEMDICT_ENTRY(resetfile),
	_SYSTEMDICT_ENTRY(resourceforall),
	_SYSTEMDICT_ENTRY(resourcestatus),
	_SYSTEMDICT_ENTRY(roll),
	_SYSTEMDICT_ENTRY(round),
	_SYSTEMDICT_ENTRY(rrand),
	_SYSTEMDICT_ENTRY(run),
	_SYSTEMDICT_ENTRY(search),
	_SYSTEMDICT_ENTRY(setaccuracy),
	_SYSTEMDICT_ENTRY(setbase),
	_SYSTEMDICT_ENTRY(setfileposition),
	_SYSTEMDICT_ENTRY(setglobal),
	_SYSTEMDICT_ENTRY(setmstate),
	_SYSTEMDICT_ENTRY(setobjectformat),
	_SYSTEMDICT_ENTRY(setpoint),
	_SYSTEMDICT_ENTRY(shift),
	_SYSTEMDICT_ENTRY(sqrt),
	_SYSTEMDICT_ENTRY(srand),
	_SYSTEMDICT_ENTRY(stack),
	_SYSTEMDICT_ENTRY(start),
	_SYSTEMDICT_ENTRY(status),
	_SYSTEMDICT_ENTRY(stop),
	_SYSTEMDICT_ENTRY(stopped),
	_SYSTEMDICT_ENTRY(store),
	_SYSTEMDICT_ENTRY(string),
	_SYSTEMDICT_ENTRY(sub),
	{"=",	systemdict_sym_eq},
	{"==",	systemdict_sym_eq_eq},
	{">>",	systemdict_sym_gt_gt},
	{"[",	systemdict_mark},
	{"<<",	systemdict_mark},
	{"]",	systemdict_array},
	_SYSTEMDICT_ENTRY(timedwait),
	_SYSTEMDICT_ENTRY(token),
	_SYSTEMDICT_ENTRY(true),
	_SYSTEMDICT_ENTRY(trylock),
	_SYSTEMDICT_ENTRY(type),
	_SYSTEMDICT_ENTRY(undef),
	_SYSTEMDICT_ENTRY(undefineresource),
	_SYSTEMDICT_ENTRY(unlock),
	_SYSTEMDICT_ENTRY(usertime),
	_SYSTEMDICT_ENTRY(version),
	_SYSTEMDICT_ENTRY(wait),
	_SYSTEMDICT_ENTRY(wcheck),
	_SYSTEMDICT_ENTRY(where),
	_SYSTEMDICT_ENTRY(write),
	_SYSTEMDICT_ENTRY(writehexstring),
	_SYSTEMDICT_ENTRY(writeobject),
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
	cw_stilo_t	name, operator;
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
}

void
systemdict_abs(cw_stilt_t *a_stilt)
{
}

void
systemdict_add(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *a, *b;

	stack = stilt_data_stack_get(a_stilt);
	/* XXX Check depth of stack. */
	
	b = stils_get(stack, 0);
	/* XXX Check type of b. */
	a = stils_get_down(stack, b);
	/* XXX Check type of a. */
	stilo_no_new(&t_stilo);
	stilo_move(&t_stilo, a);
	stilo_number_add(&t_stilo, b, a);
	stils_pop(stack, a_stilt, 1);
	stilo_delete(&t_stilo, a_stilt);
}

void
systemdict_aload(cw_stilt_t *a_stilt)
{
}

void
systemdict_anchorsearch(cw_stilt_t *a_stilt)
{
}

void
systemdict_and(cw_stilt_t *a_stilt)
{
}

void
systemdict_array(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *stilo, *arr;
	cw_uint32_t	nelements, i;

	stack = stilt_data_stack_get(a_stilt);
	/* Find the mark. */
	for (i = 0, stilo = stils_get(stack, 0);
	     stilo != NULL && stilo_type_get(stilo) != STILOT_MARK;
	     i++, stilo = stils_get_down(stack, stilo));

	_cw_assert(stilo != NULL);

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	stilo_array_new(&t_stilo, a_stilt, nelements);
	arr = stilo_array_get(&t_stilo);

	/*
	 * Traverse up the stack, moving stilo's to the array.
	 */
	for (i = 0, stilo = stils_get_up(stack, stilo); i <
	    nelements; i++, stilo = stils_get_up(stack, stilo))
		stilo_move(&arr[i], stilo);

	/* Pop the stilo's off the stack now. */
	stils_pop(stack, a_stilt, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(stack);
	stilo_move(stilo, &t_stilo);

	/* Clean up. */
	stilo_delete(&t_stilo, a_stilt);
}

void
systemdict_astore(cw_stilt_t *a_stilt)
{
}

void
systemdict_begin(cw_stilt_t *a_stilt)
{
}

void
systemdict_bind(cw_stilt_t *a_stilt)
{
}

void
systemdict_bytesavailable(cw_stilt_t *a_stilt)
{
}

void
systemdict_ceiling(cw_stilt_t *a_stilt)
{
}

void
systemdict_clear(cw_stilt_t *a_stilt)
{
}

void
systemdict_cleardictstack(cw_stilt_t *a_stilt)
{
}

void
systemdict_cleartomark(cw_stilt_t *a_stilt)
{
}

void
systemdict_closefile(cw_stilt_t *a_stilt)
{
}

void
systemdict_condition(cw_stilt_t *a_stilt)
{
}

void
systemdict_copy(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*orig, *copy;

	stack = stilt_data_stack_get(a_stilt);
	/* XXX Check depth of stack. */

	orig = stils_get(stack, 0);
	copy = stils_push(stack);
	stilo_copy(copy, orig, a_stilt);
}

void
systemdict_count(cw_stilt_t *a_stilt)
{
}

void
systemdict_countdictstack(cw_stilt_t *a_stilt)
{
}

void
systemdict_countexecstack(cw_stilt_t *a_stilt)
{
}

void
systemdict_counttomark(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentaccuracy(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentbase(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentcontext(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentdict(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentfile(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentglobal(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentmstate(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentobjectformat(cw_stilt_t *a_stilt)
{
}

void
systemdict_currentpoint(cw_stilt_t *a_stilt)
{
}

void
systemdict_cvlit(cw_stilt_t *a_stilt)
{
}

void
systemdict_cvm(cw_stilt_t *a_stilt)
{
}

void
systemdict_cvn(cw_stilt_t *a_stilt)
{
}

void
systemdict_cvrs(cw_stilt_t *a_stilt)
{
}

void
systemdict_cvs(cw_stilt_t *a_stilt)
{
}

void
systemdict_cvx(cw_stilt_t *a_stilt)
{
}

void
systemdict_def(cw_stilt_t *a_stilt)
{
}

void
systemdict_defineresource(cw_stilt_t *a_stilt)
{
}

void
systemdict_deletefile(cw_stilt_t *a_stilt)
{
}

void
systemdict_detach(cw_stilt_t *a_stilt)
{
}

void
systemdict_dict(cw_stilt_t *a_stilt)
{
}

void
systemdict_dictstack(cw_stilt_t *a_stilt)
{
}

void
systemdict_div(cw_stilt_t *a_stilt)
{
}

void
systemdict_dup(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*orig, *dup;

	stack = stilt_data_stack_get(a_stilt);
	/* XXX Check depth of stack. */

	orig = stils_get(stack, 0);
	dup = stils_push(stack);
	stilo_dup(dup, orig, a_stilt);
}

void
systemdict_echo(cw_stilt_t *a_stilt)
{
}

void
systemdict_end(cw_stilt_t *a_stilt)
{
}

void
systemdict_eq(cw_stilt_t *a_stilt)
{
}

void
systemdict_exch(cw_stilt_t *a_stilt)
{
}

void
systemdict_exec(cw_stilt_t *a_stilt)
{
}

void
systemdict_execstack(cw_stilt_t *a_stilt)
{
}

void
systemdict_executeonly(cw_stilt_t *a_stilt)
{
}

void
systemdict_executive(cw_stilt_t *a_stilt)
{
}

void
systemdict_exit(cw_stilt_t *a_stilt)
{
}

void
systemdict_exp(cw_stilt_t *a_stilt)
{
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
}

void
systemdict_filenameforall(cw_stilt_t *a_stilt)
{
}

void
systemdict_fileposition(cw_stilt_t *a_stilt)
{
}

void
systemdict_filter(cw_stilt_t *a_stilt)
{
}

void
systemdict_findresource(cw_stilt_t *a_stilt)
{
}

void
systemdict_floor(cw_stilt_t *a_stilt)
{
}

void
systemdict_flush(cw_stilt_t *a_stilt)
{
}

void
systemdict_flushfile(cw_stilt_t *a_stilt)
{
}

void
systemdict_for(cw_stilt_t *a_stilt)
{
}

void
systemdict_forall(cw_stilt_t *a_stilt)
{
}

void
systemdict_fork(cw_stilt_t *a_stilt)
{
}

void
systemdict_gcheck(cw_stilt_t *a_stilt)
{
}

void
systemdict_ge(cw_stilt_t *a_stilt)
{
}

void
systemdict_get(cw_stilt_t *a_stilt)
{
}

void
systemdict_getinterval(cw_stilt_t *a_stilt)
{
}

void
systemdict_gt(cw_stilt_t *a_stilt)
{
}

void
systemdict_if(cw_stilt_t *a_stilt)
{
}

void
systemdict_ifelse(cw_stilt_t *a_stilt)
{
}

void
systemdict_index(cw_stilt_t *a_stilt)
{
}

void
systemdict_initmath(cw_stilt_t *a_stilt)
{
}

void
systemdict_join(cw_stilt_t *a_stilt)
{
}

void
systemdict_known(cw_stilt_t *a_stilt)
{
}

void
systemdict_le(cw_stilt_t *a_stilt)
{
}

void
systemdict_length(cw_stilt_t *a_stilt)
{
}

void
systemdict_load(cw_stilt_t *a_stilt)
{
}

void
systemdict_lock(cw_stilt_t *a_stilt)
{
}

void
systemdict_loop(cw_stilt_t *a_stilt)
{
}

void
systemdict_lt(cw_stilt_t *a_stilt)
{
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
}

void
systemdict_mod(cw_stilt_t *a_stilt)
{
}

void
systemdict_mrestore(cw_stilt_t *a_stilt)
{
}

void
systemdict_mrestoreall(cw_stilt_t *a_stilt)
{
}

void
systemdict_msave(cw_stilt_t *a_stilt)
{
}

void
systemdict_mstate(cw_stilt_t *a_stilt)
{
}

void
systemdict_mul(cw_stilt_t *a_stilt)
{
}

void
systemdict_mutex(cw_stilt_t *a_stilt)
{
}

void
systemdict_ne(cw_stilt_t *a_stilt)
{
}

void
systemdict_neg(cw_stilt_t *a_stilt)
{
}

void
systemdict_noaccess(cw_stilt_t *a_stilt)
{
}

void
systemdict_not(cw_stilt_t *a_stilt)
{
}

void
systemdict_notify(cw_stilt_t *a_stilt)
{
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
}

void
systemdict_pop(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;

	stack = stilt_data_stack_get(a_stilt);
	/* XXX Check depth of stack. */

	stils_pop(stack, a_stilt, 1);
}

void
systemdict_print(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stack = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	stilo = stils_get(stack, 0);
	/* XXX Make sure stilo is a string. */
	stilo_print(stilo, fd, FALSE, FALSE);
	stils_pop(stack, a_stilt, 1);
}

void
systemdict_printobject(cw_stilt_t *a_stilt)
{
}

void
systemdict_product(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_uint8_t	code[] = "`Canonware stil'\n";

	stilts_new(&stilts, a_stilt);
	stilt_interp_str(a_stilt, &stilts, code, sizeof(code) - 1);
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_prompt(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_uint8_t	code[] = "`s> '\n";

	stilts_new(&stilts, a_stilt);
	stilt_interp_str(a_stilt, &stilts, code, sizeof(code) - 1);
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_pstack(cw_stilt_t *a_stilt)
{
	/*
	 * XXX The correct implementation depends on stilo_p_*_copy() working.
	 */
#if (0)
	cw_stilts_t	stilts;
	cw_stils_t	*stack;
	cw_uint32_t	i, count;
	cw_uint8_t	code[] = "==\n";

	stilts_new(&stilts, a_stilt);
	stack = stilt_data_stack_get(a_stilt);
	count = stils_count(stack);

	for (i = 0; i < count; i++) {
		systemdict_dup(a_stilt);
		stilt_interp_str(a_stilt, &stilts, code, sizeof(code) - 1);
		stils_roll(stack, count, 1);
	}
	stilts_delete(&stilts, a_stilt);
#else
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stack = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	for (stilo = stils_get(stack, 0); stilo != NULL; stilo =
		 stils_get_down(stack, stilo))
		stilo_print(stilo, fd, TRUE, TRUE);
#endif
}

void
systemdict_put(cw_stilt_t *a_stilt)
{
}

void
systemdict_putinterval(cw_stilt_t *a_stilt)
{
}

void
systemdict_quit(cw_stilt_t *a_stilt)
{
}

void
systemdict_rand(cw_stilt_t *a_stilt)
{
}

void
systemdict_rcheck(cw_stilt_t *a_stilt)
{
}

void
systemdict_read(cw_stilt_t *a_stilt)
{
}

void
systemdict_readhexstring(cw_stilt_t *a_stilt)
{
}

void
systemdict_readline(cw_stilt_t *a_stilt)
{
}

void
systemdict_readonly(cw_stilt_t *a_stilt)
{
}

void
systemdict_readstring(cw_stilt_t *a_stilt)
{
}

void
systemdict_realtime(cw_stilt_t *a_stilt)
{
}

void
systemdict_renamefile(cw_stilt_t *a_stilt)
{
}

void
systemdict_repeat(cw_stilt_t *a_stilt)
{
}

void
systemdict_resetfile(cw_stilt_t *a_stilt)
{
}

void
systemdict_resourceforall(cw_stilt_t *a_stilt)
{
}

void
systemdict_resourcestatus(cw_stilt_t *a_stilt)
{
}

void
systemdict_roll(cw_stilt_t *a_stilt)
{
}

void
systemdict_round(cw_stilt_t *a_stilt)
{
}

void
systemdict_rrand(cw_stilt_t *a_stilt)
{
}

void
systemdict_run(cw_stilt_t *a_stilt)
{
}

void
systemdict_search(cw_stilt_t *a_stilt)
{
}

void
systemdict_setaccuracy(cw_stilt_t *a_stilt)
{
}

void
systemdict_setbase(cw_stilt_t *a_stilt)
{
}

void
systemdict_setfileposition(cw_stilt_t *a_stilt)
{
}

void
systemdict_setglobal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_get(stack, 0);
	stilt_setglobal(a_stilt, stilo_boolean_get(stilo));
	stils_pop(stack, a_stilt, 1);
}

void
systemdict_setmstate(cw_stilt_t *a_stilt)
{
}

void
systemdict_setobjectformat(cw_stilt_t *a_stilt)
{
}

void
systemdict_setpoint(cw_stilt_t *a_stilt)
{
}

void
systemdict_shift(cw_stilt_t *a_stilt)
{
}

void
systemdict_sqrt(cw_stilt_t *a_stilt)
{
}

void
systemdict_srand(cw_stilt_t *a_stilt)
{
}

void
systemdict_stack(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_stils_t	*stack;
	cw_uint32_t	i, count;
	cw_uint8_t	code[] = "=\n";

	stilts_new(&stilts, a_stilt);
	stack = stilt_data_stack_get(a_stilt);
	count = stils_count(stack);

	for (i = 0; i < count; i++) {
		systemdict_dup(a_stilt);
		stilt_interp_str(a_stilt, &stilts, code, sizeof(code) - 1);
		stils_roll(stack, count, 1);
	}
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_start(cw_stilt_t *a_stilt)
{
}

void
systemdict_status(cw_stilt_t *a_stilt)
{
}

void
systemdict_stop(cw_stilt_t *a_stilt)
{
}

void
systemdict_stopped(cw_stilt_t *a_stilt)
{
}

void
systemdict_store(cw_stilt_t *a_stilt)
{
}

void
systemdict_string(cw_stilt_t *a_stilt)
{
}

void
systemdict_sub(cw_stilt_t *a_stilt)
{
}

/* = */
void
systemdict_sym_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stack = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	stilo = stils_get(stack, 0);
	stilo_print(stilo, fd, FALSE, TRUE);
	stils_pop(stack, a_stilt, 1);
}

/* == */
void
systemdict_sym_eq_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stack = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	stilo = stils_get(stack, 0);
	stilo_print(stilo, fd, TRUE, TRUE);
	stils_pop(stack, a_stilt, 1);
}

/* >> */
void
systemdict_sym_gt_gt(cw_stilt_t *a_stilt)
{
}

void
systemdict_timedwait(cw_stilt_t *a_stilt)
{
}

void
systemdict_token(cw_stilt_t *a_stilt)
{
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
}

void
systemdict_type(cw_stilt_t *a_stilt)
{
}

void
systemdict_undef(cw_stilt_t *a_stilt)
{
}

void
systemdict_undefineresource(cw_stilt_t *a_stilt)
{
}

void
systemdict_unlock(cw_stilt_t *a_stilt)
{
}

void
systemdict_usertime(cw_stilt_t *a_stilt)
{
}

void
systemdict_version(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_uint8_t	code[] = "`" _LIBSTIL_VERSION "'\n";

	stilts_new(&stilts, a_stilt);
	stilt_interp_str(a_stilt, &stilts, code, sizeof(code) - 1);
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_wait(cw_stilt_t *a_stilt)
{
}

void
systemdict_wcheck(cw_stilt_t *a_stilt)
{
}

void
systemdict_where(cw_stilt_t *a_stilt)
{
}

void
systemdict_write(cw_stilt_t *a_stilt)
{
}

void
systemdict_writehexstring(cw_stilt_t *a_stilt)
{
}

void
systemdict_writeobject(cw_stilt_t *a_stilt)
{
}

void
systemdict_writestring(cw_stilt_t *a_stilt)
{
}

void
systemdict_xcheck(cw_stilt_t *a_stilt)
{
}

void
systemdict_xor(cw_stilt_t *a_stilt)
{
}

void
systemdict_yield(cw_stilt_t *a_stilt)
{
}
