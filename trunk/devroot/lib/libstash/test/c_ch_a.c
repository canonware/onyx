/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * ch (chained hash) test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

	/* ch_new(), ch_delete(). */
	{
		cw_ch_t	*ch_a, ch_b;

		ch_a = ch_new(NULL, 2, ch_hash_string, ch_key_comp_string);
		_cw_check_ptr(ch_a);
		ch_delete(ch_a);

		_cw_assert(ch_new(&ch_b, 1, ch_hash_direct, ch_key_comp_direct)
		    == &ch_b);
		ch_delete(&ch_b);
	}

	/* ch_count(), ch_insert(). */
	{
		cw_ch_t	*ch;
		char	*a = "a string";
		char	*b = "A string";
		char	*c = "two of these";
		char	*d = "two of these\0foo";

		ch = ch_new(NULL, 4, ch_hash_string, ch_key_comp_string);
		_cw_check_ptr(ch);
		_cw_assert(ch_count(ch) == 0);

		_cw_assert(ch_insert(ch, a, a, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 1);

		_cw_assert(ch_insert(ch, b, b, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 2);

		_cw_assert(ch_insert(ch, c, c, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 3);

		_cw_assert(ch_insert(ch, d, d, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 4);

		_cw_assert(ch_insert(ch, d, d, NULL) == FALSE);
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

		ch = ch_new(NULL, 4, ch_hash_string, ch_key_comp_string);
		_cw_check_ptr(ch);
		_cw_assert(ch_count(ch) == 0);

		_cw_assert(ch_insert(ch, a, a, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 1);

		_cw_assert(ch_insert(ch, b, b, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 2);

		_cw_assert(ch_insert(ch, c, c, NULL) == FALSE);
		_cw_assert(ch_count(ch) == 3);

		_cw_assert(ch_insert(ch, d, d, NULL) == FALSE);
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
		cw_chi_t	*chi;
		cw_pezz_t	*chi_pezz;
		char		*a = "a string";
		char		*b = "A string";
		char		*c = "two of these";
		char		*d = "two of these\0foo";
		char		*v;

		chi_pezz = pezz_new(NULL, sizeof(cw_chi_t), 10);
		_cw_check_ptr(chi_pezz);

		ch = ch_new(NULL, 4, ch_hash_string, ch_key_comp_string);
		_cw_check_ptr(ch);

		_cw_assert(ch_insert(ch, a, a, (cw_chi_t
		    *)_cw_pezz_get(chi_pezz)) == FALSE);
		_cw_assert(ch_insert(ch, b, b, (cw_chi_t
		    *)_cw_pezz_get(chi_pezz)) == FALSE);
		_cw_assert(ch_insert(ch, c, c, (cw_chi_t
		    *)_cw_pezz_get(chi_pezz)) == FALSE);
		_cw_assert(ch_insert(ch, d, d, (cw_chi_t
		    *)_cw_pezz_get(chi_pezz)) == FALSE);

		_cw_assert(ch_search(ch, "foo", (void **)&v));

		_cw_assert(ch_search(ch, a, (void **)&v) == FALSE);
		_cw_assert(v == a);

		_cw_assert(ch_search(ch, b, (void **)&v) == FALSE);
		_cw_assert(v == b);

		_cw_assert(ch_search(ch, c, (void **)&v) == FALSE);
		_cw_assert(v == d);

		_cw_assert(ch_search(ch, d, (void **)&v) == FALSE);
		_cw_assert(v == d);

		while (ch_remove_iterate(ch, NULL, NULL, &chi) == FALSE)
			_cw_pezz_put(chi_pezz, chi);
		
		ch_delete(ch);
		pezz_delete(chi_pezz);
	}

	/* ch_get_iterate(), ch_remove_iterate(). */
	{
		cw_ch_t		*ch;
		cw_chi_t	*chi;
		cw_pezz_t	*chi_pezz;
		char		*a = "a string";
		char		*b = "A string";
		char		*c = "two of these";
		char		*d = "two of these\0foo";
		char		*k, *v;

		chi_pezz = pezz_new(NULL, sizeof(cw_chi_t), 10);
		_cw_check_ptr(chi_pezz);

		ch = ch_new(NULL, 4, ch_hash_string, ch_key_comp_string);
		_cw_check_ptr(ch);

		_cw_assert(ch_insert(ch, a, a, (cw_chi_t
		    *)_cw_pezz_get(chi_pezz)) == FALSE);
		_cw_assert(ch_insert(ch, b, b, (cw_chi_t
		    *)_cw_pezz_get(chi_pezz)) == FALSE);
		_cw_assert(ch_insert(ch, c, c, (cw_chi_t
		    *)_cw_pezz_get(chi_pezz)) == FALSE);
		_cw_assert(ch_insert(ch, d, d, (cw_chi_t
		    *)_cw_pezz_get(chi_pezz)) == FALSE);

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

		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v) ==
		    FALSE);
		_cw_assert(k == a);
		_cw_assert(v == a);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v, &chi)
		    == FALSE);
		_cw_pezz_put(chi_pezz, chi);
		_cw_assert(k == b);
		_cw_assert(v == b);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v, &chi)
		    == FALSE);
		_cw_pezz_put(chi_pezz, chi);
		_cw_assert(k == c);
		_cw_assert(v == c);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v, &chi)
		    == FALSE);
		_cw_pezz_put(chi_pezz, chi);
		_cw_assert(k == d);
		_cw_assert(v == d);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v, &chi)
		    == FALSE);
		_cw_pezz_put(chi_pezz, chi);
		_cw_assert(k == a);
		_cw_assert(v == a);

		_cw_assert(ch_remove_iterate(ch, (void **)&k, (void **)&v,
		    NULL));
		_cw_assert(ch_get_iterate(ch, (void **)&k, (void **)&v));

		ch_delete(ch);
		pezz_delete(chi_pezz);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
