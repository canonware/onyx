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

struct cw_mem_s {
	cw_mem_t	*mem;
	cw_bool_t	is_malloced;

	cw_mtx_t	lock;

#ifdef _LIBSTASH_DBG
#define _CW_MEM_TABLE_SIZE 1024		/* Number of slots in hash table. */
	cw_ch_t		*addr_hash;
#endif

        const void	*handler_data;
};

cw_mem_t *mem_new(cw_mem_t *a_mem, cw_mem_t *a_internal);
void	mem_delete(cw_mem_t *a_mem);
void	*mem_malloc_e(cw_mem_t *a_mem, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num);
void	*mem_calloc_e(cw_mem_t *a_mem, size_t a_number, size_t a_size, const
    char *a_filename, cw_uint32_t a_line_num);
void	*mem_realloc_e(cw_mem_t *a_mem, void *a_ptr, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);
void	mem_free_e(cw_mem_t *a_mem, void *a_ptr, const char *a_filename,
    cw_uint32_t a_line_num);

/*
 * These macros are declared differently, depending on whether this is a debug
 * library, because consistently using arguments of NULL and 0 reduces the size
 * of the generated binary.  Since these arguments aren't used in the optimized
 * library anyway, this is a free (though perhaps small) memory savings.
 */
#ifdef _LIBSTASH_DBG
#define mem_malloc(a_mem, a_size)					\
	mem_malloc_e((a_mem), (a_size), __FILE__, __LINE__)
#define mem_calloc(a_mem, a_number, a_size)				\
	mem_calloc_e((a_mem), (a_number), (a_size), __FILE__, __LINE__)
#define mem_realloc(a_mem, a_ptr, a_size)				\
	mem_realloc_e((a_mem), (a_ptr), (a_size), __FILE__, __LINE__)
#define	mem_free(a_mem, a_ptr)						\
	mem_free_e((a_mem), (a_ptr), __FILE__, __LINE__)

#define _cw_malloc(a_size)						\
	mem_malloc_e(cw_g_mem, (a_size), __FILE__, __LINE__)
#define _cw_calloc(a_number, a_size)					\
	mem_calloc_e(cw_g_mem, (a_number), (a_size), __FILE__, __LINE__)
#define _cw_realloc(a_ptr, a_size)					\
	mem_realloc_e(cw_g_mem, (a_ptr), (a_size), __FILE__, __LINE__)
#define	_cw_free(a_ptr)							\
	mem_free_e(cw_g_mem, (a_ptr), __FILE__, __LINE__)

#else
#define mem_malloc(a_mem, a_size)					\
	mem_malloc_e((a_mem), (a_size), NULL, 0)
#define mem_calloc(a_mem, a_number, a_size)				\
	mem_calloc_e((a_mem), (a_number), (a_size), NULL, 0)
#define mem_realloc(a_mem, a_ptr, a_size)				\
	mem_realloc_e((a_mem), (a_ptr), (a_size), NULL, 0)
#define	mem_free(a_mem, a_ptr)						\
	mem_free_e((a_mem), (a_ptr), NULL, 0)

#define _cw_malloc(a_size)						\
	mem_malloc_e(cw_g_mem, (a_size), NULL, 0)
#define _cw_calloc(a_number, a_size)					\
	mem_calloc_e(cw_g_mem, (a_number), (a_size), NULL, 0)
#define _cw_realloc(a_ptr, a_size)					\
	mem_realloc_e(cw_g_mem, (a_ptr), (a_size), NULL, 0)
#define	_cw_free(a_ptr)							\
	mem_free_e(cw_g_mem, (a_ptr), NULL, 0)

#endif
