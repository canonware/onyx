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
 * ch (chained hash) test.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

int
main()
{
    libonyx_init(0, NULL, NULL);
    fprintf(stderr, "Test begin\n");

    /* ch_new(), ch_delete(). */
    {
	cw_ch_t *ch_a, ch_b;

	ch_a = ch_new(NULL, cw_g_mema, 2, ch_string_hash, ch_string_key_comp);
	cw_check_ptr(ch_a);
	ch_delete(ch_a);

	cw_assert(ch_new(&ch_b, cw_g_mema, 1, ch_direct_hash,
			 ch_direct_key_comp)
		  == &ch_b);
	ch_delete(&ch_b);
    }

    /* ch_count(), ch_insert(). */
    {
	cw_ch_t *ch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";

	ch = ch_new(NULL, cw_g_mema, 4, ch_string_hash, ch_string_key_comp);
	cw_check_ptr(ch);
	cw_assert(ch_count(ch) == 0);

	ch_insert(ch, a, a, NULL);
	cw_assert(ch_count(ch) == 1);

	ch_insert(ch, b, b, NULL);
	cw_assert(ch_count(ch) == 2);

	ch_insert(ch, c, c, NULL);
	cw_assert(ch_count(ch) == 3);

	ch_insert(ch, d, d, NULL);
	cw_assert(ch_count(ch) == 4);

	ch_insert(ch, d, d, NULL);
	cw_assert(ch_count(ch) == 5);

	ch_delete(ch);
    }

    /* ch_count, ch_remove(). */
    {
	cw_ch_t *ch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";
	char *k, *v;

	ch = ch_new(NULL, cw_g_mema, 4, ch_string_hash, ch_string_key_comp);
	cw_check_ptr(ch);
	cw_assert(ch_count(ch) == 0);

	ch_insert(ch, a, a, NULL);
	cw_assert(ch_count(ch) == 1);

	ch_insert(ch, b, b, NULL);
	cw_assert(ch_count(ch) == 2);

	ch_insert(ch, c, c, NULL);
	cw_assert(ch_count(ch) == 3);

	ch_insert(ch, d, d, NULL);
	cw_assert(ch_count(ch) == 4);

	cw_assert(ch_remove(ch, a, (void **) &k, (void **) &v, NULL) == false);
	cw_assert(k == a);
	cw_assert(v == a);
	cw_assert(ch_count(ch) == 3);

	cw_assert(ch_remove(ch, a, NULL, NULL, NULL));

	cw_assert(ch_remove(ch, b, NULL, NULL, NULL) == false);
	cw_assert(ch_count(ch) == 2);

	cw_assert(ch_remove(ch, c, (void **) &k, (void **) &v, NULL) == false);
	cw_assert(k == d);
	cw_assert(v == d);
	cw_assert(ch_count(ch) == 1);

	cw_assert(ch_remove(ch, c, NULL, NULL, NULL) == false);
	cw_assert(ch_count(ch) == 0);

	cw_assert(ch_remove(ch, d, NULL, NULL, NULL));
	cw_assert(ch_count(ch) == 0);

	cw_assert(ch_remove(ch, a, NULL, NULL, NULL));

	ch_delete(ch);
    }

    /* ch_search(), ch_chi_remove(). */
    {
	cw_ch_t *ch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";
	char *v;
	cw_chi_t chi_a, chi_b, chi_c, chi_d;

	ch = ch_new(NULL, cw_g_mema, 4, ch_string_hash, ch_string_key_comp);
	cw_check_ptr(ch);

	ch_insert(ch, a, a, &chi_a);
	ch_insert(ch, b, b, &chi_b);
	ch_insert(ch, c, c, &chi_c);
	ch_insert(ch, d, d, &chi_d);

	cw_assert(ch_search(ch, "foo", (void **) &v));

	cw_assert(ch_search(ch, a, (void **) &v) == false);
	cw_assert(v == a);

	cw_assert(ch_search(ch, b, (void **) &v) == false);
	cw_assert(v == b);

	cw_assert(ch_search(ch, c, (void **) &v) == false);
	cw_assert(v == d);

	cw_assert(ch_search(ch, d, (void **) &v) == false);
	cw_assert(v == d);

	ch_chi_remove(ch, &chi_b);
	cw_assert(ch_count(ch) == 3);

	ch_chi_remove(ch, &chi_d);
	cw_assert(ch_count(ch) == 2);

	ch_chi_remove(ch, &chi_a);
	cw_assert(ch_count(ch) == 1);

	ch_chi_remove(ch, &chi_c);
	cw_assert(ch_count(ch) == 0);

	ch_delete(ch);
    }

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
