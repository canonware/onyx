/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * ch (chained hash) test.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

int
main()
{
	libonyx_init();
	fprintf(stderr, "Test begin\n");

	/* ch_new(), ch_delete(). */
	{
		cw_ch_t	*ch_a, ch_b;

		ch_a = ch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 2,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(ch_a);
		ch_delete(ch_a);

		_cw_assert(ch_new(&ch_b, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 1,
		    ch_direct_hash, ch_direct_key_comp) == &ch_b);
		ch_delete(&ch_b);
	}

	/* ch_count(), ch_insert(). */
	{
		cw_ch_t	*ch;
		char	*a = "a string";
		char	*b = "A string";
		char	*c = "two of these";
		char	*d = "two of these\0foo";

		ch = ch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 4,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(ch);
		_cw_assert(ch_count(ch) == 0);

		ch_insert(ch, a, a, NULL);
		_cw_assert(ch_count(ch) == 1);

		ch_insert(ch, b, b, NULL);
		_cw_assert(ch_count(ch) == 2);

		ch_insert(ch, c, c, NULL);
		_cw_assert(ch_count(ch) == 3);

		ch_insert(ch, d, d, NULL);
		_cw_assert(ch_count(ch) == 4);

		ch_insert(ch, d, d, NULL);
		_cw_assert(ch_count(ch) == 5);

		ch_delete(ch);
	}

	/* ch_count, ch_remove(). */
	{
		cw_ch_t	*ch;
		char	*a = "a string";
		char	*b = "A string";
		char	*c = "two of these";
		char	*d = "two of these\0foo";
		char	*k, *v;

		ch = ch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 4,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(ch);
		_cw_assert(ch_count(ch) == 0);

		ch_insert(ch, a, a, NULL);
		_cw_assert(ch_count(ch) == 1);

		ch_insert(ch, b, b, NULL);
		_cw_assert(ch_count(ch) == 2);

		ch_insert(ch, c, c, NULL);
		_cw_assert(ch_count(ch) == 3);

		ch_insert(ch, d, d, NULL);
		_cw_assert(ch_count(ch) == 4);

		_cw_assert(ch_remove(ch, a, (void **)&k, (void **)&v, NULL) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);
		_cw_assert(ch_count(ch) == 3);

		_cw_assert(ch_remove(ch, a, NULL, NULL, NULL));

		_cw_assert(ch_remove(ch, b, NULL, NULL, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 2);

		_cw_assert(ch_remove(ch, c, (void **)&k, (void **)&v, NULL) ==
		    FALSE);
		_cw_assert(k == d);
		_cw_assert(v == d);
		_cw_assert(ch_count(ch) == 1);

		_cw_assert(ch_remove(ch, c, NULL, NULL, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 0);

		_cw_assert(ch_remove(ch, d, NULL, NULL, NULL));
		_cw_assert(ch_count(ch) == 0);

		_cw_assert(ch_remove(ch, a, NULL, NULL, NULL));

		ch_delete(ch);
	}

	/* ch_search(). */
	{
		cw_ch_t		*ch;
		char		*a = "a string";
		char		*b = "A string";
		char		*c = "two of these";
		char		*d = "two of these\0foo";
		char		*v;

		ch = ch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 4,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(ch);

		ch_insert(ch, a, a, NULL);
		ch_insert(ch, b, b, NULL);
		ch_insert(ch, c, c, NULL);
		ch_insert(ch, d, d, NULL);

		_cw_assert(ch_search(ch, "foo", (void **)&v));

		_cw_assert(ch_search(ch, a, (void **)&v) == FALSE);
		_cw_assert(v == a);

		_cw_assert(ch_search(ch, b, (void **)&v) == FALSE);
		_cw_assert(v == b);

		_cw_assert(ch_search(ch, c, (void **)&v) == FALSE);
		_cw_assert(v == d);

		_cw_assert(ch_search(ch, d, (void **)&v) == FALSE);
		_cw_assert(v == d);

		while (ch_remove_iterate(ch, NULL, NULL, NULL) == FALSE)
			;
		
		ch_delete(ch);
	}

	/* ch_get_iterate(), ch_remove_iterate(). */
	{
		cw_ch_t		*ch;
		char		*a = "a string";
		char		*b = "A string";
		char		*c = "two of these";
		char		*d = "two of these\0foo";
		char		*k, *v;

		ch = ch_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
		    (cw_opaque_dealloc_t *)mem_free_e, cw_g_mem, 4,
		    ch_string_hash, ch_string_key_comp);
		_cw_check_ptr(ch);

		/* Iterate with 0 items. */
		/* Round 1. */
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    TRUE);
		/* Round 2. */
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    TRUE);

		ch_insert(ch, a, a, NULL);
		/* Iterate with 1 item. */
		/* Round 1. */
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		/* Round 2. */
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		ch_insert(ch, b, b, NULL);
		ch_insert(ch, c, c, NULL);
		ch_insert(ch, d, d, NULL);

		/* Iterate with 4 items. */
		/* Round 1. */
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == b);
		_cw_assert(v == b);

		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == c);
		_cw_assert(v == c);

		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == d);
		_cw_assert(v == d);

		/* Round 2. */
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == b);
		_cw_assert(v == b);

		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == c);
		_cw_assert(v == c);

		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == d);
		_cw_assert(v == d);

		/* Start round 3. */
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		/* Remove. */
		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v, NULL)
		    == FALSE);
		_cw_assert(k == b);
		_cw_assert(v == b);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v, NULL)
		    == FALSE);
		_cw_assert(k == c);
		_cw_assert(v == c);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v, NULL)
		    == FALSE);
		_cw_assert(k == d);
		_cw_assert(v == d);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v, NULL)
		    == FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v,
		    NULL));
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v));

		ch_delete(ch);
	}

	fprintf(stderr, "Test end\n");
	libonyx_shutdown();
	return 0;
}
