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
#define	_LIBSTASH_PEZZ_DBG
#endif

/* Pseudo-opaque type. */
typedef struct cw_pezz_s cw_pezz_t;
typedef struct cw_pezzi_s cw_pezzi_t;

struct cw_pezz_s {
	cw_mem_t	*mem;
	cw_bool_t	is_malloced;
#ifdef _LIBSTASH_DBG
	cw_uint32_t	magic;
#endif
#ifdef _LIBSTASH_PEZZ_DBG
	cw_dch_t	addr_hash;
#endif
	cw_mtx_t	lock;

	/* Size of one buffer, from the user's perspective. */
	cw_uint32_t	buffer_size;

	/*
	 * Size of one buffer internally, subject to machine alignment
	 * constraints.
	 */
	cw_uint32_t	aligned_size;

	/*
	 * Number of buffers in one block.  One block is (buffer_size *
	 * block_num_buffers) bytes.
	 */
	cw_uint32_t	block_num_buffers;

	/*
	 * Pointer to an array of base addresses for the memory blocks from
	 * which memory is allocated.
	 */
	void		**mem_blocks;

	/*
	 * Number of blocks allocated (number of elements in the
	 * mem_blocks[] array.
	 */
	cw_uint32_t	num_blocks;

	/* Stack of spare (unallocated) buffers. */
	qs_head(cw_pezzi_t) spares;
};

struct cw_pezzi_s {
	qs_elm(cw_pezzi_t)	link;
};

#ifdef _LIBSTASH_PEZZ_DBG
typedef struct {
	const char	*filename;
	cw_uint32_t	line_num;
}	cw_pezz_item_t;
#endif

cw_pezz_t *pezz_new(cw_pezz_t *a_pezz, cw_mem_t *a_mem, cw_uint32_t
    a_buffer_size, cw_uint32_t a_num_buffers);
void	pezz_delete(cw_pezz_t *a_pezz);
cw_uint32_t pezz_buffer_size_get(cw_pezz_t *a_pezz);
void	*pezz_get_e(cw_pezz_t *a_pezz, const char *a_filename, cw_uint32_t
    a_line_num);
void	pezz_put_e(cw_pezz_t *a_pezz, void *a_buffer, const char *a_filename,
    cw_uint32_t a_line_num);
void	pezz_dump(cw_pezz_t *a_pezz, const char *a_prefix);

/*
 * These macros are declared differently, depending on whether this is a debug
 * library, because consistently using arguments of NULL and 0 reduces the size
 * of the generated binary.  Since these arguments aren't used in the optimized
 * library anyway, this is a free (though perhaps small) memory savings.
 */
#ifdef _LIBSTASH_PEZZ_DBG
#define pezz_get(a_pezz)						\
	pezz_get_e((a_pezz), __FILE__, __LINE__)
#define pezz_put(a_pezz, a_buffer)					\
	pezz_put_e((a_pezz), (a_buffer), __FILE__, __LINE__)
#else
#define pezz_get(a_pezz)						\
	pezz_get_e((a_pezz), NULL, 0)
#define pezz_put(a_pezz, a_buffer)					\
	pezz_put_e((a_pezz), (a_buffer), NULL, 0)
#endif
