/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Test of the treen class.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

	/* treen_new(), treen_delete(). */
	{
		cw_treen_t	*treen_a, treen_b;

		treen_a = treen_new(NULL, cw_g_mem, NULL, NULL);
		_cw_check_ptr(treen_a);
		treen_delete(treen_a);

		treen_a = treen_new(_cw_malloc(sizeof(cw_treen_t)), cw_g_mem,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem);
		_cw_check_ptr(treen_a);
		treen_delete(treen_a);

		treen_a = treen_new(&treen_b, NULL, NULL, NULL);
		_cw_assert(treen_a == &treen_b);
		treen_delete(&treen_b);
	}

	/*
	 * treen_link(), treen_parent_get(), treen_child_get(),
	 * treen_sibling_get().
	 */
	{
		cw_treen_t	*treen_a, *treen_b, *treen_c, *treen_d;

		treen_a = treen_new(NULL, cw_g_mem, NULL, NULL);
		_cw_check_ptr(treen_a);
		treen_b = treen_new(NULL, cw_g_mem, NULL, NULL);
		_cw_check_ptr(treen_b);
		treen_c = treen_new(NULL, cw_g_mem, NULL, NULL);
		_cw_check_ptr(treen_c);
		treen_d = treen_new(NULL, cw_g_mem, NULL, NULL);
		_cw_check_ptr(treen_d);

		treen_link(treen_b, treen_a);
		_cw_assert(treen_child_get(treen_a) == treen_b);
		_cw_assert(treen_sibling_get(treen_b) == treen_b);

		treen_link(treen_c, treen_a);
		_cw_assert(treen_child_get(treen_a) == treen_b);
		_cw_assert(treen_sibling_get(treen_b) == treen_c);
		_cw_assert(treen_sibling_get(treen_c) == treen_b);

		treen_link(treen_d, treen_a);
		_cw_assert(treen_child_get(treen_a) == treen_b);
		_cw_assert(treen_sibling_get(treen_b) == treen_c);
		_cw_assert(treen_sibling_get(treen_c) == treen_d);
		_cw_assert(treen_sibling_get(treen_d) == treen_b);
		_cw_assert(treen_parent_get(treen_b) == treen_a);
		_cw_assert(treen_parent_get(treen_c) == treen_a);
		_cw_assert(treen_parent_get(treen_d) == treen_a);

		treen_link(treen_a, NULL);
		treen_link(treen_b, NULL);
		treen_link(treen_d, NULL);
		_cw_assert(treen_child_get(treen_a) == treen_c);

		_cw_assert(treen_child_get(treen_c) == NULL);
		treen_link(treen_b, treen_c);
		_cw_assert(treen_b == treen_child_get(treen_c));

		treen_link(treen_d, treen_c);
		_cw_assert(treen_child_get(treen_c) == treen_b);
		_cw_assert(treen_sibling_get(treen_c) == treen_c);
		_cw_assert(treen_sibling_get(treen_b) == treen_d);
		_cw_assert(treen_sibling_get(treen_d) == treen_b);
		_cw_assert(treen_parent_get(treen_b) == treen_c);
		_cw_assert(treen_parent_get(treen_d) == treen_c);

		treen_link(treen_c, NULL);
		_cw_assert(treen_child_get(treen_a) == NULL);
		_cw_assert(treen_parent_get(treen_c) == NULL);

		treen_link(treen_a, treen_d);
		treen_link(treen_a, treen_c);

		treen_delete(treen_c);
	}

	/* treen_data_ptr_get(), treen_data_ptr_set(). */
	{
		cw_treen_t	*treen_a;

		treen_a = treen_new(NULL, cw_g_mem, NULL, NULL);
		_cw_check_ptr(treen_a);

		_cw_assert(treen_data_ptr_get(treen_a) == NULL);

		treen_data_ptr_set(treen_a, (void *)treen_a);
		_cw_assert(treen_data_ptr_get(treen_a) == treen_a);

		treen_delete(treen_a);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
