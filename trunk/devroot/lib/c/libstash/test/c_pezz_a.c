/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Test the pezz class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

	dbg_register(cw_g_dbg, "pezz_error");
/*  	dbg_register(cw_g_dbg, "pezz_verbose"); */

	/* pezz_new(), pezz_delete(), pezz_get_buffer_size(). */
	{
		cw_pezz_t	pezz, *pezz_p;

		_cw_assert(pezz_new(&pezz, cw_g_mem, 123, 7) == &pezz);
		_cw_assert(pezz_get_buffer_size(&pezz) == 123);
		pezz_delete(&pezz);

		pezz_p = pezz_new(NULL, cw_g_mem, 234, 11);
		_cw_check_ptr(pezz_p);
		_cw_assert(pezz_get_buffer_size(pezz_p) == 234);
		pezz_delete(pezz_p);
	}

	/* pezz_get(), pezz_put(). */
	{
		cw_pezz_t	pezz;
		void		*pointers[100];
		cw_uint32_t	i;

		pezz_new(&pezz, cw_g_mem, 4096, 10);
		_cw_assert(pezz_get_buffer_size(&pezz) == 4096);
		for (i = 0; i < 100; i++) {
			pointers[i] = _cw_pezz_get(&pezz);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 100; i++) {
			_cw_pezz_put(&pezz, pointers[i]);
			pointers[i] = NULL;
		}

		for (i = 0; i < 5; i++) {
			pointers[i] = _cw_pezz_get(&pezz);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 5; i++) {
			_cw_pezz_put(&pezz, pointers[i]);
			pointers[i] = NULL;
		}

		for (i = 0; i < 6; i++) {
			pointers[i] = _cw_pezz_get(&pezz);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 6; i++) {
			_cw_pezz_put(&pezz, pointers[i]);
			pointers[i] = NULL;
		}

		for (i = 0; i < 1; i++) {
			pointers[i] = _cw_pezz_get(&pezz);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 1; i++) {
			_cw_pezz_put(&pezz, pointers[i]);
			pointers[i] = NULL;
		}

		for (i = 0; i < 11; i++) {
			pointers[i] = _cw_pezz_get(&pezz);
			_cw_check_ptr(pointers[i]);
		}
		for (i = 0; i < 11; i++) {
			_cw_pezz_put(&pezz, pointers[i]);
			pointers[i] = NULL;
		}

		pezz_delete(&pezz);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
