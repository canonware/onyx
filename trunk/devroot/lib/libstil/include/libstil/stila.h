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
	/*
	 * Keys are pointers to stiloe's.  Values are unused (NULL).
	 */
	cw_dch_t	seq_set;
};

struct cw_stilag_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif
	cw_mtx_t	lock;

	/*
	 * Head of list of stila's for all stilt's.  This is needed in order to
	 * supsend threads and recurse when doing GC.
	 */
	ql_head(cw_stilat_t)	head;

	/* Allocator. */
	cw_mem_t	mem;

	/* Various pools. */
	cw_pool_t	stil_bufc_pool;
	cw_pool_t	chi_pool;
	cw_pool_t	stiln_pool;
	cw_pool_t	stilsc_pool;
	cw_pool_t	dicto_pool;

	cw_stila_t	stila;
};

struct cw_stilat_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif
	/* Linkage for the list of all stilat's. */
	ql_elm(cw_stilat_t) link;

	/*
	 * TRUE  : Global allocation mode.
	 * FALSE : Local allocation mode.
	 */
	cw_bool_t	global;

	/* Pointer to the stilt associated with this stilat. */
	cw_stilt_t	*stilt;

	/* Pointer to the master of all stilat's. */
	cw_stilag_t	*stilag;

	cw_stila_t	stila;
};

/* stilag. */
cw_bool_t	stilag_new(cw_stilag_t *a_stilag);
void		stilag_delete(cw_stilag_t *a_stilag);

void		*stilag_malloc(cw_stilag_t *a_stilag, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);
void		stilag_free(cw_stilag_t *a_stilag, void *a_ptr, const char
    *a_filename, cw_uint32_t a_line_num);

void		*stilag_gc_malloc(cw_stilag_t *a_stilag, size_t a_size, const
    char *a_filename, cw_uint32_t a_line_num);
cw_bool_t	stilag_gc_register(cw_stilag_t *a_stilag, cw_stilt_t *a_stilt,
    cw_stiloe_t *a_stiloe);

#define		stilag_mem_get(a_stilag)				\
	&(a_stilag)->mem
#define		stilag_stil_bufc_pool_get(a_stilag)			\
	&(a_stilag)->stil_bufc_pool
#define		stilag_chi_pool_get(a_stilag)				\
	&(a_stilag)->chi_pool
#define		stilag_stiln_pool_get(a_stilag)				\
	&(a_stilag)->stiln_pool
#define		stilag_stilsc_pool_get(a_stilag)			\
	&(a_stilag)->stilsc_pool
#define		stilag_dicto_pool_get(a_stilag)				\
	&(a_stilag)->dicto_pool

#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
#define		_cw_stilag_malloc(a_stilag, a_size)			\
	stilag_malloc((a_stilag), (a_size), __FILE__, __LINE__)
#define		_cw_stilag_free(a_stilag, a_ptr)			\
	stilag_free((a_stilag), (a_ptr), __FILE__, __LINE__)
#define		_cw_stilag_gc_malloc(a_stilag, a_size)			\
	stilag_gc_malloc((a_stilag), (a_size), __FILE__, __LINE__)
#else
#define		_cw_stilag_malloc(a_stilag, a_size)			\
	stilag_malloc((a_stilag), (a_size), NULL, 0)
#define		_cw_stilag_free(a_stilag, a_ptr)			\
	stilag_free((a_stilag), (a_ptr), NULL, 0)
#define		_cw_stilag_gc_malloc(a_stilag, a_size)			\
	stilag_gc_malloc((a_stilag), (a_size), NULL, 0)
#endif

#define		_cw_stilag_stil_bufc_get(a_stilag)			\
	(cw_stil_bufc_t *)_cw_pool_get(&(a_stilag)->stil_bufc_pool)
#define		_cw_stilag_stil_bufc_put(a_stilag, a_stil_bufc)		\
	_cw_pool_put(&(a_stilag)->stil_bufc_pool, (a_stil_bufc))

#define		_cw_stilag_chi_get(a_stilag)				\
	(cw_chi_t *)_cw_pool_get(&(a_stilag)->chi_pool)
#define		_cw_stilag_chi_put(a_stilag, a_chi)			\
	_cw_pool_put(&(a_stilag)->chi_pool, (a_chi))

#define		_cw_stilag_stiln_get(a_stilag)				\
	(cw_stiln_t *)_cw_pool_get(&(a_stilag)->stiln_pool)
#define		_cw_stilag_stiln_put(a_stilag, a_stiln)			\
	_cw_pool_put(&(a_stilag)->stiln_pool, (a_stiln))

#define		_cw_stilag_stilsc_get(a_stilag)				\
	(cw_stilsc_t *)_cw_pool_get(&(a_stilag)->stilsc_pool)
#define		_cw_stilag_stilsc_put(a_stilag, a_stilsc)		\
	_cw_pool_put(&(a_stilag)->stilsc_pool, (a_stilsc))

#define		_cw_stilag_dicto_get(a_stilag)				\
	(cw_stiloe_dicto_t *)_cw_pool_get(&(a_stilag)->dicto_pool)
#define		_cw_stilag_dicto_put(a_stilag, a_dicto)			\
	_cw_pool_put(&(a_stilag)->dicto_pool, (a_dicto))

/* stilat. */
cw_bool_t	stilat_new(cw_stilat_t *a_stilat, cw_stilt_t *a_stilt,
    cw_stilag_t *a_stilag);
void		stilat_delete(cw_stilat_t *a_stilat);

cw_stil_bufc_t	*stilat_stil_bufc_get(cw_stilat_t *a_stilat, const char
    *a_filename, cw_uint32_t a_line_num);
cw_chi_t	*stilat_chi_get(cw_stilat_t *a_stilat, const char
    *a_filename, cw_uint32_t a_line_num);
cw_stiln_t	*stilat_stiln_get(cw_stilat_t *a_stilat, const char
    *a_filename, cw_uint32_t a_line_num);
cw_stilsc_t	*stilat_stilsc_get(cw_stilat_t *a_stilat, const char
    *a_filename, cw_uint32_t a_line_num);
cw_stiloe_dicto_t *stilat_dicto_get(cw_stilat_t *a_stilat, const char
    *a_filename, cw_uint32_t a_line_num);

void		*stilat_malloc(cw_stilat_t *a_stilat, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);
void		*stilat_gc_malloc(cw_stilat_t *a_stilat, size_t a_size, const
    char *a_filename, cw_uint32_t a_line_num);
cw_bool_t	stilat_gc_register(cw_stilat_t *a_stilat, cw_stiloe_t
    *a_stiloe);
void		stilat_free(cw_stilat_t *a_stilat, void *a_ptr, const char
    *a_filename, cw_uint32_t a_line_num);

#define		stilat_mem_get(a_stilat)				\
	stilag_mem_get((a_stilat)->stilag)
#define		stilat_stil_bufc_pool_get(a_stilat)			\
	stilag_stil_bufc_pool_get((a_stilat)->stilag)
#define		stilat_chi_pool_get(a_stilat)				\
	stilag_chi_pool_get((a_stilat)->stilag)
#define		stilat_stiln_pool_get(a_stilat)				\
	stilag_stiln_pool_get((a_stilat)->stilag)
#define		stilat_stilsc_pool_get(a_stilat)			\
	stilag_stilsc_pool_get((a_stilat)->stilag)
#define		stilat_dicto_pool_get(a_stilat)				\
	stilag_dicto_pool_get((a_stilat)->stilag)

#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
#define		_cw_stilat_stil_bufc_get(a_stilat)			\
	stilat_stil_bufc_get((a_stilat), __FILE__, __LINE__)
#define		_cw_stilat_chi_get(a_stilat)				\
	stilat_chi_get((a_stilat), __FILE__, __LINE__)
#define		_cw_stilat_stiln_get(a_stilat)				\
	stilat_stiln_get((a_stilat), __FILE__, __LINE__)
#define		_cw_stilat_stilsc_get(a_stilat)				\
	stilat_stilsc_get((a_stilat), __FILE__, __LINE__)
#define		_cw_stilat_dicto_get(a_stilat)				\
	stilat_dicto_get((a_stilat), __FILE__, __LINE__)
#define		_cw_stilat_malloc(a_stilat, a_size)			\
	stilat_malloc((a_stilat), (a_size), __FILE__, __LINE__)
#define		_cw_stilat_gc_malloc(a_stilat, a_size)			\
	stilat_gc_malloc((a_stilat), (a_size), __FILE__, __LINE__)
#define		_cw_stilat_free(a_stilat, a_ptr)			\
	stilat_free((a_stilat), (a_ptr), __FILE__, __LINE__)
#else
#define		_cw_stilat_stil_bufc_get(a_stilat)			\
	stilat_stil_bufc_get((a_stilat), NULL, 0)
#define		_cw_stilat_chi_get(a_stilat)				\
	stilat_chi_get((a_stilat), NULL, 0)
#define		_cw_stilat_stiln_get(a_stilat)				\
	stilat_stiln_get((a_stilat), NULL, 0)
#define		_cw_stilat_stilsc_get(a_stilat)				\
	stilat_stilsc_get((a_stilat), NULL, 0)
#define		_cw_stilat_dicto_get(a_stilat)				\
	stilat_dicto_get((a_stilat), NULL, 0)
#define		_cw_stilat_malloc(a_stilat, a_size)			\
	stilat_malloc((a_stilat), (a_size), NULL, 0)
#define		_cw_stilat_gc_malloc(a_stilat, a_size)			\
	stilat_gc_malloc((a_stilat), (a_size), NULL, 0)
#define		_cw_stilat_free(a_stilat, a_ptr)			\
	stilat_free((a_stilat), (a_ptr), NULL, 0)
#endif

#define		_cw_stilat_stil_bufc_put(a_stilat, a_stil_bufc)		\
	_cw_pool_put(&(a_stilat)->stilag->stil_bufc_pool, (a_stil_bufc))
#define		_cw_stilat_chi_put(a_stilat, a_chi)			\
	_cw_pool_put(&(a_stilat)->stilag->chi_pool, (a_chi))
#define		_cw_stilat_stiln_put(a_stilat, a_stiln)			\
	_cw_pool_put(&(a_stilat)->stilag->stiln_pool, (a_stiln))
#define		_cw_stilat_stilsc_put(a_stilat, a_stilsc)		\
	_cw_pool_put(&(a_stilat)->stilag->stilsc_pool, (a_stilsc))
#define		_cw_stilat_dicto_put(a_stilat, a_dicto)			\
	_cw_pool_put(&(a_stilat)->stilag->dicto_pool, (a_dicto))
