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

void
systemdict_add(cw_nxo_t *a_thread);

void
systemdict_and(cw_nxo_t *a_thread);

void
systemdict_array(cw_nxo_t *a_thread);

#ifdef CW_REAL
void
systemdict_atan(cw_nxo_t *a_thread);
#endif

void
systemdict_begin(cw_nxo_t *a_thread);

void
systemdict_bind(cw_nxo_t *a_thread);

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

void
systemdict_chown(cw_nxo_t *a_thread);
#endif

void
systemdict_clear(cw_nxo_t *a_thread);

void
systemdict_cleardstack(cw_nxo_t *a_thread);

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
systemdict_fork(cw_nxo_t *a_thread);
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

void
systemdict_gt(cw_nxo_t *a_thread);

void
systemdict_hooktag(cw_nxo_t *a_thread);

void
systemdict_idiv(cw_nxo_t *a_thread);

void
systemdict_if(cw_nxo_t *a_thread);

void
systemdict_ifelse(cw_nxo_t *a_thread);

void
systemdict_index(cw_nxo_t *a_thread);

void
systemdict_iobuf(cw_nxo_t *a_thread);

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

void
systemdict_mod(cw_nxo_t *a_thread);

#ifdef HAVE_DLOPEN
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
systemdict_ndup(cw_nxo_t *a_thread);

void
systemdict_ne(cw_nxo_t *a_thread);

void
systemdict_neg(cw_nxo_t *a_thread);

void
systemdict_not(cw_nxo_t *a_thread);

void
systemdict_npop(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_nsleep(cw_nxo_t *a_thread);

void
systemdict_open(cw_nxo_t *a_thread);
#endif

void
systemdict_or(cw_nxo_t *a_thread);

void
systemdict_ostack(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_pid(cw_nxo_t *a_thread);
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
systemdict_realtime(cw_nxo_t *a_thread);

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

#ifdef CW_REAL
void
systemdict_round(cw_nxo_t *a_thread);
#endif

void
systemdict_sclear(cw_nxo_t *a_thread);

void
systemdict_scleartomark(cw_nxo_t *a_thread);

void
systemdict_scount(cw_nxo_t *a_thread);

void
systemdict_scounttomark(cw_nxo_t *a_thread);

void
systemdict_sdup(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_seek(cw_nxo_t *a_thread);
#endif

#ifdef CW_THREADS
void
systemdict_self(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_setegid(cw_nxo_t *a_thread);

void
systemdict_setenv(cw_nxo_t *a_thread);

void
systemdict_seteuid(cw_nxo_t *a_thread);

void
systemdict_setgid(cw_nxo_t *a_thread);
#endif

void
systemdict_setiobuf(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_setlocking(cw_nxo_t *a_thread);
#endif

#ifdef CW_POSIX
void
systemdict_setuid(cw_nxo_t *a_thread);
#endif

void
systemdict_sexch(cw_nxo_t *a_thread);

void
systemdict_shift(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_signal(cw_nxo_t *a_thread);
#endif

#ifdef CW_REAL
void
systemdict_sin(cw_nxo_t *a_thread);
#endif

void
systemdict_sindex(cw_nxo_t *a_thread);

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
systemdict_sub(cw_nxo_t *a_thread);

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
systemdict_type(cw_nxo_t *a_thread);

#ifdef CW_POSIX
void
systemdict_uid(cw_nxo_t *a_thread);
#endif

void
systemdict_undef(cw_nxo_t *a_thread);

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
systemdict_write(cw_nxo_t *a_thread);

void
systemdict_xcheck(cw_nxo_t *a_thread);

void
systemdict_xor(cw_nxo_t *a_thread);

#ifdef CW_THREADS
void
systemdict_yield(cw_nxo_t *a_thread);
#endif

#ifndef CW_USE_INLINES
/* Rename the inline functions, since they're not inlined.  The normal
 * definitions for these functions are #ifdef'ed out in this case. */
#define systemdict_inline_add systemdict_add
#define systemdict_inline_dup systemdict_dup
#define systemdict_inline_exch systemdict_exch
#define systemdict_inline_index systemdict_index
#define systemdict_inline_pop systemdict_pop
#define systemdict_inline_roll systemdict_roll

void
systemdict_inline_add(cw_nxo_t *a_thread);

void
systemdict_inline_dup(cw_nxo_t *a_thread);

void
systemdict_inline_exch(cw_nxo_t *a_thread);

void
systemdict_inline_index(cw_nxo_t *a_thread);

void
systemdict_inline_pop(cw_nxo_t *a_thread);

void
systemdict_inline_roll(cw_nxo_t *a_thread);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_SYSTEMDICT_C_))
CW_INLINE void
systemdict_inline_add(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_nxoi_t integer_a, integer_b;
#ifdef CW_REAL
    cw_bool_t do_real;
    cw_nxor_t real_a, real_b;
#endif

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);
    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    do_real = FALSE;
#endif
	    integer_a = nxo_integer_get(nxo_a);
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    do_real = TRUE;
	    real_a = nxo_real_get(nxo_a);
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    if (do_real)
	    {
		real_b = (cw_nxor_t) nxo_integer_get(nxo_b);
	    }
	    else
#endif
	    {
		integer_b = nxo_integer_get(nxo_b);
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    real_b = nxo_real_get(nxo_b);
	    if (do_real == FALSE)
	    {
		do_real = TRUE;
		real_a = (cw_nxor_t) integer_a;
	    }
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

#ifdef CW_REAL
    if (do_real)
    {
	/* nxo_a may be an integer, so use nxo_real_new() rather than
	 * nxo_real_set(). */
	nxo_real_new(nxo_a, real_a + real_b);
    }
    else
#endif
    {
	nxo_integer_set(nxo_a, integer_a + integer_b);
    }

    nxo_stack_pop(ostack);
}

CW_INLINE void
systemdict_inline_dup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *orig, *dup;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(orig, ostack, a_thread);
    dup = nxo_stack_push(ostack);
    nxo_dup(dup, orig);
}

CW_INLINE void
systemdict_inline_exch(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;

    ostack = nxo_thread_ostack_get(a_thread);

    if (nxo_stack_exch(ostack))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
    }
}

CW_INLINE void
systemdict_inline_index(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo, *orig;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    NXO_STACK_NGET(orig, ostack, a_thread, index + 1);
    nxo_dup(nxo, orig);
}

CW_INLINE void
systemdict_inline_pop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_POP(ostack, a_thread);
}

CW_INLINE void
systemdict_inline_roll(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxoi_t count, amount;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    amount = nxo_integer_get(nxo);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 1)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_stack_npop(ostack, 2);
    if (nxo_stack_roll(ostack, count, amount))
    {
	cw_nxo_t *nxo;

	/* Stack underflow.  Restore the stack to its original state, then throw
	 * an error. */
	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, count);
	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, amount);

	nxo_thread_nerror(a_thread, NXN_stackunderflow);
    }
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_SYSTEMDICT_C_)) */
