/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 * dch (dynamic chained chained hash) test.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

int
main()
{
    libonyx_init(0, NULL, NULL);
    fprintf(stderr, "Test begin\n");

    /* dch_new(), dch_delete(). */
    {
	cw_dch_t *dch_a, dch_b;

	dch_a = dch_new(NULL, cw_g_mema, 2, 2, 1, ch_string_hash,
			ch_string_key_comp);
	cw_check_ptr(dch_a);
	dch_delete(dch_a);

	cw_assert(dch_new(&dch_b, cw_g_mema, 4, 3, 1, ch_direct_hash,
			  ch_direct_key_comp) == &dch_b);
	dch_delete(&dch_b);
    }

    /* dch_count(), dch_insert(). */
    {
	cw_dch_t *dch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";

	dch = dch_new(NULL, cw_g_mema, 4, 2, 1, ch_string_hash,
		      ch_string_key_comp);
	cw_check_ptr(dch);
	cw_assert(dch_count(dch) == 0);

	dch_insert(dch, a, a, NULL);
	cw_assert(dch_count(dch) == 1);

	dch_insert(dch, b, b, NULL);
	cw_assert(dch_count(dch) == 2);

	dch_insert(dch, c, c, NULL);
	cw_assert(dch_count(dch) == 3);

	dch_insert(dch, d, d, NULL);
	cw_assert(dch_count(dch) == 4);

	dch_insert(dch, d, d, NULL);
	cw_assert(dch_count(dch) == 5);

	dch_delete(dch);
    }

    /* dch_count, dch_remove(). */
    {
	cw_dch_t *dch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";
	char *k, *v;

	dch = dch_new(NULL, cw_g_mema, 4, 2, 1, ch_string_hash,
		      ch_string_key_comp);
	cw_check_ptr(dch);
	cw_assert(dch_count(dch) == 0);

	dch_insert(dch, a, a, NULL);
	cw_assert(dch_count(dch) == 1);

	dch_insert(dch, b, b, NULL);
	cw_assert(dch_count(dch) == 2);

	dch_insert(dch, c, c, NULL);
	cw_assert(dch_count(dch) == 3);

	dch_insert(dch, d, d, NULL);
	cw_assert(dch_count(dch) == 4);

	cw_assert(dch_remove(dch, a, (void **) &k, (void **) &v, NULL) ==
		  false);
	cw_assert(k == a);
	cw_assert(v == a);
	cw_assert(dch_count(dch) == 3);

	cw_assert(dch_remove(dch, a, NULL, NULL, NULL));

	cw_assert(dch_remove(dch, b, NULL, NULL, NULL) == false);
	cw_assert(dch_count(dch) == 2);

	cw_assert(dch_remove(dch, c, (void **) &k, (void **) &v, NULL) ==
		  false);
	cw_assert(k == d);
	cw_assert(v == d);
	cw_assert(dch_count(dch) == 1);

	cw_assert(dch_remove(dch, c, NULL, NULL, NULL) == false);
	cw_assert(dch_count(dch) == 0);

	cw_assert(dch_remove(dch, d, NULL, NULL, NULL));
	cw_assert(dch_count(dch) == 0);

	cw_assert(dch_remove(dch, a, NULL, NULL, NULL));

	dch_delete(dch);
    }

    /* dch_search(), dch_chi_remove(). */
    {
	cw_dch_t *dch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";
	char *v;
	cw_chi_t chi_a, chi_b, chi_c, chi_d;

	dch = dch_new(NULL, cw_g_mema, 4, 2, 1, ch_string_hash,
		      ch_string_key_comp);
	cw_check_ptr(dch);

	dch_insert(dch, a, a, &chi_a);
	dch_insert(dch, b, b, &chi_b);
	dch_insert(dch, c, c, &chi_c);
	dch_insert(dch, d, d, &chi_d);

	cw_assert(dch_search(dch, "foo", (void **) &v));

	cw_assert(dch_search(dch, a, (void **) &v) == false);
	cw_assert(v == a);

	cw_assert(dch_search(dch, b, (void **) &v) == false);
	cw_assert(v == b);

	cw_assert(dch_search(dch, c, (void **) &v) == false);
	cw_assert(v == d);

	cw_assert(dch_search(dch, d, (void **) &v) == false);
	cw_assert(v == d);

	dch_chi_remove(dch, &chi_b);
	cw_assert(dch_count(dch) == 3);

	dch_chi_remove(dch, &chi_d);
	cw_assert(dch_count(dch) == 2);

	dch_chi_remove(dch, &chi_a);
	cw_assert(dch_count(dch) == 1);

	dch_chi_remove(dch, &chi_c);
	cw_assert(dch_count(dch) == 0);

	dch_delete(dch);
    }

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
