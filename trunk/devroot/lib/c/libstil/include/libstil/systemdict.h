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

void	systemdict_abs(cw_stilo_t *a_thread);
void	systemdict_add(cw_stilo_t *a_thread);
void	systemdict_aload(cw_stilo_t *a_thread);
void	systemdict_and(cw_stilo_t *a_thread);
void	systemdict_array(cw_stilo_t *a_thread);
void	systemdict_astore(cw_stilo_t *a_thread);
void	systemdict_begin(cw_stilo_t *a_thread);
void	systemdict_bind(cw_stilo_t *a_thread);
void	systemdict_broadcast(cw_stilo_t *a_thread);
void	systemdict_bytesavailable(cw_stilo_t *a_thread);
void	systemdict_chmod(cw_stilo_t *a_thread);
void	systemdict_chown(cw_stilo_t *a_thread);
void	systemdict_clear(cw_stilo_t *a_thread);
void	systemdict_cleardstack(cw_stilo_t *a_thread);
void	systemdict_cleartomark(cw_stilo_t *a_thread);
void	systemdict_closefile(cw_stilo_t *a_thread);
void	systemdict_condition(cw_stilo_t *a_thread);
void	systemdict_copy(cw_stilo_t *a_thread);
void	systemdict_count(cw_stilo_t *a_thread);
void	systemdict_countdstack(cw_stilo_t *a_thread);
void	systemdict_countestack(cw_stilo_t *a_thread);
void	systemdict_counttomark(cw_stilo_t *a_thread);
void	systemdict_currentdict(cw_stilo_t *a_thread);
void	systemdict_currentfile(cw_stilo_t *a_thread);
void	systemdict_currentlocking(cw_stilo_t *a_thread);
void	systemdict_cvlit(cw_stilo_t *a_thread);
void	systemdict_cvn(cw_stilo_t *a_thread);
void	systemdict_cvrs(cw_stilo_t *a_thread);
void	systemdict_cvs(cw_stilo_t *a_thread);
void	systemdict_cvx(cw_stilo_t *a_thread);
void	systemdict_def(cw_stilo_t *a_thread);
void	systemdict_detach(cw_stilo_t *a_thread);
void	systemdict_dict(cw_stilo_t *a_thread);
void	systemdict_dirforeach(cw_stilo_t *a_thread);
void	systemdict_div(cw_stilo_t *a_thread);
void	systemdict_dstack(cw_stilo_t *a_thread);
void	systemdict_dup(cw_stilo_t *a_thread);
void	systemdict_end(cw_stilo_t *a_thread);
void	systemdict_eq(cw_stilo_t *a_thread);
void	systemdict_estack(cw_stilo_t *a_thread);
void	systemdict_eval(cw_stilo_t *a_thread);
void	systemdict_exch(cw_stilo_t *a_thread);
void	systemdict_exec(cw_stilo_t *a_thread);
void	systemdict_exit(cw_stilo_t *a_thread);
void	systemdict_exp(cw_stilo_t *a_thread);
void	systemdict_fino(cw_stilo_t *a_thread);
void	systemdict_flush(cw_stilo_t *a_thread);
void	systemdict_flushfile(cw_stilo_t *a_thread);
void	systemdict_for(cw_stilo_t *a_thread);
void	systemdict_foreach(cw_stilo_t *a_thread);
void	systemdict_fork(cw_stilo_t *a_thread);
void	systemdict_ge(cw_stilo_t *a_thread);
void	systemdict_get(cw_stilo_t *a_thread);
void	systemdict_getinterval(cw_stilo_t *a_thread);
void	systemdict_gt(cw_stilo_t *a_thread);
void	systemdict_handleerror(cw_stilo_t *a_thread);
void	systemdict_if(cw_stilo_t *a_thread);
void	systemdict_ifelse(cw_stilo_t *a_thread);
void	systemdict_index(cw_stilo_t *a_thread);
void	systemdict_join(cw_stilo_t *a_thread);
void	systemdict_known(cw_stilo_t *a_thread);
void	systemdict_lcheck(cw_stilo_t *a_thread);
void	systemdict_le(cw_stilo_t *a_thread);
void	systemdict_length(cw_stilo_t *a_thread);
void	systemdict_link(cw_stilo_t *a_thread);
void	systemdict_load(cw_stilo_t *a_thread);
void	systemdict_lock(cw_stilo_t *a_thread);
void	systemdict_loop(cw_stilo_t *a_thread);
void	systemdict_lt(cw_stilo_t *a_thread);
void	systemdict_mark(cw_stilo_t *a_thread);
void	systemdict_mkdir(cw_stilo_t *a_thread);
void	systemdict_mod(cw_stilo_t *a_thread);
void	systemdict_mul(cw_stilo_t *a_thread);
void	systemdict_mutex(cw_stilo_t *a_thread);
void	systemdict_ne(cw_stilo_t *a_thread);
void	systemdict_neg(cw_stilo_t *a_thread);
void	systemdict_not(cw_stilo_t *a_thread);
void	systemdict_nsleep(cw_stilo_t *a_thread);
void	systemdict_open(cw_stilo_t *a_thread);
void	systemdict_or(cw_stilo_t *a_thread);
void	systemdict_pop(cw_stilo_t *a_thread);
void	systemdict_print(cw_stilo_t *a_thread);
void	systemdict_pstack(cw_stilo_t *a_thread);
void	systemdict_put(cw_stilo_t *a_thread);
void	systemdict_putinterval(cw_stilo_t *a_thread);
void	systemdict_quit(cw_stilo_t *a_thread);
void	systemdict_rand(cw_stilo_t *a_thread);
void	systemdict_read(cw_stilo_t *a_thread);
void	systemdict_readline(cw_stilo_t *a_thread);
void	systemdict_realtime(cw_stilo_t *a_thread);
void	systemdict_renamefile(cw_stilo_t *a_thread);
void	systemdict_repeat(cw_stilo_t *a_thread);
void	systemdict_roll(cw_stilo_t *a_thread);
void	systemdict_run(cw_stilo_t *a_thread);
void	systemdict_sclear(cw_stilo_t *a_thread);
void	systemdict_scleartomark(cw_stilo_t *a_thread);
void	systemdict_scount(cw_stilo_t *a_thread);
void	systemdict_scounttomark(cw_stilo_t *a_thread);
void	systemdict_sdup(cw_stilo_t *a_thread);
void	systemdict_seek(cw_stilo_t *a_thread);
void	systemdict_self(cw_stilo_t *a_thread);
void	systemdict_setlocking(cw_stilo_t *a_thread);
void	systemdict_sexch(cw_stilo_t *a_thread);
void	systemdict_shift(cw_stilo_t *a_thread);
void	systemdict_signal(cw_stilo_t *a_thread);
void	systemdict_sindex(cw_stilo_t *a_thread);
void	systemdict_spop(cw_stilo_t *a_thread);
void	systemdict_sprint(cw_stilo_t *a_thread);
void	systemdict_spush(cw_stilo_t *a_thread);
void	systemdict_srand(cw_stilo_t *a_thread);
void	systemdict_sroll(cw_stilo_t *a_thread);
void	systemdict_stack(cw_stilo_t *a_thread);
void	systemdict_start(cw_stilo_t *a_thread);
void	systemdict_stat(cw_stilo_t *a_thread);
void	systemdict_stop(cw_stilo_t *a_thread);
void	systemdict_stopped(cw_stilo_t *a_thread);
void	systemdict_store(cw_stilo_t *a_thread);
void	systemdict_string(cw_stilo_t *a_thread);
void	systemdict_sub(cw_stilo_t *a_thread);
void	systemdict_sym_rp(cw_stilo_t *a_thread);
void	systemdict_sym_gt(cw_stilo_t *a_thread);
void	systemdict_sym_rb(cw_stilo_t *a_thread);
void	systemdict_symlink(cw_stilo_t *a_thread);
void	systemdict_tell(cw_stilo_t *a_thread);
void	systemdict_test(cw_stilo_t *a_thread);
void	systemdict_thread(cw_stilo_t *a_thread);
void	systemdict_timedwait(cw_stilo_t *a_thread);
void	systemdict_token(cw_stilo_t *a_thread);
void	systemdict_truncate(cw_stilo_t *a_thread);
void	systemdict_trylock(cw_stilo_t *a_thread);
void	systemdict_type(cw_stilo_t *a_thread);
void	systemdict_undef(cw_stilo_t *a_thread);
void	systemdict_unlink(cw_stilo_t *a_thread);
void	systemdict_unlock(cw_stilo_t *a_thread);
void	systemdict_wait(cw_stilo_t *a_thread);
void	systemdict_waitpid(cw_stilo_t *a_thread);
void	systemdict_where(cw_stilo_t *a_thread);
void	systemdict_write(cw_stilo_t *a_thread);
void	systemdict_xcheck(cw_stilo_t *a_thread);
void	systemdict_xor(cw_stilo_t *a_thread);
void	systemdict_yield(cw_stilo_t *a_thread);

#ifndef _CW_USE_INLINES
void	systemdict_inline_add(cw_stilo_t *a_thread);
void	systemdict_inline_dup(cw_stilo_t *a_thread);
void	systemdict_inline_exch(cw_stilo_t *a_thread);
void	systemdict_inline_index(cw_stilo_t *a_thread);
void	systemdict_inline_pop(cw_stilo_t *a_thread);
void	systemdict_inline_roll(cw_stilo_t *a_thread);
#endif

#if (defined(_CW_USE_INLINES) || defined(_SYSTEMDICT_C_))
_CW_INLINE void
systemdict_inline_add(cw_stilo_t *a_thread)
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

	stilo_integer_set(a, stilo_integer_get(a) + stilo_integer_get(b));
	stilo_stack_pop(ostack);
}

_CW_INLINE void
systemdict_inline_dup(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*orig, *dup;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(orig, ostack, a_thread);
	dup = stilo_stack_push(ostack);
	stilo_dup(dup, orig);
}

_CW_INLINE void
systemdict_inline_exch(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;

	ostack = stilo_thread_ostack_get(a_thread);

	if (stilo_stack_count(ostack) < 2) {
		stilo_thread_error(a_thread, STILO_THREADE_STACKUNDERFLOW);
		return;
	}

	stilo_stack_roll(ostack, 2, 1);
}

_CW_INLINE void
systemdict_inline_index(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo, *orig;
	cw_stiloi_t	index;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	index = stilo_integer_get(stilo);
	if (index < 0) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}

	STILO_STACK_NGET(orig, ostack, a_thread, index + 1);
	stilo_dup(stilo, orig);
}

_CW_INLINE void
systemdict_inline_pop(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_POP(ostack, a_thread);
}

_CW_INLINE void
systemdict_inline_roll(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stiloi_t	count, amount;

	ostack = stilo_thread_ostack_get(a_thread);

	STILO_STACK_GET(stilo, ostack, a_thread);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	amount = stilo_integer_get(stilo);
	STILO_STACK_DOWN_GET(stilo, ostack, a_thread, stilo);
	if (stilo_type_get(stilo) != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	count = stilo_integer_get(stilo);
	if (count < 1) {
		stilo_thread_error(a_thread, STILO_THREADE_RANGECHECK);
		return;
	}
	if (count > stilo_stack_count(ostack) - 2) {
		stilo_thread_error(a_thread, STILO_THREADE_STACKUNDERFLOW);
		return;
	}

	stilo_stack_npop(ostack, 2);
	stilo_stack_roll(ostack, count, amount);
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_SYSTEMDICT_C_)) */
