/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Test the pool class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");


	dbg_register(cw_g_dbg, "pool_error");
/*  	dbg_register(cw_g_dbg, "pool_verbose"); */

	/* pool_new(), pool_delete(), pool_get_buffer_size(). */
	{
		cw_pool_t	pool, *pool_p;

		_cw_assert(pool_new(&pool, cw_g_mem, 123) == &pool);
		_cw_assert(pool_get_buffer_size(&pool) == 123);
		pool_delete(&pool);

		pool_p = pool_new(NULL, cw_g_mem, 234);
		_cw_check_ptr(pool_p);
		_cw_assert(pool_get_buffer_size(pool_p) == 234);
		pool_delete(pool_p);
	}

	/* pool_get(), pool_put(), pool_drain(). */
	{
		cw_pool_t	pool;
		void		*pointers[100];
		cw_uint32_t	i;

		pool_new(&pool, cw_g_mem, 4096);
		_cw_assert(pool_get_buffer_size(&pool) == 4096);
		for (i = 0; i < 100; i++) {
			pointers[i] = _cw_pool_get(&pool);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 100; i++) {
			_cw_pool_put(&pool, pointers[i]);
			pointers[i] = NULL;
		}

		for (i = 0; i < 5; i++) {
			pointers[i] = _cw_pool_get(&pool);
			_cw_check_ptr(pointers[i]);
		}
		pool_drain(&pool);
		for (i = 0; i < 5; i++) {
			_cw_pool_put(&pool, pointers[i]);
			pointers[i] = NULL;
		}

		for (i = 0; i < 6; i++) {
			pointers[i] = _cw_pool_get(&pool);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 6; i++) {
			_cw_pool_put(&pool, pointers[i]);
			pointers[i] = NULL;
		}

		for (i = 0; i < 1; i++) {
			pointers[i] = _cw_pool_get(&pool);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 1; i++) {
			_cw_pool_put(&pool, pointers[i]);
			pointers[i] = NULL;
		}
		pool_drain(&pool);

		for (i = 0; i < 11; i++) {
			pointers[i] = _cw_pool_get(&pool);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 11; i++) {
			_cw_pool_put(&pool, pointers[i]);
			pointers[i] = NULL;
		}

		pool_delete(&pool);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
