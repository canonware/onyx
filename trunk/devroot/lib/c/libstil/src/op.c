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

/*
 * Array of operators in systemdict.
 */
struct cw_stil_op_dict_entry {
	const cw_uint8_t	*name;
	cw_op_t			*op_f;
};

#define DENTRY(name)	{#name, op_##name}

static struct cw_stil_op_dict_entry systemdict_ops[] = {
	DENTRY(abort),
	DENTRY(abs),
	DENTRY(add),
	DENTRY(aload),
	DENTRY(anchorsearch),
	DENTRY(and),
	DENTRY(array),
	DENTRY(astore),
	DENTRY(begin),
	DENTRY(bind),
	DENTRY(bytesavailable),
	DENTRY(ceiling),
	DENTRY(clear),
	DENTRY(cleardictstack),
	DENTRY(cleartomark),
	DENTRY(closefile),
	DENTRY(condition),
	DENTRY(copy),
	DENTRY(count),
	DENTRY(countdictstack),
	DENTRY(countexecstack),
	DENTRY(counttomark),
	DENTRY(currentaccuracy),
	DENTRY(currentbase),
	DENTRY(currentcontext),
	DENTRY(currentdict),
	DENTRY(currentfile),
	DENTRY(currentglobal),
	DENTRY(currentmstate),
	DENTRY(currentobjectformat),
	DENTRY(currentpacking),
	DENTRY(currentpoint),
	DENTRY(cvlit),
	DENTRY(cvm),
	DENTRY(cvn),
	DENTRY(cvrs),
	DENTRY(cvs),
	DENTRY(cvx),
	DENTRY(def),
	DENTRY(defineresource),
	DENTRY(deletefile),
	DENTRY(detach),
	DENTRY(dict),
	DENTRY(dictstack),
	DENTRY(div),
	DENTRY(dup),
	DENTRY(echo),
	DENTRY(end),
	DENTRY(eq),
	DENTRY(exch),
	DENTRY(exec),
	DENTRY(execstack),
	DENTRY(executeonly),
	DENTRY(executive),
	DENTRY(exit),
	DENTRY(exp),
	DENTRY(false),
	DENTRY(file),
	DENTRY(filenameforall),
	DENTRY(fileposition),
	DENTRY(filter),
	DENTRY(findresource),
	DENTRY(floor),
	DENTRY(flush),
	DENTRY(flushfile),
	DENTRY(for),
	DENTRY(forall),
	DENTRY(fork),
	DENTRY(gcheck),
	DENTRY(ge),
	DENTRY(get),
	DENTRY(getinterval),
	DENTRY(gt),
	DENTRY(if),
	DENTRY(ifelse),
	DENTRY(index),
	DENTRY(initmath),
	DENTRY(join),
	DENTRY(known),
	DENTRY(le),
	DENTRY(length),
	DENTRY(load),
	DENTRY(lock),
	DENTRY(loop),
	DENTRY(lt),
	DENTRY(mark),
	DENTRY(maxlength),
	DENTRY(mod),
	DENTRY(mrestore),
	DENTRY(mrestoreall),
	DENTRY(msave),
	DENTRY(mstate),
	DENTRY(mul),
	DENTRY(mutex),
	DENTRY(ne),
	DENTRY(neg),
	DENTRY(noaccess),
	DENTRY(not),
	DENTRY(notify),
	DENTRY(null),
	DENTRY(or),
	DENTRY(packedarray),
	DENTRY(pop),
	DENTRY(print),
	DENTRY(printobject),
	DENTRY(product),
	DENTRY(prompt),
	DENTRY(pstack),
	DENTRY(put),
	DENTRY(putinterval),
	DENTRY(quit),
	DENTRY(rand),
	DENTRY(rcheck),
	DENTRY(read),
	DENTRY(readhexstring),
	DENTRY(readline),
	DENTRY(readonly),
	DENTRY(readstring),
	DENTRY(realtime),
	DENTRY(renamefile),
	DENTRY(repeat),
	DENTRY(resetfile),
	DENTRY(resourceforall),
	DENTRY(resourcestatus),
	DENTRY(roll),
	DENTRY(round),
	DENTRY(rrand),
	DENTRY(run),
	DENTRY(search),
	DENTRY(setaccuracy),
	DENTRY(setbase),
	DENTRY(setfileposition),
	DENTRY(setglobal),
	DENTRY(setmstate),
	DENTRY(setobjectformat),
	DENTRY(setpacking),
	DENTRY(setpoint),
	DENTRY(shift),
	DENTRY(sqrt),
	DENTRY(srand),
	DENTRY(stack),
	DENTRY(start),
	DENTRY(status),
	DENTRY(stop),
	DENTRY(stopped),
	DENTRY(store),
	DENTRY(string),
	DENTRY(sub),
	{"=",	op_sym_eq},
	{"==",	op_sym_eq_eq},
	{">>",	op_sym_gt_gt},
	{"[",	op_sym_lb},
	{"<<",	op_sym_lt_lt},
	{"]",	op_sym_rb},
	DENTRY(timedwait),
	DENTRY(token),
	DENTRY(true),
	DENTRY(trylock),
	DENTRY(type),
	DENTRY(undef),
	DENTRY(undefineresource),
	DENTRY(unlock),
	DENTRY(usertime),
	DENTRY(version),
	DENTRY(wait),
	DENTRY(wcheck),
	DENTRY(where),
	DENTRY(write),
	DENTRY(writehexstring),
	DENTRY(writeobject),
	DENTRY(writestring),
	DENTRY(xcheck),
	DENTRY(xor),
	DENTRY(yield)
};

void
stil_op_systemdict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_uint32_t	i;
	cw_stilo_t	name, operator;
#define NENTRIES							\
	((sizeof(systemdict_ops) / sizeof(struct cw_stil_op_dict_entry)))

	stilo_dict_new(a_dict, a_stilt, NENTRIES);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stilt, systemdict_ops[i].name,
		    strlen(systemdict_ops[i].name), TRUE);
		stilo_operator_new(&operator, systemdict_ops[i].op_f);

		stilo_dict_def(a_dict, a_stilt, &name, &operator);
	}

#undef NENTRIES
}

void
op_abort(cw_stilt_t *a_stilt)
{
}

void
op_abs(cw_stilt_t *a_stilt)
{
}

void
op_add(cw_stilt_t *a_stilt)
{
}

void
op_aload(cw_stilt_t *a_stilt)
{
}

void
op_anchorsearch(cw_stilt_t *a_stilt)
{
}

void
op_and(cw_stilt_t *a_stilt)
{
}

void
op_array(cw_stilt_t *a_stilt)
{
}

void
op_astore(cw_stilt_t *a_stilt)
{
}

void
op_begin(cw_stilt_t *a_stilt)
{
}

void
op_bind(cw_stilt_t *a_stilt)
{
}

void
op_bytesavailable(cw_stilt_t *a_stilt)
{
}

void
op_ceiling(cw_stilt_t *a_stilt)
{
}

void
op_clear(cw_stilt_t *a_stilt)
{
}

void
op_cleardictstack(cw_stilt_t *a_stilt)
{
}

void
op_cleartomark(cw_stilt_t *a_stilt)
{
}

void
op_closefile(cw_stilt_t *a_stilt)
{
}

void
op_condition(cw_stilt_t *a_stilt)
{
}

void
op_copy(cw_stilt_t *a_stilt)
{
}

void
op_count(cw_stilt_t *a_stilt)
{
}

void
op_countdictstack(cw_stilt_t *a_stilt)
{
}

void
op_countexecstack(cw_stilt_t *a_stilt)
{
}

void
op_counttomark(cw_stilt_t *a_stilt)
{
}

void
op_currentaccuracy(cw_stilt_t *a_stilt)
{
}

void
op_currentbase(cw_stilt_t *a_stilt)
{
}

void
op_currentcontext(cw_stilt_t *a_stilt)
{
}

void
op_currentdict(cw_stilt_t *a_stilt)
{
}

void
op_currentfile(cw_stilt_t *a_stilt)
{
}

void
op_currentglobal(cw_stilt_t *a_stilt)
{
}

void
op_currentmstate(cw_stilt_t *a_stilt)
{
}

void
op_currentobjectformat(cw_stilt_t *a_stilt)
{
}

void
op_currentpacking(cw_stilt_t *a_stilt)
{
}

void
op_currentpoint(cw_stilt_t *a_stilt)
{
}

void
op_cvlit(cw_stilt_t *a_stilt)
{
}

void
op_cvm(cw_stilt_t *a_stilt)
{
}

void
op_cvn(cw_stilt_t *a_stilt)
{
}

void
op_cvrs(cw_stilt_t *a_stilt)
{
}

void
op_cvs(cw_stilt_t *a_stilt)
{
}

void
op_cvx(cw_stilt_t *a_stilt)
{
}

void
op_def(cw_stilt_t *a_stilt)
{
}

void
op_defineresource(cw_stilt_t *a_stilt)
{
}

void
op_deletefile(cw_stilt_t *a_stilt)
{
}

void
op_detach(cw_stilt_t *a_stilt)
{
}

void
op_dict(cw_stilt_t *a_stilt)
{
}

void
op_dictstack(cw_stilt_t *a_stilt)
{
}

void
op_div(cw_stilt_t *a_stilt)
{
}

void
op_dup(cw_stilt_t *a_stilt)
{
}

void
op_echo(cw_stilt_t *a_stilt)
{
}

void
op_end(cw_stilt_t *a_stilt)
{
}

void
op_eq(cw_stilt_t *a_stilt)
{
}

void
op_exch(cw_stilt_t *a_stilt)
{
}

void
op_exec(cw_stilt_t *a_stilt)
{
}

void
op_execstack(cw_stilt_t *a_stilt)
{
}

void
op_executeonly(cw_stilt_t *a_stilt)
{
}

void
op_executive(cw_stilt_t *a_stilt)
{
}

void
op_exit(cw_stilt_t *a_stilt)
{
}

void
op_exp(cw_stilt_t *a_stilt)
{
}

void
op_false(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_boolean_new(stilo, FALSE);
}

void
op_file(cw_stilt_t *a_stilt)
{
}

void
op_filenameforall(cw_stilt_t *a_stilt)
{
}

void
op_fileposition(cw_stilt_t *a_stilt)
{
}

void
op_filter(cw_stilt_t *a_stilt)
{
}

void
op_findresource(cw_stilt_t *a_stilt)
{
}

void
op_floor(cw_stilt_t *a_stilt)
{
}

void
op_flush(cw_stilt_t *a_stilt)
{
}

void
op_flushfile(cw_stilt_t *a_stilt)
{
}

void
op_for(cw_stilt_t *a_stilt)
{
}

void
op_forall(cw_stilt_t *a_stilt)
{
}

void
op_fork(cw_stilt_t *a_stilt)
{
}

void
op_gcheck(cw_stilt_t *a_stilt)
{
}

void
op_ge(cw_stilt_t *a_stilt)
{
}

void
op_get(cw_stilt_t *a_stilt)
{
}

void
op_getinterval(cw_stilt_t *a_stilt)
{
}

void
op_gt(cw_stilt_t *a_stilt)
{
}

void
op_if(cw_stilt_t *a_stilt)
{
}

void
op_ifelse(cw_stilt_t *a_stilt)
{
}

void
op_index(cw_stilt_t *a_stilt)
{
}

void
op_initmath(cw_stilt_t *a_stilt)
{
}

void
op_join(cw_stilt_t *a_stilt)
{
}

void
op_known(cw_stilt_t *a_stilt)
{
}

void
op_le(cw_stilt_t *a_stilt)
{
}

void
op_length(cw_stilt_t *a_stilt)
{
}

void
op_load(cw_stilt_t *a_stilt)
{
}

void
op_lock(cw_stilt_t *a_stilt)
{
}

void
op_loop(cw_stilt_t *a_stilt)
{
}

void
op_lt(cw_stilt_t *a_stilt)
{
}

void
op_mark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_mark_new(stilo);
}

void
op_maxlength(cw_stilt_t *a_stilt)
{
}

void
op_mod(cw_stilt_t *a_stilt)
{
}

void
op_mrestore(cw_stilt_t *a_stilt)
{
}

void
op_mrestoreall(cw_stilt_t *a_stilt)
{
}

void
op_msave(cw_stilt_t *a_stilt)
{
}

void
op_mstate(cw_stilt_t *a_stilt)
{
}

void
op_mul(cw_stilt_t *a_stilt)
{
}

void
op_mutex(cw_stilt_t *a_stilt)
{
}

void
op_ne(cw_stilt_t *a_stilt)
{
}

void
op_neg(cw_stilt_t *a_stilt)
{
}

void
op_noaccess(cw_stilt_t *a_stilt)
{
}

void
op_not(cw_stilt_t *a_stilt)
{
}

void
op_notify(cw_stilt_t *a_stilt)
{
}

void
op_null(cw_stilt_t *a_stilt)
{
}

void
op_or(cw_stilt_t *a_stilt)
{
}

void
op_packedarray(cw_stilt_t *a_stilt)
{
}

void
op_pop(cw_stilt_t *a_stilt)
{
}

void
op_print(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stack = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	stilo = stils_get(stack, 0);
	/* XXX Make sure stilo is a string. */
	stilo_print(stilo, fd, FALSE, FALSE);
}

void
op_printobject(cw_stilt_t *a_stilt)
{
}

void
op_product(cw_stilt_t *a_stilt)
{
}

void
op_prompt(cw_stilt_t *a_stilt)
{
}

void
op_pstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stack = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	for (stilo = stils_get(stack, 0); stilo != NULL; stilo =
		 stils_get_down(stack, stilo))
		stilo_print(stilo, fd, TRUE, TRUE);
}

void
op_put(cw_stilt_t *a_stilt)
{
}

void
op_putinterval(cw_stilt_t *a_stilt)
{
}

void
op_quit(cw_stilt_t *a_stilt)
{
}

void
op_rand(cw_stilt_t *a_stilt)
{
}

void
op_rcheck(cw_stilt_t *a_stilt)
{
}

void
op_read(cw_stilt_t *a_stilt)
{
}

void
op_readhexstring(cw_stilt_t *a_stilt)
{
}

void
op_readline(cw_stilt_t *a_stilt)
{
}

void
op_readonly(cw_stilt_t *a_stilt)
{
}

void
op_readstring(cw_stilt_t *a_stilt)
{
}

void
op_realtime(cw_stilt_t *a_stilt)
{
}

void
op_renamefile(cw_stilt_t *a_stilt)
{
}

void
op_repeat(cw_stilt_t *a_stilt)
{
}

void
op_resetfile(cw_stilt_t *a_stilt)
{
}

void
op_resourceforall(cw_stilt_t *a_stilt)
{
}

void
op_resourcestatus(cw_stilt_t *a_stilt)
{
}

void
op_roll(cw_stilt_t *a_stilt)
{
}

void
op_round(cw_stilt_t *a_stilt)
{
}

void
op_rrand(cw_stilt_t *a_stilt)
{
}

void
op_run(cw_stilt_t *a_stilt)
{
}

void
op_search(cw_stilt_t *a_stilt)
{
}

void
op_setaccuracy(cw_stilt_t *a_stilt)
{
}

void
op_setbase(cw_stilt_t *a_stilt)
{
}

void
op_setfileposition(cw_stilt_t *a_stilt)
{
}

void
op_setglobal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_get(stack, 0);
	stilt_setglobal(a_stilt, stilo_boolean_get(stilo));
	stils_pop(stack, a_stilt, 1);
}

void
op_setmstate(cw_stilt_t *a_stilt)
{
}

void
op_setobjectformat(cw_stilt_t *a_stilt)
{
}

void
op_setpacking(cw_stilt_t *a_stilt)
{
}

void
op_setpoint(cw_stilt_t *a_stilt)
{
}

void
op_shift(cw_stilt_t *a_stilt)
{
}

void
op_sqrt(cw_stilt_t *a_stilt)
{
}

void
op_srand(cw_stilt_t *a_stilt)
{
}

void
op_stack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stack = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	for (stilo = stils_get(stack, 0); stilo != NULL; stilo =
		 stils_get_down(stack, stilo))
		stilo_print(stilo, fd, FALSE, TRUE);
}

void
op_start(cw_stilt_t *a_stilt)
{
}

void
op_status(cw_stilt_t *a_stilt)
{
}

void
op_stop(cw_stilt_t *a_stilt)
{
}

void
op_stopped(cw_stilt_t *a_stilt)
{
}

void
op_store(cw_stilt_t *a_stilt)
{
}

void
op_string(cw_stilt_t *a_stilt)
{
}

void
op_sub(cw_stilt_t *a_stilt)
{
}

void
op_sym_eq(cw_stilt_t *a_stilt)
{
}

void
op_sym_eq_eq(cw_stilt_t *a_stilt)
{
}

void
op_sym_gt_gt(cw_stilt_t *a_stilt)
{
}

/* [ */
void
op_sym_lb(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_mark_new(stilo);
}

/* << */
void
op_sym_lt_lt(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_mark_new(stilo);
}

/* ] */
void
op_sym_rb(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	t_stilo, *stilo, *arr;
	cw_uint32_t	nelements, i;

	stack = stilt_data_stack_get(a_stilt);
	/* Find the mark. */
	for (i = 0, stilo = stils_get(stack, 0);
	     stilo != NULL && stilo_type_get(stilo) != _CW_STILOT_MARKTYPE;
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
op_timedwait(cw_stilt_t *a_stilt)
{
}

void
op_token(cw_stilt_t *a_stilt)
{
}

void
op_true(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stack;
	cw_stilo_t	*stilo;

	stack = stilt_data_stack_get(a_stilt);
	stilo = stils_push(stack);
	stilo_boolean_new(stilo, TRUE);
}

void
op_trylock(cw_stilt_t *a_stilt)
{
}

void
op_type(cw_stilt_t *a_stilt)
{
}

void
op_undef(cw_stilt_t *a_stilt)
{
}

void
op_undefineresource(cw_stilt_t *a_stilt)
{
}

void
op_unlock(cw_stilt_t *a_stilt)
{
}

void
op_usertime(cw_stilt_t *a_stilt)
{
}

void
op_version(cw_stilt_t *a_stilt)
{
}

void
op_wait(cw_stilt_t *a_stilt)
{
}

void
op_wcheck(cw_stilt_t *a_stilt)
{
}

void
op_where(cw_stilt_t *a_stilt)
{
}

void
op_write(cw_stilt_t *a_stilt)
{
}

void
op_writehexstring(cw_stilt_t *a_stilt)
{
}

void
op_writeobject(cw_stilt_t *a_stilt)
{
}

void
op_writestring(cw_stilt_t *a_stilt)
{
}

void
op_xcheck(cw_stilt_t *a_stilt)
{
}

void
op_xor(cw_stilt_t *a_stilt)
{
}

void
op_yield(cw_stilt_t *a_stilt)
{
}
