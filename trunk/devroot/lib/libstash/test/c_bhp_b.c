/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Unit tests for the bhp (binomial heap) class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#define _LIBSTASH_TEST_HEAP_SIZE 4097

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

	/* bhpi_new(), bhpi_delete(). */
	{
		cw_bhpi_t	bhpi, *bhpi_p;
		cw_uint32_t	prio, data;

		_cw_assert(&bhpi == bhpi_new(&bhpi, cw_g_mem, (const void
		    *)prio, (const void *)data, NULL, NULL));
		bhpi_delete(&bhpi);

		bhpi_p = bhpi_new(NULL, cw_g_mem, (const void *)prio, (const
		    void *)data, NULL, NULL);
		_cw_check_ptr(bhpi_p);
		bhpi_delete(bhpi_p);
	}

	/* bhp_new[_r](), bhp_delete(). */
	{
		cw_bhp_t	bhp, *bhp_p;

		_cw_assert(&bhp == bhp_new_r(&bhp, cw_g_mem,
		    bhp_priority_compare_uint32));
		bhp_delete(&bhp);

		bhp_p = bhp_new(NULL, cw_g_mem, bhp_priority_compare_sint32);
		_cw_check_ptr(bhp_p);
		bhp_delete(bhp_p);
	}

	/* bhp_insert(), bhp_find_min(), bhp_get_size(), bhp_del_min(). */
	{
		cw_bhp_t	*bhp_p;
		cw_bhpi_t	*bhpi_p;
		cw_uint32_t	items[_LIBSTASH_TEST_HEAP_SIZE], i;

		bhp_p = bhp_new_r(NULL, cw_g_mem, bhp_priority_compare_uint32);
		_cw_check_ptr(bhp_p);

		for (i = 0; i < _LIBSTASH_TEST_HEAP_SIZE; i++) {
			items[i] = i;

			bhpi_p = bhpi_new(NULL, cw_g_mem, (const void
			    *)&items[i], (const void *)items[i], NULL, NULL);
			_cw_check_ptr(bhpi_p);

			_cw_assert(bhp_get_size(bhp_p) == i);
			bhp_insert(bhp_p, bhpi_p);
			_cw_assert(bhp_get_size(bhp_p) == i + 1);
		}

		for (i = 0; i < 129; i++) {
			_cw_assert(bhp_get_size(bhp_p) ==
			    _LIBSTASH_TEST_HEAP_SIZE - i);
			_cw_assert(bhp_del_min(bhp_p, NULL, NULL) == FALSE);
			_cw_assert(bhp_get_size(bhp_p) ==
			    _LIBSTASH_TEST_HEAP_SIZE - 1 - i);
		}

		bhp_delete(bhp_p);
	}

	/* bhp_union(). */
	{
		cw_bhp_t	*bhp_p_a, *bhp_p_b;
		cw_bhpi_t	*bhpi_p;
		cw_uint32_t	items[_LIBSTASH_TEST_HEAP_SIZE], i;

		bhp_p_a = bhp_new_r(NULL, cw_g_mem,
		    bhp_priority_compare_uint32);
		_cw_check_ptr(bhp_p_a);

		bhp_p_b = bhp_new_r(NULL, cw_g_mem,
		    bhp_priority_compare_uint32);
		_cw_check_ptr(bhp_p_b);

		for (i = 0; i < _LIBSTASH_TEST_HEAP_SIZE; i++) {
			items[i] = i;

			bhpi_p = bhpi_new(NULL, cw_g_mem, (const void
			    *)&items[i], (const void *)items[i], NULL, NULL);
			_cw_check_ptr(bhpi_p);
			bhp_insert(bhp_p_a, bhpi_p);

			bhpi_p = bhpi_new(NULL, cw_g_mem, (const void
			    *)&items[i], (const void *)items[i], NULL, NULL);
			_cw_check_ptr(bhpi_p);
			bhp_insert(bhp_p_b, bhpi_p);
		}

		bhp_union(bhp_p_a, bhp_p_b);

		bhp_delete(bhp_p_a);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
