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
 * Implementation of the arena class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"
#include "../include/libstash/arena_p.h"

#ifdef _LIBSTASH_DBG
#  define _CW_ARENA_MAGIC 0xa430a155
#endif

cw_arena_t *
arena_new(cw_arena_t * a_arena, cw_uint32_t a_chunk_size,
	  cw_uint32_t a_max_chunks)
{
  return arena_p_new(a_arena, a_chunk_size, a_max_chunks, FALSE);
}

cw_arena_t *
arena_new_r(cw_arena_t * a_arena, cw_uint32_t a_chunk_size,
	    cw_uint32_t a_max_chunks)
{
  return arena_p_new(a_arena, a_chunk_size, a_max_chunks, TRUE);
}

void
arena_delete(cw_arena_t * a_arena)
{
  cw_list_item_t * p;
  void * chunk;

  _cw_check_ptr(a_arena);
  _cw_assert(_CW_ARENA_MAGIC == a_arena->magic);

  /* Delete all our chunks. */
  for (p = list_get_next(&a_arena->chunks, NULL);
       NULL != p;
       p = list_get_next(&a_arena->chunks, p))
  {
    chunk = list_item_get(p);
    _cw_free(chunk);
  }

  list_delete(&a_arena->chunks);

  if (a_arena->is_thread_safe)
  {
    mtx_delete(&a_arena->lock);
  }
  
  if (TRUE == a_arena->is_malloced)
  {
    _cw_free(a_arena);
  }
#ifdef _LIBSTASH_DBG
  else
  {
    memset(a_arena, 0x5a, sizeof(cw_arena_t));
  }
#endif
}

cw_uint32_t
arena_get_chunk_size(cw_arena_t * a_arena)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_arena);
  _cw_assert(_CW_ARENA_MAGIC == a_arena->magic);

  retval = a_arena->chunk_size;

  return retval;
}

cw_uint32_t
arena_get_max_chunks(cw_arena_t * a_arena)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_arena);
  _cw_assert(_CW_ARENA_MAGIC == a_arena->magic);

  retval = a_arena->max_chunks;

  return retval;
}

void *
arena_malloc(cw_arena_t * a_arena, cw_uint32_t a_size)
{
  return arena_malloc_e(a_arena, a_size, NULL, 0);
}

void *
arena_malloc_e(cw_arena_t * a_arena, cw_uint32_t a_size,
	       const char * a_filename, cw_uint32_t a_line_num)
{
  void * retval;
  void * alloc_chunk; /* Chunk that the allocation is coming from. */
  cw_uint32_t new_alloc_location;

  _cw_check_ptr(a_arena);
  _cw_assert(_CW_ARENA_MAGIC == a_arena->magic);
  if (a_arena->is_thread_safe)
  {
    mtx_lock(&a_arena->lock);
  }

  /* Is this allocation impossible to service? */
  if (a_size > a_arena->chunk_size)
  {
    retval = NULL;
    goto RETURN;
  }

  /* Does this allocation fit in the current chunk? */
  if (a_arena->next_alloc_location + a_size > a_arena->chunk_size)
  {
    void * new_chunk = arena_p_new_chunk(a_arena);

    if (NULL == new_chunk)
    {
      retval = NULL;
      goto RETURN;
    }
      
    if (NULL == list_hpush(&a_arena->chunks, new_chunk))
    {
      /* Memory allocation error. */
      _cw_free(new_chunk);
      retval = NULL;
      goto RETURN;
    }
    a_arena->next_alloc_location = 0;
  }

  alloc_chunk = list_item_get(list_get_next(&a_arena->chunks, NULL));

  retval = (void*) ((char*) alloc_chunk + a_arena->next_alloc_location);

  /* Now adjust next_alloc_location to point to the next doubleword boundary
   * past the current allocation. */
#define INCR_TO_NEXT_ALIGN(x, n) \
  do \
  { \
    cw_uint32_t t; \
    \
    t = (x) & ((n) - 1); \
    (x) = (x) ^ t; \
    t += ((n) - 1); \
    t &= (n); \
    (x) += t; \
  } while (0);

  new_alloc_location = a_arena->next_alloc_location + a_size;
  INCR_TO_NEXT_ALIGN(new_alloc_location, 8);

  /* If the next allocation location is past the end of this chunk, we need a
   * new chunk. */
  if (a_arena->chunk_size < new_alloc_location)
  {
    void * new_chunk = arena_p_new_chunk(a_arena);

    if (NULL == new_chunk)
    {
      /* We're full now.
       *
       * Don't set retval to NULL here, since the current allocation succeeded,
       * but set the next_alloc_location to new_alloc_location so all further
       * allocations will fail. */
      a_arena->next_alloc_location = new_alloc_location;
      goto RETURN;
    }

    if (NULL == list_hpush(&a_arena->chunks, new_chunk))
    {
      /* Memory allocation error.  Don't set retval to NULL here, since the
       * current allocation succeeded, but set the next_alloc_location to
       * new_alloc_location so all further allocations will fail. */
      _cw_free(new_chunk);
      goto RETURN;
    }
    a_arena->next_alloc_location = 0;
  }
  else
  {
    a_arena->next_alloc_location = new_alloc_location;
  }

  RETURN:
  if (a_arena->is_thread_safe)
  {
    mtx_unlock(&a_arena->lock);
  }
  return retval;
}

static cw_arena_t *
arena_p_new(cw_arena_t * a_arena, cw_uint32_t a_chunk_size,
	    cw_uint32_t a_max_chunks, cw_bool_t a_is_thread_safe)
{
  cw_arena_t * retval;
  void *initial_chunk;

  _cw_assert(0 != (a_chunk_size * a_max_chunks));

  if (NULL == a_arena)
  {
    retval = (cw_arena_t *) _cw_malloc(sizeof(cw_arena_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_arena_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_arena;
    bzero(retval, sizeof(cw_arena_t));
    retval->is_malloced = FALSE;
  }

  retval->is_thread_safe = a_is_thread_safe;
  if (retval->is_thread_safe)
  {
    mtx_new(&retval->lock);
  }
  
  retval->chunk_size = a_chunk_size;
  retval->max_chunks = a_max_chunks;

  /* Allocate the first chunk. */
  list_new(&retval->chunks);

  initial_chunk = _cw_calloc(1, retval->chunk_size);
  if (NULL == initial_chunk)
  {
    list_delete(&retval->chunks);
    if (retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }

  if (NULL == list_hpush(&retval->chunks, initial_chunk))
  {
    /* Memory allocation error. */
    _cw_free(initial_chunk);
    list_delete(&retval->chunks);
    if (retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_ARENA_MAGIC;
#endif

  RETURN:
  return retval;
}

static void *
arena_p_new_chunk(cw_arena_t * a_arena)
{
  void * retval, * new_chunk;
  
  /* Check if we can create a new chunk. */
  if ((0 != a_arena->max_chunks)
      && (list_count(&a_arena->chunks) == a_arena->max_chunks))
  {
    /* We're full.  Return NULL. */
    retval = NULL;
    goto RETURN;
  }
  
  /* Create a new chunk and add it to the list. */
  new_chunk = _cw_calloc(1, a_arena->chunk_size);
  if (NULL == new_chunk)
  {
    retval = NULL;
    goto RETURN;
  }

  retval = new_chunk;
  
  RETURN:
  return retval;
}
