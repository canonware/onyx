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
 * The following diagram shows the various name-related object relationships
 * that are possible.  Locally allocated name objects use a thread-specific
 * cache.  Globally allocated name objects refer directly to the global table.
 *
 * The reason for the thread-specific cache is that all operations on the global
 * table require locking.  By caching references to the global table on a
 * per-thread basis, we amortize the cost of locking the global table.
 *
 * /------\                                /-------\
 * | stil |                                | stilt |
 * \------/                                \-------/
 *    |                                        |
 *    |                                        |
 *    |                                        |
 *    v                                        v
 * /-----------------\                     /-----------------\
 * | stilng          |                     | stilnt          |
 * |                 |                     |                 |
 * | /-------------\ |                     | /-------------\ |
 * | | stiloe_name | |                     | | stiloe_name | |
 * | |             | |                     | |             | |
 * | | /-------\   | |                     | | /-------\   | |
 * | | | stiln |   | |            /------------| stilo |   | |
 * | | \-------/   | |           /         | | \-------/   | |
 * | |             | |          /          | |             | |
 * | \-------------/ |          |          | \-------------/ |
 * |                 |          |          |                 |
 * | ............... |          |          | ............... |
 * | ............... |          |          | ............... |
 * | ............... |          |          | ............... |
 * |                 |          |          |                 |
 * | /-------------\ |          |          | /-------------\ |
 * | | stiloe_name | |          |          | | stiloe_name | |
 * | |             | |          /          | |             | |
 * | | /-------\   | |         /           | | /-------\   | |
 * | | | stiln |   |<---------/   /------------| stilo |   | |
 * | | \-------/   | |           /         | | \-------/   | |
 * | |             | |          /          | |             | |
 * | \-------------/ |          |          | \-------------/ |
 * |                 |          |          |                 |
 * | ............... |          |          \-----------------/
 * | ............... |          |
 * | ............... |          |          /-------\
 * |                 |          |          | stilt |
 * | /-------------\ |          |          \-------/
 * | | stiloe_name | |          |             |
 * | |             | |          |             |
 * | | /-------\   | |          |             |
 * | | | stiln |   |<-----------+--\          v
 * | | \-------/   | |          |   \      /-----------------\
 * | |             | |          |    \     | stilnt          |
 * | \-------------/ |          |    |     |                 |
 * |                 |          |    |     | /-------------\ |
 * | ............... |          |    |     | | stiloe_name | |
 * | ............... |          |    \     | |             | |
 * | ............... |          |     \    | | /-------\   | |
 * |                 |          |      \-------| stilo |   | |
 * | /-------------\ |          /          | | \-------/   | |
 * | | stiloe_name | |         /           | |             | |
 * | |             |<---------/            | \-------------/ |
 * | | /-------\   | |                     |                 |
 * | | | stiln |   | |                     | ............... |
 * | | \-------/   |<---------\            | ............... |
 * | |             | |         \           | ............... |
 * | \-------------/ |          \          |                 |
 * |       ^         |          |          | /-------------\ |
 * \-------|---------/          |          | | stiloe_name | |
 *         |                    \          | |             | |
 *         |                     \         | | /-------\   | |
 *         |                      \------------| stilo |   | |
 *         |                               | | \-------/   | |
 * /-------------------\                   | |             | |
 * | stilo (global VM) |                   | \-------------/ |
 * | (possibly keyed)  |                   |        ^        |
 * \-------------------/                   \--------|--------/
 *                                                  |
 *                                                  |
 *                                                  |
 *                                                  |
 *                                         /------------------\
 *                                         | stilo (local VM) |
 *                                         \------------------/
 */

/*
 * Name.  stiln's are kept in a global hash table, and there is only one stiln
 * per unique character string.  This allows the address of each stiloe_name
 * container object in the global hash table to be used as a unique key, so
 * regardless of the string length of a name, once it has been converted to a
 * stiloe_name pointer, name comparisons are a constant time operation.
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
	/* Protects hash. */
	cw_mtx_t	lock;
	/*
	 * Hash of names (key: {name, len}, value: (stiloe_name *)).  This hash
	 * table keeps track of *all* name "values" in the virtual machine.
	 * When a name object is created, it actually adds a reference to a
	 * stiloe_name in this hash and uses a pointer to that stiloe_name as a
	 * unique key.
	 *
	 * Note that each stilt maintains a cache of stiloe_name's (via stilnt),
	 * so that under normal circumstances, all locally allocated objects in
	 * a stilt refer to a single reference to the global stiloe_name.
	 */
	cw_dch_t	hash;
};

/*
 * Per-thread name cache.
 */
struct cw_stilnt_s {
	/*
	 * Hash of names (key: {name, len}, value: (stiloe_name *)).  This hash
	 * table keeps track of name "values" that are in existence within a
	 * particular local VM.
	 */
	cw_dch_t	hash;
};
