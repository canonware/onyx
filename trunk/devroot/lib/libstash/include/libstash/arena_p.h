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

static void *arena_p_new_chunk(cw_arena_t *a_arena);

static cw_arena_t *arena_p_new(cw_arena_t *a_arena, cw_uint32_t a_chunk_size,
    cw_uint32_t a_max_chunks, cw_bool_t a_is_thread_safe);
