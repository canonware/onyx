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

extern const cw_uint8_t *cw_g_nx_names[];

/* Same order as in nxn.c. */
typedef enum
{
    NXN_ZERO,
    NXN_sym_bang_hash,
    NXN_sym_hash_bang,
    NXN_sym_lp,
    NXN_sym_rp,
    NXN_sym_lt,
    NXN_sym_gt,
    NXN_sym_lb,
    NXN_sym_rb,
    NXN_abs,
    NXN_active,
    NXN_add,
    NXN_and,
    NXN_argv,
    NXN_array,
    NXN_arraytype,
#ifdef CW_REAL
    NXN_atan,
#endif
#ifdef CW_POSIX
    NXN_atime,
#endif
    NXN_begin,
    NXN_bind,
#ifdef CW_POSIX
    NXN_blksize,
    NXN_blocks,
#endif
    NXN_booleantype,
#ifdef CW_THREADS
    NXN_broadcast,
#endif
    NXN_bytesavailable,
    NXN_cat,
#ifdef CW_POSIX
    NXN_cd,
#endif
#ifdef CW_REAL
    NXN_ceiling,
#endif
#ifdef CW_POSIX
    NXN_chmod,
    NXN_chown,
#endif
    NXN_clear,
    NXN_cleardstack,
    NXN_cleartomark,
#ifdef CW_POSIX
    NXN_close,
#endif
    NXN_collect,
#ifdef CW_THREADS
    NXN_condition,
    NXN_conditiontype,
#endif
    NXN_copy,
#ifdef CW_REAL
    NXN_cos,
#endif
    NXN_count,
    NXN_countdstack,
    NXN_countestack,
    NXN_counttomark,
#ifdef CW_POSIX
    NXN_ctime,
#endif
    NXN_currentdict,
#ifdef CW_THREADS
    NXN_currentlocking,
#endif
#ifdef CW_REAL
    NXN_cvds,
#endif
    NXN_cve,
#ifdef CW_REAL
    NXN_cves,
#endif
    NXN_cvlit,
    NXN_cvn,
    NXN_cvrs,
    NXN_cvs,
    NXN_cvx,
    NXN_def,
#ifdef CW_THREADS
    NXN_detach,
#endif
#ifdef CW_POSIX
    NXN_dev,
#endif
    NXN_dict,
    NXN_dicttype,
    NXN_die,
#ifdef CW_POSIX
    NXN_dirforeach,
#endif
#ifdef CW_REAL
    NXN_div,
#endif
    NXN_dstack,
    NXN_dstackunderflow,
    NXN_dup,
    NXN_echeck,
#ifdef CW_POSIX
    NXN_egid,
#endif
    NXN_end,
#ifdef CW_POSIX
    NXN_envdict,
#endif
    NXN_eq,
    NXN_errorname,
    NXN_estack,
    NXN_estackoverflow,
#ifdef CW_POSIX
    NXN_euid,
#endif
    NXN_eval,
    NXN_exch,
#ifdef CW_POSIX
    NXN_exec,
#endif
    NXN_exit,
    NXN_exp,
    NXN_false,
    NXN_filetype,
    NXN_finotype,
#ifdef CW_REAL
    NXN_floor,
#endif
    NXN_flush,
    NXN_flushfile,
    NXN_for,
    NXN_foreach,
#ifdef CW_POSIX
    NXN_fork,
#endif
    NXN_gcdict,
    NXN_ge,
    NXN_get,
    NXN_getinterval,
#ifdef CW_POSIX
    NXN_gid,
#endif
    NXN_globaldict,
    NXN_gt,
    NXN_hooktag,
    NXN_hooktype,
    NXN_idiv,
    NXN_if,
    NXN_ifelse,
    NXN_index,
#ifdef CW_POSIX
    NXN_ino,
#endif
    NXN_integertype,
    NXN_invalidaccess,
    NXN_invalidexit,
    NXN_invalidfileaccess,
    NXN_iobuf,
    NXN_ioerror,
    NXN_istack,
#ifdef CW_THREADS
    NXN_join,
#endif
    NXN_known,
#ifdef CW_THREADS
    NXN_lcheck,
#endif
    NXN_le,
    NXN_length,
    NXN_limitcheck,
#ifdef CW_POSIX
    NXN_link,
#endif
#ifdef CW_REAL
    NXN_ln,
#endif
    NXN_load,
#ifdef CW_THREADS
    NXN_lock,
#endif
#ifdef CW_REAL
    NXN_log,
#endif
    NXN_loop,
    NXN_lt,
    NXN_mark,
    NXN_marktype,
#ifdef CW_POSIX
    NXN_mkdir,
#endif
    NXN_mod,
#ifdef CW_POSIX
    NXN_mode,
#endif
#ifdef HAVE_DLOPEN
    NXN_modload,
#endif
#ifdef CW_THREADS
    NXN_monitor,
#endif
#ifdef CW_POSIX
    NXN_mtime,
#endif
    NXN_mul,
#ifdef CW_THREADS
    NXN_mutex,
    NXN_mutextype,
#endif
    NXN_nametype,
    NXN_ndup,
    NXN_ne,
    NXN_neg,
    NXN_newerror,
#ifdef CW_POSIX
    NXN_nlink,
#endif
    NXN_not,
    NXN_npop,
#ifdef CW_POSIX
    NXN_nsleep,
#endif
    NXN_null,
    NXN_nulltype,
    NXN_onyxdict,
#ifdef CW_POSIX
    NXN_open,
#endif
    NXN_operatortype,
    NXN_or,
    NXN_ostack,
#ifdef CW_THREADS
    NXN_period,
#endif
#ifdef CW_POSIX
    NXN_pid,
#endif
    NXN_pmark,
    NXN_pmarktype,
    NXN_pop,
#ifdef CW_POSIX
    NXN_ppid,
#endif
    NXN_print,
    NXN_product,
    NXN_promptstring,
    NXN_put,
    NXN_putinterval,
#ifdef CW_POSIX
    NXN_pwd,
#endif
    NXN_quit,
    NXN_rand,
    NXN_rangecheck,
#ifdef CW_POSIX
    NXN_rdev,
#endif
    NXN_read,
    NXN_readline,
#ifdef CW_POSIX
    NXN_realtime,
#endif
#ifdef CW_REAL
    NXN_realtype,
#endif
#ifdef CW_POSIX
    NXN_rename,
#endif
    NXN_repeat,
#ifdef CW_POSIX
    NXN_rmdir,
#endif
    NXN_roll,
#ifdef CW_REAL
    NXN_round,
#endif
    NXN_sclear,
    NXN_scleartomark,
    NXN_scount,
    NXN_scounttomark,
    NXN_sdup,
#ifdef CW_POSIX
    NXN_seek,
#endif
#ifdef CW_THREADS
    NXN_self,
#endif
    NXN_setactive,
#ifdef CW_POSIX
    NXN_setegid,
    NXN_setenv,
    NXN_seteuid,
    NXN_setgid,
#endif
    NXN_setiobuf,
#ifdef CW_THREADS
    NXN_setlocking,
    NXN_setperiod,
#endif
    NXN_setthreshold,
#ifdef CW_POSIX
    NXN_setuid,
#endif
    NXN_sexch,
    NXN_shift,
#ifdef CW_THREADS
    NXN_signal,
#endif
#ifdef CW_REAL
    NXN_sin,
#endif
    NXN_sindex,
#ifdef CW_POSIX
    NXN_size,
#endif
    NXN_spop,
    NXN_spush,
#ifdef CW_REAL
    NXN_sqrt,
#endif
#ifdef CW_POSIX
    NXN_srand,
#endif
    NXN_sroll,
    NXN_stack,
    NXN_stacktype,
    NXN_stackunderflow,
    NXN_start,
    NXN_stats,
#ifdef CW_POSIX
    NXN_status,
#endif
    NXN_stdin,
    NXN_stderr,
    NXN_stdout,
    NXN_stop,
    NXN_stopped,
    NXN_string,
    NXN_stringtype,
    NXN_sub,
#ifdef CW_POSIX
    NXN_symlink,
#endif
    NXN_syntaxerror,
    NXN_system,
    NXN_systemdict,
    NXN_tell,
#ifdef CW_POSIX
    NXN_test,
#endif
#ifdef CW_THREADS
    NXN_thread,
    NXN_threadtype,
#endif
    NXN_threshold,
    NXN_throw,
#ifdef CW_THREADS
    NXN_timedwait,
#endif
    NXN_token,
    NXN_true,
#ifdef CW_REAL
    NXN_trunc,
#endif
#ifdef CW_POSIX
    NXN_truncate,
#endif
#ifdef CW_THREADS
    NXN_trylock,
#endif
    NXN_type,
    NXN_typecheck,
#ifdef CW_POSIX
    NXN_uid,
#endif
    NXN_undef,
    NXN_undefined,
    NXN_undefinedfilename,
    NXN_undefinedresult,
#ifdef CW_POSIX
    NXN_unlink,
#endif
#ifdef CW_THREADS
    NXN_unlock,
#endif
    NXN_unmatchedfino,
    NXN_unmatchedmark,
    NXN_unregistered,
#ifdef CW_POSIX
    NXN_unsetenv,
#endif
    NXN_version,
#ifdef CW_THREADS
    NXN_wait,
#endif
#ifdef CW_POSIX
    NXN_waitpid,
#endif
    NXN_where,
    NXN_write,
    NXN_xcheck,
    NXN_xor
#ifdef CW_THREADS
    ,
    NXN_yield
#define NXN_LAST NXN_yield
#else
#define NXN_LAST NXN_xor
#endif
} cw_nxn_t;

#ifndef CW_USE_INLINES
const cw_uint8_t *nxn_str(cw_nxn_t a_nxn);
cw_uint32_t nxn_len(cw_nxn_t a_nxn);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXN_C_))
CW_INLINE const cw_uint8_t *
nxn_str(cw_nxn_t a_nxn)
{
    cw_assert(a_nxn > NXN_ZERO && a_nxn <= NXN_LAST);

    return cw_g_nx_names[a_nxn];
}

CW_INLINE cw_uint32_t
nxn_len(cw_nxn_t a_nxn)
{
    cw_assert(a_nxn > NXN_ZERO && a_nxn <= NXN_LAST);

    return strlen(cw_g_nx_names[a_nxn]);
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXN_C_)) */
