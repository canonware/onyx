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
    cw_ch_t * ch_a, ch_b;

    ch_a = ch_new(NULL, 2, NULL, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(ch_a);
    ch_delete(ch_a);

    _cw_assert(&ch_b == ch_new(&ch_b, 1, NULL, ch_hash_direct,
			       ch_key_comp_direct));
    ch_delete(&ch_b);
  }

  /* ch_count(), ch_insert(). */
  {
    cw_ch_t * ch;
    char * a = "a string";
    char * b = "A string";
    char * c = "two of these";
    char * d = "two of these\0foo";

    ch = ch_new(NULL, 4, NULL, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(ch);
    _cw_assert(0 == ch_count(ch));

    _cw_assert(FALSE == ch_insert(ch, a, a));
    _cw_assert(1 == ch_count(ch));
    
    _cw_assert(FALSE == ch_insert(ch, b, b));
    _cw_assert(2 == ch_count(ch));
    
    _cw_assert(FALSE == ch_insert(ch, c, c));
    _cw_assert(3 == ch_count(ch));
    
    _cw_assert(FALSE == ch_insert(ch, d, d));
    _cw_assert(4 == ch_count(ch));

    _cw_assert(FALSE == ch_insert(ch, d, d));
    _cw_assert(5 == ch_count(ch));

    ch_delete(ch);
  }

  /* ch_count, ch_remove(). */
  {
    cw_ch_t * ch;
    char * a = "a string";
    char * b = "A string";
    char * c = "two of these";
    char * d = "two of these\0foo";
    char * k, * v;

    ch = ch_new(NULL, 4, NULL, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(ch);
    _cw_assert(0 == ch_count(ch));

    _cw_assert(FALSE == ch_insert(ch, a, a));
    _cw_assert(1 == ch_count(ch));
    
    _cw_assert(FALSE == ch_insert(ch, b, b));
    _cw_assert(2 == ch_count(ch));
    
    _cw_assert(FALSE == ch_insert(ch, c, c));
    _cw_assert(3 == ch_count(ch));
    
    _cw_assert(FALSE == ch_insert(ch, d, d));
    _cw_assert(4 == ch_count(ch));

    _cw_assert(FALSE == ch_remove(ch, a, (void **) &k, (void **) &v));
    _cw_assert(a == k);
    _cw_assert(a == v);
    _cw_assert(3 == ch_count(ch));

    _cw_assert(TRUE == ch_remove(ch, a, NULL, NULL));

    _cw_assert(FALSE == ch_remove(ch, b, NULL, NULL));
    _cw_assert(2 == ch_count(ch));
    
    _cw_assert(FALSE == ch_remove(ch, c, (void **) &k, (void **) &v));
    _cw_assert(d == k);
    _cw_assert(d == v);
    _cw_assert(1 == ch_count(ch));

    _cw_assert(FALSE == ch_remove(ch, c, NULL, NULL));
    _cw_assert(0 == ch_count(ch));
    
    _cw_assert(TRUE == ch_remove(ch, d, NULL, NULL));
    _cw_assert(0 == ch_count(ch));
    
    _cw_assert(TRUE == ch_remove(ch, a, NULL, NULL));
    
    ch_delete(ch);
  }

  /* ch_search(). */
  {
    cw_ch_t * ch;
    cw_pezz_t * chi_pezz;
    char * a = "a string";
    char * b = "A string";
    char * c = "two of these";
    char * d = "two of these\0foo";
    char * v;

    chi_pezz = pezz_new(NULL, sizeof(cw_chi_t), 10);
    _cw_check_ptr(chi_pezz);

    ch = ch_new(NULL, 4, chi_pezz, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(ch);

    _cw_assert(FALSE == ch_insert(ch, a, a));
    _cw_assert(FALSE == ch_insert(ch, b, b));
    _cw_assert(FALSE == ch_insert(ch, c, c));
    _cw_assert(FALSE == ch_insert(ch, d, d));

    _cw_assert(TRUE == ch_search(ch, "foo", (void **) &v));

    _cw_assert(FALSE == ch_search(ch, a, (void **) &v));
    _cw_assert(a == v);
    
    _cw_assert(FALSE == ch_search(ch, b, (void **) &v));
    _cw_assert(b == v);
    
    _cw_assert(FALSE == ch_search(ch, c, (void **) &v));
    _cw_assert(d == v);
    
    _cw_assert(FALSE == ch_search(ch, d, (void **) &v));
    _cw_assert(d == v);

    ch_delete(ch);
    pezz_delete(chi_pezz);
  }
  
  /* ch_get_iterate(), ch_remove_iterate(). */
  {
    cw_ch_t * ch;
    cw_pezz_t * chi_pezz;
    char * a = "a string";
    char * b = "A string";
    char * c = "two of these";
    char * d = "two of these\0foo";
    char * k, * v;

    chi_pezz = pezz_new(NULL, sizeof(cw_chi_t), 10);
    _cw_check_ptr(chi_pezz);

    ch = ch_new(NULL, 4, chi_pezz, ch_hash_string, ch_key_comp_string);
    _cw_check_ptr(ch);

    _cw_assert(FALSE == ch_insert(ch, a, a));
    _cw_assert(FALSE == ch_insert(ch, b, b));
    _cw_assert(FALSE == ch_insert(ch, c, c));
    _cw_assert(FALSE == ch_insert(ch, d, d));

    _cw_assert(FALSE == ch_get_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(a == k);
    _cw_assert(a == v);
    
    _cw_assert(FALSE == ch_get_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(b == k);
    _cw_assert(b == v);
    
    _cw_assert(FALSE == ch_get_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(c == k);
    _cw_assert(c == v);
    
    _cw_assert(FALSE == ch_get_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(d == k);
    _cw_assert(d == v);
    
    _cw_assert(FALSE == ch_get_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(a == k);
    _cw_assert(a == v);

    _cw_assert(FALSE == ch_remove_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(b == k);
    _cw_assert(b == v);

    _cw_assert(FALSE == ch_remove_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(c == k);
    _cw_assert(c == v);

    _cw_assert(FALSE == ch_remove_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(d == k);
    _cw_assert(d == v);

    _cw_assert(FALSE == ch_remove_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(a == k);
    _cw_assert(a == v);

    _cw_assert(TRUE == ch_remove_iterate(ch, (void **) &k, (void **) &v));
    _cw_assert(TRUE == ch_get_iterate(ch, (void **) &k, (void **) &v));

    ch_delete(ch);
    pezz_delete(chi_pezz);
  }

  _cw_out_put("Test end\n");
  libstash_shutdown();
  return 0;
}
