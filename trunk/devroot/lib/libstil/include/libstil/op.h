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

void	stil_op_systemdict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt);

void	op_abort(cw_stilt_t *a_stilt);
void	op_abs(cw_stilt_t *a_stilt);
void	op_add(cw_stilt_t *a_stilt);
void	op_aload(cw_stilt_t *a_stilt);
void	op_anchorsearch(cw_stilt_t *a_stilt);
void	op_and(cw_stilt_t *a_stilt);
void	op_array(cw_stilt_t *a_stilt);
void	op_astore(cw_stilt_t *a_stilt);
void	op_begin(cw_stilt_t *a_stilt);
void	op_bind(cw_stilt_t *a_stilt);
void	op_bytesavailable(cw_stilt_t *a_stilt);
void	op_ceiling(cw_stilt_t *a_stilt);
void	op_clear(cw_stilt_t *a_stilt);
void	op_cleardictstack(cw_stilt_t *a_stilt);
void	op_cleartomark(cw_stilt_t *a_stilt);
void	op_closefile(cw_stilt_t *a_stilt);
void	op_condition(cw_stilt_t *a_stilt);
void	op_copy(cw_stilt_t *a_stilt);
void	op_count(cw_stilt_t *a_stilt);
void	op_countdictstack(cw_stilt_t *a_stilt);
void	op_countexecstack(cw_stilt_t *a_stilt);
void	op_counttomark(cw_stilt_t *a_stilt);
void	op_currentaccuracy(cw_stilt_t *a_stilt);
void	op_currentbase(cw_stilt_t *a_stilt);
void	op_currentcontext(cw_stilt_t *a_stilt);
void	op_currentdict(cw_stilt_t *a_stilt);
void	op_currentfile(cw_stilt_t *a_stilt);
void	op_currentglobal(cw_stilt_t *a_stilt);
void	op_currentmstate(cw_stilt_t *a_stilt);
void	op_currentobjectformat(cw_stilt_t *a_stilt);
void	op_currentpacking(cw_stilt_t *a_stilt);
void	op_currentpoint(cw_stilt_t *a_stilt);
void	op_cvlit(cw_stilt_t *a_stilt);
void	op_cvm(cw_stilt_t *a_stilt);
void	op_cvn(cw_stilt_t *a_stilt);
void	op_cvrs(cw_stilt_t *a_stilt);
void	op_cvs(cw_stilt_t *a_stilt);
void	op_cvx(cw_stilt_t *a_stilt);
void	op_def(cw_stilt_t *a_stilt);
void	op_defineresource(cw_stilt_t *a_stilt);
void	op_deletefile(cw_stilt_t *a_stilt);
void	op_detach(cw_stilt_t *a_stilt);
void	op_dict(cw_stilt_t *a_stilt);
void	op_dictstack(cw_stilt_t *a_stilt);
void	op_div(cw_stilt_t *a_stilt);
void	op_dup(cw_stilt_t *a_stilt);
void	op_echo(cw_stilt_t *a_stilt);
void	op_end(cw_stilt_t *a_stilt);
void	op_eq(cw_stilt_t *a_stilt);
void	op_exch(cw_stilt_t *a_stilt);
void	op_exec(cw_stilt_t *a_stilt);
void	op_execstack(cw_stilt_t *a_stilt);
void	op_executeonly(cw_stilt_t *a_stilt);
void	op_executive(cw_stilt_t *a_stilt);
void	op_exit(cw_stilt_t *a_stilt);
void	op_exp(cw_stilt_t *a_stilt);
void	op_false(cw_stilt_t *a_stilt);
void	op_file(cw_stilt_t *a_stilt);
void	op_filenameforall(cw_stilt_t *a_stilt);
void	op_fileposition(cw_stilt_t *a_stilt);
void	op_filter(cw_stilt_t *a_stilt);
void	op_findresource(cw_stilt_t *a_stilt);
void	op_floor(cw_stilt_t *a_stilt);
void	op_flush(cw_stilt_t *a_stilt);
void	op_flushfile(cw_stilt_t *a_stilt);
void	op_for(cw_stilt_t *a_stilt);
void	op_forall(cw_stilt_t *a_stilt);
void	op_fork(cw_stilt_t *a_stilt);
void	op_gcheck(cw_stilt_t *a_stilt);
void	op_ge(cw_stilt_t *a_stilt);
void	op_get(cw_stilt_t *a_stilt);
void	op_getinterval(cw_stilt_t *a_stilt);
void	op_gt(cw_stilt_t *a_stilt);
void	op_if(cw_stilt_t *a_stilt);
void	op_ifelse(cw_stilt_t *a_stilt);
void	op_index(cw_stilt_t *a_stilt);
void	op_initmath(cw_stilt_t *a_stilt);
void	op_join(cw_stilt_t *a_stilt);
void	op_known(cw_stilt_t *a_stilt);
void	op_le(cw_stilt_t *a_stilt);
void	op_length(cw_stilt_t *a_stilt);
void	op_load(cw_stilt_t *a_stilt);
void	op_lock(cw_stilt_t *a_stilt);
void	op_loop(cw_stilt_t *a_stilt);
void	op_lt(cw_stilt_t *a_stilt);
void	op_mark(cw_stilt_t *a_stilt);
void	op_maxlength(cw_stilt_t *a_stilt);
void	op_mod(cw_stilt_t *a_stilt);
void	op_mrestore(cw_stilt_t *a_stilt);
void	op_mrestoreall(cw_stilt_t *a_stilt);
void	op_msave(cw_stilt_t *a_stilt);
void	op_mstate(cw_stilt_t *a_stilt);
void	op_mul(cw_stilt_t *a_stilt);
void	op_mutex(cw_stilt_t *a_stilt);
void	op_ne(cw_stilt_t *a_stilt);
void	op_neg(cw_stilt_t *a_stilt);
void	op_noaccess(cw_stilt_t *a_stilt);
void	op_not(cw_stilt_t *a_stilt);
void	op_notify(cw_stilt_t *a_stilt);
void	op_null(cw_stilt_t *a_stilt);
void	op_or(cw_stilt_t *a_stilt);
void	op_packedarray(cw_stilt_t *a_stilt);
void	op_pop(cw_stilt_t *a_stilt);
void	op_print(cw_stilt_t *a_stilt);
void	op_printobject(cw_stilt_t *a_stilt);
void	op_product(cw_stilt_t *a_stilt);
void	op_prompt(cw_stilt_t *a_stilt);
void	op_pstack(cw_stilt_t *a_stilt);
void	op_put(cw_stilt_t *a_stilt);
void	op_putinterval(cw_stilt_t *a_stilt);
void	op_quit(cw_stilt_t *a_stilt);
void	op_rand(cw_stilt_t *a_stilt);
void	op_rcheck(cw_stilt_t *a_stilt);
void	op_read(cw_stilt_t *a_stilt);
void	op_readhexstring(cw_stilt_t *a_stilt);
void	op_readline(cw_stilt_t *a_stilt);
void	op_readonly(cw_stilt_t *a_stilt);
void	op_readstring(cw_stilt_t *a_stilt);
void	op_realtime(cw_stilt_t *a_stilt);
void	op_renamefile(cw_stilt_t *a_stilt);
void	op_repeat(cw_stilt_t *a_stilt);
void	op_resetfile(cw_stilt_t *a_stilt);
void	op_resourceforall(cw_stilt_t *a_stilt);
void	op_resourcestatus(cw_stilt_t *a_stilt);
void	op_roll(cw_stilt_t *a_stilt);
void	op_round(cw_stilt_t *a_stilt);
void	op_rrand(cw_stilt_t *a_stilt);
void	op_run(cw_stilt_t *a_stilt);
void	op_search(cw_stilt_t *a_stilt);
void	op_setaccuracy(cw_stilt_t *a_stilt);
void	op_setbase(cw_stilt_t *a_stilt);
void	op_setfileposition(cw_stilt_t *a_stilt);
void	op_setglobal(cw_stilt_t *a_stilt);
void	op_setmstate(cw_stilt_t *a_stilt);
void	op_setobjectformat(cw_stilt_t *a_stilt);
void	op_setpacking(cw_stilt_t *a_stilt);
void	op_setpoint(cw_stilt_t *a_stilt);
void	op_shift(cw_stilt_t *a_stilt);
void	op_sqrt(cw_stilt_t *a_stilt);
void	op_srand(cw_stilt_t *a_stilt);
void	op_stack(cw_stilt_t *a_stilt);
void	op_start(cw_stilt_t *a_stilt);
void	op_status(cw_stilt_t *a_stilt);
void	op_stop(cw_stilt_t *a_stilt);
void	op_stopped(cw_stilt_t *a_stilt);
void	op_store(cw_stilt_t *a_stilt);
void	op_string(cw_stilt_t *a_stilt);
void	op_sub(cw_stilt_t *a_stilt);
void	op_sym_eq(cw_stilt_t *a_stilt);
void	op_sym_eq_eq(cw_stilt_t *a_stilt);
void	op_sym_gt_gt(cw_stilt_t *a_stilt);
void	op_sym_lb(cw_stilt_t *a_stilt);
void	op_sym_lt_lt(cw_stilt_t *a_stilt);
void	op_sym_rb(cw_stilt_t *a_stilt);
void	op_timedwait(cw_stilt_t *a_stilt);
void	op_token(cw_stilt_t *a_stilt);
void	op_true(cw_stilt_t *a_stilt);
void	op_trylock(cw_stilt_t *a_stilt);
void	op_type(cw_stilt_t *a_stilt);
void	op_undef(cw_stilt_t *a_stilt);
void	op_undefineresource(cw_stilt_t *a_stilt);
void	op_unlock(cw_stilt_t *a_stilt);
void	op_usertime(cw_stilt_t *a_stilt);
void	op_version(cw_stilt_t *a_stilt);
void	op_wait(cw_stilt_t *a_stilt);
void	op_wcheck(cw_stilt_t *a_stilt);
void	op_where(cw_stilt_t *a_stilt);
void	op_write(cw_stilt_t *a_stilt);
void	op_writehexstring(cw_stilt_t *a_stilt);
void	op_writeobject(cw_stilt_t *a_stilt);
void	op_writestring(cw_stilt_t *a_stilt);
void	op_xcheck(cw_stilt_t *a_stilt);
void	op_xor(cw_stilt_t *a_stilt);
void	op_yield(cw_stilt_t *a_stilt);
