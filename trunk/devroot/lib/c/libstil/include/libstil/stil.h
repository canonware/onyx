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

typedef struct cw_stil_s cw_stil_t;

struct cw_stil_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced;
	cw_mtx_t	lock;

	/* Global allocator. */
	cw_stilag_t	stilag;

        /*
         * Global hash of names (key: {name, len}, value: (stiloe_name *)).
         * This hash table keeps track of *all* name "values" in the virtual
         * machine.  When a name object is created, it actually adds a reference
         * to a stiloe_name in this hash and uses a pointer to that stiloe_name
         * as a unique key.
         *
         * Note that each stilt maintains a cache of stiloe_name's, so that
         * under normal circumstances, all locally allocated objects in a stilt
         * refer to a single reference to the global stiloe_name.
         */
	cw_mtx_t	name_lock;
	cw_dch_t	name_hash;

	cw_stilo_t	systemdict;
	cw_stilo_t	globaldict;

	cw_stilo_t	stdin_stilo;
	cw_stilo_t	stdout_stilo;
	cw_stilo_t	stderr_stilo;
};

/* stil. */
cw_stil_t	*stil_new(cw_stil_t *a_stil, cw_sint32_t (*a_read_f)(void
    *a_read_arg, cw_uint32_t a_len, cw_uint8_t *r_str), void *a_read_arg);
void		stil_delete(cw_stil_t *a_stil);

#define	stil_stilag_get(a_stil) (&(a_stil)->stilag)
#define	stil_name_lock_get(a_stil) (&(a_stil)->name_lock)
#define	stil_name_hash_get(a_stil) (&(a_stil)->name_hash)

#define	stil_systemdict_get(a_stil) (&(a_stil)->systemdict)
#define	stil_globaldict_get(a_stil) (&(a_stil)->systemdict)

#define	stil_stdin_get(a_stil) (&(a_stil)->stdin_stilo)
#define	stil_stdout_get(a_stil) (&(a_stil)->stdout_stilo)
#define	stil_stderr_get(a_stil) (&(a_stil)->stderr_stilo)
