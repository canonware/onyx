/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * dch (dynamic chained chained hash) test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
  libstash_init();
  _cw_out_put("Test begin\n");

  /* dch_new(), dch_delete(). */
  {
    cw_dch_t * dch_a, dch_b;

    dch_a = dch_new(NULL, 2, 2, 1, NULL, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(dch_a);
    dch_delete(dch_a);

    _cw_assert(&dch_b == dch_new(&dch_b, 4, 3, 1, NULL, ch_hash_direct,
				 ch_key_comp_direct));
    dch_delete(&dch_b);
  }

  /* dch_count(), dch_insert(). */
  {
    cw_dch_t * dch;
    char * a = "a string";
    char * b = "A string";
    char * c = "two of these";
    char * d = "two of these\0foo";

    dch = dch_new(NULL, 4, 2, 1, NULL, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(dch);
    _cw_assert(0 == dch_count(dch));

    _cw_assert(FALSE == dch_insert(dch, a, a));
    _cw_assert(1 == dch_count(dch));
    
    _cw_assert(FALSE == dch_insert(dch, b, b));
    _cw_assert(2 == dch_count(dch));
    
    _cw_assert(FALSE == dch_insert(dch, c, c));
    _cw_assert(3 == dch_count(dch));
    
    _cw_assert(FALSE == dch_insert(dch, d, d));
    _cw_assert(4 == dch_count(dch));

    _cw_assert(FALSE == dch_insert(dch, d, d));
    _cw_assert(5 == dch_count(dch));

    dch_delete(dch);
  }

  /* dch_count, dch_remove(). */
  {
    cw_dch_t * dch;
    char * a = "a string";
    char * b = "A string";
    char * c = "two of these";
    char * d = "two of these\0foo";
    char * k, * v;

    dch = dch_new(NULL, 4, 2, 1, NULL, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(dch);
    _cw_assert(0 == dch_count(dch));

    _cw_assert(FALSE == dch_insert(dch, a, a));
    _cw_assert(1 == dch_count(dch));
    
    _cw_assert(FALSE == dch_insert(dch, b, b));
    _cw_assert(2 == dch_count(dch));
    
    _cw_assert(FALSE == dch_insert(dch, c, c));
    _cw_assert(3 == dch_count(dch));
    
    _cw_assert(FALSE == dch_insert(dch, d, d));
    _cw_assert(4 == dch_count(dch));

    _cw_assert(FALSE == dch_remove(dch, a, (void **) &k, (void **) &v));
    _cw_assert(a == k);
    _cw_assert(a == v);
    _cw_assert(3 == dch_count(dch));

    _cw_assert(TRUE == dch_remove(dch, a, NULL, NULL));

    _cw_assert(FALSE == dch_remove(dch, b, NULL, NULL));
    _cw_assert(2 == dch_count(dch));
    
    _cw_assert(FALSE == dch_remove(dch, c, (void **) &k, (void **) &v));
    _cw_assert(d == k);
    _cw_assert(d == v);
    _cw_assert(1 == dch_count(dch));

    _cw_assert(FALSE == dch_remove(dch, c, NULL, NULL));
    _cw_assert(0 == dch_count(dch));
    
    _cw_assert(TRUE == dch_remove(dch, d, NULL, NULL));
    _cw_assert(0 == dch_count(dch));
    
    _cw_assert(TRUE == dch_remove(dch, a, NULL, NULL));
    
    dch_delete(dch);
  }

  /* dch_search(). */
  {
    cw_dch_t * dch;
    cw_pezz_t * chi_pezz;
    char * a = "a string";
    char * b = "A string";
    char * c = "two of these";
    char * d = "two of these\0foo";
    char * v;

    chi_pezz = pezz_new(NULL, sizeof(cw_chi_t), 10);
    _cw_check_ptr(chi_pezz);

    dch = dch_new(NULL, 4, 2, 1, chi_pezz, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(dch);

    _cw_assert(FALSE == dch_insert(dch, a, a));
    _cw_assert(FALSE == dch_insert(dch, b, b));
    _cw_assert(FALSE == dch_insert(dch, c, c));
    _cw_assert(FALSE == dch_insert(dch, d, d));

    _cw_assert(TRUE == dch_search(dch, "foo", (void **) &v));

    _cw_assert(FALSE == dch_search(dch, a, (void **) &v));
    _cw_assert(a == v);
    
    _cw_assert(FALSE == dch_search(dch, b, (void **) &v));
    _cw_assert(b == v);
    
    _cw_assert(FALSE == dch_search(dch, c, (void **) &v));
    _cw_assert(d == v);
    
    _cw_assert(FALSE == dch_search(dch, d, (void **) &v));
    _cw_assert(d == v);

    dch_delete(dch);
    pezz_delete(chi_pezz);
  }
  
  /* dch_get_iterate(), dch_remove_iterate(). */
  {
    cw_dch_t * dch;
    cw_pezz_t * chi_pezz;
    char * a = "a string";
    char * b = "A string";
    char * c = "two of these";
    char * d = "two of these\0foo";
    char * k, * v;

    chi_pezz = pezz_new(NULL, sizeof(cw_chi_t), 10);
    _cw_check_ptr(chi_pezz);

    dch = dch_new(NULL, 3, 2, 1, chi_pezz, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(dch);

    _cw_assert(FALSE == dch_insert(dch, a, a));
    _cw_assert(FALSE == dch_insert(dch, b, b));
    _cw_assert(FALSE == dch_insert(dch, c, c));
    _cw_assert(FALSE == dch_insert(dch, d, d));

    _cw_assert(FALSE == dch_get_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(a == k);
    _cw_assert(a == v);
    
    _cw_assert(FALSE == dch_get_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(b == k);
    _cw_assert(b == v);
    
    _cw_assert(FALSE == dch_get_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(c == k);
    _cw_assert(c == v);
    
    _cw_assert(FALSE == dch_get_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(d == k);
    _cw_assert(d == v);
    
    _cw_assert(FALSE == dch_get_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(a == k);
    _cw_assert(a == v);

    _cw_assert(FALSE == dch_remove_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(b == k);
    _cw_assert(b == v);

    _cw_assert(FALSE == dch_remove_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(c == k);
    _cw_assert(c == v);

    _cw_assert(FALSE == dch_remove_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(d == k);
    _cw_assert(d == v);

    _cw_assert(FALSE == dch_remove_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(a == k);
    _cw_assert(a == v);

    _cw_assert(TRUE == dch_remove_iterate(dch, (void **) &k, (void **) &v));
    _cw_assert(TRUE == dch_get_iterate(dch, (void **) &k, (void **) &v));

    dch_delete(dch);
    pezz_delete(chi_pezz);
  }

  _cw_out_put("Test end\n");
  libstash_shutdown();
  return 0;
}
