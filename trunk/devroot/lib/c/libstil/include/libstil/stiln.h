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
typedef struct cw_stila_s cw_stila_t;
typedef struct cw_stilag_s cw_stilag_t;
typedef struct cw_stilat_s cw_stilat_t;
typedef struct cw_stiln_s cw_stiln_t;
typedef struct cw_stilt_s cw_stilt_t;
typedef struct cw_stilsc_s cw_stilsc_t;
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;

/*
 * Names are kept in a global hash table of stiloe_name's, and there is only one
 * stiloe_name per unique character string.  This allows the address of each
 * stiloe_name in the global hash table to be used as a unique key, so
 * regardless of the string length of a name, once it has been converted to a
 * stiloe_name pointer, name comparisons are a constant time operation.
 *
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
 * | | /------\    | |                     | | /-------\   | |
 * | | | name |    | |            /------------| stilo |   | |
 * | | \------/    | |           /         | | \-------/   | |
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
 * | | /------\    | |         /           | | /-------\   | |
 * | | | name |    |<---------/   /------------| stilo |   | |
 * | | \------/    | |           /         | | \-------/   | |
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
 * | | /------\    | |          |             |
 * | | | name |    |<-----------+--\          v
 * | | \------/    | |          |   \      /-----------------\
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
 * | | /------\    | |                     |                 |
 * | | | name |    | |                     | ............... |
 * | | \------/    |<---------\            | ............... |
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
 * Global name cache.
 */
struct cw_stilng_s {
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
