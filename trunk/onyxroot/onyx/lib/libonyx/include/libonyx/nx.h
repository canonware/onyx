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
#ifdef _LIBONYX_DBG
	cw_uint32_t	magic;
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
	cw_mtx_t	name_lock;
	cw_dch_t	name_hash;

	/* Memory allocator. */
	cw_nxa_t	nxa;

	/*
	 * Dictionaries.
	 */
	cw_nxo_t	threadsdict;
	cw_nxo_t	systemdict;
	cw_nxo_t	globaldict;
	cw_nxo_t	envdict;
	cw_nxo_t	sprintdict;

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
cw_nx_t *nx_new(cw_nx_t *a_nx, int a_argc, char **a_argv, char **a_envp,
    cw_nxo_file_read_t *a_stdin, cw_nxo_file_write_t *a_stdout,
    cw_nxo_file_write_t *a_stderr, void *a_arg, cw_op_t *a_thread_init);
void	nx_delete(cw_nx_t *a_nx);

#define	nx_nxa_get(a_nx) (&(a_nx)->nxa)

#define	nx_systemdict_get(a_nx) (&(a_nx)->systemdict)
#define	nx_globaldict_get(a_nx) (&(a_nx)->globaldict)
#define	nx_envdict_get(a_nx) (&(a_nx)->envdict)
#define	nx_sprintdict_get(a_nx) (&(a_nx)->sprintdict)

#define	nx_stdin_get(a_nx) (&(a_nx)->stdin_nxo)
#define	nx_stdout_get(a_nx) (&(a_nx)->stdout_nxo)
#define	nx_stderr_get(a_nx) (&(a_nx)->stderr_nxo)
