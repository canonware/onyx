/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

typedef struct cw_stilng_s cw_stilng_t;
typedef struct cw_stilnt_s cw_stilnt_t;

/* Defined here to resolve circular dependencies. */
typedef struct cw_stil_bufc_s cw_stil_bufc_t;
typedef struct cw_stila_s cw_stila_t;
typedef struct cw_stilag_s cw_stilag_t;
typedef struct cw_stilat_s cw_stilat_t;
typedef struct cw_stiln_s cw_stiln_t;
typedef struct cw_stilt_s cw_stilt_t;
typedef struct cw_stilsc_s cw_stilsc_t;
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;

/*
 * Name.
 */
struct cw_stiln_s {
	/* Must be held during access to keyed_refs. */
	cw_mtx_t	lock;
	/*
	 * If non-NULL, a hash of keyed references to this object.  Keyed
	 * references are used by global dictionary entries.  This allows a
	 * thread to determine whether an entry exists in a particular global
	 * dictionary without having to lock the entire dictionary.
	 */
	cw_dch_t	*keyed_refs;
	/*
	 * If TRUE, the string in the key is statically allocated, and should
	 * not be deallocated during stiln destruction.
	 */
	cw_bool_t	is_static_name;
	/*
	 * name is *not* required to be NULL-terminated, so we keep track of the
	 * length.
	 */
	const cw_uint8_t *name;
	cw_uint32_t	len;
};

/*
 * Global name cache.
 */
struct cw_stilng_s {
	/*
	 * Hash of names (stiln --> stiloe_name).  This hash table keeps track
	 * of *all* name "values" in the virtual machine.  When a name object is
	 * created, it actually adds a reference to a stiloe_name and uses a
	 * pointer to that stiloe_name as a unique key.
	 *
	 * Note that each stilt maintains a cache of stiln's (via stilnt), so
	 * that under normal circumstances, all objects in a stilt refer to a
	 * single reference to the global stiloe_name.
	 */
	cw_dch_t	hash;
};

/*
 * Per-thread name cache.
 */
struct cw_stilnt_s {
	/*
	 * Hash of names (stiln --> stiloe_name).  This hash table keeps track
	 * of name "values" that are in existence within a particular local VM.
	 */
	cw_dch_t	hash;

	cw_stilng_t	*stilng;
};

/* stiln. */
void		stiln_new(cw_stiln_t *a_stiln);
void		stiln_delete(cw_stiln_t *a_stiln);

#define		stiln_val_get(a_stiln)	(a_stiln)->name
#define		stiln_len_get(a_stiln)	(a_stiln)->len

/* stilng. */
cw_bool_t	stilng_new(cw_stilng_t *a_stilng, cw_mem_t *a_mem);
void		stilng_delete(cw_stilng_t *a_stilng);

/* stilnt. */
cw_bool_t	stilnt_new(cw_stilnt_t *a_stilnt, cw_mem_t *a_mem,
    cw_stilng_t *a_stilng);
void		stilnt_delete(cw_stilnt_t *a_stilnt);
