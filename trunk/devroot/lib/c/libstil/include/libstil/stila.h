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

struct cw_stila_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif
	/* Allocator. */
	cw_mem_t	mem;

	/* If TRUE, this is the top level stila. */
	cw_bool_t	global;

	/*
	 * Linkage for a list of stila's for all threads.  The global stila
	 * needs this in order to suspend threads and recurse when doing GC.
	 */
	union {
		struct {
			qq_head(cw_stila_t)	head;
		}	g;
		struct {
			qq_entry(cw_stila_t)	link;
			cw_stila_t		*global;
			cw_stilt_t		*stilt;
		}	t;
	}	l;

	/*
	 * Keys are pointers to stiloe's.  Values are unused (NULL).
	 */
	cw_dch_t	seq_set;

#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	/*
	 * Allocations that are not (yet?) part of the sequence set.
	 */
	cw_dch_t	seq_complement;
#endif

	/* Pointers to various pools. */
	cw_pool_t	*stil_bufc_pool;
	cw_pool_t	*chi_pool;
	cw_pool_t	*stiln_pool;
	cw_pool_t	*stilsc_pool;
	cw_pool_t	*dicto_pool;
};

cw_bool_t	stila_gnew(cw_stila_t *a_stila, cw_pool_t *a_stil_bufc_pool,
    cw_pool_t *a_chi_pool, cw_pool_t *a_stiln_pool, cw_pool_t *a_stilsc_pool,
    cw_pool_t *a_dicto_pool);
void		stila_tnew(cw_stila_t *a_stila, cw_stila_t *a_other, cw_stilt_t
    *a_stilt);
void		stila_delete(cw_stila_t *a_stila);

void		*stila_malloc(cw_stila_t *a_stila, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);
void		stila_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe);
void		stila_free(cw_stila_t *a_stila, void *a_ptr, const char
    *a_filename, cw_uint32_t a_line_num);

#define stila_mem_get(a_stila)						\
	(&(a_stila)->mem)

#define stila_gget(a_stila)						\
	(((a_stila)->global) ? (a_stila) : ((a_stila)->l.t.global))

#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
#define _cw_stila_malloc(a_stila, a_size)				\
	stila_malloc((a_stila), (a_size), __FILE__, __LINE__)
#define _cw_stila_free(a_stila, a_ptr)					\
	stila_free((a_stila), (a_ptr), __FILE__, __LINE__)
#else
#define _cw_stila_malloc(a_stila, a_size)				\
	stila_malloc((a_stila), (a_size), NULL, 0)
#define _cw_stila_free(a_stila, a_ptr)					\
	stila_free((a_stila), (a_ptr), NULL, 0)
#endif

#define _cw_stila_stil_bufc_get(a_stila)				\
	_cw_pool_get((a_stila)->stil_bufc_pool)
#define _cw_stila_stil_bufc_put(a_stila, a_stil_bufc)			\
	_cw_pool_put((a_stila)->stil_bufc_pool, (a_stil_bufc))

#define _cw_stila_chi_get(a_stila)					\
	(cw_chi_t *)_cw_pool_get((a_stila)->chi_pool)
#define _cw_stila_chi_put(a_stila, a_chi)				\
	_cw_pool_put((a_stila)->chi_pool, (a_chi))

#define _cw_stila_stiln_get(a_stila)					\
	_cw_pool_get((a_stila)->stiln_pool)
#define _cw_stila_stiln_put(a_stila, a_stiln)				\
	_cw_pool_put((a_stila)->stiln_pool, (a_stiln))

#define _cw_stila_stilsc_get(a_stila)					\
	_cw_pool_get((a_stila)->stilsc_pool)
#define _cw_stila_stilsc_put(a_stila, a_stilsc)				\
	_cw_pool_put((a_stila)->stilsc_pool, (a_stilsc))

#define _cw_stila_dicto_get(a_stila)					\
	(cw_stiloe_dicto_t *)_cw_pool_get((a_stila)->dicto_pool)
#define _cw_stila_dicto_put(a_stila, a_dicto)				\
	_cw_pool_put((a_stila)->dicto_pool, (a_dicto))
