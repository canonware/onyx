/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * dch (dynamic chained chained hash) test.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	out_put(out_err, "Test begin\n");

	/* dch_new(), dch_delete(). */
	{
		cw_dch_t	*dch_a, dch_b;

		dch_a = dch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 2, 2, 1,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(dch_a);
		dch_delete(dch_a);

		_cw_assert(dch_new(&dch_b, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 4, 3, 1,
		    ch_direct_hash, ch_direct_key_comp) == &dch_b);
		dch_delete(&dch_b);
	}

	/* dch_count(), dch_insert(). */
	{
		cw_dch_t	*dch;
		char		*a = "a string";
		char		*b = "A string";
		char		*c = "two of these";
		char		*d = "two of these\0foo";

		dch = dch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 4, 2, 1,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(dch);
		_cw_assert(dch_count(dch) == 0);

		dch_insert(dch, a, a, NULL);
		_cw_assert(dch_count(dch) == 1);

		dch_insert(dch, b, b, NULL);
		_cw_assert(dch_count(dch) == 2);

		dch_insert(dch, c, c, NULL);
		_cw_assert(dch_count(dch) == 3);

		dch_insert(dch, d, d, NULL);
		_cw_assert(dch_count(dch) == 4);

		dch_insert(dch, d, d, NULL);
		_cw_assert(dch_count(dch) == 5);

		dch_delete(dch);
	}

	/* dch_count, dch_remove(). */
	{
		cw_dch_t	*dch;
		char		*a = "a string";
		char		*b = "A string";
		char		*c = "two of these";
		char		*d = "two of these\0foo";
		char		*k, *v;

		dch = dch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 4, 2, 1,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(dch);
		_cw_assert(dch_count(dch) == 0);

		dch_insert(dch, a, a, NULL);
		_cw_assert(dch_count(dch) == 1);

		dch_insert(dch, b, b, NULL);
		_cw_assert(dch_count(dch) == 2);

		dch_insert(dch, c, c, NULL);
		_cw_assert(dch_count(dch) == 3);

		dch_insert(dch, d, d, NULL);
		_cw_assert(dch_count(dch) == 4);

		_cw_assert(dch_remove(dch, a, (void **)&k, (void **)&v, NULL) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);
		_cw_assert(dch_count(dch) == 3);

		_cw_assert(dch_remove(dch, a, NULL, NULL, NULL));

		_cw_assert(dch_remove(dch, b, NULL, NULL, NULL) == FALSE);
		_cw_assert(dch_count(dch) == 2);

		_cw_assert(dch_remove(dch, c, (void **)&k, (void **)&v, NULL) ==
		    FALSE);
		_cw_assert(k == d);
		_cw_assert(v == d);
		_cw_assert(dch_count(dch) == 1);

		_cw_assert(dch_remove(dch, c, NULL, NULL, NULL) == FALSE);
		_cw_assert(dch_count(dch) == 0);

		_cw_assert(dch_remove(dch, d, NULL, NULL, NULL));
		_cw_assert(dch_count(dch) == 0);

		_cw_assert(dch_remove(dch, a, NULL, NULL, NULL));

		dch_delete(dch);
	}

	/* dch_search(). */
	{
		cw_dch_t	*dch;
		cw_chi_t	*chi;
		cw_pool_t	*chi_pool;
		char		*a = "a string";
		char		*b = "A string";
		char		*c = "two of these";
		char		*d = "two of these\0foo";
		char		*v;

		chi_pool = pool_new(NULL, cw_g_mem, sizeof(cw_chi_t));
		_cw_check_ptr(chi_pool);

		dch = dch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 4, 2, 1,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(dch);

		dch_insert(dch, a, a, (cw_chi_t *)pool_get(chi_pool));
		dch_insert(dch, b, b, (cw_chi_t *)pool_get(chi_pool));
		dch_insert(dch, c, c, (cw_chi_t *)pool_get(chi_pool));
		dch_insert(dch, d, d, (cw_chi_t *)pool_get(chi_pool));

		_cw_assert(dch_search(dch, "foo", (void **)&v));

		_cw_assert(dch_search(dch, a, (void **)&v) == FALSE);
		_cw_assert(v == a);

		_cw_assert(dch_search(dch, b, (void **)&v) == FALSE);
		_cw_assert(v == b);

		_cw_assert(dch_search(dch, c, (void **)&v) == FALSE);
		_cw_assert(v == d);

		_cw_assert(dch_search(dch, d, (void **)&v) == FALSE);
		_cw_assert(v == d);

		while (dch_remove_iterate(dch, NULL, NULL, &chi) == FALSE)
			pool_put(chi_pool, chi);

		dch_delete(dch);
		pool_delete(chi_pool);
	}

	/* dch_get_iterate(), dch_remove_iterate(). */
	{
		cw_dch_t	*dch;
		cw_chi_t	*chi;
		cw_pool_t	*chi_pool;
		char		*a = "a string";
		char		*b = "A string";
		char		*c = "two of these";
		char		*d = "two of these\0foo";
		char		*k, *v;

		chi_pool = pool_new(NULL, cw_g_mem, sizeof(cw_chi_t));
		_cw_check_ptr(chi_pool);

		dch = dch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 3, 2, 1,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(dch);

		dch_insert(dch, a, a, (cw_chi_t *)pool_get(chi_pool));
		dch_insert(dch, b, b, (cw_chi_t *)pool_get(chi_pool));
		dch_insert(dch, c, c, (cw_chi_t *)pool_get(chi_pool));
		dch_insert(dch, d, d, (cw_chi_t *)pool_get(chi_pool));

		_cw_assert(dch_get_iterate(dch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		_cw_assert(dch_get_iterate(dch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == b);
		_cw_assert(v == b);

		_cw_assert(dch_get_iterate(dch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == c);
		_cw_assert(v == c);

		_cw_assert(dch_get_iterate(dch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == d);
		_cw_assert(v == d);

		_cw_assert(dch_get_iterate(dch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		_cw_assert(dch_remove_iterate(dch, (void **)&k, (void **)&v,
		    &chi) == FALSE);
		_cw_assert(k == b);
		_cw_assert(v == b);
		pool_put(chi_pool, chi);

		_cw_assert(dch_remove_iterate(dch, (void **)&k, (void **)&v,
		    &chi) == FALSE);
		_cw_assert(k == c);
		_cw_assert(v == c);
		pool_put(chi_pool, chi);

		_cw_assert(dch_remove_iterate(dch, (void **)&k, (void **)&v,
		    &chi) == FALSE);
		_cw_assert(k == d);
		_cw_assert(v == d);
		pool_put(chi_pool, chi);

		_cw_assert(dch_remove_iterate(dch, (void **)&k, (void **)&v,
		    &chi) == FALSE);
		_cw_assert(a == k);
		_cw_assert(a == v);
		pool_put(chi_pool, chi);

		_cw_assert(dch_remove_iterate(dch, (void **)&k, (void **)&v,
		    NULL));
		_cw_assert(dch_get_iterate(dch, (void **)&k, (void **)&v));

		dch_delete(dch);
		pool_delete(chi_pool);
	}

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
