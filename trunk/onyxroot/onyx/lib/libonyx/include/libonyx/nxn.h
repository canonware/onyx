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
	NXN_atime,
	NXN_begin,
	NXN_bind,
	NXN_blksize,
	NXN_blocks,
	NXN_booleantype,
#ifdef _CW_THREADS
	NXN_broadcast,
#endif
	NXN_bytesavailable,
	NXN_catenate,
	NXN_cd,
	NXN_chmod,
	NXN_chown,
	NXN_clear,
	NXN_cleardstack,
	NXN_cleartomark,
	NXN_close,
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
	NXN_ctime,
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
	NXN_dev,
	NXN_dict,
	NXN_dicttype,
	NXN_die,
	NXN_dirforeach,
	NXN_div,
	NXN_dstack,
	NXN_dstackunderflow,
	NXN_dup,
	NXN_echeck,
	NXN_egid,
	NXN_end,
	NXN_envdict,
	NXN_eq,
	NXN_errorname,
	NXN_estack,
	NXN_estackoverflow,
	NXN_euid,
	NXN_eval,
	NXN_exch,
	NXN_exec,
	NXN_exit,
	NXN_exp,
	NXN_false,
	NXN_filetype,
	NXN_finotype,
	NXN_flush,
	NXN_flushfile,
	NXN_for,
	NXN_foreach,
	NXN_fork,
	NXN_gcdict,
	NXN_ge,
	NXN_get,
	NXN_getinterval,
	NXN_gid,
	NXN_globaldict,
	NXN_gt,
	NXN_hooktag,
	NXN_hooktype,
	NXN_if,
	NXN_ifelse,
	NXN_index,
	NXN_ino,
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
	NXN_link,
	NXN_load,
#ifdef _CW_THREADS
	NXN_lock,
#endif
	NXN_loop,
	NXN_lt,
	NXN_mark,
	NXN_marktype,
	NXN_mkdir,
	NXN_mod,
	NXN_mode,
#ifdef _CW_THREADS
	NXN_monitor,
#endif
	NXN_mtime,
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
	NXN_nlink,
	NXN_not,
	NXN_npop,
	NXN_nsleep,
	NXN_null,
	NXN_nulltype,
	NXN_open,
	NXN_operatortype,
	NXN_or,
	NXN_ostack,
#ifdef _CW_THREADS
	NXN_period,
#endif
	NXN_pid,
	NXN_pmark,
	NXN_pmarktype,
	NXN_pop,
	NXN_ppid,
	NXN_print,
	NXN_product,
	NXN_promptstring,
	NXN_put,
	NXN_putinterval,
	NXN_pwd,
	NXN_quit,
	NXN_rand,
	NXN_rangecheck,
	NXN_rdev,
	NXN_read,
	NXN_readline,
	NXN_realtime,
	NXN_rename,
	NXN_repeat,
	NXN_rmdir,
	NXN_roll,
	NXN_sclear,
	NXN_scleartomark,
	NXN_scount,
	NXN_scounttomark,
	NXN_sdup,
	NXN_seek,
#ifdef _CW_THREADS
	NXN_self,
#endif
	NXN_setactive,
	NXN_setegid,
	NXN_setenv,
	NXN_seteuid,
	NXN_setgid,
#ifdef _CW_THREADS
	NXN_setlocking,
	NXN_setperiod,
#endif
	NXN_setthreshold,
	NXN_setuid,
	NXN_sexch,
	NXN_shift,
#ifdef _CW_THREADS
	NXN_signal,
#endif
	NXN_sindex,
	NXN_size,
	NXN_spop,
	NXN_spush,
	NXN_srand,
	NXN_sroll,
	NXN_stack,
	NXN_stacktype,
	NXN_stackunderflow,
	NXN_start,
	NXN_stats,
	NXN_status,
	NXN_stdin,
	NXN_stderr,
	NXN_stdout,
	NXN_stop,
	NXN_stopped,
	NXN_string,
	NXN_stringtype,
	NXN_sub,
	NXN_symlink,
	NXN_syntaxerror,
	NXN_system,
	NXN_systemdict,
	NXN_tell,
	NXN_test,
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
	NXN_truncate,
#ifdef _CW_THREADS
	NXN_trylock,
#endif
	NXN_type,
	NXN_typecheck,
	NXN_uid,
	NXN_undef,
	NXN_undefined,
	NXN_undefinedfilename,
	NXN_undefinedresult,
	NXN_unlink,
#ifdef _CW_THREADS
	NXN_unlock,
#endif
	NXN_unmatchedfino,
	NXN_unmatchedmark,
	NXN_unregistered,
	NXN_unsetenv,
	NXN_version,
#ifdef _CW_THREADS
	NXN_wait,
#endif
	NXN_waitpid,
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
