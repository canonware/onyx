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

struct cw_nx_s {
#ifdef _CW_DBG
	cw_uint32_t	magic;
#define _CW_NX_MAGIC	0xae9678fd
#endif

	cw_bool_t	is_malloced;

	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;

	/*
	 * Set to TRUE before the final garbage collection so that all objects
	 * get collected.
	 */
	cw_bool_t	shutdown;

        /*
         * Global hash of names (key: {name, len}, value: (nxoe_name *)).
         * This hash table keeps track of *all* name "values" in the virtual
         * machine.  When a name object is created, it actually adds a reference
         * to a nxoe_name in this hash and uses a pointer to that nxoe_name
         * as a unique key.
         */
#ifdef _CW_THREADS
	cw_mtx_t	name_lock;
#endif
	cw_dch_t	name_hash;

	/* Memory allocator. */
	cw_nxa_t	nxa;

	/*
	 * Dictionaries.
	 */
	cw_nxo_t	threadsdict;
	cw_nxo_t	systemdict;
	cw_nxo_t	globaldict;
#ifdef _CW_POSIX
	cw_nxo_t	envdict;
#endif

	/*
	 * Files.
	 */
	cw_nxo_t	stdin_nxo;
	cw_nxo_t	stdout_nxo;
	cw_nxo_t	stderr_nxo;

	/*
	 * Thread initialization hook.
	 */
	cw_op_t		*thread_init;
};

/* nx. */
cw_nx_t *nx_new(cw_nx_t *a_nx, cw_op_t *a_thread_init, int a_argc, char
    **a_argv, char **a_envp);
void	nx_delete(cw_nx_t *a_nx);

#ifndef _CW_USE_INLINES
cw_nxa_t *nx_nxa_get(cw_nx_t *a_nx);
cw_nxo_t *nx_systemdict_get(cw_nx_t *a_nx);
cw_nxo_t *nx_globaldict_get(cw_nx_t *a_nx);
#ifdef _CW_POSIX
cw_nxo_t *nx_envdict_get(cw_nx_t *a_nx);
#endif
cw_nxo_t *nx_stdin_get(cw_nx_t *a_nx);
cw_nxo_t *nx_stdout_get(cw_nx_t *a_nx);
cw_nxo_t *nx_stderr_get(cw_nx_t *a_nx);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NX_C_))
_CW_INLINE cw_nxa_t *
nx_nxa_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->nxa;
}

_CW_INLINE cw_nxo_t *
nx_systemdict_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->systemdict;
}

_CW_INLINE cw_nxo_t *
nx_globaldict_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->globaldict;
}

#ifdef _CW_POSIX
_CW_INLINE cw_nxo_t *
nx_envdict_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->envdict;
}
#endif

_CW_INLINE cw_nxo_t *
nx_stdin_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->stdin_nxo;
}

_CW_INLINE cw_nxo_t *
nx_stdout_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->stdout_nxo;
}

_CW_INLINE cw_nxo_t *
nx_stderr_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->stderr_nxo;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NX_C_)) */
