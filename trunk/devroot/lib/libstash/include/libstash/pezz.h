/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_pezz_s cw_pezz_t;

struct cw_pezz_s {
	cw_bool_t is_malloced;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t magic;
	cw_oh_t addr_hash;
#endif
	cw_mtx_t lock;

	/* Size of one buffer, from the user's perspective. */
	cw_uint32_t buffer_size;

	/*
	 * Number of buffers in one block.  One block is (buffer_size *
	 * block_num_buffers) bytes.
	 */
	cw_uint32_t block_num_buffers;

	/*
	 * Pointer to an array of base addresses for the memory blocks from
	 * which memory is allocated.
	 */
	void  **mem_blocks;

	/*
	 * Pointer to an array of base addresses for the memory blocks that
	 * are used for ring structures.
	 */
	cw_ring_t **ring_blocks;

	/*
	 * Number of blocks allocated (number of elements in the
	 * mem_blocks[] and ring_blocks[] arrays.
	 */
	cw_uint32_t num_blocks;

	/* Ring seam for spare (unallocated) buffers. */
	cw_ring_t *spare_buffers;

	/*
	 * Ring seam for spare rings.  This ring has no associated data, and
	 * is merely a cache of ring structures to be used for insertion
	 * into the spare_buffers ring.
	 */
	cw_ring_t *spare_rings;
};

#ifdef _LIBSTASH_DBG
typedef struct {
	const char *filename;
	cw_uint32_t line_num;
}       cw_pezz_item_t;

#endif

cw_pezz_t *pezz_new(cw_pezz_t *a_pezz, cw_uint32_t a_buffer_size, cw_uint32_t
    a_num_buffers);

void    pezz_delete(cw_pezz_t *a_pezz);

cw_uint32_t pezz_get_buffer_size(cw_pezz_t *a_pezz);

void   *pezz_get(cw_pezz_t *a_pezz);
void   *pezz_get_e(cw_pezz_t *a_pezz, const char *a_filename, cw_uint32_t
    a_line_num);

#define _cw_pezz_get(a_pezz) pezz_get_e((a_pezz), __FILE__, __LINE__)

void    pezz_put(void *a_pezz, void *a_buffer);
void    pezz_put_e(cw_pezz_t *a_pezz, void *a_buffer, const char *a_filename,
    cw_uint32_t a_line_num);

#define _cw_pezz_put(a_pezz, a_buffer)					\
	pezz_put_e((a_pezz), (a_buffer), __FILE__, __LINE__)

void    pezz_dump(cw_pezz_t *a_pezz, const char *a_prefix);
