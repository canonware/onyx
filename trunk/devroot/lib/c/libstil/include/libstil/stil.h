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

struct cw_stil_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced;

	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;

        /*
         * Global hash of names (key: {name, len}, value: (stiloe_name *)).
         * This hash table keeps track of *all* name "values" in the virtual
         * machine.  When a name object is created, it actually adds a reference
         * to a stiloe_name in this hash and uses a pointer to that stiloe_name
         * as a unique key.
         */
	cw_mtx_t	name_lock;
	cw_dch_t	name_hash;

	/* Memory allocator. */
	cw_stila_t	stila;

	/*
	 * Dictionaries.
	 */
	cw_stilo_t	threadsdict;
	cw_stilo_t	systemdict;
	cw_stilo_t	globaldict;
	cw_stilo_t	envdict;

	/*
	 * Files.
	 */
	cw_stilo_t	stdin_stilo;
	cw_stilo_t	stdout_stilo;
	cw_stilo_t	stderr_stilo;

	/*
	 * Thread initialization hook.
	 */
	cw_op_t		*thread_init;
};

/* stil. */
cw_stil_t *stil_new(cw_stil_t *a_stil, int a_argc, char **a_argv, char **a_envp,
    cw_stilo_file_read_t *a_stdin, cw_stilo_file_write_t *a_stdout,
    cw_stilo_file_write_t *a_stderr, void *a_arg, cw_op_t *a_thread_init);
void	stil_delete(cw_stil_t *a_stil);

#define	stil_stila_get(a_stil) (&(a_stil)->stila)

#define	stil_systemdict_get(a_stil) (&(a_stil)->systemdict)
#define	stil_globaldict_get(a_stil) (&(a_stil)->globaldict)
#define	stil_envdict_get(a_stil) (&(a_stil)->envdict)

#define	stil_stdin_get(a_stil) (&(a_stil)->stdin_stilo)
#define	stil_stdout_get(a_stil) (&(a_stil)->stdout_stilo)
#define	stil_stderr_get(a_stil) (&(a_stil)->stderr_stilo)
