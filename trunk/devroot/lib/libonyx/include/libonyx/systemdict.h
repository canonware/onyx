/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

typedef struct
{
    cw_uint32_t iter;
    void *dlhandle;
} cw_nxmod_t;

void
systemdict_abs(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_accept(cw_nxo_t *a_thread);
#endif

void
systemdict_add(cw_nxo_t *a_thread);

void
systemdict_adn(cw_nxo_t *a_thread);

void
systemdict_and(cw_nxo_t *a_thread);

void
systemdict_array(cw_nxo_t *a_thread);

#ifdef CW_REAL
void
systemdict_atan(cw_nxo_t *a_thread);
#endif

void
systemdict_aup(cw_nxo_t *a_thread);

void
systemdict_bdup(cw_nxo_t *a_thread);

void
systemdict_begin(cw_nxo_t *a_thread);

void
systemdict_bind(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_bindsocket(cw_nxo_t *a_thread);
#endif

void
systemdict_bpop(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_broadcast(cw_nxo_t *a_thread);
#endif

void
systemdict_bytesavailable(cw_nxo_t *a_thread);

void
systemdict_cat(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_cd(cw_nxo_t *a_thread);
#endif

#ifdef CW_REAL
void
systemdict_ceiling(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_chmod(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_chown(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_chroot(cw_nxo_t *a_thread);
#endif

void
systemdict_clear(cw_nxo_t *a_thread);

void
systemdict_cleartomark(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_close(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_condition(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_connect(cw_nxo_t *a_thread);
#endif

void
systemdict_copy(cw_nxo_t *a_thread);

#ifdef CW_REAL
void
systemdict_cos(cw_nxo_t *a_thread);
#endif

void
systemdict_count(cw_nxo_t *a_thread);

void
systemdict_countdstack(cw_nxo_t *a_thread);

void
systemdict_countestack(cw_nxo_t *a_thread);

void
systemdict_counttomark(cw_nxo_t *a_thread);

void
systemdict_currentdict(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_currentlocking(cw_nxo_t *a_thread);
#endif

#ifdef CW_REAL
void
systemdict_cvds(cw_nxo_t *a_thread);
#endif

void
systemdict_cve(cw_nxo_t *a_thread);

#ifdef CW_REAL
void
systemdict_cves(cw_nxo_t *a_thread);
#endif

void
systemdict_cvlit(cw_nxo_t *a_thread);

void
systemdict_cvn(cw_nxo_t *a_thread);

void
systemdict_cvrs(cw_nxo_t *a_thread);

void
systemdict_cvs(cw_nxo_t *a_thread);

void
systemdict_cvx(cw_nxo_t *a_thread);

void
systemdict_dec(cw_nxo_t *a_thread);

void
systemdict_def(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_detach(cw_nxo_t *a_thread);
#endif

void
systemdict_dict(cw_nxo_t *a_thread);

void
systemdict_die(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_dirforeach(cw_nxo_t *a_thread);
#endif

#ifdef CW_REAL
void
systemdict_div(cw_nxo_t *a_thread);
#endif

void
systemdict_dn(cw_nxo_t *a_thread);

void
systemdict_dstack(cw_nxo_t *a_thread);

void
systemdict_dup(cw_nxo_t *a_thread);

void
systemdict_echeck(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_egid(cw_nxo_t *a_thread);
#endif

void
systemdict_end(cw_nxo_t *a_thread);

void
systemdict_eq(cw_nxo_t *a_thread);

void
systemdict_estack(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_euid(cw_nxo_t *a_thread);
#endif

void
systemdict_eval(cw_nxo_t *a_thread);

void
systemdict_exch(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_exec(cw_nxo_t *a_thread);
#endif

void
systemdict_exit(cw_nxo_t *a_thread);

void
systemdict_exp(cw_nxo_t *a_thread);

#ifdef CW_REAL
void
systemdict_floor(cw_nxo_t *a_thread);
#endif

void
systemdict_flush(cw_nxo_t *a_thread);

void
systemdict_flushfile(cw_nxo_t *a_thread);

void
systemdict_for(cw_nxo_t *a_thread);

void
systemdict_foreach(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_forkexec(cw_nxo_t *a_thread);
#endif

void
systemdict_ge(cw_nxo_t *a_thread);

void
systemdict_get(cw_nxo_t *a_thread);

void
systemdict_getinterval(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_gid(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_gstderr(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_gstdin(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_gstdout(cw_nxo_t *a_thread);
#endif

void
systemdict_gt(cw_nxo_t *a_thread);

void
systemdict_hooktag(cw_nxo_t *a_thread);

void
systemdict_ibdup(cw_nxo_t *a_thread);

void
systemdict_ibpop(cw_nxo_t *a_thread);

void
systemdict_idiv(cw_nxo_t *a_thread);

void
systemdict_idup(cw_nxo_t *a_thread);

void
systemdict_if(cw_nxo_t *a_thread);

void
systemdict_ifelse(cw_nxo_t *a_thread);

void
systemdict_inc(cw_nxo_t *a_thread);

void
systemdict_iobuf(cw_nxo_t *a_thread);

void
systemdict_ipop(cw_nxo_t *a_thread);

void
systemdict_istack(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_join(cw_nxo_t *a_thread);
#endif

void
systemdict_known(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_lcheck(cw_nxo_t *a_thread);
#endif

void
systemdict_le(cw_nxo_t *a_thread);

void
systemdict_length(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_link(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_listen(cw_nxo_t *a_thread);
#endif

#ifdef CW_REAL
void
systemdict_ln(cw_nxo_t *a_thread);
#endif

void
systemdict_load(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_lock(cw_nxo_t *a_thread);
#endif

#ifdef CW_REAL
void
systemdict_log(cw_nxo_t *a_thread);
#endif

void
systemdict_loop(cw_nxo_t *a_thread);

void
systemdict_lt(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_mkdir(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_mkfifo(cw_nxo_t *a_thread);
#endif

void
systemdict_mod(cw_nxo_t *a_thread);

#ifdef CW_MODULES
void
systemdict_modload(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_monitor(cw_nxo_t *a_thread);
#endif

void
systemdict_mul(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_mutex(cw_nxo_t *a_thread);
#endif

void
systemdict_nbpop(cw_nxo_t *a_thread);

void
systemdict_ncat(cw_nxo_t *a_thread);

void
systemdict_ndn(cw_nxo_t *a_thread);

void
systemdict_ndup(cw_nxo_t *a_thread);

void
systemdict_ne(cw_nxo_t *a_thread);

void
systemdict_neg(cw_nxo_t *a_thread);

void
systemdict_nip(cw_nxo_t *a_thread);

void
systemdict_nonblocking(cw_nxo_t *a_thread);

void
systemdict_not(cw_nxo_t *a_thread);

void
systemdict_npop(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_nsleep(cw_nxo_t *a_thread);
#endif

void
systemdict_nup(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_open(cw_nxo_t *a_thread);
#endif

void
systemdict_or(cw_nxo_t *a_thread);

void
systemdict_ostack(cw_nxo_t *a_thread);

void
systemdict_over(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_peername(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_pid(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_pipe(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_poll(cw_nxo_t *a_thread);
#endif

void
systemdict_pop(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_ppid(cw_nxo_t *a_thread);
#endif

void
systemdict_print(cw_nxo_t *a_thread);

void
systemdict_put(cw_nxo_t *a_thread);

void
systemdict_putinterval(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_pwd(cw_nxo_t *a_thread);
#endif

void
systemdict_quit(cw_nxo_t *a_thread);

void
systemdict_rand(cw_nxo_t *a_thread);

void
systemdict_read(cw_nxo_t *a_thread);

void
systemdict_readline(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_readlink(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_realtime(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_recv(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_rename(cw_nxo_t *a_thread);
#endif

void
systemdict_repeat(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_rmdir(cw_nxo_t *a_thread);
#endif

void
systemdict_roll(cw_nxo_t *a_thread);

void
systemdict_rot(cw_nxo_t *a_thread);

#ifdef CW_REAL
void
systemdict_round(cw_nxo_t *a_thread);
#endif

void
systemdict_sadn(cw_nxo_t *a_thread);

void
systemdict_saup(cw_nxo_t *a_thread);

void
systemdict_sbdup(cw_nxo_t *a_thread);

void
systemdict_sbpop(cw_nxo_t *a_thread);

void
systemdict_sbpush(cw_nxo_t *a_thread);

void
systemdict_sclear(cw_nxo_t *a_thread);

void
systemdict_scleartomark(cw_nxo_t *a_thread);

void
systemdict_scount(cw_nxo_t *a_thread);

void
systemdict_scounttomark(cw_nxo_t *a_thread);

void
systemdict_sdn(cw_nxo_t *a_thread);

void
systemdict_sdup(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_seek(cw_nxo_t *a_thread);
#endif

void
systemdict_self(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_send(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_serviceport(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_setegid(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_setenv(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_seteuid(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_setgid(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_setgstderr(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_setgstdin(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_setgstdout(cw_nxo_t *a_thread);
#endif

void
systemdict_setiobuf(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_setlocking(cw_nxo_t *a_thread);
#endif

void
systemdict_setnonblocking(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_setsockopt(cw_nxo_t *a_thread);
#endif

void
systemdict_setstderr(cw_nxo_t *a_thread);

void
systemdict_setstdin(cw_nxo_t *a_thread);

void
systemdict_setstdout(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_setuid(cw_nxo_t *a_thread);
#endif

void
systemdict_sexch(cw_nxo_t *a_thread);

void
systemdict_shift(cw_nxo_t *a_thread);

void
systemdict_sibdup(cw_nxo_t *a_thread);

void
systemdict_sibpop(cw_nxo_t *a_thread);

void
systemdict_sidup(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_signal(cw_nxo_t *a_thread);
#endif

#ifdef CW_REAL
void
systemdict_sin(cw_nxo_t *a_thread);
#endif

void
systemdict_sipop(cw_nxo_t *a_thread);

void
systemdict_snbpop(cw_nxo_t *a_thread);

void
systemdict_sndn(cw_nxo_t *a_thread);

void
systemdict_sndup(cw_nxo_t *a_thread);

void
systemdict_snip(cw_nxo_t *a_thread);

void
systemdict_snpop(cw_nxo_t *a_thread);

void
systemdict_snup(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_socket(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_socketpair(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_sockname(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_sockopt(cw_nxo_t *a_thread);
#endif

void
systemdict_sover(cw_nxo_t *a_thread);

void
systemdict_spop(cw_nxo_t *a_thread);

void
systemdict_spush(cw_nxo_t *a_thread);

#ifdef CW_REAL
void
systemdict_sqrt(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_srand(cw_nxo_t *a_thread);
#endif

void
systemdict_sroll(cw_nxo_t *a_thread);

void
systemdict_srot(cw_nxo_t *a_thread);

void
systemdict_stack(cw_nxo_t *a_thread);

void
systemdict_start(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_status(cw_nxo_t *a_thread);
#endif

void
systemdict_stderr(cw_nxo_t *a_thread);

void
systemdict_stdin(cw_nxo_t *a_thread);

void
systemdict_stdout(cw_nxo_t *a_thread);

void
systemdict_stop(cw_nxo_t *a_thread);

void
systemdict_stopped(cw_nxo_t *a_thread);

void
systemdict_string(cw_nxo_t *a_thread);

void
systemdict_stuck(cw_nxo_t *a_thread);

void
systemdict_sub(cw_nxo_t *a_thread);

void
systemdict_sunder(cw_nxo_t *a_thread);

void
systemdict_sup(cw_nxo_t *a_thread);

void
systemdict_sym_lp(cw_nxo_t *a_thread);

void
systemdict_sym_rp(cw_nxo_t *a_thread);

void
systemdict_sym_gt(cw_nxo_t *a_thread);

void
systemdict_sym_rb(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_symlink(cw_nxo_t *a_thread);
#endif

void
systemdict_tell(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_test(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_thread(cw_nxo_t *a_thread);
#endif

void
systemdict_threaddstack(cw_nxo_t *a_thread);

void
systemdict_threadestack(cw_nxo_t *a_thread);

void
systemdict_threadistack(cw_nxo_t *a_thread);

void
systemdict_threadostack(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_threadsdict(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_timedwait(cw_nxo_t *a_thread);
#endif

void
systemdict_token(cw_nxo_t *a_thread);

#ifdef CW_REAL
void
systemdict_trunc(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_truncate(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_trylock(cw_nxo_t *a_thread);
#endif

void
systemdict_tuck(cw_nxo_t *a_thread);

void
systemdict_type(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_uid(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_umask(cw_nxo_t *a_thread);
#endif

void
systemdict_undef(cw_nxo_t *a_thread);

void
systemdict_under(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_unlink(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_unlock(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_unsetenv(cw_nxo_t *a_thread);
#endif

void
systemdict_until(cw_nxo_t *a_thread);

void
systemdict_up(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_wait(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_waitpid(cw_nxo_t *a_thread);
#endif

void
systemdict_where(cw_nxo_t *a_thread);

void
systemdict_while(cw_nxo_t *a_thread);

void
systemdict_write(cw_nxo_t *a_thread);

void
systemdict_xcheck(cw_nxo_t *a_thread);

void
systemdict_xor(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_yield(cw_nxo_t *a_thread);
#endif
