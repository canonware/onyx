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

void	systemdict_abs(cw_nxo_t *a_thread);
void	systemdict_add(cw_nxo_t *a_thread);
void	systemdict_aload(cw_nxo_t *a_thread);
void	systemdict_and(cw_nxo_t *a_thread);
void	systemdict_array(cw_nxo_t *a_thread);
void	systemdict_astore(cw_nxo_t *a_thread);
void	systemdict_begin(cw_nxo_t *a_thread);
void	systemdict_bind(cw_nxo_t *a_thread);
void	systemdict_broadcast(cw_nxo_t *a_thread);
void	systemdict_bytesavailable(cw_nxo_t *a_thread);
void	systemdict_catenate(cw_nxo_t *a_thread);
void	systemdict_cd(cw_nxo_t *a_thread);
void	systemdict_chmod(cw_nxo_t *a_thread);
void	systemdict_chown(cw_nxo_t *a_thread);
void	systemdict_clear(cw_nxo_t *a_thread);
void	systemdict_cleardstack(cw_nxo_t *a_thread);
void	systemdict_cleartomark(cw_nxo_t *a_thread);
void	systemdict_close(cw_nxo_t *a_thread);
void	systemdict_condition(cw_nxo_t *a_thread);
void	systemdict_copy(cw_nxo_t *a_thread);
void	systemdict_count(cw_nxo_t *a_thread);
void	systemdict_countdstack(cw_nxo_t *a_thread);
void	systemdict_countestack(cw_nxo_t *a_thread);
void	systemdict_counttomark(cw_nxo_t *a_thread);
void	systemdict_currentdict(cw_nxo_t *a_thread);
void	systemdict_currentfile(cw_nxo_t *a_thread);
void	systemdict_currentlocking(cw_nxo_t *a_thread);
void	systemdict_cvlit(cw_nxo_t *a_thread);
void	systemdict_cvn(cw_nxo_t *a_thread);
void	systemdict_cvrs(cw_nxo_t *a_thread);
void	systemdict_cvs(cw_nxo_t *a_thread);
void	systemdict_cvx(cw_nxo_t *a_thread);
void	systemdict_def(cw_nxo_t *a_thread);
void	systemdict_detach(cw_nxo_t *a_thread);
void	systemdict_dict(cw_nxo_t *a_thread);
void	systemdict_die(cw_nxo_t *a_thread);
void	systemdict_dirforeach(cw_nxo_t *a_thread);
void	systemdict_div(cw_nxo_t *a_thread);
void	systemdict_dstack(cw_nxo_t *a_thread);
void	systemdict_dup(cw_nxo_t *a_thread);
void	systemdict_end(cw_nxo_t *a_thread);
void	systemdict_eq(cw_nxo_t *a_thread);
void	systemdict_estack(cw_nxo_t *a_thread);
void	systemdict_eval(cw_nxo_t *a_thread);
void	systemdict_exch(cw_nxo_t *a_thread);
void	systemdict_exec(cw_nxo_t *a_thread);
void	systemdict_exit(cw_nxo_t *a_thread);
void	systemdict_exp(cw_nxo_t *a_thread);
void	systemdict_fino(cw_nxo_t *a_thread);
void	systemdict_flush(cw_nxo_t *a_thread);
void	systemdict_flushfile(cw_nxo_t *a_thread);
void	systemdict_for(cw_nxo_t *a_thread);
void	systemdict_foreach(cw_nxo_t *a_thread);
void	systemdict_fork(cw_nxo_t *a_thread);
void	systemdict_ge(cw_nxo_t *a_thread);
void	systemdict_get(cw_nxo_t *a_thread);
void	systemdict_getinterval(cw_nxo_t *a_thread);
void	systemdict_gt(cw_nxo_t *a_thread);
void	systemdict_if(cw_nxo_t *a_thread);
void	systemdict_ifelse(cw_nxo_t *a_thread);
void	systemdict_index(cw_nxo_t *a_thread);
void	systemdict_istack(cw_nxo_t *a_thread);
void	systemdict_join(cw_nxo_t *a_thread);
void	systemdict_known(cw_nxo_t *a_thread);
void	systemdict_lcheck(cw_nxo_t *a_thread);
void	systemdict_le(cw_nxo_t *a_thread);
void	systemdict_length(cw_nxo_t *a_thread);
void	systemdict_link(cw_nxo_t *a_thread);
void	systemdict_load(cw_nxo_t *a_thread);
void	systemdict_lock(cw_nxo_t *a_thread);
void	systemdict_loop(cw_nxo_t *a_thread);
void	systemdict_lt(cw_nxo_t *a_thread);
void	systemdict_mark(cw_nxo_t *a_thread);
void	systemdict_mkdir(cw_nxo_t *a_thread);
void	systemdict_mod(cw_nxo_t *a_thread);
void	systemdict_monitor(cw_nxo_t *a_thread);
void	systemdict_mul(cw_nxo_t *a_thread);
void	systemdict_mutex(cw_nxo_t *a_thread);
void	systemdict_ne(cw_nxo_t *a_thread);
void	systemdict_neg(cw_nxo_t *a_thread);
void	systemdict_not(cw_nxo_t *a_thread);
void	systemdict_nsleep(cw_nxo_t *a_thread);
void	systemdict_open(cw_nxo_t *a_thread);
void	systemdict_or(cw_nxo_t *a_thread);
void	systemdict_ostack(cw_nxo_t *a_thread);
void	systemdict_pop(cw_nxo_t *a_thread);
void	systemdict_print(cw_nxo_t *a_thread);
void	systemdict_put(cw_nxo_t *a_thread);
void	systemdict_putinterval(cw_nxo_t *a_thread);
void	systemdict_pwd(cw_nxo_t *a_thread);
void	systemdict_quit(cw_nxo_t *a_thread);
void	systemdict_rand(cw_nxo_t *a_thread);
void	systemdict_read(cw_nxo_t *a_thread);
void	systemdict_readline(cw_nxo_t *a_thread);
void	systemdict_realtime(cw_nxo_t *a_thread);
void	systemdict_rename(cw_nxo_t *a_thread);
void	systemdict_repeat(cw_nxo_t *a_thread);
void	systemdict_rmdir(cw_nxo_t *a_thread);
void	systemdict_roll(cw_nxo_t *a_thread);
void	systemdict_run(cw_nxo_t *a_thread);
void	systemdict_sclear(cw_nxo_t *a_thread);
void	systemdict_scleartomark(cw_nxo_t *a_thread);
void	systemdict_scount(cw_nxo_t *a_thread);
void	systemdict_scounttomark(cw_nxo_t *a_thread);
void	systemdict_sdup(cw_nxo_t *a_thread);
void	systemdict_seek(cw_nxo_t *a_thread);
void	systemdict_self(cw_nxo_t *a_thread);
void	systemdict_setlocking(cw_nxo_t *a_thread);
void	systemdict_sexch(cw_nxo_t *a_thread);
void	systemdict_shift(cw_nxo_t *a_thread);
void	systemdict_signal(cw_nxo_t *a_thread);
void	systemdict_sindex(cw_nxo_t *a_thread);
void	systemdict_sload(cw_nxo_t *a_thread);
void	systemdict_spop(cw_nxo_t *a_thread);
void	systemdict_spush(cw_nxo_t *a_thread);
void	systemdict_srand(cw_nxo_t *a_thread);
void	systemdict_sroll(cw_nxo_t *a_thread);
void	systemdict_sstore(cw_nxo_t *a_thread);
void	systemdict_stack(cw_nxo_t *a_thread);
void	systemdict_start(cw_nxo_t *a_thread);
void	systemdict_status(cw_nxo_t *a_thread);
void	systemdict_stderr(cw_nxo_t *a_thread);
void	systemdict_stdin(cw_nxo_t *a_thread);
void	systemdict_stdout(cw_nxo_t *a_thread);
void	systemdict_stop(cw_nxo_t *a_thread);
void	systemdict_stopped(cw_nxo_t *a_thread);
void	systemdict_store(cw_nxo_t *a_thread);
void	systemdict_string(cw_nxo_t *a_thread);
void	systemdict_sub(cw_nxo_t *a_thread);
void	systemdict_sym_rp(cw_nxo_t *a_thread);
void	systemdict_sym_gt(cw_nxo_t *a_thread);
void	systemdict_sym_rb(cw_nxo_t *a_thread);
void	systemdict_symlink(cw_nxo_t *a_thread);
void	systemdict_tell(cw_nxo_t *a_thread);
void	systemdict_test(cw_nxo_t *a_thread);
void	systemdict_thread(cw_nxo_t *a_thread);
void	systemdict_timedwait(cw_nxo_t *a_thread);
void	systemdict_token(cw_nxo_t *a_thread);
void	systemdict_truncate(cw_nxo_t *a_thread);
void	systemdict_trylock(cw_nxo_t *a_thread);
void	systemdict_type(cw_nxo_t *a_thread);
void	systemdict_undef(cw_nxo_t *a_thread);
void	systemdict_unlink(cw_nxo_t *a_thread);
void	systemdict_unlock(cw_nxo_t *a_thread);
void	systemdict_wait(cw_nxo_t *a_thread);
void	systemdict_waitpid(cw_nxo_t *a_thread);
void	systemdict_where(cw_nxo_t *a_thread);
void	systemdict_write(cw_nxo_t *a_thread);
void	systemdict_xcheck(cw_nxo_t *a_thread);
void	systemdict_xor(cw_nxo_t *a_thread);
void	systemdict_yield(cw_nxo_t *a_thread);

#ifndef _CW_USE_INLINES
/*
 * Rename the inline functions, since they're not inlined.  The normal
 * definitions for these functions are #ifdef'ed out in this case.
 */
#define	systemdict_inline_add	systemdict_add
#define	systemdict_inline_dup	systemdict_dup
#define	systemdict_inline_exch	systemdict_exch
#define	systemdict_inline_index	systemdict_index
#define	systemdict_inline_pop	systemdict_pop
#define	systemdict_inline_roll	systemdict_roll
void	systemdict_inline_add(cw_nxo_t *a_thread);
void	systemdict_inline_dup(cw_nxo_t *a_thread);
void	systemdict_inline_exch(cw_nxo_t *a_thread);
void	systemdict_inline_index(cw_nxo_t *a_thread);
void	systemdict_inline_pop(cw_nxo_t *a_thread);
void	systemdict_inline_roll(cw_nxo_t *a_thread);
#endif

#if (defined(_CW_USE_INLINES) || defined(_SYSTEMDICT_C_))
_CW_INLINE void
systemdict_inline_add(cw_nxo_t *a_thread)
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

	nxo_integer_set(a, nxo_integer_get(a) + nxo_integer_get(b));
	nxo_stack_pop(ostack);
}

_CW_INLINE void
systemdict_inline_dup(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*orig, *dup;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(orig, ostack, a_thread);
	dup = nxo_stack_push(ostack);
	nxo_dup(dup, orig);
}

_CW_INLINE void
systemdict_inline_exch(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;

	ostack = nxo_thread_ostack_get(a_thread);

	if (nxo_stack_exch(ostack))
		nxo_thread_error(a_thread, NXO_THREADE_STACKUNDERFLOW);
}

_CW_INLINE void
systemdict_inline_index(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo, *orig;
	cw_nxoi_t	index;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	index = nxo_integer_get(nxo);
	if (index < 0) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	NXO_STACK_NGET(orig, ostack, a_thread, index + 1);
	nxo_dup(nxo, orig);
}

_CW_INLINE void
systemdict_inline_pop(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_POP(ostack, a_thread);
}

_CW_INLINE void
systemdict_inline_roll(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack;
	cw_nxo_t	*nxo;
	cw_nxoi_t	count, amount;

	ostack = nxo_thread_ostack_get(a_thread);

	NXO_STACK_GET(nxo, ostack, a_thread);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	amount = nxo_integer_get(nxo);
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	if (nxo_type_get(nxo) != NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	count = nxo_integer_get(nxo);
	if (count < 1) {
		nxo_thread_error(a_thread, NXO_THREADE_RANGECHECK);
		return;
	}

	nxo_stack_npop(ostack, 2);
	if (nxo_stack_roll(ostack, count, amount)) {
		cw_nxo_t	*nxo;

		/*
		 * Stack underflow.  Restore the stack to its original state,
		 * then throw an error.
		 */
		nxo = nxo_stack_push(ostack);
		nxo_integer_new(nxo, count);
		nxo = nxo_stack_push(ostack);
		nxo_integer_new(nxo, amount);

		nxo_thread_error(a_thread, NXO_THREADE_STACKUNDERFLOW);
	}
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_SYSTEMDICT_C_)) */
