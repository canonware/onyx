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

#ifdef _LIBSTASH_DBG
/* Track allocations and report leaks. */
#define	_LIBSTASH_POOL_ERROR
/* Report all allocations.  Requires _LIBSTASH_POOL_ERROR. */
/*  #define	_LIBSTASH_POOL_VERBOSE */
#endif

/* Pseudo-opaque type. */
typedef struct cw_pool_s cw_pool_t;
typedef struct cw_pool_spare_s cw_pool_spare_t;

struct cw_pool_s {
	cw_bool_t	is_malloced;
#ifdef _LIBSTASH_DBG
	cw_uint32_t	magic;
#endif
#ifdef _LIBSTASH_POOL_ERROR
	cw_dch_t	addr_hash;
#endif
	/* Allocator. */
	cw_mem_t	*mem;

	cw_mtx_t	lock;

	/* Size of one buffer, from the user's perspective. */
	cw_uint32_t	buffer_size;

	qs_head(cw_pool_spare_t) spares;
};

struct cw_pool_spare_s {
	qs_elm(cw_pool_spare_t) link;
};

#ifdef _LIBSTASH_POOL_ERROR
typedef struct {
	const char	*filename;
	cw_uint32_t	line_num;
}	cw_pool_item_t;
#endif

cw_pool_t	*pool_new(cw_pool_t *a_pool, cw_mem_t *a_mem, cw_uint32_t
    a_buffer_size);
void		pool_delete(cw_pool_t *a_pool);
cw_uint32_t	pool_buffer_size_get(cw_pool_t *a_pool);
void		pool_drain(cw_pool_t *a_pool);
void		*pool_get_e(cw_pool_t *a_pool, const char *a_filename,
    cw_uint32_t a_line_num);
void		pool_put_e(cw_pool_t *a_pool, void *a_buffer, const char
    *a_filename, cw_uint32_t a_line_num);
void		pool_dump(cw_pool_t *a_pool, const char *a_prefix);

/*
 * These macros are declared differently, depending on whether this is a debug
 * library, because consistently using arguments of NULL and 0 reduces the size
 * of the generated binary.  Since these arguments aren't used in the optimized
 * library anyway, this is a free (though perhaps small) memory savings.
 */
#ifdef _LIBSTASH_POOL_ERROR
#define pool_get(a_pool)						\
	pool_get_e((a_pool), __FILE__, __LINE__)
#define pool_put(a_pool, a_buffer)					\
	pool_put_e((a_pool), (a_buffer), __FILE__, __LINE__)
#else
#define pool_get(a_pool)						\
	pool_get_e((a_pool), NULL, 0)
#define pool_put(a_pool, a_buffer)					\
	pool_put_e((a_pool), (a_buffer), NULL, 0)
#endif
