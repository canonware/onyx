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

extern const cw_uint8_t *cw_g_nx_names[];

/* Same order as in nxn.c. */
typedef enum {
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
#ifdef _CW_POSIX
	NXN_atime,
#endif
	NXN_begin,
	NXN_bind,
#ifdef _CW_POSIX
	NXN_blksize,
	NXN_blocks,
#endif
	NXN_booleantype,
#ifdef _CW_THREADS
	NXN_broadcast,
#endif
	NXN_bytesavailable,
	NXN_catenate,
#ifdef _CW_POSIX
	NXN_cd,
	NXN_chmod,
	NXN_chown,
#endif
	NXN_clear,
	NXN_cleardstack,
	NXN_cleartomark,
#ifdef _CW_POSIX
	NXN_close,
#endif
	NXN_collect,
#ifdef _CW_THREADS
	NXN_condition,
	NXN_conditiontype,
#endif
	NXN_copy,
	NXN_count,
	NXN_countdstack,
	NXN_countestack,
	NXN_counttomark,
#ifdef _CW_POSIX
	NXN_ctime,
#endif
	NXN_currentdict,
#ifdef _CW_THREADS
	NXN_currentlocking,
#endif
	NXN_cve,
	NXN_cvlit,
	NXN_cvn,
	NXN_cvrs,
	NXN_cvs,
	NXN_cvx,
	NXN_def,
#ifdef _CW_THREADS
	NXN_detach,
#endif
#ifdef _CW_POSIX
	NXN_dev,
#endif
	NXN_dict,
	NXN_dicttype,
	NXN_die,
#ifdef _CW_POSIX
	NXN_dirforeach,
#endif
	NXN_div,
	NXN_dstack,
	NXN_dstackunderflow,
	NXN_dup,
	NXN_echeck,
#ifdef _CW_POSIX
	NXN_egid,
#endif
	NXN_end,
#ifdef _CW_POSIX
	NXN_envdict,
#endif
	NXN_eq,
	NXN_errorname,
	NXN_estack,
	NXN_estackoverflow,
#ifdef _CW_POSIX
	NXN_euid,
#endif
	NXN_eval,
	NXN_exch,
#ifdef _CW_POSIX
	NXN_exec,
#endif
	NXN_exit,
	NXN_exp,
	NXN_false,
	NXN_filetype,
	NXN_finotype,
	NXN_flush,
	NXN_flushfile,
	NXN_for,
	NXN_foreach,
#ifdef _CW_POSIX
	NXN_fork,
#endif
	NXN_gcdict,
	NXN_ge,
	NXN_get,
	NXN_getinterval,
#ifdef _CW_POSIX
	NXN_gid,
#endif
	NXN_globaldict,
	NXN_gt,
	NXN_hooktag,
	NXN_hooktype,
	NXN_if,
	NXN_ifelse,
	NXN_index,
#ifdef _CW_POSIX
	NXN_ino,
#endif
	NXN_integertype,
	NXN_invalidaccess,
	NXN_invalidexit,
	NXN_invalidfileaccess,
	NXN_ioerror,
	NXN_istack,
#ifdef _CW_THREADS
	NXN_join,
#endif
	NXN_known,
#ifdef _CW_THREADS
	NXN_lcheck,
#endif
	NXN_le,
	NXN_length,
	NXN_limitcheck,
#ifdef _CW_POSIX
	NXN_link,
#endif
	NXN_load,
#ifdef _CW_THREADS
	NXN_lock,
#endif
	NXN_loop,
	NXN_lt,
	NXN_mark,
	NXN_marktype,
#ifdef _CW_POSIX
	NXN_mkdir,
#endif
	NXN_mod,
#ifdef _CW_POSIX
	NXN_mode,
#endif
#ifdef _CW_THREADS
	NXN_monitor,
#endif
#ifdef _CW_POSIX
	NXN_mtime,
#endif
	NXN_mul,
#ifdef _CW_THREADS
	NXN_mutex,
	NXN_mutextype,
#endif
	NXN_nametype,
	NXN_ndup,
	NXN_ne,
	NXN_neg,
	NXN_newerror,
#ifdef _CW_POSIX
	NXN_nlink,
#endif
	NXN_not,
	NXN_npop,
#ifdef _CW_POSIX
	NXN_nsleep,
#endif
	NXN_null,
	NXN_nulltype,
#ifdef _CW_POSIX
	NXN_open,
#endif
	NXN_operatortype,
	NXN_or,
	NXN_ostack,
#ifdef _CW_THREADS
	NXN_period,
#endif
#ifdef _CW_POSIX
	NXN_pid,
#endif
	NXN_pmark,
	NXN_pmarktype,
	NXN_pop,
#ifdef _CW_POSIX
	NXN_ppid,
#endif
	NXN_print,
	NXN_product,
	NXN_promptstring,
	NXN_put,
	NXN_putinterval,
#ifdef _CW_POSIX
	NXN_pwd,
#endif
	NXN_quit,
	NXN_rand,
	NXN_rangecheck,
#ifdef _CW_POSIX
	NXN_rdev,
#endif
	NXN_read,
	NXN_readline,
#ifdef _CW_POSIX
	NXN_realtime,
	NXN_rename,
#endif
	NXN_repeat,
#ifdef _CW_POSIX
	NXN_rmdir,
#endif
	NXN_roll,
	NXN_sclear,
	NXN_scleartomark,
	NXN_scount,
	NXN_scounttomark,
	NXN_sdup,
#ifdef _CW_POSIX
	NXN_seek,
#endif
#ifdef _CW_THREADS
	NXN_self,
#endif
	NXN_setactive,
#ifdef _CW_POSIX
	NXN_setegid,
	NXN_setenv,
	NXN_seteuid,
	NXN_setgid,
#endif
#ifdef _CW_THREADS
	NXN_setlocking,
	NXN_setperiod,
#endif
	NXN_setthreshold,
#ifdef _CW_POSIX
	NXN_setuid,
#endif
	NXN_sexch,
	NXN_shift,
#ifdef _CW_THREADS
	NXN_signal,
#endif
	NXN_sindex,
#ifdef _CW_POSIX
	NXN_size,
#endif
	NXN_spop,
	NXN_spush,
#ifdef _CW_POSIX
	NXN_srand,
#endif
	NXN_sroll,
	NXN_stack,
	NXN_stacktype,
	NXN_stackunderflow,
	NXN_start,
	NXN_stats,
#ifdef _CW_POSIX
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
#ifdef _CW_POSIX
	NXN_symlink,
#endif
	NXN_syntaxerror,
	NXN_system,
	NXN_systemdict,
	NXN_tell,
#ifdef _CW_POSIX
	NXN_test,
#endif
#ifdef _CW_THREADS
	NXN_thread,
	NXN_threadtype,
#endif
	NXN_threshold,
	NXN_throw,
#ifdef _CW_THREADS
	NXN_timedwait,
#endif
	NXN_token,
	NXN_true,
#ifdef _CW_POSIX
	NXN_truncate,
#endif
#ifdef _CW_THREADS
	NXN_trylock,
#endif
	NXN_type,
	NXN_typecheck,
#ifdef _CW_POSIX
	NXN_uid,
#endif
	NXN_undef,
	NXN_undefined,
	NXN_undefinedfilename,
	NXN_undefinedresult,
#ifdef _CW_POSIX
	NXN_unlink,
#endif
#ifdef _CW_THREADS
	NXN_unlock,
#endif
	NXN_unmatchedfino,
	NXN_unmatchedmark,
	NXN_unregistered,
#ifdef _CW_POSIX
	NXN_unsetenv,
#endif
	NXN_version,
#ifdef _CW_THREADS
	NXN_wait,
#endif
#ifdef _CW_POSIX
	NXN_waitpid,
#endif
	NXN_where,
	NXN_write,
	NXN_xcheck,
	NXN_xor
#ifdef _CW_THREADS
	,
	NXN_yield
#define	NXN_LAST	NXN_yield
#else
#define	NXN_LAST	NXN_xor
#endif
} cw_nxn_t;

#ifndef _CW_USE_INLINES
const cw_uint8_t *nxn_str(cw_nxn_t a_nxn);
cw_uint32_t nxn_len(cw_nxn_t a_nxn);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NXN_C_))
_CW_INLINE const cw_uint8_t *
nxn_str(cw_nxn_t a_nxn)
{
	_cw_assert(a_nxn > NXN_ZERO && a_nxn <= NXN_LAST);

	return cw_g_nx_names[a_nxn];
}

_CW_INLINE cw_uint32_t
nxn_len(cw_nxn_t a_nxn)
{
	_cw_assert(a_nxn > NXN_ZERO && a_nxn <= NXN_LAST);

	return strlen(cw_g_nx_names[a_nxn]);
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXN_C_)) */
