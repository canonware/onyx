/****************************************************************************
 *
 * <Copyright = toshok>
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_arena_s cw_arena_t;

struct cw_arena_s {
	cw_bool_t is_malloced;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t magic;
#endif
	cw_bool_t is_thread_safe;
	cw_mtx_t lock;

	/*
	 * If we notice we're full, this is set to true.  Then all
	 * allocation is short circuited and returns FALSE.
	 */
	cw_bool_t full;

	/* Hard limit to the number of chunks that the arena will allocate. */
	cw_uint32_t max_chunks;

	/*
	 * The size of the chunk, and therefore how much the arena grows by
	 * when the current chunk is full.
	 */
	cw_uint32_t chunk_size;

	/*
	 * List of malloced chunks, each of size chunk_size.  The tail
	 * pointer points to the page where allocations can occur.
	 */
	cw_list_t chunks;

	/*
	 * The place in the chunk where the next allocation request will be
	 * located. This number is aligned such that the allocated pointer
	 * will be aligned on a doubleword boundary.  If an allocation
	 * request for n bytes comes in through arena_alloc, and n +
	 * next_alloc_location > chunk_size, a new chunk will be allocated.
	 * No effort is made to reduce fragmentation.
	 */
	cw_uint32_t next_alloc_location;
};

cw_arena_t *arena_new(cw_arena_t *a_arena, cw_uint32_t a_chunk_size,
    cw_uint32_t a_max_chunks);

cw_arena_t *arena_new_r(cw_arena_t *a_arena, cw_uint32_t a_chunk_size,
    cw_uint32_t a_max_chunks);

void    arena_delete(cw_arena_t *a_arena);

cw_uint32_t arena_get_chunk_size(cw_arena_t *a_arena);

cw_uint32_t arena_get_max_chunks(cw_arena_t *a_arena);

void   *arena_malloc(cw_arena_t *a_arena, cw_uint32_t a_size);
void   *arena_malloc_e(cw_arena_t *a_arena, cw_uint32_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);

#define _cw_arena_malloc(a_arena, a_size)				\
	arena_alloc_e((a_arena), (a_size), __FILE__, __LINE__)
