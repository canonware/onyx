/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = toshok>
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Public interface for the arena class.  arena affords very fast allocation
 * at the expense of never freeing anything until you free everything, in
 * arena_delete
 * many equal-size buffers.  It does incremental block allocation, then carves
 * buffers from those blocks.  No memory is freed until pezz_delete() is called.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_arena_s cw_arena_t;

struct cw_arena_s
{
  cw_bool_t is_malloced;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
#endif

  /* If we notice we're full, this is set to true.  Then all allocation is short
   * circuited and returns FALSE. */
  cw_bool_t full;

  /* Hard limit to the number of chunks that the arena will allocate. */
  cw_uint32_t max_chunks;

  /* The size of the chunk, and therefore how much the arena grows by when the
   * current chunk is full. */
  cw_uint32_t chunk_size;

  /* List of malloced chunks, each of size chunk_size.  The tail pointer points
   * to the page where allocations can occur. */
  cw_list_t chunks;

  /* The place in the chunk where the next allocation request will be located.
   * This number is aligned such that the allocated pointer will be aligned on a
   * doubleword boundary.  If an allocation request for n bytes comes in through
   * arena_alloc, and n + next_alloc_location > chunk_size, a new chunk will be
   * allocated.  No effort is made to reduce fragmentation. */
  cw_uint32_t next_alloc_location;
};

#ifdef _LIBSTASH_DBG
typedef struct
{
  const char * filename;
  cw_uint32_t line_num;
} cw_arena_item_t;
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_arena : Pointer to space for an arena, or NULL.
 *
 * a_chunk_size : Size of chunks the arena will use.
 *
 * a_max_chunks : Hard limit to the number of chunks the arena will
 *                allocate, or 0 for no limit.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to an arena, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Non-thread-safe and thread-safe constructors.
 *
 ****************************************************************************/
cw_arena_t *
arena_new(cw_arena_t * a_arena, cw_uint32_t a_chunk_size,
	  cw_uint32_t a_max_chunks);
#ifdef _CW_REENTRANT
cw_arena_t *
arena_new_r(cw_arena_t * a_arena, cw_uint32_t a_chunk_size,
	    cw_uint32_t a_max_chunks);
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_arena : Pointer to an arena.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
arena_delete(cw_arena_t * a_arena);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_arena : Pointer to an arena.
 *
 * <<< Output(s) >>>
 *
 * retval : Size of chunks that a_arena is using.
 *
 * <<< Description >>>
 *
 * Return the size of the chunks that a_pezz is using.
 *
 ****************************************************************************/
cw_uint32_t
arena_get_chunk_size(cw_arena_t * a_arena);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_arena : Pointer to an arena.
 *
 * <<< Output(s) >>>
 *
 * retval : Maximum number of chunks allowed in this arena.
 *
 * <<< Description >>>
 *
 * Return the maximum number of chunks that a_arena can have.
 *
 ****************************************************************************/
cw_uint32_t
arena_get_max_chunks(cw_arena_t * a_arena);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_arena : Pointer to an arena.
 *
 * a_size : Size of memory area to allocate.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a memory buffer.
 *
 * <<< Description >>>
 *
 * Allocate and return a zero filled memory area of the given size
 * from the arena.
 *
 ****************************************************************************/
void *
arena_malloc(cw_arena_t * a_arena, cw_uint32_t a_size);
void *
arena_malloc_e(cw_arena_t * a_arena, cw_uint32_t a_size,
	      const char * a_filename, cw_uint32_t a_line_num);

#define _cw_arena_malloc(a_arena, a_size) \
  arena_alloc_e((a_arena), (a_size), __FILE__, __LINE__)
