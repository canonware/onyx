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

extern const cw_uint8_t *cw_g_stil_names[];

#define	stiln_str(a_stiln)	cw_g_stil_names[(a_stiln)]
#define	stiln_len(a_stiln)	strlen(cw_g_stil_names[(a_stiln)])

/* Same order as in stiln.c. */
typedef enum {
	STILN_ZERO,
	STILN_sym_bang_hash,
	STILN_sym_hash_bang,
	STILN_sym_lp,
	STILN_sym_rp,
	STILN_sym_lt,
	STILN_sym_gt,
	STILN_sym_lb,
	STILN_sym_rb,
	STILN_abs,
	STILN_active,
	STILN_add,
	STILN_aload,
	STILN_and,
	STILN_argv,
	STILN_array,
	STILN_arraytype,
	STILN_astore,
	STILN_begin,
	STILN_bind,
	STILN_booleantype,
	STILN_broadcast,
	STILN_bytesavailable,
	STILN_chmod,
	STILN_chown,
	STILN_clear,
	STILN_cleardstack,
	STILN_cleartomark,
	STILN_closefile,
	STILN_collect,
	STILN_collections,
	STILN_column,
	STILN_command,
	STILN_condition,
	STILN_conditiontype,
	STILN_copy,
	STILN_count,
	STILN_countdstack,
	STILN_countestack,
	STILN_counttomark,
	STILN_current,
	STILN_currentdict,
	STILN_currenterror,
	STILN_currentfile,
	STILN_currentlocking,
	STILN_cvlit,
	STILN_cvn,
	STILN_cvrs,
	STILN_cvs,
	STILN_cvx,
	STILN_def,
	STILN_detach,
	STILN_dict,
	STILN_dicttype,
	STILN_dirforeach,
	STILN_div,
	STILN_dstack,
	STILN_dstackunderflow,
	STILN_dump,
	STILN_dup,
	STILN_end,
	STILN_envdict,
	STILN_eq,
	STILN_errordict,
	STILN_errorname,
	STILN_estack,
	STILN_estackoverflow,
	STILN_eval,
	STILN_exch,
	STILN_exec,
	STILN_exit,
	STILN_exp,
	STILN_false,
	STILN_filetype,
	STILN_fino,
	STILN_finotype,
	STILN_flush,
	STILN_flushfile,
	STILN_for,
	STILN_foreach,
	STILN_fork,
	STILN_gcdict,
	STILN_ge,
	STILN_get,
	STILN_getinterval,
	STILN_globaldict,
	STILN_gt,
	STILN_handleerror,
	STILN_hooktype,
	STILN_if,
	STILN_ifelse,
	STILN_index,
	STILN_integertype,
	STILN_interrupt,
	STILN_invalidaccess,
	STILN_invalidcontext,
	STILN_invalidexit,
	STILN_invalidfileaccess,
	STILN_ioerror,
	STILN_join,
	STILN_known,
	STILN_lcheck,
	STILN_le,
	STILN_length,
	STILN_limitcheck,
	STILN_line,
	STILN_link,
	STILN_load,
	STILN_lock,
	STILN_loop,
	STILN_lt,
	STILN_mark,
	STILN_marktype,
	STILN_maximum,
	STILN_mkdir,
	STILN_mod,
	STILN_mul,
	STILN_mutex,
	STILN_mutextype,
	STILN_nametype,
	STILN_ne,
	STILN_neg,
	STILN_new,
	STILN_newerror,
	STILN_not,
	STILN_nsleep,
	STILN_null,
	STILN_nulltype,
	STILN_open,
	STILN_operatortype,
	STILN_or,
	STILN_ostack,
	STILN_period,
	STILN_pop,
	STILN_print,
	STILN_product,
	STILN_promptstring,
	STILN_pstack,
	STILN_put,
	STILN_putinterval,
	STILN_quit,
	STILN_rand,
	STILN_rangecheck,
	STILN_read,
	STILN_readline,
	STILN_realtime,
	STILN_recordstacks,
	STILN_renamefile,
	STILN_repeat,
	STILN_roll,
	STILN_run,
	STILN_sclear,
	STILN_scleartomark,
	STILN_scount,
	STILN_scounttomark,
	STILN_sdup,
	STILN_seek,
	STILN_self,
	STILN_setactive,
	STILN_setlocking,
	STILN_setperiod,
	STILN_setthreshold,
	STILN_sexch,
	STILN_shift,
	STILN_signal,
	STILN_sindex,
	STILN_spop,
	STILN_sprint,
	STILN_spush,
	STILN_srand,
	STILN_sroll,
	STILN_stack,
	STILN_stacktype,
	STILN_stackunderflow,
	STILN_start,
	STILN_stat,
	STILN_stdin,
	STILN_stderr,
	STILN_stdout,
	STILN_stop,
	STILN_stopped,
	STILN_store,
	STILN_string,
	STILN_stringtype,
	STILN_sub,
	STILN_sum,
	STILN_symlink,
	STILN_syntaxerror,
	STILN_system,
	STILN_systemdict,
	STILN_tell,
	STILN_test,
	STILN_thread,
	STILN_threaddict,
	STILN_threadtype,
	STILN_threshold,
	STILN_timedwait,
	STILN_timeout,
	STILN_token,
	STILN_true,
	STILN_truncate,
	STILN_trylock,
	STILN_type,
	STILN_typecheck,
	STILN_undef,
	STILN_undefined,
	STILN_undefinedfilename,
	STILN_undefinedresult,
	STILN_unlink,
	STILN_unlock,
	STILN_unmatchedfino,
	STILN_unmatchedmark,
	STILN_unregistered,
	STILN_userdict,
	STILN_version,
	STILN_wait,
	STILN_waitpid,
	STILN_where,
	STILN_write,
	STILN_xcheck,
	STILN_xor,
	STILN_yield
#define	STILN_LAST	STILN_yield
} cw_stiln_t;
