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

/* Size of buffer used for large extended objects. */
#define _CW_STIL_BUFC_SIZE 256

typedef struct cw_stil_s cw_stil_t;
typedef struct cw_stilnk_s cw_stilnk_t;

/* Defined in stilo.h to avoid a circular dependency. */
#if (0)
typedef struct cw_stiln_s cw_stiln_t;

#endif

struct cw_stil_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced;
	cw_mtx_t	lock;

	cw_pezz_t	stil_bufc_pezz;
	cw_pezz_t	chi_pezz;

	/* pezz from which stiln's are allocated for the names hash. */
	cw_pezz_t	stiln_pezz;

	/* pezz from which stilsc's are allocated for stacks. */
	cw_pezz_t	stilsc_pezz;

	/* pool from which dicto's are allocated for dicts. */
	cw_pool_t	dicto_pool; /* XXX Initialize! */

	/*
	 * Hash of names ((cw_stilnk_t *) string is hashed) to (cw_stiln_t ).
	 * *This hash table keeps track of *all* name "values" in the * virtual
	 * *machine.  When a name object is created, it actually adds * a
	 * *reference to a stiln and uses a pointer to that stiln as a * unique
	 * *key.  Note that each stilt maintains a cache of stiln's, * so that
	 * *under normal circumstances, all objects in a stilt share * a single
	 * *reference.
	 */
	cw_dch_t	stiln_dch;

	/*
	 * Hash of external references to the local VM, used for mark and sweep
	 * garbage collection.  Keys are (cw_stilo_t *); values are (cw_stiloe_t
	 * *). References need not be looked at directly, since the value field
	 * in the hash table is all we need to know.
	 */
	cw_dch_t	roots_dch;
};

/* Not opaque. */
typedef struct {
	cw_bufc_t	bufc;
	cw_uint8_t	buffer[_CW_STIL_BUFC_SIZE];
}	cw_stil_bufc_t;

struct cw_stilnk_s {
	/*
	 * If a name that is long enough to be stored as a buf is referenced,
	 * the value of the buf is copied to a contiguous string, so that the
	 * representation of the name is always a contiguous string.  In
	 * practice, it would be ridiculous to have such a long name, but we do
	 * the right thing in any case.
	 */
	const cw_uint8_t *name;
	cw_uint32_t	len;
};

struct cw_stiln_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_mtx_t	lock;

	/* Number of references to this object, including keyed references. */
	cw_uint32_t	ref_count;
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

	/* Key.  The value is merely a pointer to this stiln. */
	cw_stilnk_t	key;
};

/* stil. */
cw_stil_t	*stil_new(cw_stil_t *a_stil);
void		stil_delete(cw_stil_t *a_stil);

cw_stil_bufc_t	*stil_get_stil_bufc(cw_stil_t *a_stil);

#define stil_get_chi_pezz(a_stil) (&(a_stil)->chi_pezz)
#define stil_get_stilsc_pezz(a_stil) (&(a_stil)->stilsc_pezz)
#define stil_get_dicto_pool(a_stil) (&(a_stil)->dicto_pool)

const cw_stiln_t *stil_stiln_ref(cw_stil_t *a_stil, const cw_uint8_t *a_name,
    cw_uint32_t a_len, cw_bool_t a_force, cw_bool_t a_is_static, const void
    *a_key, const void *a_data);

void		stil_stiln_unref(cw_stil_t *a_stil, const cw_stiln_t *a_stiln,
    const void *a_key);

/* stiln. */
const cw_stilnk_t *stiln_get_stilnk(const cw_stiln_t *a_stiln);

/* stilnk. */
void		stilnk_init(cw_stilnk_t *a_stilnk, const cw_uint8_t *a_name,
    cw_uint32_t a_len);

void		stilnk_copy(cw_stilnk_t *a_to, const cw_stilnk_t *a_from);

const cw_uint8_t *stilnk_get_val(cw_stilnk_t *a_stilnk);

cw_uint32_t	stilnk_get_len(cw_stilnk_t *a_stilnk);
