/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Test the list class.
 *
 ****************************************************************************/

#define _STASH_USE_LIST
#include <libstash/libstash_r.h>

int
main()
{
  libstash_init();

  /* list_item_new(), list_item_delete(). */
  {
    cw_list_item_t * list_item;

    list_item = list_item_new();
    _cw_check_ptr(list_item);
    list_item_delete(list_item);
  }

  /* list_item_get(), list_item_set(). */
  {
    cw_list_item_t * list_item;

    list_item = list_item_new();

    _cw_assert(NULL == list_item_get(list_item));
    list_item_set(list_item, (void *) list_item);
    _cw_assert((void *) list_item == list_item_get(list_item));
    list_item_set(list_item, NULL);
    _cw_assert(NULL == list_item_get(list_item));

    list_item_delete(list_item);
  }

  /* list_new(), list_delete(). */
  {
    cw_list_t list_a, * list_b;

    _cw_assert(&list_a == list_new(&list_a, TRUE));
    list_delete(&list_a);

    _cw_assert(&list_a == list_new(&list_a, FALSE));
    list_delete(&list_a);

    list_b = list_new(NULL, TRUE);
    _cw_check_ptr(list_b);
    list_delete(list_b);
    
    list_b = list_new(NULL, FALSE);
    _cw_check_ptr(list_b);
    list_delete(list_b);
  }

  /* list_hpush(), list_hpop, list_hpeek(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_sint32_t ints[13], i;

    list = list_new(NULL, TRUE);

    _cw_assert(0 == list_count(list));

    /* Push all the ints onto the list. */
    for (i = 0; i < 13; i++)
    {
      ints[i] = i;
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_hpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpeek(list));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpop(list));
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_hpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
    }

    /* Pop all the ints off the list. */
    for (i = 12; i >= 0; i--)
    {
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpeek(list));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpop(list));
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_hpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpop(list));
      _cw_assert(i == list_count(list));
    }

    /* Leave something in the list. */
    list_hpush(list, &ints[0]);
    
    list_delete(list);
  }

  /* list_tpush(), list_tpop(), list_tpeek(),
   * list_purge_spares(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_sint32_t ints[13], i;

    list = list_new(NULL, TRUE);

    _cw_assert(0 == list_count(list));

    /* Push all the ints onto the list. */
    for (i = 0; i < 13; i++)
    {
      ints[i] = i;
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_tpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpeek(list));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpop(list));
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_tpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
    }

    /* Pop all the ints off the list. */
    for (i = 12; i >= 0; i--)
    {
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpeek(list));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpop(list));
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_tpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpop(list));
      _cw_assert(i == list_count(list));
    }

    list_purge_spares(list);
      
    /* Leave something in the list. */
    list_tpush(list, &ints[0]);
    list_tpush(list, &ints[1]);

    list_delete(list);
  }

  /* list_insert_before(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_list_item_t * items[11];
    cw_uint32_t ints_a[11], ints_b[11], i;

    list = list_new(NULL, TRUE);

    /* Push the ints onto the list. */
    for (i = 0; i < 11; i++)
    {
      ints_a[i] = i;
      _cw_assert(i == list_count(list));
      items[i] = list_hpush(list, &ints_a[i]);
      _cw_assert((i + 1) == list_count(list));
    }

    /* Insert before each int in the list. */
    for (i = 0; i < 11; i++)
    {
      ints_b[i] = i + 11;
      _cw_assert((i + 11) == list_count(list));
      _cw_assert(NULL != list_insert_before(list, items[i], &ints_b[i]));
      _cw_assert((i + 12) == list_count(list));
    }

    /* Push and pop from each end of the list to make sure the head and tail are
     * still okay.  If they aren't, the problem will show up later. */
    list_hpush(list, list_hpop(list));
    list_tpush(list, list_tpop(list));

    /* Make sure that things are in the proper order in the list. */
    for (i = 0; i < 11; i++)
    {
      _cw_assert(i == *(cw_uint32_t *) list_tpop(list));
      _cw_assert((i + 11) == *(cw_uint32_t *) list_tpop(list));
    }

    _cw_assert(0 == list_count(list));

    list_delete(list);
  }

  /* list_insert_after(),
   * list_purge_spares(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_list_item_t * items[11];
    cw_uint32_t ints_a[11], ints_b[11], i;

    list = list_new(NULL, TRUE);

    /* Push the ints onto the list. */
    for (i = 0; i < 11; i++)
    {
      ints_a[i] = i;
      _cw_assert(i == list_count(list));
      items[i] = list_tpush(list, &ints_a[i]);
      _cw_assert((i + 1) == list_count(list));
    }

    /* Insert before each int in the list. */
    for (i = 0; i < 11; i++)
    {
      ints_b[i] = i + 11;
      _cw_assert((i + 11) == list_count(list));
      _cw_assert(NULL != list_insert_after(list, items[i], &ints_b[i]));
      _cw_assert((i + 12) == list_count(list));
    }

    /* Push and pop from each end of the list to make sure the head and tail are
     * still okay.  If they aren't, the problem will show up later. */
    list_hpush(list, list_hpop(list));
    list_tpush(list, list_tpop(list));

    /* Make sure that things are in the proper order in the list. */
    for (i = 0; i < 11; i++)
    {
      _cw_assert(i == *(cw_uint32_t *) list_hpop(list));
      _cw_assert((i + 11) == *(cw_uint32_t *) list_hpop(list));
    }

    list_purge_spares(list);
    
    _cw_assert(0 == list_count(list));

    list_delete(list);
  }

  /* list_remove(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_list_item_t * items[11];
    cw_uint32_t ints[11], i;

    list = list_new(NULL, TRUE);

    /* Push the ints onto the list. */
    for (i = 0; i < 11; i++)
    {
      ints[i] = i;
      _cw_assert(i == list_count(list));
      items[i] = list_tpush(list, &ints[i]);
      _cw_assert((i + 1) == list_count(list));
    }

    /* Remove all the items in the list. */
    for (i = 0; i < 11; i++)
    {
      _cw_assert((11 - i) == list_count(list));
      _cw_assert(ints[i] == *(cw_uint32_t *) list_remove(list, items[i]));
    }

    _cw_assert(0 == list_count(list));

    list_delete(list);
  }
  
#define NUM_ITEMS 50
  {
    char * strings[NUM_ITEMS], * str_ptr;
    cw_list_t * list1, list2;
    int i;
  

    for (i = 0; i < NUM_ITEMS; i++)
    {
      strings[i] = (char *) _cw_malloc(NUM_ITEMS);
      sprintf(strings[i], "This is string %d", i);
    }

    list1 = list_new(NULL, FALSE);
    list_new(&list2, FALSE);
    _cw_assert(list_count(list1) == 0);
    _cw_assert(list_count(&list2) == 0);
	     
    for (i = 0; i < NUM_ITEMS; i++)
    {
      _cw_assert(list_count(list1) == i);

      list_tpush(list1, (void *) strings[i]);
    }
    _cw_assert(list_count(list1) == NUM_ITEMS);
  
    log_printf(g_log_o, "hpop()ping from list1 and hpush()ing to list2\n");
  
    for (i = 0; i < NUM_ITEMS; i++)
    {
      str_ptr = (char *) list_hpop(list1);
      _cw_check_ptr(str_ptr);
      log_printf(g_log_o, "%s\n", str_ptr);
      list_hpush(&list2, (void *) str_ptr);
    }

    _cw_assert(list_count(list1) == 0);
    _cw_assert(list_count(&list2) == NUM_ITEMS);

    log_printf(g_log_o, "tpop()ping from list2 and hpush()ing to list2\n");
  
    for (i = 0; i < NUM_ITEMS; i++)
    {
      str_ptr = (char *) list_tpop(&list2);
      _cw_check_ptr(str_ptr);
      log_printf(g_log_o, "%s\n", str_ptr);
      list_hpush(&list2, (void *) str_ptr);
    }

    _cw_assert(list_count(list1) == 0);
    log_printf(g_log_o, "list2->count == %d\n", list_count(&list2));
    _cw_assert(list_count(&list2) == NUM_ITEMS);

    log_printf(g_log_o, "tpop()ping from list2 and tpush()ing to list1\n");
  
    for (i = 0; i < NUM_ITEMS; i++)
    {
      str_ptr = (char *) list_tpop(&list2);
      _cw_check_ptr(str_ptr);
      log_printf(g_log_o, "%s\n", str_ptr);
      list_tpush(list1, (void *) str_ptr);
    }

    _cw_assert(list_count(list1) == NUM_ITEMS);
    _cw_assert(list_count(&list2) == 0);
  
    list_delete(list1);
    list_delete(&list2);
  }
  
  libstash_shutdown();
  
  return 0;
}
