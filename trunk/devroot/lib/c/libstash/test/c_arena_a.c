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
 * <<< Description >>>
 *
 * Test the arena class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	out_put(cw_g_out, "Test begin\n");

/*    dbg_register(cw_g_dbg, "mem_verbose"); */

	/*
	 * arena_new(), arena_new_r(), arena_delete(),
	 * arena_get_chunk_size(), arena_get_max_chunks().
	 */
	{
		cw_arena_t arena, *arena_p;

		_cw_assert(&arena == arena_new(&arena, 100, 240));
		_cw_assert(100 == arena_get_chunk_size(&arena));
		_cw_assert(240 == arena_get_max_chunks(&arena));
		arena_delete(&arena);

		_cw_assert(&arena == arena_new_r(&arena, 100, 240));
		_cw_assert(100 == arena_get_chunk_size(&arena));
		_cw_assert(240 == arena_get_max_chunks(&arena));
		arena_delete(&arena);

		arena_p = arena_new(NULL, 234, 11);
		_cw_check_ptr(arena_p);
		_cw_assert(234 == arena_get_chunk_size(arena_p));
		arena_delete(arena_p);

		arena_p = arena_new_r(NULL, 234, 11);
		_cw_check_ptr(arena_p);
		_cw_assert(234 == arena_get_chunk_size(arena_p));
		arena_delete(arena_p);
	}

	/* arena_malloc(), check alignment. */
	/* XXX Missing?. */

	/* Check multiple allocations, all within one chunk. */
	{
		cw_arena_t arena;
		void   *pointers[10];
		int     i;

		arena_new_r(&arena, 4096, 10);
		_cw_assert(4096 == arena_get_chunk_size(&arena));
		_cw_assert(10 == arena_get_max_chunks(&arena));

		for (i = 0; i < 10; i++) {
			pointers[i] = arena_malloc(&arena, 10);
			_cw_check_ptr(pointers[i]);
			/* Propertly aligned? */
			_cw_assert(0 == (((cw_uint32_t)pointers[i]) & 7));
		}
		arena_delete(&arena);
	}

	/* Check multiple allocations, requiring a new chunk to be created. */
	{
		cw_arena_t arena;
		void   *pointers[10];
		int     i;

		arena_new_r(&arena, 4096, 10);
		_cw_assert(4096 == arena_get_chunk_size(&arena));
		_cw_assert(10 == arena_get_max_chunks(&arena));

		for (i = 0; i < 10; i++) {
			pointers[i] = arena_malloc(&arena, 500);
			_cw_check_ptr(pointers[i]);
			/* Propertly aligned? */
			_cw_assert(0 == (((cw_uint32_t)pointers[i]) & 7));
		}
		arena_delete(&arena);
	}

	/*
	 * Check single allocation larger than chunk size (should return
	 * NULL).
	 */
	{
		cw_arena_t arena;
		void   *pointer;

		arena_new_r(&arena, 4096, 10);
		_cw_assert(4096 == arena_get_chunk_size(&arena));
		_cw_assert(10 == arena_get_max_chunks(&arena));

		pointer = arena_malloc(&arena, 8192);
		_cw_assert(NULL == pointer);
		arena_delete(&arena);
	}

	/*
	 * Check return value after arena has been filled up and no further
	 * arenas can be allocated.
	 */
	{
		cw_arena_t arena;
		void   *pointer;

		arena_new_r(&arena, 4096, 1);
		_cw_assert(4096 == arena_get_chunk_size(&arena));
		_cw_assert(1 == arena_get_max_chunks(&arena));

		pointer = arena_malloc(&arena, 4000);
		_cw_check_ptr(pointer);
		/* Propertly aligned? */
		_cw_assert(0 == (((cw_uint32_t)pointer) & 7));

		pointer = arena_malloc(&arena, 4000);
		_cw_assert(NULL == pointer);

		arena_delete(&arena);
	}

	/*
	 * Check return value after arena has been filled up so that the
	 * next pointer would be after the last chunk.
	 */
	{
		cw_arena_t arena;
		void   *pointer;

		arena_new_r(&arena, 4095, 1);
		_cw_assert(4095 == arena_get_chunk_size(&arena));
		_cw_assert(1 == arena_get_max_chunks(&arena));

		pointer = arena_malloc(&arena, 4094);
		_cw_check_ptr(pointer);
		/* Propertly aligned? */
		_cw_assert(0 == (((cw_uint32_t)pointer) & 7));

		/* Now any allocation should fail. */
		pointer = arena_malloc(&arena, 4000);
		_cw_assert(NULL == pointer);

		pointer = arena_malloc(&arena, 20);
		_cw_assert(NULL == pointer);

		arena_delete(&arena);
	}

	out_put(cw_g_out, "Test end\n");
	libstash_shutdown();
	return 0;
}
