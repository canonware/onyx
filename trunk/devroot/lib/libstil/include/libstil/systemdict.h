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

void	systemdict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt);

void	systemdict_abort(cw_stilt_t *a_stilt);
void	systemdict_abs(cw_stilt_t *a_stilt);
void	systemdict_add(cw_stilt_t *a_stilt);
void	systemdict_aload(cw_stilt_t *a_stilt);
void	systemdict_anchorsearch(cw_stilt_t *a_stilt);
void	systemdict_and(cw_stilt_t *a_stilt);
void	systemdict_array(cw_stilt_t *a_stilt);
void	systemdict_astore(cw_stilt_t *a_stilt);
void	systemdict_begin(cw_stilt_t *a_stilt);
void	systemdict_bind(cw_stilt_t *a_stilt);
void	systemdict_bytesavailable(cw_stilt_t *a_stilt);
void	systemdict_clear(cw_stilt_t *a_stilt);
void	systemdict_cleardictstack(cw_stilt_t *a_stilt);
void	systemdict_cleartomark(cw_stilt_t *a_stilt);
void	systemdict_closefile(cw_stilt_t *a_stilt);
void	systemdict_condition(cw_stilt_t *a_stilt);
void	systemdict_copy(cw_stilt_t *a_stilt);
void	systemdict_count(cw_stilt_t *a_stilt);
void	systemdict_countdictstack(cw_stilt_t *a_stilt);
void	systemdict_countexecstack(cw_stilt_t *a_stilt);
void	systemdict_counttomark(cw_stilt_t *a_stilt);
void	systemdict_currentcontext(cw_stilt_t *a_stilt);
void	systemdict_currentdict(cw_stilt_t *a_stilt);
void	systemdict_currentfile(cw_stilt_t *a_stilt);
void	systemdict_currentglobal(cw_stilt_t *a_stilt);
void	systemdict_cvlit(cw_stilt_t *a_stilt);
void	systemdict_cvm(cw_stilt_t *a_stilt);
void	systemdict_cvn(cw_stilt_t *a_stilt);
void	systemdict_cvrs(cw_stilt_t *a_stilt);
void	systemdict_cvs(cw_stilt_t *a_stilt);
void	systemdict_cvx(cw_stilt_t *a_stilt);
void	systemdict_def(cw_stilt_t *a_stilt);
void	systemdict_defineresource(cw_stilt_t *a_stilt);
void	systemdict_deletefile(cw_stilt_t *a_stilt);
void	systemdict_detach(cw_stilt_t *a_stilt);
void	systemdict_dict(cw_stilt_t *a_stilt);
void	systemdict_dictstack(cw_stilt_t *a_stilt);
void	systemdict_div(cw_stilt_t *a_stilt);
void	systemdict_dup(cw_stilt_t *a_stilt);
void	systemdict_echo(cw_stilt_t *a_stilt);
void	systemdict_end(cw_stilt_t *a_stilt);
void	systemdict_eq(cw_stilt_t *a_stilt);
void	systemdict_errordict(cw_stilt_t *a_stilt);
void	systemdict_exch(cw_stilt_t *a_stilt);
void	systemdict_exec(cw_stilt_t *a_stilt);
void	systemdict_execstack(cw_stilt_t *a_stilt);
void	systemdict_executeonly(cw_stilt_t *a_stilt);
void	systemdict_executive(cw_stilt_t *a_stilt);
void	systemdict_exit(cw_stilt_t *a_stilt);
void	systemdict_exp(cw_stilt_t *a_stilt);
void	systemdict_false(cw_stilt_t *a_stilt);
void	systemdict_file(cw_stilt_t *a_stilt);
void	systemdict_filenameforall(cw_stilt_t *a_stilt);
void	systemdict_fileposition(cw_stilt_t *a_stilt);
void	systemdict_filter(cw_stilt_t *a_stilt);
void	systemdict_findresource(cw_stilt_t *a_stilt);
void	systemdict_flush(cw_stilt_t *a_stilt);
void	systemdict_flushfile(cw_stilt_t *a_stilt);
void	systemdict_for(cw_stilt_t *a_stilt);
void	systemdict_forall(cw_stilt_t *a_stilt);
void	systemdict_fork(cw_stilt_t *a_stilt);
void	systemdict_gcheck(cw_stilt_t *a_stilt);
void	systemdict_ge(cw_stilt_t *a_stilt);
void	systemdict_get(cw_stilt_t *a_stilt);
void	systemdict_getinterval(cw_stilt_t *a_stilt);
void	systemdict_gt(cw_stilt_t *a_stilt);
void	systemdict_if(cw_stilt_t *a_stilt);
void	systemdict_ifelse(cw_stilt_t *a_stilt);
void	systemdict_index(cw_stilt_t *a_stilt);
void	systemdict_internaldict(cw_stilt_t *a_stilt);
void	systemdict_join(cw_stilt_t *a_stilt);
void	systemdict_known(cw_stilt_t *a_stilt);
void	systemdict_le(cw_stilt_t *a_stilt);
void	systemdict_length(cw_stilt_t *a_stilt);
void	systemdict_load(cw_stilt_t *a_stilt);
void	systemdict_lock(cw_stilt_t *a_stilt);
void	systemdict_localinstancedict(cw_stilt_t *a_stilt);
void	systemdict_loop(cw_stilt_t *a_stilt);
void	systemdict_lt(cw_stilt_t *a_stilt);
void	systemdict_mark(cw_stilt_t *a_stilt);
void	systemdict_maxlength(cw_stilt_t *a_stilt);
void	systemdict_mod(cw_stilt_t *a_stilt);
void	systemdict_mul(cw_stilt_t *a_stilt);
void	systemdict_mutex(cw_stilt_t *a_stilt);
void	systemdict_ne(cw_stilt_t *a_stilt);
void	systemdict_neg(cw_stilt_t *a_stilt);
void	systemdict_noaccess(cw_stilt_t *a_stilt);
void	systemdict_not(cw_stilt_t *a_stilt);
void	systemdict_notify(cw_stilt_t *a_stilt);
void	systemdict_null(cw_stilt_t *a_stilt);
void	systemdict_or(cw_stilt_t *a_stilt);
void	systemdict_output(cw_stilt_t *a_stilt);
void	systemdict_pop(cw_stilt_t *a_stilt);
void	systemdict_print(cw_stilt_t *a_stilt);
void	systemdict_printobject(cw_stilt_t *a_stilt);
void	systemdict_product(cw_stilt_t *a_stilt);
void	systemdict_prompt(cw_stilt_t *a_stilt);
void	systemdict_pstack(cw_stilt_t *a_stilt);
void	systemdict_put(cw_stilt_t *a_stilt);
void	systemdict_putinterval(cw_stilt_t *a_stilt);
void	systemdict_quit(cw_stilt_t *a_stilt);
void	systemdict_rand(cw_stilt_t *a_stilt);
void	systemdict_rcheck(cw_stilt_t *a_stilt);
void	systemdict_read(cw_stilt_t *a_stilt);
void	systemdict_readonly(cw_stilt_t *a_stilt);
void	systemdict_readstring(cw_stilt_t *a_stilt);
void	systemdict_realtime(cw_stilt_t *a_stilt);
void	systemdict_renamefile(cw_stilt_t *a_stilt);
void	systemdict_repeat(cw_stilt_t *a_stilt);
void	systemdict_resourceforall(cw_stilt_t *a_stilt);
void	systemdict_resourcestatus(cw_stilt_t *a_stilt);
void	systemdict_roll(cw_stilt_t *a_stilt);
void	systemdict_run(cw_stilt_t *a_stilt);
void	systemdict_search(cw_stilt_t *a_stilt);
void	systemdict_serverdict(cw_stilt_t *a_stilt);
void	systemdict_setfileposition(cw_stilt_t *a_stilt);
void	systemdict_setglobal(cw_stilt_t *a_stilt);
void	systemdict_shift(cw_stilt_t *a_stilt);
void	systemdict_srand(cw_stilt_t *a_stilt);
void	systemdict_stack(cw_stilt_t *a_stilt);
void	systemdict_start(cw_stilt_t *a_stilt);
void	systemdict_status(cw_stilt_t *a_stilt);
void	systemdict_statusdict(cw_stilt_t *a_stilt);
void	systemdict_stdin(cw_stilt_t *a_stilt);
void	systemdict_stderr(cw_stilt_t *a_stilt);
void	systemdict_stdout(cw_stilt_t *a_stilt);
void	systemdict_stop(cw_stilt_t *a_stilt);
void	systemdict_stopped(cw_stilt_t *a_stilt);
void	systemdict_store(cw_stilt_t *a_stilt);
void	systemdict_string(cw_stilt_t *a_stilt);
void	systemdict_sub(cw_stilt_t *a_stilt);
void	systemdict_sym_derror(cw_stilt_t *a_stilt);
void	systemdict_sym_eq(cw_stilt_t *a_stilt);
void	systemdict_sym_eq_eq(cw_stilt_t *a_stilt);
void	systemdict_sym_gt_gt(cw_stilt_t *a_stilt);
void	systemdict_sym_rb(cw_stilt_t *a_stilt);
void	systemdict_timedwait(cw_stilt_t *a_stilt);
void	systemdict_token(cw_stilt_t *a_stilt);
void	systemdict_true(cw_stilt_t *a_stilt);
void	systemdict_trylock(cw_stilt_t *a_stilt);
void	systemdict_type(cw_stilt_t *a_stilt);
void	systemdict_undef(cw_stilt_t *a_stilt);
void	systemdict_undefineresource(cw_stilt_t *a_stilt);
void	systemdict_unlock(cw_stilt_t *a_stilt);
void	systemdict_userdict(cw_stilt_t *a_stilt);
void	systemdict_userparams(cw_stilt_t *a_stilt);
void	systemdict_usertime(cw_stilt_t *a_stilt);
void	systemdict_version(cw_stilt_t *a_stilt);
void	systemdict_wait(cw_stilt_t *a_stilt);
void	systemdict_wcheck(cw_stilt_t *a_stilt);
void	systemdict_where(cw_stilt_t *a_stilt);
void	systemdict_write(cw_stilt_t *a_stilt);
void	systemdict_writestring(cw_stilt_t *a_stilt);
void	systemdict_xcheck(cw_stilt_t *a_stilt);
void	systemdict_xor(cw_stilt_t *a_stilt);
void	systemdict_yield(cw_stilt_t *a_stilt);
