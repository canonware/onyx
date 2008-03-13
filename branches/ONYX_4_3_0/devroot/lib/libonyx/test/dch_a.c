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
    libonyx_init();
    fprintf(stderr, "Test begin\n");

    /* dch_new(), dch_delete(). */
    {
	cw_dch_t *dch_a, dch_b;

	dch_a = dch_new(NULL, (cw_opaque_alloc_t *) mem_malloc_e,
			(cw_opaque_dealloc_t *) mem_free_e, cw_g_mem, 2, 2, 1,
			ch_string_hash, ch_string_key_comp);
	cw_check_ptr(dch_a);
	dch_delete(dch_a);

	cw_assert(dch_new(&dch_b, (cw_opaque_alloc_t *) mem_malloc_e,
			  (cw_opaque_dealloc_t *) mem_free_e, cw_g_mem, 4, 3, 1,
			  ch_direct_hash, ch_direct_key_comp) == &dch_b);
	dch_delete(&dch_b);
    }

    /* dch_count(), dch_insert(). */
    {
	cw_dch_t *dch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";

	dch = dch_new(NULL, (cw_opaque_alloc_t *) mem_malloc_e,
		      (cw_opaque_dealloc_t *) mem_free_e, cw_g_mem, 4, 2, 1,
		      ch_string_hash, ch_string_key_comp);
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

	dch = dch_new(NULL, (cw_opaque_alloc_t *) mem_malloc_e,
		      (cw_opaque_dealloc_t *) mem_free_e, cw_g_mem, 4, 2, 1,
		      ch_string_hash, ch_string_key_comp);
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
		  FALSE);
	cw_assert(k == a);
	cw_assert(v == a);
	cw_assert(dch_count(dch) == 3);

	cw_assert(dch_remove(dch, a, NULL, NULL, NULL));

	cw_assert(dch_remove(dch, b, NULL, NULL, NULL) == FALSE);
	cw_assert(dch_count(dch) == 2);

	cw_assert(dch_remove(dch, c, (void **) &k, (void **) &v, NULL) ==
		  FALSE);
	cw_assert(k == d);
	cw_assert(v == d);
	cw_assert(dch_count(dch) == 1);

	cw_assert(dch_remove(dch, c, NULL, NULL, NULL) == FALSE);
	cw_assert(dch_count(dch) == 0);

	cw_assert(dch_remove(dch, d, NULL, NULL, NULL));
	cw_assert(dch_count(dch) == 0);

	cw_assert(dch_remove(dch, a, NULL, NULL, NULL));

	dch_delete(dch);
    }

    /* dch_search(). */
    {
	cw_dch_t *dch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";
	char *v;

	dch = dch_new(NULL, (cw_opaque_alloc_t *) mem_malloc_e,
		      (cw_opaque_dealloc_t *) mem_free_e, cw_g_mem, 4, 2, 1,
		      ch_string_hash, ch_string_key_comp);
	cw_check_ptr(dch);

	dch_insert(dch, a, a, NULL);
	dch_insert(dch, b, b, NULL);
	dch_insert(dch, c, c, NULL);
	dch_insert(dch, d, d, NULL);

	cw_assert(dch_search(dch, "foo", (void **) &v));

	cw_assert(dch_search(dch, a, (void **) &v) == FALSE);
	cw_assert(v == a);

	cw_assert(dch_search(dch, b, (void **) &v) == FALSE);
	cw_assert(v == b);

	cw_assert(dch_search(dch, c, (void **) &v) == FALSE);
	cw_assert(v == d);

	cw_assert(dch_search(dch, d, (void **) &v) == FALSE);
	cw_assert(v == d);

	while (dch_remove_iterate(dch, NULL, NULL, NULL) == FALSE)
	{
	}

	dch_delete(dch);
    }

    /* dch_get_iterate(), dch_remove_iterate(). */
    {
	cw_dch_t *dch;
	char *a = "a string";
	char *b = "A string";
	char *c = "two of these";
	char *d = "two of these\0foo";
	char *k, *v;

	dch = dch_new(NULL, (cw_opaque_alloc_t *) mem_malloc_e,
		      (cw_opaque_dealloc_t *) mem_free_e, cw_g_mem, 3, 2, 1,
		      ch_string_hash, ch_string_key_comp);
	cw_check_ptr(dch);

	dch_insert(dch, a, a, NULL);
	dch_insert(dch, b, b, NULL);
	dch_insert(dch, c, c, NULL);
	dch_insert(dch, d, d, NULL);

	cw_assert(dch_get_iterate(dch, (void **) &k, (void **) &v) == FALSE);
	cw_assert(k == a);
	cw_assert(v == a);

	cw_assert(dch_get_iterate(dch, (void **) &k, (void **) &v) == FALSE);
	cw_assert(k == b);
	cw_assert(v == b);

	cw_assert(dch_get_iterate(dch, (void **) &k, (void **) &v) == FALSE);
	cw_assert(k == c);
	cw_assert(v == c);

	cw_assert(dch_get_iterate(dch, (void **) &k, (void **) &v) == FALSE);
	cw_assert(k == d);
	cw_assert(v == d);

	cw_assert(dch_get_iterate(dch, (void **) &k, (void **) &v) == FALSE);
	cw_assert(k == a);
	cw_assert(v == a);

	cw_assert(dch_remove_iterate(dch, (void **) &k, (void **) &v, NULL)
		  == FALSE);
	cw_assert(k == b);
	cw_assert(v == b);

	cw_assert(dch_remove_iterate(dch, (void **) &k, (void **) &v, NULL)
		  == FALSE);
	cw_assert(k == c);
	cw_assert(v == c);

	cw_assert(dch_remove_iterate(dch, (void **) &k, (void **) &v, NULL)
		  == FALSE);
	cw_assert(k == d);
	cw_assert(v == d);

	cw_assert(dch_remove_iterate(dch, (void **) &k, (void **) &v, NULL)
		  == FALSE);
	cw_assert(a == k);
	cw_assert(a == v);

	cw_assert(dch_remove_iterate(dch, (void **) &k, (void **) &v, NULL));
	cw_assert(dch_get_iterate(dch, (void **) &k, (void **) &v));

	dch_delete(dch);
    }

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
