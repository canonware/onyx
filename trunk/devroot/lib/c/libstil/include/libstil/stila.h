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

/*
 * Book keeping for GC.
 */
struct cw_stila_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t	magic;
#endif
	/*
	 * Keys are pointers to stiloe's.  Values are unused (NULL).
	 */
	cw_dch_t	seq_set;
};

/*
 * Global memory allocator.
 */
struct cw_stilag_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t	magic;
#endif
	cw_mtx_t	lock;

	/*
	 * Head of list of stilat's for all stilt's.  This is needed in order to
	 * suspend threads and recurse when doing GC.
	 */
	ql_head(cw_stilat_t)	head;

	/* Allocator. */
	cw_mem_t	mem;

	/* Various pools. */
	cw_pool_t	chi_pool;
	cw_pool_t	stilsc_pool;
	cw_pool_t	dicto_pool;

	cw_stila_t	stila;
};

/*
 * Per-thread memory allocator.
 */
struct cw_stilat_s {
#ifdef _LIBSTIL_DBG
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
void		stilag_new(cw_stilag_t *a_stilag);
void		stilag_delete(cw_stilag_t *a_stilag);

void		*stilag_malloc_e(cw_stilag_t *a_stilag, size_t a_size, const
    char *a_filename, cw_uint32_t a_line_num);
void		stilag_free_e(cw_stilag_t *a_stilag, void *a_ptr, const char
    *a_filename, cw_uint32_t a_line_num);

void		*stilag_gc_malloc_e(cw_stilag_t *a_stilag, size_t a_size, const
    char *a_filename, cw_uint32_t a_line_num);
void		stilag_gc_register(cw_stilag_t *a_stilag, cw_stilt_t *a_stilt,
    cw_stiloe_t *a_stiloe);

#define		stilag_mem_get(a_stilag)				\
	&(a_stilag)->mem
#define		stilag_chi_pool_get(a_stilag)				\
	&(a_stilag)->chi_pool
#define		stilag_stilsc_pool_get(a_stilag)			\
	&(a_stilag)->stilsc_pool
#define		stilag_dicto_pool_get(a_stilag)				\
	&(a_stilag)->dicto_pool

#ifdef _LIBSTIL_DBG
#define		stilag_malloc(a_stilag, a_size)				\
	stilag_malloc_e((a_stilag), (a_size), __FILE__, __LINE__)
#define		stilag_free(a_stilag, a_ptr)				\
	stilag_free_e((a_stilag), (a_ptr), __FILE__, __LINE__)
#define		stilag_gc_malloc(a_stilag, a_size)			\
	stilag_gc_malloc_e((a_stilag), (a_size), __FILE__, __LINE__)
#else
#define		stilag_malloc(a_stilag, a_size)				\
	stilag_malloc_e((a_stilag), (a_size), NULL, 0)
#define		stilag_free(a_stilag, a_ptr)				\
	stilag_free_e((a_stilag), (a_ptr), NULL, 0)
#define		stilag_gc_malloc(a_stilag, a_size)			\
	stilag_gc_malloc_e((a_stilag), (a_size), NULL, 0)
#endif

#define		stilag_chi_get(a_stilag)				\
	(cw_chi_t *)pool_get(&(a_stilag)->chi_pool)
#define		stilag_chi_put(a_stilag, a_chi)				\
	pool_put(&(a_stilag)->chi_pool, (a_chi))

#define		stilag_stilsc_get(a_stilag)				\
	(cw_stilsc_t *)pool_get(&(a_stilag)->stilsc_pool)
#define		stilag_stilsc_put(a_stilag, a_stilsc)		\
	pool_put(&(a_stilag)->stilsc_pool, (a_stilsc))

#define		stilag_dicto_get(a_stilag)				\
	(cw_stiloe_dicto_t *)pool_get(&(a_stilag)->dicto_pool)
#define		stilag_dicto_put(a_stilag, a_dicto)			\
	pool_put(&(a_stilag)->dicto_pool, (a_dicto))

/* stilat. */
void		stilat_new(cw_stilat_t *a_stilat, cw_stilt_t *a_stilt,
    cw_stilag_t *a_stilag);
void		stilat_delete(cw_stilat_t *a_stilat);

cw_chi_t	*stilat_chi_get_e(cw_stilat_t *a_stilat, const char
    *a_filename, cw_uint32_t a_line_num);
cw_stilsc_t	*stilat_stilsc_get_e(cw_stilat_t *a_stilat, const char
    *a_filename, cw_uint32_t a_line_num);
cw_stiloe_dicto_t *stilat_dicto_get_e(cw_stilat_t *a_stilat, const char
    *a_filename, cw_uint32_t a_line_num);

void		*stilat_malloc_e(cw_stilat_t *a_stilat, size_t a_size, const
    char *a_filename, cw_uint32_t a_line_num);
void		*stilat_gc_malloc_e(cw_stilat_t *a_stilat, size_t a_size, const
    char *a_filename, cw_uint32_t a_line_num);
void		stilat_gc_register(cw_stilat_t *a_stilat, cw_stiloe_t
    *a_stiloe);
void		stilat_free_e(cw_stilat_t *a_stilat, void *a_ptr, const char
    *a_filename, cw_uint32_t a_line_num);

#define		stilat_currentglobal(a_stilat)				\
	(a_stilat)->global
#define		stilat_setglobal(a_stilat, a_global)			\
	(a_stilat)->global = (a_global)
#define		stilat_mem_get(a_stilat)				\
	stilag_mem_get((a_stilat)->stilag)
#define		stilat_chi_pool_get(a_stilat)				\
	stilag_chi_pool_get((a_stilat)->stilag)
#define		stilat_stilsc_pool_get(a_stilat)			\
	stilag_stilsc_pool_get((a_stilat)->stilag)
#define		stilat_dicto_pool_get(a_stilat)				\
	stilag_dicto_pool_get((a_stilat)->stilag)

#ifdef _LIBSTIL_DBG
#define		stilat_chi_get(a_stilat)				\
	stilat_chi_get_e((a_stilat), __FILE__, __LINE__)
#define		stilat_stilsc_get(a_stilat)				\
	stilat_stilsc_get_e((a_stilat), __FILE__, __LINE__)
#define		stilat_dicto_get(a_stilat)				\
	stilat_dicto_get_e((a_stilat), __FILE__, __LINE__)
#define		stilat_malloc(a_stilat, a_size)				\
	stilat_malloc_e((a_stilat), (a_size), __FILE__, __LINE__)
#define		stilat_gc_malloc(a_stilat, a_size)			\
	stilat_gc_malloc_e((a_stilat), (a_size), __FILE__, __LINE__)
#define		stilat_free(a_stilat, a_ptr)				\
	stilat_free_e((a_stilat), (a_ptr), __FILE__, __LINE__)
#else
#define		stilat_chi_get(a_stilat)				\
	stilat_chi_get_e((a_stilat), NULL, 0)
#define		stilat_stilsc_get(a_stilat)				\
	stilat_stilsc_get_e((a_stilat), NULL, 0)
#define		stilat_dicto_get(a_stilat)				\
	stilat_dicto_get_e((a_stilat), NULL, 0)
#define		stilat_malloc(a_stilat, a_size)				\
	stilat_malloc_e((a_stilat), (a_size), NULL, 0)
#define		stilat_gc_malloc(a_stilat, a_size)			\
	stilat_gc_malloc_e((a_stilat), (a_size), NULL, 0)
#define		stilat_free(a_stilat, a_ptr)				\
	stilat_free_e((a_stilat), (a_ptr), NULL, 0)
#endif

#define		stilat_chi_put(a_stilat, a_chi)				\
	pool_put(&(a_stilat)->stilag->chi_pool, (a_chi))
#define		stilat_stilsc_put(a_stilat, a_stilsc)			\
	pool_put(&(a_stilat)->stilag->stilsc_pool, (a_stilsc))
#define		stilat_dicto_put(a_stilat, a_dicto)			\
	pool_put(&(a_stilat)->stilag->dicto_pool, (a_dicto))
